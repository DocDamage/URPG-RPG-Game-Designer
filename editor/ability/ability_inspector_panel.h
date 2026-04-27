#pragma once

#include "editor/ability/ability_inspector_model.h"
#include "editor/ability/pattern_field_model.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/ability/gameplay_effect.h"
#include <functional>
#include <string>
#include <vector>

namespace urpg::ability {
class AbilitySystemComponent;
class GameplayAbility;
} // namespace urpg::ability

namespace urpg::editor {

using namespace urpg::ability;

class AbilityInspectorPanel {
  public:
    using DraftAbilityDefinition = urpg::ability::AuthoredAbilityAsset;
    using CommandCallback = std::function<bool()>;

    struct CommandCallbacks {
        CommandCallback preview_selected;
        CommandCallback apply_draft_to_runtime;
        CommandCallback save_draft;
        CommandCallback load_draft;
    };

    struct DraftPreviewSnapshot {
        bool has_draft = false;
        std::string ability_id;
        float cooldown_seconds = 0.0f;
        float mp_cost = 0.0f;
        std::string effect_id;
        std::string effect_attribute;
        std::string effect_operation;
        float effect_value = 0.0f;
        float effect_duration = 0.0f;
        float preview_mp_before = 0.0f;
        float preview_mp_after = 0.0f;
        float preview_attribute_before = 0.0f;
        float preview_attribute_after = 0.0f;
        PatternFieldModel::PreviewSnapshot pattern_preview;
    };

    struct RenderSnapshot {
        struct ControlState {
            std::string id;
            std::string label;
            bool enabled = false;
            std::string disabled_reason;
        };

        bool visible = true;
        bool has_rendered_frame = false;
        std::vector<std::string> diagnostic_lines;
        size_t diagnostic_count = 0;
        std::string latest_ability_id;
        std::string latest_outcome;
        std::string latest_command_id;
        bool latest_command_success = false;
        std::string latest_command_message;
        std::string selected_ability_id;
        bool selected_ability_can_activate = false;
        std::string selected_ability_blocking_reason;
        DraftPreviewSnapshot draft_preview;
        std::vector<ControlState> controls;
        std::vector<std::string> validation_issues;
    };

    AbilityInspectorPanel() = default;

    void update(const AbilitySystemComponent& asc);
    void clear();
    void render();
    void setCommandCallbacks(CommandCallbacks callbacks);
    void recordCommandResult(const std::string& command_id, bool success, const std::string& message);
    bool selectAbility(size_t index, const AbilitySystemComponent& asc);
    bool previewSelectedAbility(AbilitySystemComponent& asc);
    void resetDraftAbility();
    bool setDraftAbilityId(const std::string& ability_id);
    bool setDraftCooldownSeconds(float cooldown_seconds);
    bool setDraftMpCost(float mp_cost);
    bool setDraftEffectId(const std::string& effect_id);
    bool setDraftEffectAttribute(const std::string& effect_attribute);
    bool setDraftEffectOperation(urpg::ModifierOp effect_operation);
    bool setDraftEffectValue(float effect_value);
    bool setDraftEffectDuration(float effect_duration);
    bool setDraftPatternName(const std::string& pattern_name);
    bool applyDraftPatternPreset(const std::string& preset_id);
    bool toggleDraftPatternPoint(int32_t x, int32_t y);
    bool clearDraftPattern();
    void populateDraftPreviewRuntime(AbilitySystemComponent& asc) const;
    void applyDraftToRuntime(AbilitySystemComponent& asc) const;
    bool selectDraftAbility(AbilitySystemComponent& asc);

    const AbilityInspectorModel& getModel() const { return m_model; }
    const RenderSnapshot& getRenderSnapshot() const { return m_snapshot; }
    AbilityDiagnosticsSnapshot getDiagnosticsSnapshot(const AbilitySystemComponent& asc) const {
        return m_model.buildDiagnosticsSnapshot(asc);
    }
    DraftAbilityDefinition getDraftAsset() const;
    void setDraftFromAsset(const DraftAbilityDefinition& asset);
    const PatternFieldModel& getDraftPatternModel() const { return m_draft_pattern_model; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

  private:
    void rebuildSnapshot(const AbilitySystemComponent& asc);
    void rebuildControlState();
    void validateDraftPattern();
    std::shared_ptr<urpg::ability::GameplayAbility> buildDraftAbility() const;
    DraftAbilityDefinition buildDraftAsset() const;
    DraftPreviewSnapshot buildDraftPreviewSnapshot() const;
    static const char* modifierOpLabel(urpg::ModifierOp operation);

    AbilityInspectorModel m_model;
    DraftAbilityDefinition m_draft_definition;
    PatternFieldModel m_draft_pattern_model;
    RenderSnapshot m_snapshot;
    CommandCallbacks m_command_callbacks;
    bool m_visible = true;
};

} // namespace urpg::editor
