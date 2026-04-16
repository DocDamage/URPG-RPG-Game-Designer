#include "editor/ability/ability_inspector_model.h"
#include "engine/core/ability/gameplay_ability.h"
#include <algorithm>

namespace urpg::editor {

void AbilityInspectorModel::refresh(const AbilitySystemComponent& asc) {
    m_abilities.clear();
    m_active_tags.clear();

    // Map Active Tags
    const auto& tags = asc.getActiveTags();
    for (const auto& [tag, count] : tags) {
        if (count > 0) {
            m_active_tags.push_back({tag, count});
        }
    }

    // Sort tags alphabetically for UI
    std::sort(m_active_tags.begin(), m_active_tags.end(), 
        [](const auto& a, const auto& b) { return a.tag < b.tag; });

    // Map Abilities (Assuming ASC holds some registered prototype list or we iterate current)
    // For now, we project the specific abilities the ASC is tracking cooldowns for
    for (const auto& [abilityName, cooldown] : asc.getActiveCooldowns()) {
        AbilityInfo info;
        info.name = abilityName;
        info.cooldown_remaining = cooldown;
        info.can_activate = (cooldown <= 0.0f);
        
        if (!info.can_activate) {
            info.blocking_reason = "Cooldown Active";
        }

        m_abilities.push_back(info);
    }
}

} // namespace urpg::editor
