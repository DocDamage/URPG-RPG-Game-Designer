#include "engine/core/map/grid_part_runtime_state.h"

#include <algorithm>

namespace urpg::map {

GridPartInstanceState& GridPartRuntimeState::getOrCreate(const std::string& instance_id) {
    auto [it, inserted] = states_.try_emplace(instance_id);
    if (inserted) {
        it->second.instance_id = instance_id;
    }
    return it->second;
}

const GridPartInstanceState* GridPartRuntimeState::find(const std::string& instance_id) const {
    const auto found = states_.find(instance_id);
    return found == states_.end() ? nullptr : &found->second;
}

bool GridPartRuntimeState::setFlag(const std::string& instance_id, const std::string& key, const std::string& value) {
    if (instance_id.empty() || key.empty()) {
        return false;
    }

    auto& instanceState = getOrCreate(instance_id);
    instanceState.state[key] = value;
    return true;
}

std::string GridPartRuntimeState::getFlag(const std::string& instance_id, const std::string& key,
                                          const std::string& fallback) const {
    const auto* instanceState = find(instance_id);
    if (instanceState == nullptr) {
        return fallback;
    }

    const auto found = instanceState->state.find(key);
    return found == instanceState->state.end() ? fallback : found->second;
}

std::vector<GridPartInstanceState> GridPartRuntimeState::states() const {
    std::vector<GridPartInstanceState> values;
    values.reserve(states_.size());
    for (const auto& [instanceId, state] : states_) {
        (void)instanceId;
        values.push_back(state);
    }

    std::sort(values.begin(), values.end(),
              [](const auto& left, const auto& right) { return left.instance_id < right.instance_id; });
    return values;
}

} // namespace urpg::map
