#include "tests/unit/diagnostics_workspace_test_helpers.h"

TEST_CASE("DiagnosticsWorkspace - Menu runtime binding populates and clears menu diagnostics",
          "[editor][diagnostics][integration][menu]") {
    urpg::ui::MenuCommandRegistry registry;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    registry.registerCommand(itemCommand);

    urpg::MenuCommandMeta deadEndCommand;
    deadEndCommand.id = "urpg.menu.dead_end";
    deadEndCommand.label = "Dead End";
    deadEndCommand.route = urpg::MenuRouteTarget::Custom;

    auto menu = std::make_shared<urpg::ui::MenuScene>("MainMenu");

    urpg::ui::MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main Menu";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {itemCommand, deadEndCommand};

    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuRouteResolver resolver;
    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    graph.setRouteResolver(&resolver);

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;
    graph.setCommandStateFromRegistry(registry, switches, variables);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMenuRuntime(graph, registry, switches, variables);

    const auto menuSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu);
    REQUIRE(menuSummary.item_count == 2);
    REQUIRE(menuSummary.issue_count == 2);
    REQUIRE(menuSummary.has_data);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    REQUIRE(workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu).active);
    REQUIRE(workspace.menuPanel().IsVisible());
    REQUIRE(workspace.menuPreviewPanel().GetTitle() == "Menu Preview");
    REQUIRE(workspace.menuPreviewPanel().IsVisible());

    const auto menuSnapshot = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(menuSnapshot["active_tab"] == "menu");
    REQUIRE(menuSnapshot["active_tab_detail"]["tab"] == "menu");
    REQUIRE(menuSnapshot["active_tab_detail"]["summary"]["item_count"] == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["menu_summary"]["active_scene_id"] == "MainMenu");
    REQUIRE(menuSnapshot["active_tab_detail"]["menu_summary"]["total_commands"] == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["menu_summary"]["issue_count"] == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["selected_command_id"] == nullptr);
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"].is_array());
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"][0]["command_id"] == "urpg.menu.item");
    REQUIRE(menuSnapshot["active_tab_detail"]["visible_rows"][1]["command_id"] == "urpg.menu.dead_end");
    REQUIRE(menuSnapshot["active_tab_detail"]["issues"].is_array());
    REQUIRE(menuSnapshot["active_tab_detail"]["issues"].size() == 2);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["title"] == "Menu Preview");
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible"] == true);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["has_data"] == true);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["active_scene_id"] == "MainMenu");
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"].is_array());
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"].size() == 1);
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"][0]["pane_id"] == "main_pane");
    REQUIRE(menuSnapshot["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] ==
            "urpg.menu.item");

    workspace.clearMenuRuntime();

    const auto clearedSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Menu);
    REQUIRE(clearedSummary.item_count == 0);
    REQUIRE(clearedSummary.issue_count == 0);
    REQUIRE_FALSE(clearedSummary.has_data);

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Compat);
    workspace.update();
    REQUIRE_FALSE(workspace.menuPanel().IsVisible());
    REQUIRE_FALSE(workspace.menuPreviewPanel().IsVisible());
}

TEST_CASE("DiagnosticsWorkspace - Menu workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][menu_actions]") {
    urpg::ui::MenuCommandRegistry registry;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    registry.registerCommand(itemCommand);

    urpg::MenuCommandMeta hiddenCommand;
    hiddenCommand.id = "urpg.menu.hidden";
    hiddenCommand.label = "Hidden";
    hiddenCommand.route = urpg::MenuRouteTarget::Options;
    hiddenCommand.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "secret_level",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    registry.registerCommand(hiddenCommand);

    urpg::MenuCommandMeta deadEndCommand;
    deadEndCommand.id = "urpg.menu.dead_end";
    deadEndCommand.label = "Dead End";
    deadEndCommand.route = urpg::MenuRouteTarget::Custom;

    auto menu = std::make_shared<urpg::ui::MenuScene>("MainMenu");

    urpg::ui::MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main Menu";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {itemCommand, hiddenCommand, deadEndCommand};

    menu->addPane(pane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuRouteResolver resolver;
    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    graph.setRouteResolver(&resolver);

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMenuRuntime(graph, registry, switches, variables);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_command_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["command_id_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["show_issues_only"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);

    REQUIRE(workspace.setMenuCommandIdFilter("hidden"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["command_id_filter"] == "hidden");
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["command_id"] == "urpg.menu.hidden");

    REQUIRE(workspace.selectMenuRow(0));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.hidden");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_id"] == "urpg.menu.hidden");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_visible"] == false);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["issue_count"].get<size_t>() >= 1);
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] == "urpg.menu.hidden");

    REQUIRE(workspace.clearMenuCommandIdFilter());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["command_id_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.hidden");

    REQUIRE(workspace.setMenuShowIssuesOnly(true));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_issues_only"] == true);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);

    REQUIRE(workspace.setMenuShowIssuesOnly(false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_issues_only"] == false);
}

TEST_CASE("DiagnosticsWorkspace - Save workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][save_actions]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_diagnostics_workspace_save_actions";
    std::filesystem::create_directories(base);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);
    coordinator.setAutosavePolicy({true, 0});
    coordinator.setRetentionPolicy({1, 2, 20, true});
    coordinator.metadataRegistry().registerField({"difficulty", "Difficulty", false, "Normal"});

    urpg::SaveCatalogEntry autosave;
    autosave.meta.slot_id = 0;
    autosave.meta.flags.autosave = true;
    autosave.meta.category = urpg::SaveSlotCategory::Autosave;
    autosave.meta.retention_class = urpg::SaveRetentionClass::Autosave;
    autosave.meta.map_display_name = "Autosave Camp";
    autosave.last_operation = "save";
    REQUIRE(catalog.upsert(autosave));

    urpg::SaveCatalogEntry problem;
    problem.meta.slot_id = 6;
    problem.meta.category = urpg::SaveSlotCategory::Manual;
    problem.meta.retention_class = urpg::SaveRetentionClass::Manual;
    problem.meta.flags.corrupted = true;
    problem.meta.map_display_name = "Broken Tower";
    problem.last_operation = "load";
    problem.last_recovery_tier = urpg::SaveRecoveryTier::Level3SafeSkeleton;
    problem.diagnostic = "safe_mode_triggered";
    REQUIRE(catalog.upsert(problem));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindSaveRuntime(catalog, coordinator);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Save);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "save");
    REQUIRE(exported["active_tab_detail"]["selected_slot_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["show_problem_slots_only"] == false);
    REQUIRE(exported["active_tab_detail"]["include_autosave"] == true);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_slot_id"] == 0);
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["can_apply"] == true);
    REQUIRE(exported["active_tab_detail"]["policy_issues"].size() == 0);
    REQUIRE(exported["active_tab_detail"]["metadata_fields"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);

    REQUIRE(workspace.setSaveShowProblemSlotsOnly(true));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_problem_slots_only"] == true);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["slot_id"] == 6);

    REQUIRE(workspace.selectSaveRow(0));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_slot_id"] == 6);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["slot_id"] == 6);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["boot_safe_mode"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["diagnostic"] == "safe_mode_triggered");
    REQUIRE(exported["active_tab_detail"]["recovery_diagnostics"]["total_recovery_slots"] == 1);
    REQUIRE(exported["active_tab_detail"]["recovery_diagnostics"]["safe_mode_recovery_slots"] == 1);

    REQUIRE(workspace.setSaveIncludeAutosave(false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["include_autosave"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["selected_slot_id"] == 6);

    REQUIRE(workspace.setSaveShowProblemSlotsOnly(false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["show_problem_slots_only"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["slot_id"] == 6);

    REQUIRE(workspace.setSavePolicyAutosaveEnabled(false));
    REQUIRE(workspace.setSavePolicyAutosaveSlotId(3));
    REQUIRE(workspace.setSavePolicyRetentionLimits(2, 4, 18, false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["autosave_slot_id"] == 3);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["max_autosave_slots"] == 2);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["max_quicksave_slots"] == 4);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["max_manual_slots"] == 18);
    REQUIRE(exported["active_tab_detail"]["policy_draft"]["prune_excess_on_save"] == false);
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["can_apply"] == true);
    REQUIRE(workspace.applySavePolicyChanges());
    REQUIRE(coordinator.autosavePolicy().enabled == false);
    REQUIRE(coordinator.autosavePolicy().slot_id == 3);
    REQUIRE(coordinator.retentionPolicy().max_autosave_slots == 2);
    REQUIRE(coordinator.retentionPolicy().max_quicksave_slots == 4);
    REQUIRE(coordinator.retentionPolicy().max_manual_slots == 18);
    REQUIRE(coordinator.retentionPolicy().prune_excess_on_save == false);

    REQUIRE(workspace.setSavePolicyAutosaveEnabled(true));
    REQUIRE(workspace.setSavePolicyAutosaveSlotId(-1));
    REQUIRE(workspace.setSavePolicyRetentionLimits(0, 4, 18, false));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["can_apply"] == false);
    REQUIRE(exported["active_tab_detail"]["policy_validation"]["error_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["policy_issues"].size() == 2);
    REQUIRE_FALSE(workspace.applySavePolicyChanges());
    REQUIRE(coordinator.autosavePolicy().enabled == false);
    REQUIRE(coordinator.autosavePolicy().slot_id == 3);

    std::filesystem::remove_all(base);
}

TEST_CASE("DiagnosticsWorkspace - Menu preview actions drive runtime selection and blocked-command export",
          "[editor][diagnostics][integration][menu_preview_actions]") {
    urpg::ui::MenuCommandRegistry registry;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    registry.registerCommand(itemCommand);

    urpg::MenuCommandMeta deadEndCommand;
    deadEndCommand.id = "urpg.menu.dead_end";
    deadEndCommand.label = "Dead End";
    deadEndCommand.route = urpg::MenuRouteTarget::Custom;

    auto menu = std::make_shared<urpg::ui::MenuScene>("MainMenu");

    urpg::ui::MenuPane mainPane;
    mainPane.id = "main_pane";
    mainPane.displayName = "Main Pane";
    mainPane.isVisible = true;
    mainPane.isActive = true;
    mainPane.commands = {itemCommand, deadEndCommand};

    urpg::ui::MenuPane sidePane;
    sidePane.id = "side_pane";
    sidePane.displayName = "Side Pane";
    sidePane.isVisible = true;
    sidePane.isActive = false;
    sidePane.commands = {itemCommand};

    menu->addPane(mainPane);
    menu->addPane(sidePane);

    urpg::ui::MenuSceneGraph graph;
    graph.registerScene(menu);
    graph.pushScene("MainMenu");

    urpg::ui::MenuRouteResolver resolver;
    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    graph.setRouteResolver(&resolver);

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMenuRuntime(graph, registry, switches, variables);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] == "urpg.menu.item");
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["pane_active"] == true);
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][1]["pane_active"] == false);
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_command_id"] == "");
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_reason"] == "");

    REQUIRE(workspace.dispatchMenuPreviewAction(urpg::input::InputAction::MoveDown));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["selected_command_id"] ==
            "urpg.menu.dead_end");
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.dead_end");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_id"] == "urpg.menu.dead_end");

    REQUIRE(workspace.dispatchMenuPreviewAction(urpg::input::InputAction::Confirm));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_command_id"] == "urpg.menu.dead_end");
    REQUIRE(exported["active_tab_detail"]["preview"]["last_blocked_reason"] == "No route resolved for command.");

    REQUIRE(workspace.dispatchMenuPreviewAction(urpg::input::InputAction::MoveRight));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][0]["pane_active"] == false);
    REQUIRE(exported["active_tab_detail"]["preview"]["visible_panes"][1]["pane_active"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_command_id"] == "urpg.menu.item");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["pane_id"] == "side_pane");
    REQUIRE(exported["active_tab_detail"]["selected_row"]["command_id"] == "urpg.menu.item");
}

