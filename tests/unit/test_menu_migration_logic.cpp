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

TEST_CASE("MenuMigration: Fallback route and rich rule mapping", "[ui][menu][migration]") {
    nlohmann::json rm_rich = {
        {
            {"symbol", "item"},
            {"name", "Item"},
            {"enabled", true},
            {"fallback_symbol", "skill"},
            {"visibility_rules", nlohmann::json::array({
                {{"switch_id", "item_unlocked"}, {"invert", false}}
            })},
            {"enable_rules", nlohmann::json::array({
                {{"variable_id", "player_level"}, {"variable_threshold", 5}}
            })}
        }
    };

    MenuMigration::Progress progress;
    auto native = MenuMigration::MigrateCommandPanel("rich_menu", rm_rich, progress);

    REQUIRE(native["commands"].size() == 1);
    REQUIRE(native["commands"][0]["handler_route"] == "scene.item");
    REQUIRE(native["commands"][0]["fallback_route"] == "scene.skill");
    REQUIRE(native["commands"][0]["visibility_rules"].is_array());
    REQUIRE(native["commands"][0]["visibility_rules"].size() == 1);
    REQUIRE(native["commands"][0]["visibility_rules"][0]["switch_id"] == "item_unlocked");
    REQUIRE(native["commands"][0]["enable_rules"].is_array());
    REQUIRE(native["commands"][0]["enable_rules"].size() == 1);
    REQUIRE(native["commands"][0]["enable_rules"][0]["variable_id"] == "player_level");
    REQUIRE(native["commands"][0]["enable_rules"][0]["variable_threshold"] == 5);
}

TEST_CASE("MenuMigration: Unsupported construct emits safe fallback diagnostics", "[ui][menu][migration]") {
    nlohmann::json rm_unsupported = {
        {
            {"symbol", "unknown_plugin_command"},
            {"name", "Plugin Command"},
            {"enabled", true},
            {"plugin_param", "complex_object"}
        }
    };

    MenuMigration::Progress progress;
    auto native = MenuMigration::MigrateCommandPanel("plugin_menu", rm_unsupported, progress);

    REQUIRE(native["commands"].size() == 1);
    REQUIRE(native["commands"][0]["handler_route"] == "scene.custom.unknown_plugin_command");
    REQUIRE(progress.warnings.empty());
    REQUIRE(progress.errors.empty());
}
