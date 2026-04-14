#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_route_resolver.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/input/input_core.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace urpg;
using namespace urpg::ui;
using namespace urpg::input;

TEST_CASE("MenuSceneGraph: Input Navigation", "[ui][graph][input]") {
    MenuSceneGraph graph;
    
    auto menu = std::make_shared<MenuScene>("Test");
    MenuPane pane;
    pane.id = "p1";
    pane.isActive = true;
    pane.commands = { {"c1"}, {"c2"}, {"c3"} };
    menu->addPane(pane);
    graph.registerScene(menu);
    graph.pushScene("Test");

    SECTION("MoveDown cycles forward") {
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 0);
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 1);
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 2);
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 0);
    }

    SECTION("MoveUp cycles backward") {
        graph.handleInput(InputAction::MoveUp, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 2);
    }
}

TEST_CASE("MenuSceneGraph handles basic navigation and stack", "[ui][graph]") {
    MenuSceneGraph graph;
    
    auto mainMenu = std::make_shared<MenuScene>("MainMenu");
    mainMenu->addPane({"main_pane", "Main Menu", true, true, {}});
    graph.registerScene(mainMenu);

    auto settingsMenu = std::make_shared<MenuScene>("Settings");
    settingsMenu->addPane({"audio_pane", "Audio", true, false, {}});
    graph.registerScene(settingsMenu);

    SECTION("Empty stack initially") {
        REQUIRE(graph.getActiveScene() == nullptr);
        REQUIRE(graph.stackSize() == 0);
    }

    SECTION("Push and Pop Scenes") {
        graph.pushScene("MainMenu");
        REQUIRE(graph.getActiveScene()->getId() == "MainMenu");
        REQUIRE(graph.stackSize() == 1);

        graph.pushScene("Settings");
        REQUIRE(graph.getActiveScene()->getId() == "Settings");
        REQUIRE(graph.stackSize() == 2);

        graph.popScene();
        REQUIRE(graph.getActiveScene()->getId() == "MainMenu");
        REQUIRE(graph.stackSize() == 1);
    }
}

TEST_CASE("MenuCommandRegistry handles priority and sorting", "[ui][menu]") {
    MenuCommandRegistry registry;
    
    urpg::MenuCommandMeta cmd1;
    cmd1.id = "low_prio";
    cmd1.priority = 10;
    registry.registerCommand(cmd1);

    urpg::MenuCommandMeta cmd2;
    cmd2.id = "high_prio";
    cmd2.priority = 0;
    registry.registerCommand(cmd2);

    auto list = registry.listCommands();
    REQUIRE(list.size() == 2);
    REQUIRE(list[0].id == "high_prio");
    REQUIRE(list[1].id == "low_prio");
}

TEST_CASE("MenuCommandRegistry loads from JSON schema", "[ui][menu][schema]") {
    nlohmann::json schema = R"({
        "commands": [
            { "id": "save", "label": "Save Progress", "route": "save", "priority": 100 },
            { "id": "custom_01", "label": "Special", "route": "custom", "custom_route_id": "sp_one", "priority": 50 }
        ]
    })"_json;

    MenuCommandRegistry registry;
    REQUIRE(registry.loadFromSchema(schema));

    const auto* saveCmd = registry.getCommand("save");
    REQUIRE(saveCmd != nullptr);
    REQUIRE(saveCmd->route == urpg::MenuRouteTarget::Save);
    REQUIRE(saveCmd->priority == 100);

    const auto* customCmd = registry.getCommand("custom_01");
    REQUIRE(customCmd != nullptr);
    REQUIRE(customCmd->route == urpg::MenuRouteTarget::Custom);
    REQUIRE(customCmd->custom_route_id == "sp_one");

    auto list = registry.listCommands();
    REQUIRE(list[0].id == "custom_01");
    REQUIRE(list[1].id == "save");
}

TEST_CASE("MenuRouteResolver resolves native and custom targets", "[ui][menu][route]") {
    MenuRouteResolver resolver;
    bool nativeTriggered = false;
    bool customTriggered = false;

    resolver.bindRoute(urpg::MenuRouteTarget::Item, [&](const urpg::MenuCommandMeta&) {
        nativeTriggered = true;
    });

    resolver.bindCustomRoute("quest_ext", [&](const urpg::MenuCommandMeta&) {
        customTriggered = true;
    });

    urpg::MenuCommandMeta itemCmd;
    itemCmd.route = urpg::MenuRouteTarget::Item;
    REQUIRE(resolver.resolve(itemCmd));
    REQUIRE(nativeTriggered);

    urpg::MenuCommandMeta customCmd;
    customCmd.route = urpg::MenuRouteTarget::Custom;
    customCmd.custom_route_id = "quest_ext";
    REQUIRE(resolver.resolve(customCmd));
    REQUIRE(customTriggered);

    urpg::MenuCommandMeta deadCmd;
    deadCmd.route = urpg::MenuRouteTarget::Status;
    REQUIRE_FALSE(resolver.resolve(deadCmd));
}
