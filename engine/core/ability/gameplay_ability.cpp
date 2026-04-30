#include "gameplay_ability.h"
#include "ability_condition_evaluator.h"
#include "ability_system_component.h"

namespace urpg::ability {
namespace {

GameplayAbility::ActivationCheckResult
evaluateActivationWithContext(const GameplayAbility& ability, const AbilitySystemComponent& source,
                              const GameplayAbility::AbilityExecutionContext* context) {
    GameplayAbility::ActivationCheckResult result;
    const auto& info = ability.getActivationInfo();
    const auto& sourceTags = source.getTags();
    const float cooldown_remaining = source.getCooldownRemaining(ability.getId());
    const float current_mp = source.getAttribute("MP", 9999.0f);

    result.cooldown_remaining = cooldown_remaining;
    result.current_mp = current_mp;

    if (!sourceTags.hasAllTags(
            std::vector<GameplayTag>(info.requiredTags.getTags().begin(), info.requiredTags.getTags().end()))) {
        result.allowed = false;
        result.reason = "missing_required_tags";
        return result;
    }

    if (sourceTags.hasAnyTags(
            std::vector<GameplayTag>(info.blockingTags.getTags().begin(), info.blockingTags.getTags().end()))) {
        result.allowed = false;
        result.reason = "blocking_tags_present";
        return result;
    }

    if (cooldown_remaining > 0.0f) {
        result.allowed = false;
        result.reason = "cooldown_active";
        return result;
    }

    const float resolvedMpCost = info.mpCost > 0 ? static_cast<float>(info.mpCost) : ability.mpCost;
    if (current_mp < resolvedMpCost) {
        result.allowed = false;
        result.reason = "insufficient_mp";
        return result;
    }

    const auto& activeCondition = info.activeCondition.empty() ? ability.activeCondition : info.activeCondition;
    if (!activeCondition.empty()) {
        const AbilityConditionEvaluator evaluator;
        const auto condition = evaluator.evaluate(activeCondition, source, context);
        if (!condition.parsed) {
            result.allowed = false;
            result.reason = condition.reason;
            result.detail = condition.detail;
            return result;
        }
        if (!condition.value) {
            result.allowed = false;
            result.reason = condition.reason;
            result.detail = condition.detail;
            return result;
        }
    }

    return result;
}

} // namespace

bool GameplayAbility::canActivate(const AbilitySystemComponent& source) const {
    return evaluateActivation(source).allowed;
}

bool GameplayAbility::canActivate(const AbilitySystemComponent& source, const AbilityExecutionContext& context) const {
    return evaluateActivation(source, context).allowed;
}

GameplayAbility::ActivationCheckResult GameplayAbility::evaluateActivation(const AbilitySystemComponent& source) const {
    return evaluateActivationWithContext(*this, source, nullptr);
}

GameplayAbility::ActivationCheckResult
GameplayAbility::evaluateActivation(const AbilitySystemComponent& source,
                                    const AbilityExecutionContext& context) const {
    return evaluateActivationWithContext(*this, source, &context);
}

void GameplayAbility::activate(AbilitySystemComponent& source, const AbilityExecutionContext& context) {
    (void)context;
    activate(source);
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
    for (auto it = m_activeTasks.begin(); it != m_activeTasks.end();) {
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
    (void)effect;
    // Effects are currently gated by ability activation; effect-specific tag requirements are not part of this API.
    return true;
}

bool AbilitySystemComponent::canActivateAbility(const GameplayAbility& ability) const {
    return ability.canActivate(*this);
}

bool AbilitySystemComponent::canActivateAbility(const GameplayAbility& ability,
                                                const GameplayAbility::AbilityExecutionContext& context) const {
    return ability.canActivate(*this, context);
}

bool AbilitySystemComponent::tryActivateAbility(GameplayAbility& ability) {
    const auto check = ability.evaluateActivation(*this);
    const float mp_before = getAttribute("MP", defaultBaseAttribute("MP"));

    if (!check.allowed) {
        recordAbilityExecution(ability.getId(), "activate", "blocked", check.reason, "", mp_before, mp_before,
                               getCooldownRemaining(ability.getId()), m_activeEffects.size(), check.detail);
        return false;
    }

    ability.activate(*this);
    const float mp_after = getAttribute("MP", defaultBaseAttribute("MP"));
    recordAbilityExecution(ability.getId(), "activate", "executed", "", "", mp_before, mp_after,
                           getCooldownRemaining(ability.getId()), m_activeEffects.size());
    return true;
}

bool AbilitySystemComponent::tryActivateAbility(GameplayAbility& ability,
                                                const GameplayAbility::AbilityExecutionContext& context) {
    const auto check = ability.evaluateActivation(*this, context);
    const float mp_before = getAttribute("MP", defaultBaseAttribute("MP"));

    if (!check.allowed) {
        recordAbilityExecution(ability.getId(), "activate", "blocked", check.reason, "", mp_before, mp_before,
                               getCooldownRemaining(ability.getId()), m_activeEffects.size(), check.detail);
        return false;
    }

    ability.activate(*this, context);
    const float mp_after = getAttribute("MP", defaultBaseAttribute("MP"));
    recordAbilityExecution(ability.getId(), "activate", "executed", "", "", mp_before, mp_after,
                           getCooldownRemaining(ability.getId()), m_activeEffects.size());
    return true;
}

bool AbilitySystemComponent::isTargetInPattern(const GameplayAbility& ability, int32_t sourceX, int32_t sourceY,
                                               int32_t targetX, int32_t targetY) const {
    const auto& info = ability.getActivationInfo();
    if (!info.pattern)
        return true; // No pattern means distance and placement aren't restricted by GAF

    int32_t dx = targetX - sourceX;
    int32_t dy = targetY - sourceY;

    return info.pattern->hasPoint(dx, dy);
}

} // namespace urpg::ability
