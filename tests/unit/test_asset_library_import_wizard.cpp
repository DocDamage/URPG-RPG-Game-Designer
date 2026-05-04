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
    const auto ready =
        std::find_if(model.snapshot().import_review_rows.begin(), model.snapshot().import_review_rows.end(),
                     [](const auto& row) { return row["relative_path"] == "characters/hero.png"; });
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
    const auto reviewStep =
        std::find_if(wizard.steps.begin(), wizard.steps.end(), [](const auto& step) { return step.id == "review"; });
    REQUIRE(reviewStep != wizard.steps.end());
    REQUIRE(reviewStep->active);
    REQUIRE(reviewStep->count == 1);

    const auto promoteAction = std::find_if(wizard.actions.begin(), wizard.actions.end(),
                                            [](const auto& action) { return action.id == "promote_selected"; });
    REQUIRE(promoteAction != wizard.actions.end());
    REQUIRE(promoteAction->enabled);
    REQUIRE(promoteAction->eligible_count == 1);
    REQUIRE(promoteAction->disabled_reason.empty());

    const auto packageAction = std::find_if(wizard.actions.begin(), wizard.actions.end(),
                                            [](const auto& action) { return action.id == "package_validate"; });
    REQUIRE(packageAction != wizard.actions.end());
    REQUIRE_FALSE(packageAction->enabled);
    REQUIRE(packageAction->disabled_reason == "no_attached_project_assets");
    REQUIRE_FALSE(wizard.package_validation_ready);
}

TEST_CASE("AssetLibraryPanel dispatches import wizard actions through the model",
          "[assets][asset_library][editor][asset_import][wizard][actions]") {
    const auto root = uniqueTempRoot("urpg_asset_library_panel_wizard_actions");
    std::filesystem::remove_all(root);
    const auto source = root / "downloads" / "fantasy_pack";
    const auto libraryRoot = root / ".urpg" / "asset-library";
    const auto promotedRoot = libraryRoot / "promoted";
    const auto projectRoot = root / "project";

    urpg::editor::AssetLibraryPanel panel;
    const auto request =
        panel.requestImportSource(source, libraryRoot, "import_panel_actions_001", "User-provided test license.");
    REQUIRE(request["success"] == true);
    REQUIRE(panel.lastImportWizardSnapshot().status == "source_requested");
    REQUIRE(panel.lastImportWizardSnapshot().current_step == "add_source");
    REQUIRE(panel.lastImportWizardSnapshot().pending_request["session_id"] == "import_panel_actions_001");

    urpg::assets::AssetImportSession session;
    session.sessionId = "import_panel_actions_001";
    session.sourceKind = urpg::assets::AssetImportSourceKind::Folder;
    session.sourcePath = source.generic_string();
    session.managedSourceRoot = (libraryRoot / "sources" / "import_panel_actions_001" / "original").generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.createdAt = "2026-05-01T00:01:00Z";
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_panel_actions_001/characters/hero.png",
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
    REQUIRE(panel.lastImportWizardSnapshot().status == "review_required");

    const auto promotion = panel.promoteSelectedImportRecords("import_panel_actions_001", {"asset.hero"},
                                                              "user_license_note", promotedRoot.generic_string(), true);
    REQUIRE(promotion["success"] == true);
    REQUIRE(panel.lastImportWizardSnapshot().status == "ready_to_attach");
    REQUIRE(panel.lastImportWizardSnapshot().current_step == "attach");
    const auto attachAction =
        std::find_if(panel.lastImportWizardSnapshot().actions.begin(), panel.lastImportWizardSnapshot().actions.end(),
                     [](const auto& action) { return action.id == "attach_selected"; });
    REQUIRE(attachAction != panel.lastImportWizardSnapshot().actions.end());
    REQUIRE(attachAction->enabled);
    REQUIRE(attachAction->eligible_count == 1);

    const auto promotedPayload = promotedRoot / "asset.hero" / "payloads" / "hero.png";
    writeBinaryFile(promotedPayload, "hero-payload");
    const auto attachment = panel.attachSelectedPromotedAssetsToProject(
        {((libraryRoot / "sources" / "import_panel_actions_001" / "original" / "characters" / "hero.png")
              .generic_string())},
        projectRoot);
    REQUIRE(attachment["success"] == true);
    REQUIRE(panel.lastImportWizardSnapshot().status == "package_ready");
    REQUIRE(panel.lastImportWizardSnapshot().current_step == "package");
    REQUIRE(panel.lastImportWizardSnapshot().package_validation_ready);

    urpg::tools::ExportConfig config{};
    config.outputDir = (root / "export").generic_string();
    config.assetDiscoveryRoots = {(projectRoot / "content" / "assets" / "imported").generic_string()};
    const auto validation = panel.validatePackage(config);
    REQUIRE(validation["action"] == "asset_library_package_validate");
    REQUIRE(validation["success"] == true);
    REQUIRE(validation["code"] == "package_validation_passed");
    REQUIRE(validation["errors"].empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryPanel requests add-source through an import source picker",
          "[assets][asset_library][editor][asset_import][wizard][picker]") {
    const auto root = uniqueTempRoot("urpg_asset_library_panel_picker");
    const auto source = root / "downloads" / "fantasy_pack.zip";
    const auto libraryRoot = root / ".urpg" / "asset-library";
    std::filesystem::create_directories(source.parent_path());
    writeBinaryFile(source, "zip");

    urpg::editor::AssetLibraryPanel panel;
    panel.setImportSourcePicker([&](const urpg::editor::AssetLibraryPanel::ImportSourcePickerRequest& request) {
        REQUIRE(request.mode == urpg::editor::AssetLibraryPanel::ImportSourcePickerMode::FileOrArchive);
        REQUIRE(request.library_root == libraryRoot);
        REQUIRE(request.session_id == "import_picker_001");
        REQUIRE(request.license_note == "User-provided picker license.");
        return std::optional<std::filesystem::path>{source};
    });

    const auto request = panel.requestImportSourceFromPicker({
        urpg::editor::AssetLibraryPanel::ImportSourcePickerMode::FileOrArchive,
        libraryRoot,
        "import_picker_001",
        "User-provided picker license.",
        {},
    });

    REQUIRE(request["success"] == true);
    REQUIRE(request["source_path"] == source.generic_string());
    REQUIRE(request["library_root"] == libraryRoot.generic_string());
    REQUIRE(panel.lastImportWizardSnapshot().status == "source_requested");
    REQUIRE(panel.lastImportWizardSnapshot().pending_request["session_id"] == "import_picker_001");

    panel.setImportSourcePicker([](const urpg::editor::AssetLibraryPanel::ImportSourcePickerRequest&) {
        return std::optional<std::filesystem::path>{};
    });
    const auto cancelled = panel.requestImportSourceFromPicker({
        urpg::editor::AssetLibraryPanel::ImportSourcePickerMode::Folder,
        libraryRoot,
        "import_picker_cancelled",
        "",
        {},
    });
    REQUIRE(cancelled["success"] == false);
    REQUIRE(cancelled["code"] == "import_source_picker_cancelled");

    const auto missingSource = root / "downloads" / "missing_pack.zip";
    panel.setImportSourcePicker([&](const urpg::editor::AssetLibraryPanel::ImportSourcePickerRequest&) {
        return std::optional<std::filesystem::path>{missingSource};
    });
    const auto invalid = panel.requestImportSourceFromPicker({
        urpg::editor::AssetLibraryPanel::ImportSourcePickerMode::FileOrArchive,
        libraryRoot,
        "import_picker_invalid",
        "",
        {},
    });
    REQUIRE(invalid["success"] == false);
    REQUIRE(invalid["code"] == "import_source_picker_invalid_path");
    REQUIRE(invalid["invalid_reason"] == "source_path_not_found");
    REQUIRE(invalid["source_path"] == missingSource.generic_string());
}

TEST_CASE("AssetLibraryPanel exposes native import picker availability",
          "[assets][asset_library][editor][asset_import][wizard][picker]") {
    const auto availability = urpg::editor::AssetLibraryPanel::nativeImportSourcePickerAvailability();

#ifdef _WIN32
    REQUIRE(availability.available == true);
    REQUIRE(availability.code == "native_import_source_picker_available");
#else
    REQUIRE(availability.available == false);
    REQUIRE(availability.code == "native_import_source_picker_unsupported");
#endif
    REQUIRE(availability.path_entry_available == true);
}

TEST_CASE("AssetLibraryModel requests add-source import command handoff",
          "[assets][asset_library][editor][asset_import][wizard]") {
    const auto root = uniqueTempRoot("urpg_asset_library_add_source_request");
    const auto source = root / "fantasy_pack";
    const auto libraryRoot = root / ".urpg" / "asset-library";

    urpg::editor::AssetLibraryModel model;
    const auto request =
        model.requestImportSource(source, libraryRoot, "import_manual_001", "User-provided test license.");

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

TEST_CASE("AssetLibraryModel request import source supports configured importer paths",
          "[assets][asset_library][editor][asset_import][wizard]") {
    const auto root = uniqueTempRoot("urpg_asset_library_add_source_tool_paths");
    const auto source = root / "fantasy_pack.zip";
    const auto libraryRoot = root / ".urpg" / "asset-library";

    urpg::editor::AssetLibraryModel model;
    model.setImportToolCommand({"C:/Tools/Python/python.exe", "C:/URPG/tools/assets/global_asset_import.py"});

    const auto request =
        model.requestImportSource(source, libraryRoot, "import_tool_paths_001", "User-provided test license.");

    REQUIRE(request["command"][0] == "C:/Tools/Python/python.exe");
    REQUIRE(request["command"][1] == "C:/URPG/tools/assets/global_asset_import.py");
    REQUIRE(model.snapshot().import_wizard["pending_request"]["command"][0] == "C:/Tools/Python/python.exe");
}

TEST_CASE("AssetLibraryModel requests add-source with external archive extractor handoff",
          "[assets][asset_library][editor][asset_import][wizard][archive]") {
    const auto root = uniqueTempRoot("urpg_asset_library_add_source_external_extractor");
    const auto source = root / "packs" / "sprites.7z";
    const auto libraryRoot = root / ".urpg" / "asset-library";

    urpg::editor::AssetLibraryModel model;
    const auto request =
        model.requestImportSource(source, libraryRoot, "import_external_archive_001", "User-provided archive license.",
                                  {"C:/Program Files/7-Zip/7z.exe", "x", "-y"});

    REQUIRE(request["success"] == true);
    REQUIRE(request["external_extractor_command"].size() == 3);
    REQUIRE(request["external_extractor_command"][0] == "C:/Program Files/7-Zip/7z.exe");
    const auto& command = request["command"];
    const auto extractorArg = std::find(command.begin(), command.end(), "--external-extractor-command");
    REQUIRE(extractorArg != command.end());
    REQUIRE(std::next(extractorArg) != command.end());
    REQUIRE(*std::next(extractorArg) == "\"C:/Program Files/7-Zip/7z.exe\" x -y");
    REQUIRE(model.snapshot().import_wizard["pending_request"]["external_extractor_command"].size() == 3);
}

TEST_CASE("AssetLibraryModel applies configured external archive extractor to add-source requests",
          "[assets][asset_library][editor][asset_import][wizard][archive]") {
    EnvironmentVariableGuard extractorEnv("URPG_ASSET_ARCHIVE_EXTRACTOR");
    extractorEnv.set("\"C:/Program Files/7-Zip/7z.exe\" x -y {source} -o{destination}");

    const auto root = uniqueTempRoot("urpg_asset_library_add_source_configured_external_extractor");
    const auto source = root / "packs" / "portraits.rar";
    const auto libraryRoot = root / ".urpg" / "asset-library";

    urpg::editor::AssetLibraryModel model;
    const auto request = model.requestImportSource(source, libraryRoot, "import_configured_external_archive_001",
                                                   "User-provided archive license.");

    REQUIRE(request["success"] == true);
    REQUIRE(request["external_extractor_command"].size() == 5);
    REQUIRE(request["external_extractor_command"][0] == "C:/Program Files/7-Zip/7z.exe");
    REQUIRE(request["external_extractor_command"][3] == "{source}");
    REQUIRE(request["external_extractor_command"][4] == "-o{destination}");

    const auto& command = request["command"];
    const auto extractorArg = std::find(command.begin(), command.end(), "--external-extractor-command");
    REQUIRE(extractorArg != command.end());
    REQUIRE(std::next(extractorArg) != command.end());
    REQUIRE(*std::next(extractorArg) == "\"C:/Program Files/7-Zip/7z.exe\" x -y {source} -o{destination}");
    REQUIRE(model.snapshot().import_wizard["pending_request"]["external_extractor_command"][4] == "-o{destination}");
}

TEST_CASE("AssetLibraryModel exposes configured archive extractor status in the wizard snapshot",
          "[assets][asset_library][editor][asset_import][wizard][archive]") {
    EnvironmentVariableGuard extractorEnv("URPG_ASSET_ARCHIVE_EXTRACTOR");
    extractorEnv.set("\"C:/Program Files/7-Zip/7z.exe\" x -y {source} -o{destination}");

    urpg::editor::AssetLibraryModel model;
    model.rebuildCleanupPreview();

    const auto& configuration = model.snapshot().import_wizard["extractor_configuration"];
    REQUIRE(configuration["configured"] == true);
    REQUIRE(configuration["source"] == "environment");
    REQUIRE(configuration["environment_variable"] == "URPG_ASSET_ARCHIVE_EXTRACTOR");
    REQUIRE(configuration["supports_rar_7z"] == true);
    REQUIRE(configuration["command"].size() == 5);
    REQUIRE(configuration["command"][0] == "C:/Program Files/7-Zip/7z.exe");
    REQUIRE(configuration["command"][3] == "{source}");
    REQUIRE(configuration["command"][4] == "-o{destination}");
}

TEST_CASE("AssetLibraryPanel exposes configured archive extractor status in render snapshot",
          "[assets][asset_library][editor][asset_import][wizard][archive]") {
    EnvironmentVariableGuard extractorEnv("URPG_ASSET_ARCHIVE_EXTRACTOR");
    extractorEnv.set("\"C:/Program Files/7-Zip/7z.exe\" x -y {source} -o{destination}");

    urpg::editor::AssetLibraryPanel panel;
    panel.render();

    const auto& configuration = panel.lastImportWizardSnapshot().extractor_configuration;
    REQUIRE(configuration["configured"] == true);
    REQUIRE(configuration["source"] == "environment");
    REQUIRE(configuration["environment_variable"] == "URPG_ASSET_ARCHIVE_EXTRACTOR");
    REQUIRE(configuration["supports_rar_7z"] == true);
    REQUIRE(configuration["command"][4] == "-o{destination}");
}

TEST_CASE("AssetLibraryModel reports invalid external archive extractor configuration",
          "[assets][asset_library][editor][asset_import][wizard][archive]") {
    EnvironmentVariableGuard extractorEnv("URPG_ASSET_ARCHIVE_EXTRACTOR");
    extractorEnv.set("\"C:/Program Files/7-Zip/7z.exe");

    urpg::editor::AssetLibraryModel model;

    const auto& configuration = model.snapshot().import_wizard["extractor_configuration"];
    REQUIRE(configuration["configured"] == false);
    REQUIRE(configuration["source"] == "environment");
    REQUIRE(configuration["environment_variable"] == "URPG_ASSET_ARCHIVE_EXTRACTOR");
    REQUIRE(configuration["supports_rar_7z"] == false);
    REQUIRE(configuration["diagnostics"][0] == "external_extractor_command_parse_error");
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

    const auto missing_license =
        std::find_if(model.snapshot().import_review_rows.begin(), model.snapshot().import_review_rows.end(),
                     [](const auto& row) { return row["relative_path"] == "ui/window.png"; });
    REQUIRE(missing_license != model.snapshot().import_review_rows.end());
    REQUIRE((*missing_license)["review_state"] == "missing_license");
    REQUIRE((*missing_license)["recommended_action"] == "add_license_attribution");

    std::filesystem::remove_all(root);
}
