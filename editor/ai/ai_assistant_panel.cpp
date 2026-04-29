#include "editor/ai/ai_assistant_panel.h"

#include "engine/core/ai/wysiwyg_chatbot_coverage.h"
#include "engine/core/assets/asset_action_view.h"

#include <utility>

namespace {

nlohmann::json diagnosticsToJson(const std::vector<urpg::ai::AiKnowledgeDiagnostic>& diagnostics) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& item : diagnostics) {
        out.push_back({{"code", item.code}, {"message", item.message}, {"target", item.target}});
    }
    return out;
}

std::string patchPathRoot(const std::string& path) {
    if (path.empty() || path == "/") {
        return "/";
    }
    const auto next = path.find('/', 1);
    return next == std::string::npos ? path.substr(1) : path.substr(1, next - 1);
}

std::string patchTone(const std::string& operation) {
    if (operation == "add") {
        return "added";
    }
    if (operation == "remove") {
        return "removed";
    }
    if (operation == "replace") {
        return "changed";
    }
    return "review";
}

nlohmann::json valueAtPointerOrNull(const nlohmann::json& document, const std::string& path) {
    try {
        const nlohmann::json::json_pointer pointer(path);
        if (document.contains(pointer)) {
            return document.at(pointer);
        }
    } catch (const nlohmann::json::exception&) {
        return nullptr;
    }
    return nullptr;
}

} // namespace

namespace urpg::editor {

void AiAssistantPanel::setConfig(urpg::ai::AiAssistantConfig config, bool providerAvailable) {
    config_ = std::move(config);
    provider_available_ = providerAvailable;
}

void AiAssistantPanel::setOpenAiProviderConfig(urpg::ai::OpenAiCompatibleChatConfig config,
                                               std::string selectedProviderId) {
    provider_config_ = std::move(config);
    if (!selectedProviderId.empty()) {
        selected_provider_id_ = std::move(selectedProviderId);
    }
}

void AiAssistantPanel::setSuggestion(urpg::ai::AiSuggestionRecord suggestion) {
    suggestion_ = std::move(suggestion);
}

void AiAssistantPanel::setProjectData(nlohmann::json projectData) {
    project_data_ = std::move(projectData);
    rebuildTaskPlan();
}

void AiAssistantPanel::setAssetLibrarySnapshot(urpg::assets::AssetLibrarySnapshot assetLibrarySnapshot) {
    asset_library_snapshot_ = std::move(assetLibrarySnapshot);
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
        {"provider_ui", buildProviderUiSnapshot()},
        {"suggestion", policy.toJson(suggestion_)},
        {"knowledge", {
            {"capability_count", knowledge_.capabilities.capabilities().size()},
            {"project_entry_count", knowledge_.project_index.entries().size()},
            {"doc_entry_count", knowledge_.docs_index.entries().size()},
            {"tool_count", knowledge_.tools.tools().size()},
        }},
        {"wysiwyg_chatbot_coverage",
         urpg::ai::buildWysiwygChatbotCoverageReport(knowledge_, asset_library_snapshot_).toJson()},
        {"asset_preview_rows", urpg::assets::buildAssetPreviewRows(asset_library_snapshot_)},
        {"task_plan", current_task_plan_.toJson()},
        {"approval", knowledge_.tools.approvalManifest(current_task_plan_, knowledge_.capabilities)},
        {"controls", buildControlSnapshot()},
        {"validation", buildValidationSnapshot()},
        {"apply_preview", buildApplyPreviewSnapshot()},
        {"apply_history", buildApplyHistorySnapshot()},
        {"rationale_rows", buildRationaleRows()},
    };
    if (!applied_changes_.empty()) {
        last_render_snapshot_["last_apply"] = applied_changes_.back().toJson();
        const auto diffRows = buildDiffRows(applied_changes_.back());
        last_render_snapshot_["result_diff"] = {
            {"has_changes", !applied_changes_.back().project_patch.empty()},
            {"forward_patch_count", applied_changes_.back().project_patch.size()},
            {"revert_patch_count", applied_changes_.back().revert_patch.size()},
            {"forward_patch", applied_changes_.back().project_patch},
            {"revert_patch", applied_changes_.back().revert_patch},
            {"rows", diffRows},
            {"row_count", diffRows.size()},
        };
    } else {
        last_render_snapshot_["result_diff"] = {
            {"has_changes", false},
            {"forward_patch_count", 0},
            {"revert_patch_count", 0},
            {"forward_patch", nlohmann::json::array()},
            {"revert_patch", nlohmann::json::array()},
            {"rows", nlohmann::json::array()},
            {"row_count", 0},
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
        {"undo_stack", {
            {"available", !applied_changes_.empty()},
            {"count", applied_changes_.size()},
            {"latest_forward_patch_count", applied_changes_.empty() ? 0 : applied_changes_.back().project_patch.size()},
            {"latest_revert_patch_count", applied_changes_.empty() ? 0 : applied_changes_.back().revert_patch.size()},
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
        {"project_patch_count", result.project_patch.size()},
        {"revert_patch_count", result.revert_patch.size()},
        {"project_patch", result.project_patch},
        {"revert_patch", result.revert_patch},
        {"diff_rows", buildDiffRows(result)},
        {"rationale_rows", buildRationaleRows()},
    };
}

nlohmann::json AiAssistantPanel::buildApplyHistorySnapshot() const {
    nlohmann::json entries = nlohmann::json::array();
    for (std::size_t index = 0; index < applied_changes_.size(); ++index) {
        const auto& change = applied_changes_[index];
        entries.push_back({
            {"index", index},
            {"applied", change.applied},
            {"project_patch_count", change.project_patch.size()},
            {"revert_patch_count", change.revert_patch.size()},
            {"diagnostic_count", change.diagnostics.size()},
            {"can_revert", index + 1 == applied_changes_.size()},
            {"project_patch", change.project_patch},
            {"revert_patch", change.revert_patch},
            {"diff_rows", buildDiffRows(change)},
        });
    }
    return {
        {"count", applied_changes_.size()},
        {"can_revert_latest", !applied_changes_.empty()},
        {"entries", entries},
    };
}

nlohmann::json AiAssistantPanel::buildValidationSnapshot() const {
    if (current_task_plan_.id.empty()) {
        return {
            {"valid", false},
            {"diagnostic_count", 0},
            {"diagnostics", nlohmann::json::array()},
            {"blocked_reason", "no_task_plan"},
        };
    }
    const auto diagnostics = knowledge_.tools.validatePlan(current_task_plan_);
    nlohmann::json blockedReason = nullptr;
    if (!diagnostics.empty()) {
        blockedReason = diagnostics.front().code;
    }
    return {
        {"valid", diagnostics.empty()},
        {"diagnostic_count", diagnostics.size()},
        {"diagnostics", diagnosticsToJson(diagnostics)},
        {"blocked_reason", blockedReason},
    };
}

nlohmann::json AiAssistantPanel::buildProviderUiSnapshot() const {
    const auto selected = urpg::ai::openAiCompatibleProviderProfileById(selected_provider_id_);
    const auto selectedConfig = urpg::ai::applyOpenAiCompatibleProviderProfile(provider_config_, selected);
    nlohmann::json profileRows = nlohmann::json::array();
    for (const auto& profile : urpg::ai::openAiCompatibleProviderProfiles()) {
        profileRows.push_back({
            {"id", profile.id},
            {"label", profile.label},
            {"endpoint", profile.endpoint},
            {"default_model", profile.default_model},
            {"selected", profile.id == selected.id},
            {"local_provider", profile.local_provider},
            {"api_key_required", profile.api_key_required},
            {"streaming_supported", profile.streaming_supported},
            {"select_button",
             {
                 {"visible", true},
                 {"enabled", profile.id != selected.id},
                 {"action", "select_openai_provider"},
             }},
        });
    }
    const bool apiKeyMissing = selected.api_key_required && selectedConfig.api_key.empty();
    return {
        {"selected_provider_id", selected.id},
        {"selected_label", selected.label},
        {"endpoint", selectedConfig.endpoint},
        {"model", selectedConfig.model},
        {"execute_live", selectedConfig.execute},
        {"temperature", selectedConfig.temperature},
        {"timeout_seconds", selectedConfig.timeout_seconds},
        {"request_path", selectedConfig.request_path},
        {"response_path", selectedConfig.response_path},
        {"curl_executable", selectedConfig.curl_executable},
        {"api_key_required", selected.api_key_required},
        {"api_key_configured", !selectedConfig.api_key.empty()},
        {"local_provider", selected.local_provider},
        {"streaming_supported", selected.streaming_supported},
        {"streaming_state", selected.streaming_supported ? "available" : "not_yet_wired"},
        {"connection_state", !selectedConfig.execute ? "dry_run" : (apiKeyMissing ? "blocked_missing_api_key" : "ready_to_execute")},
        {"dry_run_button",
         {
             {"visible", true},
             {"enabled", selectedConfig.execute},
             {"action", "set_provider_dry_run"},
         }},
        {"live_execute_toggle",
         {
             {"visible", true},
             {"enabled", !apiKeyMissing || selected.local_provider},
             {"checked", selectedConfig.execute},
             {"action", "toggle_provider_live_execute"},
             {"disabled_reason", apiKeyMissing && !selected.local_provider ? "missing_api_key" : nlohmann::json(nullptr)},
         }},
        {"test_request_button",
         {
             {"visible", true},
             {"enabled", !apiKeyMissing || selected.local_provider},
             {"action", "test_provider_request"},
             {"mode", selectedConfig.execute ? "live" : "dry_run"},
         }},
        {"profiles", profileRows},
    };
}

nlohmann::json AiAssistantPanel::buildRationaleRows() const {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& step : current_task_plan_.steps) {
        const auto* tool = knowledge_.tools.find(step.tool_id);
        const auto* capability = tool == nullptr ? nullptr : knowledge_.capabilities.find(tool->capability_id);
        const std::string state = step.rejected ? "rejected" : (step.approved ? "approved" : "needs_review");
        nlohmann::json projectPaths = nlohmann::json::array();
        if (capability != nullptr) {
            for (const auto& path : capability->project_paths) {
                projectPaths.push_back(path);
            }
        }
        rows.push_back({
            {"step_id", step.id},
            {"tool_id", step.tool_id},
            {"tool_title", tool == nullptr ? "" : tool->title},
            {"capability_id", tool == nullptr ? "" : tool->capability_id},
            {"capability_title", capability == nullptr ? "" : capability->title},
            {"summary", step.summary},
            {"state", state},
            {"mutates_project", tool != nullptr && tool->mutates_project},
            {"requires_approval", tool != nullptr && tool->requires_approval},
            {"project_paths", projectPaths},
            {"arguments", step.arguments},
            {"rationale",
             step.rejected
                 ? "Step was rejected by the user and is blocked from apply."
                 : (step.approved
                        ? "Step is approved for a controlled project-data patch."
                        : "Step requires review before it can modify project data.")},
        });
    }
    return rows;
}

nlohmann::json AiAssistantPanel::buildDiffRows(const urpg::ai::AiToolApplyResult& result) const {
    nlohmann::json rows = nlohmann::json::array();
    if (!result.project_patch.is_array()) {
        return rows;
    }
    for (std::size_t index = 0; index < result.project_patch.size(); ++index) {
        const auto& patch = result.project_patch[index];
        if (!patch.is_object()) {
            continue;
        }
        const std::string op = patch.value("op", "");
        const std::string path = patch.value("path", "");
        rows.push_back({
            {"index", index},
            {"operation", op},
            {"path", path},
            {"root", patchPathRoot(path)},
            {"tone", patchTone(op)},
            {"before", valueAtPointerOrNull(result.before_project_data, path)},
            {"after", valueAtPointerOrNull(result.project_data, path)},
            {"patch", patch},
            {"summary", patchTone(op) + std::string(" ") + path},
        });
    }
    return rows;
}

} // namespace urpg::editor
