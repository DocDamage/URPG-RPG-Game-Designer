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
}
