#pragma once

#include "engine/core/ai/ai_assistant_config.h"
#include "engine/core/ai/ai_knowledge_base.h"
#include "engine/core/ai/openai_compatible_chat_service.h"
#include "engine/core/ai/ai_suggestion_record.h"
#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

#include <vector>

namespace urpg::editor {

class AiAssistantPanel {
public:
    void setConfig(urpg::ai::AiAssistantConfig config, bool providerAvailable);
    void setOpenAiProviderConfig(urpg::ai::OpenAiCompatibleChatConfig config, std::string selectedProviderId = {});
    void setSuggestion(urpg::ai::AiSuggestionRecord suggestion);
    void setProjectData(nlohmann::json projectData);
    void setAssetLibrarySnapshot(urpg::assets::AssetLibrarySnapshot assetLibrarySnapshot);
    void setTaskRequest(std::string taskRequest);
    void render();
    bool approveStep(const std::string& stepId);
    std::size_t approveAllPendingSteps();
    bool rejectStep(const std::string& stepId);
    bool applyApprovedPlan();
    bool revertLastAppliedPlan();
    bool selectOpenAiProvider(const std::string& providerId);
    nlohmann::json testOpenAiProviderRequest();
    nlohmann::json lastRenderSnapshot() const;

private:
    void rebuildTaskPlan();
    nlohmann::json buildControlSnapshot() const;
    nlohmann::json buildApplyPreviewSnapshot() const;
    nlohmann::json buildApplyHistorySnapshot() const;
    nlohmann::json buildValidationSnapshot() const;
    nlohmann::json buildRationaleRows() const;
    nlohmann::json buildDiffRows(const urpg::ai::AiToolApplyResult& result) const;
    nlohmann::json buildProviderUiSnapshot() const;

    urpg::ai::AiAssistantConfig config_;
    urpg::ai::OpenAiCompatibleChatConfig provider_config_{};
    std::string selected_provider_id_ = "ollama";
    bool provider_available_ = false;
    urpg::ai::AiSuggestionRecord suggestion_;
    nlohmann::json project_data_ = nlohmann::json::object();
    urpg::assets::AssetLibrarySnapshot asset_library_snapshot_{};
    std::string task_request_;
    urpg::ai::AiKnowledgeSnapshot knowledge_;
    urpg::ai::AiTaskPlan current_task_plan_;
    std::vector<urpg::ai::AiToolApplyResult> applied_changes_;
    nlohmann::json last_provider_test_ = nlohmann::json::object();
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
