#pragma once

#include "engine/core/ai/ai_assistant_config.h"
#include "engine/core/ai/ai_suggestion_record.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class AiAssistantPanel {
public:
    void setConfig(urpg::ai::AiAssistantConfig config, bool providerAvailable);
    void setSuggestion(urpg::ai::AiSuggestionRecord suggestion);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::ai::AiAssistantConfig config_;
    bool provider_available_ = false;
    urpg::ai::AiSuggestionRecord suggestion_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
