#pragma once

#include "engine/core/plugin/plugin_compatibility_score.h"

#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

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
    int32_t project_score = 100;
    bool release_authoritative = false;
};

class PluginInspectorModel {
public:
    void analyze(const plugin::PluginCompatibilityAnalysisInput& input);
    bool loadManifestsFromDirectory(const std::filesystem::path& directory, std::string* error_message = nullptr);
    void clear();
    nlohmann::json exportSnapshotJson() const;

    const plugin::PluginCompatibilityReport& report() const { return report_; }
    const PluginInspectorSnapshot& snapshot() const { return snapshot_; }

private:
    void refreshSnapshot();

    plugin::PluginCompatibilityReport report_{};
    PluginInspectorSnapshot snapshot_{};
};

} // namespace urpg::editor
