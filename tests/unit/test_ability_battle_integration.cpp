#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "engine/core/ability/ability_compat_mapper.h"
#include "engine/core/ability/ability_battle_integration.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/battle/battle_core.h"
#include <nlohmann/json.hpp>
#include <string>

using namespace urpg;
using namespace urpg::ability;
using json = nlohmann::json;
using Catch::Approx;

// ---------------------------------------------------------------------------
// Minimal concrete ability for battle integration tests
// ---------------------------------------------------------------------------

class SimpleTestAbility : public GameplayAbility {
public:
    explicit SimpleTestAbility(const std::string& id_, float cooldown = 2.0f, int32_t mp = 5)
        : id_(id_), cooldown_(cooldown), mp_(mp) {}

    const std::string& getId() const override { return id_; }

    const ActivationInfo& getActivationInfo() const override {
        info_.cooldownSeconds = cooldown_;
        info_.mpCost = mp_;
        return info_;
    }

    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);
    }

private:
    std::string id_;
    float cooldown_;
    int32_t mp_;
    mutable ActivationInfo info_;
};

// ---------------------------------------------------------------------------
// S24-T01: Ability schema field coverage
// ---------------------------------------------------------------------------

TEST_CASE("AuthoredAbilityAsset serializes all schema-required fields", "[ability][schema][s24]") {
    AuthoredAbilityAsset asset;
    asset.ability_id = "skill.fire_bolt";
    asset.cooldown_seconds = 3.0f;
    asset.mp_cost = 10.0f;
    asset.effect_id = "effect.fire_bolt";
    asset.effect_attribute = "HP";
    asset.effect_operation = ModifierOp::Add;
    asset.effect_value = -25.0f;
    asset.effect_duration = 0.0f;

    json j;
    to_json(j, asset);

    REQUIRE(j.contains("ability_id"));
    REQUIRE(j.contains("cooldown_seconds"));
    REQUIRE(j.contains("mp_cost"));
    REQUIRE(j.contains("effect_id"));
    REQUIRE(j.contains("effect_attribute"));
    REQUIRE(j.contains("effect_operation"));
    REQUIRE(j.contains("effect_value"));
    REQUIRE(j.contains("effect_duration"));
    REQUIRE(j.contains("pattern"));
    REQUIRE(j["ability_id"] == "skill.fire_bolt");
    REQUIRE(j["effect_operation"] == "Add");
}

TEST_CASE("AuthoredAbilityAsset round-trips via JSON deserialization", "[ability][schema][s24]") {
    AuthoredAbilityAsset original;
    original.ability_id = "skill.ice_lance";
    original.cooldown_seconds = 5.0f;
    original.mp_cost = 20.0f;
    original.effect_id = "effect.ice";
    original.effect_attribute = "Defense";
    original.effect_operation = ModifierOp::Multiply;
    original.effect_value = 0.8f;
    original.effect_duration = 3.0f;

    json j;
    to_json(j, original);

    AuthoredAbilityAsset restored;
    from_json(j, restored);

    REQUIRE(restored.ability_id == original.ability_id);
    REQUIRE(restored.cooldown_seconds == Approx(original.cooldown_seconds));
    REQUIRE(restored.mp_cost == Approx(original.mp_cost));
    REQUIRE(restored.effect_id == original.effect_id);
    REQUIRE(restored.effect_attribute == original.effect_attribute);
    REQUIRE(restored.effect_operation == original.effect_operation);
    REQUIRE(restored.effect_value == Approx(original.effect_value));
    REQUIRE(restored.effect_duration == Approx(original.effect_duration));
}

// ---------------------------------------------------------------------------
// S24-T01/T02: Compat mapper — MZ skill → native ability
// ---------------------------------------------------------------------------

TEST_CASE("mapMzSkillToNativeAbility maps id and mpCost fields", "[ability][compat][s24]") {
    json mz_skill = {
        {"id", 7},
        {"name", "Fire Bolt"},
        {"mpCost", 12},
    };

    const auto result = mapMzSkillToNativeAbility(mz_skill);

    REQUIRE(result.asset.ability_id == "skill.7");
    REQUIRE(result.asset.mp_cost == Approx(12.0f));
    REQUIRE(result.mapped_json.contains("ability_id"));
    REQUIRE(result.mapped_json["ability_id"] == "skill.7");
}

TEST_CASE("mapMzSkillToNativeAbility falls back to name-slug when id is absent", "[ability][compat][s24]") {
    json mz_skill = {
        {"name", "Ice Lance"},
        {"mpCost", 8},
    };

    const auto result = mapMzSkillToNativeAbility(mz_skill);

    REQUIRE(result.asset.ability_id == "skill.ice_lance");
}

TEST_CASE("mapMzSkillToNativeAbility preserves damage formula in unsupported_fields", "[ability][compat][s24]") {
    json mz_skill = {
        {"id", 3},
        {"name", "Slam"},
        {"mpCost", 5},
        {"damage", {
            {"formula", "a.atk * 4 - b.def * 2"},
            {"type", 1},
            {"elementId", 2}
        }},
    };

    const auto result = mapMzSkillToNativeAbility(mz_skill);

    REQUIRE_FALSE(result.all_mapped);
    REQUIRE_FALSE(result.fallback_field_names.empty());
    REQUIRE(result.mapped_json.contains("unsupported_fields"));
    const auto& uf = result.mapped_json["unsupported_fields"];
    REQUIRE(uf.contains("damage_formula"));
    REQUIRE(uf["damage_formula"] == "a.atk * 4 - b.def * 2");
    REQUIRE(uf.contains("damage_type"));
    REQUIRE(uf.contains("damage_element"));
    // Nominal fallback value applied for un-evaluatable formula
    REQUIRE(result.asset.effect_value == Approx(0.0f));
}

TEST_CASE("mapMzSkillToNativeAbility preserves all known unsupported MZ fields verbatim", "[ability][compat][s24]") {
    json mz_skill = {
        {"id", 10},
        {"name", "Heal"},
        {"mpCost", 6},
        {"tpCost", 0},
        {"occasion", 1},
        {"scope", 7},
        {"speed", 0},
        {"successRate", 100},
        {"repeats", 1},
        {"hitType", 0},
        {"animationId", 44},
        {"effects", json::array()},
    };

    const auto result = mapMzSkillToNativeAbility(mz_skill);

    REQUIRE(result.mapped_json.contains("unsupported_fields"));
    const auto& uf = result.mapped_json["unsupported_fields"];
    REQUIRE(uf.contains("tpCost"));
    REQUIRE(uf.contains("occasion"));
    REQUIRE(uf.contains("scope"));
    REQUIRE(uf.contains("speed"));
    REQUIRE(uf.contains("successRate"));
    REQUIRE(uf.contains("repeats"));
    REQUIRE(uf.contains("hitType"));
    REQUIRE(uf.contains("animationId"));
    REQUIRE(uf.contains("effects"));
}

TEST_CASE("mapMzSkillToNativeAbility result has schema field set", "[ability][compat][s24]") {
    json mz_skill = {{"id", 1}, {"name", "Attack"}, {"mpCost", 0}};
    const auto result = mapMzSkillToNativeAbility(mz_skill);
    REQUIRE(result.mapped_json.contains("schema"));
    REQUIRE(result.mapped_json["schema"] == "content/schemas/gameplay_ability.schema.json");
    REQUIRE(result.mapped_json["schemaVersion"] == 1);
}

TEST_CASE("mapMzSkillArrayToNativeAbilities skips null entries", "[ability][compat][s24]") {
    // MZ uses 1-based arrays: index 0 is always null
    json mz_array = {
        nullptr,
        {{"id", 1}, {"name", "Attack"}, {"mpCost", 0}},
        {{"id", 2}, {"name", "Guard"}, {"mpCost", 0}},
    };

    const auto results = mapMzSkillArrayToNativeAbilities(mz_array);

    REQUIRE(results.size() == 2);
    REQUIRE(results[0].asset.ability_id == "skill.1");
    REQUIRE(results[1].asset.ability_id == "skill.2");
}

TEST_CASE("mapMzSkillToNativeAbility handles empty skill object without crash", "[ability][compat][s24]") {
    json mz_skill = json::object();
    const auto result = mapMzSkillToNativeAbility(mz_skill);
    // Default ability_id should be used
    REQUIRE(!result.asset.ability_id.empty());
    REQUIRE(result.mapped_json.contains("ability_id"));
}

// ---------------------------------------------------------------------------
// S24-T03/T04: AbilityBattleQueue — command ordering and execution
// ---------------------------------------------------------------------------

TEST_CASE("AbilityBattleQueue starts empty", "[ability][battle][s24]") {
    AbilityBattleQueue queue;
    REQUIRE(queue.empty());
    REQUIRE(queue.size() == 0);
}

TEST_CASE("AbilityBattleQueue flush returns empty snapshot when queue is empty", "[ability][battle][s24]") {
    AbilityBattleQueue queue;
    const auto snap = queue.flush({}, 1);
    REQUIRE(snap.commands_received == 0);
    REQUIRE(snap.commands_executed == 0);
    REQUIRE(snap.commands_blocked == 0);
    REQUIRE(snap.outcomes.empty());
}

TEST_CASE("AbilityBattleQueue produces error outcome for unknown subject", "[ability][battle][s24]") {
    AbilityBattleQueue queue;
    queue.enqueue({"unknown_subject", {}, "skill.fire", 0, 0});

    const auto snap = queue.flush({}, 1);

    REQUIRE(snap.commands_received == 1);
    REQUIRE(snap.commands_blocked == 1);
    REQUIRE(snap.commands_executed == 0);
    REQUIRE(snap.outcomes[0].result == "error");
    REQUIRE(snap.outcomes[0].reason == "subject_not_found");
}

TEST_CASE("AbilityBattleQueue produces not_found outcome for unregistered ability", "[ability][battle][s24]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 100.0f);

    AbilityBattleQueue queue;
    queue.enqueue({"hero", {}, "skill.nonexistent", 0, 0});

    const auto snap = queue.flush({{"hero", &asc}}, 1);

    REQUIRE(snap.commands_blocked == 1);
    REQUIRE(snap.outcomes[0].result == "not_found");
    REQUIRE(snap.outcomes[0].reason == "ability_not_registered");
}

TEST_CASE("AbilityBattleQueue activates a registered ability and records outcome", "[ability][battle][s24]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 100.0f);
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.fire_bolt", 2.0f, 10));

    AbilityBattleQueue queue;
    queue.enqueue({"hero", {}, "skill.fire_bolt", 0, 0});

    const auto snap = queue.flush({{"hero", &asc}}, 1);

    REQUIRE(snap.commands_executed == 1);
    REQUIRE(snap.commands_blocked == 0);
    REQUIRE(snap.outcomes[0].result == "activated");
    REQUIRE(snap.outcomes[0].ability_id == "skill.fire_bolt");
}

TEST_CASE("AbilityBattleQueue blocks ability on cooldown", "[ability][battle][s24]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 100.0f);
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.slam", 5.0f, 10));

    AbilityBattleQueue queue;
    // First activation succeeds and starts cooldown
    queue.enqueue({"hero", {}, "skill.slam", 0, 0});
    auto snap1 = queue.flush({{"hero", &asc}}, 1);
    REQUIRE(snap1.commands_executed == 1);

    // Second activation on same turn should fail (cooldown set)
    queue.enqueue({"hero", {}, "skill.slam", 0, 0});
    auto snap2 = queue.flush({{"hero", &asc}}, 1);
    REQUIRE(snap2.commands_blocked == 1);
    REQUIRE(snap2.outcomes[0].result == "blocked");
}

TEST_CASE("AbilityBattleQueue executes commands ordered by priority DESC then speed DESC", "[ability][battle][s24]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 500.0f);
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.a", 0.0f, 1));
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.b", 0.0f, 1));
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.c", 0.0f, 1));

    AbilityBattleQueue queue;
    // Enqueue in reverse priority order
    queue.enqueue({"hero", {}, "skill.c", /*speed=*/5,  /*priority=*/1});
    queue.enqueue({"hero", {}, "skill.b", /*speed=*/10, /*priority=*/2});
    queue.enqueue({"hero", {}, "skill.a", /*speed=*/3,  /*priority=*/3});

    const auto snap = queue.flush({{"hero", &asc}}, 1);

    REQUIRE(snap.commands_received == 3);
    // Outcomes should be ordered: a (priority 3) → b (priority 2) → c (priority 1)
    REQUIRE(snap.outcomes[0].ability_id == "skill.a");
    REQUIRE(snap.outcomes[1].ability_id == "skill.b");
    REQUIRE(snap.outcomes[2].ability_id == "skill.c");
}

TEST_CASE("AbilityBattleQueue diagnostics snapshot serializes to JSON", "[ability][battle][s24]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 100.0f);
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.fire_bolt", 2.0f, 10));

    AbilityBattleQueue queue;
    queue.enqueue({"hero", {}, "skill.fire_bolt", 0, 0});
    const auto snap = queue.flush({{"hero", &asc}}, 3);

    const auto j = snap.toJson();
    REQUIRE(j.contains("turn_number"));
    REQUIRE(j["turn_number"] == 3);
    REQUIRE(j.contains("commands_received"));
    REQUIRE(j.contains("commands_executed"));
    REQUIRE(j.contains("commands_blocked"));
    REQUIRE(j.contains("outcomes"));
    REQUIRE(j["outcomes"].is_array());
    REQUIRE(!j["outcomes"].empty());

    const auto& o = j["outcomes"][0];
    REQUIRE(o.contains("subject_id"));
    REQUIRE(o.contains("ability_id"));
    REQUIRE(o.contains("result"));
    REQUIRE(o["result"] == "activated");
}

TEST_CASE("AbilityBattleQueue is deterministic across independent flush calls", "[ability][battle][s24]") {
    // Same input → same outcome on two separate queues
    auto make_asc = []() {
        auto asc = std::make_shared<AbilitySystemComponent>();
        asc->setAttribute("MP", 100.0f);
        asc->grantAbility(std::make_shared<SimpleTestAbility>("skill.fire_bolt", 2.0f, 10));
        return asc;
    };

    auto asc1 = make_asc();
    auto asc2 = make_asc();

    AbilityBattleQueue q1, q2;
    q1.enqueue({"hero", {}, "skill.fire_bolt", 0, 0});
    q2.enqueue({"hero", {}, "skill.fire_bolt", 0, 0});

    const auto snap1 = q1.flush({{"hero", asc1.get()}}, 1);
    const auto snap2 = q2.flush({{"hero", asc2.get()}}, 1);

    REQUIRE(snap1.commands_executed == snap2.commands_executed);
    REQUIRE(snap1.commands_blocked == snap2.commands_blocked);
    REQUIRE(snap1.outcomes[0].result == snap2.outcomes[0].result);
    REQUIRE(snap1.outcomes[0].ability_id == snap2.outcomes[0].ability_id);
}

TEST_CASE("AbilityBattleQueue records mp_before and mp_after in outcome", "[ability][battle][s24]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 50.0f);
    asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.drain", 2.0f, 15));

    AbilityBattleQueue queue;
    queue.enqueue({"hero", {}, "skill.drain", 0, 0});
    const auto snap = queue.flush({{"hero", &asc}}, 1);

    REQUIRE(snap.outcomes[0].result == "activated");
    REQUIRE(snap.outcomes[0].mp_before == Approx(50.0f));
    // After activation mp_cost is deducted
    REQUIRE(snap.outcomes[0].mp_after < snap.outcomes[0].mp_before);
}

// ---------------------------------------------------------------------------
// S24-T04: Diagnostics — BattleFlowController + ability queue integration
// ---------------------------------------------------------------------------

TEST_CASE("BattleFlowController advances through phases with ability commands queued", "[ability][battle][integration][s24]") {
    urpg::battle::BattleFlowController flow;
    AbilitySystemComponent hero_asc;
    hero_asc.setAttribute("MP", 100.0f);
    hero_asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.strike", 2.0f, 5));

    AbilityBattleQueue queue;

    // Phase: Start → Input
    flow.beginBattle(true);
    REQUIRE(flow.phase() == urpg::battle::BattleFlowPhase::Start);
    flow.enterInput();
    REQUIRE(flow.phase() == urpg::battle::BattleFlowPhase::Input);

    // During Input phase: queue an ability command
    queue.enqueue({"hero", {"enemy"}, "skill.strike", 10, 1});

    // Transition to Action phase and flush queue
    flow.enterAction();
    REQUIRE(flow.phase() == urpg::battle::BattleFlowPhase::Action);

    const auto snap = queue.flush({{"hero", &hero_asc}}, flow.turnCount());
    REQUIRE(snap.commands_executed == 1);
    REQUIRE(snap.outcomes[0].result == "activated");

    // End turn
    flow.endTurn();
    REQUIRE(flow.phase() == urpg::battle::BattleFlowPhase::TurnEnd);
    REQUIRE(flow.turnCount() == 2);
}

TEST_CASE("AbilityBattleQueue diagnostics JSON is stable across identical battle turns", "[ability][battle][integration][s24]") {
    // Verify diagnostics output is bit-stable for the same turn/command set
    auto run_turn = [&]() {
        AbilitySystemComponent asc;
        asc.setAttribute("MP", 100.0f);
        asc.grantAbility(std::make_shared<SimpleTestAbility>("skill.fire_bolt", 0.0f, 5));

        AbilityBattleQueue queue;
        queue.enqueue({"hero", {}, "skill.fire_bolt", 5, 0});

        return queue.flush({{"hero", &asc}}, 1).toJson().dump();
    };

    const std::string run1 = run_turn();
    const std::string run2 = run_turn();
    REQUIRE(run1 == run2);
}
