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

bool AbilityInspectorPanel::selectAbility(size_t index, const AbilitySystemComponent& asc) {
    const bool changed = m_model.selectAbility(index);
    if (changed) {
        rebuildSnapshot(asc);
    }
    return changed;
}

bool AbilityInspectorPanel::previewSelectedAbility(AbilitySystemComponent& asc) {
    const auto selected_id = m_model.selectedAbilityId();
    if (!selected_id.has_value()) {
        return false;
    }

    (void)m_model.previewActivateSelected(asc);
    update(asc);
    return true;
}

void AbilityInspectorPanel::resetDraftAbility() {
    m_draft_definition = {};
    m_draft_pattern_model = PatternFieldModel{};
    m_draft_pattern_model.setName(m_draft_definition.pattern.getName());
}

bool AbilityInspectorPanel::setDraftAbilityId(const std::string& ability_id) {
    if (m_draft_definition.ability_id == ability_id) {
        return false;
    }
    m_draft_definition.ability_id = ability_id;
    return true;
}

bool AbilityInspectorPanel::setDraftCooldownSeconds(float cooldown_seconds) {
    if (m_draft_definition.cooldown_seconds == cooldown_seconds) {
        return false;
    }
    m_draft_definition.cooldown_seconds = cooldown_seconds;
    return true;
}

bool AbilityInspectorPanel::setDraftMpCost(float mp_cost) {
    if (m_draft_definition.mp_cost == mp_cost) {
        return false;
    }
    m_draft_definition.mp_cost = mp_cost;
    return true;
}

bool AbilityInspectorPanel::setDraftEffectId(const std::string& effect_id) {
    if (m_draft_definition.effect_id == effect_id) {
        return false;
    }
    m_draft_definition.effect_id = effect_id;
    return true;
}

bool AbilityInspectorPanel::setDraftEffectAttribute(const std::string& effect_attribute) {
    if (m_draft_definition.effect_attribute == effect_attribute) {
        return false;
    }
    m_draft_definition.effect_attribute = effect_attribute;
    return true;
}

bool AbilityInspectorPanel::setDraftEffectOperation(urpg::ModifierOp effect_operation) {
    if (m_draft_definition.effect_operation == effect_operation) {
        return false;
    }
    m_draft_definition.effect_operation = effect_operation;
    return true;
}

bool AbilityInspectorPanel::setDraftEffectValue(float effect_value) {
    if (m_draft_definition.effect_value == effect_value) {
        return false;
    }
    m_draft_definition.effect_value = effect_value;
    return true;
}

bool AbilityInspectorPanel::setDraftEffectDuration(float effect_duration) {
    if (m_draft_definition.effect_duration == effect_duration) {
        return false;
    }
    m_draft_definition.effect_duration = effect_duration;
    return true;
}

bool AbilityInspectorPanel::setDraftPatternName(const std::string& pattern_name) {
    if (auto current = m_draft_pattern_model.getCurrentPattern();
        current && current->getName() == pattern_name) {
        return false;
    }
    m_draft_pattern_model.setName(pattern_name);
    m_draft_definition.pattern.setName(pattern_name);
    return true;
}

bool AbilityInspectorPanel::applyDraftPatternPreset(const std::string& preset_id) {
    const auto before = m_draft_pattern_model.buildPreviewSnapshot().grid_rows;
    m_draft_pattern_model.applyPreset(preset_id);
    return before != m_draft_pattern_model.buildPreviewSnapshot().grid_rows;
}

bool AbilityInspectorPanel::toggleDraftPatternPoint(int32_t x, int32_t y) {
    const bool before = m_draft_pattern_model.isPointSelected(x, y);
    m_draft_pattern_model.togglePoint(x, y);
    return before != m_draft_pattern_model.isPointSelected(x, y);
}

bool AbilityInspectorPanel::clearDraftPattern() {
    const auto before = m_draft_pattern_model.buildPreviewSnapshot().grid_rows;
    m_draft_pattern_model.clearPattern();
    return before != m_draft_pattern_model.buildPreviewSnapshot().grid_rows;
}

std::shared_ptr<urpg::ability::GameplayAbility> AbilityInspectorPanel::buildDraftAbility() const {
    return urpg::ability::makeGameplayAbilityFromAsset(buildDraftAsset());
}

AbilityInspectorPanel::DraftAbilityDefinition AbilityInspectorPanel::buildDraftAsset() const {
    DraftAbilityDefinition asset = m_draft_definition;
    if (const auto pattern = m_draft_pattern_model.getCurrentPattern()) {
        asset.pattern = *pattern;
    }
    return asset;
}

AbilityInspectorPanel::DraftAbilityDefinition AbilityInspectorPanel::getDraftAsset() const {
    return buildDraftAsset();
}

void AbilityInspectorPanel::setDraftFromAsset(const DraftAbilityDefinition& asset) {
    m_draft_definition = asset;
    m_draft_pattern_model.setCurrentPattern(std::make_shared<urpg::PatternField>(asset.pattern));
}

AbilityInspectorPanel::DraftPreviewSnapshot AbilityInspectorPanel::buildDraftPreviewSnapshot() const {
    DraftPreviewSnapshot snapshot;
    snapshot.has_draft = true;
    const auto asset = buildDraftAsset();
    snapshot.ability_id = asset.ability_id;
    snapshot.cooldown_seconds = asset.cooldown_seconds;
    snapshot.mp_cost = asset.mp_cost;
    snapshot.effect_id = asset.effect_id;
    snapshot.effect_attribute = asset.effect_attribute;
    snapshot.effect_operation = modifierOpLabel(asset.effect_operation);
    snapshot.effect_value = asset.effect_value;
    snapshot.effect_duration = asset.effect_duration;
    snapshot.preview_mp_before = 30.0f;
    snapshot.preview_mp_after = snapshot.preview_mp_before - asset.mp_cost;
    snapshot.preview_attribute_before = 100.0f;

    switch (asset.effect_operation) {
    case urpg::ModifierOp::Add:
        snapshot.preview_attribute_after = snapshot.preview_attribute_before + asset.effect_value;
        break;
    case urpg::ModifierOp::Multiply:
        snapshot.preview_attribute_after = snapshot.preview_attribute_before * asset.effect_value;
        break;
    case urpg::ModifierOp::Override:
        snapshot.preview_attribute_after = asset.effect_value;
        break;
    }

    snapshot.pattern_preview = m_draft_pattern_model.buildPreviewSnapshot();
    return snapshot;
}

const char* AbilityInspectorPanel::modifierOpLabel(urpg::ModifierOp operation) {
    switch (operation) {
    case urpg::ModifierOp::Add:
        return "Add";
    case urpg::ModifierOp::Multiply:
        return "Multiply";
    case urpg::ModifierOp::Override:
        return "Override";
    }

    return "Unknown";
}

void AbilityInspectorPanel::populateDraftPreviewRuntime(AbilitySystemComponent& asc) const {
    asc = AbilitySystemComponent{};
    asc.setAttribute("MP", 30.0f);
    asc.setAttribute(buildDraftAsset().effect_attribute, 100.0f);
    asc.grantAbility(buildDraftAbility());
}

void AbilityInspectorPanel::applyDraftToRuntime(AbilitySystemComponent& asc) const {
    asc.grantOrReplaceAbility(buildDraftAbility());
}

bool AbilityInspectorPanel::selectDraftAbility(AbilitySystemComponent& asc) {
    if (m_model.getAbilities().empty()) {
        update(asc);
    }

    if (m_model.getAbilities().empty()) {
        return false;
    }

    return selectAbility(0, asc);
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
    if (m_snapshot.draft_preview.has_draft) {
        std::cout << "Draft Preview:\n";
        std::cout << "  - Ability: " << m_snapshot.draft_preview.ability_id << "\n";
        std::cout << "  - Cost / Cooldown: " << m_snapshot.draft_preview.mp_cost
                  << " MP / " << m_snapshot.draft_preview.cooldown_seconds << "s\n";
        std::cout << "  - Effect: " << m_snapshot.draft_preview.effect_id
                  << " -> " << m_snapshot.draft_preview.effect_attribute
                  << " " << m_snapshot.draft_preview.effect_operation
                  << " " << m_snapshot.draft_preview.effect_value
                  << " (" << m_snapshot.draft_preview.effect_duration << "s)\n";
    }
    std::cout << "------------------------\n";
}

void AbilityInspectorPanel::rebuildSnapshot(const AbilitySystemComponent& asc) {
    m_snapshot = {};
    m_snapshot.visible = m_visible;
    m_snapshot.draft_preview = buildDraftPreviewSnapshot();

    const auto& history = asc.getAbilityExecutionHistory();
    m_snapshot.diagnostic_count = history.size();
    if (const auto* selected_ability = m_model.selectedAbility()) {
        m_snapshot.selected_ability_id = selected_ability->name;
        m_snapshot.selected_ability_can_activate = selected_ability->can_activate;
        m_snapshot.selected_ability_blocking_reason = selected_ability->blocking_reason;
    }
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
