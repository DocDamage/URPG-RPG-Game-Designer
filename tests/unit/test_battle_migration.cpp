#include <catch2/catch_test_macros.hpp>
#include "engine/core/battle/battle_migration.h"
#include <nlohmann/json.hpp>

using namespace urpg::battle;

TEST_CASE("BattleMigration: Troop Mapping", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 1},
        {"name", "Slime x2"},
        {"members", {
            {{"enemyId", 1}, {"x", 100}, {"y", 200}, {"hidden", false}},
            {{"enemyId", 1}, {"x", 200}, {"y", 200}, {"hidden", false}}
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["id"] == "TRP_1");
    REQUIRE(native["name"] == "Slime x2");
    REQUIRE(native["members"].size() == 2);
    REQUIRE(native["members"][0]["enemy_id"] == "ENM_1");
    REQUIRE(native["members"][0]["x"] == 100);
}

TEST_CASE("BattleMigration: Action Mapping", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 10},
        {"name", "Fireball"},
        {"description", "A ball of fire."},
        {"scope", 1}, // single enemy
        {"mpCost", 5},
        {"tpCost", 0},
        {"damage", {
            {"formula", "a.mat * 4 - b.mdf * 2"},
            {"variance", 20},
            {"critical", true}
        }},
        {"animationId", 50}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);

    REQUIRE(native["id"] == "SKL_10");
    REQUIRE(native["name"] == "Fireball");
    REQUIRE(native["scope"] == "single_enemy");
    REQUIRE(native["cost"]["mp"] == 5);
    REQUIRE(native["effects"].size() == 1);
    REQUIRE(native["effects"][0]["formula"] == "a.mat * 4 - b.mdf * 2");
    REQUIRE(native["animation_id"] == "ANI_50");
}
