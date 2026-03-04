#include "engine/core/save/save_runtime.h"

#include <catch2/catch_test_macros.hpp>

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
