#include "engine/core/battle/battle_core.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::battle;

TEST_CASE("BattleFlowController transitions deterministically across phases", "[battle][core][flow]") {
    BattleFlowController flow;
    REQUIRE(flow.phase() == BattleFlowPhase::None);
    REQUIRE_FALSE(flow.isActive());

    flow.beginBattle(true);
    REQUIRE(flow.phase() == BattleFlowPhase::Start);
    REQUIRE(flow.isActive());
    REQUIRE(flow.canEscape());
    REQUIRE(flow.turnCount() == 1);
    REQUIRE(flow.escapeFailures() == 0);

    flow.enterInput();
    REQUIRE(flow.phase() == BattleFlowPhase::Input);

    flow.enterAction();
    REQUIRE(flow.phase() == BattleFlowPhase::Action);

    flow.noteEscapeFailure();
    flow.noteEscapeFailure();
    REQUIRE(flow.escapeFailures() == 2);

    flow.endTurn();
    REQUIRE(flow.phase() == BattleFlowPhase::TurnEnd);
    REQUIRE(flow.turnCount() == 2);

    flow.markVictory();
    REQUIRE(flow.phase() == BattleFlowPhase::Victory);
    REQUIRE_FALSE(flow.isActive());
    REQUIRE_FALSE(flow.canEscape());

    flow.enterInput();
    REQUIRE(flow.phase() == BattleFlowPhase::Victory);
}

TEST_CASE("BattleActionQueue sorts by speed then priority then deterministic IDs", "[battle][core][queue]") {
    BattleActionQueue queue;
    queue.enqueue({"actor_b", "enemy_0", "attack", 95, 1});
    queue.enqueue({"actor_a", "enemy_0", "attack", 120, 1});
    queue.enqueue({"actor_c", "enemy_0", "attack", 120, 0});
    queue.enqueue({"actor_c", "enemy_1", "attack", 120, 0});

    const auto ordered = queue.snapshotOrdered();
    REQUIRE(ordered.size() == 4);
    REQUIRE(ordered[0].subject_id == "actor_c");
    REQUIRE(ordered[0].target_id == "enemy_0");
    REQUIRE(ordered[1].subject_id == "actor_c");
    REQUIRE(ordered[1].target_id == "enemy_1");
    REQUIRE(ordered[2].subject_id == "actor_a");
    REQUIRE(ordered[3].subject_id == "actor_b");

    auto first = queue.popNext();
    REQUIRE(first.has_value());
    REQUIRE(first->subject_id == "actor_c");
    REQUIRE(first->target_id == "enemy_0");
    REQUIRE(queue.size() == 3);

    queue.clear();
    REQUIRE(queue.empty());
    REQUIRE_FALSE(queue.popNext().has_value());
}

TEST_CASE("BattleRuleResolver resolves damage with guard, crit, and variance rules", "[battle][core][rules]") {
    BattleDamageContext physical;
    physical.subject.atk = 20;
    physical.target.def = 8;
    physical.target.hp = 200;
    physical.power = 10;
    physical.variance_percent = 20;

    const int32_t normal = BattleRuleResolver::resolveDamage(physical);
    REQUIRE(normal > 0);

    BattleDamageContext guarded = physical;
    guarded.target.guarding = true;
    const int32_t reduced = BattleRuleResolver::resolveDamage(guarded);
    REQUIRE(reduced < normal);

    BattleDamageContext critical = physical;
    critical.critical = true;
    const int32_t boosted = BattleRuleResolver::resolveDamage(critical);
    REQUIRE(boosted > normal);

    BattleDamageContext magical;
    magical.subject.mat = 30;
    magical.target.mdf = 5;
    magical.target.hp = 120;
    magical.power = 12;
    magical.magical = true;
    const int32_t magical_damage = BattleRuleResolver::resolveDamage(magical);
    REQUIRE(magical_damage > 0);
}

TEST_CASE("BattleRuleResolver resolves guarded scene attacks without exceeding current HP", "[battle][core][rules]") {
    BattleDamageContext context;
    context.subject.atk = 12;
    context.target.hp = 20;
    context.target.def = 4;
    context.target.guarding = true;
    context.power = 6;

    const int32_t guarded = BattleRuleResolver::resolveDamage(context);
    context.target.guarding = false;
    const int32_t normal = BattleRuleResolver::resolveDamage(context);

    REQUIRE(guarded > 0);
    REQUIRE(normal > guarded);
    REQUIRE(normal <= 20);
}

TEST_CASE("BattleRuleResolver escape ratio ramps with failed attempts", "[battle][core][rules]") {
    const int32_t base = BattleRuleResolver::resolveEscapeRatio(80, 100, 0);
    const int32_t ramp1 = BattleRuleResolver::resolveEscapeRatio(80, 100, 1);
    const int32_t ramp3 = BattleRuleResolver::resolveEscapeRatio(80, 100, 3);
    REQUIRE(base < ramp1);
    REQUIRE(ramp1 < ramp3);
    REQUIRE(ramp3 <= 100);
}
