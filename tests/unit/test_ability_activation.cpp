#include <catch2/catch_test_macros.hpp>
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/ability_state_machine.h"
#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/gameplay_effect.h"

using namespace urpg::ability;

class MockAbility : public GameplayAbility {
public:
    MockAbility(std::string id) : m_id(std::move(id)) {}

    const std::string& getId() const override { return m_id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }
    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);
    }

    ActivationInfo& editInfo() { return m_info; }

private:
    std::string m_id;
    ActivationInfo m_info;
};

class EffectAbility : public GameplayAbility {
public:
    EffectAbility() {
        id = "skill.effect_burst";
        cooldownTime = 2.0f;
        mpCost = 5.0f;
        m_effect.id = "BUFF_SPEED";
        m_effect.duration = 4.0f;
        urpg::GameplayEffectModifier modifier;
        modifier.attributeName = "Speed";
        modifier.operation = urpg::ModifierOp::Add;
        modifier.value = 10.0f;
        m_effect.modifiers.push_back(modifier);
    }

    const std::string& getId() const override { return id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }

    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);
        source.applyEffect(m_effect);
    }

private:
    ActivationInfo m_info;
    urpg::GameplayEffect m_effect;
};

TEST_CASE("GameplayAbility: Activation Pipeline", "[ability][activation]") {
    AbilitySystemComponent owner;
    MockAbility fireAbility("skill.fireball");

    SECTION("Ability activation fails if owner is Stunned") {
        fireAbility.editInfo().blockingTags.addTag(GameplayTag("State.Stunned"));
        
        owner.addTag(GameplayTag("State.Stunned"));
        REQUIRE_FALSE(fireAbility.canActivate(owner));

        owner.removeTag(GameplayTag("State.Stunned"));
        REQUIRE(fireAbility.canActivate(owner));
    }

    SECTION("Ability activation fails if cooldown is active") {
        owner.setCooldown("skill.fireball", 5.0f);
        REQUIRE_FALSE(fireAbility.canActivate(owner));

        owner.update(6.0f);
        REQUIRE(fireAbility.canActivate(owner));
    }

    SECTION("Ability activation requires specific tags") {
        fireAbility.editInfo().requiredTags.addTag(GameplayTag("State.Concentrated"));
        
        REQUIRE_FALSE(fireAbility.canActivate(owner));

        owner.addTag(GameplayTag("State.Concentrated"));
        REQUIRE(fireAbility.canActivate(owner));
    }

    SECTION("Replay-safe execution history records blocked and successful activations deterministically") {
        owner.setAttribute("MP", 15.0f);
        fireAbility.mpCost = 5.0f;
        fireAbility.cooldownTime = 3.0f;
        fireAbility.editInfo().blockingTags.addTag(GameplayTag("State.Silenced"));

        owner.addTag(GameplayTag("State.Silenced"));
        REQUIRE_FALSE(owner.tryActivateAbility(fireAbility));

        owner.removeTag(GameplayTag("State.Silenced"));
        REQUIRE(owner.tryActivateAbility(fireAbility));
        REQUIRE_FALSE(owner.tryActivateAbility(fireAbility));

        const auto& history = owner.getAbilityExecutionHistory();
        REQUIRE(history.size() == 3);
        REQUIRE(history[0].sequence_id == 1);
        REQUIRE(history[0].outcome == "blocked");
        REQUIRE(history[0].reason == "blocking_tags_present");
        REQUIRE(history[1].sequence_id == 2);
        REQUIRE(history[1].outcome == "executed");
        REQUIRE(history[1].mp_before == 15.0f);
        REQUIRE(history[1].mp_after == 10.0f);
        REQUIRE(history[1].cooldown_after == 3.0f);
        REQUIRE(history[2].sequence_id == 3);
        REQUIRE(history[2].outcome == "blocked");
        REQUIRE(history[2].reason == "cooldown_active");
    }

    SECTION("Ability execution history captures deterministic effect application and panel diagnostics") {
        AbilitySystemComponent effectOwner;
        effectOwner.setAttribute("MP", 12.0f);
        EffectAbility effectAbility;

        REQUIRE(effectOwner.tryActivateAbility(effectAbility));
        REQUIRE(effectOwner.getAttribute("Speed", 100.0f) == 110.0f);

        urpg::editor::AbilityInspectorPanel panel;
        panel.update(effectOwner);

        const auto& history = effectOwner.getAbilityExecutionHistory();
        REQUIRE(history.size() == 1);
        REQUIRE(history[0].active_effect_count == 1);

        const auto& snapshot = panel.getRenderSnapshot();
        REQUIRE(snapshot.diagnostic_count == 1);
        REQUIRE(snapshot.latest_ability_id == "skill.effect_burst");
        REQUIRE(snapshot.latest_outcome == "executed");
        REQUIRE(snapshot.diagnostic_lines[0].find("skill.effect_burst") != std::string::npos);
        REQUIRE(snapshot.diagnostic_lines[0].find("effects=1") != std::string::npos);
    }
}

TEST_CASE("AbilityStateMachine records replay-safe deterministic state transitions", "[ability][state_machine]") {
    AbilitySystemComponent asc;
    urpg::AbilityStateMachine machine("skill.combo_strike");

    urpg::AbilityState windup;
    windup.name = "windup";
    windup.onTick = [](AbilitySystemComponent&, float) { return true; };

    urpg::AbilityState impact;
    impact.name = "impact";
    impact.onTick = [](AbilitySystemComponent&, float) { return true; };

    machine.addState(windup);
    machine.addState(impact);

    machine.start(asc);
    machine.update(asc, 0.016f);
    machine.update(asc, 0.016f);

    const auto& history = asc.getAbilityExecutionHistory();
    REQUIRE(history.size() == 3);
    REQUIRE(history[0].sequence_id == 1);
    REQUIRE(history[0].stage == "state_machine");
    REQUIRE(history[0].outcome == "entered");
    REQUIRE(history[0].state_name == "windup");
    REQUIRE(history[1].sequence_id == 2);
    REQUIRE(history[1].outcome == "entered");
    REQUIRE(history[1].state_name == "impact");
    REQUIRE(history[2].sequence_id == 3);
    REQUIRE(history[2].outcome == "finished");
    REQUIRE(history[2].state_name == "impact");
}
