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

TEST_CASE("AssetLibrary ingests local promotion catalog records", "[assets][asset_library][asset_intake]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"export_eligible", false},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/UI/click.ogg"},
                 {"normalized_path", "asset://src-007/audio/ui/click-abc123.ogg"},
                 {"preview_path", "imports/raw/urpg_stuff/audio/UI/click.ogg"},
                 {"preview_kind", "audio"},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"pack", "UI Soundpack"},
                 {"size_bytes", 1234},
                 {"sha256", "abc123"},
                 {"tags", {"kind:audio", "category:audio-ui", "ui"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/UI/click-copy.ogg"},
                 {"normalized_path", "asset://src-007/audio/ui/click-copy-abc123.ogg"},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"pack", "UI Soundpack"},
                 {"size_bytes", 1234},
                 {"sha256", "abc123"},
                 {"duplicate_of", "src007:abc123"},
                 {"status", "duplicate"},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }},
    });

    const auto asset = library.findAsset("imports/raw/urpg_stuff/audio/UI/click.ogg");
    REQUIRE(asset.has_value());
    REQUIRE(asset->normalized_path == "asset://src-007/audio/ui/click-abc123.ogg");
    REQUIRE(asset->preview_kind == "audio");
    REQUIRE(asset->media_kind == "audio");
    REQUIRE(asset->category == "audio/ui");
    REQUIRE(asset->tags.size() == 3);

    const auto duplicate = library.findAsset("imports/raw/urpg_stuff/audio/UI/click-copy.ogg");
    REQUIRE(duplicate.has_value());
    REQUIRE(duplicate->statuses.contains(urpg::assets::AssetStatus::Duplicate));
}

TEST_CASE("AssetLibrary ingests promotion catalog summary", "[assets][asset_library][asset_intake]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"promotion_status", "cataloged_local"},
        {"export_eligible", false},
        {"summary",
         {
             {"asset_count", 55192},
             {"canonical_asset_count", 53126},
             {"duplicate_group_count", 2066},
             {"duplicate_asset_count", 2066},
             {"unsupported_count", 0},
             {"category_counts", {{"audio/ui", 240}, {"characters", 1438}}},
             {"kind_counts", {{"audio", 1303}, {"image", 53881}}},
         }},
        {"shards",
         {
             {{"category", "audio/ui"}, {"path", "imports/reports/asset_intake/urpg_stuff_promotion_catalog/audio-ui.json"}},
             {{"category", "characters"}, {"path", "imports/reports/asset_intake/urpg_stuff_promotion_catalog/characters.json"}},
         }},
    });

    const auto& snapshot = library.snapshot();
    REQUIRE(snapshot.catalog_asset_count == 55192);
    REQUIRE(snapshot.canonical_asset_count == 53126);
    REQUIRE(snapshot.duplicate_group_count == 2066);
    REQUIRE(snapshot.duplicate_asset_count == 2066);
    REQUIRE(snapshot.unsupported_count == 0);
    REQUIRE(snapshot.catalog_shard_count == 2);
    REQUIRE_FALSE(snapshot.export_eligible);
    REQUIRE(snapshot.promotion_status == "cataloged_local");
    REQUIRE(snapshot.category_counts.at("audio/ui") == 240);
    REQUIRE(snapshot.kind_counts.at("image") == 53881);
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

TEST_CASE("AssetLibrary filters by tags status references and runtime readiness", "[assets][asset_library][browser]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"export_eligible", false},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"normalized_path", "asset://src-007/characters/hero.png"},
                 {"preview_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"preview_kind", "image"},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"pack", "Hero Pack"},
                 {"tags", {"kind:image", "character", "hero"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                 {"normalized_path", "asset://src-007/audio/click.ogg"},
                 {"preview_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                 {"preview_kind", "audio"},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"pack", "UI Pack"},
                 {"tags", {"kind:audio", "ui"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/click-copy.ogg"},
                 {"normalized_path", "asset://src-007/audio/click-copy.ogg"},
                 {"preview_path", "imports/raw/urpg_stuff/audio/click-copy.ogg"},
                 {"preview_kind", "audio"},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"pack", "UI Pack"},
                 {"tags", {"kind:audio", "ui"}},
                 {"duplicate_of", "click"},
                 {"status", "duplicate"},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }}});

    library.addUsageReference("imports/raw/urpg_stuff/characters/hero.png", "project.map_001");
    library.addUsageReference("imports/raw/urpg_stuff/characters/hero.png", "project.actor_hero");

    urpg::assets::AssetLibraryFilter image_filter;
    image_filter.media_kind = "image";
    image_filter.required_tag = "hero";
    image_filter.referenced_only = true;
    image_filter.runtime_ready_only = true;
    image_filter.previewable_only = true;
    const auto images = library.filterAssets(image_filter);

    REQUIRE(images.size() == 1);
    REQUIRE(images.front().used_by.size() == 2);
    REQUIRE(library.snapshot().referenced_asset_count == 1);
    REQUIRE(library.snapshot().runtime_ready_count == 2);
    REQUIRE(library.snapshot().previewable_count == 3);

    urpg::assets::AssetLibraryFilter duplicate_filter;
    duplicate_filter.required_status = urpg::assets::AssetStatus::Duplicate;
    REQUIRE(library.filterAssets(duplicate_filter).size() == 1);
}

TEST_CASE("AssetLibrary promotes and archives curated assets", "[assets][asset_library][browser][actions]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"normalized_path", "asset://src-007/characters/hero.png"},
                 {"preview_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"preview_kind", "image"},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"tags", {"hero", "kind:image"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/hero-copy.png"},
                 {"normalized_path", "asset://src-007/characters/hero-copy.png"},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"duplicate_of", "hero"},
                 {"status", "duplicate"},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }}});

    const auto promoted = library.promoteAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(promoted.success);
    REQUIRE(promoted.code == "asset_promoted");
    auto hero = library.findAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(hero.has_value());
    REQUIRE(hero->statuses.contains(urpg::assets::AssetStatus::Promoted));
    REQUIRE(hero->provenance.export_eligible);
    REQUIRE(library.snapshot().promoted_count == 1);

    const auto duplicate = library.promoteAsset("imports/raw/urpg_stuff/characters/hero-copy.png");
    REQUIRE_FALSE(duplicate.success);
    REQUIRE(duplicate.code == "asset_duplicate");

    const auto archived = library.archiveAsset("imports/raw/urpg_stuff/characters/hero.png", "not needed");
    REQUIRE(archived.success);
    hero = library.findAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(hero.has_value());
    REQUIRE(hero->statuses.contains(urpg::assets::AssetStatus::Archived));
    REQUIRE_FALSE(hero->statuses.contains(urpg::assets::AssetStatus::Promoted));
    REQUIRE_FALSE(hero->provenance.export_eligible);
    REQUIRE(library.snapshot().promoted_count == 0);
    REQUIRE(library.snapshot().archived_count == 1);
    REQUIRE(library.snapshot().runtime_ready_count == 0);
}
