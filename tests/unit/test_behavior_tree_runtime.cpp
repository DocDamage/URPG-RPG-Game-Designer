#include "engine/core/ai/behavior_tree_runtime.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("BehaviorTreeRuntime executes sequence nodes deterministically", "[ai][behavior_tree]") {
    urpg::ai::BehaviorTreeDefinition tree;
    tree.root = "root";
    tree.nodes = {
        urpg::ai::BehaviorTreeNode::sequence("root", {"can_attack", "attack"}),
        urpg::ai::BehaviorTreeNode::condition("can_attack", "can_attack"),
        urpg::ai::BehaviorTreeNode::action("attack", "attack"),
    };

    urpg::ai::BehaviorBlackboard blackboard;
    blackboard.setBool("can_attack", true);

    std::vector<std::string> actions;
    urpg::ai::BehaviorTreeRuntime runtime;
    runtime.setActionExecutor([&](const std::string& action_id, urpg::ai::BehaviorBlackboard&) {
        actions.push_back(action_id);
        return urpg::ai::BehaviorStatus::Success;
    });

    const auto first = runtime.tick(tree, blackboard);
    const auto second = runtime.tick(tree, blackboard);

    REQUIRE(first.status == urpg::ai::BehaviorStatus::Success);
    REQUIRE(second.status == urpg::ai::BehaviorStatus::Success);
    REQUIRE(actions == std::vector<std::string>{"attack", "attack"});
    REQUIRE(first.visited_node_ids == second.visited_node_ids);
    REQUIRE(first.visited_node_ids == std::vector<std::string>{"root", "can_attack", "attack"});
}

TEST_CASE("BehaviorTreeRuntime selector falls back and cooldown gates actions", "[ai][behavior_tree]") {
    urpg::ai::BehaviorTreeDefinition tree;
    tree.root = "root";
    tree.nodes = {
        urpg::ai::BehaviorTreeNode::selector("root", {"special_branch", "basic_attack"}),
        urpg::ai::BehaviorTreeNode::sequence("special_branch", {"special_ready", "special_action"}),
        urpg::ai::BehaviorTreeNode::cooldown("special_ready", "special", 2),
        urpg::ai::BehaviorTreeNode::action("special_action", "special"),
        urpg::ai::BehaviorTreeNode::action("basic_attack", "attack"),
    };

    urpg::ai::BehaviorBlackboard blackboard;
    urpg::ai::BehaviorTreeRuntime runtime;
    runtime.setActionExecutor(
        [](const std::string&, urpg::ai::BehaviorBlackboard&) { return urpg::ai::BehaviorStatus::Success; });

    const auto first = runtime.tick(tree, blackboard);
    const auto second = runtime.tick(tree, blackboard);
    blackboard.tickCooldowns();
    blackboard.tickCooldowns();
    const auto third = runtime.tick(tree, blackboard);

    REQUIRE(first.status == urpg::ai::BehaviorStatus::Success);
    REQUIRE(first.selected_action_id == "special");
    REQUIRE_FALSE(blackboard.cooldownReady("special"));

    REQUIRE(second.status == urpg::ai::BehaviorStatus::Success);
    REQUIRE(second.selected_action_id == "attack");

    REQUIRE(third.status == urpg::ai::BehaviorStatus::Success);
    REQUIRE(third.selected_action_id == "special");
}
