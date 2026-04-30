#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::localization {

struct TemplateLocalizationAuditResult {
    std::string template_id;
    std::size_t required_key_count = 0;
    std::size_t missing_key_count = 0;
    std::size_t missing_font_profile_count = 0;
    std::vector<std::string> missing_keys;
    std::vector<std::string> diagnostics;
};

TemplateLocalizationAuditResult auditTemplateLocalization(const nlohmann::json& templateManifest);

} // namespace urpg::localization
