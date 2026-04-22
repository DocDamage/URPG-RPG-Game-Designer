#include "editor/ui/menu_inspector_panel.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::MenuCommandMeta MakeCommand(const std::string& id,
                                  const std::string& label,
                                  urpg::MenuRouteTarget route,
                                  int32_t priority) {
    urpg::MenuCommandMeta command;
    command.id = id;
    command.label = label;
    command.route = route;
    command.priority = priority;
    return command;
}

} // namespace

TEST_CASE("MenuInspectorPanel - Render snapshot exposes actionable menu workflow state",
          "[ui][editor][menu_inspector][panel]") {
    urpg::ui::MenuCommandRegistry registry;

    auto visibleCommand = MakeCommand("urpg.menu.item", "Item", urpg::MenuRouteTarget::Item, 10);
    registry.registerCommand(visibleCommand);

    auto blockedCommand = MakeCommand("urpg.menu.hidden", "Hidden", urpg::MenuRouteTarget::Options, 20);
    blockedCommand.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "secret_level",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    registry.registerCommand(blockedCommand);

    auto menu = std::make_shared<urpg::ui::MenuScene>("InspectorMenu");

    urpg::ui::MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main Pane";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {visibleCommand, blockedCommand};
    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("InspectorMenu");

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    auto model = std::make_shared<urpg::editor::MenuInspectorModel>();
    model->LoadFromRuntime(graph, registry, switches, variables);
    model->SetCommandIdFilter(std::string("hidden"));
    REQUIRE(model->SelectRow(0));
    model->SetShowIssuesOnly(true);

    urpg::editor::MenuInspectorPanel panel(model);
    panel.SetVisible(true);

    urpg::FrameContext context{0.016f, 1};
    panel.Render(context);

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().show_issues_only == true);
    REQUIRE(panel.lastRenderSnapshot().summary.active_scene_id == "InspectorMenu");
    REQUIRE(panel.lastRenderSnapshot().visible_rows.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().visible_rows[0].command_id == "urpg.menu.hidden");
    REQUIRE(panel.lastRenderSnapshot().selected_command_id == std::optional<std::string>{"urpg.menu.hidden"});
    REQUIRE(panel.lastRenderSnapshot().selected_row.has_value());
    REQUIRE(panel.lastRenderSnapshot().selected_row->summary.find("hidden") != std::string::npos);
    REQUIRE(panel.lastRenderSnapshot().issues.size() >= 1);
}
