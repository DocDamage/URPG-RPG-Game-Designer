#include "editor/database/database_reference_workspace.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::editor::ProjectDatabaseTableModel allTables() {
    urpg::editor::ProjectDatabaseTableModel tables;
    for (const auto kind : urpg::editor::allDatabaseTableKinds()) {
        tables.table(kind).replaceRows({
            {std::string(urpg::editor::databaseReferenceType(kind)) + ".one", "One", "", {}, 0},
            {std::string(urpg::editor::databaseReferenceType(kind)) + ".two", "Two", "", {}, 0},
        });
    }
    return tables;
}

urpg::project::ProjectReferenceIndex referenceIndex() {
    urpg::project::ProjectReferenceIndex index;
    REQUIRE(index.rebuild({{"content/quests/intro.json",
                            {{"quest", "quest.one", "item", "item.one", "reward", {}, "reward:0", true},
                             {"quest", "quest.one", "switch", "switch.one", "condition", {}, "condition:0", false}}},
                           {"content/vendors/town.json",
                            {{"vendor", "vendor.one", "item", "item.one", "stock", {}, "stock:0", true}}}})
                .success);
    return index;
}

} // namespace

TEST_CASE("Database reference pickers cover every golden record kind with stable ids",
          "[editor][database][references][pcq452]") {
    const auto tables = allTables();
    const auto index = referenceIndex();
    const urpg::editor::DatabaseReferenceWorkspace workspace(tables, index);
    for (const auto kind : urpg::editor::allDatabaseTableKinds()) {
        const auto options = workspace.pickerOptions(kind, "two", 1);
        REQUIRE(options.size() == 1);
        REQUIRE(options.front().kind == kind);
        REQUIRE(options.front().id == std::string(urpg::editor::databaseReferenceType(kind)) + ".two");
        REQUIRE(options.front().orphan);
    }
}

TEST_CASE("Database find uses and orphan detection delegate to project reference authority",
          "[editor][database][references][pcq452]") {
    const auto tables = allTables();
    const auto index = referenceIndex();
    const urpg::editor::DatabaseReferenceWorkspace workspace(tables, index);
    const auto uses = workspace.findUses(urpg::editor::DatabaseTableKind::Items, "item.one");
    REQUIRE(uses.success);
    REQUIRE(uses.matches.size() == 2);
    REQUIRE(uses.matches[0].document_path == "content/quests/intro.json");
    REQUIRE(workspace.orphanIds(urpg::editor::DatabaseTableKind::Items) == std::vector<std::string>{"item.two"});
    REQUIRE(workspace.orphanIds(urpg::editor::DatabaseTableKind::Switches) ==
            std::vector<std::string>{"switch.two"});
}

TEST_CASE("Database delete and replacement previews prevent dangling references",
          "[editor][database][references][pcq452]") {
    const auto tables = allTables();
    const auto index = referenceIndex();
    const urpg::editor::DatabaseReferenceWorkspace workspace(tables, index);

    const auto blocked = workspace.previewDelete(urpg::editor::DatabaseTableKind::Items, "item.one", "delete.item");
    REQUIRE(blocked.record_found);
    REQUIRE_FALSE(blocked.safe);
    REQUIRE(blocked.code == "project_reference_delete_blocked");
    REQUIRE(blocked.uses.size() == 2);
    REQUIRE(blocked.shared_plan.package_impact.size() == 2);

    const auto safe = workspace.previewDelete(urpg::editor::DatabaseTableKind::Items, "item.two", "delete.orphan");
    REQUIRE(safe.safe);
    REQUIRE(safe.code == "project_reference_delete_safe");

    const auto replacement = workspace.previewReplace(urpg::editor::DatabaseTableKind::Items, "item.one",
                                                       "item.two", "replace.item");
    REQUIRE(replacement.record_found);
    REQUIRE(replacement.replacement_found);
    REQUIRE(replacement.safe);
    REQUIRE(replacement.shared_plan.updates.size() == 2);
    REQUIRE(replacement.shared_plan.updates[0].after.target_id == "item.two");
    REQUIRE(replacement.shared_plan.inverse_request.source_id == "item.two");

    const auto rename = workspace.previewRename(urpg::editor::DatabaseTableKind::Items, "item.one",
                                                "item.renamed", "rename.item");
    REQUIRE(rename.safe);
    REQUIRE(rename.uses.size() == 2);
    REQUIRE(rename.shared_plan.updates[0].after.target_id == "item.renamed");

    const auto missing = workspace.previewReplace(urpg::editor::DatabaseTableKind::Items, "item.one",
                                                   "item.missing", "replace.missing");
    REQUIRE_FALSE(missing.safe);
    REQUIRE(missing.code == "database_replacement_missing");
}
