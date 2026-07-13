#include "engine/core/diagnostics/runtime_diagnostics.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

TEST_CASE("runtime diagnostics appends bounded JSONL records", "[runtime_diagnostics][diagnostics]") {
    const auto tempDiagFile = std::filesystem::temp_directory_path() / "test_diagnostics.jsonl";
    std::error_code ec;
    std::filesystem::remove(tempDiagFile, ec);

    urpg::diagnostics::g_DiagnosticsFilePath = tempDiagFile;
    urpg::diagnostics::g_ActiveMapId = "test_active_map";

    // Clear snapshot
    urpg::diagnostics::RuntimeDiagnostics::clear();

    // Emit warning
    urpg::diagnostics::RuntimeDiagnostics::warning("subsystem_a", "code_1", "message_1", "", "obj_2", "file_c");

    // Verify snapshot
    const auto snapshot = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(snapshot.size() == 1);
    REQUIRE(snapshot[0].code == "code_1");
    REQUIRE(snapshot[0].map_id == "test_active_map");
    REQUIRE(snapshot[0].object_id == "obj_2");

    // Verify file output
    REQUIRE(std::filesystem::exists(tempDiagFile));
    std::ifstream in(tempDiagFile);
    std::string line;
    REQUIRE(std::getline(in, line));

    nlohmann::json j = nlohmann::json::parse(line);
    REQUIRE(j["version"] == 1);
    REQUIRE(j["severity"] == "warning");
    REQUIRE(j["subsystem"] == "subsystem_a");
    REQUIRE(j["code"] == "code_1");
    REQUIRE(j["message"] == "message_1");
    REQUIRE(j["map_id"] == "test_active_map");
    REQUIRE(j["object_id"] == "obj_2");
    REQUIRE(j["source_file"] == "file_c");
    REQUIRE(j["timestamp"].is_number());

    urpg::diagnostics::g_DiagnosticsFilePath.clear();
    for (size_t index = 0; index < urpg::diagnostics::RuntimeDiagnostics::kMaxRetainedEntries + 25; ++index) {
        urpg::diagnostics::RuntimeDiagnostics::info("bounded", "entry", std::to_string(index));
    }
    const auto boundedSnapshot = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(boundedSnapshot.size() == urpg::diagnostics::RuntimeDiagnostics::kMaxRetainedEntries);
    REQUIRE(boundedSnapshot.front().message == "25");

    // Clean up
    urpg::diagnostics::RuntimeDiagnostics::clear();
    urpg::diagnostics::g_ActiveMapId.clear();
    std::filesystem::remove(tempDiagFile, ec);
}
