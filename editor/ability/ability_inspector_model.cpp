#include "editor/ability/ability_inspector_model.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include <algorithm>

namespace urpg::editor {

using namespace urpg::ability;

void AbilityInspectorModel::refresh(const AbilitySystemComponent& asc) {
    m_abilities.clear();
    m_active_tags.clear();

    // Map Active Tags
    const auto& tags = asc.getTags();
    for (const auto& tagObj : tags.getTags()) {
        m_active_tags.push_back({tagObj.getName(), 1});
    }

    // Sort tags alphabetically for UI
    std::sort(m_active_tags.begin(), m_active_tags.end(), 
        [](const auto& a, const auto& b) { return a.tag < b.tag; });

    // Project abilities from ASC
    for (const auto& ability : asc.getAbilities()) {
        AbilityInfo info;
        const auto& abilityId = ability->getId();
        info.name = abilityId;
        info.cooldown_remaining = asc.getCooldownRemaining(abilityId);
        info.can_activate = ability->canActivate(asc);
        info.pattern = ability->getActivationInfo().pattern;
        
        if (!info.can_activate) {
            // Determine a simple blocking reason
            if (info.cooldown_remaining > 0.0f) {
                info.blocking_reason = "Cooldown (" + std::to_string((int)info.cooldown_remaining) + "s)";
            } else if (ability->mpCost > asc.getAttribute("MP", 0.0f)) {
                info.blocking_reason = "Insufficient MP";
            } else {
                info.blocking_reason = "Tag requirement not met";
            }
        }

        m_abilities.push_back(info);
    }
}

void AbilityInspectorModel::clear() {
    m_abilities.clear();
    m_active_tags.clear();
}

AbilityDiagnosticsSnapshot AbilityInspectorModel::buildDiagnosticsSnapshot(const AbilitySystemComponent& asc) const {
    AbilityDiagnosticsSnapshot snap;

    const auto& abilities = asc.getAbilities();
    snap.ability_count = abilities.size();
    for (const auto& ability : abilities) {
        AbilityDiagnosticsAbilityState state;
        const auto& abilityId = ability->getId();
        state.id = abilityId;
        state.cooldown_remaining = asc.getCooldownRemaining(abilityId);
        state.can_activate = ability->canActivate(asc);
        if (!state.can_activate) {
            if (state.cooldown_remaining > 0.0f) {
                state.blocking_reason = "Cooldown (" + std::to_string(static_cast<int>(state.cooldown_remaining)) + "s)";
            } else if (ability->mpCost > asc.getAttribute("MP", 0.0f)) {
                state.blocking_reason = "Insufficient MP";
            } else {
                state.blocking_reason = "Tag requirement not met";
            }
        }
        snap.ability_states.push_back(std::move(state));
    }

    // Refresh to get active effects
    // We query the ASC directly for effects since they are not exposed through the model yet.
    snap.active_effect_count = asc.getActiveEffectCount();
    // Note: active effects detailed enumeration would require ASC to expose effect list.
    // For now, we capture the count and any available cooldowns.

    const auto& cooldowns = asc.getActiveCooldowns();
    snap.active_cooldown_count = cooldowns.size();

    const auto& history = asc.getAbilityExecutionHistory();
    if (!history.empty()) {
        snap.last_execution_sequence_id = history.back().sequence_id;
    }

    return snap;
}

} // namespace urpg::editor
