#include <catch2/catch_test_macros.hpp>
#include "editor/accessibility/accessibility_panel.h"
#include "editor/accessibility/accessibility_menu_adapter.h"
#include "engine/core/accessibility/accessibility_auditor.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/ui_types.h"

using namespace urpg::editor;
using namespace urpg::accessibility;
using namespace urpg::ui;
using urpg::MenuRouteTarget;

TEST_CASE("AccessibilityPanel: Empty snapshot when no auditor bound", "[accessibility][editor][panel]") {
    AccessibilityPanel panel;
    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot.is_object());
    REQUIRE(snapshot.empty());
}

TEST_CASE("AccessibilityPanel: Snapshot reflects issues after audit", "[accessibility][editor][panel]") {
    AccessibilityAuditor auditor;
    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);

    auditor.ingestElements({
        UiElementSnapshot{"btn_ok", "", true, 1, 4.5f},
        UiElementSnapshot{"text_1", "Low Contrast", false, 0, 2.5f}
    });

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot["issueCount"] == 2);
    REQUIRE(snapshot["issues"].is_array());
    REQUIRE(snapshot["issues"].size() == 2);
}

TEST_CASE("AccessibilityPanel: Counts are accurate", "[accessibility][editor][panel]") {
    AccessibilityAuditor auditor;
    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);

    auditor.ingestElements({
        UiElementSnapshot{"btn_a", "", true, 1, 4.5f},
        UiElementSnapshot{"btn_b", "", true, 1, 4.5f},
        UiElementSnapshot{"text_1", "Low Contrast", false, 0, 2.5f}
    });

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot["issueCount"] == 5);
    REQUIRE(snapshot["errorCount"] == 3);
    REQUIRE(snapshot["warningCount"] == 2);
}

TEST_CASE("AccessibilityMenuAdapter ingests live MenuInspectorModel and produces audit issues", "[accessibility][editor][panel]") {
    // 1. Set up a command registry with commands (one missing label, two with same priority)
    MenuCommandRegistry registry;
    registry.registerCommand({"cmd_start", "Start Game", "", MenuRouteTarget::None, "", MenuRouteTarget::None, "", {}, {}, 1});
    registry.registerCommand({"cmd_load", "", "", MenuRouteTarget::None, "", MenuRouteTarget::None, "", {}, {}, 2}); // missing label
    registry.registerCommand({"cmd_options", "Options", "", MenuRouteTarget::None, "", MenuRouteTarget::None, "", {}, {}, 1}); // duplicate priority

    // 2. Set up a menu scene with a pane containing those commands
    auto scene = std::make_shared<MenuScene>("main_menu");
    MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands.push_back(*registry.getCommand("cmd_start"));
    pane.commands.push_back(*registry.getCommand("cmd_load"));
    pane.commands.push_back(*registry.getCommand("cmd_options"));
    scene->addPane(pane);

    // 3. Set up scene graph and push the scene
    MenuSceneGraph graph;
    graph.registerScene(scene);
    graph.pushScene("main_menu");

    // 4. Load into MenuInspectorModel
    MenuInspectorModel model;
    MenuCommandRegistry::SwitchState switches;
    MenuCommandRegistry::VariableState variables;
    model.LoadFromRuntime(graph, registry, switches, variables);

    // 5. Ingest via adapter
    auto elements = AccessibilityMenuAdapter::ingest(model);
    REQUIRE(elements.size() == 3);

    // 6. Audit
    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    // MenuInspectorModel always synthesizes fallback labels, so MissingLabel
    // cannot be triggered from normal menu data. We verify FocusOrder instead.
    bool foundDuplicateFocusOrder = false;
    for (const auto& issue : issues) {
        if (issue.category == IssueCategory::FocusOrder) {
            foundDuplicateFocusOrder = true;
        }
    }

    REQUIRE(foundDuplicateFocusOrder);
}
