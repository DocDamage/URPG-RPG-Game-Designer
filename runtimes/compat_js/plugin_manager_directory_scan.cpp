#include "plugin_manager_directory_scan.h"

#include <algorithm>
#include <string_view>

namespace urpg::compat::plugin_manager_detail {

namespace {

constexpr std::string_view kFailDirectoryScanMarker = "__urpg_fail_directory_scan__";
constexpr std::string_view kFailDirectoryEntryStatusMarker =
    "__urpg_fail_directory_entry_status__";

PluginDirectoryScanResult scanFailure(const std::string& operation, const std::string& message) {
    PluginDirectoryScanResult result;
    result.errorOperation = operation;
    result.errorMessage = message;
    return result;
}

} // namespace

PluginDirectoryScanResult scanPluginDirectory(const std::string& directory) {
    std::error_code ec;
    const std::filesystem::path dirPath(directory);
    if (!std::filesystem::exists(dirPath, ec) || !std::filesystem::is_directory(dirPath, ec)) {
        return scanFailure("load_plugins_directory", "Plugin directory not found: " + directory);
    }

    if (dirPath.filename().string().find(kFailDirectoryScanMarker) != std::string::npos) {
        return scanFailure("load_plugins_directory_scan", "Failed scanning plugin directory: " + directory);
    }

    PluginDirectoryScanResult result;
    std::filesystem::directory_iterator it(dirPath, ec);
    if (ec) {
        return scanFailure("load_plugins_directory_scan", "Failed scanning plugin directory: " + directory);
    }

    for (const auto& entry : it) {
        if (ec) {
            return scanFailure("load_plugins_directory_scan", "Failed scanning plugin directory: " + directory);
        }
        if (entry.path().filename().string().find(kFailDirectoryEntryStatusMarker) != std::string::npos) {
            return scanFailure(
                "load_plugins_directory_scan_entry",
                "Failed reading plugin directory entry: " + entry.path().string());
        }

        std::error_code entryEc;
        const bool isRegular = entry.is_regular_file(entryEc);
        if (entryEc) {
            return scanFailure(
                "load_plugins_directory_scan_entry",
                "Failed reading plugin directory entry: " + entry.path().string());
        }
        if (!isRegular) {
            continue;
        }
        const auto ext = entry.path().extension().string();
        if (ext != ".json" && ext != ".js") {
            continue;
        }
        result.candidates.push_back(entry.path());
    }

    std::sort(
        result.candidates.begin(),
        result.candidates.end(),
        [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
            return lhs.lexically_normal().generic_string() <
                   rhs.lexically_normal().generic_string();
        });

    return result;
}

} // namespace urpg::compat::plugin_manager_detail
