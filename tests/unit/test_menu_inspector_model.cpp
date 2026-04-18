#include "editor/ui/menu_inspector_model.h"

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

TEST_CASE("Menu inspector model builds rows and summary from active scene",
          "[ui][editor][menu_inspector][model]") {
    urpg::ui::MenuCommandRegistry registry;

    auto registeredItem = MakeCommand("urpg.menu.item", "Item", urpg::MenuRouteTarget::Item, 10);
    registry.registerCommand(registeredItem);

    auto registeredSave = MakeCommand("urpg.menu.save", "Save", urpg::MenuRouteTarget::Save, 20);
    registeredSave.enable_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "save_allowed",
            .variable_id = "",
            .variable_threshold = 0,
            .invert = false,
        },
    };
    registry.registerCommand(registeredSave);

    auto registeredHidden = MakeCommand("urpg.menu.hidden", "Hidden", urpg::MenuRouteTarget::Options, 30);
    registeredHidden.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "secret_level",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    registry.registerCommand(registeredHidden);

    auto menu = std::make_shared<urpg::ui::MenuScene>("MainMenu");

    urpg::ui::MenuPane mainPane;
    mainPane.id = "main_pane";
    mainPane.displayName = "Main Menu";
    mainPane.isVisible = true;
    mainPane.isActive = true;

    urpg::MenuCommandMeta itemCommand = registeredItem;

    urpg::MenuCommandMeta saveCommand = registeredSave;
    saveCommand.label = "Save Game";
    saveCommand.icon_id = "icon_save";
    saveCommand.enable_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "save_allowed",
            .variable_id = "",
            .variable_threshold = 0,
            .invert = false,
        },
    };

    urpg::MenuCommandMeta hiddenCommand = registeredHidden;
    hiddenCommand.label = "Hidden Option";
    hiddenCommand.icon_id = "icon_hidden";

    mainPane.commands = {itemCommand, saveCommand, hiddenCommand};

    urpg::ui::MenuPane sidePane;
    sidePane.id = "side_pane";
    sidePane.displayName = "Side";
    sidePane.isVisible = true;

    urpg::MenuCommandMeta duplicateItem = itemCommand;
    duplicateItem.label = "Item Duplicate";

    urpg::MenuCommandMeta deadEnd;
    deadEnd.id = "urpg.menu.dead_end";
    deadEnd.label = "Dead End";
    deadEnd.route = urpg::MenuRouteTarget::Custom;
    deadEnd.custom_route_id = "";
    deadEnd.fallback_route = urpg::MenuRouteTarget::Options;

    sidePane.commands = {duplicateItem, deadEnd};

    menu->addPane(mainPane);
    menu->addPane(sidePane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuCommandRegistry::SwitchState switches{
        {"save_allowed", false},
    };
    urpg::ui::MenuCommandRegistry::VariableState variables;
    graph.setCommandStateFromRegistry(registry, switches, variables);

    urpg::editor::MenuInspectorModel model;
    model.LoadFromRuntime(graph, registry, switches, variables);

    const auto& summary = model.Summary();
    REQUIRE(summary.stack_depth == 1);
    REQUIRE(summary.active_scene_id == "MainMenu");
    REQUIRE(summary.total_panes == 2);
    REQUIRE(summary.visible_panes == 2);
    REQUIRE(summary.active_panes == 1);
    REQUIRE(summary.navigable_panes == 2);
    REQUIRE(summary.total_commands == 5);
    REQUIRE(summary.visible_commands == 4);
    REQUIRE(summary.enabled_commands == 3);
    REQUIRE(summary.blocked_commands == 1);
    REQUIRE(summary.duplicate_command_ids == 1);
    REQUIRE(summary.missing_registry_entries == 1);
    REQUIRE(summary.route_binding_issues == 1);
    REQUIRE(summary.rule_validation_issues == 1);
    REQUIRE(summary.issue_count == 5);

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 4);

    REQUIRE(rows[0].pane_label == "Main Menu");
    REQUIRE(rows[0].command_id == "urpg.menu.item");
    REQUIRE(rows[0].command_registered);
    REQUIRE(rows[0].command_visible);
    REQUIRE(rows[0].command_enabled);
    REQUIRE(rows[0].row_navigable);
    REQUIRE(rows[0].issue_count == 1);

    REQUIRE(rows[1].command_id == "urpg.menu.save");
    REQUIRE_FALSE(rows[1].command_enabled);
    REQUIRE_FALSE(rows[1].row_navigable);
    REQUIRE(rows[1].summary.find("blocked") != std::string::npos);

    REQUIRE(rows[2].command_id == "urpg.menu.item");
    REQUIRE(rows[2].issue_count == 1);

    REQUIRE(rows[3].command_id == "urpg.menu.dead_end");
    REQUIRE_FALSE(rows[3].command_registered);
    REQUIRE(rows[3].route_label == "custom:<missing>");
    REQUIRE(rows[3].fallback_route_label == "options");
    REQUIRE(rows[3].issue_count == 2);
    REQUIRE(rows[3].summary.find("unregistered") != std::string::npos);
}

TEST_CASE("Menu inspector model filters issue rows and command id search",
          "[ui][editor][menu_inspector][model]") {
    urpg::ui::MenuCommandRegistry registry;

    auto visibleCommand = MakeCommand("urpg.menu.item", "Item", urpg::MenuRouteTarget::Item, 10);
    registry.registerCommand(visibleCommand);

    auto hiddenCommand = MakeCommand("urpg.menu.hidden", "Hidden", urpg::MenuRouteTarget::Options, 20);
    hiddenCommand.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "secret_level",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    registry.registerCommand(hiddenCommand);

    auto menu = std::make_shared<urpg::ui::MenuScene>("FilterMenu");

    urpg::ui::MenuPane pane;
    pane.id = "filter_pane";
    pane.displayName = "Filter Pane";
    pane.isVisible = true;
    pane.isActive = true;

    urpg::MenuCommandMeta itemCommand = visibleCommand;
    urpg::MenuCommandMeta hiddenIssueCommand = hiddenCommand;
    hiddenIssueCommand.label = "Hidden Option";

    pane.commands = {itemCommand, hiddenIssueCommand};
    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("FilterMenu");

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::MenuInspectorModel model;
    model.LoadFromRuntime(graph, registry, switches, variables);

    model.SetCommandIdFilter(std::string("hidden"));
    auto rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].command_id == "urpg.menu.hidden");
    REQUIRE_FALSE(rows[0].command_visible);
    REQUIRE(rows[0].issue_count >= 1);

    model.SetCommandIdFilter(std::nullopt);
    model.SetShowIssuesOnly(true);
    rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].command_id == "urpg.menu.hidden");
    REQUIRE(rows[0].summary.find("hidden") != std::string::npos);

    REQUIRE(model.SelectRow(0));
    const auto selected = model.SelectedCommandId();
    REQUIRE(selected.has_value());
    REQUIRE(*selected == "urpg.menu.hidden");

    REQUIRE_FALSE(model.SelectRow(9));
    REQUIRE_FALSE(model.SelectedCommandId().has_value());
}

TEST_CASE("Menu inspector model preserves selected command across filter rebuilds when still visible",
          "[ui][editor][menu_inspector][model]") {
    urpg::ui::MenuCommandRegistry registry;

    auto visibleCommand = MakeCommand("urpg.menu.item", "Item", urpg::MenuRouteTarget::Item, 10);
    registry.registerCommand(visibleCommand);

    auto issueCommand = MakeCommand("urpg.menu.dead_end", "Dead End", urpg::MenuRouteTarget::Custom, 20);

    auto menu = std::make_shared<urpg::ui::MenuScene>("FilterMenu");

    urpg::ui::MenuPane pane;
    pane.id = "filter_pane";
    pane.displayName = "Filter Pane";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {visibleCommand, issueCommand};
    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("FilterMenu");

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::MenuInspectorModel model;
    model.LoadFromRuntime(graph, registry, switches, variables);

    REQUIRE(model.SelectRow(1));
    REQUIRE(model.SelectedCommandId() == std::optional<std::string>{"urpg.menu.dead_end"});

    model.SetShowIssuesOnly(true);
    REQUIRE(model.SelectedCommandId() == std::optional<std::string>{"urpg.menu.dead_end"});

    model.SetCommandIdFilter(std::string("dead_end"));
    REQUIRE(model.SelectedCommandId() == std::optional<std::string>{"urpg.menu.dead_end"});

    model.SetCommandIdFilter(std::nullopt);
    REQUIRE(model.SelectedCommandId() == std::optional<std::string>{"urpg.menu.dead_end"});
}

TEST_CASE("Menu inspector model preserves selected hidden command identity across filter clear",
          "[ui][editor][menu_inspector][model]") {
    urpg::ui::MenuCommandRegistry registry;

    auto visibleCommand = MakeCommand("urpg.menu.item", "Item", urpg::MenuRouteTarget::Item, 10);
    registry.registerCommand(visibleCommand);

    auto hiddenCommand = MakeCommand("urpg.menu.hidden", "Hidden", urpg::MenuRouteTarget::Options, 20);
    hiddenCommand.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "secret_level",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    registry.registerCommand(hiddenCommand);

    auto menu = std::make_shared<urpg::ui::MenuScene>("HiddenSelectionMenu");

    urpg::ui::MenuPane pane;
    pane.id = "hidden_selection_pane";
    pane.displayName = "Hidden Selection Pane";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {visibleCommand, hiddenCommand};
    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("HiddenSelectionMenu");

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::MenuInspectorModel model;
    model.LoadFromRuntime(graph, registry, switches, variables);

    model.SetCommandIdFilter(std::string("hidden"));
    REQUIRE(model.SelectRow(0));
    REQUIRE(model.SelectedCommandId() == std::optional<std::string>{"urpg.menu.hidden"});

    model.SetCommandIdFilter(std::nullopt);
    REQUIRE(model.SelectedCommandId() == std::optional<std::string>{"urpg.menu.hidden"});
    REQUIRE(model.SelectedRow().has_value());
    REQUIRE(model.SelectedRow()->command_id == "urpg.menu.hidden");
    REQUIRE_FALSE(model.SelectedRow()->command_visible);
}
