#pragma once

#include "engine/core/compat/mz_event_command_coverage.h"
#include "engine/core/compat/mz_project_compatibility_report.h"
#include "engine/core/compat/mz_visual_diff_report.h"
#include "engine/core/plugin/plugin_compatibility_score.h"

#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

namespace urpg::editor {

struct MzMigrationWorkbenchInput {
    plugin::PluginCompatibilityReport plugin_report;
    compat::MzProjectCompatibilityReport project_report;
    compat::MzEventCommandCoverageReport command_report;
    compat::MzVisualDiffReport visual_report;
};

struct MzMigrationWorkbenchSnapshot {
    int32_t project_score = 100;
    int32_t manual_repair_minutes = 0;
    size_t plugin_count = 0;
    size_t low_confidence_plugin_count = 0;
    size_t unsupported_event_command_count = 0;
    size_t visual_diff_scene_count = 0;
    size_t visual_diff_failed_scene_count = 0;
    std::string visual_diff_status = "passed";
    bool can_auto_migrate = false;
    bool release_authoritative = false;

    [[nodiscard]] nlohmann::json toJson() const;
};

MzMigrationWorkbenchSnapshot BuildMzMigrationWorkbenchSnapshot(const MzMigrationWorkbenchInput& input);

}  // namespace urpg::editor
