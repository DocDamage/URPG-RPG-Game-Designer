#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/diagnostics/diagnostics_facade.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/input/input_core.h"
#include "engine/core/message/message_core.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace {

urpg::message::DialoguePage makeDialoguePage(std::string id,
                                             std::string body,
                                             urpg::message::MessagePresentationVariant variant,
                                             bool wait_for_advance = true,
                                             std::vector<urpg::message::ChoiceOption> choices = {},
                                             int32_t default_choice_index = 0) {
    urpg::message::DialoguePage page;
    page.id = std::move(id);
    page.body = std::move(body);
    page.variant = std::move(variant);
    page.wait_for_advance = wait_for_advance;
    page.choices = std::move(choices);
    page.default_choice_index = default_choice_index;
    return page;
}

} // namespace

TEST_CASE("DiagnosticsWorkspace - Message inspector workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][message_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    urpg::message::MessageFlowRunner runner;
    runner.begin({
        makeDialoguePage("speaker_a", "Welcome back.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3)),
        makeDialoguePage("narration_b", "", urpg::message::variantFromCompatRoute("narration", "Alicia", 3)),
    });
    urpg::message::RichTextLayoutEngine layout;

    workspace.bindMessageRuntime(runner, layout);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);
    workspace.update();

    REQUIRE(workspace.messagePanel().hasRenderedFrame());
    REQUIRE(workspace.messagePanel().lastRenderSnapshot().has_data);
    REQUIRE(workspace.messagePanel().lastRenderSnapshot().visible_row_count == 2);
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "message_text");
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_page_id"].is_null());

    REQUIRE(workspace.setMessageShowIssuesOnly(true));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "narration_b");

    REQUIRE(workspace.setMessageShowIssuesOnly(false));
    REQUIRE(workspace.setMessageRouteFilter(urpg::message::MessagePresentationMode::Speaker));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "speaker_a");

    REQUIRE(workspace.selectMessageRow(0));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_page_id"] == "speaker_a");

    REQUIRE(workspace.clearMessageRouteFilter());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_page_id"] == "speaker_a");
}

TEST_CASE("DiagnosticsWorkspace - Message inspector actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][message_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);

    urpg::message::MessageFlowRunner runner;
    runner.begin({
        makeDialoguePage("speaker_a", "Welcome back.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3)),
        makeDialoguePage("narration_b", "", urpg::message::variantFromCompatRoute("narration", "Alicia", 3)),
    });
    urpg::message::RichTextLayoutEngine layout;
    workspace.bindMessageRuntime(runner, layout);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "message_text");
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "speaker_a");

    workspace.clearMessageRuntime();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].empty());

    urpg::message::MessageFlowRunner runner2;
    runner2.begin({
        makeDialoguePage("page_c", "Hello.", urpg::message::variantFromCompatRoute("speaker", "Bob", 1)),
    });
    workspace.bindMessageRuntime(runner2, layout);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "page_c");
}


TEST_CASE("DiagnosticsWorkspace - Menu edit and export round-trip", "[editor][diagnostics][integration][menu_parity]") {
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
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Menu);
    workspace.update();

    // Edit a command label through the workspace
    REQUIRE(workspace.updateMenuCommandLabel(1, "New Label"));
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"][1]["command_label"] == "New Label");

    // Apply changes to runtime
    REQUIRE(workspace.applyMenuChangesToRuntime());

    // Export menu state JSON and verify the change persisted in runtime
    const auto menuStateJson = nlohmann::json::parse(workspace.exportMenuStateJson());
    REQUIRE(menuStateJson["scenes"].size() == 1);
    REQUIRE(menuStateJson["scenes"][0]["panes"][0]["commands"][1]["label"] == "New Label");

    // Edit route
    REQUIRE(workspace.updateMenuCommandRoute(1, urpg::MenuRouteTarget::Options, ""));
    REQUIRE(workspace.applyMenuChangesToRuntime());
    const auto routeStateJson = nlohmann::json::parse(workspace.exportMenuStateJson());
    REQUIRE(routeStateJson["scenes"][0]["panes"][0]["commands"][1]["route"] == "Options");

    // Remove command
    REQUIRE(workspace.removeMenuCommand(1));
    REQUIRE(workspace.applyMenuChangesToRuntime());
    const auto removedStateJson = nlohmann::json::parse(workspace.exportMenuStateJson());
    REQUIRE(removedStateJson["scenes"][0]["panes"][0]["commands"].size() == 1);

    // Add command back
    urpg::MenuCommandMeta newCmd;
    newCmd.id = "urpg.menu.new";
    newCmd.label = "New Command";
    newCmd.route = urpg::MenuRouteTarget::Save;
    REQUIRE(workspace.addMenuCommand(0, newCmd));
    REQUIRE(workspace.applyMenuChangesToRuntime());
    const auto addedStateJson = nlohmann::json::parse(workspace.exportMenuStateJson());
    REQUIRE(addedStateJson["scenes"][0]["panes"][0]["commands"].size() == 2);
    REQUIRE(addedStateJson["scenes"][0]["panes"][0]["commands"][1]["id"] == "urpg.menu.new");
}

TEST_CASE("DiagnosticsWorkspace - Menu save and load round-trip", "[editor][diagnostics][integration][menu_parity]") {
    urpg::ui::MenuCommandRegistry registry;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "urpg.menu.item";
    itemCommand.label = "Item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    registry.registerCommand(itemCommand);

    auto menu = std::make_shared<urpg::ui::MenuScene>("MainMenu");
    urpg::ui::MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main Menu";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands = {itemCommand};
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

    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_menu_state.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(workspace.saveMenuStateToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    // Create a fresh graph and load into it
    urpg::ui::MenuSceneGraph loadedGraph;
    urpg::editor::DiagnosticsWorkspace loadWorkspace;
    loadWorkspace.bindMenuRuntime(loadedGraph, registry, switches, variables);
    REQUIRE(loadWorkspace.loadMenuStateFromFile(temp_path));

    const auto loadedJson = nlohmann::json::parse(loadWorkspace.exportMenuStateJson());
    REQUIRE(loadedJson["scenes"].size() == 1);
    REQUIRE(loadedJson["scenes"][0]["scene_id"] == "MainMenu");
    REQUIRE(loadedJson["scenes"][0]["panes"][0]["commands"].size() == 1);
    REQUIRE(loadedJson["scenes"][0]["panes"][0]["commands"][0]["id"] == "urpg.menu.item");

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Message edit round-trip through workspace",
          "[editor][diagnostics][integration][message_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);

    urpg::message::MessageFlowRunner runner;
    runner.begin({
        makeDialoguePage("page_a", "Hello world.", urpg::message::variantFromCompatRoute("speaker", "Alice", 1)),
        makeDialoguePage("page_b", "Goodbye.", urpg::message::variantFromCompatRoute("narration", "", 0)),
    });
    urpg::message::RichTextLayoutEngine layout;
    workspace.bindMessageRuntime(runner, layout);
    workspace.update();

    REQUIRE(workspace.updateMessagePageBody(0, "Updated body."));
    REQUIRE(workspace.updateMessagePageSpeaker(0, "Bob"));
    REQUIRE(workspace.removeMessagePage(1));

    urpg::message::DialoguePage newPage;
    newPage.id = "page_c";
    newPage.body = "New page content.";
    newPage.variant = urpg::message::variantFromCompatRoute("speaker", "Carol", 2);
    newPage.wait_for_advance = true;
    REQUIRE(workspace.addMessagePage(newPage));

    REQUIRE(workspace.applyMessageChangesToRuntime(runner));

    const auto& pages = runner.pages();
    REQUIRE(pages.size() == 2);
    REQUIRE(pages[0].body == "Updated body.");
    REQUIRE(pages[0].variant.speaker == "Bob");
    REQUIRE(pages[1].id == "page_c");
    REQUIRE(pages[1].body == "New page content.");
    REQUIRE(pages[1].variant.speaker == "Carol");
}

TEST_CASE("DiagnosticsWorkspace - Message state export and import round-trip",
          "[editor][diagnostics][integration][message_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);

    urpg::message::MessageFlowRunner runner;
    runner.begin({
        makeDialoguePage("page_a", "Hello world.", urpg::message::variantFromCompatRoute("speaker", "Alice", 1)),
    });
    urpg::message::RichTextLayoutEngine layout;
    workspace.bindMessageRuntime(runner, layout);
    workspace.update();

    const auto jsonStr = workspace.exportMessageStateJson();
    const auto json = nlohmann::json::parse(jsonStr);
    REQUIRE(json.is_array());
    REQUIRE(json.size() == 1);
    REQUIRE(json[0]["id"] == "page_a");
    REQUIRE(json[0]["body"] == "Hello world.");
    REQUIRE(json[0]["speaker"] == "Alice");
    REQUIRE(json[0]["mode"] == "speaker");
    REQUIRE(json[0]["wait_for_advance"] == true);

    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_message_state.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(workspace.saveMessageStateToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    urpg::message::MessageFlowRunner loadedRunner;
    urpg::editor::DiagnosticsWorkspace loadWorkspace;
    loadWorkspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);
    loadWorkspace.bindMessageRuntime(loadedRunner, layout);
    REQUIRE(loadWorkspace.loadMessageStateFromFile(temp_path, loadedRunner));

    const auto& loadedPages = loadedRunner.pages();
    REQUIRE(loadedPages.size() == 1);
    REQUIRE(loadedPages[0].id == "page_a");
    REQUIRE(loadedPages[0].body == "Hello world.");
    REQUIRE(loadedPages[0].variant.speaker == "Alice");
    REQUIRE(loadedPages[0].variant.mode == urpg::message::MessagePresentationMode::Speaker);

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Message workspace actions keep snapshot current after edits",
          "[editor][diagnostics][integration][message_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);

    urpg::message::MessageFlowRunner runner;
    runner.begin({
        makeDialoguePage("page_a", "Hello world.", urpg::message::variantFromCompatRoute("speaker", "Alice", 1)),
    });
    urpg::message::RichTextLayoutEngine layout;
    workspace.bindMessageRuntime(runner, layout);
    workspace.update();

    REQUIRE(workspace.updateMessagePageBody(0, "Updated body."));
    REQUIRE(workspace.applyMessageChangesToRuntime(runner));
    workspace.refresh();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["page_id"] == "page_a");
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["body_preview"] == "Updated body.");
}


