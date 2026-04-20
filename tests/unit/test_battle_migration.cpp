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

TEST_CASE("BattleMigration: Troop Mapping warns when troop pages are dropped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 2},
        {"name", "Goblin Ambush"},
        {"members", {
            {{"enemyId", 3}, {"x", 140}, {"y", 210}, {"hidden", true}}
        }},
        {"pages", {
            {
                {"conditions", {{"turnEnding", true}}},
                {"list", {{{"code", 101}}}}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["id"] == "TRP_2");
    REQUIRE(native["members"].size() == 1);
    REQUIRE(native["phases"].is_array());
    REQUIRE(native["phases"].empty());
    REQUIRE(progress.total_troops == 1);
    REQUIRE(progress.warnings.size() == 1);
    REQUIRE(progress.warnings[0].find("pages") != std::string::npos);
}

TEST_CASE("BattleMigration: Action Mapping warns on unsupported scope and effects", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 11},
        {"name", "Chaos Burst"},
        {"description", "Unstable effect payload."},
        {"scope", 99},
        {"mpCost", 9},
        {"tpCost", 3},
        {"damage", {
            {"formula", "a.atk * 2"},
            {"variance", 15},
            {"critical", false}
        }},
        {"effects", {
            {{"code", 21}, {"dataId", 4}, {"value1", 1.0}}
        }},
        {"animationId", 12}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);

    REQUIRE(native["id"] == "SKL_11");
    REQUIRE(native["scope"] == "none");
    REQUIRE(native["cost"]["mp"] == 9);
    REQUIRE(native["cost"]["tp"] == 3);
    REQUIRE(native["effects"].size() == 1);
    REQUIRE(native["effects"][0]["type"] == "damage");
    REQUIRE(progress.total_actions == 1);
    REQUIRE(progress.warnings.size() == 2);
    REQUIRE(progress.warnings[0].find("scope") != std::string::npos);
    REQUIRE(progress.warnings[1].find("effects") != std::string::npos);
}
