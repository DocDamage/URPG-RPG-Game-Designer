#include "editor/plugin/plugin_inspector_model.h"

namespace urpg::editor {

void PluginInspectorModel::analyze(const plugin::PluginCompatibilityAnalysisInput& input) {
    report_ = plugin::AnalyzePluginCompatibility(input);
    refreshSnapshot();
}

bool PluginInspectorModel::loadManifestsFromDirectory(const std::filesystem::path& directory, std::string* error_message) {
    inspection_only_ = false;
    source_kind_ = "native compatibility manifests";
    discovery_diagnostics_.clear();
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

bool PluginInspectorModel::inspectMzPluginScriptsFromDirectory(const std::filesystem::path& directory,
                                                                std::string* error_message) {
    inspection_only_ = true;
    source_kind_ = "MZ plugin source (static inspection only)";
    const auto inspection = plugin::InspectMzPluginScriptsFromDirectory(directory);
    discovery_diagnostics_ = inspection.diagnostics;
    if (error_message != nullptr) {
        *error_message = discovery_diagnostics_.empty() ? std::string{} : discovery_diagnostics_.front();
    }
    plugin::PluginCompatibilityAnalysisInput input;
    input.manifests = inspection.manifests;
    input.native_shim_hints = plugin::DefaultNativePluginShimHints();
    if (input.manifests.empty()) {
        report_ = {};
        refreshSnapshot();
        return discovery_diagnostics_.empty();
    }
    analyze(input);
    return true;
}

void PluginInspectorModel::clear() {
    report_ = {};
    snapshot_ = {};
    inspection_only_ = false;
    source_kind_.clear();
    discovery_diagnostics_.clear();
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
        {"estimated_repair_minutes", snapshot_.estimated_repair_minutes},
        {"low_confidence_plugin_count", snapshot_.low_confidence_plugin_count},
        {"project_score", snapshot_.project_score},
        {"release_authoritative", snapshot_.release_authoritative},
        {"inspection_only", snapshot_.inspection_only},
        {"source_kind", snapshot_.source_kind},
        {"discovery_diagnostics", snapshot_.discovery_diagnostics},
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
    snapshot_.inspection_only = inspection_only_;
    snapshot_.source_kind = source_kind_;
    snapshot_.discovery_diagnostics = discovery_diagnostics_;

    for (const auto& plugin : report_.plugins) {
        snapshot_.issue_count += plugin.issues.size();
        snapshot_.missing_dependency_count += plugin.missing_dependencies.size();
        snapshot_.permission_denial_count += plugin.denied_permissions.size();
        snapshot_.unsupported_api_count += plugin.unsupported_apis.size();
        snapshot_.shim_hint_count += plugin.shim_hints.size();
        snapshot_.estimated_repair_minutes += plugin.estimated_repair_minutes;
        if (plugin.confidence == "low") {
            ++snapshot_.low_confidence_plugin_count;
        }
    }
}

} // namespace urpg::editor
