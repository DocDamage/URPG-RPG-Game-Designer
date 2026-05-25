#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::ai {

enum class BehaviorStatus : uint8_t {
    Success = 0,
    Failure = 1,
    Running = 2,
};

enum class BehaviorNodeKind : uint8_t {
    Sequence = 0,
    Selector = 1,
    Condition = 2,
    Action = 3,
    Cooldown = 4,
};

struct BehaviorTreeNode {
    std::string id;
    BehaviorNodeKind kind = BehaviorNodeKind::Action;
    std::vector<std::string> children;
    std::string key;
    std::string action_id;
    int32_t cooldown_ticks = 0;

    [[nodiscard]] static BehaviorTreeNode sequence(std::string id, std::vector<std::string> children);
    [[nodiscard]] static BehaviorTreeNode selector(std::string id, std::vector<std::string> children);
    [[nodiscard]] static BehaviorTreeNode condition(std::string id, std::string key);
    [[nodiscard]] static BehaviorTreeNode action(std::string id, std::string action_id);
    [[nodiscard]] static BehaviorTreeNode cooldown(std::string id, std::string key, int32_t ticks);
};

struct BehaviorTreeDefinition {
    std::string root;
    std::vector<BehaviorTreeNode> nodes;
};

} // namespace urpg::ai
