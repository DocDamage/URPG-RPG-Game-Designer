#include "engine/core/migrate/migration_runner.h"
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
