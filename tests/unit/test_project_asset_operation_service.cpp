#include "engine/core/assets/project_asset_operation_service.h"

#include <catch2/catch_test_macros.hpp>

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
