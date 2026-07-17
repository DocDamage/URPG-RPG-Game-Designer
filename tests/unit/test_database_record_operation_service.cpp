#include "editor/database/database_record_operation_service.h"
#include "editor/database/database_reference_workspace.h"
#include "editor/database/database_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

namespace {

using namespace urpg::editor;
using namespace urpg::project;

ProjectDatabaseTableModel allTables() {
    ProjectDatabaseTableModel tables;
    for (const auto kind : allDatabaseTableKinds()) {
        const auto type = std::string(databaseReferenceType(kind));
        tables.table(kind).replaceRows({{type + ".one", "One", "", {}, 1},
                                       {type + ".two", "Two", "", {}, 1}});
    }
    return tables;
}

struct DurableValueOwner {
    std::filesystem::path path;
    std::string value;
    std::string before;
    uint64_t revision = 1;

    bool write(std::string& diagnostic) const {
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            diagnostic = error.message();
            return false;
        }
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output << nlohmann::json{{"value", value}, {"revision", revision}}.dump(2) << '\n';
        if (!output) diagnostic = "database_record_test_owner_write_failed";
        return static_cast<bool>(output);
    }
};

ProjectOperationParticipant participant(const std::string& owner_id,
                                        const std::shared_ptr<DurableValueOwner>& owner,
                                        std::string after) {
    const auto original = owner->value;
    return {
        owner_id,
        owner->revision,
        [owner] { return owner->revision; },
        [](std::string& diagnostic) { diagnostic.clear(); return true; },
        [owner, after = std::move(after)](std::string& diagnostic) {
            owner->before = owner->value;
            owner->value = after;
            ++owner->revision;
            return owner->write(diagnostic);
        },
        [owner, original] {
            owner->value = original;
            ++owner->revision;
            std::string ignored;
            (void)owner->write(ignored);
        },
        [owner, original](std::string& diagnostic) {
            owner->value = original;
            ++owner->revision;
            return owner->write(diagnostic);
        },
        [owner] { return nlohmann::json{{"value", owner->value}, {"revision", owner->revision}}.dump(); },
        owner->path,
    };
}

std::string reopenedValue(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return nlohmann::json::parse(input).value("value", "");
}

} // namespace

TEST_CASE("Reviewed database rename commits every golden record kind through typed owners",
          "[editor][database][references][pcq452][project_operation]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_database_record_rename_all_kinds";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto tables = allTables();

    for (const auto kind : allDatabaseTableKinds()) {
        const auto type = std::string(databaseReferenceType(kind));
        const auto source = type + ".one";
        const auto renamed = type + ".renamed";
        const auto recordPath = root / "content/database" / (type + ".json");
        const auto usePath = root / "content/typed_owners" / (type + "_use.json");
        ProjectReferenceIndex index;
        REQUIRE(index.rebuild({{usePath, {{"typed_consumer", type + ".consumer", type, source,
                                         "golden_slice_reference", {}, "field:target", true}}}}).success);
        DatabaseRecordOperationService service;
        const auto preview = service.preview(
            tables, index, {"database.rename." + type, DatabaseRecordOperationKind::Rename,
                            kind, source, renamed, recordPath});
        REQUIRE(preview.success);
        REQUIRE(preview.applicable);
        REQUIRE(preview.reference_plan.updates.size() == 1);

        auto recordOwner = std::make_shared<DurableValueOwner>(DurableValueOwner{recordPath, source, {}, 1});
        auto useOwner = std::make_shared<DurableValueOwner>(DurableValueOwner{usePath, source, {}, 1});
        const auto applied = service.execute(
            preview, {participant("database." + type, recordOwner, renamed),
                      participant("typed_consumer." + type, useOwner, renamed)});
        REQUIRE(applied.success);
        REQUIRE(reopenedValue(recordPath) == renamed);
        REQUIRE(reopenedValue(usePath) == renamed);

        ProjectReferenceIndex reopened;
        REQUIRE(reopened.rebuild({{usePath, {{"typed_consumer", type + ".consumer", type, renamed,
                                             "golden_slice_reference", {}, "field:target", true}}}}).success);
        REQUIRE(reopened.inbound(type, source).empty());
        REQUIRE(reopened.inbound(type, renamed).size() == 1);
    }
    std::filesystem::remove_all(root, error);
}

TEST_CASE("Reviewed database delete and replacement save reopen and share undo redo history",
          "[editor][database][references][pcq452][roundtrip][undo]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_database_record_safe_changes";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto tables = allTables();
    const auto recordPath = root / "content/database/items.json";
    const auto usePath = root / "content/quests/reward.json";

    ProjectReferenceIndex orphanIndex;
    REQUIRE(orphanIndex.rebuild({}).success);
    DatabaseRecordOperationService deleteService;
    const auto deletion = deleteService.preview(
        tables, orphanIndex, {"database.delete.orphan", DatabaseRecordOperationKind::Delete,
                              DatabaseTableKind::Items, "item.two", {}, recordPath});
    REQUIRE(deletion.success);
    REQUIRE(deletion.applicable);
    auto deletedRecord = std::make_shared<DurableValueOwner>(DurableValueOwner{recordPath, "item.two", {}, 1});
    REQUIRE(deleteService.execute(deletion, {participant("database.items", deletedRecord, "deleted")}).success);
    REQUIRE(reopenedValue(recordPath) == "deleted");

    ProjectReferenceIndex usedIndex;
    REQUIRE(usedIndex.rebuild({{usePath, {{"quest", "quest.reward", "item", "item.one",
                                         "reward_item", {}, "reward:0", true}}}}).success);
    DatabaseRecordOperationService replaceService(root / ".urpg/project_operations/database.json");
    const auto replacement = replaceService.preview(
        tables, usedIndex, {"database.replace.item", DatabaseRecordOperationKind::Replace,
                            DatabaseTableKind::Items, "item.one", "item.two", recordPath});
    REQUIRE(replacement.success);
    REQUIRE(replacement.applicable);
    auto sourceRecord = std::make_shared<DurableValueOwner>(DurableValueOwner{recordPath, "item.one", {}, 1});
    auto questOwner = std::make_shared<DurableValueOwner>(DurableValueOwner{usePath, "item.one", {}, 1});
    REQUIRE(replaceService.execute(
        replacement, {participant("database.items", sourceRecord, "deleted:item.one"),
                      participant("quest.reward", questOwner, "item.two")}).success);
    REQUIRE(reopenedValue(recordPath) == "deleted:item.one");
    REQUIRE(reopenedValue(usePath) == "item.two");
    REQUIRE(replaceService.undoLabel() == "Replace Database Record");
    REQUIRE(replaceService.undoLast().success);
    REQUIRE(reopenedValue(recordPath) == "item.one");
    REQUIRE(reopenedValue(usePath) == "item.one");
    REQUIRE(replaceService.redoLast().success);
    REQUIRE(reopenedValue(recordPath) == "deleted:item.one");
    REQUIRE(reopenedValue(usePath) == "item.two");
    std::filesystem::remove_all(root, error);
}

TEST_CASE("Database record apply rejects incomplete typed owner coverage before mutation",
          "[editor][database][references][pcq452][atomic]") {
    const auto tables = allTables();
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({{"content/quests/reward.json",
                            {{"quest", "quest.reward", "item", "item.one", "reward_item", {}, "reward:0", true}}}})
                .success);
    DatabaseRecordOperationService service;
    const auto preview = service.preview(
        tables, index, {"database.replace.uncovered", DatabaseRecordOperationKind::Replace,
                        DatabaseTableKind::Items, "item.one", "item.two", "content/database.json"});
    auto recordOwner = std::make_shared<DurableValueOwner>(
        DurableValueOwner{"content/database.json", "item.one", {}, 1});
    const auto result = service.execute(preview, {participant("database.items", recordOwner, "deleted")});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "database_record_operation_owner_coverage_missing");
    REQUIRE(recordOwner->value == "item.one");
    REQUIRE(recordOwner->revision == 1);
}

TEST_CASE("Database panel exposes reviewed reference impact before explicit apply",
          "[editor][database][references][pcq452][panel]") {
    DatabasePanel panel;
    for (const auto kind : allDatabaseTableKinds()) {
        const auto type = std::string(databaseReferenceType(kind));
        panel.tables().table(kind).replaceRows({{type + ".one", "One", "", {}, 1},
                                                {type + ".two", "Two", "", {}, 1}});
    }
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({{"content/quests/reward.json",
                            {{"quest", "quest.reward", "item", "item.one", "reward_item", {}, "reward:0", true}}}})
                .success);
    panel.bindReferenceIndex(&index);
    const auto preview = panel.previewRecordOperation(
        {"database.panel.replace", DatabaseRecordOperationKind::Replace, DatabaseTableKind::Items,
         "item.one", "item.two", "content/database.json"});
    REQUIRE(preview.applicable);
    const auto visible = panel.snapshot();
    REQUIRE(visible.reference_review_pending);
    REQUIRE(visible.reference_review_can_apply);
    REQUIRE(visible.reference_review_use_count == 1);
    REQUIRE(visible.reference_review_package_impact_count == 1);
    REQUIRE(visible.reference_review_code == "project_reference_change_preview_ready");
}
