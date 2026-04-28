#include "editor/ai/ai_assistant_panel.h"

#include <utility>

namespace {

nlohmann::json diagnosticsToJson(const std::vector<urpg::ai::AiKnowledgeDiagnostic>& diagnostics) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& item : diagnostics) {
        out.push_back({{"code", item.code}, {"message", item.message}, {"target", item.target}});
    }
    return out;
}

} // namespace

namespace urpg::editor {

void AiAssistantPanel::setConfig(urpg::ai::AiAssistantConfig config, bool providerAvailable) {
    config_ = std::move(config);
    provider_available_ = providerAvailable;
}

void AiAssistantPanel::setSuggestion(urpg::ai::AiSuggestionRecord suggestion) {
    suggestion_ = std::move(suggestion);
}

void AiAssistantPanel::setProjectData(nlohmann::json projectData) {
    project_data_ = std::move(projectData);
    rebuildTaskPlan();
}

void AiAssistantPanel::setTaskRequest(std::string taskRequest) {
    task_request_ = std::move(taskRequest);
    rebuildTaskPlan();
}

void AiAssistantPanel::rebuildTaskPlan() {
    knowledge_ = urpg::ai::buildDefaultAiKnowledgeSnapshot(project_data_);
    urpg::ai::AiTaskPlanner planner;
    current_task_plan_ = planner.planTask(task_request_, knowledge_.capabilities, knowledge_.project_index,
                                          knowledge_.docs_index, knowledge_.tools);
}

void AiAssistantPanel::render() {
    urpg::ai::AiAssistantConfigValidator validator;
    urpg::ai::AiSuggestionPolicy policy;
    if (current_task_plan_.id.empty()) {
        rebuildTaskPlan();
    }
    last_render_snapshot_ = {
        {"status", validator.evaluate(config_, provider_available_).toJson()},
        {"suggestion", policy.toJson(suggestion_)},
        {"knowledge", {
            {"capability_count", knowledge_.capabilities.capabilities().size()},
            {"project_entry_count", knowledge_.project_index.entries().size()},
            {"doc_entry_count", knowledge_.docs_index.entries().size()},
            {"tool_count", knowledge_.tools.tools().size()},
        }},
        {"task_plan", current_task_plan_.toJson()},
        {"approval", knowledge_.tools.approvalManifest(current_task_plan_, knowledge_.capabilities)},
        {"controls", buildControlSnapshot()},
        {"apply_preview", buildApplyPreviewSnapshot()},
    };
    if (!applied_changes_.empty()) {
        last_render_snapshot_["last_apply"] = applied_changes_.back().toJson();
        last_render_snapshot_["result_diff"] = {
            {"has_changes", !applied_changes_.back().project_patch.empty()},
            {"forward_patch", applied_changes_.back().project_patch},
            {"revert_patch", applied_changes_.back().revert_patch},
        };
    } else {
        last_render_snapshot_["result_diff"] = {
            {"has_changes", false},
            {"forward_patch", nlohmann::json::array()},
            {"revert_patch", nlohmann::json::array()},
        };
    }
}

bool AiAssistantPanel::approveStep(const std::string& stepId) {
    if (current_task_plan_.id.empty()) {
        rebuildTaskPlan();
    }
    for (auto& step : current_task_plan_.steps) {
        if (step.id == stepId) {
            step.approved = true;
            step.rejected = false;
            return true;
        }
    }
    return false;
}

std::size_t AiAssistantPanel::approveAllPendingSteps() {
    if (current_task_plan_.id.empty()) {
        rebuildTaskPlan();
    }
    std::size_t approved = 0;
    for (auto& step : current_task_plan_.steps) {
        const auto* tool = knowledge_.tools.find(step.tool_id);
        if (tool != nullptr && tool->requires_approval && !step.approved) {
            step.approved = true;
            step.rejected = false;
            ++approved;
        }
    }
    return approved;
}

bool AiAssistantPanel::rejectStep(const std::string& stepId) {
    if (current_task_plan_.id.empty()) {
        rebuildTaskPlan();
    }
    for (auto& step : current_task_plan_.steps) {
        if (step.id == stepId) {
            step.approved = false;
            step.rejected = true;
            return true;
        }
    }
    return false;
}

bool AiAssistantPanel::applyApprovedPlan() {
    if (current_task_plan_.id.empty()) {
        rebuildTaskPlan();
    }
    const auto result = knowledge_.tools.applyApprovedPlan(current_task_plan_, project_data_);
    project_data_ = result.project_data;
    last_render_snapshot_["last_apply"] = result.toJson();
    if (result.applied) {
        applied_changes_.push_back(result);
        knowledge_ = urpg::ai::buildDefaultAiKnowledgeSnapshot(project_data_);
    }
    return result.applied;
}

bool AiAssistantPanel::revertLastAppliedPlan() {
    if (applied_changes_.empty()) {
        last_render_snapshot_["last_revert"] = {{"reverted", false}, {"reason", "no_applied_ai_change"}};
        return false;
    }
    const auto change = applied_changes_.back();
    try {
        project_data_ = project_data_.patch(change.revert_patch);
    } catch (const nlohmann::json::exception&) {
        last_render_snapshot_["last_revert"] = {{"reverted", false}, {"reason", "patch_apply_failed"}};
        return false;
    }
    applied_changes_.pop_back();
    knowledge_ = urpg::ai::buildDefaultAiKnowledgeSnapshot(project_data_);
    last_render_snapshot_["last_revert"] = {
        {"reverted", true},
        {"revert_patch", change.revert_patch},
        {"project_data", project_data_},
    };
    return true;
}

nlohmann::json AiAssistantPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

nlohmann::json AiAssistantPanel::buildControlSnapshot() const {
    const auto diagnostics = knowledge_.tools.validatePlan(current_task_plan_);
    const bool canApply = current_task_plan_.id.empty() ? false : diagnostics.empty();
    const auto approval = knowledge_.tools.approvalManifest(current_task_plan_, knowledge_.capabilities);
    nlohmann::json stepControls = nlohmann::json::array();
    for (const auto& step : current_task_plan_.steps) {
        const auto* tool = knowledge_.tools.find(step.tool_id);
        const bool requiresApproval = tool != nullptr && tool->requires_approval;
        stepControls.push_back({
            {"step_id", step.id},
            {"tool_id", step.tool_id},
            {"summary", step.summary},
            {"state", step.rejected ? "rejected" : (step.approved ? "approved" : "needs_review")},
            {"approve_button", {
                {"visible", requiresApproval},
                {"enabled", requiresApproval && !step.approved && !step.rejected},
                {"label", "Approve"},
                {"action", "approve_step"},
            }},
            {"reject_button", {
                {"visible", requiresApproval},
                {"enabled", requiresApproval && !step.rejected},
                {"label", "Reject"},
                {"action", "reject_step"},
            }},
        });
    }
    return {
        {"approve_all_button", {
            {"visible", true},
            {"enabled", approval.value("pending_count", std::size_t{0}) > 0U},
            {"label", "Approve All"},
            {"action", "approve_all_pending_steps"},
        }},
        {"apply_button", {
            {"visible", true},
            {"enabled", canApply},
            {"label", "Apply"},
            {"action", "apply_approved_plan"},
        }},
        {"revert_button", {
            {"visible", true},
            {"enabled", !applied_changes_.empty()},
            {"label", "Revert AI Change"},
            {"action", "revert_last_applied_plan"},
        }},
        {"step_controls", stepControls},
        {"diagnostics", current_task_plan_.id.empty() ? nlohmann::json::array() : diagnosticsToJson(diagnostics)},
    };
}

nlohmann::json AiAssistantPanel::buildApplyPreviewSnapshot() const {
    if (current_task_plan_.id.empty()) {
        return {{"would_apply", false}, {"diagnostics", nlohmann::json::array()}};
    }
    const auto result = knowledge_.tools.applyApprovedPlan(current_task_plan_, project_data_);
    return {
        {"would_apply", result.applied},
        {"diagnostics", result.toJson()["diagnostics"]},
        {"project_patch", result.project_patch},
        {"revert_patch", result.revert_patch},
    };
}

} // namespace urpg::editor
