#include "engine/core/assets/project_asset_operation_service.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

using namespace urpg::assets;
using namespace urpg::project;

namespace {

ProjectReferenceIndex operationIndex() {
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({
        {"content/maps/start.p2d.json", {{"map", "start", "asset", "asset.hero.old", "event_sprite", {}, "event:hero", true}}},
        {"content/assets/asset.hero.old.json", {{"asset", "asset.hero.old", "license", "license.private", "license", {}, "asset", true}}},
    }).success);
    return index;
}

AssetLibrary duplicateLibrary() {
    AssetLibrary library;
    library.ingestDuplicateCsv("sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n"
                               "samehash,12,asset.hero.old,asset.hero.new,True\n"
                               "samehash,12,asset.hero.new,asset.hero.new,False\n");
    return library;
}

} // namespace

TEST_CASE("Project asset lifecycle previews all reference-aware operations", "[assets][project_operation]") {
    ProjectAssetOperationService service;
    const auto index = operationIndex();
    const auto library = duplicateLibrary();
    const std::vector replacementKinds{ProjectAssetOperationKind::Relink, ProjectAssetOperationKind::Replace,
                                       ProjectAssetOperationKind::Deduplicate, ProjectAssetOperationKind::Rename};
    for (const auto kind : replacementKinds) {
        const auto preview = service.preview(index, library,
            {std::string("asset.") + projectAssetOperationName(kind), kind, "asset.hero.old", "asset.hero.new", {}});
        REQUIRE(preview.success);
        REQUIRE(preview.applicable);
        REQUIRE(preview.reference_plan.updates.size() == 1);
        REQUIRE(preview.reference_plan.package_impact.size() == 1);
        REQUIRE_FALSE(preview.label.empty());
    }
    const auto move = service.preview(index, library,
        {"asset.move", ProjectAssetOperationKind::Move, "asset.hero.old", {}, "content/assets/characters/hero.json"});
    REQUIRE(move.success);
    REQUIRE(move.applicable);
    REQUIRE(move.reference_plan.updates.size() == 1);
    REQUIRE(move.reference_plan.inverse_request.destination_document == "content/assets/asset.hero.old.json");

    for (const auto kind : {ProjectAssetOperationKind::Detach, ProjectAssetOperationKind::Delete}) {
        const auto blocked = service.preview(index, library,
            {std::string("asset.") + projectAssetOperationName(kind), kind, "asset.hero.old", {}, {}});
        REQUIRE(blocked.success);
        REQUIRE_FALSE(blocked.applicable);
        REQUIRE(blocked.reference_plan.blocked_references.size() == 1);
    }
    const auto mismatch = service.preview(index, AssetLibrary{},
        {"asset.dedup.bad", ProjectAssetOperationKind::Deduplicate, "asset.hero.old", "asset.hero.new", {}});
    REQUIRE_FALSE(mismatch.success);
    REQUIRE(mismatch.code == "project_asset_deduplicate_hash_mismatch");
}

TEST_CASE("Project asset lifecycle executes as one undoable typed-owner operation", "[assets][project_operation][undo]") {
    ProjectAssetOperationService service;
    const auto preview = service.preview(operationIndex(), duplicateLibrary(),
        {"asset.replace", ProjectAssetOperationKind::Replace, "asset.hero.old", "asset.hero.new", {}});
    std::string documentTarget = "asset.hero.old";
    std::uint64_t revision = 4;
    auto participant = ProjectOperationParticipant{
        "map:start", revision, [&] { return revision; },
        [&](std::string&) { return documentTarget == "asset.hero.old"; },
        [&](std::string&) { documentTarget = "asset.hero.new"; ++revision; return true; },
        [&] { documentTarget = "asset.hero.old"; ++revision; },
        [&](std::string&) { documentTarget = "asset.hero.old"; ++revision; return true; },
        {},
        "content/maps/start.p2d.json",
    };
    const auto applied = service.execute(preview, {participant});
    REQUIRE(applied.success);
    REQUIRE(documentTarget == "asset.hero.new");
    REQUIRE(service.undoLabel() == "Replace Asset");
    REQUIRE(service.undoLast().success);
    REQUIRE(documentTarget == "asset.hero.old");
    REQUIRE(service.redoLabel() == "Replace Asset");
    REQUIRE(service.redoLast().success);
    REQUIRE(documentTarget == "asset.hero.new");
}

TEST_CASE("Project asset lifecycle records a durable real-owner operation boundary",
          "[assets][project_operation][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_asset_operation_journal_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto journalPath = root / "journal.json";
    ProjectAssetOperationService service(journalPath);
    const auto preview = service.preview(operationIndex(), duplicateLibrary(),
        {"asset.replace.journaled", ProjectAssetOperationKind::Replace,
         "asset.hero.old", "asset.hero.new", {}});
    std::string documentTarget = "asset.hero.old";
    std::uint64_t revision = 4;
    auto participant = ProjectOperationParticipant{
        "map:start", revision, [&] { return revision; },
        [&](std::string&) { return documentTarget == "asset.hero.old"; },
        [&](std::string&) { documentTarget = "asset.hero.new"; ++revision; return true; },
        [&] { documentTarget = "asset.hero.old"; ++revision; },
        [&](std::string&) { documentTarget = "asset.hero.old"; ++revision; return true; },
        [&] { return documentTarget; },
        "content/maps/start.p2d.json",
    };
    REQUIRE(service.execute(preview, {participant}).success);
    ProjectOperationJournal journal(journalPath);
    const auto recovered = journal.recover();
    REQUIRE(recovered.success);
    REQUIRE(recovered.has_acknowledged_operation);
    REQUIRE(recovered.last_acknowledged.operation_id == "asset.replace.journaled");
    REQUIRE(recovered.last_acknowledged.owners[0].snapshot == "asset.hero.new");
    std::filesystem::remove_all(root, error);
}

TEST_CASE("Project asset lifecycle refuses partial typed-owner coverage",
          "[assets][project_operation][atomic]") {
    ProjectAssetOperationService service;
    const auto preview = service.preview(operationIndex(), duplicateLibrary(),
        {"asset.replace.uncovered", ProjectAssetOperationKind::Replace,
         "asset.hero.old", "asset.hero.new", {}});
    std::string documentTarget = "asset.hero.old";
    std::uint64_t revision = 1;
    auto wrongOwner = ProjectOperationParticipant{
        "other", revision, [&] { return revision; }, [](std::string&) { return true; },
        [&](std::string&) { documentTarget = "asset.hero.new"; ++revision; return true; },
        [&] { documentTarget = "asset.hero.old"; --revision; }, [](std::string&) { return true; }, {},
        "content/dialogues/other.json",
    };
    const auto result = service.execute(preview, {wrongOwner});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_asset_operation_owner_coverage_missing");
    REQUIRE(documentTarget == "asset.hero.old");
    REQUIRE(revision == 1);
}

TEST_CASE("Reference-aware replacement commits map event and data files as one reopenable history step",
          "[project][project_operation][map][event][data][roundtrip]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_cross_owner_reference_operation";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const std::vector<std::filesystem::path> paths = {
        root / "content/maps/start.p2d.json",
        root / "content/events/guide.json",
        root / "content/database.json",
    };
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({
        {paths[0], {{"map", "start", "asset", "asset.hero.old", "tile_asset", {}, "tile:0", true}}},
        {paths[1], {{"event", "guide", "asset", "asset.hero.old", "portrait_asset", {}, "command:0", true}}},
        {paths[2], {{"actor", "hero", "asset", "asset.hero.old", "actor_portrait", {}, "actor:hero", true}}},
    }).success);
    ProjectAssetOperationService service(root / ".urpg/project_operations/journal.json");
    const auto preview = service.preview(
        index, duplicateLibrary(),
        {"asset.replace.cross_owner", ProjectAssetOperationKind::Replace,
         "asset.hero.old", "asset.hero.new", {}});
    REQUIRE(preview.success);
    REQUIRE(preview.reference_plan.updates.size() == 3);

    struct FileOwner {
        std::filesystem::path path;
        std::string value = "asset.hero.old";
        std::string before;
        std::uint64_t revision = 1;
    };
    std::vector<std::shared_ptr<FileOwner>> owners;
    std::vector<ProjectOperationParticipant> participants;
    for (std::size_t indexValue = 0; indexValue < paths.size(); ++indexValue) {
        auto owner = std::make_shared<FileOwner>();
        owner->path = paths[indexValue];
        owners.push_back(owner);
        participants.push_back({
            "owner." + std::to_string(indexValue), owner->revision,
            [owner] { return owner->revision; },
            [owner](std::string&) { owner->before = owner->value; return true; },
            [owner](std::string&) {
                owner->value = "asset.hero.new";
                ++owner->revision;
                std::filesystem::create_directories(owner->path.parent_path());
                std::ofstream(owner->path, std::ios::binary | std::ios::trunc)
                    << nlohmann::json{{"asset_id", owner->value}}.dump(2);
                return std::filesystem::is_regular_file(owner->path);
            },
            [owner] {
                owner->value = owner->before;
                --owner->revision;
                std::ofstream(owner->path, std::ios::binary | std::ios::trunc)
                    << nlohmann::json{{"asset_id", owner->value}}.dump(2);
            },
            [owner](std::string&) {
                owner->value = owner->before;
                ++owner->revision;
                std::ofstream(owner->path, std::ios::binary | std::ios::trunc)
                    << nlohmann::json{{"asset_id", owner->value}}.dump(2);
                return true;
            },
            [owner] { return nlohmann::json{{"asset_id", owner->value}}.dump(); },
            owner->path,
        });
    }

    REQUIRE(service.execute(preview, participants).success);
    REQUIRE(service.undoLabel() == "Replace Asset");
    for (const auto& path : paths) {
        std::ifstream input(path, std::ios::binary);
        REQUIRE(nlohmann::json::parse(input)["asset_id"] == "asset.hero.new");
    }
    REQUIRE(service.undoLast().success);
    for (const auto& path : paths) {
        std::ifstream input(path, std::ios::binary);
        REQUIRE(nlohmann::json::parse(input)["asset_id"] == "asset.hero.old");
    }
    REQUIRE(service.redoLast().success);
    for (const auto& path : paths) {
        std::ifstream input(path, std::ios::binary);
        REQUIRE(nlohmann::json::parse(input)["asset_id"] == "asset.hero.new");
    }
    REQUIRE(ProjectOperationJournal(root / ".urpg/project_operations/journal.json")
                .recover().last_acknowledged.owners.size() == 3);
    std::filesystem::remove_all(root, error);
}
