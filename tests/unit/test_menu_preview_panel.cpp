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
