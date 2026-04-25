#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::ai {

struct AiAssistantConfig {
    bool enabled = false;
    std::string providerId;
    bool allowNetwork = false;
    bool allowGeneratedContent = false;
};

struct AiAssistantStatus {
    bool available = false;
    bool error = false;
    std::string reason;
    nlohmann::json toJson() const;
};

class AiAssistantConfigValidator {
public:
    AiAssistantStatus evaluate(const AiAssistantConfig& config, bool providerAvailable) const;
};

} // namespace urpg::ai
