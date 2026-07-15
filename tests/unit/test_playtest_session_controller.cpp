#include "editor/playtest/playtest_session_controller.h"
#include "engine/core/platform/process_runner.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

std::filesystem::path currentExecutablePath() {
#ifdef _WIN32
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (size == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    buffer.resize(size);
    return std::filesystem::path(buffer);
#else
    std::string buffer(4096, '\0');
    const ssize_t size = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    REQUIRE(size > 0);
    buffer.resize(static_cast<size_t>(size));
    return std::filesystem::path(buffer);
#endif
}

} // namespace

TEST_CASE("playtest session controller handles lifecycle and manifests", "[playtest_session_controller][editor]") {
    const auto tempDir = std::filesystem::temp_directory_path() / "urpg_playtest_controller_test";
    std::error_code ec;
    std::filesystem::remove_all(tempDir, ec);
    std::filesystem::create_directories(tempDir, ec);

    urpg::editor::PlaytestSessionController controller(currentExecutablePath());

    SECTION("Initial state is inactive") {
        REQUIRE(controller.state() == urpg::editor::PlaytestSessionState::Inactive);
        REQUIRE(controller.diagnostics().empty());
    }

    SECTION("Overlay files and manifest are written on startSession") {
        // Inject the test executable so staging does not depend on a separately built runtime.
        const std::string gridJson = "{\"mapId\": \"test_map\", \"width\": 16, \"height\": 12, \"parts\": []}";

        const auto freshSession = tempDir / ".urpg" / "playtest" / "fresh_other_session";
        std::filesystem::create_directories(freshSession);

        // Start a headless child and verify the staged filesystem contract.
        const bool started = controller.startSession(tempDir, "test_map", "2,3", true, gridJson, "");
        REQUIRE(started);

        // Verify overlay files staged
        std::filesystem::path sessionManifestPath = tempDir / ".urpg" / "playtest" / controller.sessionId() / "session.json";
        std::filesystem::path gridOverlayPath = tempDir / ".urpg" / "playtest" / controller.sessionId() / "content" / "maps" / "test_map.grid.json";

        REQUIRE(std::filesystem::exists(sessionManifestPath));
        REQUIRE(std::filesystem::exists(gridOverlayPath));

        // Verify session manifest contains the correct values
        std::ifstream in(sessionManifestPath);
        nlohmann::json manifest = nlohmann::json::parse(in);
        REQUIRE(manifest["map_id"] == "test_map");
        REQUIRE(manifest["spawn"] == "2,3");
        REQUIRE(manifest["project_root"] == tempDir.generic_string());
        REQUIRE(manifest["schema_version"] == "urpg.playtest_session.v1");
        REQUIRE(std::filesystem::is_directory(freshSession));

        controller.stopSession();
    }

    SECTION("Diagnostics polling parses JSONL correctly") {
        REQUIRE(controller.startSession(tempDir, "test_map", "2,3", true, "", ""));
        std::filesystem::path diagFile = controller.sessionDir() / "diagnostics.jsonl";

        std::ofstream out(diagFile);
        nlohmann::json log1 = {
            {"version", 1},
            {"severity", "warning"},
            {"subsystem", "test_sub"},
            {"code", "test_code"},
            {"message", "warning message"},
            {"map_id", "test_map"},
            {"object_id", "obj_123"},
            {"source_file", "main.js"},
            {"timestamp", 1234567890LL}
        };
        out << log1.dump() << "\n";
        out.close();

        // Trigger updates to process diagnostics
        controller.update();

        const auto& diags = controller.diagnostics();
        REQUIRE(diags.size() == 1);
        REQUIRE(diags[0].subsystem == "test_sub");
        REQUIRE(diags[0].code == "test_code");
        REQUIRE(diags[0].message == "warning message");
        REQUIRE(diags[0].object_id == "obj_123");
        REQUIRE(diags[0].severity == urpg::diagnostics::DiagnosticSeverity::Warning);

        controller.stopSession();
    }

    SECTION("triggerReload writes reload request JSON") {
        REQUIRE(controller.startSession(tempDir, "test_map", "2,3", true, "", ""));

        std::vector<urpg::editor::PlaytestReloadResource> resources = {
            {"test_map", "map", "content/maps/test_map.grid.json"}
        };

        controller.triggerReload(resources);
        const auto requestPath = controller.sessionDir() / "reload_request.json";
        REQUIRE(std::filesystem::is_regular_file(requestPath));
        std::ifstream requestInput(requestPath);
        const auto request = nlohmann::json::parse(requestInput);
        REQUIRE(request["version"] == 1);
        REQUIRE(request["resources"].size() == 1);
        REQUIRE(request["resources"][0]["id"] == "test_map");
        controller.stopSession();
    }

    SECTION("unsafe map identifiers are rejected before staging") {
        REQUIRE_FALSE(controller.startSession(tempDir, "../outside", "2,3", true, "{}", ""));
        REQUIRE(controller.state() == urpg::editor::PlaytestSessionState::Crashed);
        REQUIRE_FALSE(std::filesystem::exists(tempDir.parent_path() / "outside.grid.json"));
    }

    std::filesystem::remove_all(tempDir, ec);
}
