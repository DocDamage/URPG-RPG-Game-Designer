#include "engine/core/audio/audio_core.h"
#include "engine/core/global_state_hub.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_route_resolver.h"
#include "engine/core/ui/menu_scene_graph.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

using namespace urpg::ui;
using namespace urpg::audio;
using namespace urpg;

TEST_CASE("MenuSceneGraph: Command Orchestration", "[ui][menu][orchestration]") {
    auto& hub = GlobalStateHub::getInstance();
    hub.resetAll();

    // Prepare components
    auto audio = std::make_shared<AudioCore>();
    MenuRouteResolver resolver;
    MenuSceneGraph graph;
    graph.setAudio(audio);
    graph.setRouteResolver(&resolver);

    // Track route execution
    bool itemRouteTriggered = false;
    resolver.bindRoute(MenuRouteTarget::Item, [&](const MenuCommandMeta&) { itemRouteTriggered = true; });

    // Create a scene with a pane
    auto scene = std::make_shared<MenuScene>("main_menu");
    MenuPane mainPane;
    mainPane.id = "main_pane";
    mainPane.isActive = true;

    MenuCommandMeta itemCmd;
    itemCmd.id = "urpg.menu.item";
    itemCmd.label = "Items";
    itemCmd.route = MenuRouteTarget::Item;
    mainPane.commands.push_back(itemCmd);

    scene->addPane(mainPane);
    graph.registerScene(scene);
    graph.pushScene("main_menu");

    SECTION("Confirming an enabled command triggers its route") {
        graph.handleInput(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
        REQUIRE(itemRouteTriggered);
    }

    SECTION("Command visibility integration with GlobalStateHub") {
        // Condition: S001 must be true
        MenuCommandMeta secretCmd;
        secretCmd.id = "urpg.menu.secret";
        secretCmd.label = "Secret";
        secretCmd.route = MenuRouteTarget::Status;

        MenuCommandCondition cond;
        cond.switch_id = "S001_SecretVisible";
        secretCmd.visibility_rules.push_back(cond);

        // Registry check
        MenuCommandRegistry registry;
        registry.registerCommand(secretCmd);

        MenuCommandRegistry::SwitchState switches;
        MenuCommandRegistry::VariableState variables;

        // Initial state: hidden
        MenuCommandRegistry::captureGlobalState(switches, variables);
        REQUIRE_FALSE(registry.isVisible(secretCmd, switches, variables));

        // State update: visible
        hub.setSwitch("S001_SecretVisible", true);
        MenuCommandRegistry::captureGlobalState(switches, variables);
        REQUIRE(registry.isVisible(secretCmd, switches, variables));
    }

    SECTION("Scene stack navigation") {
        auto subScene = std::make_shared<MenuScene>("sub_menu");
        MenuPane subPane;
        subPane.id = "sub_pane";
        subPane.isActive = true;
        MenuCommandMeta backCmd;
        backCmd.id = "urpg.menu.back";
        backCmd.label = "Back";
        backCmd.route = MenuRouteTarget::Custom;
        subPane.commands.push_back(backCmd);
        subScene->addPane(subPane);
        graph.registerScene(subScene);

        REQUIRE(graph.getActiveScene()->getId() == "main_menu");

        graph.pushScene("sub_menu");
        REQUIRE(graph.getActiveScene()->getId() == "sub_menu");

        graph.handleInput(urpg::input::InputAction::Cancel, urpg::input::ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getId() == "main_menu");
    }
}
