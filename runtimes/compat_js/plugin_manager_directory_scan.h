#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::compat::plugin_manager_detail {

struct PluginDirectoryScanResult {
    std::vector<std::filesystem::path> candidates;
    std::string errorOperation;
    std::string errorMessage;

    bool ok() const { return errorOperation.empty(); }
};

PluginDirectoryScanResult scanPluginDirectory(const std::string& directory);

} // namespace urpg::compat::plugin_manager_detail
