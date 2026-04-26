#include "engine/core/save/save_recovery.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

void WriteText(const std::filesystem::path& path, const std::string& value) {
    std::ofstream out(path, std::ios::binary);
    out << value;
}

} // namespace

TEST_CASE("Save recovery Level 1 loads autosave payload", "[save][recovery]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_recovery_l1";
    std::filesystem::create_directories(base);

    const auto autosave = base / "autosave.json";
    WriteText(autosave, "{\"state\":\"full\"}");

    urpg::SaveRecoveryRequest request;
    request.autosave_path = autosave;
    request.metadata_path = base / "meta.json";
    request.variables_path = base / "vars.json";

    const auto result = urpg::SaveRecoveryManager::Recover(request);
    REQUIRE(result.ok);
    REQUIRE(result.tier == urpg::SaveRecoveryTier::Level1Autosave);
    REQUIRE(result.full_payload == "{\"state\":\"full\"}");

    std::filesystem::remove_all(base);
}

TEST_CASE("Save recovery Level 2 loads metadata and variables", "[save][recovery]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_recovery_l2";
    std::filesystem::create_directories(base);

    const auto meta = base / "meta.json";
    const auto vars = base / "vars.json";

    WriteText(meta, "{\"_slot_id\":3}");
    WriteText(vars, "{\"switches\":{\"boss\":true}}");

    urpg::SaveRecoveryRequest request;
    request.autosave_path = base / "missing_autosave.json";
    request.metadata_path = meta;
    request.variables_path = vars;

    const auto result = urpg::SaveRecoveryManager::Recover(request);
    REQUIRE(result.ok);
    REQUIRE(result.tier == urpg::SaveRecoveryTier::Level2MetadataVariables);
    REQUIRE(result.metadata_payload == "{\"_slot_id\":3}");
    REQUIRE(result.variables_payload == "{\"switches\":{\"boss\":true}}");

    std::filesystem::remove_all(base);
}

TEST_CASE("Save recovery Level 3 preserves party and resets variables", "[save][recovery]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_recovery_l3";
    std::filesystem::create_directories(base);

    const auto meta = base / "meta.json";
    WriteText(meta,
        "{\"_slot_id\":7,\"_map_display_name\":\"Crimson Keep - B2\",\"_party_snapshot\":[{\"name\":\"Aria\",\"level\":18,\"hp\":340,\"max_hp\":340}]}"
    );

    urpg::SaveRecoveryRequest request;
    request.autosave_path = base / "missing_autosave.json";
    request.metadata_path = meta;
    request.variables_path = base / "missing_vars.json";
    request.safe_mode_fallback_map = "Fallback Origin";

    const auto result = urpg::SaveRecoveryManager::Recover(request);
    REQUIRE(result.ok);
    REQUIRE(result.tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE(result.variables_reset);
    REQUIRE(result.skeleton_meta.map_display_name == "Crimson Keep - B2");
    REQUIRE(result.skeleton_meta.party_snapshot.size() == 1);
    REQUIRE(result.skeleton_meta.party_snapshot[0].name == "Aria");

    std::filesystem::remove_all(base);
}

TEST_CASE("Save recovery Level 3 survives corrupted metadata", "[save][recovery]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_recovery_l3_corrupt";
    std::filesystem::create_directories(base);

    const auto meta = base / "meta.json";
    WriteText(meta, "{ this is not valid json");

    urpg::SaveRecoveryRequest request;
    request.metadata_path = meta;
    request.safe_mode_fallback_map = "Safe Mode - Origin";

    const auto result = urpg::SaveRecoveryManager::Recover(request);
    REQUIRE(result.ok);
    REQUIRE(result.tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE(result.variables_reset);
    REQUIRE(result.skeleton_meta.map_display_name == "Safe Mode - Origin");

    std::filesystem::remove_all(base);
}
