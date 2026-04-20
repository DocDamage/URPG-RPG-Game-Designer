#include "engine/core/save/save_migration.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST_CASE("Save migration upgrades compat metadata into native save metadata shape", "[save][migration]") {
    const json compat = {
        {"_urpg_format_version", "mz_compat_1"},
        {"meta",
         {
             {"slotId", 5},
             {"mapName", "Ruined Archive"},
             {"playtimeSeconds", 1234},
             {"saveVersion", "mz-import"},
         }},
        {"pluginHeader",
         {
             {"uiTab", "party"},
             {"thumbnailHash", "thumb123"},
         }},
    };

    const auto migrated = urpg::save::UpgradeCompatSaveMetadataDocument(compat);

    REQUIRE(migrated.migrated_metadata["_urpg_format_version"] == "1.0");
    REQUIRE(migrated.migrated_metadata["_slot_id"] == 5);
    REQUIRE(migrated.migrated_metadata["_map_display_name"] == "Ruined Archive");
    REQUIRE(migrated.migrated_metadata["_playtime_seconds"] == 1234);
    REQUIRE(migrated.migrated_metadata["_save_version"] == "mz-import");
    REQUIRE(migrated.migrated_metadata["_thumbnail_hash"] == "thumb123");
    REQUIRE(migrated.migrated_metadata["_ui_tab"] == "party");
    REQUIRE_FALSE(migrated.used_safe_fallback);
}

TEST_CASE("Save migration records unmapped compat fields as diagnostics and mapping notes", "[save][migration]") {
    const json compat = {
        {"_urpg_format_version", "mz_compat_1"},
        {"meta",
         {
             {"slotId", 2},
             {"mapName", "Glass Harbor"},
             {"customBadge", "veteran"},
         }},
        {"pluginHeader",
         {
             {"uiTab", "system"},
             {"theme", "midnight"},
         }},
    };

    const auto migrated = urpg::save::UpgradeCompatSaveMetadataDocument(compat);

    const auto has_code = [&](std::string_view code) {
        return std::any_of(
            migrated.diagnostics.begin(),
            migrated.diagnostics.end(),
            [&](const urpg::save::SaveMigrationDiagnostic& diagnostic) { return diagnostic.code == code; });
    };

    REQUIRE(has_code("unmapped_meta_field"));
    REQUIRE(has_code("unmapped_plugin_header_field"));
    REQUIRE(migrated.used_safe_fallback);
    REQUIRE(migrated.migrated_metadata.contains("_compat_mapping_notes"));
    REQUIRE(migrated.migrated_metadata["_compat_mapping_notes"].is_array());
    REQUIRE(migrated.migrated_metadata["_compat_mapping_notes"].size() == 2);
}

TEST_CASE("Save migration diagnostics export emits JSONL stream", "[save][migration]") {
    const json compat = {
        {"_urpg_format_version", "mz_compat_1"},
        {"meta", {{"slotId", 3}, {"unknownField", "mystery"}}},
    };

    const auto migrated = urpg::save::UpgradeCompatSaveMetadataDocument(compat);
    const auto jsonl = urpg::save::ExportSaveMigrationDiagnosticsJsonl(migrated);

    REQUIRE_FALSE(jsonl.empty());
    const auto first_line = jsonl.substr(0, jsonl.find('\n'));
    const auto first = json::parse(first_line);
    REQUIRE(first["subsystem"] == "save_migration");
    REQUIRE(first.contains("level"));
    REQUIRE(first.contains("code"));
    REQUIRE(first.contains("field_path"));
}
