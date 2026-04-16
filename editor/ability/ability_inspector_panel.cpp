#include "editor/ability/ability_inspector_panel.h"
#include <iostream>
#include <iomanip>

namespace urpg::editor {

void AbilityInspectorPanel::update(const AbilitySystemComponent& asc) {
    m_model.refresh(asc);
}

void AbilityInspectorPanel::render() {
    if (!m_visible) return;

    std::cout << "\n--- Ability Inspector ---\n";
    
    // Header - Active Tags
    const auto& tags = m_model.getActiveTags();
    if (tags.empty()) {
        std::cout << "Active Tags: None\n";
    } else {
        std::cout << "Active Tags:\n";
        for (const auto& tagInfo : tags) {
            std::cout << "  - " << tagInfo.tag << " (" << tagInfo.count << " stacks)\n";
        }
    }

    // Header - Abilities / Cooldowns
    const auto& abilities = m_model.getAbilities();
    if (abilities.empty()) {
        std::cout << "Abilities: No active cooldowns to track.\n";
    } else {
        std::cout << "Abilities / Cooldowns:\n";
        for (const auto& info : abilities) {
            std::fixed;
            std::setprecision(1);
            std::cout << "  - " << info.name 
                      << ": " << (info.can_activate ? "[Ready]" : "[Cooldown: " + std::to_string(info.cooldown_remaining) + "s]")
                      << (info.blocking_reason.empty() ? "" : " (Blocked: " + info.blocking_reason + ")")
                      << "\n";
        }
    }
    std::cout << "------------------------\n";
}

} // namespace urpg::editor
