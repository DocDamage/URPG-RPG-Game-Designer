#include "runtimes/compat_js/game_party.h"
#include "runtimes/compat_js/game_troop.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

TEST_CASE("GameParty: member management", "[game_party]") {
    GameParty party;

    REQUIRE(party.isEmpty());
    REQUIRE(party.size() == 0);

    party.addActor(1);
    REQUIRE(party.size() == 1);
    REQUIRE(party.exists(1));
    REQUIRE_FALSE(party.isEmpty());

    party.addActor(2);
    REQUIRE(party.size() == 2);
    REQUIRE(party.exists(2));

    // Duplicate add should be ignored
    party.addActor(1);
    REQUIRE(party.size() == 2);

    party.removeActor(1);
    REQUIRE(party.size() == 1);
    REQUIRE_FALSE(party.exists(1));
    REQUIRE(party.exists(2));

    party.removeActor(2);
    REQUIRE(party.isEmpty());
    REQUIRE(party.size() == 0);

    party.setMembers({3, 4, 5});
    REQUIRE(party.size() == 3);
    REQUIRE(party.members() == std::vector<int32_t>{3, 4, 5});
}

TEST_CASE("GameParty: gold management", "[game_party]") {
    GameParty party;

    REQUIRE(party.gold() == 0);
    REQUIRE_FALSE(party.hasGold(1));

    party.setGold(1000);
    REQUIRE(party.gold() == 1000);
    REQUIRE(party.hasGold(1000));
    REQUIRE(party.hasGold(500));
    REQUIRE_FALSE(party.hasGold(1001));

    party.gainGold(250);
    REQUIRE(party.gold() == 1250);

    party.loseGold(50);
    REQUIRE(party.gold() == 1200);

    // Bounds at 0
    party.loseGold(5000);
    REQUIRE(party.gold() == 0);

    party.setGold(-100);
    REQUIRE(party.gold() == 0);

    party.gainGold(-50);
    REQUIRE(party.gold() == 0);
}

TEST_CASE("GameParty: item management", "[game_party]") {
    GameParty party;

    REQUIRE(party.numItems(1) == 0);
    REQUIRE_FALSE(party.hasItem(1));

    party.gainItem(1, 5);
    REQUIRE(party.numItems(1) == 5);
    REQUIRE(party.hasItem(1));

    party.loseItem(1, 3);
    REQUIRE(party.numItems(1) == 2);

    party.loseItem(1, 50);
    REQUIRE(party.numItems(1) == 0);
    REQUIRE_FALSE(party.hasItem(1));

    // Erase at 0
    party.gainItem(2, 3);
    party.gainItem(2, -3);
    REQUIRE(party.numItems(2) == 0);
    REQUIRE_FALSE(party.hasItem(2));
}

TEST_CASE("GameParty: weapon and armor management", "[game_party]") {
    GameParty party;

    // Weapons
    REQUIRE(party.numWeapons(1) == 0);
    party.gainWeapon(1, 3);
    REQUIRE(party.numWeapons(1) == 3);

    party.loseWeapon(1, 2);
    REQUIRE(party.numWeapons(1) == 1);

    party.loseWeapon(1, 10);
    REQUIRE(party.numWeapons(1) == 0);

    party.gainWeapon(2, 2);
    party.gainWeapon(2, -2);
    REQUIRE(party.numWeapons(2) == 0);

    // Armors
    REQUIRE(party.numArmors(1) == 0);
    party.gainArmor(1, 4);
    REQUIRE(party.numArmors(1) == 4);

    party.loseArmor(1, 1);
    REQUIRE(party.numArmors(1) == 3);

    party.loseArmor(1, 100);
    REQUIRE(party.numArmors(1) == 0);

    party.gainArmor(2, 1);
    party.gainArmor(2, -1);
    REQUIRE(party.numArmors(2) == 0);
}

TEST_CASE("GameParty: steps", "[game_party]") {
    GameParty party;

    REQUIRE(party.steps() == 0);

    party.increaseSteps();
    REQUIRE(party.steps() == 1);

    party.increaseSteps(5);
    REQUIRE(party.steps() == 6);

    party.setSteps(100);
    REQUIRE(party.steps() == 100);

    // Negative amount should be clamped
    party.increaseSteps(-10);
    REQUIRE(party.steps() == 100);
}

TEST_CASE("GameTroop: member management", "[game_troop]") {
    GameTroop troop;

    REQUIRE(troop.isEmpty());
    REQUIRE(troop.size() == 0);

    troop.setMembers({1, 2, 3});
    REQUIRE(troop.size() == 3);
    REQUIRE_FALSE(troop.isEmpty());
    REQUIRE(troop.members() == std::vector<int32_t>{1, 2, 3});

    troop.setMembers({});
    REQUIRE(troop.isEmpty());
    REQUIRE(troop.size() == 0);
}

TEST_CASE("GameTroop: totalExp and totalGold with mock enemy data", "[game_troop]") {
    DataManager::instance().clearDatabase();

    // Add all enemies first (avoid reference invalidation across push_back)
    DataManager::instance().addTestEnemy();
    DataManager::instance().addTestEnemy();

    EnemyData* enemy1 = DataManager::instance().getEnemy(1);
    REQUIRE(enemy1 != nullptr);
    enemy1->exp = 25;
    enemy1->gold = 15;

    EnemyData* enemy2 = DataManager::instance().getEnemy(2);
    REQUIRE(enemy2 != nullptr);
    enemy2->exp = 40;
    enemy2->gold = 30;

    GameTroop troop;
    troop.setMembers({1, 2});

    REQUIRE(troop.totalExp() == 65);
    REQUIRE(troop.totalGold() == 45);

    // Member that doesn't resolve returns 0
    troop.setMembers({1, 999, 2});
    REQUIRE(troop.totalExp() == 65);
    REQUIRE(troop.totalGold() == 45);

    // Empty troop
    troop.setMembers({});
    REQUIRE(troop.totalExp() == 0);
    REQUIRE(troop.totalGold() == 0);

    DataManager::instance().clearDatabase();
}
