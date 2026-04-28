#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

namespace urpg::project {

struct TemplateRuntimeProfile {
    std::string id;
    std::string displayName;
    std::string status;
    std::vector<std::string> tags;
    std::vector<std::string> requiredSubsystems;
    std::vector<std::string> loops;
    nlohmann::json bars;
    nlohmann::json systems;
};

std::vector<TemplateRuntimeProfile> allTemplateRuntimeProfiles();
std::optional<TemplateRuntimeProfile> findTemplateRuntimeProfile(const std::string& templateId);
nlohmann::json templateRuntimeProfileToJson(const TemplateRuntimeProfile& profile);
std::vector<std::string> validateTemplateRuntimeProfile(const TemplateRuntimeProfile& profile);

} // namespace urpg::project
