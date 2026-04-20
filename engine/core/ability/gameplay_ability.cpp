#include "gameplay_ability.h"
#include "ability_system_component.h"
#include "runtimes/compat_js/quickjs_runtime.h"

namespace urpg::ability {

bool GameplayAbility::canActivate(const AbilitySystemComponent& source) const {
    return evaluateActivation(source).allowed;
}

GameplayAbility::ActivationCheckResult GameplayAbility::evaluateActivation(const AbilitySystemComponent& source) const {
    ActivationCheckResult result;
    const auto& info = getActivationInfo();
    const auto& sourceTags = source.getTags();
    const float cooldown_remaining = source.getCooldownRemaining(this->getId());
    const float current_mp = source.getAttribute("MP", 9999.0f);

    result.cooldown_remaining = cooldown_remaining;
    result.current_mp = current_mp;

    // Required tags check
    if (!sourceTags.hasAllTags(std::vector<GameplayTag>(info.requiredTags.getTags().begin(), info.requiredTags.getTags().end()))) {
        result.allowed = false;
        result.reason = "missing_required_tags";
        return result;
    }

    // Blocking tags check
    if (sourceTags.hasAnyTags(std::vector<GameplayTag>(info.blockingTags.getTags().begin(), info.blockingTags.getTags().end()))) {
        result.allowed = false;
        result.reason = "blocking_tags_present";
        return result;
    }

    // Cooldown check
    if (cooldown_remaining > 0.0f) {
        result.allowed = false;
        result.reason = "cooldown_active";
        return result;
    }

    // Cost check (MP)
    if (current_mp < resolveMpCost()) {
        result.allowed = false;
        result.reason = "insufficient_mp";
        return result;
    }

    // Scripted Condition check
    if (!resolveActiveCondition().empty()) {
        // ... (Scripting logic)
        result.allowed = false;
        result.reason = "active_condition_unimplemented";
        return result;
    }

    return result;
}

void GameplayAbility::commitAbility(AbilitySystemComponent& source) {
    // Deduct cost
    const float resolved_mp_cost = resolveMpCost();
    if (resolved_mp_cost > 0.0f) {
        source.modifyAttribute("MP", -resolved_mp_cost);
    }

    // Start cooldown
    const float cooldown = resolveCooldownSeconds();
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

float GameplayAbility::resolveCooldownSeconds() const {
    const auto& info = getActivationInfo();
    return info.cooldownSeconds > 0.0f ? info.cooldownSeconds : cooldownTime;
}

float GameplayAbility::resolveMpCost() const {
    const auto& info = getActivationInfo();
    return info.mpCost > 0 ? static_cast<float>(info.mpCost) : mpCost;
}

const std::string& GameplayAbility::resolveActiveCondition() const {
    const auto& info = getActivationInfo();
    return info.activeCondition.empty() ? activeCondition : info.activeCondition;
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

bool AbilitySystemComponent::tryActivateAbility(GameplayAbility& ability) {
    const auto check = ability.evaluateActivation(*this);
    const float mp_before = getAttribute("MP", defaultBaseAttribute("MP"));

    if (!check.allowed) {
        recordAbilityExecution(
            ability.getId(),
            "activate",
            "blocked",
            check.reason,
            "",
            mp_before,
            mp_before,
            getCooldownRemaining(ability.getId()),
            m_activeEffects.size());
        return false;
    }

    ability.activate(*this);
    const float mp_after = getAttribute("MP", defaultBaseAttribute("MP"));
    recordAbilityExecution(
        ability.getId(),
        "activate",
        "executed",
        "",
        "",
        mp_before,
        mp_after,
        getCooldownRemaining(ability.getId()),
        m_activeEffects.size());
    return true;
}

bool AbilitySystemComponent::isTargetInPattern(const GameplayAbility& ability, int32_t sourceX, int32_t sourceY, int32_t targetX, int32_t targetY) const {
    const auto& info = ability.getActivationInfo();
    if (!info.pattern) return true; // No pattern means distance and placement aren't restricted by GAF

    int32_t dx = targetX - sourceX;
    int32_t dy = targetY - sourceY;
    
    return info.pattern->hasPoint(dx, dy);
}

} // namespace urpg::ability
