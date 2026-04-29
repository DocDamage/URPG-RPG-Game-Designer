#include "editor/ai/ai_assistant_panel.h"

#include "engine/core/ai/wysiwyg_chatbot_coverage.h"
#include "engine/core/assets/asset_action_view.h"

#include <algorithm>
#include <optional>
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

nlohmann::json makeAiChangeRecord(const urpg::ai::AiTaskPlan& plan,
                                  const urpg::ai::AiToolApplyResult& result,
                                  std::size_t index) {
    return {
        {"change_id", "ai_change_" + std::to_string(index + 1)},
        {"plan_id", plan.id},
        {"user_request", plan.user_request},
        {"step_count", plan.steps.size()},
        {"forward_patch", result.project_patch},
        {"revert_patch", result.revert_patch},
        {"before_project_data", result.before_project_data},
        {"after_project_data", result.project_data},
        {"reverted", false},
    };
}

std::optional<std::size_t> latestUnrevertedAiChangeIndex(const nlohmann::json& projectData) {
    if (!projectData.is_object() || !projectData.contains("_ai_change_history") ||
        !projectData["_ai_change_history"].is_array()) {
        return std::nullopt;
    }
    const auto& history = projectData["_ai_change_history"];
    for (std::size_t offset = 0; offset < history.size(); ++offset) {
        const auto index = history.size() - 1U - offset;
        if (!history[index].value("reverted", false)) {
            return index;
        }
    }
    return std::nullopt;
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
    auto result = knowledge_.tools.applyApprovedPlan(current_task_plan_, project_data_);
    project_data_ = result.project_data;
    last_render_snapshot_["last_apply"] = result.toJson();
    if (result.applied) {
        auto& history = project_data_["_ai_change_history"];
        if (!history.is_array()) {
            history = nlohmann::json::array();
        }
        history.push_back(makeAiChangeRecord(current_task_plan_, result, history.size()));
        result.project_data = project_data_;
        applied_changes_.push_back(result);
        knowledge_ = urpg::ai::buildDefaultAiKnowledgeSnapshot(project_data_);
        last_render_snapshot_["last_apply"] = result.toJson();
    }
    return result.applied;
}

bool AiAssistantPanel::revertLastAppliedPlan() {
    const auto changeIndex = latestUnrevertedAiChangeIndex(project_data_);
    if (!changeIndex.has_value()) {
        last_render_snapshot_["last_revert"] = {{"reverted", false}, {"reason", "no_applied_ai_change"}};
        return false;
    }
    const auto change = project_data_["_ai_change_history"][*changeIndex];
    try {
        project_data_ = project_data_.patch(change.value("revert_patch", nlohmann::json::array()));
    } catch (const nlohmann::json::exception&) {
        last_render_snapshot_["last_revert"] = {{"reverted", false}, {"reason", "patch_apply_failed"}};
        return false;
    }
    auto& history = project_data_["_ai_change_history"];
    if (!history.is_array()) {
        history = nlohmann::json::array();
    }
    if (*changeIndex < history.size()) {
        history[*changeIndex]["reverted"] = true;
    } else {
        auto restoredChange = change;
        restoredChange["reverted"] = true;
        history.push_back(std::move(restoredChange));
    }
    if (!applied_changes_.empty()) {
        applied_changes_.pop_back();
    }
    knowledge_ = urpg::ai::buildDefaultAiKnowledgeSnapshot(project_data_);
    last_render_snapshot_["last_revert"] = {
        {"reverted", true},
        {"change_id", change.value("change_id", "")},
        {"revert_patch", change.value("revert_patch", nlohmann::json::array())},
        {"project_data", project_data_},
    };
    return true;
}

bool AiAssistantPanel::selectOpenAiProvider(const std::string& providerId) {
    const auto profiles = urpg::ai::openAiCompatibleProviderProfiles();
    const auto found = std::find_if(profiles.begin(), profiles.end(), [&](const auto& profile) {
        return profile.id == providerId;
    });
    if (found == profiles.end()) {
        last_provider_test_ = {
            {"attempted", false},
            {"success", false},
            {"connection_state", "provider_not_found"},
            {"failure_reason", "provider_not_found"},
            {"provider_id", providerId},
        };
        return false;
    }
    selected_provider_id_ = providerId;
    return true;
}

nlohmann::json AiAssistantPanel::testOpenAiProviderRequest() {
    const auto selected = urpg::ai::openAiCompatibleProviderProfileById(selected_provider_id_);
    const auto selectedConfig = urpg::ai::applyOpenAiCompatibleProviderProfile(provider_config_, selected);
    const bool apiKeyMissing = selected.api_key_required && selectedConfig.api_key.empty();
    if (apiKeyMissing) {
        last_provider_test_ = {
            {"attempted", false},
            {"success", false},
            {"connection_state", "blocked_missing_api_key"},
            {"failure_reason", "missing_api_key"},
            {"provider_id", selected.id},
            {"endpoint", selectedConfig.endpoint},
            {"model", selectedConfig.model},
        };
        return last_provider_test_;
    }
    const auto result = urpg::ai::invokeOpenAiCompatibleChat(
        {{"user", "URPG provider connectivity test."}}, selectedConfig);
    const std::string connectionState =
        !selectedConfig.execute ? "dry_run" : (result.success ? "connected" : "failed");
    const std::string failureReason = result.success ? "" : result.message;
    last_provider_test_ = result.toJson();
    last_provider_test_["connection_state"] = connectionState;
    last_provider_test_["failure_reason"] = failureReason;
    last_provider_test_["provider_id"] = selected.id;
    last_provider_test_["endpoint"] = selectedConfig.endpoint;
    last_provider_test_["model"] = selectedConfig.model;
    return last_provider_test_;
}

nlohmann::json AiAssistantPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

nlohmann::json AiAssistantPanel::buildControlSnapshot() const {
    const auto diagnostics = knowledge_.tools.validatePlan(current_task_plan_);
    const bool canApply = current_task_plan_.id.empty() ? false : diagnostics.empty();
    const auto approval = knowledge_.tools.approvalManifest(current_task_plan_, knowledge_.capabilities);
    const auto latestChangeIndex = latestUnrevertedAiChangeIndex(project_data_);
    const bool canRevert = latestChangeIndex.has_value();
    const auto history = buildApplyHistorySnapshot();
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
            {"enabled", canRevert},
            {"label", "Revert AI Change"},
            {"action", "revert_last_applied_plan"},
        }},
        {"undo_stack", {
            {"available", canRevert},
            {"count", history.value("count", std::size_t{0})},
            {"latest_change_id", history.value("latest_change_id", nlohmann::json(nullptr))},
            {"latest_forward_patch_count",
             canRevert ? project_data_["_ai_change_history"][*latestChangeIndex].value("forward_patch", nlohmann::json::array()).size() : 0},
            {"latest_revert_patch_count",
             canRevert ? project_data_["_ai_change_history"][*latestChangeIndex].value("revert_patch", nlohmann::json::array()).size() : 0},
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
    nlohmann::json persisted = nlohmann::json::array();
    if (project_data_.is_object() && project_data_.contains("_ai_change_history") &&
        project_data_["_ai_change_history"].is_array()) {
        persisted = project_data_["_ai_change_history"];
    }
    for (std::size_t index = 0; index < persisted.size(); ++index) {
        const auto& record = persisted[index];
        if (record.value("reverted", false)) {
            continue;
        }
        urpg::ai::AiToolApplyResult change;
        change.applied = true;
        change.project_patch = record.value("forward_patch", nlohmann::json::array());
        change.revert_patch = record.value("revert_patch", nlohmann::json::array());
        change.before_project_data = record.value("before_project_data", nlohmann::json::object());
        change.project_data = record.value("after_project_data", nlohmann::json::object());
        entries.push_back({
            {"index", index},
            {"applied", change.applied},
            {"project_patch_count", change.project_patch.size()},
            {"revert_patch_count", change.revert_patch.size()},
            {"diagnostic_count", change.diagnostics.size()},
            {"can_revert", latestUnrevertedAiChangeIndex(project_data_).value_or(index) == index},
            {"change_id", record.value("change_id", "")},
            {"persisted_record", record},
            {"project_patch", change.project_patch},
            {"revert_patch", change.revert_patch},
            {"diff_rows", buildDiffRows(change)},
        });
    }
    const auto latestIndex = latestUnrevertedAiChangeIndex(project_data_);
    const nlohmann::json latestChangeId = latestIndex.has_value() && *latestIndex < persisted.size()
                                              ? nlohmann::json(persisted[*latestIndex].value("change_id", ""))
                                              : nlohmann::json(nullptr);
    return {
        {"count", entries.size()},
        {"can_revert_latest", latestIndex.has_value()},
        {"latest_change_id", latestChangeId},
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
            {"status_badge",
             {
                 {"tone", profile.local_provider ? "local" : "hosted"},
                 {"label", profile.local_provider ? "Local" : "Hosted"},
             }},
            {"select_button",
             {
                 {"visible", true},
                 {"enabled", profile.id != selected.id},
                 {"label", profile.id == selected.id ? "Selected" : "Select"},
                 {"action", "select_openai_provider"},
             }},
        });
    }
    const bool apiKeyMissing = selected.api_key_required && selectedConfig.api_key.empty();
    const bool streamEnabled = selected.streaming_supported && selectedConfig.stream;
    const std::string streamingState =
        streamEnabled ? "streaming_requested" : (selected.streaming_supported ? "available" : "not_supported");
    const std::string connectionState =
        !selectedConfig.execute ? "dry_run" : (apiKeyMissing ? "blocked_missing_api_key" : "ready_to_execute");
    const nlohmann::json failureReason =
        apiKeyMissing ? nlohmann::json("missing_api_key")
                      : (!last_provider_test_.empty() && last_provider_test_.contains("failure_reason")
                             ? last_provider_test_["failure_reason"]
                             : nlohmann::json(nullptr));
    nlohmann::json selectedProfile = selected.toJson();
    selectedProfile["status_badge"] = {
        {"tone", apiKeyMissing ? "blocked" : (selected.local_provider ? "local" : "hosted")},
        {"label", apiKeyMissing ? "Missing API key" : (selected.local_provider ? "Local" : "Hosted")},
    };
    return {
        {"selected_provider_id", selected.id},
        {"selected_label", selected.label},
        {"selected_profile", selectedProfile},
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
        {"streaming_requested", streamEnabled},
        {"streaming_state", streamingState},
        {"connection_state", connectionState},
        {"failure_reason", failureReason},
        {"last_test", last_provider_test_},
        {"stream_toggle",
         {
             {"visible", true},
             {"enabled", selected.streaming_supported},
             {"checked", streamEnabled},
             {"action", "toggle_provider_streaming"},
             {"disabled_reason", selected.streaming_supported ? nlohmann::json(nullptr) : "provider_streaming_not_supported"},
         }},
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
