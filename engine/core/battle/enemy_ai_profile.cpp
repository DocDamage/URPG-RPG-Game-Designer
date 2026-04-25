#include "engine/core/battle/enemy_ai_profile.h"

#include <algorithm>

namespace urpg::battle {

namespace {

bool hasState(const EnemyAiState& state, const std::string& required_state) {
    return required_state.empty() ||
           std::find(state.states.begin(), state.states.end(), required_state) != state.states.end();
}

} // namespace

std::optional<std::string> ChooseEnemyAiAction(const EnemyAiProfile& profile,
                                               const EnemyAiState& state,
                                               uint32_t seed) {
    std::vector<EnemyAiActionRule> legal;
    for (const auto& action : profile.actions) {
        if (action.weight > 0 && !action.action_id.empty() && hasState(state, action.required_state)) {
            legal.push_back(action);
        }
    }
    std::stable_sort(legal.begin(), legal.end(), [](const auto& a, const auto& b) {
        return a.action_id < b.action_id;
    });
    int32_t total_weight = 0;
    for (const auto& action : legal) {
        total_weight += action.weight;
    }
    if (total_weight <= 0) {
        return std::nullopt;
    }

    int32_t pick = static_cast<int32_t>(seed % static_cast<uint32_t>(total_weight));
    for (const auto& action : legal) {
        if (pick < action.weight) {
            return action.action_id;
        }
        pick -= action.weight;
    }
    return legal.back().action_id;
}

} // namespace urpg::battle
