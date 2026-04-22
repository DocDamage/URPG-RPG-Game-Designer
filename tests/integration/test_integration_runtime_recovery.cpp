#include "engine/core/migrate/migration_runner.h"
#include "engine/core/save/rpgmaker_save_reader.h"
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

TEST_CASE("Integration: migration output can be loaded through runtime recovery path", "[integration][save][migrate]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_integration_runtime_recovery";
    std::filesystem::create_directories(base);

    const auto output_path = base / "output.json";
    const auto metadata_path = base / "meta.json";
    const auto variables_path = base / "vars.json";

    nlohmann::json document = nlohmann::json::parse(R"({"_urpg_format_version":"1.0","player":{"atk":12}})");
    const nlohmann::json migration_spec = nlohmann::json::parse(
        R"({"from":"1.0","to":"1.1","ops":[{"op":"rename","fromPath":"/player/atk","toPath":"/player/attack"}]})"
    );

    const auto migration_error = urpg::MigrationRunner::Apply(migration_spec, document);
    REQUIRE_FALSE(migration_error.has_value());
    WriteText(output_path, document.dump());

    WriteText(metadata_path, R"({"_slot_id":2,"_map_display_name":"Integration Map"})");
    WriteText(variables_path, R"({"variables":{"quest_started":true}})");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = output_path;
    request.metadata_path = metadata_path;
    request.variables_path = variables_path;

    const auto runtime_result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(runtime_result.ok);
    REQUIRE_FALSE(runtime_result.loaded_from_recovery);
    REQUIRE(runtime_result.payload.find("\"player\":{\"attack\":12}") != std::string::npos);
    REQUIRE(runtime_result.active_meta.map_display_name == "Integration Map");

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: missing primary save falls back to Level 2 metadata+variables", "[integration][save]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_integration_runtime_l2";
    std::filesystem::create_directories(base);

    const auto metadata_path = base / "meta.json";
    const auto variables_path = base / "vars.json";

    WriteText(metadata_path, R"({"_slot_id":5,"_map_display_name":"Recovery Integration"})");
    WriteText(variables_path, R"({"switches":{"boss":true}})");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = base / "missing_primary.json";
    request.autosave_path = base / "missing_autosave.json";
    request.metadata_path = metadata_path;
    request.variables_path = variables_path;

    const auto runtime_result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(runtime_result.ok);
    REQUIRE(runtime_result.loaded_from_recovery);
    REQUIRE(runtime_result.recovery_tier == urpg::SaveRecoveryTier::Level2MetadataVariables);
    REQUIRE(runtime_result.variables_payload.find("\"boss\":true") != std::string::npos);
    REQUIRE(runtime_result.active_meta.map_display_name == "Recovery Integration");

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: RPG Maker save import loads through native runtime path", "[integration][save][import]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_integration_rpgmaker_import";
    std::filesystem::create_directories(base);

    const auto compat_save_path = base / "slot1.rpgsave";
    const auto native_payload_path = base / "slot1.json";
    const auto native_meta_path = base / "slot1_meta.json";

    WriteText(compat_save_path,
              "N4IgDghgTgLgniAXAbVBAxjA9lAkgEyQEYBfAGjUxwKQCYSBdMkAcywBtDEBWABl+YBbCGBqIAzCSA==");

    const auto read_result = urpg::save::RPGMakerSaveFileReader::readFile(compat_save_path.string());
    REQUIRE(read_result.success);
    REQUIRE(read_result.data["gold"] == 500);
    REQUIRE(read_result.data["mapId"] == 3);

    const auto import_result = urpg::save::ImportCompatSaveDocument(read_result.data);
    REQUIRE(import_result.native_payload["player"]["gold"] == 500);
    REQUIRE(import_result.native_payload["map_id"] == 3);
    REQUIRE(import_result.native_payload["party"].size() == 2);

    WriteText(native_payload_path, import_result.native_payload.dump());
    WriteText(native_meta_path, import_result.migrated_metadata.dump());

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = native_payload_path;
    request.metadata_path = native_meta_path;

    const auto runtime_result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(runtime_result.ok);
    REQUIRE_FALSE(runtime_result.loaded_from_recovery);

    const auto loaded = nlohmann::json::parse(runtime_result.payload);
    REQUIRE(loaded["player"]["gold"] == 500);
    REQUIRE(loaded["map_id"] == 3);
    REQUIRE(loaded["party"].size() == 2);
    REQUIRE(loaded["party"][0]["actor_id"] == 1);
    REQUIRE(loaded["party"][1]["actor_id"] == 2);

    const auto resaved_path = base / "slot1_resaved.json";
    urpg::RuntimeSaveLoadRequest save_request;
    save_request.primary_save_path = resaved_path;
    REQUIRE(urpg::RuntimeSaveLoader::Save(save_request, runtime_result.payload));

    urpg::RuntimeSaveLoadRequest reload_request;
    reload_request.primary_save_path = resaved_path;
    const auto reload_result = urpg::RuntimeSaveLoader::Load(reload_request);
    REQUIRE(reload_result.ok);
    const auto reloaded = nlohmann::json::parse(reload_result.payload);
    REQUIRE(reloaded["player"]["gold"] == 500);
    REQUIRE(reloaded["party"].size() == 2);

    std::filesystem::remove_all(base);
}
