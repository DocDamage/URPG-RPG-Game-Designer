#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::battle {

struct EnemyAiActionRule {
    std::string action_id;
    int32_t weight = 0;
    std::string required_state;
};

struct EnemyAiProfile {
    std::string id;
    std::vector<EnemyAiActionRule> actions;
};

struct EnemyAiState {
    std::vector<std::string> states;
};

std::optional<std::string> ChooseEnemyAiAction(const EnemyAiProfile& profile,
                                               const EnemyAiState& state,
                                               uint32_t seed);

} // namespace urpg::battle
