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

