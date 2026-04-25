#include "engine/core/npc/npc_schedule.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("NPC routine fallback activates when map is missing", "[npc][simulation][ffs13]") {
    urpg::npc::NpcSchedule schedule;
    schedule.addRoutine({"npc.merchant", "market", 8, 17, {4, 6}, "idle", "dialogue.shop"});
    schedule.setFallback({"home", {1, 1}, "wait", "dialogue.closed"});

    const auto state = schedule.resolve("npc.merchant", 10, {"home"});

    REQUIRE(state.usedFallback);
    REQUIRE(state.mapId == "home");
    REQUIRE(state.dialogueState == "dialogue.closed");
}
