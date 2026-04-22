#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/ability_system_component.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace urpg::editor {

using namespace urpg::ability;

void AbilityInspectorPanel::update(const AbilitySystemComponent& asc) {
    m_model.refresh(asc);
    rebuildSnapshot(asc);
}

void AbilityInspectorPanel::clear() {
    m_model.clear();
    m_snapshot = {};
    m_snapshot.visible = m_visible;
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
            std::cout << "  - " << info.name 
                      << ": " << (info.can_activate ? "[Ready]" : "[Cooldown: " + std::to_string(info.cooldown_remaining) + "s]")
                      << (info.blocking_reason.empty() ? "" : " (Blocked: " + info.blocking_reason + ")")
                      << "\n";

            if (info.pattern) {
                // Validation
                auto validation = PatternValidator::Validate(*info.pattern);
                if (!validation.isValid) {
                    for (const auto& issue : validation.issues) {
                        std::cout << "      [!] Error: " << issue << "\n";
                    }
                }

                // Mini Preview (3x3 area around center for inspector simplicity)
                std::cout << "      Pattern: ";
                for (int py = -1; py <= 1; ++py) {
                    if (py != -1) std::cout << "               ";
                    for (int px = -1; px <= 1; ++px) {
                        if (px == 0 && py == 0) {
                            std::cout << (info.pattern->hasPoint(px, py) ? "[O]" : "[.]");
                        } else {
                            std::cout << (info.pattern->hasPoint(px, py) ? "[X]" : "[ ]");
                        }
                    }
                    std::cout << "\n";
                }
            }
        }
    }

    if (!m_snapshot.diagnostic_lines.empty()) {
        std::cout << "Diagnostics / Replay Log:\n";
        for (const auto& line : m_snapshot.diagnostic_lines) {
            std::cout << "  - " << line << "\n";
        }
    }
    std::cout << "------------------------\n";
}

void AbilityInspectorPanel::rebuildSnapshot(const AbilitySystemComponent& asc) {
    m_snapshot = {};
    m_snapshot.visible = m_visible;

    const auto& history = asc.getAbilityExecutionHistory();
    m_snapshot.diagnostic_count = history.size();
    if (history.empty()) {
        return;
    }

    const auto& latest = history.back();
    m_snapshot.latest_ability_id = latest.ability_id;
    m_snapshot.latest_outcome = latest.outcome;

    for (const auto& record : history) {
        std::ostringstream line;
        line << "#" << record.sequence_id << " " << record.ability_id << " " << record.stage
             << " -> " << record.outcome;
        if (!record.state_name.empty()) {
            line << " [" << record.state_name << "]";
        }
        if (!record.reason.empty()) {
            line << " (" << record.reason << ")";
        }
        if (!record.detail.empty()) {
            line << " {" << record.detail << "}";
        }
        if (record.stage == "activate") {
            line << " mp " << record.mp_before << " -> " << record.mp_after
                 << ", cd=" << record.cooldown_after
                 << ", effects=" << record.active_effect_count;
        }
        m_snapshot.diagnostic_lines.push_back(line.str());
    }
}

} // namespace urpg::editor
