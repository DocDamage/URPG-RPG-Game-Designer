#include "editor/ability/ability_orchestration_panel.h"

namespace urpg::editor {

void AbilityOrchestrationPanel::loadDocument(urpg::ability::AbilityOrchestrationDocument document) {
    document_ = std::move(document);
    has_document_ = true;
    refreshPreview();
}

bool AbilityOrchestrationPanel::loadProjectData(const nlohmann::json& json) {
    if (!json.is_object()) {
        return false;
    }
    loadDocument(urpg::ability::AbilityOrchestrationDocument::fromJson(json));
    return has_document_;
}

nlohmann::json AbilityOrchestrationPanel::saveProjectData() const {
    if (!has_document_) {
        return nlohmann::json::object();
    }
    return document_.toJson();
}

bool AbilityOrchestrationPanel::addTask(urpg::ability::AbilityOrchestrationTask task) {
    if (!has_document_ || task.id.empty() || task.kind.empty()) {
        return false;
    }
    document_.tasks.push_back(std::move(task));
    refreshPreview();
    return true;
}

bool AbilityOrchestrationPanel::applyPreview() {
    if (!has_document_) {
        return false;
    }
    refreshPreview();
    if (!result_.valid || !result_.diagnostics.empty()) {
        return false;
    }
    applied_project_json_ = document_.toJson();
    has_applied_project_ = true;
    refreshPreview();
    return true;
}

bool AbilityOrchestrationPanel::revertLastApply() {
    if (!has_applied_project_) {
        return false;
    }
    applied_project_json_ = nlohmann::json::object();
    has_applied_project_ = false;
    refreshPreview();
    return true;
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
    snapshot_.task_execution_events = result_.task_execution_events;
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.validation_messages.clear();
    for (const auto& diagnostic : result_.diagnostics) {
        snapshot_.validation_messages.push_back(diagnostic.code + ": " + diagnostic.message);
    }
    snapshot_.can_save = has_document_ && result_.valid && result_.diagnostics.empty();
    snapshot_.can_load = true;
    snapshot_.can_apply = snapshot_.can_save && result_.activation_executed;
    snapshot_.can_revert = has_applied_project_;
    snapshot_.saved_project_json = document_.toJson();
    snapshot_.applied_project_json = has_applied_project_ ? applied_project_json_ : nlohmann::json::object();
    snapshot_.runtime_result_json = urpg::ability::abilityOrchestrationResultToJson(result_);
    snapshot_.status_message = result_.diagnostics.empty() ? "Ability orchestration preview is ready."
                                                           : "Ability orchestration preview has diagnostics.";
}

} // namespace urpg::editor
