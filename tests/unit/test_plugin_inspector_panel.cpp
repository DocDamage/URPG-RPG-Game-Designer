#include "editor/plugin/plugin_inspector_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("PluginInspectorPanel renders compatibility inspector summary", "[editor][plugin][compatibility][ffs04]") {
    urpg::editor::PluginInspectorPanel panel;

    urpg::plugin::PluginCompatibilityAnalysisInput input;
    input.granted_permissions = {"save.read"};
    input.native_shim_hints = urpg::plugin::DefaultNativePluginShimHints();
    input.manifests = {
        urpg::plugin::ParsePluginCompatibilityManifest({
            {"name", "CoreEngine"},
            {"permissions", nlohmann::json::array({"save.read"})},
        }),
        urpg::plugin::ParsePluginCompatibilityManifest({
            {"name", "Addon"},
            {"dependencies", nlohmann::json::array({"MissingDependency"})},
            {"permissions", nlohmann::json::array({"network.fetch"})},
            {"unsupportedApis", nlohmann::json::array({"Window_Base.drawText"})},
        }),
    };

    panel.model().analyze(input);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().plugin_count == 2);
    REQUIRE(panel.lastRenderSnapshot().missing_dependency_count == 1);
    REQUIRE(panel.lastRenderSnapshot().permission_denial_count == 1);
    REQUIRE(panel.lastRenderSnapshot().unsupported_api_count == 1);
    REQUIRE(panel.lastRenderSnapshot().shim_hint_count == 1);
    REQUIRE_FALSE(panel.lastRenderSnapshot().release_authoritative);

    const auto exported = panel.model().exportSnapshotJson();
    REQUIRE(exported["snapshot"]["plugin_count"] == 2);
    REQUIRE(exported["snapshot"]["release_authoritative"] == false);
    REQUIRE(exported["plugins"].is_array());
}

TEST_CASE("PluginInspectorPanel hidden render keeps previous frame stable", "[editor][plugin][compatibility][ffs04]") {
    urpg::editor::PluginInspectorPanel panel;
    panel.setVisible(false);
    panel.render();

    REQUIRE_FALSE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
}
