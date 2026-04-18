#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/audio_manager.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

TEST_CASE("BattleManager: setup and flow", "[battlemgr]") {
    DataManager::instance().clearDatabase();
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
    DataManager::instance().clearDatabase();
    BattleManager bm;
    bm.setup(2, false, false);
    bm.startBattle();

    REQUIRE_FALSE(bm.canEscape());
    REQUIRE_FALSE(bm.processEscape());
}

TEST_CASE("BattleManager: escape attempts ramp to deterministic success", "[battlemgr]") {
    DataManager::instance().clearDatabase();
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
    DataManager::instance().clearDatabase();
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
    DataManager::instance().clearDatabase();
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
    DataManager::instance().clearDatabase();
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
    DataManager::instance().clearDatabase();
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
    DataManager::instance().clearDatabase();
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

TEST_CASE("BattleManager: setup populates enemies from DataManager troop", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 50;
    enemy.mmp = 20;
    enemy.atk = 12;
    enemy.def = 8;
    enemy.mat = 10;
    enemy.mdf = 10;
    enemy.agi = 10;
    enemy.luk = 10;
    enemy.exp = 25;
    enemy.gold = 15;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    ActorData& actor = DataManager::instance().addTestActor();
    actor.initialLevel = 1;
    actor.params = {{60, 40, 15, 12, 10, 10, 11, 9}};
    DataManager::instance().getGlobalState().partyMembers.push_back(actor.id);
    DataManager::instance().setupGameActors();

    BattleManager bm;
    bm.setup(troop.id, true, false);

    REQUIRE(bm.getEnemies().size() == 1);
    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);
    REQUIRE(es->type == BattleSubjectType::ENEMY);
    REQUIRE(es->id == enemy.id);
    REQUIRE(es->mhp == 50);
    REQUIRE(es->hp == 50);
    REQUIRE(es->mmp == 20);
    REQUIRE(es->mp == 20);
    REQUIRE(es->atk == 12);
    REQUIRE(es->def == 8);
    REQUIRE(es->mat == 10);
    REQUIRE(es->mdf == 10);
    REQUIRE(es->agi == 10);
    REQUIRE(es->luk == 10);

    REQUIRE(bm.getActors().size() == 1);
    BattleSubject* as = bm.getActor(0);
    REQUIRE(as != nullptr);
    REQUIRE(as->type == BattleSubjectType::ACTOR);
    REQUIRE(as->id == actor.id);
    REQUIRE(as->mhp == 60);
    REQUIRE(as->hp == 60);
    REQUIRE(as->mmp == 40);
    REQUIRE(as->mp == 40);
    REQUIRE(as->atk == 15);
    REQUIRE(as->def == 12);
    REQUIRE(as->mat == 10);
    REQUIRE(as->mdf == 10);
    REQUIRE(as->agi == 11);
    REQUIRE(as->luk == 9);
}

TEST_CASE("BattleManager: rewards apply gold and drops through DataManager", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.gold = 42;
    enemy.exp = 30;
    enemy.dropItems = {1, 100}; // 100% drop rate for item 1

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    ItemData& item = DataManager::instance().addTestItem();
    (void)item;

    BattleManager bm;
    bm.setup(troop.id, true, false);

    REQUIRE(bm.calculateExp() == 30);
    REQUIRE(bm.calculateGold() == 42);

    auto drops = bm.calculateDrops();
    REQUIRE(drops.size() == 1);
    REQUIRE(drops[0] == 1);

    DataManager::instance().setGold(0);
    DataManager::instance().setItemCount(1, 0);

    bm.applyGold();
    REQUIRE(DataManager::instance().getGold() == 42);

    bm.applyDrops();
    REQUIRE(DataManager::instance().getItemCount(1) == 1);

    int hookCalls = 0;
    int receivedExp = 0;
    bm.registerHook(
        BattleManager::HookPoint::ON_VICTORY,
        "test_reward",
        [&hookCalls, &receivedExp](const std::vector<urpg::Value>& args) -> urpg::Value {
            ++hookCalls;
            if (!args.empty() && std::holds_alternative<int64_t>(args[0].v)) {
                receivedExp = static_cast<int>(std::get<int64_t>(args[0].v));
            }
            return urpg::Value::Nil();
        });
    bm.applyExp();
    REQUIRE(hookCalls == 1);
    REQUIRE(receivedExp == 30);
}

TEST_CASE("BattleManager: checkSwitchCondition reads DataManager switches", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    BattleManager bm;
    REQUIRE_FALSE(bm.checkSwitchCondition(1));

    DataManager::instance().setSwitch(1, true);
    REQUIRE(bm.checkSwitchCondition(1));

    DataManager::instance().setSwitch(1, false);
    REQUIRE_FALSE(bm.checkSwitchCondition(1));
}

TEST_CASE("BattleManager: applySkill and applyItem through DataManager", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    SkillData& skill = DataManager::instance().addTestSkill();
    ItemData& item = DataManager::instance().addTestItem();

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);
    REQUIRE(es->hp == 100);

    bm.applySkill(nullptr, es, skill.id);
    REQUIRE(es->hp == 90);

    bm.applyItem(nullptr, es, item.id);
    REQUIRE(es->hp == 100);
}

TEST_CASE("BattleManager: audio setup routes to AudioManager", "[battlemgr]") {
    DataManager::instance().clearDatabase();
    AudioManager::instance().stopBgm();
    BattleManager bm;

    bm.setBattleBgm("Battle1", 80, 100);
    REQUIRE(AudioManager::instance().getCurrentBgm().name == "Battle1");

    bm.setVictoryMe("Victory", 90, 100);
    // ME doesn't have a direct getter; just verify no crash

    bm.setDefeatMe("Defeat", 90, 100);
    // ME doesn't have a direct getter; just verify no crash

    AudioManager::instance().stopBgm();
}

TEST_CASE("BattleManager: setup stores transition and background", "[battlemgr]") {
    DataManager::instance().clearDatabase();
    BattleManager bm;

    bm.setup(1);
    bm.setBattleTransition(2);
    REQUIRE(bm.getBattleTransition() == 2);

    bm.setBattleBackground("Castle");
    REQUIRE(bm.getBattleBackground() == "Castle");
}

TEST_CASE("BattleManager: attack action uses CombatCalc for damage", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;
    enemy.def = 5;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    ActorData& actor = DataManager::instance().addTestActor();
    DataManager::instance().getGlobalState().partyMembers.push_back(actor.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* actorSubject = bm.getActor(0);
    REQUIRE(actorSubject != nullptr);
    actorSubject->atk = 20;

    BattleSubject* enemySubject = bm.getEnemy(0);
    REQUIRE(enemySubject != nullptr);
    REQUIRE(enemySubject->hp == 100);

    bm.queueAction(actorSubject, BattleActionType::ATTACK, 0);
    BattleAction* action = bm.getNextAction();
    REQUIRE(action != nullptr);
    bm.processAction(action);

    REQUIRE(enemySubject->hp < 100);
    int damage1 = 100 - enemySubject->hp;
    REQUIRE(damage1 > 0);

    // Verify deterministic: same setup -> same damage
    BattleManager bm2;
    bm2.setup(troop.id, true, false);
    BattleSubject* actor2 = bm2.getActor(0);
    actor2->atk = 20;
    BattleSubject* enemy2 = bm2.getEnemy(0);
    REQUIRE(enemy2->hp == 100);

    bm2.queueAction(actor2, BattleActionType::ATTACK, 0);
    BattleAction* action2 = bm2.getNextAction();
    REQUIRE(action2 != nullptr);
    bm2.processAction(action2);

    int damage2 = 100 - enemy2->hp;
    REQUIRE(damage1 == damage2);
}

TEST_CASE("BattleManager: applyStateEffects deals tick damage", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    StateData& state = DataManager::instance().addTestState();
    state.slipDamage = 2;

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 50;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);
    REQUIRE(es->hp == 50);

    es->addState(state.id, 3);
    bm.applyStateEffects(es);
    REQUIRE(es->hp == 48);

    bm.applyStateEffects(nullptr);
    // Test passes if no crash
}

TEST_CASE("BattleManager: updateBattleEvents ends active events", "[battlemgr]") {
    DataManager::instance().clearDatabase();
    BattleManager bm;
    bm.setup(1);

    bm.startBattleEvent(1);
    REQUIRE(bm.isBattleEventActive());

    bm.updateBattleEvents();
    bm.updateBattleEvents();
    bm.updateBattleEvents();
    REQUIRE_FALSE(bm.isBattleEventActive());
}

TEST_CASE("BattleManager: playAnimation records request", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);

    bm.playAnimation(1, es);
    REQUIRE(bm.getLastAnimationRequest().animationId == 1);
    REQUIRE(bm.getLastAnimationRequest().targetIndex == es->index);
    REQUIRE(bm.getLastAnimationRequest().targetType == static_cast<int32_t>(BattleSubjectType::ENEMY));

    bm.clearLastAnimationRequest();
    REQUIRE(bm.getLastAnimationRequest().animationId == 0);
    REQUIRE(bm.getLastAnimationRequest().targetIndex == 0);
}

TEST_CASE("BattleManager: updateBattleEvents ticks to completion", "[battlemgr]") {
    DataManager::instance().clearDatabase();
    BattleManager bm;
    bm.setup(1);

    bm.startBattleEvent(1);
    REQUIRE(bm.isBattleEventActive());

    bm.updateBattleEvents();
    REQUIRE(bm.isBattleEventActive());

    bm.updateBattleEvents();
    REQUIRE(bm.isBattleEventActive());

    bm.updateBattleEvents();
    REQUIRE_FALSE(bm.isBattleEventActive());
}

TEST_CASE("BattleSubject: state management", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    BattleSubject subject;
    REQUIRE_FALSE(subject.hasState(1));

    subject.addState(1, 5);
    REQUIRE(subject.hasState(1));
    REQUIRE(subject.getStateTurns(1) == 5);

    subject.addState(2, 3);
    REQUIRE(subject.hasState(2));
    REQUIRE(subject.getStateTurns(2) == 3);

    subject.removeState(1);
    REQUIRE_FALSE(subject.hasState(1));
    REQUIRE(subject.getStateTurns(1) == 0);
    REQUIRE(subject.hasState(2));

    subject.clearStates();
    REQUIRE_FALSE(subject.hasState(2));
    REQUIRE(subject.states.empty());
    REQUIRE(subject.stateTurns.empty());

    // Negative stateId is ignored
    subject.addState(-1, 5);
    REQUIRE(subject.states.empty());

    // Duplicate add preserves turns if not specified
    subject.addState(3, 5);
    subject.addState(3);
    REQUIRE(subject.states.size() == 1);
    REQUIRE(subject.getStateTurns(3) == 5);
}

TEST_CASE("BattleManager: applyStateEffects with slipDamage", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    StateData& state = DataManager::instance().addTestState();
    state.slipDamage = 5;

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);
    REQUIRE(es->hp == 100);

    es->addState(state.id, 3);
    bm.applyStateEffects(es);
    REQUIRE(es->hp == 95);
}

TEST_CASE("BattleManager: applyStateEffects with no active states", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);
    REQUIRE(es->hp == 100);

    bm.applyStateEffects(es);
    REQUIRE(es->hp == 100);
}

TEST_CASE("BattleManager: applyStateEffects decrements auto-removal turns", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    StateData& state = DataManager::instance().addTestState();
    state.autoRemovalTiming = 1; // end of turn
    state.minTurns = 1;
    state.maxTurns = 3;

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);

    es->addState(state.id, 3);
    REQUIRE(es->getStateTurns(state.id) == 3);

    bm.applyStateEffects(es);
    REQUIRE(es->getStateTurns(state.id) == 2);

    bm.applyStateEffects(es);
    REQUIRE(es->getStateTurns(state.id) == 1);
}

TEST_CASE("BattleManager: removeExpiredStates removes expired states", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    StateData& state = DataManager::instance().addTestState();
    state.autoRemovalTiming = 1;
    state.minTurns = 1;
    state.maxTurns = 1;

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);

    es->addState(state.id, 1);
    REQUIRE(es->hasState(state.id));

    bm.applyStateEffects(es);
    REQUIRE(es->getStateTurns(state.id) == 0);
    REQUIRE(es->hasState(state.id));

    bm.removeExpiredStates(es);
    REQUIRE_FALSE(es->hasState(state.id));
}

TEST_CASE("BattleManager: applyDamage may remove removeByDamage states", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    StateData& state = DataManager::instance().addTestState();
    state.removeByDamage = true;
    state.chanceByDamage = 100; // always remove

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);

    es->addState(state.id, 3);
    REQUIRE(es->hasState(state.id));

    bm.applyDamage(es, 10);
    REQUIRE_FALSE(es->hasState(state.id));
    REQUIRE(es->hp == 90);
}

TEST_CASE("BattleManager: applyDamage keeps removeByDamage state on failed roll", "[battlemgr]") {
    DataManager::instance().clearDatabase();

    StateData& state = DataManager::instance().addTestState();
    state.removeByDamage = true;
    state.chanceByDamage = 0; // never remove

    EnemyData& enemy = DataManager::instance().addTestEnemy();
    enemy.mhp = 100;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members.push_back(enemy.id);

    BattleManager bm;
    bm.setup(troop.id, true, false);

    BattleSubject* es = bm.getEnemy(0);
    REQUIRE(es != nullptr);

    es->addState(state.id, 3);
    REQUIRE(es->hasState(state.id));

    bm.applyDamage(es, 10);
    REQUIRE(es->hasState(state.id));
    REQUIRE(es->hp == 90);
}
