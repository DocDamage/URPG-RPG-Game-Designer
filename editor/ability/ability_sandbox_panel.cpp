#include "editor/ability/ability_sandbox_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void AbilitySandboxPanel::loadDocument(urpg::ability::AbilitySandboxDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refreshPreview();
}

void AbilitySandboxPanel::setSourceMp(float source_mp) {
    document_.source_mp = source_mp;
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::setAbilityCost(float mp_cost) {
    document_.ability.mp_cost = mp_cost;
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::setAbilityCooldown(float cooldown_seconds) {
    document_.ability.cooldown_seconds = cooldown_seconds;
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::setEffectValue(float effect_value) {
    document_.ability.effect_value = effect_value;
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::setActivationAttempts(int32_t attempts, float seconds_between_attempts) {
    document_.activation_attempts = attempts;
    document_.seconds_between_attempts = seconds_between_attempts;
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::addSourceTag(std::string tag) {
    if (!tag.empty() && std::find(document_.source_tags.begin(), document_.source_tags.end(), tag) ==
                            document_.source_tags.end()) {
        document_.source_tags.push_back(std::move(tag));
    }
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::removeSourceTag(std::string tag) {
    document_.source_tags.erase(std::remove(document_.source_tags.begin(), document_.source_tags.end(), tag),
                                document_.source_tags.end());
    if (loaded_) {
        refreshPreview();
    }
}

void AbilitySandboxPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load an ability sandbox before rendering this panel.";
        return;
    }
    refreshPreview();
}

void AbilitySandboxPanel::refreshPreview() {
    result_ = urpg::ability::RunAbilitySandbox(document_);
    snapshot_.disabled = false;
    snapshot_.sandbox_id = document_.id;
    snapshot_.ability_id = document_.ability.ability_id;
    snapshot_.mp_cost = document_.ability.mp_cost;
    snapshot_.cooldown_seconds = document_.ability.cooldown_seconds;
    snapshot_.cooldown_after = result_.cooldown_after;
    snapshot_.seconds_between_attempts = document_.seconds_between_attempts;
    snapshot_.mp_before = result_.mp_before;
    snapshot_.mp_after = result_.mp_after;
    snapshot_.effect_id = document_.ability.effect_id;
    snapshot_.effect_attribute = document_.ability.effect_attribute;
    snapshot_.effect_before = result_.effect_attribute_before;
    snapshot_.effect_after = result_.effect_attribute_after;
    snapshot_.source_tag_count = document_.source_tags.size();
    snapshot_.required_tag_count = document_.required_tags.size();
    snapshot_.blocking_tag_count = document_.blocking_tags.size();
    snapshot_.active_effect_count = result_.active_effect_count;
    snapshot_.activation_attempt_count = result_.activation_steps.size();
    snapshot_.execution_history_count = result_.execution_history_count;
    snapshot_.runtime_trace_count = result_.runtime_trace.size();
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.activation_allowed = result_.activation_allowed;
    snapshot_.activation_executed = result_.activation_executed;
    snapshot_.blocking_reason = result_.blocking_reason;
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Ability sandbox preview is ready." : "Ability sandbox preview has diagnostics.";
}

} // namespace urpg::editor
