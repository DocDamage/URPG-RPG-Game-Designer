#include "editor/ui/menu_preview_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MenuPreviewPanel - Render snapshot exposes active scene and pane command state",
          "[ui][editor][menu_preview][panel]") {
    auto menu = std::make_shared<urpg::ui::MenuScene>("PreviewMenu");

    urpg::ui::MenuPane mainPane;
    mainPane.id = "main_pane";
    mainPane.displayName = "Main Pane";
    mainPane.isVisible = true;
    mainPane.isActive = true;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;

    urpg::MenuCommandMeta saveCommand;
    saveCommand.id = "urpg.menu.save";
    saveCommand.label = "Save";
    saveCommand.route = urpg::MenuRouteTarget::Save;

    mainPane.commands = {itemCommand, saveCommand};
    mainPane.selectedCommandIndex = 1;

    urpg::ui::MenuPane hiddenPane;
    hiddenPane.id = "hidden_pane";
    hiddenPane.displayName = "Hidden Pane";
    hiddenPane.isVisible = false;
    hiddenPane.commands = {itemCommand};

    menu->addPane(mainPane);
    menu->addPane(hiddenPane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("PreviewMenu");

    urpg::editor::MenuPreviewPanel panel;
    panel.bindRuntime(graph);
    panel.SetVisible(true);

    urpg::FrameContext context{0.016f, 1};
    panel.Render(context);

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().active_scene_id == "PreviewMenu");
    REQUIRE(panel.lastRenderSnapshot().visible_panes.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].pane_id == "main_pane");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].pane_active == true);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].selected_command_id == "urpg.menu.save");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_ids.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_ids[0] == "urpg.menu.item");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_ids[1] == "urpg.menu.save");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_labels[0] == "Item");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_labels[1] == "Save");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_enabled.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_enabled[0] == true);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_enabled[1] == true);
}

TEST_CASE("MenuPreviewPanel reflects runtime edits and clear behavior",
          "[ui][editor][menu_preview][panel][edit]") {
    auto menu = std::make_shared<urpg::ui::MenuScene>("EditPreviewMenu");

    urpg::ui::MenuPane mainPane;
    mainPane.id = "main_pane";
    mainPane.displayName = "Main Pane";
    mainPane.isVisible = true;
    mainPane.isActive = true;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;

    mainPane.commands = {itemCommand};
    menu->addPane(mainPane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("EditPreviewMenu");

    urpg::editor::MenuPreviewPanel panel;
    panel.bindRuntime(graph);
    panel.SetVisible(true);

    urpg::FrameContext context{0.016f, 1};
    panel.Render(context);

    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_labels[0] == "Item");

    graph.getActiveScene()->getPanesMutable()[0].commands[0].label = "Updated Item";
    panel.refresh();
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_labels[0] == "Updated Item");

    panel.clearRuntime();
    REQUIRE_FALSE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().has_data == false);
}

TEST_CASE("MenuPreviewPanel materializes authoring documents through the native runtime graph",
          "[ui][editor][menu_preview][authoring][pcq480][pcq485]") {
    const auto templates = urpg::ui::MenuStarterTemplateLibrary::originalUrpgTemplates();
    const auto* title = templates.find("title");
    REQUIRE(title != nullptr);
    urpg::editor::MenuPreviewPanel panel;
    std::vector<std::string> diagnostics;
    REQUIRE(panel.bindAuthoringDocument(title->document, "authored_title", &diagnostics));
    REQUIRE(diagnostics.empty());
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().active_scene_id == "authored_title");
    REQUIRE(panel.lastRenderSnapshot().visible_panes.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].pane_id == "title.root");
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_ids ==
            std::vector<std::string>{"title.primary"});
    panel.clearRuntime();
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
}

TEST_CASE("MenuPreviewPanel resolves typed bindings states transitions audio and reduced motion",
          "[ui][editor][menu_preview][bindings][states][pcq482][pcq483][pcq484]") {
    using namespace urpg::ui;
    MenuAuthoringDocument document;
    MenuCanvasNode root;
    root.id = "root";
    root.kind = MenuElementKind::Panel;
    root.layout = {32, 32, 640, 360};
    REQUIRE(document.addNode(root));
    MenuCanvasNode button;
    button.id = "start";
    button.parent_id = "root";
    button.kind = MenuElementKind::Button;
    button.layout = {64, 64, 240, 64};
    button.label = "Start";
    button.accessible_label = "Start";
    button.focusable = true;
    button.focus_next_id = "start";
    button.route = urpg::MenuRouteTarget::Custom;
    button.custom_route_id = "game.start";
    button.bindings = {
        {"label", MenuBindingSource::Localization, "menu.start", MenuBindingValueType::String,
         std::string("[missing start]"), {}, true},
        {"enabled", MenuBindingSource::Runtime, "can_start", MenuBindingValueType::Boolean,
         false, {}, false}};
    button.state_styles = {
        {MenuVisualState::Default, {{"fill", "normal"}}},
        {MenuVisualState::Focus, {{"fill", "focus"}}},
        {MenuVisualState::Disabled, {{"fill", "disabled"}}}};
    button.transitions = {
        {MenuVisualState::Default, MenuVisualState::Focus, 180,
         MenuTransitionInterruption::Replace, "ui.focus", false, false}};
    REQUIRE(document.addNode(button));

    MenuBindingContext context;
    context.localization["menu.start"] = "Begin Adventure";
    context.runtime["can_start"] = true;
    urpg::editor::MenuPreviewPanel panel;
    std::vector<std::string> diagnostics;
    REQUIRE(panel.bindAuthoringDocument(document, "bound_menu", context, &diagnostics));
    REQUIRE(diagnostics.empty());
    const auto& enabled = panel.lastRenderSnapshot().visible_panes[0];
    REQUIRE(enabled.command_labels == std::vector<std::string>{"Begin Adventure"});
    REQUIRE(enabled.command_enabled == std::vector<bool>{true});
    REQUIRE(enabled.command_visual_states == std::vector<MenuVisualState>{MenuVisualState::Focus});
    REQUIRE(enabled.command_state_properties[0].at("fill") == "focus");
    REQUIRE(enabled.command_transition_duration_ms == std::vector<uint32_t>{180});
    REQUIRE(enabled.command_audio_hooks == std::vector<std::string>{"ui.focus"});

    panel.setPreviewAccessibilityPolicy(true, false);
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_transition_duration_ms ==
            std::vector<uint32_t>{0});
    REQUIRE(panel.lastRenderSnapshot().visible_panes[0].command_audio_hooks ==
            std::vector<std::string>{""});

    diagnostics.clear();
    REQUIRE(panel.bindAuthoringDocument(document, "fallback_menu", &diagnostics));
    REQUIRE(diagnostics.size() == 3);
    const auto& fallback = panel.lastRenderSnapshot().visible_panes[0];
    REQUIRE(fallback.command_labels == std::vector<std::string>{"[missing start]"});
    REQUIRE(fallback.command_enabled == std::vector<bool>{false});
    REQUIRE(fallback.command_visual_states == std::vector<MenuVisualState>{MenuVisualState::Disabled});
    REQUIRE(fallback.command_state_properties[0].at("fill") == "disabled");
}

TEST_CASE("MenuPreviewPanel feeds effective authored nodes to the object-linked inclusive auditor",
          "[ui][editor][menu_preview][accessibility_audit][pcq655]") {
    using namespace urpg::ui;
    MenuAuthoringDocument document;
    MenuCanvasNode root;
    root.id = "root";
    root.layout = {0, 0, 640, 360};
    REQUIRE(document.addNode(root));
    MenuCanvasNode bad;
    bad.id = "bad_action";
    bad.parent_id = "root";
    bad.kind = MenuElementKind::Button;
    bad.layout = {16, 16, 30, 30, 0, 0};
    bad.label = "Localized label that overflows";
    bad.focusable = true;
    bad.route = urpg::MenuRouteTarget::Custom;
    bad.custom_route_id = "bad.action";
    bad.state_styles = {{MenuVisualState::Focus, {{"contrast_ratio", "2.5"}}}};
    bad.transitions = {{MenuVisualState::Default, MenuVisualState::Focus, 300,
                        MenuTransitionInterruption::Replace, {}, false, false}};
    REQUIRE(document.addNode(bad));
    urpg::editor::MenuPreviewPanel panel;
    REQUIRE(panel.bindAuthoringDocument(document, "audit_menu"));
    const auto issues = panel.auditInclusiveSnapshot(true, true);
    const std::set<std::string> codes = {"missing_label", "contrast", "hit_target",
                                          "localization_overflow", "unsafe_motion"};
    REQUIRE(issues.size() == codes.size());
    for (const auto& issue : issues) {
        REQUIRE(codes.contains(issue.code));
        REQUIRE(issue.object_id == "bad_action");
    }
}
