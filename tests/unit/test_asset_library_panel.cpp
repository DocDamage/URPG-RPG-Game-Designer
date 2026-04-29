#include "editor/assets/asset_library_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

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

TEST_CASE("AssetLibraryPanel empty snapshot explains missing reports", "[assets][asset_library][editor]") {
    urpg::editor::AssetLibraryPanel panel;
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().asset_count == 0);
    REQUIRE_FALSE(panel.lastRenderSnapshot().reports_loaded);
    REQUIRE(panel.lastRenderSnapshot().status == "empty");
    REQUIRE(panel.lastRenderSnapshot().status_message == "No asset library reports are loaded.");
    REQUIRE_FALSE(panel.lastRenderSnapshot().remediation.empty());
}

TEST_CASE("AssetLibraryPanel load error snapshot includes remediation", "[assets][asset_library][editor][error]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_asset_library_missing_reports";
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
    const auto root = std::filesystem::temp_directory_path() / "urpg_asset_library_model_reports";
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
    const auto root = std::filesystem::temp_directory_path() / "urpg_asset_library_model_promotion_reports";
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
              "media_kind": "image",
              "category": "characters",
              "pack": "side scroller stuff",
              "tags": ["kind:image", "category:characters"],
              "license": "user_attested_free_for_game_use_pending_per_pack_attribution"
            }
          ]
        })";
    }

    urpg::editor::AssetLibraryModel model;
    std::string error;
    REQUIRE(model.loadReportsFromDirectory(root, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().asset_count == 1);
    REQUIRE(model.snapshot().catalog_asset_count == 1);
    REQUIRE(model.snapshot().canonical_asset_count == 1);
    REQUIRE(model.snapshot().catalog_shard_count == 1);
    REQUIRE(model.snapshot().promotion_status == "cataloged_local");
    REQUIRE_FALSE(model.snapshot().export_eligible);
    REQUIRE(model.snapshot().category_counts.at("characters") == 1);
    REQUIRE(model.snapshot().kind_counts.at("image") == 1);
    REQUIRE(model.snapshot().runtime_ready_count == 1);
    REQUIRE(model.snapshot().previewable_count == 1);
    REQUIRE(model.snapshot().filtered_asset_count == 1);
    const auto asset = model.library().findAsset("imports/raw/urpg_stuff/side scroller stuff/Hero/Idle.png");
    REQUIRE(asset.has_value());
    REQUIRE(asset->preview_kind == "image");
    REQUIRE(asset->normalized_path == "asset://src-007/characters/idle-123.png");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel exposes filters and used-by reference counts", "[assets][asset_library][editor][browser]") {
    urpg::editor::AssetLibraryModel model;
    model.ingestReports(nlohmann::json{{"file_count", 2}, {"duplicate_groups", 0}, {"oversize_count", 0}},
                        nlohmann::json{{"sources", nlohmann::json::array()}},
                        nlohmann::json{
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

TEST_CASE("AssetLibraryPanel exposes promote and archive action state", "[assets][asset_library][editor][browser][actions]") {
    urpg::editor::AssetLibraryPanel panel;
    panel.model().ingestReports(nlohmann::json{{"file_count", 1}, {"duplicate_groups", 0}, {"oversize_count", 0}},
                                nlohmann::json{{"sources", nlohmann::json::array()}},
                                nlohmann::json{
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
}
