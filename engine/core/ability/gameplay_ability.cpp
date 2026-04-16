#include "gameplay_ability.h"
#include "ability_system_component.h"

namespace urpg::ability {

bool GameplayAbility::canActivate(const AbilitySystemComponent& source) const {
    const auto& info = getActivationInfo();
    const auto& sourceTags = source.getTags();

    // Required tags check (e.g. MUST have "State.Alive" and "Action.Ready")
    if (!sourceTags.hasAllTags(std::vector<GameplayTag>(info.requiredTags.getTags().begin(), info.requiredTags.getTags().end()))) {
        return false;
    }

    // Blocking tags check (e.g. CANNOT have "State.Stunned")
    if (sourceTags.hasAnyTags(std::vector<GameplayTag>(info.blockingTags.getTags().begin(), info.blockingTags.getTags().end()))) {
        return false;
    }

    // Cooldown check
    if (source.getCooldownRemaining(getId()) > 0.0f) {
        return false;
    }

    return true;
}

bool AbilitySystemComponent::canActivateAbility(const GameplayAbility& ability) const {
    return ability.canActivate(*this);
}

} // namespace urpg::ability
