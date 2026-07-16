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
        REQUIRE(controller.start(root, "starter", "3,4", "{\"grid\":true}\n", "{\"p2d\":true}\n",
                                 "starter:event.guide"));
        const auto session = controller.sessionDirectory();
        REQUIRE(session.parent_path() == root / ".urpg" / "playtest");
        REQUIRE(std::filesystem::is_regular_file(session / "content" / "maps" / "starter.grid.json"));
        REQUIRE(std::filesystem::is_regular_file(session / "content" / "maps" / "starter.p2d.json"));
        std::ifstream manifestInput(session / "session.json", std::ios::binary);
        const auto manifest = nlohmann::json::parse(manifestInput);
        REQUIRE(manifest["schema"] == "urpg.playtest_session.v1");
        REQUIRE(manifest["map_id"] == "starter");
        REQUIRE(manifest["spawn"] == "3,4");
        REQUIRE(manifest["selected_object_id"] == "starter:event.guide");
        REQUIRE(manifest["checkpoint"]["id"] == "launch");
        REQUIRE(manifest["checkpoint"]["disposable_overlay"] == true);
        REQUIRE(manifest["diagnostics_path"] == (session / "diagnostics.jsonl").generic_string());
        REQUIRE(controller.mapId() == "starter");
        REQUIRE(controller.spawn() == "3,4");
        REQUIRE(controller.elapsed() >= std::chrono::seconds::zero());
        controller.returnToEditor();
        REQUIRE(controller.state() == urpg::editor::PlaytestSessionState::Returned);
        REQUIRE(controller.lastReturnContext().valid);
        REQUIRE(controller.lastReturnContext().map_id == "starter");
        REQUIRE(controller.lastReturnContext().spawn == "3,4");
        REQUIRE(controller.lastReturnContext().selected_object_id == "starter:event.guide");
        REQUIRE(controller.lastReturnContext().checkpoint_id == "launch");

        const auto support = controller.writeRedactedSupportBundle();
        REQUIRE(support.success);
        REQUIRE(support.diagnostic_count == 0);
        std::ifstream supportInput(support.path, std::ios::binary);
        const auto supportJson = nlohmann::json::parse(supportInput);
        REQUIRE(supportJson["schema"] == "urpg.playtest_support_summary.v1");
        REQUIRE(supportJson["session_state"] == "returned");
        REQUIRE(supportJson["map_id"] == "starter");
        REQUIRE(supportJson["redaction"]["project_paths"] == "omitted");
        REQUIRE(supportJson["redaction"]["session_paths"] == "omitted");
        REQUIRE(supportJson["redaction"]["process_output"] == "omitted");
        REQUIRE(supportJson.dump().find(root.generic_string()) == std::string::npos);
        REQUIRE(supportJson.dump().find(session.generic_string()) == std::string::npos);
    }
    std::filesystem::remove_all(root);
}

TEST_CASE("Playtest spatial bridge preserves selection through play teleport diagnostic and return",
          "[playtest session][editor][spatial_bridge]") {
    const auto root = temporaryProjectRoot();
    {
        urpg::editor::PlaytestSessionController controller(currentExecutablePath());
        REQUIRE(controller.startFromHere(root,"starter",2,3,"{\"grid\":true}\n","{\"p2d\":true}\n","event.guide"));
        REQUIRE(controller.spawn()=="2,3"); REQUIRE(controller.selectedObjectId()=="event.guide");
        REQUIRE(controller.teleportHere("starter",6,7,"event.vendor"));
        REQUIRE(controller.spawn()=="6,7"); REQUIRE(controller.selectedObjectId()=="event.vendor");
        std::ifstream commands(controller.sessionDirectory()/"editor_commands.jsonl"); nlohmann::json command; commands>>command;
        REQUIRE(command["command"]=="teleport_here"); REQUIRE(command["map_id"]=="starter"); REQUIRE(command["tile_x"]==6); REQUIRE(command["tile_y"]==7);
        urpg::diagnostics::RuntimeDiagnostic diagnostic; diagnostic.code="collision_bad"; diagnostic.map_id="starter"; diagnostic.object_id="tile:4,5";
        controller.returnToDiagnostic(diagnostic,4,5);
        REQUIRE(controller.state()==urpg::editor::PlaytestSessionState::Returned);
        REQUIRE(controller.lastReturnContext().map_id=="starter"); REQUIRE(controller.lastReturnContext().spawn=="4,5");
        REQUIRE(controller.lastReturnContext().selected_object_id=="tile:4,5"); REQUIRE(controller.lastReturnContext().checkpoint_id=="diagnostic:collision_bad");
    }
    std::filesystem::remove_all(root);
}
