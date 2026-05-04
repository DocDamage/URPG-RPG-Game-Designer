#include "editor/assets/asset_library_panel.h"
#include "tests/unit/asset_library_test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("AssetLibraryPanel renders cleanup preview summary", "[assets][asset_library][editor]") {
    urpg::editor::AssetLibraryPanel panel;
    panel.model().ingestReports(nlohmann::json{{"file_count", 2}, {"duplicate_groups", 1}, {"oversize_count", 0}},
                                nlohmann::json{{"sources", nlohmann::json::array()}},
                                "sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n"
                                "aaa,10,content/hero.png,imports/hero.png,yes\n"
                                "aaa,10,imports/hero.png,imports/hero.png,no\n");
    panel.model().addReferencedAsset("content/hero.png");

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().asset_count == 2);
    REQUIRE(panel.lastRenderSnapshot().duplicate_group_count == 1);
    REQUIRE(panel.lastRenderSnapshot().cleanup_allowed_count == 0);
    REQUIRE(panel.lastRenderSnapshot().cleanup_refused_count == 1);
    REQUIRE(panel.lastRenderSnapshot().reports_loaded);
    REQUIRE(panel.lastRenderSnapshot().status_message.empty());
}

TEST_CASE("AssetLibraryPanel load error snapshot includes remediation", "[assets][asset_library][editor][error]") {
    const auto root = uniqueTempRoot("urpg_asset_library_missing_reports");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    urpg::editor::AssetLibraryPanel panel;
    std::string error;
    REQUIRE_FALSE(panel.model().loadReportsFromDirectory(root, &error));
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.status == "empty");
    REQUIRE(snapshot.status_message == "Asset library reports are missing.");
    REQUIRE_FALSE(snapshot.error_message.empty());
    REQUIRE(snapshot.error_message.find(root.string()) != std::string::npos);
    REQUIRE(snapshot.remediation.find("asset_hygiene.py") != std::string::npos);

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel loads canonical report directory shape", "[assets][asset_library][editor]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_reports");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "asset_intake");

    {
        std::ofstream out(root / "asset_hygiene_summary.json");
        out << R"({"file_count":2,"duplicate_groups":1,"oversize_count":0})";
    }
    {
        std::ofstream out(root / "asset_hygiene_duplicates.csv");
        out << "sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n";
        out << "aaa,10,content/hero.png,imports/hero.png,yes\n";
        out << "aaa,10,imports/hero.png,imports/hero.png,no\n";
    }
    {
        std::ofstream out(root / "asset_intake" / "source_capture_status.json");
        out << R"({"sources":[]})";
    }

    urpg::editor::AssetLibraryModel model;
    std::string error;
    REQUIRE(model.loadReportsFromDirectory(root, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().asset_count == 2);
    REQUIRE(model.snapshot().duplicate_group_count == 1);
}

TEST_CASE("AssetLibraryModel loads optional local promotion catalog", "[assets][asset_library][editor][asset_intake]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_promotion_reports");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "asset_intake");

    {
        std::ofstream out(root / "asset_hygiene_summary.json");
        out << R"({"file_count":1,"duplicate_groups":0,"oversize_count":0})";
    }
    {
        std::ofstream out(root / "asset_hygiene_duplicates.csv");
        out << "sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n";
    }
    {
        std::ofstream out(root / "asset_intake" / "source_capture_status.json");
        out << R"({"sources":[]})";
    }
    {
        std::ofstream out(root / "asset_intake" / "urpg_stuff_promotion_catalog.json");
        out << R"({
          "source_id": "SRC-007",
          "source_root": "imports/raw/urpg_stuff",
          "promotion_status": "cataloged_local",
          "export_eligible": false,
          "summary": {
            "asset_count": 1,
            "canonical_asset_count": 1,
            "duplicate_group_count": 0,
            "duplicate_asset_count": 0,
            "unsupported_count": 0,
            "category_counts": {"characters": 1},
            "kind_counts": {"image": 1}
          },
          "shards": [
            {
              "category": "characters",
              "path": "asset_intake/urpg_stuff_promotion_catalog/characters.json",
              "asset_count": 1
            }
          ]
        })";
    }
    {
        std::ofstream out(root / "asset_intake" / "assets_to_ingest_20260429_promotion_catalog.json");
        out << R"({
          "source_id": "SRC-008",
          "source_root": "imports/raw/urpg_stuff/assets_to_ingest_20260429",
          "promotion_status": "cataloged_local_aggregate",
          "export_eligible": false,
          "summary": {
            "asset_record_count": 1,
            "category_counts": {"characters/isometric": 1},
            "kind_counts": {"image_sequence_collection": 1}
          },
          "shards": [
            {
              "category": "characters/isometric",
              "path": "asset_intake/assets_to_ingest_20260429_promotion_catalog/characters-isometric.json",
              "asset_count": 1
            }
          ]
        })";
    }
    std::filesystem::create_directories(root / "asset_intake" / "urpg_stuff_promotion_catalog");
    {
        std::ofstream out(root / "asset_intake" / "urpg_stuff_promotion_catalog" / "characters.json");
        out << R"({
          "source_id": "SRC-007",
          "source_root": "imports/raw/urpg_stuff",
          "category": "characters",
          "assets": [
            {
              "source_path": "imports/raw/urpg_stuff/side scroller stuff/Hero/Idle.png",
              "normalized_path": "asset://src-007/characters/idle-123.png",
              "preview_path": "imports/raw/urpg_stuff/side scroller stuff/Hero/Idle.png",
              "preview_kind": "image",
              "preview_width": 96,
              "preview_height": 64,
              "media_kind": "image",
              "category": "characters",
              "pack": "side scroller stuff",
              "tags": ["kind:image", "category:characters"],
              "license": "user_attested_free_for_game_use_pending_per_pack_attribution"
            }
          ]
        })";
    }
    std::filesystem::create_directories(root / "asset_intake" / "assets_to_ingest_20260429_promotion_catalog");
    {
        std::ofstream out(root / "asset_intake" / "assets_to_ingest_20260429_promotion_catalog" /
                          "characters-isometric.json");
        out << R"({
          "source_id": "SRC-008",
          "source_root": "imports/raw/urpg_stuff/assets_to_ingest_20260429",
          "category": "characters/isometric",
          "assets": [
            {
              "source_path": "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon",
              "normalized_path": "asset://src-008/characters/isometric/idle-sequence",
              "preview_path": "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon/Idle/0001.png",
              "preview_kind": "image",
              "media_kind": "image_sequence_collection",
              "category": "characters/isometric",
              "pack": "Animated Demon",
              "tags": ["kind:image_sequence", "category:characters-isometric"],
              "license": "user_attested_free_for_game_use_pending_per_pack_attribution"
            }
          ]
        })";
    }

    urpg::editor::AssetLibraryModel model;
    std::string error;
    REQUIRE(model.loadReportsFromDirectory(root, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().asset_count == 2);
    REQUIRE(model.snapshot().catalog_asset_count == 2);
    REQUIRE(model.snapshot().canonical_asset_count == 2);
    REQUIRE(model.snapshot().catalog_shard_count == 2);
    REQUIRE(model.snapshot().promotion_status == "cataloged_local_aggregate");
    REQUIRE_FALSE(model.snapshot().export_eligible);
    REQUIRE(model.snapshot().category_counts.at("characters") == 1);
    REQUIRE(model.snapshot().category_counts.at("characters/isometric") == 1);
    REQUIRE(model.snapshot().kind_counts.at("image") == 1);
    REQUIRE(model.snapshot().kind_counts.at("image_sequence_collection") == 1);
    REQUIRE(model.snapshot().runtime_ready_count == 2);
    REQUIRE(model.snapshot().previewable_count == 2);
    REQUIRE(model.snapshot().sequence_asset_count == 1);
    REQUIRE(model.snapshot().sequence_frame_count == 0);
    REQUIRE(model.snapshot().sequence_clip_count == 0);
    REQUIRE(model.snapshot().filtered_asset_count == 2);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["sequence_packs"]["enabled"] == true);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["sequence_packs"]["count"] == 1);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["sequence_packs"]["media_kind"] ==
            "image_sequence_collection");
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["attachable"]["enabled"] == false);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["project_attached"]["enabled"] == false);
    REQUIRE(model.snapshot().asset_preview_rows.size() == 2);
    const auto sized_preview = std::find_if(
        model.snapshot().asset_preview_rows.begin(), model.snapshot().asset_preview_rows.end(), [](const auto& row) {
            return row["thumbnail"]["ready"] == true && row["thumbnail"]["width"] == 96 &&
                   row["thumbnail"]["height"] == 64;
        });
    REQUIRE(sized_preview != model.snapshot().asset_preview_rows.end());
    const auto asset = model.library().findAsset("imports/raw/urpg_stuff/side scroller stuff/Hero/Idle.png");
    REQUIRE(asset.has_value());
    REQUIRE(asset->preview_kind == "image");
    REQUIRE(asset->normalized_path == "asset://src-007/characters/idle-123.png");
    const auto sequence = model.library().findAsset("imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon");
    REQUIRE(sequence.has_value());
    REQUIRE(sequence->media_kind == "image_sequence_collection");
    REQUIRE(sequence->normalized_path == "asset://src-008/characters/isometric/idle-sequence");

    REQUIRE(model.applyQuickFilter("sequence_packs"));
    REQUIRE(model.snapshot().last_action["action"] == "filter_assets");
    REQUIRE(model.snapshot().last_action["success"] == true);
    REQUIRE(model.snapshot().last_action["code"] == "quick_filter_applied");
    REQUIRE(model.snapshot().last_action["filter_id"] == "sequence_packs");
    REQUIRE(model.snapshot().action_history.size() == 1);
    REQUIRE(model.snapshot().action_history.back()["filter_id"] == "sequence_packs");
    REQUIRE(model.snapshot().filtered_asset_count == 1);
    REQUIRE(model.snapshot().filter_controls["active_filter"]["media_kind"] == "image_sequence_collection");
    REQUIRE(model.snapshot().filter_controls["active_filter"]["runtime_ready_only"] == true);
    REQUIRE(model.snapshot().filter_controls["active_filter"]["previewable_only"] == true);
    REQUIRE(model.snapshot().asset_action_rows.size() == 1);
    REQUIRE(model.snapshot().asset_action_rows[0]["media_kind"] == "image_sequence_collection");
    REQUIRE(model.snapshot().asset_preview_rows.size() == 1);
    REQUIRE(model.snapshot().asset_preview_rows[0]["media_kind"] == "image_sequence_collection");

    REQUIRE(model.applyQuickFilter("all_assets"));
    REQUIRE(model.snapshot().last_action["filter_id"] == "all_assets");
    REQUIRE(model.snapshot().action_history.size() == 2);
    REQUIRE(model.snapshot().action_history.back()["filter_id"] == "all_assets");
    REQUIRE(model.snapshot().filtered_asset_count == 2);
    REQUIRE(model.snapshot().filter_controls["active_filter"]["media_kind"] == "");
    REQUIRE_FALSE(model.applyQuickFilter("missing_filter"));
    REQUIRE(model.snapshot().last_action["success"] == false);
    REQUIRE(model.snapshot().last_action["code"] == "unknown_quick_filter");
    REQUIRE(model.snapshot().last_action["filter_id"] == "missing_filter");
    REQUIRE(model.snapshot().action_history.size() == 3);
    REQUIRE(model.snapshot().action_history.back()["filter_id"] == "missing_filter");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel exposes filters and used-by reference counts",
          "[assets][asset_library][editor][browser]") {
    urpg::editor::AssetLibraryModel model;
    model.ingestReports(
        nlohmann::json{{"file_count", 2}, {"duplicate_groups", 0}, {"oversize_count", 0}},
        nlohmann::json{{"sources", nlohmann::json::array()}},
        nlohmann::json{{"source_id", "SRC-007"},
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
                                {"source_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                                {"normalized_path", "asset://src-007/audio/click.ogg"},
                                {"preview_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                                {"preview_kind", "audio"},
                                {"media_kind", "audio"},
                                {"category", "audio/ui"},
                                {"tags", {"ui", "kind:audio"}},
                                {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
                            },
                        }}},
        "");
    model.addUsageReference("imports/raw/urpg_stuff/characters/hero.png", "actor.hero");

    urpg::assets::AssetLibraryFilter filter;
    filter.media_kind = "image";
    filter.required_tag = "hero";
    filter.referenced_only = true;
    filter.runtime_ready_only = true;
    model.setFilter(filter);

    REQUIRE(model.snapshot().referenced_asset_count == 1);
    REQUIRE(model.snapshot().runtime_ready_count == 2);
    REQUIRE(model.snapshot().previewable_count == 2);
    REQUIRE(model.snapshot().filtered_asset_count == 1);
}

TEST_CASE("AssetLibraryModel builds virtual game-use catalog from governed bundle manifests",
          "[assets][asset_library][editor][browser][virtual_catalog]") {
    const auto root = uniqueTempRoot("urpg_asset_library_virtual_catalog");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    {
        std::ofstream out(root / "BND-101.json");
        out << R"({
          "bundle_id": "BND-101",
          "bundle_name": "test_release_bundle",
          "source_id": "SRC-010",
          "bundle_state": "promoted",
          "release_required": false,
          "release_surfaces": ["map", "ui"],
          "assets": [
            {
              "original_relative_path": "imports/raw/source/fantasy-hero.png",
              "promoted_relative_path": "test_bundle/characters/fantasy-hero.png",
              "category": "prototype_sprite",
              "status": "promoted",
              "release_required": false,
              "release_surfaces": ["map"],
              "license_cleared": true,
              "release_eligible": true,
              "distribution": "deferred",
              "checksum_sha256": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
              "attribution_record": "imports/reports/asset_intake/attribution/BND-101.json",
              "package_destination": "share/urpg/imports/normalized/test_bundle/characters/fantasy-hero.png"
            },
            {
              "original_relative_path": "imports/raw/source/isometric-stone-tile.png",
              "promoted_relative_path": "test_bundle/tilesets/isometric-stone-tile.png",
              "category": "tileset",
              "status": "promoted",
              "release_required": false,
              "release_surfaces": ["map"],
              "license_cleared": true,
              "release_eligible": true,
              "distribution": "bundled",
              "checksum_sha256": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
              "attribution_record": "imports/reports/asset_intake/attribution/BND-101.json",
              "package_destination": "share/urpg/imports/normalized/test_bundle/tilesets/isometric-stone-tile.png"
            },
            {
              "original_relative_path": "imports/raw/source/menu-panel.png",
              "promoted_relative_path": "test_bundle/ui/menu-panel.png",
              "category": "ui_frames_chrome",
              "status": "promoted",
              "release_required": false,
              "release_surfaces": ["ui"],
              "license_cleared": true,
              "release_eligible": true,
              "distribution": "bundled",
              "checksum_sha256": "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
              "attribution_record": "imports/reports/asset_intake/attribution/BND-101.json",
              "package_destination": "share/urpg/imports/normalized/test_bundle/ui/menu-panel.png"
            }
          ]
        })";
    }

    urpg::editor::AssetLibraryModel model;
    std::string error;
    REQUIRE(model.loadAssetBundleManifestsFromDirectory(root, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().asset_count == 3);
    REQUIRE(model.snapshot().source_bundle_counts.at("BND-101") == 3);
    REQUIRE(model.snapshot().game_use_category_counts.at("characters/sprites") == 1);
    REQUIRE(model.snapshot().game_use_category_counts.at("environment/tiles/isometric") == 1);
    REQUIRE(model.snapshot().game_use_category_counts.at("ui/widgets") == 1);
    REQUIRE(model.snapshot().game_use_tag_counts.at("release:eligible") == 3);
    REQUIRE(model.snapshot().game_use_tag_counts.at("asset_type:tileset") == 1);
    REQUIRE(model.snapshot().virtual_catalog["preserves_physical_layout"] == true);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["release_eligible"]["count"] == 3);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["tilesets"]["count"] == 1);
    REQUIRE(model.snapshot().asset_action_rows[0].contains("game_use_category"));
    REQUIRE(model.snapshot().asset_action_rows[0].contains("source_bundle_id"));

    REQUIRE(model.applyQuickFilter("tilesets"));
    REQUIRE(model.snapshot().filtered_asset_count == 1);
    REQUIRE(model.snapshot().asset_action_rows[0]["game_use_category"] == "environment/tiles/isometric");
    REQUIRE(model.snapshot().filter_controls["active_filter"]["required_game_use_tag"] == "asset_type:tileset");

    REQUIRE(model.applyQuickFilter("release_eligible"));
    REQUIRE(model.snapshot().filtered_asset_count == 3);
    REQUIRE(model.snapshot().filter_controls["active_filter"]["release_eligible_only"] == true);

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryPanel exposes promote and archive action state",
          "[assets][asset_library][editor][browser][actions]") {
    urpg::editor::AssetLibraryPanel panel;
    panel.model().ingestReports(
        nlohmann::json{{"file_count", 1}, {"duplicate_groups", 0}, {"oversize_count", 0}},
        nlohmann::json{{"sources", nlohmann::json::array()}},
        nlohmann::json{{"source_id", "SRC-007"},
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
                        }}},
        "");

    const auto promoted = panel.model().promoteAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(promoted.success);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().promoted_count == 1);
    REQUIRE(panel.lastRenderSnapshot().archived_count == 0);
    REQUIRE(panel.lastRenderSnapshot().last_action["action"] == "promote");
    REQUIRE(panel.lastRenderSnapshot().last_action["success"] == true);
    REQUIRE(panel.lastRenderSnapshot().action_history.size() == 1);

    const auto archived = panel.model().archiveAsset("imports/raw/urpg_stuff/characters/hero.png", "archive test");
    REQUIRE(archived.success);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().promoted_count == 0);
    REQUIRE(panel.lastRenderSnapshot().archived_count == 1);
    REQUIRE(panel.lastRenderSnapshot().runtime_ready_count == 0);
    REQUIRE(panel.lastRenderSnapshot().last_action["action"] == "archive");
    REQUIRE(panel.lastRenderSnapshot().action_history.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["recommended_action"] == "archived");
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["promote_button"]["enabled"] == false);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["promote_button"]["disabled_reason"] == "asset_archived");
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["archive_button"]["enabled"] == false);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["archive_button"]["disabled_reason"] ==
            "asset_already_archived");
}

TEST_CASE("AssetLibraryPanel exposes governed promotion manifest action rows",
          "[assets][asset_library][editor][promotion]") {
    urpg::editor::AssetLibraryPanel panel;

    panel.model().ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.walk"},
        {"sourcePath", "imports/raw/example/hero.png"},
        {"promotedPath", "resources/assets/characters/hero.png"},
        {"licenseId", "BND-001"},
        {"status", "runtime_ready"},
        {"preview",
         {{"kind", "image"}, {"thumbnailPath", "resources/previews/hero.thumb.png"}, {"width", 48}, {"height", 48}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    }));
    panel.model().ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.unlicensed"},
        {"sourcePath", "imports/raw/example/unlicensed.png"},
        {"promotedPath", "resources/assets/characters/unlicensed.png"},
        {"status", "runtime_ready"},
        {"preview",
         {{"kind", "image"},
          {"thumbnailPath", "resources/previews/unlicensed.thumb.png"},
          {"width", 48},
          {"height", 48}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    }));
    panel.model().ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.missing"},
        {"sourcePath", "imports/raw/example/missing.png"},
        {"licenseId", "BND-001"},
        {"status", "runtime_ready"},
        {"preview", {{"kind", "pending"}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    }));
    panel.model().ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.copy"},
        {"sourcePath", "imports/raw/example/hero-copy.png"},
        {"promotedPath", "resources/assets/characters/hero-copy.png"},
        {"licenseId", "BND-001"},
        {"status", "archived"},
        {"preview", {{"kind", "none"}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", true}}},
        {"diagnostics", nlohmann::json::array()},
    }));

    panel.render();

    REQUIRE(panel.lastRenderSnapshot().asset_action_rows.size() == 4);
    const auto rows = panel.lastRenderSnapshot().asset_action_rows;
    const auto promoted = std::find_if(rows.begin(), rows.end(),
                                       [](const auto& row) { return row["path"] == "imports/raw/example/hero.png"; });
    REQUIRE(promoted != rows.end());
    REQUIRE((*promoted)["recommended_action"] == "attach_to_project");
    REQUIRE((*promoted)["attach_button"]["enabled"] == true);
    REQUIRE((*promoted)["promotion_status"] == "runtime_ready");
    REQUIRE((*promoted)["include_in_runtime"] == true);

    const auto unlicensed = std::find_if(
        rows.begin(), rows.end(), [](const auto& row) { return row["path"] == "imports/raw/example/unlicensed.png"; });
    REQUIRE(unlicensed != rows.end());
    REQUIRE((*unlicensed)["recommended_action"] == "add_license_evidence");
    REQUIRE((*unlicensed)["include_in_runtime"] == false);

    const auto missing = std::find_if(rows.begin(), rows.end(),
                                      [](const auto& row) { return row["path"] == "imports/raw/example/missing.png"; });
    REQUIRE(missing != rows.end());
    REQUIRE((*missing)["recommended_action"] == "fix_missing_file");

    const auto archived = std::find_if(
        rows.begin(), rows.end(), [](const auto& row) { return row["path"] == "imports/raw/example/hero-copy.png"; });
    REQUIRE(archived != rows.end());
    REQUIRE((*archived)["recommended_action"] == "archived");
    REQUIRE((*archived)["include_in_runtime"] == false);
    REQUIRE((*archived)["required_for_release"] == false);
}
