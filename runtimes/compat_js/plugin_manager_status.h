#pragma once

#include "quickjs_runtime.h"

#include <string>
#include <unordered_map>

namespace urpg::compat::plugin_manager_detail {

void initializePluginManagerMethodStatus(
    std::unordered_map<std::string, CompatStatus>& methodStatus,
    std::unordered_map<std::string, std::string>& methodDeviations);

} // namespace urpg::compat::plugin_manager_detail
