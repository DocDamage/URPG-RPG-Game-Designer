#pragma once

#include "engine/core/ai/behavior_blackboard.h"
#include "engine/core/ai/behavior_tree_node.h"

#include <functional>
#include <string>
#include <vector>

namespace urpg::ai {

struct BehaviorTreeTickResult {
    BehaviorStatus status = BehaviorStatus::Failure;
    std::string selected_action_id;
    std::vector<std::string> visited_node_ids;
    std::vector<std::string> diagnostics;
};

class BehaviorTreeRuntime {
  public:
    using ActionExecutor = std::function<BehaviorStatus(const std::string&, BehaviorBlackboard&)>;

    void setActionExecutor(ActionExecutor executor);
    [[nodiscard]] BehaviorTreeTickResult tick(const BehaviorTreeDefinition& tree, BehaviorBlackboard& blackboard) const;

  private:
    BehaviorStatus evaluateNode(const BehaviorTreeDefinition& tree, const std::string& node_id,
                                BehaviorBlackboard& blackboard, BehaviorTreeTickResult& result) const;

    ActionExecutor action_executor_;
};

} // namespace urpg::ai
