#include "editor/ability/ability_orchestration_panel.h"

namespace urpg::editor {

void AbilityOrchestrationPanel::loadDocument(urpg::ability::AbilityOrchestrationDocument document) {
    document_ = std::move(document);
    has_document_ = true;
    refreshPreview();
}

void AbilityOrchestrationPanel::render() {
    snapshot_.rendered = true;
    if (!has_document_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load an ability orchestration before rendering this panel.";
        snapshot_.saved_project_json = nlohmann::json::object();
        snapshot_.runtime_result_json = nlohmann::json::object();
        return;
    }

    refreshPreview();
}

void AbilityOrchestrationPanel::refreshPreview() {
    result_ = urpg::ability::runAbilityOrchestration(document_);
    snapshot_.disabled = false;
    snapshot_.orchestration_id = document_.id;
    snapshot_.mode = urpg::ability::abilityOrchestrationModeName(document_.mode);
    snapshot_.ability_id = document_.ability.ability_id;
    snapshot_.activation_executed = result_.activation_executed;
    snapshot_.source_mp_before = result_.source_mp_before;
    snapshot_.source_mp_after = result_.source_mp_after;
    snapshot_.cooldown_after = result_.cooldown_after;
    snapshot_.target_count = result_.targets.size();
    snapshot_.task_count = result_.task_preview_rows.size();
    snapshot_.task_preview_rows = result_.task_preview_rows;
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.saved_project_json = document_.toJson();
    snapshot_.runtime_result_json = urpg::ability::abilityOrchestrationResultToJson(result_);
    snapshot_.status_message = result_.diagnostics.empty() ? "Ability orchestration preview is ready."
                                                           : "Ability orchestration preview has diagnostics.";
}

} // namespace urpg::editor
