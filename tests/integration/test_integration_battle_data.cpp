#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/battle_manager.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

using namespace urpg::compat;

namespace {

std::string getTestDataPath() {
    return std::string(URPG_SOURCE_DIR) + "/tests/data/mz_data";
}

void resetDataManager() {
    DataManager::instance().setDataPath("");
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();
}

} // namespace

TEST_CASE("Integration: BattleManager setup loads troop from DataManager", "[integration]") {
    resetDataManager();
    const auto path = getTestDataPath();
    DataManager::instance().setDataPath(path);
    REQUIRE(DataManager::instance().loadDatabase());
    DataManager::instance().setupNewGame();

    BattleManager::instance().setup(1);

    REQUIRE(BattleManager::instance().getPhase() == BattlePhase::INIT);

    const auto& enemies = BattleManager::instance().getEnemiesConst();
    REQUIRE(enemies.size() == 2);
    REQUIRE(enemies[0].id == 1);
    REQUIRE(enemies[0].type == BattleSubjectType::ENEMY);
    REQUIRE(enemies[0].hp == 100); // Slime mhp
    REQUIRE(enemies[1].id == 1);

    const auto& actors = BattleManager::instance().getActorsConst();
    REQUIRE(actors.size() == 2); // System.json partyMembers = [1,2]
    REQUIRE(actors[0].id == 1);
    REQUIRE(actors[0].type == BattleSubjectType::ACTOR);
    REQUIRE(actors[0].hp == 100);
}

TEST_CASE("Integration: Battle victory distributes rewards through DataManager", "[integration]") {
    resetDataManager();
    const auto path = getTestDataPath();
    DataManager::instance().setDataPath(path);
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    int32_t initialGold = DataManager::instance().getGold();

    BattleManager::instance().setup(1);
    BattleManager::instance().startBattle();

    // Kill all enemies
    auto enemies = BattleManager::instance().getEnemies();
    for (auto* enemy : enemies) {
        enemy->hp = 0;
    }

    // Trigger victory check
    BattleManager::instance().endTurn();

    REQUIRE(BattleManager::instance().getResult() == BattleResult::WIN);
    REQUIRE(BattleManager::instance().getPhase() == BattlePhase::NONE);

    // Slime*2, each gives 10 gold = 20 total
    REQUIRE(DataManager::instance().getGold() == initialGold + 20);
}

TEST_CASE("Integration: BattleManager switch conditions read DataManager", "[integration]") {
    resetDataManager();

    DataManager::instance().setSwitch(5, true);
    REQUIRE(BattleManager::instance().checkSwitchCondition(5));

    DataManager::instance().setSwitch(5, false);
    REQUIRE_FALSE(BattleManager::instance().checkSwitchCondition(5));
}

TEST_CASE("Integration: Battle action skill uses DataManager skill lookup", "[integration]") {
    resetDataManager();
    const auto path = getTestDataPath();
    DataManager::instance().setDataPath(path);
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager::instance().setup(1);
    BattleManager::instance().startBattle();

    const auto& enemies = BattleManager::instance().getEnemiesConst();
    REQUIRE(enemies.size() > 0);
    int32_t initialHp = enemies[0].hp;
    REQUIRE(initialHp > 0);

    auto* actor = BattleManager::instance().getActor(0);
    REQUIRE(actor != nullptr);

    BattleManager::instance().queueAction(actor, BattleActionType::SKILL, 0, 1);

    auto* action = BattleManager::instance().getNextAction();
    REQUIRE(action != nullptr);
    REQUIRE(action->type == BattleActionType::SKILL);
    REQUIRE(action->skillId == 1);

    BattleManager::instance().processAction(action);

    const auto& enemiesAfter = BattleManager::instance().getEnemiesConst();
    REQUIRE(enemiesAfter[0].hp < initialHp);
}
