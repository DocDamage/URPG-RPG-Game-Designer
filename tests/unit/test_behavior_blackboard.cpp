#include "engine/core/ai/behavior_blackboard.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BehaviorBlackboard stores deterministic scalar values and cooldowns", "[ai][behavior_tree]") {
    urpg::ai::BehaviorBlackboard blackboard;

    blackboard.setBool("can_attack", true);
    blackboard.setInt("hp", 7);
    blackboard.setString("target", "slime_a");
    blackboard.startCooldown("attack", 2);

    REQUIRE(blackboard.boolValue("can_attack").value() == true);
    REQUIRE(blackboard.intValue("hp").value() == 7);
    REQUIRE(blackboard.stringValue("target").value() == "slime_a");
    REQUIRE_FALSE(blackboard.cooldownReady("attack"));
    REQUIRE(blackboard.cooldownRemaining("attack") == 2);

    blackboard.tickCooldowns();
    REQUIRE_FALSE(blackboard.cooldownReady("attack"));
    REQUIRE(blackboard.cooldownRemaining("attack") == 1);

    blackboard.tickCooldowns();
    REQUIRE(blackboard.cooldownReady("attack"));
    REQUIRE(blackboard.cooldownRemaining("attack") == 0);
}
