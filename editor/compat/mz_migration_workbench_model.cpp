#include "editor/compat/mz_migration_workbench_model.h"

#include <algorithm>

namespace urpg::editor {

namespace {

size_t eventLaneUnsupportedCount(const compat::MzProjectCompatibilityReport& report) {
    const auto it = report.lanes.find("events");
    if (it == report.lanes.end()) {
        return 0;
    }
    return static_cast<size_t>(std::max(0, it->second.unsupported_count));
}

std::string visualDiffStatus(const compat::MzVisualDiffReport& report) {
    if (report.failed_scene_count > 0) {
        return "failed";
    }
    return "passed";
}

}  // namespace

nlohmann::json MzMigrationWorkbenchSnapshot::toJson() const {
    return nlohmann::json{{"project_score", project_score},
                          {"manual_repair_minutes", manual_repair_minutes},
                          {"plugin_count", plugin_count},
                          {"low_confidence_plugin_count", low_confidence_plugin_count},
                          {"unsupported_event_command_count", unsupported_event_command_count},
                          {"visual_diff_scene_count", visual_diff_scene_count},
                          {"visual_diff_failed_scene_count", visual_diff_failed_scene_count},
                          {"parity_reference_count", parity_reference_count},
                          {"parity_backend_count", parity_backend_count},
                          {"parity_failed_comparison_count", parity_failed_comparison_count},
                          {"visual_diff_status", visual_diff_status},
                          {"can_auto_migrate", can_auto_migrate},
                          {"release_authoritative", release_authoritative}};
}

MzMigrationWorkbenchSnapshot BuildMzMigrationWorkbenchSnapshot(const MzMigrationWorkbenchInput& input) {
    MzMigrationWorkbenchSnapshot snapshot;
    snapshot.project_score = input.project_report.project_score;
    snapshot.plugin_count = input.plugin_report.plugins.size();
    snapshot.visual_diff_scene_count = input.visual_report.scene_count;
    snapshot.visual_diff_failed_scene_count = input.visual_report.failed_scene_count;
    snapshot.visual_diff_status = visualDiffStatus(input.visual_report);
    snapshot.parity_reference_count = static_cast<size_t>(std::max(0, input.parity_report.reference_count));
    snapshot.parity_backend_count = static_cast<size_t>(std::max(0, input.parity_report.backend_count));
    snapshot.parity_failed_comparison_count =
        static_cast<size_t>(std::max(0, input.parity_report.failed_comparison_count));
    snapshot.unsupported_event_command_count =
        std::max(input.command_report.unsupported_command_count, eventLaneUnsupportedCount(input.project_report));
    snapshot.release_authoritative = input.plugin_report.release_authoritative &&
                                     input.project_report.release_authoritative &&
                                     input.visual_report.release_authoritative &&
                                     input.parity_report.release_authoritative;

    for (const auto& plugin : input.plugin_report.plugins) {
        snapshot.manual_repair_minutes += plugin.estimated_repair_minutes;
        if (plugin.confidence == "low") {
            ++snapshot.low_confidence_plugin_count;
        }
    }

    snapshot.can_auto_migrate = snapshot.project_score >= 90 &&
                                snapshot.unsupported_event_command_count == 0 &&
                                snapshot.manual_repair_minutes == 0 &&
                                snapshot.visual_diff_failed_scene_count == 0 &&
                                snapshot.parity_failed_comparison_count == 0;
    return snapshot;
}

}  // namespace urpg::editor
