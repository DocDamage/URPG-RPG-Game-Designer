#include "engine/core/ai/behavior_blackboard.h"

#include <algorithm>

namespace urpg::ai {

void BehaviorBlackboard::setBool(const std::string& key, bool value) {
    bools_[key] = value;
}

std::optional<bool> BehaviorBlackboard::boolValue(const std::string& key) const {
    const auto it = bools_.find(key);
    if (it == bools_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void BehaviorBlackboard::setInt(const std::string& key, int32_t value) {
    ints_[key] = value;
}

std::optional<int32_t> BehaviorBlackboard::intValue(const std::string& key) const {
    const auto it = ints_.find(key);
    if (it == ints_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void BehaviorBlackboard::setString(const std::string& key, std::string value) {
    strings_[key] = std::move(value);
}

std::optional<std::string> BehaviorBlackboard::stringValue(const std::string& key) const {
    const auto it = strings_.find(key);
    if (it == strings_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void BehaviorBlackboard::startCooldown(const std::string& key, int32_t ticks) {
    cooldowns_[key] = std::max(0, ticks);
}

void BehaviorBlackboard::tickCooldowns() {
    for (auto& [_, ticks] : cooldowns_) {
        ticks = std::max(0, ticks - 1);
    }
}

bool BehaviorBlackboard::cooldownReady(const std::string& key) const {
    return cooldownRemaining(key) == 0;
}

int32_t BehaviorBlackboard::cooldownRemaining(const std::string& key) const {
    const auto it = cooldowns_.find(key);
    if (it == cooldowns_.end()) {
        return 0;
    }
    return std::max(0, it->second);
}

} // namespace urpg::ai
