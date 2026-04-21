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

TEST_CASE("BattleMigration: Troop Mapping maps show text commands", "[battle][migration][troop]") {
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
    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["condition"].is_object());
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "message");
    REQUIRE(progress.total_troops == 1);
    REQUIRE(progress.warnings.empty());
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

TEST_CASE("BattleMigration: Troop phase turn condition is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 3},
        {"name", "Turn Spawn"},
        {"members", {{{"enemyId", 1}, {"x", 100}, {"y", 200}}}},
        {"pages", {
            {
                {"conditions", {
                    {"turnValid", true},
                    {"turnA", 3},
                    {"turnB", 0},
                    {"turnEnding", false}
                }},
                {"list", {{{"code", 0}}}},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["condition"]["turn_count"] == 3);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "common_event");
    // No event-command warning because list only contains terminator {code:0}
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop phase enemy HP condition is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 4},
        {"name", "HP Threshold"},
        {"members", {{{"enemyId", 2}, {"x", 150}, {"y", 250}}}},
        {"pages", {
            {
                {"conditions", {
                    {"enemyValid", true},
                    {"enemyHp", 30},
                    {"enemyIndex", 1}
                }},
                {"list", {{{"code", 101}}, {{"code", 0}}}},
                {"span", 1}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["condition"]["hp_below_percent"] == 30);
    REQUIRE(native["phases"][0]["condition"]["enemy_index"] == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "message");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop phase switch condition is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 5},
        {"name", "Switch Trigger"},
        {"members", {{{"enemyId", 3}, {"x", 200}, {"y", 300}}}},
        {"pages", {
            {
                {"conditions", {
                    {"switchValid", true},
                    {"switchId", 5}
                }},
                {"list", {{{"code", 0}}}},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["condition"]["switch_id"] == "SW_5");
}

TEST_CASE("BattleMigration: Troop phase actor condition is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 6},
        {"name", "Actor Condition"},
        {"members", {{{"enemyId", 1}, {"x", 100}, {"y", 200}}}},
        {"pages", {
            {
                {"conditions", {
                    {"actorValid", true},
                    {"actorId", 1}
                }},
                {"list", {{{"code", 0}}}},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["condition"]["actor_id"] == "ACT_1");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop with multiple pages maps multiple phases", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 7},
        {"name", "Multi-Phase"},
        {"members", {{{"enemyId", 1}, {"x", 100}, {"y", 200}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {{{"code", 0}}}},
                {"span", 0}
            },
            {
                {"conditions", {{"turnValid", true}, {"turnA", 5}}},
                {"list", {{{"code", 0}}}},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 2);
    REQUIRE(native["phases"][0]["condition"]["turn_count"] == 1);
    REQUIRE(native["phases"][1]["condition"]["turn_count"] == 5);
}


TEST_CASE("BattleMigration: Action scope random enemy is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 20},
        {"name", "Random Shot"},
        {"scope", 3},
        {"mpCost", 4}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "random_enemy");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Action scope random ally is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 21},
        {"name", "Heal Roulette"},
        {"scope", 4},
        {"mpCost", 3}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "random_ally");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Action scope dead ally is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 22},
        {"name", "Raise Dead"},
        {"scope", 5},
        {"mpCost", 10}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "ally_dead");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Action scope all dead allies is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 23},
        {"name", "Mass Resurrection"},
        {"scope", 6},
        {"mpCost", 25}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "all_allies_dead");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Action scope enemy dead is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 24},
        {"name", "Exorcise"},
        {"scope", 9},
        {"mpCost", 8}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "enemy_dead");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Action scope all enemies dead is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 25},
        {"name", "Purge All"},
        {"scope", 10},
        {"mpCost", 15}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "all_enemies_dead");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Action scope ally except user is mapped", "[battle][migration][action]") {
    nlohmann::json rm_skill = {
        {"id", 26},
        {"name", "Team Buff"},
        {"scope", 12},
        {"mpCost", 6}
    };
    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateAction(rm_skill, false, progress);
    REQUIRE(native["scope"] == "ally_except_user");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Show Text command is mapped to message effect", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 10},
        {"name", "Talking Boss"},
        {"members", {{{"enemyId", 1}, {"x", 100}, {"y", 200}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 101}, {"parameters", {"You cannot defeat me!"}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "message");
    REQUIRE(native["phases"][0]["effects"][0]["text"] == "You cannot defeat me!");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Common Event command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 11},
        {"name", "Event Boss"},
        {"members", {{{"enemyId", 2}, {"x", 150}, {"y", 250}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 2}}},
                {"list", {
                    {{"code", 117}, {"parameters", {5}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "common_event");
    REQUIRE(native["phases"][0]["effects"][0]["event_id"] == 5);
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page unmapped commands emit partial warning", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 12},
        {"name", "Complex Boss"},
        {"members", {{{"enemyId", 3}, {"x", 200}, {"y", 300}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 101}, {"parameters", {"Hello!"}}},
                    {{"code", 999}, {"parameters", {}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    // First effect is the mapped message
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "message");
    // Second effect is the unmapped fallback
    REQUIRE(native["phases"][0]["effects"][1]["type"] == "common_event");
    REQUIRE(native["phases"][0]["effects"][1]["note"] == "unmapped_commands");
    REQUIRE(progress.warnings.size() == 1);
    REQUIRE(progress.warnings[0].find("unmapped") != std::string::npos);
}

TEST_CASE("BattleMigration: Troop page Change Gold command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 13},
        {"name", "Rich Boss"},
        {"members", {{{"enemyId", 1}, {"x", 100}, {"y", 200}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 125}, {"parameters", {0, 0, 500}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "change_gold");
    REQUIRE(native["phases"][0]["effects"][0]["operation"] == "increase");
    REQUIRE(native["phases"][0]["effects"][0]["value"] == 500);
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Change Items command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 14},
        {"name", "Item Boss"},
        {"members", {{{"enemyId", 2}, {"x", 150}, {"y", 250}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 126}, {"parameters", {1, 1, 0, 3}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "change_items");
    REQUIRE(native["phases"][0]["effects"][0]["item_id"] == "ITM_1");
    REQUIRE(native["phases"][0]["effects"][0]["operation"] == "decrease");
    REQUIRE(native["phases"][0]["effects"][0]["value"] == 3);
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Change Weapons command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 15},
        {"name", "Weapon Boss"},
        {"members", {{{"enemyId", 3}, {"x", 200}, {"y", 300}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 127}, {"parameters", {5, 0, 0, 1}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "change_weapons");
    REQUIRE(native["phases"][0]["effects"][0]["weapon_id"] == "WPN_5");
    REQUIRE(native["phases"][0]["effects"][0]["operation"] == "increase");
    REQUIRE(native["phases"][0]["effects"][0]["value"] == 1);
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Change Armors command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 16},
        {"name", "Armor Boss"},
        {"members", {{{"enemyId", 4}, {"x", 250}, {"y", 350}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 128}, {"parameters", {2, 0, 0, 1}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "change_armors");
    REQUIRE(native["phases"][0]["effects"][0]["armor_id"] == "ARM_2");
    REQUIRE(native["phases"][0]["effects"][0]["operation"] == "increase");
    REQUIRE(native["phases"][0]["effects"][0]["value"] == 1);
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Transfer Player command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 17},
        {"name", "Escape Boss"},
        {"members", {{{"enemyId", 5}, {"x", 300}, {"y", 400}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 201}, {"parameters", {2, 10, 15, 0}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "transfer_player");
    REQUIRE(native["phases"][0]["effects"][0]["map_id"] == "MAP_2");
    REQUIRE(native["phases"][0]["effects"][0]["x"] == 10);
    REQUIRE(native["phases"][0]["effects"][0]["y"] == 15);
    REQUIRE(native["phases"][0]["effects"][0]["direction"] == 0);
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page Game Over command is mapped", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 18},
        {"name", "Instant Death"},
        {"members", {{{"enemyId", 6}, {"x", 350}, {"y", 450}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 353}, {"parameters", {}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 1);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "game_over");
    REQUIRE(progress.warnings.empty());
}

TEST_CASE("BattleMigration: Troop page mixed mapped and unmapped commands", "[battle][migration][troop]") {
    nlohmann::json rm_troop = {
        {"id", 19},
        {"name", "Mixed Boss"},
        {"members", {{{"enemyId", 7}, {"x", 400}, {"y", 500}}}},
        {"pages", {
            {
                {"conditions", {{"turnValid", true}, {"turnA", 1}}},
                {"list", {
                    {{"code", 125}, {"parameters", {0, 0, 100}}},
                    {{"code", 353}, {"parameters", {}}},
                    {{"code", 999}, {"parameters", {}}},
                    {{"code", 0}}
                }},
                {"span", 0}
            }
        }}
    };

    BattleMigration::Progress progress;
    auto native = BattleMigration::migrateTroop(rm_troop, progress);

    REQUIRE(native["phases"].size() == 1);
    REQUIRE(native["phases"][0]["effects"].size() == 3);
    REQUIRE(native["phases"][0]["effects"][0]["type"] == "change_gold");
    REQUIRE(native["phases"][0]["effects"][1]["type"] == "game_over");
    REQUIRE(native["phases"][0]["effects"][2]["type"] == "common_event");
    REQUIRE(native["phases"][0]["effects"][2]["note"] == "unmapped_commands");
    REQUIRE(progress.warnings.size() == 1);
    REQUIRE(progress.warnings[0].find("unmapped") != std::string::npos);
}
