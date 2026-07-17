#include "engine/core/playtest/playtest_command_bridge.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <vector>

namespace {
std::filesystem::path temporarySession() {
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    auto path = std::filesystem::temp_directory_path() / ("urpg_playtest_bridge_" + std::to_string(nonce));
    std::filesystem::create_directories(path);
    return path;
}
}

TEST_CASE("Playtest command bridge consumes bounded teleports once and acknowledges every line",
          "[playtest][runtime_bridge]") {
    const auto session = temporarySession();
    {
        std::ofstream output(session / "editor_commands.jsonl", std::ios::binary);
        output << nlohmann::json{{"version",1},{"command_id",1},{"command","teleport_here"},
                                 {"map_id","town"},{"tile_x",4},{"tile_y",5},
                                 {"selected_object_id","event.vendor"}}.dump() << '\n';
        output << "{not-json}\n";
        output << nlohmann::json{{"version",1},{"command_id",2},{"command","teleport_here"},
                                 {"map_id","forest"},{"tile_x",1},{"tile_y",2}}.dump() << '\n';
    }
    urpg::playtest::PlaytestCommandBridge bridge(session);
    std::vector<urpg::playtest::EditorCommand> applied;
    const auto first = bridge.poll([&](const auto& command) { applied.push_back(command); return true; }, 2);
    REQUIRE(first.processed == 2);
    REQUIRE(first.applied == 1);
    REQUIRE(first.rejected == 1);
    const auto second = bridge.poll([&](const auto& command) { applied.push_back(command); return true; }, 2);
    REQUIRE(second.processed == 1);
    REQUIRE(second.applied == 1);
    REQUIRE(applied.size() == 2);
    REQUIRE(applied[1].map_id == "forest");
    REQUIRE(bridge.poll([](const auto&) { return true; }).processed == 0);
    std::ifstream acknowledgements(session / "runtime_acknowledgements.jsonl", std::ios::binary);
    const std::string text{std::istreambuf_iterator<char>(acknowledgements), std::istreambuf_iterator<char>()};
    REQUIRE(std::count(text.begin(), text.end(), '\n') == 3);
    acknowledgements.close();
    std::filesystem::remove_all(session);
}

TEST_CASE("Playtest command bridge admits only revisioned reset-affected Map reloads",
          "[playtest][runtime_bridge][hot_reload]") {
    const auto session = temporarySession();
    {
        std::ofstream output(session / "editor_commands.jsonl", std::ios::binary);
        output << nlohmann::json{{"version",1},{"command_id",1},{"command","hot_reload_map"},
                                 {"map_id","town"},{"expected_revision",0},
                                 {"state_policy","reset_affected"}}.dump() << '\n';
        output << nlohmann::json{{"version",1},{"command_id",2},{"command","hot_reload_map"},
                                 {"map_id","town"},{"expected_revision",1},
                                 {"state_policy","preserve"}}.dump() << '\n';
    }
    urpg::playtest::PlaytestCommandBridge bridge(session);
    std::vector<urpg::playtest::EditorCommand> applied;
    const auto result = bridge.poll([&](const auto& command) { applied.push_back(command); return true; });
    REQUIRE(result.processed == 2);
    REQUIRE(result.applied == 1);
    REQUIRE(result.rejected == 1);
    REQUIRE(applied.front().expected_revision == 0);
    REQUIRE(applied.front().state_policy == "reset_affected");
    std::filesystem::remove_all(session);
}
