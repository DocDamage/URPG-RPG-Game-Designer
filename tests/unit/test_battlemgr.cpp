#include "runtimes/compat_js/battle_manager.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

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

TEST_CASE("BattleManager: method status registry", "[battlemgr]") {
    BattleManager bm;
    (void)bm;

    REQUIRE(BattleManager::getMethodStatus("setup") == CompatStatus::PARTIAL);
    REQUIRE(BattleManager::getMethodStatus("processEscape") == CompatStatus::FULL);
    REQUIRE(BattleManager::getMethodStatus("setBattleTransition") == CompatStatus::STUB);
    REQUIRE(BattleManager::getMethodDeviation("setup").find("troop loading") != std::string::npos);
    REQUIRE(BattleManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
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
