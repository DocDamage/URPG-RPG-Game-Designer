#include <catch2/catch_test_macros.hpp>
#include "engine/core/ui/menu_serializer.h"
#include <nlohmann/json.hpp>

using namespace urpg::ui;

TEST_CASE("MenuSceneSerializer: Legacy Import", "[ui][menu][legacy]") {
    MenuSceneGraph graph;
    
    SECTION("Import standard System.json commands") {
        nlohmann::json legacy_system = {
            {"menuCommands", {true, true, false, true, false, true}} // item, skill, equip, status, formation, save
        };

        bool success = MenuSceneSerializer::ImportLegacy(legacy_system, graph);
        REQUIRE(success);

        // Verify "MainMenu" scene was registered
        // (Note: Currently MenuSceneGraph doesn't allow direct scene lookup by ID,
        // so we check behavior via pushScene)
        graph.pushScene("MainMenu");
        auto active = graph.getActiveScene();
        REQUIRE(active != nullptr);
        REQUIRE(active->getId() == "MainMenu");

        auto panes = active->getPanes();
        REQUIRE(panes.size() == 1);
        
        const auto& commands = panes[0].commands;
        // Expected: Item, Skill, Status, Save + (Options, GameEnd) = 6 commands
        REQUIRE(commands.size() == 6);
        
        REQUIRE(commands[0].label == "Item");
        REQUIRE(commands[1].label == "Skill");
        REQUIRE(commands[2].label == "Status");
        REQUIRE(commands[3].label == "Save");
        REQUIRE(commands[4].label == "Options");
        REQUIRE(commands[5].label == "Game End");
    }

    SECTION("Fail on invalid legacy data") {
        nlohmann::json bad_data = {{"not_a_menu", 123}};
        bool success = MenuSceneSerializer::ImportLegacy(bad_data, graph);
        REQUIRE_FALSE(success);
    }

    SECTION("Import rich mainMenu command metadata with fallback routes and state rules") {
        nlohmann::json legacy_menu = {
            {"mainMenu", {
                {"commands", {
                    {
                        {"id", "codex"},
                        {"label", "Codex"},
                        {"route", "codex"},
                        {"fallback_route", "options"},
                        {"priority", 25},
                        {"visibility_rules", {{
                            {"switch_id", "codex_unlocked"}
                        }}},
                        {"enable_rules", {{
                            {"variable_id", "codex_points"},
                            {"variable_threshold", 3}
                        }}}
                    },
                    {
                        {"id", "mods"},
                        {"label", "Mods"},
                        {"route", "custom"},
                        {"custom_route_id", "mods_root"},
                        {"fallback_route", "custom"},
                        {"fallback_custom_route_id", "mods_safe"},
                        {"priority", 40}
                    }
                }}
            }}
        };

        bool success = MenuSceneSerializer::ImportLegacy(legacy_menu, graph);
        REQUIRE(success);

        graph.pushScene("MainMenu");
        auto active = graph.getActiveScene();
        REQUIRE(active != nullptr);

        const auto panes = active->getPanes();
        REQUIRE(panes.size() == 1);
        REQUIRE(panes[0].commands.size() == 2);

        const auto& codex = panes[0].commands[0];
        REQUIRE(codex.id == "codex");
        REQUIRE(codex.route == urpg::MenuRouteTarget::Codex);
        REQUIRE(codex.fallback_route == urpg::MenuRouteTarget::Options);
        REQUIRE(codex.priority == 25);
        REQUIRE(codex.visibility_rules.size() == 1);
        REQUIRE(codex.visibility_rules[0].switch_id == "codex_unlocked");
        REQUIRE(codex.enable_rules.size() == 1);
        REQUIRE(codex.enable_rules[0].variable_id == "codex_points");
        REQUIRE(codex.enable_rules[0].variable_threshold == 3);

        const auto& mods = panes[0].commands[1];
        REQUIRE(mods.id == "mods");
        REQUIRE(mods.route == urpg::MenuRouteTarget::Custom);
        REQUIRE(mods.custom_route_id == "mods_root");
        REQUIRE(mods.fallback_route == urpg::MenuRouteTarget::Custom);
        REQUIRE(mods.fallback_custom_route_id == "mods_safe");
    }
}

TEST_CASE("MenuSceneSerializer: Serialize round-trips native menu scene graphs", "[ui][menu][serialize]") {
    MenuSceneGraph graph;

    auto scene = std::make_shared<MenuScene>("MainMenu");

    MenuPane mainPane;
    mainPane.id = "main";
    mainPane.displayName = "Main Pane";
    mainPane.isVisible = true;

    urpg::MenuCommandMeta itemCommand;
    itemCommand.id = "item";
    itemCommand.label = "Item";
    itemCommand.icon_id = "icon_item";
    itemCommand.route = urpg::MenuRouteTarget::Item;
    itemCommand.priority = 10;

    urpg::MenuCommandMeta codexCommand;
    codexCommand.id = "codex";
    codexCommand.label = "Codex";
    codexCommand.route = urpg::MenuRouteTarget::Custom;
    codexCommand.custom_route_id = "codex_root";
    codexCommand.fallback_route = urpg::MenuRouteTarget::Options;
    codexCommand.priority = 20;

    mainPane.commands = {itemCommand, codexCommand};
    scene->addPane(mainPane);
    graph.registerScene(scene);

    const auto serialized = MenuSceneSerializer::Serialize(graph);
    REQUIRE_FALSE(serialized.empty());
    REQUIRE(serialized["scene_id"] == "MainMenu");
    REQUIRE(serialized["panes"].is_array());
    REQUIRE(serialized["panes"].size() == 1);
    REQUIRE(serialized["panes"][0]["id"] == "main");
    REQUIRE(serialized["panes"][0]["label"] == "Main Pane");
    REQUIRE(serialized["panes"][0]["commands"].size() == 2);
    REQUIRE(serialized["panes"][0]["commands"][1]["fallback_route"] == "Options");

    MenuSceneGraph restored;
    REQUIRE(MenuSceneSerializer::Deserialize(serialized, restored));

    restored.pushScene("MainMenu");
    auto active = restored.getActiveScene();
    REQUIRE(active != nullptr);
    REQUIRE(active->getId() == "MainMenu");

    const auto& panes = active->getPanes();
    REQUIRE(panes.size() == 1);
    REQUIRE(panes[0].id == "main");
    REQUIRE(panes[0].displayName == "Main Pane");
    REQUIRE(panes[0].commands.size() == 2);
    REQUIRE(panes[0].commands[0].id == "item");
    REQUIRE(panes[0].commands[0].route == urpg::MenuRouteTarget::Item);
    REQUIRE(panes[0].commands[1].id == "codex");
    REQUIRE(panes[0].commands[1].custom_route_id == "codex_root");
    REQUIRE(panes[0].commands[1].fallback_route == urpg::MenuRouteTarget::Options);
}

TEST_CASE("MenuSceneSerializer: Serialize round-trips visibility and enable rules", "[ui][menu][serialize]") {
    MenuSceneGraph graph;

    auto scene = std::make_shared<MenuScene>("RuleMenu");

    MenuPane pane;
    pane.id = "main";
    pane.displayName = "Main Pane";

    urpg::MenuCommandMeta cmd;
    cmd.id = "secret";
    cmd.label = "Secret";
    cmd.route = urpg::MenuRouteTarget::Custom;
    cmd.custom_route_id = "secret_scene";
    cmd.visibility_rules = {
        urpg::MenuCommandCondition{.switch_id = "reveal_secret", .variable_id = "", .variable_threshold = 0, .invert = false}
    };
    cmd.enable_rules = {
        urpg::MenuCommandCondition{.variable_id = "player_level", .variable_threshold = 10, .invert = false}
    };

    pane.commands = {cmd};
    scene->addPane(pane);
    graph.registerScene(scene);

    const auto serialized = MenuSceneSerializer::Serialize(graph);
    REQUIRE(serialized["panes"][0]["commands"][0].contains("visibility_rules"));
    REQUIRE(serialized["panes"][0]["commands"][0]["visibility_rules"].size() == 1);
    REQUIRE(serialized["panes"][0]["commands"][0]["visibility_rules"][0]["switch_id"] == "reveal_secret");
    REQUIRE(serialized["panes"][0]["commands"][0].contains("enable_rules"));
    REQUIRE(serialized["panes"][0]["commands"][0]["enable_rules"].size() == 1);
    REQUIRE(serialized["panes"][0]["commands"][0]["enable_rules"][0]["variable_id"] == "player_level");
    REQUIRE(serialized["panes"][0]["commands"][0]["enable_rules"][0]["variable_threshold"] == 10);

    MenuSceneGraph restored;
    REQUIRE(MenuSceneSerializer::Deserialize(serialized, restored));

    restored.pushScene("RuleMenu");
    auto active = restored.getActiveScene();
    REQUIRE(active != nullptr);

    const auto& commands = active->getPanes()[0].commands;
    REQUIRE(commands.size() == 1);
    REQUIRE(commands[0].visibility_rules.size() == 1);
    REQUIRE(commands[0].visibility_rules[0].switch_id == "reveal_secret");
    REQUIRE(commands[0].enable_rules.size() == 1);
    REQUIRE(commands[0].enable_rules[0].variable_id == "player_level");
    REQUIRE(commands[0].enable_rules[0].variable_threshold == 10);
}


TEST_CASE("MenuSceneSerializer: SerializeGraph round-trips multi-scene graphs", "[ui][menu][serialize][parity]") {
    MenuSceneGraph graph;

    auto mainScene = std::make_shared<MenuScene>("MainMenu");
    MenuPane mainPane;
    mainPane.id = "main";
    mainPane.displayName = "Main";
    urpg::MenuCommandMeta itemCmd;
    itemCmd.id = "item";
    itemCmd.label = "Items";
    itemCmd.route = urpg::MenuRouteTarget::Item;
    mainPane.commands.push_back(itemCmd);
    mainScene->addPane(mainPane);
    graph.registerScene(mainScene);

    auto settingsScene = std::make_shared<MenuScene>("Settings");
    MenuPane settingsPane;
    settingsPane.id = "settings";
    settingsPane.displayName = "Settings";
    urpg::MenuCommandMeta audioCmd;
    audioCmd.id = "audio";
    audioCmd.label = "Audio";
    audioCmd.route = urpg::MenuRouteTarget::Options;
    settingsPane.commands.push_back(audioCmd);
    settingsScene->addPane(settingsPane);
    graph.registerScene(settingsScene);

    const auto serialized = MenuSceneSerializer::SerializeGraph(graph);
    REQUIRE(serialized.contains("scenes"));
    REQUIRE(serialized["scenes"].is_array());
    REQUIRE(serialized["scenes"].size() == 2);

    // Verify both scenes are present
    bool foundMain = false;
    bool foundSettings = false;
    for (const auto& scene_json : serialized["scenes"]) {
        if (scene_json["scene_id"] == "MainMenu") {
            foundMain = true;
            REQUIRE(scene_json["panes"].size() == 1);
            REQUIRE(scene_json["panes"][0]["commands"][0]["id"] == "item");
        }
        if (scene_json["scene_id"] == "Settings") {
            foundSettings = true;
            REQUIRE(scene_json["panes"].size() == 1);
            REQUIRE(scene_json["panes"][0]["commands"][0]["id"] == "audio");
        }
    }
    REQUIRE(foundMain);
    REQUIRE(foundSettings);

    // Verify round-trip via DeserializeGraph
    MenuSceneGraph restored;
    REQUIRE(MenuSceneSerializer::DeserializeGraph(serialized, restored));
    REQUIRE(restored.getRegisteredScenes().size() == 2);

    restored.pushScene("MainMenu");
    auto active = restored.getActiveScene();
    REQUIRE(active != nullptr);
    REQUIRE(active->getId() == "MainMenu");
    REQUIRE(active->getPanes()[0].commands[0].id == "item");

    restored.pushScene("Settings");
    active = restored.getActiveScene();
    REQUIRE(active != nullptr);
    REQUIRE(active->getId() == "Settings");
    REQUIRE(active->getPanes()[0].commands[0].id == "audio");
}
