#include "engine/core/achievement/achievement_trigger.h"
#include "engine/core/achievement/achievement_registry.h"

#include <algorithm>
#include <sstream>

namespace urpg::achievement {

// --- AchievementTrigger ---

std::optional<uint32_t> AchievementTrigger::parseLegacyTarget(const std::string& condition) {
    auto pos = condition.rfind('_');
    if (pos == std::string::npos || pos + 1 >= condition.size()) {
        return std::nullopt;
    }
    try {
        return static_cast<uint32_t>(std::stoul(condition.substr(pos + 1)));
    } catch (...) {
        return std::nullopt;
    }
}

AchievementTrigger AchievementTrigger::parse(const std::string& condition) {
    AchievementTrigger trigger;
    if (condition.empty()) {
        return trigger;
    }

    // Check for parameterized format: "event_type:key1=val1:key2=val2:count=N"
    auto firstColon = condition.find(':');
    if (firstColon != std::string::npos) {
        trigger.eventType = condition.substr(0, firstColon);

        std::string rest = condition.substr(firstColon + 1);
        std::stringstream ss(rest);
        std::string token;
        while (std::getline(ss, token, ':')) {
            auto eqPos = token.find('=');
            if (eqPos != std::string::npos) {
                std::string key = token.substr(0, eqPos);
                std::string value = token.substr(eqPos + 1);
                if (key == "count") {
                    try {
                        trigger.target = static_cast<uint32_t>(std::stoul(value));
                    } catch (...) {
                        trigger.target = 1;
                    }
                } else {
                    trigger.params[key] = value;
                }
            }
        }
    } else {
        // Legacy format: "event_type_10" or "battle_victory"
        if (auto targetOpt = parseLegacyTarget(condition)) {
            trigger.target = *targetOpt;
            trigger.eventType = condition.substr(0, condition.rfind('_'));
        } else {
            trigger.eventType = condition;
            trigger.target = 1;
        }
    }

    return trigger;
}

bool AchievementTrigger::matches(const std::string& eventType,
                                 const std::unordered_map<std::string, std::string>& eventParams) const {
    if (this->eventType != eventType) {
        return false;
    }

    // All trigger parameters must be present and equal in the event parameters
    for (const auto& [key, value] : this->params) {
        auto it = eventParams.find(key);
        if (it == eventParams.end() || it->second != value) {
            return false;
        }
    }

    return true;
}

// --- AchievementEventBus ---

void AchievementEventBus::addListener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

void AchievementEventBus::emit(const AchievementEvent& event) {
    for (const auto& listener : listeners_) {
        listener(event);
    }
}

// --- AchievementTriggerResolver ---

void AchievementTriggerResolver::bindRegistry(AchievementRegistry* registry) {
    registry_ = registry;
}

void AchievementTriggerResolver::bindEventBus(AchievementEventBus* bus) {
    if (bus_) {
        // No unsubscription needed; listeners_ is append-only in this simple bus.
        // Rebinding is supported by just updating the pointer.
    }
    bus_ = bus;
    bus_->addListener([this](const AchievementEvent& event) { onEvent(event); });
}

void AchievementTriggerResolver::refreshAll() {
    if (!registry_) {
        return;
    }
    // Re-evaluate all registered achievements against their own triggers.
    // This is primarily useful after loading save data to ensure consistency.
    for (const auto& def : registry_->getAllAchievements()) {
        auto trigger = AchievementTrigger::parse(def.unlockCondition);
        auto progress = registry_->getProgress(def.id);
        if (progress && progress->unlocked && trigger.target > 0 && progress->current < trigger.target) {
            // Defensive: if save says unlocked but progress is short, report remaining
            registry_->reportProgress(def.id, trigger.target - progress->current);
        }
    }
}

void AchievementTriggerResolver::onEvent(const AchievementEvent& event) {
    if (!registry_) {
        return;
    }

    for (const auto& def : registry_->getAllAchievements()) {
        auto trigger = AchievementTrigger::parse(def.unlockCondition);
        if (trigger.matches(event.eventType, event.params)) {
            registry_->reportProgress(def.id, event.increment);
        }
    }
}

} // namespace urpg::achievement
