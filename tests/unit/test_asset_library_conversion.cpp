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

TEST_CASE("AssetLibraryModel runs audio conversion handoff and unblocks promotion",
          "[assets][asset_library][editor][asset_import][conversion]") {
    const auto root = uniqueTempRoot("urpg_asset_library_audio_conversion");
    std::filesystem::remove_all(root);
    const auto managedRoot = root / ".urpg" / "asset-library" / "sources" / "import_audio" / "original";
    writeBinaryFile(managedRoot / "audio" / "bgm" / "theme.mp3", "source-audio");

    urpg::editor::AssetLibraryModel model;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_audio";
    session.managedSourceRoot = managedRoot.generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records = {
        {
            "asset.theme",
            "audio/bgm/theme.mp3",
            "asset://import_audio/audio/bgm/theme.mp3",
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
            "converted/audio/bgm/theme.wav",
            {"ffmpeg", "-y", "-i", "audio/bgm/theme.mp3", "converted/audio/bgm/theme.wav"},
            true,
        },
    };
    model.ingestImportSession(std::move(session));
    REQUIRE(model.snapshot().import_needs_conversion_count == 1);

    const auto result = model.runImportRecordConversion(
        "import_audio", "asset.theme", [&](const urpg::editor::AssetLibraryModel::ConversionCommand& command) {
            REQUIRE(command.working_directory == managedRoot);
            REQUIRE(command.arguments.size() == 5);
            REQUIRE(command.arguments[0] == "ffmpeg");
            writeBinaryFile(managedRoot / "converted" / "audio" / "bgm" / "theme.wav", "converted-audio");
            return urpg::editor::AssetLibraryModel::ConversionCommandResult{0, "", ""};
        });

    REQUIRE(result["success"] == true);
    REQUIRE(result["code"] == "import_record_converted");
    REQUIRE(model.snapshot().import_needs_conversion_count == 0);
    REQUIRE(model.snapshot().import_ready_count == 1);
    REQUIRE(model.snapshot().import_review_rows[0]["relative_path"] == "converted/audio/bgm/theme.wav");
    REQUIRE(model.snapshot().import_review_rows[0]["extension"] == ".wav");
    REQUIRE(model.snapshot().import_review_rows[0]["conversion_required"] == false);
    REQUIRE(model.snapshot().import_review_rows[0]["conversion_command"].empty());
    REQUIRE(model.snapshot().import_review_rows[0]["review_state"] == "ready_to_promote");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryModel conversion process runner uses explicit working directory",
          "[assets][asset_library][editor][asset_import][conversion]") {
    const auto original = std::filesystem::current_path();
    const auto root = uniqueTempRoot("urpg_asset_library_conversion_process_runner");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    urpg::editor::AssetLibraryModel::ConversionCommand command;
    command.working_directory = root;
    command.output_path = root / "converted.wav";
    command.arguments = {
        "python",
        "-c",
        "from pathlib import Path; Path('converted.wav').write_bytes(b'converted')",
    };

    const auto result = urpg::editor::AssetLibraryModel::runConversionCommand(command);

    REQUIRE(result.exit_code == 0);
    REQUIRE(std::filesystem::is_regular_file(command.output_path));
    REQUIRE(std::filesystem::current_path() == original);

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibraryPanel dispatches selected import conversions through the wizard",
          "[assets][asset_library][editor][asset_import][wizard][conversion]") {
    const auto root = uniqueTempRoot("urpg_asset_library_panel_conversion");
    std::filesystem::remove_all(root);
    const auto managedRoot = root / ".urpg" / "asset-library" / "sources" / "import_audio_panel" / "original";
    writeBinaryFile(managedRoot / "audio" / "bgm" / "theme.mp3", "source-audio");

    urpg::editor::AssetLibraryPanel panel;
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_audio_panel";
    session.managedSourceRoot = managedRoot.generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records = {
        {
            "asset.theme",
            "audio/bgm/theme.mp3",
            "asset://import_audio_panel/audio/bgm/theme.mp3",
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
            "converted/audio/bgm/theme.wav",
            {"ffmpeg", "-y", "-i", "audio/bgm/theme.mp3", "converted/audio/bgm/theme.wav"},
            true,
        },
    };
    panel.model().ingestImportSession(std::move(session));
    panel.render();

    const auto convertAction =
        std::find_if(panel.lastImportWizardSnapshot().actions.begin(), panel.lastImportWizardSnapshot().actions.end(),
                     [](const auto& action) { return action.id == "convert_selected"; });
    REQUIRE(convertAction != panel.lastImportWizardSnapshot().actions.end());
    REQUIRE(convertAction->enabled);
    REQUIRE(convertAction->eligible_count == 1);

    const auto result = panel.convertSelectedImportRecords(
        "import_audio_panel", {"asset.theme"}, [&](const urpg::editor::AssetLibraryModel::ConversionCommand& command) {
            REQUIRE(command.working_directory == managedRoot);
            writeBinaryFile(managedRoot / "converted" / "audio" / "bgm" / "theme.wav", "converted-audio");
            return urpg::editor::AssetLibraryModel::ConversionCommandResult{0, "", ""};
        });

    REQUIRE(result["action"] == "convert_import_records");
    REQUIRE(result["success"] == true);
    REQUIRE(result["converted_count"] == 1);
    REQUIRE(panel.lastImportWizardSnapshot().status == "review_required");
    const auto promoteAction =
        std::find_if(panel.lastImportWizardSnapshot().actions.begin(), panel.lastImportWizardSnapshot().actions.end(),
                     [](const auto& action) { return action.id == "promote_selected"; });
    REQUIRE(promoteAction != panel.lastImportWizardSnapshot().actions.end());
    REQUIRE(promoteAction->enabled);
    REQUIRE(promoteAction->eligible_count == 1);

    std::filesystem::remove_all(root);
}
