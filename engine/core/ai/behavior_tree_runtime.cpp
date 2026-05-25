#include "engine/core/ai/behavior_tree_runtime.h"

#include <algorithm>

namespace urpg::ai {

BehaviorTreeNode BehaviorTreeNode::sequence(std::string id, std::vector<std::string> children) {
    BehaviorTreeNode node;
    node.id = std::move(id);
    node.kind = BehaviorNodeKind::Sequence;
    node.children = std::move(children);
    return node;
}

BehaviorTreeNode BehaviorTreeNode::selector(std::string id, std::vector<std::string> children) {
    BehaviorTreeNode node;
    node.id = std::move(id);
    node.kind = BehaviorNodeKind::Selector;
    node.children = std::move(children);
    return node;
}

BehaviorTreeNode BehaviorTreeNode::condition(std::string id, std::string key) {
    BehaviorTreeNode node;
    node.id = std::move(id);
    node.kind = BehaviorNodeKind::Condition;
    node.key = std::move(key);
    return node;
}

BehaviorTreeNode BehaviorTreeNode::action(std::string id, std::string action_id) {
    BehaviorTreeNode node;
    node.id = std::move(id);
    node.kind = BehaviorNodeKind::Action;
    node.action_id = std::move(action_id);
    return node;
}

BehaviorTreeNode BehaviorTreeNode::cooldown(std::string id, std::string key, int32_t ticks) {
    BehaviorTreeNode node;
    node.id = std::move(id);
    node.kind = BehaviorNodeKind::Cooldown;
    node.key = std::move(key);
    node.cooldown_ticks = ticks;
    return node;
}

void BehaviorTreeRuntime::setActionExecutor(ActionExecutor executor) {
    action_executor_ = std::move(executor);
}

BehaviorTreeTickResult BehaviorTreeRuntime::tick(const BehaviorTreeDefinition& tree,
                                                 BehaviorBlackboard& blackboard) const {
    BehaviorTreeTickResult result;
    if (tree.root.empty()) {
        result.diagnostics.push_back("behavior_tree.missing_root");
        return result;
    }

    result.status = evaluateNode(tree, tree.root, blackboard, result);
    return result;
}

BehaviorStatus BehaviorTreeRuntime::evaluateNode(const BehaviorTreeDefinition& tree, const std::string& node_id,
                                                 BehaviorBlackboard& blackboard, BehaviorTreeTickResult& result) const {
    const auto it = std::find_if(tree.nodes.begin(), tree.nodes.end(),
                                 [&](const BehaviorTreeNode& node) { return node.id == node_id; });
    if (it == tree.nodes.end()) {
        result.diagnostics.push_back("behavior_tree.missing_node:" + node_id);
        return BehaviorStatus::Failure;
    }

    const BehaviorTreeNode& node = *it;
    result.visited_node_ids.push_back(node.id);

    switch (node.kind) {
    case BehaviorNodeKind::Sequence:
        for (const auto& child : node.children) {
            const auto status = evaluateNode(tree, child, blackboard, result);
            if (status != BehaviorStatus::Success) {
                return status;
            }
        }
        return BehaviorStatus::Success;

    case BehaviorNodeKind::Selector:
        for (const auto& child : node.children) {
            const auto status = evaluateNode(tree, child, blackboard, result);
            if (status != BehaviorStatus::Failure) {
                return status;
            }
        }
        return BehaviorStatus::Failure;

    case BehaviorNodeKind::Condition:
        return blackboard.boolValue(node.key).value_or(false) ? BehaviorStatus::Success : BehaviorStatus::Failure;

    case BehaviorNodeKind::Action: {
        const auto status = action_executor_ ? action_executor_(node.action_id, blackboard) : BehaviorStatus::Success;
        if (status == BehaviorStatus::Success || status == BehaviorStatus::Running) {
            result.selected_action_id = node.action_id;
        }
        return status;
    }

    case BehaviorNodeKind::Cooldown:
        if (!blackboard.cooldownReady(node.key)) {
            return BehaviorStatus::Failure;
        }
        blackboard.startCooldown(node.key, node.cooldown_ticks);
        return BehaviorStatus::Success;
    }

    return BehaviorStatus::Failure;
}

} // namespace urpg::ai
