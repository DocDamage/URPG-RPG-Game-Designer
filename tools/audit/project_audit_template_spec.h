#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::tools::audit {

std::string templateBarDisplayName(const std::string& barName);
std::vector<std::string> extractTemplateSpecRequiredSubsystems(const std::string& text);
nlohmann::json extractTemplateSpecBars(const std::string& text);
std::string joinItems(const std::vector<std::string>& items);

} // namespace urpg::tools::audit
