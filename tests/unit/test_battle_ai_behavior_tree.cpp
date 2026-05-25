#include "engine/core/ai/behavior_tree_runtime.h"
#include "engine/core/battle/battle_ai_bridge.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Battle behavior tree adapter queues selected action when feature flag is enabled",
          "[battle][ai][behavior_tree]") {
    urpg::ai::BehaviorTreeDefinition tree;
    tree.root = "root";
    tree.nodes = {
        urpg::ai::BehaviorTreeNode::selector("root", {"can_skill", "attack"}),
        urpg::ai::BehaviorTreeNode::sequence("can_skill", {"skill_ready", "skill"}),
        urpg::ai::BehaviorTreeNode::condition("skill_ready", "skill_ready"),
        urpg::ai::BehaviorTreeNode::action("skill", "skill:fire"),
        urpg::ai::BehaviorTreeNode::action("attack", "attack"),
    };

    urpg::ai::BehaviorBlackboard blackboard;
    blackboard.setBool("skill_ready", true);

    urpg::battle::BattleActionQueue queue;
    const auto result = urpg::ai::queueBehaviorTreeBattleAction(tree, blackboard, true, "enemy_1", "actor_1", queue);

    REQUIRE(result.enabled);
    REQUIRE(result.tick.status == urpg::ai::BehaviorStatus::Success);
    REQUIRE(result.tick.selected_action_id == "skill:fire");
    REQUIRE(queue.size() == 1);

    const auto action = queue.popNext();
    REQUIRE(action.has_value());
    REQUIRE(action->subject_id == "enemy_1");
    REQUIRE(action->target_id == "actor_1");
    REQUIRE(action->command == "skill:fire");
}
