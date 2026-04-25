#include "editor/plugin/plugin_inspector_model.h"

namespace urpg::editor {

void PluginInspectorModel::analyze(const plugin::PluginCompatibilityAnalysisInput& input) {
    report_ = plugin::AnalyzePluginCompatibility(input);
    refreshSnapshot();
}

bool PluginInspectorModel::loadManifestsFromDirectory(const std::filesystem::path& directory, std::string* error_message) {
    plugin::PluginCompatibilityAnalysisInput input;
    input.manifests = plugin::LoadPluginCompatibilityManifestsFromDirectory(directory, error_message);
    input.native_shim_hints = plugin::DefaultNativePluginShimHints();
    if (input.manifests.empty()) {
        clear();
        return error_message == nullptr || error_message->empty();
    }
    analyze(input);
    return true;
}

void PluginInspectorModel::clear() {
    report_ = {};
    snapshot_ = {};
}

nlohmann::json PluginInspectorModel::exportSnapshotJson() const {
    nlohmann::json root = plugin::PluginCompatibilityReportToJson(report_);
    root["snapshot"] = {
        {"has_data", snapshot_.has_data},
        {"plugin_count", snapshot_.plugin_count},
        {"issue_count", snapshot_.issue_count},
        {"missing_dependency_count", snapshot_.missing_dependency_count},
        {"permission_denial_count", snapshot_.permission_denial_count},
        {"unsupported_api_count", snapshot_.unsupported_api_count},
        {"cycle_count", snapshot_.cycle_count},
        {"shim_hint_count", snapshot_.shim_hint_count},
        {"project_score", snapshot_.project_score},
        {"release_authoritative", snapshot_.release_authoritative},
    };
    return root;
}

void PluginInspectorModel::refreshSnapshot() {
    snapshot_ = {};
    snapshot_.has_data = !report_.plugins.empty();
    snapshot_.plugin_count = report_.plugins.size();
    snapshot_.cycle_count = report_.dependency_cycles.size();
    snapshot_.project_score = report_.project_score;
    snapshot_.release_authoritative = report_.release_authoritative;

    for (const auto& plugin : report_.plugins) {
        snapshot_.issue_count += plugin.issues.size();
        snapshot_.missing_dependency_count += plugin.missing_dependencies.size();
        snapshot_.permission_denial_count += plugin.denied_permissions.size();
        snapshot_.unsupported_api_count += plugin.unsupported_apis.size();
        snapshot_.shim_hint_count += plugin.shim_hints.size();
    }
}

} // namespace urpg::editor
