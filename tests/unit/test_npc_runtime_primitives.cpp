#include "engine/core/npc/npc_runtime_primitives.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("NPC runtime primitives use path routing and bounded authored bark text", "[npc][runtime]") {
    urpg::level::PathfindingGraph graph(4, 4);
    urpg::npc::NpcRuntimePrimitives npcs;
    urpg::npc::NpcRuntimeState merchant;
    merchant.npcId = "merchant";
    merchant.mapId = "town";
    REQUIRE(npcs.registerNpc(merchant));
    const auto moved = npcs.moveTo("merchant", graph, {2, 0});
    REQUIRE(moved.success);
    REQUIRE(moved.path.size() == 3);
    REQUIRE(npcs.observeProximity("merchant", {2, 1}, 1).success);
    REQUIRE_FALSE(npcs.observeProximity("merchant", {0, 0}, 1).success);
    REQUIRE_FALSE(npcs.setThought("merchant", std::string(161, 'x')).success);
    REQUIRE(npcs.setThought("merchant", "Welcome.").success);
}
