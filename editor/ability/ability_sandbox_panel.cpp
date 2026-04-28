#include "editor/ability/ability_sandbox_panel.h"

#include <utility>

namespace urpg::editor {

void AbilitySandboxPanel::loadDocument(urpg::ability::AbilitySandboxDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refreshPreview();
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
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.activation_allowed = result_.activation_allowed;
    snapshot_.activation_executed = result_.activation_executed;
    snapshot_.blocking_reason = result_.blocking_reason;
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Ability sandbox preview is ready." : "Ability sandbox preview has diagnostics.";
}

} // namespace urpg::editor
