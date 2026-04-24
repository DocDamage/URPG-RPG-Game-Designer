#include "engine/core/assets/asset_library.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("AssetLibrary ingests duplicate groups deterministically", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;

    library.ingestDuplicateCsv(
        "sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n"
        "bbb,20,z/path.png,z/path.png,no\n"
        "aaa,10,b/path.png,a/path.png,yes\n"
        "aaa,10,a/path.png,a/path.png,no\n");

    const auto& snapshot = library.snapshot();
    REQUIRE(snapshot.duplicate_groups.size() == 2);
    REQUIRE(snapshot.duplicate_groups[0].sha256 == "aaa");
    REQUIRE(snapshot.duplicate_groups[0].entries[0].path == "a/path.png");
    REQUIRE(snapshot.duplicate_groups[0].entries[1].path == "b/path.png");
    REQUIRE(snapshot.assets[0].path == "a/path.png");
    REQUIRE(snapshot.assets[1].path == "b/path.png");
}

TEST_CASE("AssetLibrary provenance packet round-trips JSON", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;
    library.ingestIntakeReport(nlohmann::json{
        {"sources", {
            {
                {"source_id", "SRC-001"},
                {"repo_name", "GDQuest/game-sprites"},
                {"legal_disposition", "cc0_candidate_recorded_for_private_use_intake"},
                {"promotion_status", "promoted"},
                {"normalized_assets", {"imports/normalized/prototype_sprites/gdquest_blue_actor.svg"}}
            }
        }}
    });

    const auto asset = library.findAsset("imports/normalized/prototype_sprites/gdquest_blue_actor.svg");
    REQUIRE(asset.has_value());

    const auto packet = nlohmann::json::parse(urpg::assets::exportProvenancePacket(*asset));
    const auto provenance = urpg::assets::assetProvenanceFromJson(packet["provenance"]);
    REQUIRE(provenance.original_source == "GDQuest/game-sprites");
    REQUIRE(provenance.export_eligible);
}

TEST_CASE("AssetLibrary detects case collisions and unsupported paths", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;
    library.markUnsupportedFormat("Assets/Hero.PNG");
    library.markMissingFile("assets/hero.png");

    library.detectCaseCollisions();

    const auto upper = library.findAsset("Assets/Hero.PNG");
    const auto lower = library.findAsset("assets/hero.png");
    REQUIRE(upper.has_value());
    REQUIRE(lower.has_value());
    REQUIRE(upper->statuses.contains(urpg::assets::AssetStatus::UnsupportedFormat));
    REQUIRE(upper->statuses.contains(urpg::assets::AssetStatus::CaseCollision));
    REQUIRE(lower->statuses.contains(urpg::assets::AssetStatus::MissingFile));
    REQUIRE(lower->statuses.contains(urpg::assets::AssetStatus::CaseCollision));
    REQUIRE(library.snapshot().case_collision_count == 2);
}
