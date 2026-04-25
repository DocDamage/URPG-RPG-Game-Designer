#include "engine/core/plugin/plugin_compatibility_score.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>

using namespace urpg::plugin;

namespace {

PluginCompatibilityManifest manifest(nlohmann::json json) {
    return ParsePluginCompatibilityManifest(json);
}

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 &&
        sourceRoot.front() == '"' &&
        sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

const PluginCompatibilityResult& findPlugin(const PluginCompatibilityReport& report, const std::string& id) {
    const auto it = std::find_if(report.plugins.begin(), report.plugins.end(), [&](const auto& result) {
        return result.plugin_id == id;
    });
    REQUIRE(it != report.plugins.end());
    return *it;
}

} // namespace

TEST_CASE("PluginCompatibilityScore reports missing dependencies exactly", "[plugin][compatibility][ffs04]") {
    PluginCompatibilityAnalysisInput input;
    input.manifests = {
        manifest({{"name", "Addon"}, {"dependencies", nlohmann::json::array({"CoreEngine", "MissingTarget"})}}),
        manifest({{"name", "CoreEngine"}}),
    };

    const auto report = AnalyzePluginCompatibility(input);

    const auto& addon = findPlugin(report, "Addon");
    REQUIRE(addon.score == 65);
    REQUIRE(addon.tier == PluginCompatibilityTier::Partial);
    REQUIRE(addon.missing_dependencies.size() == 1);
    REQUIRE(addon.missing_dependencies[0] == "MissingTarget");
    REQUIRE(addon.issues[0].code == "missing_dependency");
    REQUIRE(addon.issues[0].target == "MissingTarget");
}

TEST_CASE("PluginCompatibilityScore turns denied sandbox permission into blocking issue", "[plugin][compatibility][ffs04]") {
    PluginCompatibilityAnalysisInput input;
    input.granted_permissions = {"save.read"};
    input.manifests = {
        manifest({{"name", "CloudSave"}, {"permissions", nlohmann::json::array({"save.read", "network.fetch"})}}),
    };

    const auto report = AnalyzePluginCompatibility(input);
    const auto& cloud = findPlugin(report, "CloudSave");

    REQUIRE(cloud.tier == PluginCompatibilityTier::Risky);
    REQUIRE(cloud.denied_permissions.size() == 1);
    REQUIRE(cloud.denied_permissions[0] == "network.fetch");
    REQUIRE(cloud.issues.size() == 1);
    REQUIRE(cloud.issues[0].blocking);
    REQUIRE(cloud.issues[0].kind == PluginCompatibilityIssueKind::PermissionDenied);
}

TEST_CASE("PluginCompatibilityScore cycle detection is deterministic", "[plugin][compatibility][ffs04]") {
    PluginCompatibilityAnalysisInput input;
    input.manifests = {
        manifest({{"name", "CycleC"}, {"dependencies", nlohmann::json::array({"CycleA"})}}),
        manifest({{"name", "CycleA"}, {"dependencies", nlohmann::json::array({"CycleB"})}}),
        manifest({{"name", "CycleB"}, {"dependencies", nlohmann::json::array({"CycleC"})}}),
    };

    const auto report = AnalyzePluginCompatibility(input);

    REQUIRE(report.dependency_cycles.size() == 1);
    REQUIRE(report.dependency_cycles[0].size() == 3);
    REQUIRE(report.dependency_cycles[0][0] == "CycleA");
    REQUIRE(report.dependency_cycles[0][1] == "CycleB");
    REQUIRE(report.dependency_cycles[0][2] == "CycleC");

    const auto& cycle_a = findPlugin(report, "CycleA");
    REQUIRE(cycle_a.tier == PluginCompatibilityTier::Risky);
    REQUIRE(std::any_of(cycle_a.issues.begin(), cycle_a.issues.end(), [](const auto& issue) {
        return issue.kind == PluginCompatibilityIssueKind::LoadOrderCycle && issue.blocking;
    }));
}

TEST_CASE("PluginCompatibilityScore lists unsupported APIs and native shim hints only for mapped APIs", "[plugin][compatibility][ffs04]") {
    PluginCompatibilityAnalysisInput input;
    input.native_shim_hints = DefaultNativePluginShimHints();
    input.manifests = {
        manifest({
            {"name", "WindowTweaks"},
            {"unsupportedApis", nlohmann::json::array({"Window_Base.drawText", "SceneManager.snapUnknown"})},
        }),
    };

    const auto report = AnalyzePluginCompatibility(input);
    const auto& window = findPlugin(report, "WindowTweaks");

    REQUIRE(window.unsupported_apis.size() == 2);
    REQUIRE(window.unsupported_apis[0] == "SceneManager.snapUnknown");
    REQUIRE(window.unsupported_apis[1] == "Window_Base.drawText");
    REQUIRE(window.shim_hints.size() == 1);
    REQUIRE(window.shim_hints[0].api == "Window_Base.drawText");
    REQUIRE(window.shim_hints[0].native_feature == "engine/core/message");

    size_t hinted_issue_count = 0;
    for (const auto& issue : window.issues) {
        if (issue.shim_hint.has_value()) {
            ++hinted_issue_count;
            REQUIRE(issue.target == "Window_Base.drawText");
        }
    }
    REQUIRE(hinted_issue_count == 1);
}

TEST_CASE("PluginCompatibilityScore ingests fixture-only behavior, fallback paths, failure reports, and override conflicts", "[plugin][compatibility][ffs04]") {
    PluginCompatibilityAnalysisInput input;
    input.failure_diagnostics_jsonl =
        R"({"seq":1,"subsystem":"plugin_manager","event":"compat_failure","plugin":"MenuA","operation":"load_plugin_js_eval","message":"eval unavailable","severity":"CRASH_PREVENTED"})";
    input.manifests = {
        manifest({
            {"name", "MenuA"},
            {"fixtureOnlyBehaviors", nlohmann::json::array({"command_fixture"})},
            {"fallbackPaths", nlohmann::json::array({"menu_scene_fallback"})},
            {"overrides", nlohmann::json::array({"Scene_Menu.create"})},
        }),
        manifest({
            {"name", "MenuB"},
            {"overrides", nlohmann::json::array({"Scene_Menu.create"})},
        }),
    };

    const auto report = AnalyzePluginCompatibility(input);

    REQUIRE(report.override_conflicts.size() == 1);
    REQUIRE(report.override_conflicts[0][0] == "MenuA");
    REQUIRE(report.override_conflicts[0][1] == "MenuB");

    const auto& menu_a = findPlugin(report, "MenuA");
    REQUIRE(menu_a.score == 25);
    REQUIRE(menu_a.tier == PluginCompatibilityTier::Unsupported);
    REQUIRE(std::any_of(menu_a.issues.begin(), menu_a.issues.end(), [](const auto& issue) {
        return issue.kind == PluginCompatibilityIssueKind::FailureDiagnostic && issue.blocking;
    }));
}

TEST_CASE("PluginCompatibilityScore loads real compat fixture manifests with profile-allowed dependency drift", "[plugin][compatibility][ffs04]") {
    const auto source_root = sourceRootFromMacro();
    REQUIRE_FALSE(source_root.empty());

    std::string error;
    const auto manifests = LoadPluginCompatibilityManifestsFromDirectory(
        source_root / "tests" / "compat" / "fixtures" / "plugins",
        &error
    );
    REQUIRE(error.empty());
    REQUIRE(manifests.size() >= 10);

    PluginCompatibilityAnalysisInput input;
    input.manifests = manifests;
    input.native_shim_hints = DefaultNativePluginShimHints();

    const auto report = AnalyzePluginCompatibility(input);
    const auto& drift = findPlugin(report, "URPG_DependencyDrift_Fixture");

    REQUIRE(std::find(
                drift.missing_dependencies.begin(),
                drift.missing_dependencies.end(),
                "URPG_NonExistent_DependencyTarget"
            ) == drift.missing_dependencies.end());
    REQUIRE(std::any_of(report.dependency_edges.begin(), report.dependency_edges.end(), [](const auto& edge) {
        return edge.from_plugin == "URPG_DependencyDrift_Fixture" &&
               edge.to_plugin == "URPG_NonExistent_DependencyTarget" &&
               edge.missing &&
               edge.optional;
    }));
}

TEST_CASE("PluginCompatibilityScore exports machine-readable non-authoritative report", "[plugin][compatibility][ffs04]") {
    PluginCompatibilityAnalysisInput input;
    input.native_shim_hints = DefaultNativePluginShimHints();
    input.manifests = {
        manifest({
            {"name", "WindowTweaks"},
            {"unsupportedApis", nlohmann::json::array({"Window_Base.drawText"})},
        }),
    };

    const auto report = AnalyzePluginCompatibility(input);
    const auto exported = PluginCompatibilityReportToJson(report);

    REQUIRE(exported["release_authoritative"] == false);
    REQUIRE(exported["plugins"].size() == 1);
    REQUIRE(exported["plugins"][0]["tier"] == "partial");
    REQUIRE(exported["plugins"][0]["issues"][0]["shim_hint"]["native_feature"] == "engine/core/message");
}
