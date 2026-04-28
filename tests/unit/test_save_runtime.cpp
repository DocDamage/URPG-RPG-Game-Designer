#include "engine/core/save/save_migration.h"
#include "editor/save/save_load_preview_lab_panel.h"
#include "engine/core/save/save_runtime.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace {

void WriteText(const std::filesystem::path& path, const std::string& value) {
    std::ofstream out(path, std::ios::binary);
    out << value;
}

std::filesystem::path saveRuntimeRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json LoadSaveRuntimeJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

} // namespace

TEST_CASE("Runtime save loader uses primary save when available", "[save][runtime]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_runtime_primary";
    std::filesystem::create_directories(base);

    const auto primary = base / "slot_001.json";
    const auto meta = base / "meta.json";

    WriteText(primary, "{\"state\":\"primary\"}");
    WriteText(meta, "{\"_slot_id\":1,\"_map_display_name\":\"Town\"}");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = primary;
    request.metadata_path = meta;

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE_FALSE(result.boot_safe_mode);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::None);
    REQUIRE(result.payload == "{\"state\":\"primary\"}");
    REQUIRE(result.active_meta.map_display_name == "Town");

    std::filesystem::remove_all(base);
}

TEST_CASE("Runtime save loader falls back to recovery autosave", "[save][runtime]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_runtime_recover_l1";
    std::filesystem::create_directories(base);

    const auto autosave = base / "autosave.json";
    WriteText(autosave, "{\"state\":\"autosave\"}");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = base / "missing_primary.json";
    request.autosave_path = autosave;
    request.metadata_path = base / "missing_meta.json";
    request.variables_path = base / "missing_vars.json";

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE_FALSE(result.boot_safe_mode);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level1Autosave);
    REQUIRE(result.payload == "{\"state\":\"autosave\"}");

    std::filesystem::remove_all(base);
}

TEST_CASE("Runtime save loader enters safe mode on level 3 recovery", "[save][runtime]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_runtime_recover_l3";
    std::filesystem::create_directories(base);

    const auto meta = base / "meta.json";
    WriteText(meta, "{\"_slot_id\":9,\"_map_display_name\":\"Crimson Keep - B2\"}");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = base / "missing_primary.json";
    request.metadata_path = meta;
    request.variables_path = base / "missing_vars.json";

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE(result.boot_safe_mode);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE(result.active_meta.map_display_name == "Crimson Keep - B2");

    std::filesystem::remove_all(base);
}

TEST_CASE("Runtime save loader force-safe-mode bypasses primary save", "[save][runtime]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_runtime_force_safe";
    std::filesystem::create_directories(base);

    const auto primary = base / "slot_001.json";
    WriteText(primary, "{\"state\":\"primary\"}");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = primary;
    request.force_safe_mode = true;
    request.safe_mode_fallback_map = "Safe Mode - Origin";

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE(result.boot_safe_mode);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE(result.active_meta.map_display_name == "Safe Mode - Origin");
    REQUIRE(result.payload.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("Save/load preview lab round-trips authored slot through runtime loader",
          "[save][runtime][preview_lab][wysiwyg]") {
    const auto json = LoadSaveRuntimeJson(
        saveRuntimeRepoRoot() / "content" / "fixtures" / "save_load_preview_lab_fixture.json");
    const auto document = urpg::save::SaveLoadPreviewLabDocument::fromJson(json);
    const auto workspace = std::filesystem::temp_directory_path() / "urpg_save_load_preview_lab_roundtrip";
    std::filesystem::remove_all(workspace);

    urpg::editor::SaveLoadPreviewLabPanel panel;
    panel.loadDocument(document, workspace);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().lab_id == "slot_roundtrip_lab");
    REQUIRE(panel.snapshot().saved_primary);
    REQUIRE(panel.snapshot().loaded_ok);
    REQUIRE_FALSE(panel.snapshot().loaded_from_recovery);
    REQUIRE_FALSE(panel.snapshot().boot_safe_mode);
    REQUIRE(panel.snapshot().recovery_tier == "none");
    REQUIRE(panel.snapshot().payload_matches_expected);
    REQUIRE(panel.snapshot().loaded_slot_id == 3);
    REQUIRE(panel.snapshot().loaded_map_display_name == "Town");
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().status_message == "Save/load preview lab is ready.");
    REQUIRE_FALSE(panel.snapshot().saved_project_json.empty());

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Save/load preview lab saved project data round-trips and uses recovery preview",
          "[save][runtime][preview_lab][wysiwyg]") {
    const auto json = LoadSaveRuntimeJson(
        saveRuntimeRepoRoot() / "content" / "fixtures" / "save_load_preview_lab_fixture.json");
    auto document = urpg::save::SaveLoadPreviewLabDocument::fromJson(json);
    const auto saved = document.toJson();
    auto restored = urpg::save::SaveLoadPreviewLabDocument::fromJson(saved);
    restored.id = "slot_recovery_lab";
    restored.corrupt_primary = true;

    const auto workspace = std::filesystem::temp_directory_path() / "urpg_save_load_preview_lab_recovery";
    std::filesystem::remove_all(workspace);
    const auto result = urpg::save::RunSaveLoadPreviewLab(restored, workspace);

    REQUIRE(saved["schema"] == "urpg.save_load_preview_lab.v1");
    REQUIRE(result.loaded_ok);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level1Autosave);
    REQUIRE(result.payload_matches_expected);
    REQUIRE(result.loaded_payload.find("\"state\":\"autosave\"") != std::string::npos);
    REQUIRE_FALSE(result.diagnostics.empty());
    REQUIRE(result.diagnostics.back().code == "recovery_path_used");

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Save/load preview lab diagnostics block false complete claims",
          "[save][runtime][preview_lab][wysiwyg]") {
    urpg::save::SaveLoadPreviewLabDocument document;
    document.id = "";
    document.slot_id = -1;
    document.primary_payload = nlohmann::json::array();
    document.safe_mode_fallback_map = "";

    const auto workspace = std::filesystem::temp_directory_path() / "urpg_save_load_preview_lab_broken";
    std::filesystem::remove_all(workspace);

    urpg::editor::SaveLoadPreviewLabPanel panel;
    panel.loadDocument(document, workspace);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().diagnostic_count >= 4);
    REQUIRE_FALSE(panel.snapshot().loaded_ok);
    REQUIRE_FALSE(panel.snapshot().payload_matches_expected);
    REQUIRE(panel.snapshot().status_message == "Save/load preview lab has diagnostics.");

    const auto& diagnostics = panel.result().diagnostics;
    const auto hasCode = [&diagnostics](const std::string& code) {
        return std::any_of(diagnostics.begin(), diagnostics.end(), [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
    };
    REQUIRE(hasCode("missing_lab_id"));
    REQUIRE(hasCode("invalid_slot_id"));
    REQUIRE(hasCode("invalid_primary_payload"));
    REQUIRE(hasCode("missing_safe_mode_map"));

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Runtime save loader hydrates metadata after imported save migration", "[save][runtime][migrate]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_runtime_migrated_meta";
    std::filesystem::create_directories(base);

    nlohmann::json legacyMeta = {
        {"_urpg_format_version", "mz_compat_1"},
        {"slotId", 6},
        {"mapName", "Sunken Atrium"},
        {"playtimeSeconds", 9876},
        {"saveVersion", "mz-import"},
        {"thumbnailHash", "thumb-save-6"}
    };

    const auto migrationResult = urpg::save::UpgradeCompatSaveMetadataDocument(legacyMeta);
    REQUIRE(migrationResult.diagnostics.empty());

    const auto primary = base / "slot_006.json";
    const auto meta = base / "meta.json";
    WriteText(primary, "{\"state\":\"migrated_primary\"}");
    WriteText(meta, migrationResult.migrated_metadata.dump());

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = primary;
    request.metadata_path = meta;

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE(result.payload == "{\"state\":\"migrated_primary\"}");
    REQUIRE(result.active_meta.slot_id == 6);
    REQUIRE(result.active_meta.map_display_name == "Sunken Atrium");
    REQUIRE(result.active_meta.playtime_seconds == 9876);
    REQUIRE(result.active_meta.save_version == "mz-import");
    REQUIRE(result.active_meta.thumbnail_hash == "thumb-save-6");

    std::filesystem::remove_all(base);
}

TEST_CASE("Runtime save loader consumes imported compat save payload through native path", "[save][runtime][import]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_runtime_imported_payload";
    std::filesystem::create_directories(base);

    nlohmann::json compatSave = {
        {"_urpg_format_version", "mz_compat_1"},
        {"meta", {
            {"slotId", 12},
            {"mapName", "Signal Spire"},
            {"playtimeSeconds", 4444}
        }},
        {"gold", 875},
        {"mapId", 5},
        {"playerX", 22},
        {"playerY", 11},
        {"direction", 4},
        {"party", {
            {{"actorId", 1}, {"name", "Iris"}, {"level", 14}, {"hp", 310}, {"mhp", 360}}
        }},
        {"switches", {{"boss_gate_open", true}}},
        {"variables", {{"chapter", 3}}},
        {"pluginData", {{"campfire", {{"enabled", true}}}}}
    };

    const auto imported = urpg::save::ImportCompatSaveDocument(compatSave);
    REQUIRE_FALSE(imported.diagnostics.empty());

    const auto primary = base / "slot_012.json";
    const auto meta = base / "meta.json";
    WriteText(primary, imported.native_payload.dump());
    WriteText(meta, imported.migrated_metadata.dump());

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = primary;
    request.metadata_path = meta;

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE(result.active_meta.slot_id == 12);
    REQUIRE(result.active_meta.map_display_name == "Signal Spire");
    REQUIRE(result.active_meta.playtime_seconds == 4444);

    const auto payload = nlohmann::json::parse(result.payload);
    REQUIRE(payload["player"]["gold"] == 875);
    REQUIRE(payload["player"]["x"] == 22);
    REQUIRE(payload["player"]["direction"] == 4);
    REQUIRE(payload["map_id"] == 5);
    REQUIRE(payload["party"].size() == 1);
    REQUIRE(payload["party"][0]["actor_id"] == 1);
    REQUIRE(payload["switches"]["boss_gate_open"] == true);
    REQUIRE(payload["variables"]["chapter"] == 3);
    REQUIRE(payload["_compat_payload_retained"]["/pluginData"]["campfire"]["enabled"] == true);

    std::filesystem::remove_all(base);
}
