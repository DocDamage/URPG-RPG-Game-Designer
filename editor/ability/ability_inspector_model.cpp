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
        info.name = ability->id;
        info.cooldown_remaining = asc.getCooldownRemaining(ability->id);
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

} // namespace urpg::editor
