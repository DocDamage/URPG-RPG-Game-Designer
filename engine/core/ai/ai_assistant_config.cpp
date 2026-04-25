#include "engine/core/ai/ai_assistant_config.h"

namespace urpg::ai {

nlohmann::json AiAssistantStatus::toJson() const {
    return {
        {"available", available},
        {"error", error},
        {"reason", reason},
    };
}

AiAssistantStatus AiAssistantConfigValidator::evaluate(const AiAssistantConfig& config, bool providerAvailable) const {
    if (!config.enabled) {
        return {false, false, "disabled"};
    }
    if (config.providerId.empty()) {
        return {false, true, "missing_provider"};
    }
    if (!providerAvailable) {
        return {false, false, "provider_unavailable"};
    }
    return {true, false, "ready"};
}

} // namespace urpg::ai
