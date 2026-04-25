#include "editor/ai/ai_assistant_panel.h"

#include <utility>

namespace urpg::editor {

void AiAssistantPanel::setConfig(urpg::ai::AiAssistantConfig config, bool providerAvailable) {
    config_ = std::move(config);
    provider_available_ = providerAvailable;
}

void AiAssistantPanel::setSuggestion(urpg::ai::AiSuggestionRecord suggestion) {
    suggestion_ = std::move(suggestion);
}

void AiAssistantPanel::render() {
    urpg::ai::AiAssistantConfigValidator validator;
    urpg::ai::AiSuggestionPolicy policy;
    last_render_snapshot_ = {
        {"status", validator.evaluate(config_, provider_available_).toJson()},
        {"suggestion", policy.toJson(suggestion_)},
    };
}

nlohmann::json AiAssistantPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
