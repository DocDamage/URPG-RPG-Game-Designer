#include "engine/core/plugin/plugin_compatibility_score.h"

#include <algorithm>
#include <functional>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace urpg::plugin {

namespace {

std::string stringValue(const nlohmann::json& object, std::initializer_list<const char*> keys, std::string fallback = {}) {
    if (!object.is_object()) {
        return fallback;
    }
    for (const char* key : keys) {
        const auto it = object.find(key);
        if (it != object.end() && it->is_string()) {
            return it->get<std::string>();
        }
    }
    return fallback;
}

std::vector<std::string> stringArrayValue(const nlohmann::json& object, std::initializer_list<const char*> keys) {
    std::vector<std::string> values;
    if (!object.is_object()) {
        return values;
    }
    for (const char* key : keys) {
        const auto it = object.find(key);
        if (it == object.end() || !it->is_array()) {
            continue;
        }
        for (const auto& row : *it) {
            if (row.is_string()) {
                values.push_back(row.get<std::string>());
            }
        }
    }
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    return values;
}

std::string canonicalPluginId(const nlohmann::json& manifest_json, const std::string& source_path) {
    std::string id = stringValue(manifest_json, {"id", "pluginId", "name"});
    if (!id.empty()) {
        return id;
    }
    if (!source_path.empty()) {
        return std::filesystem::path(source_path).stem().string();
    }
    return {};
}

PluginCompatibilityTier tierForScore(int32_t score) {
    if (score >= 90) {
        return PluginCompatibilityTier::Compatible;
    }
    if (score >= 65) {
        return PluginCompatibilityTier::Partial;
    }
    if (score >= 35) {
        return PluginCompatibilityTier::Risky;
    }
    return PluginCompatibilityTier::Unsupported;
}

void appendIssue(PluginCompatibilityResult& result,
                 PluginCompatibilityIssueKind kind,
                 std::string code,
                 std::string message,
                 std::string target,
                 bool blocking,
                 std::optional<PluginShimHint> shim_hint = std::nullopt) {
    PluginCompatibilityIssue issue;
    issue.kind = kind;
    issue.plugin_id = result.plugin_id;
    issue.code = std::move(code);
    issue.message = std::move(message);
    issue.target = std::move(target);
    issue.blocking = blocking;
    issue.shim_hint = std::move(shim_hint);
    result.issues.push_back(std::move(issue));
}

void lowerScore(PluginCompatibilityResult& result, int32_t amount) {
    result.score = std::max<int32_t>(0, result.score - amount);
}

void sortIssueVectors(PluginCompatibilityResult& result) {
    auto sortUnique = [](std::vector<std::string>& values) {
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(), values.end()), values.end());
    };

    sortUnique(result.dependencies);
    sortUnique(result.missing_dependencies);
    sortUnique(result.denied_permissions);
    sortUnique(result.unsupported_apis);

    std::sort(result.shim_hints.begin(), result.shim_hints.end(), [](const auto& a, const auto& b) {
        return a.api < b.api;
    });

    std::sort(result.issues.begin(), result.issues.end(), [](const auto& a, const auto& b) {
        if (a.blocking != b.blocking) {
            return a.blocking > b.blocking;
        }
        if (a.code != b.code) {
            return a.code < b.code;
        }
        return a.target < b.target;
    });
}

std::vector<std::vector<std::string>> detectCycles(const std::vector<PluginDependencyEdge>& edges) {
    std::map<std::string, std::vector<std::string>> adjacency;
    for (const auto& edge : edges) {
        if (!edge.missing && !edge.optional) {
            adjacency[edge.from_plugin].push_back(edge.to_plugin);
        }
    }
    for (auto& [_, neighbors] : adjacency) {
        std::sort(neighbors.begin(), neighbors.end());
    }

    std::vector<std::vector<std::string>> cycles;
    std::vector<std::string> stack;
    std::unordered_set<std::string> emitted;

    const auto canonicalCycleKey = [](std::vector<std::string> cycle) {
        if (cycle.empty()) {
            return std::string{};
        }
        auto min_it = std::min_element(cycle.begin(), cycle.end());
        std::rotate(cycle.begin(), min_it, cycle.end());
        std::ostringstream key;
        for (const auto& plugin : cycle) {
            key << plugin << '\n';
        }
        return key.str();
    };

    std::function<void(const std::string&)> visit = [&](const std::string& node) {
        const auto stack_it = std::find(stack.begin(), stack.end(), node);
        if (stack_it != stack.end()) {
            std::vector<std::string> cycle(stack_it, stack.end());
            const std::string key = canonicalCycleKey(cycle);
            if (emitted.insert(key).second) {
                auto min_it = std::min_element(cycle.begin(), cycle.end());
                std::rotate(cycle.begin(), min_it, cycle.end());
                cycles.push_back(std::move(cycle));
            }
            return;
        }

        stack.push_back(node);
        if (const auto it = adjacency.find(node); it != adjacency.end()) {
            for (const auto& next : it->second) {
                visit(next);
            }
        }
        stack.pop_back();
    };

    for (const auto& [node, _] : adjacency) {
        visit(node);
    }

    std::sort(cycles.begin(), cycles.end());
    return cycles;
}

std::vector<std::string> computeLoadOrder(const std::vector<PluginCompatibilityManifest>& manifests,
                                          const std::vector<PluginDependencyEdge>& edges,
                                          const std::vector<std::vector<std::string>>& cycles) {
    std::set<std::string> cyclic_plugins;
    for (const auto& cycle : cycles) {
        cyclic_plugins.insert(cycle.begin(), cycle.end());
    }

    std::map<std::string, std::set<std::string>> outgoing;
    std::map<std::string, int32_t> indegree;
    for (const auto& manifest : manifests) {
        if (!manifest.plugin_id.empty()) {
            indegree.try_emplace(manifest.plugin_id, 0);
        }
    }
    for (const auto& edge : edges) {
        if (edge.missing || edge.optional || cyclic_plugins.contains(edge.from_plugin) || cyclic_plugins.contains(edge.to_plugin)) {
            continue;
        }
        if (outgoing[edge.to_plugin].insert(edge.from_plugin).second) {
            ++indegree[edge.from_plugin];
        }
    }

    std::set<std::string> ready;
    for (const auto& [plugin, degree] : indegree) {
        if (degree == 0) {
            ready.insert(plugin);
        }
    }

    std::vector<std::string> order;
    while (!ready.empty()) {
        const std::string plugin = *ready.begin();
        ready.erase(ready.begin());
        order.push_back(plugin);
        for (const auto& dependent : outgoing[plugin]) {
            --indegree[dependent];
            if (indegree[dependent] == 0) {
                ready.insert(dependent);
            }
        }
    }

    for (const auto& plugin : cyclic_plugins) {
        order.push_back(plugin);
    }
    return order;
}

void ingestFailureDiagnostics(const std::string& jsonl, std::unordered_map<std::string, PluginCompatibilityResult*>& by_id) {
    std::istringstream stream(jsonl);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = nlohmann::json::parse(line, nullptr, false);
        if (row.is_discarded() || !row.is_object() ||
            row.value("subsystem", "") != "plugin_manager" ||
            row.value("event", "") != "compat_failure") {
            continue;
        }

        const std::string plugin = row.value("plugin", "");
        const auto it = by_id.find(plugin);
        if (it == by_id.end()) {
            continue;
        }

        auto& result = *it->second;
        const std::string severity = row.value("severity", "HARD_FAIL");
        const bool blocking = severity != "WARN";
        lowerScore(result, blocking ? 30 : 10);
        appendIssue(
            result,
            PluginCompatibilityIssueKind::FailureDiagnostic,
            blocking ? "plugin_failure" : "plugin_warning",
            row.value("message", "Plugin manager failure diagnostic"),
            row.value("operation", ""),
            blocking
        );
    }
}

} // namespace

PluginCompatibilityManifest ParsePluginCompatibilityManifest(const nlohmann::json& manifest_json,
                                                             std::string source_path) {
    PluginCompatibilityManifest manifest;
    manifest.source_path = std::move(source_path);
    if (!manifest_json.is_object()) {
        manifest.malformed = true;
        manifest.malformed_reason = "Manifest JSON must be an object.";
        return manifest;
    }

    manifest.plugin_id = canonicalPluginId(manifest_json, manifest.source_path);
    manifest.name = stringValue(manifest_json, {"name"}, manifest.plugin_id);
    manifest.version = stringValue(manifest_json, {"version"});
    manifest.enabled = manifest_json.value("enabled", true);
    manifest.permissions = stringArrayValue(manifest_json, {"permissions", "requestedPermissions"});
    manifest.used_apis = stringArrayValue(manifest_json, {"unsupportedApis", "usedApis", "apiCalls"});
    manifest.fixture_only_behaviors = stringArrayValue(manifest_json, {"fixtureOnlyBehaviors", "fixture_only_behaviors"});
    manifest.fallback_paths = stringArrayValue(manifest_json, {"fallbackPaths", "fallback_paths"});
    manifest.overridden_methods = stringArrayValue(manifest_json, {"overrides", "overriddenMethods", "overridden_methods"});
    manifest.allowed_missing_dependencies =
        stringArrayValue(manifest_json.value("healthExpectations", nlohmann::json::object()), {"allowMissingDependencies"});

    if (manifest.plugin_id.empty()) {
        manifest.malformed = true;
        manifest.malformed_reason = "Manifest is missing plugin id/name.";
    }

    if (const auto deps_it = manifest_json.find("dependencies");
        deps_it != manifest_json.end()) {
        if (!deps_it->is_array()) {
            manifest.malformed = true;
            manifest.malformed_reason = "Manifest dependencies must be an array.";
        } else {
            for (const auto& dep : *deps_it) {
                if (dep.is_string()) {
                    manifest.dependencies.push_back({dep.get<std::string>(), "*", false});
                } else if (dep.is_object()) {
                    manifest.dependencies.push_back({
                        stringValue(dep, {"id", "pluginId", "name"}),
                        stringValue(dep, {"version", "versionRange"}, "*"),
                        dep.value("optional", false),
                    });
                }
            }
        }
    }

    std::sort(manifest.dependencies.begin(), manifest.dependencies.end(), [](const auto& a, const auto& b) {
        return a.plugin_id < b.plugin_id;
    });
    return manifest;
}

std::vector<PluginCompatibilityManifest> LoadPluginCompatibilityManifestsFromDirectory(
    const std::filesystem::path& directory,
    std::string* error_message
) {
    std::vector<PluginCompatibilityManifest> manifests;
    if (error_message) {
        error_message->clear();
    }
    if (!std::filesystem::exists(directory)) {
        if (error_message) {
            *error_message = "Plugin manifest directory not found: " + directory.string();
        }
        return manifests;
    }

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());

    for (const auto& file : files) {
        std::ifstream input(file);
        if (!input.is_open()) {
            PluginCompatibilityManifest manifest;
            manifest.plugin_id = file.stem().string();
            manifest.name = manifest.plugin_id;
            manifest.source_path = file.string();
            manifest.malformed = true;
            manifest.malformed_reason = "Unable to open manifest.";
            manifests.push_back(std::move(manifest));
            continue;
        }
        const auto json = nlohmann::json::parse(input, nullptr, false);
        if (json.is_discarded()) {
            PluginCompatibilityManifest manifest;
            manifest.plugin_id = file.stem().string();
            manifest.name = manifest.plugin_id;
            manifest.source_path = file.string();
            manifest.malformed = true;
            manifest.malformed_reason = "Unable to parse manifest JSON.";
            manifests.push_back(std::move(manifest));
            continue;
        }
        manifests.push_back(ParsePluginCompatibilityManifest(json, file.string()));
    }

    return manifests;
}

PluginCompatibilityReport AnalyzePluginCompatibility(const PluginCompatibilityAnalysisInput& input) {
    PluginCompatibilityReport report;
    std::set<std::string> available_plugins;
    std::map<std::string, std::vector<std::string>> override_owners;

    for (const auto& manifest : input.manifests) {
        if (!manifest.plugin_id.empty()) {
            available_plugins.insert(manifest.plugin_id);
        }
        for (const auto& method : manifest.overridden_methods) {
            override_owners[method].push_back(manifest.plugin_id);
        }
    }

    for (const auto& manifest : input.manifests) {
        PluginCompatibilityResult result;
        result.plugin_id = manifest.plugin_id;
        result.name = manifest.name.empty() ? manifest.plugin_id : manifest.name;

        if (manifest.malformed) {
            result.score = 0;
            appendIssue(
                result,
                PluginCompatibilityIssueKind::MalformedManifest,
                "malformed_manifest",
                manifest.malformed_reason.empty() ? "Manifest is malformed." : manifest.malformed_reason,
                manifest.source_path,
                true
            );
        }

        for (const auto& dependency : manifest.dependencies) {
            if (dependency.plugin_id.empty()) {
                continue;
            }
            result.dependencies.push_back(dependency.plugin_id);
            const bool missing = !available_plugins.contains(dependency.plugin_id);
            const bool profile_allowed_missing =
                std::binary_search(
                    manifest.allowed_missing_dependencies.begin(),
                    manifest.allowed_missing_dependencies.end(),
                    dependency.plugin_id
                );
            report.dependency_edges.push_back({
                manifest.plugin_id,
                dependency.plugin_id,
                missing,
                dependency.optional || profile_allowed_missing,
            });
            if (missing && !dependency.optional && !profile_allowed_missing) {
                result.missing_dependencies.push_back(dependency.plugin_id);
                lowerScore(result, 35);
                appendIssue(
                    result,
                    PluginCompatibilityIssueKind::MissingDependency,
                    "missing_dependency",
                    "Required plugin dependency is missing: " + dependency.plugin_id,
                    dependency.plugin_id,
                    false
                );
            } else if (missing && profile_allowed_missing) {
                appendIssue(
                    result,
                    PluginCompatibilityIssueKind::FallbackPath,
                    "missing_dependency_profile_allowed",
                    "Declared dependency is missing but allowed by this fixture/profile: " + dependency.plugin_id,
                    dependency.plugin_id,
                    false
                );
            }
        }

        for (const auto& permission : manifest.permissions) {
            if (!input.granted_permissions.contains(permission)) {
                result.denied_permissions.push_back(permission);
                lowerScore(result, 45);
                appendIssue(
                    result,
                    PluginCompatibilityIssueKind::PermissionDenied,
                    "permission_denied",
                    "Plugin requests permission not granted by the sandbox: " + permission,
                    permission,
                    true
                );
            }
        }

        for (const auto& api : manifest.used_apis) {
            result.unsupported_apis.push_back(api);
            lowerScore(result, 30);
            std::optional<PluginShimHint> hint;
            if (const auto hint_it = input.native_shim_hints.find(api);
                hint_it != input.native_shim_hints.end()) {
                hint = hint_it->second;
                result.shim_hints.push_back(*hint);
            }
            appendIssue(
                result,
                PluginCompatibilityIssueKind::UnsupportedApi,
                "unsupported_api",
                "Unsupported JavaScript API observed: " + api,
                api,
                false,
                hint
            );
        }

        for (const auto& behavior : manifest.fixture_only_behaviors) {
            lowerScore(result, 15);
            appendIssue(
                result,
                PluginCompatibilityIssueKind::FixtureOnlyBehavior,
                "fixture_only_behavior",
                "Behavior is covered by fixture only, not a live runtime implementation: " + behavior,
                behavior,
                false
            );
        }

        for (const auto& fallback : manifest.fallback_paths) {
            lowerScore(result, 10);
            appendIssue(
                result,
                PluginCompatibilityIssueKind::FallbackPath,
                "fallback_path",
                "Plugin uses a compatibility fallback path: " + fallback,
                fallback,
                false
            );
        }

        sortIssueVectors(result);
        report.plugins.push_back(std::move(result));
    }

    report.dependency_cycles = detectCycles(report.dependency_edges);
    for (const auto& cycle : report.dependency_cycles) {
        for (const auto& plugin_id : cycle) {
            auto it = std::find_if(report.plugins.begin(), report.plugins.end(), [&](const auto& result) {
                return result.plugin_id == plugin_id;
            });
            if (it == report.plugins.end()) {
                continue;
            }
            lowerScore(*it, 40);
            appendIssue(
                *it,
                PluginCompatibilityIssueKind::LoadOrderCycle,
                "load_order_cycle",
                "Plugin participates in a dependency load-order cycle.",
                plugin_id,
                true
            );
        }
    }

    for (auto& [method, owners] : override_owners) {
        std::sort(owners.begin(), owners.end());
        owners.erase(std::unique(owners.begin(), owners.end()), owners.end());
        if (owners.size() < 2) {
            continue;
        }
        report.override_conflicts.push_back(owners);
        for (const auto& plugin_id : owners) {
            auto it = std::find_if(report.plugins.begin(), report.plugins.end(), [&](const auto& result) {
                return result.plugin_id == plugin_id;
            });
            if (it == report.plugins.end()) {
                continue;
            }
            lowerScore(*it, 20);
            appendIssue(
                *it,
                PluginCompatibilityIssueKind::OverrideConflict,
                "override_conflict",
                "Multiple plugins override the same RPG Maker method: " + method,
                method,
                false
            );
        }
    }

    std::unordered_map<std::string, PluginCompatibilityResult*> by_id;
    for (auto& result : report.plugins) {
        by_id[result.plugin_id] = &result;
    }
    ingestFailureDiagnostics(input.failure_diagnostics_jsonl, by_id);

    for (auto& result : report.plugins) {
        result.tier = tierForScore(result.score);
        if (std::any_of(result.issues.begin(), result.issues.end(), [](const auto& issue) {
                return issue.kind == PluginCompatibilityIssueKind::MalformedManifest;
            })) {
            result.tier = PluginCompatibilityTier::Unsupported;
        }
        sortIssueVectors(result);
    }

    std::sort(report.plugins.begin(), report.plugins.end(), [](const auto& a, const auto& b) {
        if (a.tier != b.tier) {
            return static_cast<int>(a.tier) > static_cast<int>(b.tier);
        }
        if (a.score != b.score) {
            return a.score < b.score;
        }
        return a.plugin_id < b.plugin_id;
    });
    std::sort(report.dependency_edges.begin(), report.dependency_edges.end(), [](const auto& a, const auto& b) {
        if (a.from_plugin != b.from_plugin) {
            return a.from_plugin < b.from_plugin;
        }
        return a.to_plugin < b.to_plugin;
    });
    std::sort(report.override_conflicts.begin(), report.override_conflicts.end());
    report.load_order = computeLoadOrder(input.manifests, report.dependency_edges, report.dependency_cycles);

    if (!report.plugins.empty()) {
        int32_t total = 0;
        for (const auto& result : report.plugins) {
            total += result.score;
        }
        report.project_score = total / static_cast<int32_t>(report.plugins.size());
    }
    report.release_authoritative = false;
    return report;
}

std::map<std::string, PluginShimHint> DefaultNativePluginShimHints() {
    return {
        {"AudioManager.playBgm", {"AudioManager.playBgm", "engine/core/audio", "Use the native audio playback bridge for BGM cues."}},
        {"BattleManager.startBattle", {"BattleManager.startBattle", "engine/core/battle", "Use the native battle scene and battle runtime entry points."}},
        {"DataManager.saveGame", {"DataManager.saveGame", "engine/core/save", "Use save catalog serialization and save integrity recovery."}},
        {"PluginManager.registerCommand", {"PluginManager.registerCommand", "runtimes/compat_js/plugin_manager", "Use the fixture-backed plugin command bridge."}},
        {"Window_Base.drawText", {"Window_Base.drawText", "engine/core/message", "Use native message text layout and presentation rendering."}},
    };
}

nlohmann::json PluginCompatibilityReportToJson(const PluginCompatibilityReport& report) {
    nlohmann::json root;
    root["release_authoritative"] = report.release_authoritative;
    root["project_score"] = report.project_score;

    root["plugins"] = nlohmann::json::array();
    for (const auto& plugin : report.plugins) {
        nlohmann::json plugin_json;
        plugin_json["plugin_id"] = plugin.plugin_id;
        plugin_json["name"] = plugin.name;
        plugin_json["score"] = plugin.score;
        plugin_json["tier"] = ToString(plugin.tier);
        plugin_json["dependencies"] = plugin.dependencies;
        plugin_json["missing_dependencies"] = plugin.missing_dependencies;
        plugin_json["denied_permissions"] = plugin.denied_permissions;
        plugin_json["unsupported_apis"] = plugin.unsupported_apis;

        plugin_json["shim_hints"] = nlohmann::json::array();
        for (const auto& hint : plugin.shim_hints) {
            plugin_json["shim_hints"].push_back({
                {"api", hint.api},
                {"native_feature", hint.native_feature},
                {"note", hint.note},
            });
        }

        plugin_json["issues"] = nlohmann::json::array();
        for (const auto& issue : plugin.issues) {
            nlohmann::json issue_json{
                {"kind", ToString(issue.kind)},
                {"code", issue.code},
                {"message", issue.message},
                {"target", issue.target},
                {"blocking", issue.blocking},
            };
            if (issue.shim_hint.has_value()) {
                issue_json["shim_hint"] = {
                    {"api", issue.shim_hint->api},
                    {"native_feature", issue.shim_hint->native_feature},
                    {"note", issue.shim_hint->note},
                };
            }
            plugin_json["issues"].push_back(std::move(issue_json));
        }

        root["plugins"].push_back(std::move(plugin_json));
    }

    root["dependency_edges"] = nlohmann::json::array();
    for (const auto& edge : report.dependency_edges) {
        root["dependency_edges"].push_back({
            {"from_plugin", edge.from_plugin},
            {"to_plugin", edge.to_plugin},
            {"missing", edge.missing},
            {"optional", edge.optional},
        });
    }
    root["dependency_cycles"] = report.dependency_cycles;
    root["override_conflicts"] = report.override_conflicts;
    root["load_order"] = report.load_order;
    return root;
}

const char* ToString(PluginCompatibilityTier tier) {
    switch (tier) {
    case PluginCompatibilityTier::Compatible:
        return "compatible";
    case PluginCompatibilityTier::Partial:
        return "partial";
    case PluginCompatibilityTier::Risky:
        return "risky";
    case PluginCompatibilityTier::Unsupported:
        return "unsupported";
    }
    return "unknown";
}

const char* ToString(PluginCompatibilityIssueKind kind) {
    switch (kind) {
    case PluginCompatibilityIssueKind::MalformedManifest:
        return "malformed_manifest";
    case PluginCompatibilityIssueKind::MissingDependency:
        return "missing_dependency";
    case PluginCompatibilityIssueKind::PermissionDenied:
        return "permission_denied";
    case PluginCompatibilityIssueKind::UnsupportedApi:
        return "unsupported_api";
    case PluginCompatibilityIssueKind::FixtureOnlyBehavior:
        return "fixture_only_behavior";
    case PluginCompatibilityIssueKind::FallbackPath:
        return "fallback_path";
    case PluginCompatibilityIssueKind::LoadOrderCycle:
        return "load_order_cycle";
    case PluginCompatibilityIssueKind::OverrideConflict:
        return "override_conflict";
    case PluginCompatibilityIssueKind::FailureDiagnostic:
        return "failure_diagnostic";
    }
    return "unknown";
}

} // namespace urpg::plugin
