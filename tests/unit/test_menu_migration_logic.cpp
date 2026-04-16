#include <catch2/catch_test_macros.hpp>
#include "engine/core/ui/menu_migration.h"
#include <nlohmann/json.hpp>

using namespace urpg::ui;

TEST_CASE("MenuMigration: Command Panel Mapping", "[ui][menu][migration]") {
    nlohmann::json rm_menu = {
        {{"symbol", "item"}, {"name", "Item"}, {"enabled", true}},
        {{"symbol", "skill"}, {"name", "Skill"}, {"enabled", true}},
        {{"symbol", "save"}, {"name", "Save"}, {"enabled", false}},
        {{"symbol", "gameEnd"}, {"name", "Game End"}, {"enabled", true}}
    };

    MenuMigration::Progress progress;
    auto native = MenuMigration::MigrateCommandPanel("main_menu", rm_menu, progress);

    REQUIRE(native["id"] == "PANEL_main_menu");
    REQUIRE(native["commands"].size() == 4);
    REQUIRE(native["commands"][0]["symbol"] == "item");
    REQUIRE(native["commands"][0]["handler_route"] == "scene.item");
    REQUIRE(native["commands"][2]["symbol"] == "save");
    REQUIRE(native["commands"][2]["handler_route"] == "scene.save");
    REQUIRE(native["commands"][2]["enabled_by"]["value"] == false);
    
    REQUIRE(progress.total_commands == 4);
    REQUIRE(progress.total_scenes == 1);
}

TEST_CASE("MenuMigration: Custom Command Routing", "[ui][menu][migration]") {
    nlohmann::json rm_custom = {
        {{"symbol", "quests"}, {"name", "Journal"}, {"enabled", true}}
    };

    MenuMigration::Progress progress;
    auto native = MenuMigration::MigrateCommandPanel("custom_menu", rm_custom, progress);

    // Custom symbols should map to a namespaced route
    REQUIRE(native["commands"][0]["handler_route"] == "scene.custom.quests");
}
