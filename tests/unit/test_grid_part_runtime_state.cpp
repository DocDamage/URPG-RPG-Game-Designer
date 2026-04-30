#include "engine/core/map/grid_part_runtime_state.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

using namespace urpg::map;

namespace {

nlohmann::json loadJson(const std::string& relativePath) {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/" + relativePath);
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

} // namespace

TEST_CASE("Grid part runtime state creates state by stable instance id", "[grid_part][runtime_state]") {
    GridPartRuntimeState runtimeState;

    auto& chest = runtimeState.getOrCreate("map001:treasure.chest:1:2");
    chest.consumed = true;
    chest.state["opened"] = "true";

    auto& sameChest = runtimeState.getOrCreate("map001:treasure.chest:1:2");
    REQUIRE(&sameChest == &chest);
    REQUIRE(sameChest.instance_id == "map001:treasure.chest:1:2");
    REQUIRE(sameChest.consumed);
    REQUIRE(sameChest.state.at("opened") == "true");
}

TEST_CASE("Grid part runtime state find does not create missing state", "[grid_part][runtime_state]") {
    GridPartRuntimeState runtimeState;

    REQUIRE(runtimeState.find("map001:door.locked:3:4") == nullptr);
    REQUIRE(runtimeState.states().empty());

    runtimeState.getOrCreate("map001:door.locked:3:4");
    REQUIRE(runtimeState.find("map001:door.locked:3:4") != nullptr);
    REQUIRE(runtimeState.states().size() == 1);
}

TEST_CASE("Grid part runtime state sets and gets instance flags", "[grid_part][runtime_state]") {
    GridPartRuntimeState runtimeState;

    REQUIRE(runtimeState.setFlag("map001:door.locked:3:4", "locked", "false"));
    REQUIRE(runtimeState.getFlag("map001:door.locked:3:4", "locked") == "false");
    REQUIRE(runtimeState.getFlag("map001:door.locked:3:4", "open", "false") == "false");
    REQUIRE(runtimeState.getFlag("map001:missing:0:0", "locked", "unknown") == "unknown");
}

TEST_CASE("Grid part runtime state rejects empty flag identity and keys", "[grid_part][runtime_state]") {
    GridPartRuntimeState runtimeState;

    REQUIRE_FALSE(runtimeState.setFlag("", "locked", "false"));
    REQUIRE_FALSE(runtimeState.setFlag("map001:door.locked:3:4", "", "false"));
    REQUIRE(runtimeState.states().empty());
}

TEST_CASE("Grid part runtime state returns deterministic state ordering", "[grid_part][runtime_state]") {
    GridPartRuntimeState runtimeState;
    runtimeState.getOrCreate("map001:z:1:1");
    runtimeState.getOrCreate("map001:a:1:1");
    runtimeState.getOrCreate("map001:m:1:1");

    const auto states = runtimeState.states();

    REQUIRE(states.size() == 3);
    REQUIRE(states[0].instance_id == "map001:a:1:1");
    REQUIRE(states[1].instance_id == "map001:m:1:1");
    REQUIRE(states[2].instance_id == "map001:z:1:1");
}

TEST_CASE("Grid part runtime state schema is parseable", "[grid_part][runtime_state][schema]") {
    const auto schema = loadJson("content/schemas/grid_part_runtime_state.schema.json");

    REQUIRE(schema["title"] == "Grid Part Runtime State");
    REQUIRE(schema["required"].is_array());
    REQUIRE(schema["properties"].contains("states"));
}
