#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/ability_system_component.h"
#include <iomanip>
#include <sstream>
#include <string_view>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

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
    rebuildControlState();
}

void AbilityInspectorPanel::setCommandCallbacks(CommandCallbacks callbacks) {
    m_command_callbacks = std::move(callbacks);
    rebuildControlState();
}

void AbilityInspectorPanel::recordCommandResult(const std::string& command_id, bool success,
                                                const std::string& message) {
    m_snapshot.latest_command_id = command_id;
    m_snapshot.latest_command_success = success;
    m_snapshot.latest_command_message = message;
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
    if (auto current = m_draft_pattern_model.getCurrentPattern(); current && current->getName() == pattern_name) {
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
    if (!m_visible)
        return;

    m_snapshot.visible = m_visible;
    m_snapshot.has_rendered_frame = true;
    rebuildControlState();

#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    if (!ImGui::Begin("Ability Inspector", &m_visible)) {
        ImGui::End();
        return;
    }

    const auto& tags = m_model.getActiveTags();
    ImGui::Text("Active Tags");
    if (tags.empty()) {
        ImGui::TextDisabled("None");
    } else {
        for (const auto& tagInfo : tags) {
            ImGui::BulletText("%s (%d stack%s)", tagInfo.tag.c_str(), tagInfo.count, tagInfo.count == 1 ? "" : "s");
        }
    }

    ImGui::Separator();
    const auto& abilities = m_model.getAbilities();
    ImGui::Text("Abilities");
    if (abilities.empty()) {
        ImGui::TextDisabled("No abilities are bound to this runtime.");
    } else {
        if (ImGui::BeginTable("AbilityInspectorAbilities", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Ability");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Cooldown");
            ImGui::TableSetupColumn("Blocking Reason");
            ImGui::TableHeadersRow();

            const auto selected_index = m_model.selectedAbilityIndex();
            for (size_t index = 0; index < abilities.size(); ++index) {
                const auto& info = abilities[index];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool selected = selected_index.has_value() && *selected_index == index;
                if (ImGui::Selectable(info.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    m_model.selectAbility(index);
                    m_snapshot.selected_ability_id = info.name;
                    m_snapshot.selected_ability_can_activate = info.can_activate;
                    m_snapshot.selected_ability_blocking_reason = info.blocking_reason;
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.can_activate ? "Ready" : "Blocked");
                ImGui::TableNextColumn();
                ImGui::Text("%.2fs", info.cooldown_remaining);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.blocking_reason.empty() ? "-" : info.blocking_reason.c_str());
            }

            ImGui::EndTable();
        }
    }

    ImGui::Separator();
    ImGui::Text("Actions");
    const auto controlById = [this](std::string_view id) -> const RenderSnapshot::ControlState* {
        for (const auto& control : m_snapshot.controls) {
            if (control.id == id) {
                return &control;
            }
        }
        return nullptr;
    };

    const auto button = [&](const char* id, const char* label, bool enabled, const CommandCallback& callback) {
        if (!enabled) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(label) && enabled && callback) {
            (void)callback();
        }
        if (!enabled) {
            if (const auto* control = controlById(id);
                control != nullptr && !control->disabled_reason.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("%s", control->disabled_reason.c_str());
            }
        }
        if (!enabled) {
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        (void)id;
    };

    button("preview_selected", "Preview",
           !m_snapshot.selected_ability_id.empty() && static_cast<bool>(m_command_callbacks.preview_selected),
           m_command_callbacks.preview_selected);
    button("validate_draft", "Validate", m_snapshot.draft_preview.has_draft, [this]() {
        validateDraftPattern();
        return m_snapshot.validation_issues.empty();
    });
    button("apply_draft", "Apply", static_cast<bool>(m_command_callbacks.apply_draft_to_runtime),
           m_command_callbacks.apply_draft_to_runtime);
    button("save_draft", "Save", static_cast<bool>(m_command_callbacks.save_draft), m_command_callbacks.save_draft);
    button("load_draft", "Load", static_cast<bool>(m_command_callbacks.load_draft), m_command_callbacks.load_draft);
    ImGui::NewLine();

    if (!m_snapshot.diagnostic_lines.empty()) {
        ImGui::Separator();
        ImGui::Text("Diagnostics");
        for (const auto& line : m_snapshot.diagnostic_lines) {
            ImGui::BulletText("%s", line.c_str());
        }
    }

    if (!m_snapshot.latest_command_message.empty()) {
        ImGui::Separator();
        ImGui::Text("Last Command");
        ImGui::BulletText("%s: %s", m_snapshot.latest_command_success ? "OK" : "Error",
                          m_snapshot.latest_command_message.c_str());
    }

    if (!m_snapshot.validation_issues.empty()) {
        ImGui::Separator();
        ImGui::Text("Validation");
        for (const auto& issue : m_snapshot.validation_issues) {
            ImGui::BulletText("%s", issue.c_str());
        }
    }

    if (m_snapshot.draft_preview.has_draft) {
        ImGui::Separator();
        ImGui::Text("Draft Preview");
        ImGui::Text("Ability: %s", m_snapshot.draft_preview.ability_id.c_str());
        ImGui::Text("Cost / Cooldown: %.2f MP / %.2fs", m_snapshot.draft_preview.mp_cost,
                    m_snapshot.draft_preview.cooldown_seconds);
        ImGui::Text("Effect: %s -> %s %s %.2f (%.2fs)", m_snapshot.draft_preview.effect_id.c_str(),
                    m_snapshot.draft_preview.effect_attribute.c_str(),
                    m_snapshot.draft_preview.effect_operation.c_str(), m_snapshot.draft_preview.effect_value,
                    m_snapshot.draft_preview.effect_duration);
        for (const auto& row : m_snapshot.draft_preview.pattern_preview.grid_rows) {
            ImGui::TextUnformatted(row.c_str());
        }
    }

    ImGui::End();
#endif
}

void AbilityInspectorPanel::rebuildControlState() {
    const bool hasAbilities = !m_model.getAbilities().empty();
    const bool hasSelection = !m_snapshot.selected_ability_id.empty();
    const bool hasPreviewCallback = static_cast<bool>(m_command_callbacks.preview_selected);
    const bool hasApplyCallback = static_cast<bool>(m_command_callbacks.apply_draft_to_runtime);
    const bool hasSaveCallback = static_cast<bool>(m_command_callbacks.save_draft);
    const bool hasLoadCallback = static_cast<bool>(m_command_callbacks.load_draft);

    m_snapshot.controls = {
        {"select_ability", "Select Ability", hasAbilities,
         hasAbilities ? "" : "No abilities are bound to this runtime."},
        {"preview_selected", "Preview Selected",
         hasSelection && hasPreviewCallback,
         !hasSelection ? "Select an ability before previewing."
                       : (!hasPreviewCallback ? "Editor host has not registered a preview command handler." : "")},
        {"validate_draft", "Validate Draft", m_snapshot.draft_preview.has_draft,
         m_snapshot.draft_preview.has_draft ? "" : "Create or load a draft ability before validation."},
        {"apply_draft", "Apply Draft", hasApplyCallback,
         hasApplyCallback ? "" : "Editor host has not registered an apply-draft command handler."},
        {"save_draft", "Save Draft", hasSaveCallback,
         hasSaveCallback ? "" : "Editor host has not registered a save-draft command handler."},
        {"load_draft", "Load Draft", hasLoadCallback,
         hasLoadCallback ? "" : "Editor host has not registered a load-draft command handler."},
    };
}

void AbilityInspectorPanel::validateDraftPattern() {
    m_snapshot.validation_issues.clear();
    const auto asset = buildDraftAsset();
    const auto validation = PatternValidator::Validate(asset.pattern);
    if (!validation.isValid) {
        m_snapshot.validation_issues = validation.issues;
    }
    if (asset.ability_id.empty()) {
        m_snapshot.validation_issues.push_back("Ability id is required.");
    }
    if (asset.effect_id.empty()) {
        m_snapshot.validation_issues.push_back("Effect id is required.");
    }
    if (asset.effect_attribute.empty()) {
        m_snapshot.validation_issues.push_back("Effect attribute is required.");
    }
}

void AbilityInspectorPanel::rebuildSnapshot(const AbilitySystemComponent& asc) {
    const auto latest_command_id = m_snapshot.latest_command_id;
    const auto latest_command_success = m_snapshot.latest_command_success;
    const auto latest_command_message = m_snapshot.latest_command_message;
    m_snapshot = {};
    m_snapshot.visible = m_visible;
    m_snapshot.latest_command_id = latest_command_id;
    m_snapshot.latest_command_success = latest_command_success;
    m_snapshot.latest_command_message = latest_command_message;
    m_snapshot.draft_preview = buildDraftPreviewSnapshot();

    validateDraftPattern();

    const auto& history = asc.getAbilityExecutionHistory();
    m_snapshot.diagnostic_count = history.size();
    if (const auto* selected_ability = m_model.selectedAbility()) {
        m_snapshot.selected_ability_id = selected_ability->name;
        m_snapshot.selected_ability_can_activate = selected_ability->can_activate;
        m_snapshot.selected_ability_blocking_reason = selected_ability->blocking_reason;
    }
    if (history.empty()) {
        rebuildControlState();
        return;
    }

    const auto& latest = history.back();
    m_snapshot.latest_ability_id = latest.ability_id;
    m_snapshot.latest_outcome = latest.outcome;

    for (const auto& record : history) {
        std::ostringstream line;
        line << "#" << record.sequence_id << " " << record.ability_id << " " << record.stage << " -> "
             << record.outcome;
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
            line << " mp " << record.mp_before << " -> " << record.mp_after << ", cd=" << record.cooldown_after
                 << ", effects=" << record.active_effect_count;
        }
        m_snapshot.diagnostic_lines.push_back(line.str());
    }

    rebuildControlState();
}

} // namespace urpg::editor
