#include "editor/assets/asset_library_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace {

std::filesystem::path uniqueTempRoot(const std::string& prefix) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (prefix + "_" + std::to_string(tick));
}

void writeBinaryFile(const std::filesystem::path& path, std::string_view payload) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << payload;
}

} // namespace

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

TEST_CASE("AssetLibraryModel exposes global import sessions and review queues",
          "[assets][asset_library][editor][asset_import]") {
    urpg::editor::AssetLibraryModel model;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_20260430_001";
    session.sourceKind = urpg::assets::AssetImportSourceKind::Folder;
    session.sourcePath = "C:/Users/Creator/Downloads/fantasy_pack";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_20260430_001/original";
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.createdAt = "2026-04-30T00:00:00Z";
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_20260430_001/characters/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.theme",
            "audio/theme.ogg",
            "asset://import_20260430_001/audio/theme.ogg",
            ".ogg",
            "audio",
            "audio/bgm",
            "Fantasy Pack",
            "bbb",
            2048,
            0,
            0,
            110000,
            false,
            "",
            false,
            false,
            false,
            false,
            {"conversion_required"},
        },
        {
            "asset.unlicensed",
            "ui/window.png",
            "asset://import_20260430_001/ui/window.png",
            ".png",
            "image",
            "ui",
            "Fantasy Pack",
            "ccc",
            512,
            64,
            32,
            0,
            false,
            "",
            false,
            false,
            true,
            true,
            {},
        },
    };

    model.ingestImportSession(std::move(session));

    REQUIRE(model.snapshot().status == "ready");
    REQUIRE(model.snapshot().reports_loaded);
    REQUIRE(model.snapshot().import_session_count == 1);
    REQUIRE(model.snapshot().import_review_row_count == 3);
    REQUIRE(model.snapshot().import_ready_count == 1);
    REQUIRE(model.snapshot().import_needs_conversion_count == 1);
    REQUIRE(model.snapshot().import_missing_license_count == 1);
    REQUIRE(model.snapshot().import_session_rows.size() == 1);
    REQUIRE(model.snapshot().import_session_rows[0]["status"] == "review_ready");
    REQUIRE(model.snapshot().import_session_rows[0]["summary"]["filesScanned"] == 3);
    REQUIRE(model.snapshot().import_review_rows.size() == 3);
    REQUIRE(model.snapshot().import_wizard["status"] == "review_required");
    REQUIRE(model.snapshot().import_wizard["current_step"] == "review");
    REQUIRE(model.snapshot().import_wizard["actions"]["add_source"]["enabled"] == true);
    REQUIRE(model.snapshot().import_wizard["actions"]["promote_selected"]["enabled"] == true);
    REQUIRE(model.snapshot().import_wizard["actions"]["promote_selected"]["eligible_count"] == 1);
    REQUIRE(model.snapshot().import_wizard["actions"]["attach_selected"]["enabled"] == false);
    REQUIRE(model.snapshot().import_wizard["steps"][0]["id"] == "add_source");
    REQUIRE(model.snapshot().import_wizard["steps"][0]["state"] == "complete");
    REQUIRE(model.snapshot().import_wizard["steps"][1]["id"] == "review");
    REQUIRE(model.snapshot().import_wizard["steps"][1]["state"] == "active");
    const auto ready = std::find_if(
        model.snapshot().import_review_rows.begin(), model.snapshot().import_review_rows.end(), [](const auto& row) {
            return row["relative_path"] == "characters/hero.png";
        });
    REQUIRE(ready != model.snapshot().import_review_rows.end());
    REQUIRE((*ready)["review_state"] == "ready_to_promote");
    REQUIRE((*ready)["promotable"] == true);
}

TEST_CASE("AssetLibraryPanel renders project import wizard steps and actions",
          "[assets][asset_library][editor][asset_import][wizard]") {
    urpg::editor::AssetLibraryPanel panel;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_panel_wizard_001";
    session.sourceKind = urpg::assets::AssetImportSourceKind::Folder;
    session.sourcePath = "C:/Users/Creator/Downloads/fantasy_pack";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_panel_wizard_001/original";
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.createdAt = "2026-05-01T00:00:00Z";
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_panel_wizard_001/characters/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
    };
    panel.model().ingestImportSession(std::move(session));

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    const auto& wizard = panel.lastImportWizardSnapshot();
    REQUIRE(wizard.status == "review_required");
    REQUIRE(wizard.current_step == "review");
    REQUIRE(wizard.steps.size() == 5);
    const auto reviewStep = std::find_if(wizard.steps.begin(), wizard.steps.end(), [](const auto& step) {
        return step.id == "review";
    });
    REQUIRE(reviewStep != wizard.steps.end());
    REQUIRE(reviewStep->active);
    REQUIRE(reviewStep->count == 1);

    const auto promoteAction = std::find_if(wizard.actions.begin(), wizard.actions.end(), [](const auto& action) {
        return action.id == "promote_selected";
    });
    REQUIRE(promoteAction != wizard.actions.end());
    REQUIRE(promoteAction->enabled);
    REQUIRE(promoteAction->eligible_count == 1);
    REQUIRE(promoteAction->disabled_reason.empty());

    const auto packageAction = std::find_if(wizard.actions.begin(), wizard.actions.end(), [](const auto& action) {
        return action.id == "package_validate";
    });
    REQUIRE(packageAction != wizard.actions.end());
    REQUIRE_FALSE(packageAction->enabled);
    REQUIRE(packageAction->disabled_reason == "no_attached_project_assets");
    REQUIRE_FALSE(wizard.package_validation_ready);
}

TEST_CASE("AssetLibraryModel requests add-source import command handoff",
          "[assets][asset_library][editor][asset_import][wizard]") {
    const auto root = uniqueTempRoot("urpg_asset_library_add_source_request");
    const auto source = root / "fantasy_pack";
    const auto libraryRoot = root / ".urpg" / "asset-library";

    urpg::editor::AssetLibraryModel model;
    const auto request = model.requestImportSource(
        source, libraryRoot, "import_manual_001", "User-provided test license.");

    REQUIRE(request["action"] == "request_import_source");
    REQUIRE(request["success"] == true);
    REQUIRE(request["code"] == "import_source_requested");
    REQUIRE(request["source_path"] == source.generic_string());
    REQUIRE(request["library_root"] == libraryRoot.generic_string());
    REQUIRE(request["session_id"] == "import_manual_001");
    REQUIRE(request["expected_manifest_path"] ==
            (libraryRoot / "catalog" / "import_sessions" / "import_manual_001.json").generic_string());
    REQUIRE(request["command"][0] == "python");
    REQUIRE(request["command"][1] == "tools/assets/global_asset_import.py");
    REQUIRE(request["command"][2] == "--source");
    REQUIRE(request["command"][3] == source.generic_string());
    REQUIRE(request["command"][4] == "--library-root");
    REQUIRE(request["command"][5] == libraryRoot.generic_string());

    REQUIRE(model.snapshot().last_action["action"] == "request_import_source");
    REQUIRE(model.snapshot().import_wizard["status"] == "source_requested");
    REQUIRE(model.snapshot().import_wizard["current_step"] == "add_source");
    REQUIRE(model.snapshot().import_wizard["pending_request"]["session_id"] == "import_manual_001");
    REQUIRE(model.snapshot().import_wizard["actions"]["add_source"]["pending_request"] == true);
}

TEST_CASE("AssetLibraryModel loads import session manifests from global library root",
          "[assets][asset_library][editor][asset_import]") {
    const auto root = uniqueTempRoot("urpg_asset_library_import_sessions");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "catalog" / "import_sessions");
    std::filesystem::create_directories(root / "sources" / "import_source_002");

    {
        std::ofstream out(root / "catalog" / "import_sessions" / "import_catalog_001.json");
        out << R"({
          "schemaVersion": "1.0.0",
          "sessionId": "import_catalog_001",
          "sourceKind": "zip",
          "sourcePath": "C:/Users/Creator/Downloads/fantasy_pack.zip",
          "managedSourceRoot": ".urpg/asset-library/sources/import_catalog_001/extracted",
          "status": "review_ready",
          "createdAt": "2026-04-30T00:00:00Z",
          "records": [
            {
              "assetId": "asset.hero",
              "relativePath": "characters/hero.png",
              "normalizedPath": "asset://import_catalog_001/sprite/hero.png",
              "extension": ".png",
              "mediaKind": "image",
              "category": "sprite",
              "pack": "Fantasy Pack",
              "sha256": "aaa",
              "sizeBytes": 1024,
              "width": 48,
              "height": 48,
              "runtimeReady": true,
              "licenseRequired": false,
              "diagnostics": []
            },
            {
              "assetId": "asset.theme",
              "relativePath": "audio/theme.mp3",
              "normalizedPath": "asset://import_catalog_001/audio/bgm/theme.mp3",
              "extension": ".mp3",
              "mediaKind": "audio",
              "category": "audio/bgm",
              "pack": "Fantasy Pack",
              "sha256": "bbb",
              "sizeBytes": 2048,
              "runtimeReady": false,
              "licenseRequired": false,
              "diagnostics": ["conversion_required"]
            }
          ],
          "diagnostics": []
        })";
    }
    {
        std::ofstream out(root / "sources" / "import_source_002" / "source_manifest.json");
        out << R"({
          "schemaVersion": "1.0.0",
          "sessionId": "import_source_002",
          "sourceKind": "folder",
          "sourcePath": "C:/Users/Creator/Downloads/ui_pack",
          "managedSourceRoot": ".urpg/asset-library/sources/import_source_002/original",
          "status": "review_ready",
          "createdAt": "2026-04-30T00:01:00Z",
          "records": [
            {
              "assetId": "asset.window",
              "relativePath": "ui/window.png",
              "normalizedPath": "asset://import_source_002/ui/window.png",
              "extension": ".png",
              "mediaKind": "image",
              "category": "ui",
              "pack": "UI Pack",
              "sha256": "ccc",
              "sizeBytes": 512,
              "width": 64,
              "height": 32,
              "runtimeReady": true,
              "licenseRequired": true,
              "diagnostics": []
            }
          ],
          "diagnostics": []
        })";
    }

    urpg::editor::AssetLibraryModel model;
    std::string error = "not cleared";
    REQUIRE(model.loadImportSessionsFromLibraryRoot(root, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().status == "ready");
    REQUIRE(model.snapshot().reports_loaded);
    REQUIRE(model.snapshot().import_session_count == 2);
    REQUIRE(model.snapshot().import_review_row_count == 3);
    REQUIRE(model.snapshot().import_ready_count == 1);
    REQUIRE(model.snapshot().import_needs_conversion_count == 1);
    REQUIRE(model.snapshot().import_missing_license_count == 1);
    REQUIRE(model.snapshot().import_session_rows.size() == 2);

    const auto missing_license = std::find_if(
        model.snapshot().import_review_rows.begin(), model.snapshot().import_review_rows.end(), [](const auto& row) {
            return row["relative_path"] == "ui/window.png";
        });
    REQUIRE(missing_license != model.snapshot().import_review_rows.end());
    REQUIRE((*missing_license)["review_state"] == "missing_license");
    REQUIRE((*missing_license)["recommended_action"] == "add_license_attribution");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel promotes selected import records through governed manifests",
          "[assets][asset_library][editor][asset_import][promotion]") {
    urpg::editor::AssetLibraryModel model;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_20260430_001";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_20260430_001/extracted";
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_20260430_001/sprite/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.theme",
            "audio/theme.mp3",
            "asset://import_20260430_001/audio/bgm/theme.mp3",
            ".mp3",
            "audio",
            "audio/bgm",
            "Fantasy Pack",
            "bbb",
            2048,
            0,
            0,
            93000,
            false,
            "",
            false,
            false,
            false,
            false,
            {"conversion_required"},
        },
    };
    model.ingestImportSession(std::move(session));

    const auto promoted = model.promoteImportRecord(
        "import_20260430_001", "asset.hero", "user_license_note", ".urpg/asset-library/promoted", true);
    REQUIRE(promoted.success);
    REQUIRE(promoted.code == "import_record_promoted");
    REQUIRE(model.snapshot().promoted_count == 1);
    REQUIRE(model.snapshot().runtime_ready_count == 1);
    REQUIRE(model.snapshot().project_attachable_count == 1);
    REQUIRE(model.snapshot().project_attached_count == 0);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["attachable"]["enabled"] == true);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["attachable"]["count"] == 1);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["project_attached"]["enabled"] == false);
    REQUIRE(model.snapshot().asset_action_rows.size() == 1);
    REQUIRE(model.snapshot().asset_action_rows[0]["recommended_action"] == "attach_to_project");
    REQUIRE(model.snapshot().asset_action_rows[0]["attach_button"]["enabled"] == true);
    REQUIRE(model.snapshot().asset_action_rows[0]["promoted_path"] ==
            ".urpg/asset-library/promoted/asset.hero/payloads/hero.png");
    REQUIRE(model.snapshot().last_action["action"] == "promote_import_record");
    REQUIRE(model.snapshot().last_action["success"] == true);
    REQUIRE(model.applyQuickFilter("attachable"));
    REQUIRE(model.snapshot().filtered_asset_count == 1);
    REQUIRE(model.snapshot().filter_controls["active_filter"]["attachable_only"] == true);
    REQUIRE(model.snapshot().asset_action_rows[0]["asset_id"] == "asset.hero");
    REQUIRE(model.applyQuickFilter("all_assets"));

    const auto blocked = model.promoteImportRecord(
        "import_20260430_001", "asset.theme", "user_license_note", ".urpg/asset-library/promoted", true);
    REQUIRE_FALSE(blocked.success);
    REQUIRE(blocked.code == "import_record_blocked");
    REQUIRE(model.snapshot().asset_action_rows.size() == 2);
    const auto theme = std::find_if(
        model.snapshot().asset_action_rows.begin(), model.snapshot().asset_action_rows.end(), [](const auto& row) {
            return row["asset_id"] == "asset.theme";
    });
    REQUIRE(theme != model.snapshot().asset_action_rows.end());
    REQUIRE((*theme)["recommended_action"] == "convert_or_replace");
    REQUIRE((*theme)["promotion_status"] == "blocked");
    REQUIRE((*theme)["include_in_runtime"] == false);
    REQUIRE((*theme)["promotion_diagnostics"][0] == "conversion_required");
}

TEST_CASE("AssetLibraryModel bulk promotes selected import records with per-record diagnostics",
          "[assets][asset_library][editor][asset_import][promotion]") {
    urpg::editor::AssetLibraryModel model;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_20260430_bulk";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_20260430_bulk/extracted";
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_20260430_bulk/sprite/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.theme",
            "audio/theme.mp3",
            "asset://import_20260430_bulk/audio/bgm/theme.mp3",
            ".mp3",
            "audio",
            "audio/bgm",
            "Fantasy Pack",
            "bbb",
            2048,
            0,
            0,
            93000,
            false,
            "",
            false,
            false,
            false,
            false,
            {"conversion_required"},
        },
    };
    model.ingestImportSession(std::move(session));

    const auto result = model.promoteImportRecords(
        "import_20260430_bulk",
        std::vector<std::string>{"asset.hero", "asset.theme", "asset.missing"},
        "user_license_note",
        ".urpg/asset-library/promoted",
        true);

    REQUIRE(result["action"] == "promote_import_records");
    REQUIRE(result["success"] == false);
    REQUIRE(result["selected_count"] == 3);
    REQUIRE(result["promoted_count"] == 1);
    REQUIRE(result["blocked_count"] == 1);
    REQUIRE(result["missing_count"] == 1);
    REQUIRE(result["rows"].size() == 3);
    REQUIRE(model.snapshot().last_action["action"] == "promote_import_records");
    REQUIRE(model.snapshot().last_action["promoted_count"] == 1);
    REQUIRE(model.snapshot().promoted_count == 1);

    const auto theme = std::find_if(result["rows"].begin(), result["rows"].end(), [](const auto& row) {
        return row["asset_id"] == "asset.theme";
    });
    REQUIRE(theme != result["rows"].end());
    REQUIRE((*theme)["success"] == false);
    REQUIRE((*theme)["code"] == "import_record_blocked");
    REQUIRE((*theme)["diagnostics"][0] == "conversion_required");

    const auto missing = std::find_if(result["rows"].begin(), result["rows"].end(), [](const auto& row) {
        return row["asset_id"] == "asset.missing";
    });
    REQUIRE(missing != result["rows"].end());
    REQUIRE((*missing)["code"] == "import_record_not_found");
}

TEST_CASE("AssetLibraryModel attaches promoted assets to a project",
          "[assets][asset_library][editor][asset_attachment]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_attach");
    std::filesystem::remove_all(root);
    const auto payload = root / ".urpg" / "asset-library" / "promoted" / "asset.hero" / "payloads" / "hero.png";
    const auto projectRoot = root / "project";
    writeBinaryFile(payload, "hero-payload");

    urpg::editor::AssetLibraryModel model;
    model.ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero"},
        {"sourcePath", "imports/raw/example/hero.png"},
        {"promotedPath", payload.string()},
        {"licenseId", "user_license_note"},
        {"status", "runtime_ready"},
        {"preview", {{"kind", "image"}, {"thumbnailPath", payload.string()}, {"width", 48}, {"height", 48}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
        {"diagnostics", nlohmann::json::array()},
    }));

    const auto result = model.attachPromotedAssetToProject("imports/raw/example/hero.png", projectRoot);
    REQUIRE(result.success);
    REQUIRE(result.code == "project_asset_attached");
    REQUIRE(model.snapshot().last_action["action"] == "attach_project_asset");
    REQUIRE(model.snapshot().last_action["success"] == true);
    REQUIRE(model.snapshot().referenced_asset_count == 1);
    REQUIRE(model.snapshot().project_attached_count == 1);
    REQUIRE(model.snapshot().project_attachable_count == 0);
    REQUIRE(model.snapshot().import_wizard["status"] == "package_ready");
    REQUIRE(model.snapshot().import_wizard["current_step"] == "package");
    REQUIRE(model.snapshot().import_wizard["actions"]["package_validate"]["enabled"] == true);
    REQUIRE(model.snapshot().import_wizard["steps"][4]["id"] == "package");
    REQUIRE(model.snapshot().import_wizard["steps"][4]["state"] == "active");
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["project_attached"]["enabled"] == true);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["project_attached"]["count"] == 1);
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["attachable"]["enabled"] == false);
    REQUIRE(model.snapshot().asset_action_rows.size() == 1);
    REQUIRE(model.snapshot().asset_action_rows[0]["archive_button"]["enabled"] == false);
    REQUIRE(model.snapshot().asset_action_rows[0]["archive_button"]["disabled_reason"] == "asset_in_use");
    REQUIRE(model.snapshot().asset_action_rows[0]["attach_button"]["enabled"] == false);
    REQUIRE(model.snapshot().asset_action_rows[0]["attach_button"]["disabled_reason"] == "asset_already_attached");
    REQUIRE(model.snapshot().asset_action_rows[0]["project_attached"] == true);
    REQUIRE(model.snapshot().asset_action_rows[0]["recommended_action"] == "project_attached");
    REQUIRE(model.snapshot().asset_action_rows[0]["used_by"].size() == 1);

    const auto projectPayload = projectRoot / "content" / "assets" / "imported" / "asset.hero" / "hero.png";
    const auto projectManifest = projectRoot / "content" / "assets" / "manifests" / "asset.hero.json";
    REQUIRE(std::filesystem::is_regular_file(projectPayload));
    REQUIRE(std::filesystem::is_regular_file(projectManifest));
    REQUIRE(model.applyQuickFilter("project_attached"));
    REQUIRE(model.snapshot().filtered_asset_count == 1);
    REQUIRE(model.snapshot().filter_controls["active_filter"]["project_attached_only"] == true);
    REQUIRE(model.snapshot().asset_action_rows[0]["project_attached"] == true);
    REQUIRE(model.applyQuickFilter("attachable"));
    REQUIRE(model.snapshot().filtered_asset_count == 0);

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel attaches selected promoted assets to a project",
          "[assets][asset_library][editor][asset_attachment]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_attach_selected");
    std::filesystem::remove_all(root);
    const auto promotedRoot = root / ".urpg" / "asset-library" / "promoted";
    const auto projectRoot = root / "project";
    writeBinaryFile(promotedRoot / "asset.hero" / "payloads" / "hero.png", "hero-payload");
    writeBinaryFile(promotedRoot / "asset.click" / "payloads" / "click.wav", "click-payload");

    urpg::editor::AssetLibraryModel model;
    model.ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero"},
        {"sourcePath", "imports/raw/example/hero.png"},
        {"promotedPath", (promotedRoot / "asset.hero" / "payloads" / "hero.png").string()},
        {"licenseId", "user_license_note"},
        {"status", "runtime_ready"},
        {"preview",
         {{"kind", "image"},
          {"thumbnailPath", (promotedRoot / "asset.hero" / "payloads" / "hero.png").string()},
          {"width", 48},
          {"height", 48}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
        {"diagnostics", nlohmann::json::array()},
    }));
    model.ingestPromotionManifest(urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.click"},
        {"sourcePath", "imports/raw/example/click.wav"},
        {"promotedPath", (promotedRoot / "asset.click" / "payloads" / "click.wav").string()},
        {"licenseId", "user_license_note"},
        {"status", "runtime_ready"},
        {"preview",
         {{"kind", "audio"},
          {"thumbnailPath", (promotedRoot / "asset.click" / "payloads" / "click.wav").string()},
          {"width", 0},
          {"height", 0}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
        {"diagnostics", nlohmann::json::array()},
    }));
    REQUIRE(model.snapshot().project_attachable_count == 2);

    const auto result = model.attachPromotedAssetsToProject(
        std::vector<std::string>{
            "imports/raw/example/hero.png",
            "imports/raw/example/click.wav",
            "imports/raw/example/missing.png",
        },
        projectRoot);

    REQUIRE(result["action"] == "attach_project_assets");
    REQUIRE(result["success"] == false);
    REQUIRE(result["selected_count"] == 3);
    REQUIRE(result["attached_count"] == 2);
    REQUIRE(result["blocked_count"] == 0);
    REQUIRE(result["missing_count"] == 1);
    REQUIRE(result["rows"].size() == 3);
    REQUIRE(model.snapshot().last_action["action"] == "attach_project_assets");
    REQUIRE(model.snapshot().project_attached_count == 2);
    REQUIRE(model.snapshot().project_attachable_count == 0);
    REQUIRE(model.snapshot().project_asset_picker_rows.size() == 2);

    REQUIRE(std::filesystem::is_regular_file(projectRoot / "content" / "assets" / "imported" / "asset.hero" /
                                             "hero.png"));
    REQUIRE(std::filesystem::is_regular_file(projectRoot / "content" / "assets" / "imported" / "asset.click" /
                                             "click.wav"));
    REQUIRE(std::filesystem::is_regular_file(projectRoot / "content" / "assets" / "manifests" / "asset.hero.json"));
    REQUIRE(std::filesystem::is_regular_file(projectRoot / "content" / "assets" / "manifests" / "asset.click.json"));

    const auto heroPicker = std::find_if(
        model.snapshot().project_asset_picker_rows.begin(), model.snapshot().project_asset_picker_rows.end(),
        [](const auto& row) {
            return row["asset_id"] == "asset.hero";
        });
    REQUIRE(heroPicker != model.snapshot().project_asset_picker_rows.end());
    REQUIRE((*heroPicker)["project_path"] ==
            (projectRoot / "content" / "assets" / "imported" / "asset.hero" / "hero.png").generic_string());
    REQUIRE((*heroPicker)["manifest_path"] ==
            (projectRoot / "content" / "assets" / "manifests" / "asset.hero.json").generic_string());
    REQUIRE((*heroPicker)["picker_kind"] == "sprite");
    REQUIRE((*heroPicker)["picker_targets"][0] == "level_builder");

    const auto audioPicker = std::find_if(
        model.snapshot().project_asset_picker_rows.begin(), model.snapshot().project_asset_picker_rows.end(),
        [](const auto& row) {
            return row["asset_id"] == "asset.click";
        });
    REQUIRE(audioPicker != model.snapshot().project_asset_picker_rows.end());
    REQUIRE((*audioPicker)["picker_kind"] == "audio");

    const auto missing = std::find_if(result["rows"].begin(), result["rows"].end(), [](const auto& row) {
        return row["path"] == "imports/raw/example/missing.png";
    });
    REQUIRE(missing != result["rows"].end());
    REQUIRE((*missing)["code"] == "asset_not_found");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel promotes imported payloads globally before project attachment",
          "[assets][asset_library][editor][asset_import][promotion][asset_attachment]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_global_promote_attach");
    std::filesystem::remove_all(root);
    const auto quarantineRoot = root / ".urpg" / "asset-library" / "sources" / "import_001" / "extracted";
    writeBinaryFile(quarantineRoot / "characters" / "hero.png", "hero-payload");

    urpg::editor::AssetLibraryModel model;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_001";
    session.managedSourceRoot = quarantineRoot.generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_001/sprite/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            12,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
    };
    model.ingestImportSession(std::move(session));

    const auto promoted = model.promoteImportRecordToGlobalLibrary(
        "import_001", "asset.hero", "user_license_note", root / ".urpg" / "asset-library" / "promoted");
    REQUIRE(promoted.success);
    REQUIRE(promoted.code == "global_asset_promoted");
    REQUIRE(model.snapshot().last_action["action"] == "promote_import_record_global");
    REQUIRE(model.snapshot().promoted_count == 1);
    const auto promotedPath = root / ".urpg" / "asset-library" / "promoted" / "asset.hero" / "payloads" / "hero.png";
    REQUIRE(std::filesystem::is_regular_file(promotedPath));

    const auto attached = model.attachPromotedAssetToProject(
        (quarantineRoot / "characters" / "hero.png").generic_string(), root / "project");
    REQUIRE(attached.success);
    REQUIRE(attached.code == "project_asset_attached");
    REQUIRE(std::filesystem::is_regular_file(root / "project" / "content" / "assets" / "imported" / "asset.hero" /
                                             "hero.png"));
    REQUIRE(std::filesystem::is_regular_file(root / "project" / "content" / "assets" / "manifests" /
                                             "asset.hero.json"));
    REQUIRE(model.snapshot().referenced_asset_count == 1);

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel bulk promotes selected records into the global library",
          "[assets][asset_library][editor][asset_import][promotion]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_bulk_global_promote");
    std::filesystem::remove_all(root);
    const auto quarantineRoot = root / ".urpg" / "asset-library" / "sources" / "import_bulk" / "extracted";
    writeBinaryFile(quarantineRoot / "characters" / "hero.png", "hero-payload");

    urpg::editor::AssetLibraryModel model;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_bulk";
    session.managedSourceRoot = quarantineRoot.generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_bulk/sprite/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            12,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.theme",
            "audio/theme.mp3",
            "asset://import_bulk/audio/bgm/theme.mp3",
            ".mp3",
            "audio",
            "audio/bgm",
            "Fantasy Pack",
            "bbb",
            2048,
            0,
            0,
            93000,
            false,
            "",
            false,
            false,
            false,
            false,
            {"conversion_required"},
        },
    };
    model.ingestImportSession(std::move(session));

    const auto promotedRoot = root / ".urpg" / "asset-library" / "promoted";
    const auto result = model.promoteImportRecordsToGlobalLibrary(
        "import_bulk",
        std::vector<std::string>{"asset.hero", "asset.theme", "asset.missing"},
        "user_license_note",
        promotedRoot);

    REQUIRE(result["action"] == "promote_import_records_global");
    REQUIRE(result["success"] == false);
    REQUIRE(result["selected_count"] == 3);
    REQUIRE(result["promoted_count"] == 1);
    REQUIRE(result["blocked_count"] == 1);
    REQUIRE(result["missing_count"] == 1);
    REQUIRE(result["rows"].size() == 3);
    REQUIRE(model.snapshot().last_action["action"] == "promote_import_records_global");
    REQUIRE(model.snapshot().promoted_count == 1);
    REQUIRE(model.snapshot().project_attachable_count == 1);

    const auto payload = promotedRoot / "asset.hero" / "payloads" / "hero.png";
    const auto manifest = promotedRoot / "asset.hero" / "asset_promotion_manifest.json";
    REQUIRE(std::filesystem::is_regular_file(payload));
    REQUIRE(std::filesystem::is_regular_file(manifest));

    const auto hero = std::find_if(result["rows"].begin(), result["rows"].end(), [](const auto& row) {
        return row["asset_id"] == "asset.hero";
    });
    REQUIRE(hero != result["rows"].end());
    REQUIRE((*hero)["success"] == true);
    REQUIRE((*hero)["code"] == "global_asset_promoted");
    REQUIRE((*hero)["payload_path"] == payload.generic_string());
    REQUIRE((*hero)["manifest_path"] == manifest.generic_string());

    const auto theme = std::find_if(result["rows"].begin(), result["rows"].end(), [](const auto& row) {
        return row["asset_id"] == "asset.theme";
    });
    REQUIRE(theme != result["rows"].end());
    REQUIRE((*theme)["success"] == false);
    REQUIRE((*theme)["code"] == "global_promotion_blocked");
    REQUIRE((*theme)["diagnostics"][0] == "conversion_required");

    const auto missing = std::find_if(result["rows"].begin(), result["rows"].end(), [](const auto& row) {
        return row["asset_id"] == "asset.missing";
    });
    REQUIRE(missing != result["rows"].end());
    REQUIRE((*missing)["code"] == "import_record_not_found");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel reloads promoted global asset manifests",
          "[assets][asset_library][editor][promotion]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_reload_promoted");
    std::filesystem::remove_all(root);
    const auto libraryRoot = root / ".urpg" / "asset-library";
    const auto heroPayload = libraryRoot / "promoted" / "asset.hero" / "payloads" / "hero.png";
    writeBinaryFile(heroPayload, "hero-payload");
    std::filesystem::create_directories(libraryRoot / "promoted" / "asset.hero");
    {
        std::ofstream out(libraryRoot / "promoted" / "asset.hero" / "asset_promotion_manifest.json",
                          std::ios::binary | std::ios::trunc);
        out << nlohmann::json{
                   {"schemaVersion", "1.0.0"},
                   {"assetId", "asset.hero"},
                   {"sourcePath", ".urpg/asset-library/sources/import_001/extracted/characters/hero.png"},
                   {"promotedPath", heroPayload.generic_string()},
                   {"licenseId", "user_license_note"},
                   {"status", "runtime_ready"},
                   {"preview",
                    {{"kind", "image"}, {"thumbnailPath", heroPayload.generic_string()}, {"width", 48}, {"height", 48}}},
                   {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
                   {"diagnostics", nlohmann::json::array()},
               }.dump(2);
    }
    std::filesystem::create_directories(libraryRoot / "promoted" / "asset.theme");
    {
        std::ofstream out(libraryRoot / "promoted" / "asset.theme" / "asset_promotion_manifest.json",
                          std::ios::binary | std::ios::trunc);
        out << nlohmann::json{
                   {"schemaVersion", "1.0.0"},
                   {"assetId", "asset.theme"},
                   {"sourcePath", ".urpg/asset-library/sources/import_001/extracted/audio/theme.mp3"},
                   {"promotedPath", ""},
                   {"licenseId", "user_license_note"},
                   {"status", "blocked"},
                   {"preview", {{"kind", "pending"}}},
                   {"package", {{"includeInRuntime", false}, {"requiredForRelease", false}}},
                   {"diagnostics", {"conversion_required", "source_record_requires_conversion"}},
               }.dump(2);
    }

    urpg::editor::AssetLibraryModel model;
    std::string error = "not cleared";
    REQUIRE(model.loadPromotedAssetsFromLibraryRoot(libraryRoot, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().asset_count == 2);
    REQUIRE(model.snapshot().promoted_count == 1);
    REQUIRE(model.snapshot().runtime_ready_count == 1);
    REQUIRE(model.snapshot().asset_action_rows.size() == 2);

    const auto hero = std::find_if(
        model.snapshot().asset_action_rows.begin(), model.snapshot().asset_action_rows.end(), [](const auto& row) {
            return row["asset_id"] == "asset.hero";
        });
    REQUIRE(hero != model.snapshot().asset_action_rows.end());
    REQUIRE((*hero)["recommended_action"] == "attach_to_project");
    REQUIRE((*hero)["attach_button"]["enabled"] == true);

    const auto theme = std::find_if(
        model.snapshot().asset_action_rows.begin(), model.snapshot().asset_action_rows.end(), [](const auto& row) {
            return row["asset_id"] == "asset.theme";
        });
    REQUIRE(theme != model.snapshot().asset_action_rows.end());
    REQUIRE((*theme)["recommended_action"] == "convert_or_replace");
    REQUIRE((*theme)["attach_button"]["enabled"] == false);
    REQUIRE((*theme)["attach_button"]["disabled_reason"] == "asset_not_promoted");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel reloads project asset attachment manifests",
          "[assets][asset_library][editor][asset_attachment]") {
    const auto root = uniqueTempRoot("urpg_asset_library_model_reload_attach");
    std::filesystem::remove_all(root);
    const auto projectRoot = root / "project";
    const auto projectPayload = projectRoot / "content" / "assets" / "imported" / "asset.hero" / "hero.png";
    writeBinaryFile(projectPayload, "hero-payload");
    const auto projectManifest = projectRoot / "content" / "assets" / "manifests" / "asset.hero.json";
    std::filesystem::create_directories(projectManifest.parent_path());
    {
        std::ofstream out(projectManifest, std::ios::binary | std::ios::trunc);
        out << nlohmann::json{
                   {"schemaVersion", "1.0.0"},
                   {"assetId", "asset.hero"},
                   {"sourcePath", ".urpg/asset-library/promoted/asset.hero/payloads/hero.png"},
                   {"promotedPath", projectPayload.generic_string()},
                   {"licenseId", "user_license_note"},
                   {"status", "runtime_ready"},
                   {"preview",
                    {{"kind", "image"},
                     {"thumbnailPath", projectPayload.generic_string()},
                     {"width", 48},
                     {"height", 48}}},
                   {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
                   {"diagnostics", nlohmann::json::array()},
               }.dump(2);
    }

    urpg::editor::AssetLibraryModel model;
    std::string error = "not cleared";
    REQUIRE(model.loadProjectAssetAttachments(projectRoot, &error));
    REQUIRE(error.empty());
    REQUIRE(model.snapshot().asset_count == 1);
    REQUIRE(model.snapshot().promoted_count == 1);
    REQUIRE(model.snapshot().referenced_asset_count == 1);
    REQUIRE(model.snapshot().asset_action_rows.size() == 1);
    REQUIRE(model.snapshot().asset_action_rows[0]["asset_id"] == "asset.hero");
    REQUIRE(model.snapshot().asset_action_rows[0]["project_attached"] == true);
    REQUIRE(model.snapshot().asset_action_rows[0]["recommended_action"] == "project_attached");
    REQUIRE(model.snapshot().asset_action_rows[0]["attach_button"]["enabled"] == false);
    REQUIRE(model.snapshot().asset_action_rows[0]["attach_button"]["disabled_reason"] == "asset_already_attached");
    REQUIRE(model.snapshot().project_asset_picker_rows.size() == 1);
    REQUIRE(model.snapshot().project_asset_picker_rows[0]["asset_id"] == "asset.hero");
    REQUIRE(model.snapshot().project_asset_picker_rows[0]["project_path"] == projectPayload.generic_string());
    REQUIRE(model.snapshot().project_asset_picker_rows[0]["manifest_path"] == projectManifest.generic_string());
    REQUIRE(model.snapshot().project_asset_picker_rows[0]["picker_kind"] == "sprite");
    REQUIRE(model.snapshot().project_asset_picker_rows[0]["picker_targets"][0] == "level_builder");

    std::filesystem::remove_all(root);
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
    REQUIRE(model.snapshot().filter_controls["quick_filters"]["sequence_packs"]["media_kind"] == "image_sequence_collection");
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
    const auto sequence =
        model.library().findAsset("imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon");
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
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["recommended_action"] == "archived");
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["promote_button"]["enabled"] == false);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["promote_button"]["disabled_reason"] == "asset_archived");
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["archive_button"]["enabled"] == false);
    REQUIRE(panel.lastRenderSnapshot().asset_action_rows[0]["archive_button"]["disabled_reason"] == "asset_already_archived");
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
    const auto promoted = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/example/hero.png";
    });
    REQUIRE(promoted != rows.end());
    REQUIRE((*promoted)["recommended_action"] == "attach_to_project");
    REQUIRE((*promoted)["attach_button"]["enabled"] == true);
    REQUIRE((*promoted)["promotion_status"] == "runtime_ready");
    REQUIRE((*promoted)["include_in_runtime"] == true);

    const auto unlicensed = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/example/unlicensed.png";
    });
    REQUIRE(unlicensed != rows.end());
    REQUIRE((*unlicensed)["recommended_action"] == "add_license_evidence");
    REQUIRE((*unlicensed)["include_in_runtime"] == false);

    const auto missing = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/example/missing.png";
    });
    REQUIRE(missing != rows.end());
    REQUIRE((*missing)["recommended_action"] == "fix_missing_file");

    const auto archived = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/example/hero-copy.png";
    });
    REQUIRE(archived != rows.end());
    REQUIRE((*archived)["recommended_action"] == "archived");
    REQUIRE((*archived)["include_in_runtime"] == false);
    REQUIRE((*archived)["required_for_release"] == false);
}
