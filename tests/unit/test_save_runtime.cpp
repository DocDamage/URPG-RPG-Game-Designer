#include "engine/core/save/save_migration.h"
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
