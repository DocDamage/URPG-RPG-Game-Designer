#include "gameplay_ability.h"
#include "ability_system_component.h"
#include "runtimes/compat_js/quickjs_runtime.h"

namespace urpg::ability {

bool GameplayAbility::canActivate(const AbilitySystemComponent& source) const {
    const auto& info = getActivationInfo();
    const auto& sourceTags = source.getTags();

    // Required tags check
    if (!sourceTags.hasAllTags(std::vector<GameplayTag>(info.requiredTags.getTags().begin(), info.requiredTags.getTags().end()))) {
        return false;
    }

    // Blocking tags check
    if (sourceTags.hasAnyTags(std::vector<GameplayTag>(info.blockingTags.getTags().begin(), info.blockingTags.getTags().end()))) {
        return false;
    }

    // Cooldown check
    if (source.getCooldownRemaining(this->getId()) > 0.0f) {
        return false;
    }

    // Cost check (MP)
    if (source.getAttribute("MP", 9999.0f) < mpCost) {
        return false;
    }

    // Scripted Condition check
    if (!activeCondition.empty()) {
        // ... (Scripting logic)
    }

    return true;
}

void GameplayAbility::commitAbility(AbilitySystemComponent& source) {
    // Deduct cost
    if (mpCost > 0.0f) {
        source.modifyAttribute("MP", -mpCost);
    }

    // Start cooldown
    const float cooldown = getActivationInfo().cooldownSeconds > 0.0f ? getActivationInfo().cooldownSeconds : cooldownTime;
    if (cooldown > 0.0f) {
        source.setCooldown(this->getId(), cooldown);
    }
}

void GameplayAbility::update(float deltaTime) {
    for (auto it = m_activeTasks.begin(); it != m_activeTasks.end(); ) {
        (*it)->tick(deltaTime);
        if ((*it)->isFinished()) {
            it = m_activeTasks.erase(it);
        } else {
            ++it;
        }
    }
}

bool AbilitySystemComponent::canApplyEffect(const GameplayEffect& effect) const {
    // Basic implementation for canApplyEffect
    // Check if the source has any tags that would block this effect
    // (This is a placeholder for future logic where Effects have their own Tag requirements)
    return true;
}

bool AbilitySystemComponent::canActivateAbility(const GameplayAbility& ability) const {
    return ability.canActivate(*this);
}

bool AbilitySystemComponent::isTargetInPattern(const GameplayAbility& ability, int32_t sourceX, int32_t sourceY, int32_t targetX, int32_t targetY) const {
    const auto& info = ability.getActivationInfo();
    if (!info.pattern) return true; // No pattern means distance and placement aren't restricted by GAF

    int32_t dx = targetX - sourceX;
    int32_t dy = targetY - sourceY;
    
    return info.pattern->hasPoint(dx, dy);
}

} // namespace urpg::ability
