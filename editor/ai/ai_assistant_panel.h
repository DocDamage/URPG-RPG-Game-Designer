#pragma once

#include "engine/core/ai/ai_assistant_config.h"
#include "engine/core/ai/ai_knowledge_base.h"
#include "engine/core/ai/ai_suggestion_record.h"

#include <nlohmann/json.hpp>

#include <vector>

namespace urpg::editor {

class AiAssistantPanel {
public:
    void setConfig(urpg::ai::AiAssistantConfig config, bool providerAvailable);
    void setSuggestion(urpg::ai::AiSuggestionRecord suggestion);
    void setProjectData(nlohmann::json projectData);
    void setTaskRequest(std::string taskRequest);
    void render();
    bool approveStep(const std::string& stepId);
    std::size_t approveAllPendingSteps();
    bool rejectStep(const std::string& stepId);
    bool applyApprovedPlan();
    bool revertLastAppliedPlan();
    nlohmann::json lastRenderSnapshot() const;

private:
    void rebuildTaskPlan();
    nlohmann::json buildControlSnapshot() const;
    nlohmann::json buildApplyPreviewSnapshot() const;
    nlohmann::json buildApplyHistorySnapshot() const;
    nlohmann::json buildValidationSnapshot() const;

    urpg::ai::AiAssistantConfig config_;
    bool provider_available_ = false;
    urpg::ai::AiSuggestionRecord suggestion_;
    nlohmann::json project_data_ = nlohmann::json::object();
    std::string task_request_;
    urpg::ai::AiKnowledgeSnapshot knowledge_;
    urpg::ai::AiTaskPlan current_task_plan_;
    std::vector<urpg::ai::AiToolApplyResult> applied_changes_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
