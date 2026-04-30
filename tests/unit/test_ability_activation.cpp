#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/ability_state_machine.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/gameplay_effect.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>

using namespace urpg::ability;

class MockAbility : public GameplayAbility {
  public:
    MockAbility(std::string id) : m_id(std::move(id)) {}

    const std::string& getId() const override { return m_id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }
    void activate(AbilitySystemComponent& source) override { commitAbility(source); }

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

class ContextAwareAbility : public GameplayAbility {
  public:
    ContextAwareAbility() {
        id = "skill.context_mark";
        mpCost = 2.0f;
    }

    const std::string& getId() const override { return id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }

    void activate(AbilitySystemComponent& source) override { commitAbility(source); }

    void activate(AbilitySystemComponent& source, const AbilityExecutionContext& context) override {
        commitAbility(source);
        activated_with_target = !context.targets.empty();
        if (!context.targets.empty() && context.targets.front().abilitySystem != nullptr) {
            context.targets.front().abilitySystem->addTag(GameplayTag("State.Marked"));
        }
    }

    bool activated_with_target = false;

  private:
    ActivationInfo m_info;
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

    SECTION("Bounded active condition allows activation from source attributes and tags") {
        owner.setAttribute("HP", 42.0f);
        owner.setAttribute("MP", 12.0f);
        owner.addTag(GameplayTag("State.Focused"));
        fireAbility.editInfo().activeCondition = "source.hp >= 40 && source.hasTag('State.Focused') == true";

        const auto check = fireAbility.evaluateActivation(owner);
        REQUIRE(check.allowed);
        REQUIRE(check.reason.empty());
    }

    SECTION("Bounded active condition blocks activation with deterministic reason") {
        owner.setAttribute("HP", 8.0f);
        fireAbility.editInfo().activeCondition = "source.hp >= 40";

        const auto check = fireAbility.evaluateActivation(owner);
        REQUIRE_FALSE(check.allowed);
        REQUIRE(check.reason == "active_condition_false");
        REQUIRE(check.detail.find("source.hp >= 40") != std::string::npos);

        REQUIRE_FALSE(owner.tryActivateAbility(fireAbility));
        const auto& history = owner.getAbilityExecutionHistory();
        REQUIRE(history.size() == 1);
        REQUIRE(history[0].outcome == "blocked");
        REQUIRE(history[0].reason == "active_condition_false");
        REQUIRE(history[0].detail.find("source.hp >= 40") != std::string::npos);

        urpg::editor::AbilityInspectorPanel panel;
        panel.update(owner);
        const auto& snapshot = panel.getRenderSnapshot();
        REQUIRE(snapshot.diagnostic_count == 1);
        REQUIRE(snapshot.diagnostic_lines[0].find("active_condition_false") != std::string::npos);
        REQUIRE(snapshot.diagnostic_lines[0].find("source.hp >= 40") != std::string::npos);
    }

    SECTION("Bounded active condition can inspect the primary target") {
        AbilitySystemComponent targetOwner;
        targetOwner.addTag(GameplayTag("State.Marked"));
        fireAbility.editInfo().activeCondition = "target.hasTag('State.Marked') == true";

        GameplayAbility::AbilityExecutionContext context;
        GameplayAbility::AbilityExecutionTarget target;
        target.abilitySystem = &targetOwner;
        target.runtimeId = "target.1";
        context.targets.push_back(target);

        const auto check = fireAbility.evaluateActivation(owner, context);
        REQUIRE(check.allowed);
    }

    SECTION("Unsupported active condition grammar fails closed") {
        fireAbility.editInfo().activeCondition = "source.hp + source.mp > 50";

        const auto check = fireAbility.evaluateActivation(owner);
        REQUIRE_FALSE(check.allowed);
        REQUIRE(check.reason == "condition_parse_error");
        REQUIRE(check.detail.find("source.hp + source.mp > 50") != std::string::npos);

        REQUIRE_FALSE(owner.tryActivateAbility(fireAbility));
        const auto& history = owner.getAbilityExecutionHistory();
        REQUIRE(history.size() == 1);
        REQUIRE(history[0].outcome == "blocked");
        REQUIRE(history[0].reason == "condition_parse_error");
        REQUIRE(history[0].detail.find("source.hp + source.mp > 50") != std::string::npos);

        urpg::editor::AbilityInspectorPanel panel;
        panel.update(owner);
        const auto& snapshot = panel.getRenderSnapshot();
        REQUIRE(snapshot.diagnostic_count == 1);
        REQUIRE(snapshot.diagnostic_lines[0].find("condition_parse_error") != std::string::npos);
        REQUIRE(snapshot.diagnostic_lines[0].find("source.hp + source.mp > 50") != std::string::npos);
    }

    SECTION("Passive conditions remain out of activation evaluation until a runtime evaluator exists") {
        fireAbility.editInfo().passiveCondition = "source.hp > 0";
        REQUIRE(fireAbility.canActivate(owner));
        REQUIRE(owner.tryActivateAbility(fireAbility));

        const auto& history = owner.getAbilityExecutionHistory();
        REQUIRE(history.size() == 1);
        REQUIRE(history[0].outcome == "executed");
        REQUIRE(history[0].reason.empty());
    }

    SECTION("Effects are always admitted even when modifier tag gates are unmet") {
        AbilitySystemComponent effectOwner;
        urpg::GameplayEffect gatedEffect;
        gatedEffect.id = "BUFF_ATTACK";
        gatedEffect.duration = 8.0f;

        urpg::GameplayEffectModifier gatedModifier;
        gatedModifier.attributeName = "Attack";
        gatedModifier.operation = urpg::ModifierOp::Add;
        gatedModifier.value = 12.0f;
        gatedModifier.requiredTag = "State.Empowered";
        gatedEffect.modifiers.push_back(gatedModifier);

        REQUIRE(effectOwner.canApplyEffect(gatedEffect));
        effectOwner.applyEffect(gatedEffect);
        REQUIRE(effectOwner.getActiveEffectCount() == 1);
        REQUIRE(effectOwner.getAttribute("Attack", 100.0f) == 100.0f);

        effectOwner.addTag(GameplayTag("State.Empowered"));
        REQUIRE(effectOwner.getAttribute("Attack", 100.0f) == 112.0f);
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

    SECTION("Panel snapshot exposes empty runtime and disabled command states") {
        AbilitySystemComponent emptyOwner;
        urpg::editor::AbilityInspectorPanel panel;
        panel.update(emptyOwner);

        const auto& snapshot = panel.getRenderSnapshot();
        REQUIRE(snapshot.status == "empty");
        REQUIRE(snapshot.empty_reason == "No abilities are bound to this runtime.");
        REQUIRE_FALSE(snapshot.error_message.empty());

        const auto selectControl = std::find_if(snapshot.controls.begin(), snapshot.controls.end(), [](const auto& control) {
            return control.id == "select_ability";
        });
        REQUIRE(selectControl != snapshot.controls.end());
        REQUIRE_FALSE(selectControl->enabled);
        REQUIRE(selectControl->disabled_reason == "No abilities are bound to this runtime.");
    }

    SECTION("Context-aware activation can affect a target ASC without breaking source-only activation") {
        AbilitySystemComponent targetOwner;
        owner.setAttribute("MP", 10.0f);
        ContextAwareAbility markAbility;

        GameplayAbility::AbilityExecutionContext context;
        GameplayAbility::AbilityExecutionTarget target;
        target.abilitySystem = &targetOwner;
        target.runtimeId = "target.1";
        context.targets.push_back(target);

        REQUIRE(owner.tryActivateAbility(markAbility, context));
        REQUIRE(markAbility.activated_with_target);
        REQUIRE(targetOwner.getTags().hasTag(GameplayTag("State.Marked")));
        REQUIRE(owner.getAttribute("MP", 0.0f) == 8.0f);

        ContextAwareAbility sourceOnlyAbility;
        owner.setAttribute("MP", 10.0f);
        REQUIRE(owner.tryActivateAbility(sourceOnlyAbility));
        REQUIRE_FALSE(sourceOnlyAbility.activated_with_target);
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
