#include "editor/playtest/playtest_session_controller.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

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
    const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length == buffer.size()) return {};
    buffer.resize(length);
    return std::filesystem::path(buffer);
#else
    std::string buffer(4096, '\0');
    const auto length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0) return {};
    buffer.resize(static_cast<size_t>(length));
    return std::filesystem::path(buffer);
#endif
}

std::filesystem::path temporaryProjectRoot() {
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() / ("urpg_playtest_session_" + std::to_string(nonce));
    std::filesystem::create_directories(root / "content" / "maps");
    return root;
}

} // namespace

TEST_CASE("PlaytestSessionController rejects unsafe map IDs", "[playtest session][editor]") {
    const auto root = temporaryProjectRoot();
    urpg::editor::PlaytestSessionController controller(currentExecutablePath());
    REQUIRE_FALSE(controller.start(root, "../outside", "0,0", "{}", "{}"));
    REQUIRE(controller.state() == urpg::editor::PlaytestSessionState::Crashed);
    REQUIRE(controller.message().find("valid map") != std::string::npos);
    std::filesystem::remove_all(root);
}

TEST_CASE("PlaytestSessionController stages a private current-map overlay", "[playtest session][editor]") {
    const auto root = temporaryProjectRoot();
    {
        urpg::editor::PlaytestSessionController controller(currentExecutablePath());
        REQUIRE(controller.start(root, "starter", "3,4", "{\"grid\":true}\n", "{\"p2d\":true}\n"));
        const auto session = controller.sessionDirectory();
        REQUIRE(session.parent_path() == root / ".urpg" / "playtest");
        REQUIRE(std::filesystem::is_regular_file(session / "content" / "maps" / "starter.grid.json"));
        REQUIRE(std::filesystem::is_regular_file(session / "content" / "maps" / "starter.p2d.json"));
        std::ifstream manifestInput(session / "session.json", std::ios::binary);
        const auto manifest = nlohmann::json::parse(manifestInput);
        REQUIRE(manifest["schema"] == "urpg.playtest_session.v1");
        REQUIRE(manifest["map_id"] == "starter");
        REQUIRE(manifest["spawn"] == "3,4");
        REQUIRE(manifest["diagnostics_path"] == (session / "diagnostics.jsonl").generic_string());
        REQUIRE(controller.mapId() == "starter");
        REQUIRE(controller.spawn() == "3,4");
        REQUIRE(controller.elapsed() >= std::chrono::seconds::zero());
        controller.returnToEditor();
        REQUIRE(controller.state() == urpg::editor::PlaytestSessionState::Returned);
    }
    std::filesystem::remove_all(root);
}
