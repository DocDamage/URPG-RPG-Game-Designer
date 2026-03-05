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
}

TEST_CASE("BattleManager: method status registry", "[battlemgr]") {
    BattleManager bm;
    (void)bm;

    REQUIRE(BattleManager::getMethodStatus("setup") == CompatStatus::FULL);
    REQUIRE(BattleManager::getMethodStatus("processEscape") == CompatStatus::PARTIAL);
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
