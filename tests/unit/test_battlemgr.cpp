#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <variant>
#include <nlohmann/json.hpp>

using namespace urpg::compat;
namespace fs = std::filesystem;

namespace {

fs::path writeBattleEventTroopsFixture(const nlohmann::json& troops) {
    const fs::path fixtureDir = fs::temp_directory_path() / "urpg_battle_event_fixture";
    fs::create_directories(fixtureDir);

    std::ofstream out(fixtureDir / "Troops.json", std::ios::binary | std::ios::trunc);
    out << troops.dump();
    out.close();

    return fixtureDir;
}

}

TEST_CASE("BattleManager: setup and flow", "[battlemgr]") {
    BattleManager bm;

    bm.setup(1, true, false);
    REQUIRE(bm.getPhase() == BattlePhase::INIT);
    REQUIRE(bm.getTurnCount() == 0);
    REQUIRE_FALSE(bm.canEscape());

    bm.startBattle();
    REQUIRE(bm.getPhase() == BattlePhase::INPUT);
    REQUIRE(bm.isBattleActive());
    REQUIRE(bm.canEscape());

    bm.abortBattle();
    REQUIRE(bm.getPhase() == BattlePhase::NONE);
    REQUIRE(bm.getResult() == BattleResult::ABORT);
    REQUIRE_FALSE(bm.isBattleActive());
}

TEST_CASE("BattleManager: escape gate", "[battlemgr]") {
    BattleManager bm;
    bm.setup(2, false, false);
    bm.startBattle();

    REQUIRE_FALSE(bm.canEscape());
    REQUIRE_FALSE(bm.processEscape());
}

TEST_CASE("BattleManager: escape attempts ramp to deterministic success", "[battlemgr]") {
    BattleManager bm;
    bm.setup(11, true, false);
    bm.startBattle();

    bool escaped = false;
    int32_t attempts = 0;
    while (attempts < 6 && bm.canEscape()) {
        ++attempts;
        escaped = bm.processEscape();
        if (escaped) {
            break;
        }
    }

    REQUIRE(escaped);
    REQUIRE(attempts <= 6);
    REQUIRE(bm.getResult() == BattleResult::ESCAPE);
    REQUIRE(bm.getPhase() == BattlePhase::NONE);
}

TEST_CASE("BattleManager: escape outcomes are deterministic for same setup", "[battlemgr]") {
    BattleManager a;
    BattleManager b;

    a.setup(17, true, false);
    b.setup(17, true, false);
    a.startBattle();
    b.startBattle();

    bool aFirst = a.processEscape();
    bool bFirst = b.processEscape();
    REQUIRE(aFirst == bFirst);
    REQUIRE(a.getResult() == b.getResult());
}

TEST_CASE("BattleManager: hook registration and unregistration", "[battlemgr]") {
    BattleManager bm;
    int startHookCalls = 0;

    bm.registerHook(
        BattleManager::HookPoint::ON_START,
        "test_plugin",
        [&startHookCalls](const std::vector<urpg::Value>&) -> urpg::Value {
            ++startHookCalls;
            return urpg::Value::Nil();
        });

    bm.setup(1, true, false);
    bm.startBattle();
    REQUIRE(startHookCalls == 1);

    bm.unregisterHooks("test_plugin");
    bm.setup(1, true, false);
    bm.startBattle();
    REQUIRE(startHookCalls == 1);
}

TEST_CASE("BattleManager: action queue lifecycle", "[battlemgr]") {
    BattleManager bm;

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 100;
    actor.mhp = 100;

    bm.queueAction(&actor, BattleActionType::WAIT);

    BattleAction* action = bm.getNextAction();
    REQUIRE(action != nullptr);
    REQUIRE(action->subject == &actor);
    REQUIRE(action->type == BattleActionType::WAIT);

    bm.processAction(action);
    REQUIRE(bm.getNextAction() == nullptr);

    bm.queueAction(&actor, BattleActionType::ATTACK, 0);
    bm.clearActions();
    REQUIRE(bm.getNextAction() == nullptr);
}

TEST_CASE("BattleManager: damage and healing", "[battlemgr]") {
    BattleManager bm;

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 60;
    actor.mhp = 100;
    actor.mp = 20;
    actor.mmp = 30;

    bm.applyDamage(&actor, 25);
    REQUIRE(actor.hp == 35);

    bm.applyHeal(&actor, 20);
    REQUIRE(actor.hp == 55);

    bm.applyHeal(&actor, 100);
    REQUIRE(actor.hp == actor.mhp);

    bm.applyDamage(&actor, 7, false);
    REQUIRE(actor.mp == 13);

    actor.hp = 80;
    REQUIRE(bm.addBuff(&actor, 4, 2, 1));
    bm.applyHeal(&actor, 8);
    REQUIRE(actor.hp == 90);
}

TEST_CASE("BattleManager: modifiers affect attack resolution and targeting priority", "[battlemgr]") {
    BattleManager bm;
    bm.setup(12, true, false);

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 40;
    actor.mhp = 40;

    BattleSubject enemyA;
    enemyA.type = BattleSubjectType::ENEMY;
    enemyA.index = 0;
    enemyA.hp = 20;
    enemyA.mhp = 20;

    BattleSubject enemyB;
    enemyB.type = BattleSubjectType::ENEMY;
    enemyB.index = 1;
    enemyB.hp = 20;
    enemyB.mhp = 20;

    bm.addActorSubject(actor);
    bm.addEnemySubject(enemyA);
    bm.addEnemySubject(enemyB);

    BattleSubject* seededActor = bm.getActor(0);
    BattleSubject* seededEnemyA = bm.getEnemy(0);
    BattleSubject* seededEnemyB = bm.getEnemy(1);
    REQUIRE(seededActor != nullptr);
    REQUIRE(seededEnemyA != nullptr);
    REQUIRE(seededEnemyB != nullptr);

    REQUIRE(bm.addBuff(seededActor, 2, 2, 1));
    REQUIRE(bm.addDebuff(seededEnemyB, 3, 2, 1));
    REQUIRE(bm.addDebuff(seededEnemyB, 6, 2, 1));

    bm.startBattle();
    bm.autoBattleActor(0);
    BattleAction* action = bm.getNextAction();
    REQUIRE(action != nullptr);
    REQUIRE(action->targetIndex == 1);

    bm.processAction(action);
    REQUIRE(seededEnemyA->hp == 20);
    REQUIRE(seededEnemyB->hp == 4);
}

TEST_CASE("BattleManager: state effects and outcome hooks", "[battlemgr]") {
    BattleManager bm;
    int stateAddedCount = 0;
    int stateRemovedCount = 0;
    int actorDeathCount = 0;
    int victoryCount = 0;

    bm.registerHook(
        BattleManager::HookPoint::ON_STATE_ADDED,
        "test_plugin",
        [&stateAddedCount](const std::vector<urpg::Value>&) -> urpg::Value {
            ++stateAddedCount;
            return urpg::Value::Nil();
        });
    bm.registerHook(
        BattleManager::HookPoint::ON_STATE_REMOVED,
        "test_plugin",
        [&stateRemovedCount](const std::vector<urpg::Value>&) -> urpg::Value {
            ++stateRemovedCount;
            return urpg::Value::Nil();
        });
    bm.registerHook(
        BattleManager::HookPoint::ON_ACTOR_DEATH,
        "test_plugin",
        [&actorDeathCount](const std::vector<urpg::Value>&) -> urpg::Value {
            ++actorDeathCount;
            return urpg::Value::Nil();
        });
    bm.registerHook(
        BattleManager::HookPoint::ON_VICTORY,
        "test_plugin",
        [&victoryCount](const std::vector<urpg::Value>&) -> urpg::Value {
            ++victoryCount;
            return urpg::Value::Nil();
        });

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 9;
    actor.mhp = 20;
    actor.mp = 3;
    actor.mmp = 10;

    BattleSubject enemy;
    enemy.type = BattleSubjectType::ENEMY;
    enemy.index = 0;
    enemy.hp = 10;
    enemy.mhp = 10;

    REQUIRE(bm.addState(&actor, 4, 1, -12, 2));
    REQUIRE(bm.addState(&enemy, 5, 1, 6, 0));
    REQUIRE(bm.hasState(&actor, 4));
    REQUIRE(bm.hasState(&enemy, 5));
    REQUIRE(stateAddedCount == 2);

    bm.applyStateEffects(&actor);
    bm.applyStateEffects(&enemy);
    REQUIRE(actor.hp == 0);
    REQUIRE(actor.mp == 5);
    REQUIRE(enemy.hp == 10);
    REQUIRE_FALSE(bm.hasState(&actor, 4));
    REQUIRE_FALSE(bm.hasState(&enemy, 5));
    REQUIRE(stateRemovedCount == 2);
    REQUIRE(actorDeathCount == 1);

    bm.setup(3, true, false);
    bm.startBattle();
    bm.endBattle(BattleResult::WIN);
    REQUIRE(victoryCount == 1);
    REQUIRE(bm.getResult() == BattleResult::WIN);
    REQUIRE(bm.getPhase() == BattlePhase::NONE);
}

TEST_CASE("BattleManager: turn-end status ordering and buff durations", "[battlemgr]") {
    BattleManager bm;
    std::vector<std::string> damageOrder;

    bm.registerHook(
        BattleManager::HookPoint::ON_DAMAGE,
        "test_plugin",
        [&damageOrder](const std::vector<urpg::Value>& args) -> urpg::Value {
            const auto subjectType = std::get<int64_t>(args[0].v);
            const auto subjectIndex = std::get<int64_t>(args[1].v);
            damageOrder.push_back(std::to_string(subjectType) + ":" + std::to_string(subjectIndex));
            return urpg::Value::Nil();
        });

    bm.setup(9, true, false);

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 20;
    actor.mhp = 20;

    BattleSubject enemy;
    enemy.type = BattleSubjectType::ENEMY;
    enemy.index = 0;
    enemy.hp = 6;
    enemy.mhp = 6;

    bm.addActorSubject(actor);
    bm.addEnemySubject(enemy);

    BattleSubject* seededActor = bm.getActor(0);
    BattleSubject* seededEnemy = bm.getEnemy(0);
    REQUIRE(seededActor != nullptr);
    REQUIRE(seededEnemy != nullptr);

    REQUIRE(bm.addState(seededActor, 10, 1, -3, 0));
    REQUIRE(bm.addState(seededEnemy, 11, 1, -6, 0));
    REQUIRE(bm.addBuff(seededActor, 6, 2, 1));
    REQUIRE(bm.addDebuff(seededEnemy, 2, 1, 1));
    REQUIRE(bm.getModifierStage(seededActor, 6) == 1);
    REQUIRE(bm.getModifierStage(seededEnemy, 2) == -1);

    bm.startBattle();
    bm.startTurn();
    bm.endTurn();

    REQUIRE(damageOrder.size() == 2);
    REQUIRE(damageOrder[0] == "0:0");
    REQUIRE(damageOrder[1] == "1:0");
    REQUIRE(seededActor->hp == 17);
    REQUIRE(seededEnemy->hp == 0);
    REQUIRE_FALSE(bm.hasState(seededActor, 10));
    REQUIRE_FALSE(bm.hasState(seededEnemy, 11));
    REQUIRE(bm.getModifierStage(seededActor, 6) == 1);
    REQUIRE(bm.getModifierStage(seededEnemy, 2) == 0);
    REQUIRE(bm.getResult() == BattleResult::WIN);

    bm.startBattle();
    bm.startTurn();
    bm.endTurn();
    REQUIRE(bm.getModifierStage(seededActor, 6) == 0);
}

TEST_CASE("BattleManager: battle environment metadata is retained through JS helpers", "[battlemgr]") {
    QuickJSContext ctx;
    QuickJSConfig config;
    REQUIRE(ctx.initialize(config));

    BattleManager::registerAPI(ctx);

    urpg::Value ruinsNight;
    ruinsNight.v = std::string("ruins_night");

    auto setBackground = ctx.callMethod("BattleManager", "changeBattleBackground",
                                        std::vector<urpg::Value>{ruinsNight});
    REQUIRE(setBackground.success);

    urpg::Value battleTheme;
    battleTheme.v = std::string("battle_theme");
    urpg::Value bgmVolume;
    bgmVolume.v = 72.5;
    urpg::Value bgmPitch;
    bgmPitch.v = 108.0;
    auto setBgm = ctx.callMethod("BattleManager", "changeBattleBgm",
                                 std::vector<urpg::Value>{battleTheme, bgmVolume, bgmPitch});
    REQUIRE(setBgm.success);

    urpg::Value victoryName;
    victoryName.v = std::string("victory_fanfare");
    auto setVictoryMe = ctx.callMethod("BattleManager", "changeVictoryMe",
                                       std::vector<urpg::Value>{victoryName,
                                                                 urpg::Value::Int(80),
                                                                 urpg::Value::Int(105)});
    REQUIRE(setVictoryMe.success);

    urpg::Value defeatName;
    defeatName.v = std::string("defeat_dirge");
    auto setDefeatMe = ctx.callMethod("BattleManager", "changeDefeatMe",
                                      std::vector<urpg::Value>{defeatName,
                                                                urpg::Value::Int(60),
                                                                urpg::Value::Int(95)});
    REQUIRE(setDefeatMe.success);

    auto backgroundResult = ctx.callMethod("BattleManager", "getBattleBackground", {});
    REQUIRE(backgroundResult.success);
    REQUIRE(std::get<std::string>(backgroundResult.value.v) == "ruins_night");

    auto bgmResult = ctx.callMethod("BattleManager", "getBattleBgm", {});
    REQUIRE(bgmResult.success);
    REQUIRE(std::holds_alternative<urpg::Object>(bgmResult.value.v));
    const auto& bgm = std::get<urpg::Object>(bgmResult.value.v);
    REQUIRE(std::get<std::string>(bgm.at("name").v) == "battle_theme");
    REQUIRE(std::get<double>(bgm.at("volume").v) == 72.5);
    REQUIRE(std::get<double>(bgm.at("pitch").v) == 108.0);

    auto victoryResult = ctx.callMethod("BattleManager", "getVictoryMe", {});
    REQUIRE(victoryResult.success);
    REQUIRE(std::holds_alternative<urpg::Object>(victoryResult.value.v));
    const auto& victory = std::get<urpg::Object>(victoryResult.value.v);
    REQUIRE(std::get<std::string>(victory.at("name").v) == "victory_fanfare");
    REQUIRE(std::get<double>(victory.at("volume").v) == 80.0);
    REQUIRE(std::get<double>(victory.at("pitch").v) == 105.0);

    auto defeatResult = ctx.callMethod("BattleManager", "getDefeatMe", {});
    REQUIRE(defeatResult.success);
    REQUIRE(std::holds_alternative<urpg::Object>(defeatResult.value.v));
    const auto& defeat = std::get<urpg::Object>(defeatResult.value.v);
    REQUIRE(std::get<std::string>(defeat.at("name").v) == "defeat_dirge");
    REQUIRE(std::get<double>(defeat.at("volume").v) == 60.0);
    REQUIRE(std::get<double>(defeat.at("pitch").v) == 95.0);
}

TEST_CASE("BattleManager: direct animation playback runs deterministic lifecycle", "[battlemgr]") {
    DataManager::instance().loadDatabase();

    BattleManager bm;
    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 100;
    actor.mhp = 100;

    bm.playAnimation(0, &actor);
    REQUIRE_FALSE(bm.isAnimationPlaying());
    REQUIRE(bm.getActiveAnimations().empty());

    bm.playAnimation(41, &actor);
    REQUIRE(bm.isAnimationPlaying());
    REQUIRE(bm.getActiveAnimations().size() == 1);
    REQUIRE(bm.getActiveAnimations()[0].animationId == 41);
    REQUIRE(bm.getActiveAnimations()[0].targetType == BattleSubjectType::ACTOR);
    REQUIRE(bm.getActiveAnimations()[0].targetIndex == 0);
    REQUIRE_FALSE(bm.getActiveAnimations()[0].subjectAnimation);

    const int32_t frames = bm.getActiveAnimations()[0].framesRemaining;
    REQUIRE(frames > 0);
    for (int32_t i = 0; i < frames - 1; ++i) {
        REQUIRE(bm.isAnimationPlaying());
        bm.updateBattleEvents();
    }

    REQUIRE(bm.isAnimationPlaying());
    bm.updateBattleEvents();
    REQUIRE_FALSE(bm.isAnimationPlaying());
    REQUIRE(bm.getActiveAnimations().empty());
}

TEST_CASE("BattleManager: skills and items enqueue their configured animations", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 40;
    actor.mhp = 100;
    bm.addActorSubject(actor);
    BattleSubject* seededActor = bm.getActor(static_cast<int32_t>(bm.getActorsConst().size() - 1));
    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(seededActor != nullptr);
    REQUIRE(enemy != nullptr);

    bm.applySkill(nullptr, enemy, 2);
    REQUIRE(bm.isAnimationPlaying());
    REQUIRE(bm.getActiveAnimations().size() == 1);
    REQUIRE(bm.getActiveAnimations()[0].animationId == 67);
    REQUIRE(bm.getActiveAnimations()[0].targetType == BattleSubjectType::ENEMY);
    REQUIRE(bm.getActiveAnimations()[0].targetIndex == enemy->index);

    bm.applyItem(nullptr, seededActor, 1);
    REQUIRE(bm.getActiveAnimations().size() == 2);
    REQUIRE(bm.getActiveAnimations()[1].animationId == 41);
    REQUIRE(bm.getActiveAnimations()[1].targetType == BattleSubjectType::ACTOR);
    REQUIRE(bm.getActiveAnimations()[1].targetIndex == seededActor->index);
}

TEST_CASE("BattleManager: troop battle events execute real page conditions and commands", "[battlemgr]") {
    const nlohmann::json troops = nlohmann::json::array({
        nullptr,
        {
            {"id", 1},
            {"name", "Event Troop"},
            {"members", nlohmann::json::array({
                {{"enemyId", 1}}
            })},
            {"pages", nlohmann::json::array({
                {
                    {"conditions", {
                        {"actorHp", 50},
                        {"actorId", 1},
                        {"actorValid", false},
                        {"enemyHp", 50},
                        {"enemyIndex", 0},
                        {"enemyValid", false},
                        {"switchId", 1},
                        {"switchValid", false},
                        {"turnA", 0},
                        {"turnB", 0},
                        {"turnEnding", false},
                        {"turnValid", false}
                    }},
                    {"list", nlohmann::json::array({
                        {{"code", 121}, {"indent", 0}, {"parameters", nlohmann::json::array({1, 1, 0})}},
                        {{"code", 122}, {"indent", 0}, {"parameters", nlohmann::json::array({2, 2, 0, 0, 7})}},
                        {{"code", 125}, {"indent", 0}, {"parameters", nlohmann::json::array({0, 0, 50})}},
                        {{"code", 0}, {"indent", 0}, {"parameters", nlohmann::json::array()}}
                    })},
                    {"span", 0}
                }
            })}
        }
    });

    const fs::path fixtureDir = writeBattleEventTroopsFixture(troops);
    DataManager::setDataDirectory(fixtureDir.string());
    REQUIRE(DataManager::instance().loadDatabase());
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);
    bm.startBattle();

    REQUIRE_FALSE(DataManager::instance().getSwitch(1));
    REQUIRE(DataManager::instance().getVariable(2) == 0);
    REQUIRE(DataManager::instance().getGold() == 0);

    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getSwitch(1));
    REQUIRE(DataManager::instance().getVariable(2) == 7);
    REQUIRE(DataManager::instance().getGold() == 50);
    REQUIRE_FALSE(bm.isBattleEventActive());

    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getGold() == 50);

    fs::remove_all(fixtureDir);
    DataManager::setDataDirectory("");
}

TEST_CASE("BattleManager: troop battle events honor live switch state through updateBattleEvents", "[battlemgr]") {
    const nlohmann::json troops = nlohmann::json::array({
        nullptr,
        {
            {"id", 1},
            {"name", "Switch Event Troop"},
            {"members", nlohmann::json::array({
                {{"enemyId", 1}}
            })},
            {"pages", nlohmann::json::array({
                {
                    {"conditions", {
                        {"actorHp", 50},
                        {"actorId", 1},
                        {"actorValid", false},
                        {"enemyHp", 50},
                        {"enemyIndex", 0},
                        {"enemyValid", false},
                        {"switchId", 33},
                        {"switchValid", true},
                        {"turnA", 0},
                        {"turnB", 0},
                        {"turnEnding", false},
                        {"turnValid", false}
                    }},
                    {"list", nlohmann::json::array({
                        {{"code", 122}, {"indent", 0}, {"parameters", nlohmann::json::array({9, 9, 0, 0, 4})}},
                        {{"code", 0}, {"indent", 0}, {"parameters", nlohmann::json::array()}}
                    })},
                    {"span", 0}
                }
            })}
        }
    });

    const fs::path fixtureDir = writeBattleEventTroopsFixture(troops);
    DataManager::setDataDirectory(fixtureDir.string());
    REQUIRE(DataManager::instance().loadDatabase());
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);
    bm.startBattle();

    REQUIRE_FALSE(DataManager::instance().getSwitch(33));
    REQUIRE(DataManager::instance().getVariable(9) == 0);

    bm.updateBattleEvents();
    REQUIRE_FALSE(bm.isBattleEventActive());
    REQUIRE(DataManager::instance().getVariable(9) == 0);

    DataManager::instance().setSwitch(33, true);
    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getVariable(9) == 4);
    REQUIRE_FALSE(bm.isBattleEventActive());

    fs::remove_all(fixtureDir);
    DataManager::setDataDirectory("");
}

TEST_CASE("BattleManager: troop battle events honor variable conditional branches against live DataManager state", "[battlemgr]") {
    const nlohmann::json troops = nlohmann::json::array({
        nullptr,
        {
            {"id", 1},
            {"name", "Variable Branch Troop"},
            {"members", nlohmann::json::array({
                {{"enemyId", 1}}
            })},
            {"pages", nlohmann::json::array({
                {
                    {"conditions", {
                        {"actorHp", 50},
                        {"actorId", 1},
                        {"actorValid", false},
                        {"enemyHp", 50},
                        {"enemyIndex", 0},
                        {"enemyValid", false},
                        {"switchId", 1},
                        {"switchValid", false},
                        {"turnA", 0},
                        {"turnB", 0},
                        {"turnEnding", false},
                        {"turnValid", true}
                    }},
                    {"list", nlohmann::json::array({
                        {{"code", 111}, {"indent", 0}, {"parameters", nlohmann::json::array({1, 7, 0, 3, 1})}},
                        {{"code", 122}, {"indent", 1}, {"parameters", nlohmann::json::array({8, 8, 0, 0, 9})}},
                        {{"code", 411}, {"indent", 0}, {"parameters", nlohmann::json::array()}},
                        {{"code", 122}, {"indent", 1}, {"parameters", nlohmann::json::array({8, 8, 0, 0, 4})}},
                        {{"code", 412}, {"indent", 0}, {"parameters", nlohmann::json::array()}},
                        {{"code", 0}, {"indent", 0}, {"parameters", nlohmann::json::array()}}
                    })},
                    {"span", 0}
                }
            })}
        }
    });

    const fs::path fixtureDir = writeBattleEventTroopsFixture(troops);
    DataManager::setDataDirectory(fixtureDir.string());
    REQUIRE(DataManager::instance().loadDatabase());
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);
    bm.startBattle();

    DataManager::instance().setVariable(7, 2);
    REQUIRE(DataManager::instance().getVariable(8) == 0);

    bm.startBattleEvent(1);
    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getVariable(8) == 4);

    DataManager::instance().setVariable(7, 3);
    bm.startBattleEvent(1);
    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getVariable(8) == 9);

    fs::remove_all(fixtureDir);
    DataManager::setDataDirectory("");
}

TEST_CASE("BattleManager: turn-span battle events rerun on later eligible turns", "[battlemgr]") {
    const nlohmann::json troops = nlohmann::json::array({
        nullptr,
        {
            {"id", 1},
            {"name", "Turn Event Troop"},
            {"members", nlohmann::json::array({
                {{"enemyId", 1}}
            })},
            {"pages", nlohmann::json::array({
                {
                    {"conditions", {
                        {"actorHp", 50},
                        {"actorId", 1},
                        {"actorValid", false},
                        {"enemyHp", 50},
                        {"enemyIndex", 0},
                        {"enemyValid", false},
                        {"switchId", 1},
                        {"switchValid", false},
                        {"turnA", 1},
                        {"turnB", 1},
                        {"turnEnding", false},
                        {"turnValid", true}
                    }},
                    {"list", nlohmann::json::array({
                        {{"code", 122}, {"indent", 0}, {"parameters", nlohmann::json::array({3, 3, 1, 0, 2})}},
                        {{"code", 0}, {"indent", 0}, {"parameters", nlohmann::json::array()}}
                    })},
                    {"span", 1}
                }
            })}
        }
    });

    const fs::path fixtureDir = writeBattleEventTroopsFixture(troops);
    DataManager::setDataDirectory(fixtureDir.string());
    REQUIRE(DataManager::instance().loadDatabase());
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);
    bm.startBattle();

    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getVariable(3) == 0);

    bm.endTurn();
    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getVariable(3) == 2);

    bm.endTurn();
    bm.updateBattleEvents();
    REQUIRE(DataManager::instance().getVariable(3) == 4);

    fs::remove_all(fixtureDir);
    DataManager::setDataDirectory("");
}

TEST_CASE("BattleManager: method status registry", "[battlemgr]") {
    BattleManager bm;
    (void)bm;

    REQUIRE(BattleManager::getMethodStatus("setup") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("applySkill") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("applyItem") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("startBattleEvent") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("updateBattleEvents") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("calculateExp") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("processEscape") == CompatStatus::FULL);
    REQUIRE(BattleManager::getMethodStatus("setBattleTransition") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("changeBattleBgm") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("playAnimation") == CompatStatus::FULL);
    REQUIRE(BattleManager::getMethodDeviation("setup").find("partial") != std::string::npos);
    REQUIRE(BattleManager::getMethodDeviation("applySkill").find("bounded arithmetic formula subset") != std::string::npos);
    REQUIRE(BattleManager::getMethodDeviation("applyItem").find("bounded arithmetic formula subset") != std::string::npos);
    REQUIRE(BattleManager::getMethodDeviation("startBattleEvent").find("Conditional Branch currently supports switch, variable") != std::string::npos);
    REQUIRE(BattleManager::getMethodDeviation("updateBattleEvents").find("Conditional Branch currently supports switch, variable") != std::string::npos);
    REQUIRE(BattleManager::getMethodDeviation("calculateExp").find("enemy loader is still partial") != std::string::npos);
    REQUIRE(BattleManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("BattleManager: turn condition cadence honors threshold and span", "[battlemgr]") {
    BattleManager bm;

    SECTION("span 0 matches only the exact turn") {
        REQUIRE_FALSE(bm.checkTurnCondition(2, 0));
        bm.incrementTurn();
        bm.incrementTurn();
        REQUIRE(bm.checkTurnCondition(2, 0));
        bm.incrementTurn();
        REQUIRE_FALSE(bm.checkTurnCondition(2, 0));
    }

    SECTION("span 1 triggers on the threshold turn and every turn after") {
        REQUIRE_FALSE(bm.checkTurnCondition(2, 1));
        bm.incrementTurn();
        REQUIRE_FALSE(bm.checkTurnCondition(2, 1));
        bm.incrementTurn();
        REQUIRE(bm.checkTurnCondition(2, 1));
        bm.incrementTurn();
        REQUIRE(bm.checkTurnCondition(2, 1));
    }

    SECTION("span 2 triggers on cadence after the threshold turn") {
        REQUIRE_FALSE(bm.checkTurnCondition(2, 2));
        bm.incrementTurn();
        REQUIRE_FALSE(bm.checkTurnCondition(2, 2));
        bm.incrementTurn();
        REQUIRE(bm.checkTurnCondition(2, 2));
        bm.incrementTurn();
        REQUIRE_FALSE(bm.checkTurnCondition(2, 2));
        bm.incrementTurn();
        REQUIRE(bm.checkTurnCondition(2, 2));
    }
}

TEST_CASE("BattleManager: troop setup creates enemies from DataManager", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);
    REQUIRE(bm.getEnemiesConst().size() == 2); // Troop 1 has two Slimes
    REQUIRE(bm.getActorsConst().size() == 1);  // Party has Hero (actor 1)
}

TEST_CASE("BattleManager: defeating all enemies yields expected gold and exp", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    ActorData* battleActor = DataManager::instance().getActor(1);
    ClassData* battleClass = DataManager::instance().getClass(battleActor ? battleActor->classId : 0);
    REQUIRE(battleActor != nullptr);
    REQUIRE(battleClass != nullptr);
    battleActor->level = 1;
    battleActor->exp = 0;
    battleClass->expTable = {9999, 9999, 9999};
    battleClass->maxLevel = 99;

    BattleManager bm;
    bm.setup(2, true, false); // Troop 2 = one Goblin (20 exp, 10 gold)
    REQUIRE(bm.getEnemiesConst().size() == 1);

    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    bm.applyDamage(enemy, enemy->hp); // kill enemy

    DataManager::instance().getGlobalState().partyMembers.clear();

    REQUIRE(bm.calculateExp() == 20);
    REQUIRE(bm.calculateGold() == 10);

    int32_t initialGold = DataManager::instance().getGold();
    bm.applyGold();
    REQUIRE(DataManager::instance().getGold() == initialGold + 10);

    int32_t initialBattleActorExp = battleActor->exp;
    bm.applyExp();
    REQUIRE(battleActor->exp == initialBattleActorExp);
}

TEST_CASE("BattleManager: drop logic handles probability", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(2, true, false); // Goblin drops: item 1 (rate 1 = 100%), item 2 (rate 2 = 50%)
    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    bm.applyDamage(enemy, enemy->hp); // kill

    // Seed RNG so that drop rolls are deterministic
    bm.seedRng(12345);
    auto drops = bm.calculateDrops();
    // Item 1 should always be present because rate 1
    REQUIRE(std::find(drops.begin(), drops.end(), 1) != drops.end());

    // Re-seed to same value and apply drops; inventory change should match identical drop set
    bm.seedRng(12345);
    int32_t before1 = DataManager::instance().getItemCount(1);
    int32_t before2 = DataManager::instance().getItemCount(2);
    bm.applyDrops();
    REQUIRE(DataManager::instance().getItemCount(1) == before1 + 1);
    bool hasItem2 = std::find(drops.begin(), drops.end(), 2) != drops.end();
    REQUIRE(DataManager::instance().getItemCount(2) == before2 + (hasItem2 ? 1 : 0));
}

TEST_CASE("BattleManager: victory and defeat switches are set correctly", "[battlemgr]") {
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);
    bm.startBattle();
    bm.endBattle(BattleResult::WIN);
    REQUIRE(DataManager::instance().getSwitch(101) == true);

    DataManager::instance().setupNewGame(); // reset switches
    bm.setup(1, true, true);
    bm.startBattle();
    bm.endBattle(BattleResult::DEFEAT);
    REQUIRE(DataManager::instance().getSwitch(102) == true);

    DataManager::instance().setupNewGame();
    bm.setup(1, true, false);
    bm.startBattle();
    bool escaped = bm.processEscape();
    if (escaped) {
        REQUIRE(DataManager::instance().getSwitch(103) == true);
    }
}

TEST_CASE("BattleManager: processAction resolves attack and advances queue", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false); // 2 slimes
    REQUIRE(bm.getEnemiesConst().size() == 2);

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 100;
    actor.mhp = 100;

    bm.queueAction(&actor, BattleActionType::ATTACK, 0);
    BattleAction* action = bm.getNextAction();
    REQUIRE(action != nullptr);
    bm.processAction(action);
    REQUIRE(bm.getEnemy(0)->hp < bm.getEnemy(0)->mhp);
    REQUIRE(bm.getNextAction() == nullptr);
}

TEST_CASE("BattleManager: applySkill deals expected damage and healing", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();
    BattleManager bm;
    bm.setup(1, true, false);

    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    int32_t initialHp = enemy->hp;

    bm.applySkill(nullptr, enemy, 2); // Fire skill (damage.type=1, power=25)
    REQUIRE(enemy->hp < initialHp);

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 10;
    actor.mhp = 100;
    bm.addActorSubject(actor);
    BattleSubject* seededActor = bm.getActor(bm.getActorsConst().size() - 1);
    REQUIRE(seededActor != nullptr);
    bm.applySkill(nullptr, seededActor, 1); // Heal skill (damage.type=3, power=30)
    REQUIRE(seededActor->hp == 40);
}

TEST_CASE("BattleManager: applyItem heals or damages correctly", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();
    BattleManager bm;
    bm.setup(1, true, false);

    BattleSubject actor;
    actor.type = BattleSubjectType::ACTOR;
    actor.index = 0;
    actor.hp = 20;
    actor.mhp = 100;
    bm.addActorSubject(actor);
    BattleSubject* seededActor = bm.getActor(bm.getActorsConst().size() - 1);
    REQUIRE(seededActor != nullptr);

    bm.applyItem(nullptr, seededActor, 1); // Potion (effect code 11, value2=200)
    REQUIRE(seededActor->hp == 100); // capped at max

    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    int32_t initialHp = enemy->hp;
    bm.applyItem(nullptr, enemy, 999); // non-existent item
    REQUIRE(enemy->hp == initialHp);
}

TEST_CASE("BattleManager: applySkill uses the supported compat formula subset when present", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);

    SkillData* skill = DataManager::instance().getSkill(2);
    REQUIRE(skill != nullptr);
    const auto originalDamage = skill->damage;

    skill->damage.type = 1;
    skill->damage.power = 999;
    skill->damage.formula = "a.atk + 5";

    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    const int32_t initialHp = enemy->hp;

    bm.applySkill(nullptr, enemy, 2);

    REQUIRE(enemy->hp == initialHp - 15);
    REQUIRE(BattleManager::getMethodDeviation("applySkill").find("Last formula fallback") == std::string::npos);

    skill->damage = originalDamage;
}

TEST_CASE("BattleManager: applyItem exposes unsupported compat formulas and falls back safely", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    BattleManager bm;
    bm.setup(1, true, false);

    ItemData* item = DataManager::instance().getItem(1);
    REQUIRE(item != nullptr);
    const auto originalDamage = item->damage;
    const auto originalEffects = item->effects;
    const int32_t originalAnimationId = item->animationId;

    item->damage.type = 1;
    item->damage.power = 9;
    item->damage.formula = "a.level * 2";
    item->effects.clear();
    item->animationId = 0;

    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    const int32_t initialHp = enemy->hp;

    bm.applyItem(nullptr, enemy, 1);

    REQUIRE(enemy->hp == initialHp - 9);
    REQUIRE(BattleManager::getMethodDeviation("applyItem").find("Last formula fallback: unsupported_formula_symbol:a.level") != std::string::npos);

    item->damage = originalDamage;
    item->effects = originalEffects;
    item->animationId = originalAnimationId;
}

TEST_CASE("BattleManager: applyExp after battle triggers level-up", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    ActorData* actor = DataManager::instance().getActor(1);
    ClassData* cls = DataManager::instance().getClass(actor->classId);
    REQUIRE(actor != nullptr);
    REQUIRE(cls != nullptr);
    actor->level = 1;
    actor->exp = 0;
    actor->skills.clear();
    cls->expTable = {15};
    cls->skillsToLearn = {{2, 1}};
    cls->maxLevel = 99;

    BattleManager bm;
    bm.setup(2, true, false); // Goblin gives 20 exp
    bm.getEnemy(0)->hp = 0; // kill
    bm.applyExp();

    REQUIRE(actor->level == 2);
    REQUIRE(actor->exp == 5);
    REQUIRE(std::find(actor->skills.begin(), actor->skills.end(), 1) != actor->skills.end());
}

TEST_CASE("BattleManager: switch condition reads live DataManager state", "[battlemgr]") {
    DataManager::instance().setupNewGame();

    BattleManager bm;

    REQUIRE_FALSE(bm.checkSwitchCondition(33));
    DataManager::instance().setSwitch(33, true);
    REQUIRE(bm.checkSwitchCondition(33));
}

TEST_CASE("BattleManager: JS bindings return non-default values", "[battlemgr]") {
    BattleManager::instance().setup(1, true, false);
    BattleManager::instance().startBattle();

    QuickJSContext ctx;
    QuickJSConfig config;
    REQUIRE(ctx.initialize(config));

    BattleManager::registerAPI(ctx);

    auto phaseResult = ctx.callMethod("BattleManager", "getPhase", {});
    REQUIRE(phaseResult.success);
    REQUIRE(std::holds_alternative<int64_t>(phaseResult.value.v));
    REQUIRE(std::get<int64_t>(phaseResult.value.v) == static_cast<int32_t>(BattlePhase::INPUT));

    auto canEscapeResult = ctx.callMethod("BattleManager", "canEscape", {});
    REQUIRE(canEscapeResult.success);
    REQUIRE(std::get<int64_t>(canEscapeResult.value.v) == 1);

    auto canLoseResult = ctx.callMethod("BattleManager", "canLose", {});
    REQUIRE(canLoseResult.success);
    REQUIRE(std::get<int64_t>(canLoseResult.value.v) == 0);

    auto isBattleTestResult = ctx.callMethod("BattleManager", "isBattleTest", {});
    REQUIRE(isBattleTestResult.success);
    REQUIRE(std::get<int64_t>(isBattleTestResult.value.v) == 0);

    auto turnResult = ctx.callMethod("BattleManager", "getTurnCount", {});
    REQUIRE(turnResult.success);
    REQUIRE(std::get<int64_t>(turnResult.value.v) == 0);

    auto checkResult = ctx.callMethod("BattleManager", "checkTurnCondition", {urpg::Value::Int(1), urpg::Value::Int(0)});
    REQUIRE(checkResult.success);
    REQUIRE(std::get<int64_t>(checkResult.value.v) == 0);

    BattleManager::instance().queueActionByIndices(0, BattleSubjectType::ACTOR, BattleActionType::ATTACK, 0, 0, 0);
    auto nextActionResult = ctx.callMethod("BattleManager", "getNextAction", {});
    REQUIRE(nextActionResult.success);
    REQUIRE(std::holds_alternative<urpg::Object>(nextActionResult.value.v));
    auto& obj = std::get<urpg::Object>(nextActionResult.value.v);
    REQUIRE(obj.find("type") != obj.end());
    const auto& typeValue = obj.at("type");
    const auto* typePtr = std::get_if<int64_t>(&typeValue.v);
    REQUIRE(typePtr != nullptr);
    REQUIRE(*typePtr == static_cast<int32_t>(BattleActionType::ATTACK));
}

TEST_CASE("Battle structs and enums", "[battlemgr]") {
    BattleSubject subject;
    REQUIRE(subject.type == BattleSubjectType::ACTOR);
    REQUIRE(subject.pendingAction == BattleActionType::WAIT);

    BattleAction action;
    REQUIRE(action.subject == nullptr);
    REQUIRE(action.type == BattleActionType::ATTACK);
    REQUIRE(action.targetIndex == -1);

    REQUIRE(static_cast<int>(BattlePhase::NONE) == 0);
    REQUIRE(static_cast<int>(BattlePhase::ABORT) == 7);
    REQUIRE(static_cast<int>(BattleResult::WIN) == 1);
    REQUIRE(static_cast<int>(BattleActionType::ITEM) == 3);
}
