#pragma once

#include "engine/core/ability/ability_sandbox.h"

#include <string>

namespace urpg::editor {

struct AbilitySandboxPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    std::string sandbox_id;
    std::string ability_id;
    float mp_cost = 0.0f;
    float cooldown_seconds = 0.0f;
    float cooldown_after = 0.0f;
    float seconds_between_attempts = 0.0f;
    float mp_before = 0.0f;
    float mp_after = 0.0f;
    std::string effect_id;
    std::string effect_attribute;
    float effect_before = 0.0f;
    float effect_after = 0.0f;
    size_t source_tag_count = 0;
    size_t required_tag_count = 0;
    size_t blocking_tag_count = 0;
    size_t active_effect_count = 0;
    size_t activation_attempt_count = 0;
    size_t execution_history_count = 0;
    size_t runtime_trace_count = 0;
    size_t diagnostic_count = 0;
    bool activation_allowed = false;
    bool activation_executed = false;
    float mp_delta = 0.0f;
    float effect_delta = 0.0f;
    float activation_success_ratio = 0.0f;
    std::string ux_focus_lane = "ability_tuning";
    std::string blocking_reason;
    std::string primary_action = "Load an ability sandbox.";
    std::string saved_project_json;
    std::string status_message = "Load an ability sandbox before rendering this panel.";
};

class AbilitySandboxPanel {
public:
    void loadDocument(urpg::ability::AbilitySandboxDocument document);
    void setSourceMp(float source_mp);
    void setAbilityCost(float mp_cost);
    void setAbilityCooldown(float cooldown_seconds);
    void setEffectValue(float effect_value);
    void setActivationAttempts(int32_t attempts, float seconds_between_attempts);
    void addSourceTag(std::string tag);
    void removeSourceTag(std::string tag);
    void render();

    const AbilitySandboxPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::ability::AbilitySandboxResult& result() const { return result_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();

    urpg::ability::AbilitySandboxDocument document_;
    urpg::ability::AbilitySandboxResult result_;
    AbilitySandboxPanelSnapshot snapshot_{};
    bool loaded_ = false;
};

} // namespace urpg::editor
