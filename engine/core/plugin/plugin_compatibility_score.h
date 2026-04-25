#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::plugin {

enum class PluginCompatibilityTier : uint8_t {
    Compatible = 0,
    Partial = 1,
    Risky = 2,
    Unsupported = 3,
};

enum class PluginCompatibilityIssueKind : uint8_t {
    MalformedManifest = 0,
    MissingDependency = 1,
    PermissionDenied = 2,
    UnsupportedApi = 3,
    FixtureOnlyBehavior = 4,
    FallbackPath = 5,
    LoadOrderCycle = 6,
    OverrideConflict = 7,
    FailureDiagnostic = 8,
};

struct PluginCompatibilityDependency {
    std::string plugin_id;
    std::string version_range = "*";
    bool optional = false;
};

struct PluginCompatibilityManifest {
    std::string plugin_id;
    std::string name;
    std::string version;
    std::string source_path;
    bool enabled = true;
    bool malformed = false;
    std::string malformed_reason;
    std::vector<PluginCompatibilityDependency> dependencies;
    std::vector<std::string> permissions;
    std::vector<std::string> used_apis;
    std::vector<std::string> fixture_only_behaviors;
    std::vector<std::string> fallback_paths;
    std::vector<std::string> overridden_methods;
    std::vector<std::string> allowed_missing_dependencies;
};

struct PluginShimHint {
    std::string api;
    std::string native_feature;
    std::string note;
};

struct PluginCompatibilityIssue {
    PluginCompatibilityIssueKind kind = PluginCompatibilityIssueKind::UnsupportedApi;
    std::string plugin_id;
    std::string code;
    std::string message;
    std::string target;
    bool blocking = false;
    std::optional<PluginShimHint> shim_hint;
};

struct PluginCompatibilityResult {
    std::string plugin_id;
    std::string name;
    int32_t score = 100;
    PluginCompatibilityTier tier = PluginCompatibilityTier::Compatible;
    std::vector<PluginCompatibilityIssue> issues;
    std::vector<std::string> dependencies;
    std::vector<std::string> missing_dependencies;
    std::vector<std::string> denied_permissions;
    std::vector<std::string> unsupported_apis;
    std::vector<PluginShimHint> shim_hints;
};

struct PluginDependencyEdge {
    std::string from_plugin;
    std::string to_plugin;
    bool missing = false;
    bool optional = false;
};

struct PluginCompatibilityReport {
    std::vector<PluginCompatibilityResult> plugins;
    std::vector<PluginDependencyEdge> dependency_edges;
    std::vector<std::vector<std::string>> dependency_cycles;
    std::vector<std::vector<std::string>> override_conflicts;
    std::vector<std::string> load_order;
    int32_t project_score = 100;
    bool release_authoritative = false;
};

struct PluginCompatibilityAnalysisInput {
    std::vector<PluginCompatibilityManifest> manifests;
    std::set<std::string> granted_permissions;
    std::map<std::string, PluginShimHint> native_shim_hints;
    std::string failure_diagnostics_jsonl;
};

PluginCompatibilityManifest ParsePluginCompatibilityManifest(const nlohmann::json& manifest_json,
                                                             std::string source_path = {});
std::vector<PluginCompatibilityManifest> LoadPluginCompatibilityManifestsFromDirectory(
    const std::filesystem::path& directory,
    std::string* error_message = nullptr
);

PluginCompatibilityReport AnalyzePluginCompatibility(const PluginCompatibilityAnalysisInput& input);

std::map<std::string, PluginShimHint> DefaultNativePluginShimHints();
nlohmann::json PluginCompatibilityReportToJson(const PluginCompatibilityReport& report);

const char* ToString(PluginCompatibilityTier tier);
const char* ToString(PluginCompatibilityIssueKind kind);

} // namespace urpg::plugin
