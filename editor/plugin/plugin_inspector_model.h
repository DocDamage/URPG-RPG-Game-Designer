#pragma once

#include "engine/core/plugin/plugin_compatibility_score.h"

#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::editor {

struct PluginInspectorSnapshot {
    bool has_data = false;
    size_t plugin_count = 0;
    size_t issue_count = 0;
    size_t missing_dependency_count = 0;
    size_t permission_denial_count = 0;
    size_t unsupported_api_count = 0;
    size_t cycle_count = 0;
    size_t shim_hint_count = 0;
    int32_t estimated_repair_minutes = 0;
    size_t low_confidence_plugin_count = 0;
    int32_t project_score = 100;
    bool release_authoritative = false;
    bool inspection_only = false;
    std::string source_kind;
    std::vector<std::string> discovery_diagnostics;
};

class PluginInspectorModel {
public:
    void analyze(const plugin::PluginCompatibilityAnalysisInput& input);
    bool loadManifestsFromDirectory(const std::filesystem::path& directory, std::string* error_message = nullptr);
    bool inspectMzPluginScriptsFromDirectory(const std::filesystem::path& directory,
                                             std::string* error_message = nullptr);
    void clear();
    nlohmann::json exportSnapshotJson() const;

    const plugin::PluginCompatibilityReport& report() const { return report_; }
    const PluginInspectorSnapshot& snapshot() const { return snapshot_; }

private:
    void refreshSnapshot();

    plugin::PluginCompatibilityReport report_{};
    PluginInspectorSnapshot snapshot_{};
    bool inspection_only_ = false;
    std::string source_kind_;
    std::vector<std::string> discovery_diagnostics_;
};

} // namespace urpg::editor
