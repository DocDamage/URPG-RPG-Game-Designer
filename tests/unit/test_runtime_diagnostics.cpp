#include "engine/core/diagnostics/runtime_diagnostics.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

TEST_CASE("RuntimeDiagnostics writes a versioned playtest JSONL record", "[runtime diagnostics][playtest]") {
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() / ("urpg_runtime_diagnostics_" + std::to_string(nonce) + ".jsonl");
    const auto previousPath = urpg::diagnostics::g_DiagnosticsFilePath;
    const auto previousMap = urpg::diagnostics::g_ActiveMapId;
    urpg::diagnostics::g_DiagnosticsFilePath = path;
    urpg::diagnostics::g_ActiveMapId = "starter";
    urpg::diagnostics::RuntimeDiagnostics::clear();

    urpg::diagnostics::RuntimeDiagnostics::warning(
        "runtime.playtest", "playtest.example", "Example diagnostic", {}, "event_01", "content/maps/starter.grid.json");

    const auto entries = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(entries.size() == 1);
    REQUIRE(entries.front().map_id == "starter");
    REQUIRE(entries.front().object_id == "event_01");
    REQUIRE(entries.front().timestamp > 0);

    std::ifstream input(path, std::ios::binary);
    const auto record = nlohmann::json::parse(input);
    REQUIRE(record["version"] == 1);
    REQUIRE(record["severity"] == "warning");
    REQUIRE(record["map_id"] == "starter");
    REQUIRE(record["object_id"] == "event_01");
    REQUIRE(record["source_file"] == "content/maps/starter.grid.json");

    urpg::diagnostics::RuntimeDiagnostics::clear();
    urpg::diagnostics::g_DiagnosticsFilePath = previousPath;
    urpg::diagnostics::g_ActiveMapId = previousMap;
    std::filesystem::remove(path);
}
