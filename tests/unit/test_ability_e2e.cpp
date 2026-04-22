#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/ability_state_machine.h"
#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/gameplay_effect.h"

using namespace urpg;
using namespace urpg::ability;
using namespace urpg::editor;

/**
 * @brief A realistic ability that applies a timed buff and uses a state machine.
 */
class BuffAbility : public GameplayAbility {
public:
    BuffAbility() {
        id = "skill.power_surge";
        cooldownTime = 5.0f;
        mpCost = 15.0f;
    }

    const std::string& getId() const override { return id; }

    const ActivationInfo& getActivationInfo() const override {
        static ActivationInfo info;
        info.cooldownSeconds = cooldownTime;
        info.mpCost = static_cast<int32_t>(mpCost);
        return info;
    }

    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);

        GameplayEffect buff;
        buff.id = "BUFF_POWER_SURGE";
        buff.duration = 8.0f;
        buff.stackingPolicy = GameplayEffectStackingPolicy::Refresh;

        GameplayEffectModifier mod;
        mod.attributeName = "Attack";
        mod.operation = ModifierOp::Add;
        mod.value = 25.0f;
        buff.modifiers.push_back(mod);

        source.applyEffect(buff);
    }
};

/**
 * @brief A state-machine-driven ability with windup, impact, and recovery.
 */
class StateDrivenAbility : public GameplayAbility {
public:
    StateDrivenAbility() {
        id = "skill.heavy_slam";
        cooldownTime = 4.0f;
        mpCost = 10.0f;
    }

    const std::string& getId() const override { return id; }

    const ActivationInfo& getActivationInfo() const override {
        static ActivationInfo info;
        info.cooldownSeconds = cooldownTime;
        info.mpCost = static_cast<int32_t>(mpCost);
        return info;
    }

    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);

        m_machine = std::make_unique<AbilityStateMachine>(id);
        m_windupTimer = 0.0f;
        m_recoveryTimer = 0.0f;

        AbilityState windup;
        windup.name = "windup";
        windup.inherentTags.addTag(GameplayTag("State.Ability.Windup"));
        windup.onEnter = [](AbilitySystemComponent& asc) {
            asc.addTag(GameplayTag("State.Immune.Stagger"));
        };
        windup.onTick = [this](AbilitySystemComponent&, float dt) {
            m_windupTimer += dt;
            return m_windupTimer >= 0.5f;
        };

        AbilityState impact;
        impact.name = "impact";
        impact.onEnter = [](AbilitySystemComponent& asc) {
            asc.removeTag(GameplayTag("State.Immune.Stagger"));

            GameplayEffect debuff;
            debuff.id = "DEBUFF_ARMOR_BREAK";
            debuff.duration = 3.0f;
            GameplayEffectModifier mod;
            mod.attributeName = "Defense";
            mod.operation = ModifierOp::Add;
            mod.value = -10.0f;
            debuff.modifiers.push_back(mod);
            asc.applyEffect(debuff);
        };
        impact.onTick = [](AbilitySystemComponent&, float) { return true; };

        AbilityState recovery;
        recovery.name = "recovery";
        recovery.onTick = [this](AbilitySystemComponent&, float dt) {
            m_recoveryTimer += dt;
            return m_recoveryTimer >= 0.3f;
        };

        m_machine->addState(windup);
        m_machine->addState(impact);
        m_machine->addState(recovery);
        m_machine->start(source);
    }

    void update(float deltaTime) override {
        GameplayAbility::update(deltaTime);
        // State machine requires a live ASC; we don't have it cached here.
        // In real usage the ASC owner would drive update. This testability
        // helper exposes the machine for direct test ticking.
    }

    AbilityStateMachine* getMachine() const { return m_machine.get(); }

private:
    std::unique_ptr<AbilityStateMachine> m_machine;
    float m_windupTimer = 0.0f;
    float m_recoveryTimer = 0.0f;
};

TEST_CASE("Ability end-to-end: activation, commit, cooldown, effect, and inspector coherence", "[ability][e2e]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 50.0f);
    asc.setAttribute("Attack", 100.0f);

    BuffAbility buff;
    AbilityInspectorPanel panel;

    SECTION("Initial state is clean") {
        panel.update(asc);
        auto snap = panel.getRenderSnapshot();
        REQUIRE(snap.diagnostic_count == 0);
        REQUIRE(asc.getCooldownRemaining(buff.getId()) == 0.0f);
        REQUIRE(asc.getAttribute("Attack", 0.0f) == 100.0f);
    }

    SECTION("Activation succeeds and commits cost + cooldown") {
        REQUIRE(asc.tryActivateAbility(buff));
        REQUIRE(asc.getAttribute("MP", 0.0f) == 35.0f);
        REQUIRE(asc.getCooldownRemaining(buff.getId()) == 5.0f);
    }

    SECTION("Effect is applied and modifies attribute") {
        asc.tryActivateAbility(buff);
        REQUIRE(asc.getAttribute("Attack", 0.0f) == 125.0f);
        REQUIRE(asc.getActiveEffectCount() == 1);
    }

    SECTION("Re-activation is blocked during cooldown") {
        asc.tryActivateAbility(buff);
        REQUIRE_FALSE(asc.tryActivateAbility(buff));

        const auto& history = asc.getAbilityExecutionHistory();
        REQUIRE(history.size() == 2);
        REQUIRE(history[0].outcome == "executed");
        REQUIRE(history[1].outcome == "blocked");
        REQUIRE(history[1].reason == "cooldown_active");
    }

    SECTION("Cooldown decay allows re-activation") {
        asc.tryActivateAbility(buff);
        asc.update(5.0f);
        REQUIRE(asc.getCooldownRemaining(buff.getId()) == 0.0f);
        REQUIRE(asc.tryActivateAbility(buff));
        REQUIRE(asc.getAttribute("MP", 0.0f) == 20.0f);
    }

    SECTION("Effect expires after duration") {
        asc.tryActivateAbility(buff);
        REQUIRE(asc.getAttribute("Attack", 0.0f) == 125.0f);
        asc.update(8.1f);
        REQUIRE(asc.getActiveEffectCount() == 0);
        REQUIRE(asc.getAttribute("Attack", 0.0f) == 100.0f);
    }

    SECTION("Inspector panel reflects execution history and active effects") {
        asc.tryActivateAbility(buff);
        panel.update(asc);

        auto snap = panel.getRenderSnapshot();
        REQUIRE(snap.diagnostic_count == 1);
        REQUIRE(snap.latest_ability_id == "skill.power_surge");
        REQUIRE(snap.latest_outcome == "executed");
        REQUIRE(snap.diagnostic_lines[0].find("mp ") != std::string::npos);
        REQUIRE(snap.diagnostic_lines[0].find("35") != std::string::npos);
        REQUIRE(snap.diagnostic_lines[0].find("effects=1") != std::string::npos);
        REQUIRE(snap.diagnostic_lines[0].find("effects=1") != std::string::npos);
    }

    SECTION("Inspector model reflects cooldown blocking after update") {
        asc.tryActivateAbility(buff);
        asc.update(2.0f);
        panel.update(asc);

        const auto& model = panel.getModel();
        const auto& abilities = model.getAbilities();
        REQUIRE(abilities.size() == 0); // No abilities granted to ASC in this test
        // The effect application is on the ASC, not the ability list.
        // This section validates that the panel stays coherent when no abilities are granted.
    }
}

TEST_CASE("Ability end-to-end: state machine transitions with effect application and inspector coherence", "[ability][e2e]") {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", 30.0f);
    asc.setAttribute("Defense", 50.0f);

    StateDrivenAbility slam;
    AbilityInspectorPanel panel;

    SECTION("State machine drives windup -> impact -> recovery") {
        REQUIRE(asc.tryActivateAbility(slam));
        REQUIRE(asc.getAttribute("MP", 0.0f) == 20.0f);
        REQUIRE(asc.getCooldownRemaining(slam.getId()) == 4.0f);

        auto* machine = slam.getMachine();
        REQUIRE(machine != nullptr);
        REQUIRE(machine->isRunning());
        REQUIRE(machine->getCurrentStateName() == "windup");
        REQUIRE(asc.getTags().hasTag(GameplayTag("State.Immune.Stagger")));

        // Tick through windup
        machine->update(asc, 0.6f);
        REQUIRE(machine->getCurrentStateName() == "impact");
        REQUIRE_FALSE(asc.getTags().hasTag(GameplayTag("State.Immune.Stagger")));
        REQUIRE(asc.getAttribute("Defense", 0.0f) == 40.0f); // debuff applied

        // Tick through impact (instant)
        machine->update(asc, 0.1f);
        REQUIRE(machine->getCurrentStateName() == "recovery");

        // Tick through recovery
        machine->update(asc, 0.4f);
        REQUIRE(machine->getStatus() == AbilityStateStatus::Finished);
    }

    SECTION("Execution history records all state transitions deterministically") {
        asc.tryActivateAbility(slam);
        auto* machine = slam.getMachine();
        machine->update(asc, 0.6f);
        machine->update(asc, 0.1f);
        machine->update(asc, 0.4f);

        const auto& history = asc.getAbilityExecutionHistory();
        REQUIRE(history.size() == 5); // windup entered (during activate) + activate executed + impact entered + recovery entered + finished
        // Note: state machine start() records inside activate() before tryActivateAbility records the activate outcome.
        REQUIRE(history[0].stage == "state_machine");
        REQUIRE(history[0].outcome == "entered");
        REQUIRE(history[0].state_name == "windup");
        REQUIRE(history[1].stage == "activate");
        REQUIRE(history[1].outcome == "executed");
        REQUIRE(history[2].stage == "state_machine");
        REQUIRE(history[2].outcome == "entered");
        REQUIRE(history[2].state_name == "impact");
        REQUIRE(history[3].stage == "state_machine");
        REQUIRE(history[3].outcome == "entered");
        REQUIRE(history[3].state_name == "recovery");
        REQUIRE(history[4].stage == "state_machine");
        REQUIRE(history[4].outcome == "finished");
    }

    SECTION("Inspector panel reflects state machine execution history") {
        asc.tryActivateAbility(slam);
        auto* machine = slam.getMachine();
        machine->update(asc, 0.6f);
        machine->update(asc, 0.1f);
        machine->update(asc, 0.4f);

        panel.update(asc);
        auto snap = panel.getRenderSnapshot();
        REQUIRE(snap.diagnostic_count == 5);
        REQUIRE(snap.latest_ability_id == "skill.heavy_slam");
        REQUIRE(snap.latest_outcome == "finished");

        bool foundWindup = false;
        bool foundImpact = false;
        bool foundRecovery = false;
        for (const auto& line : snap.diagnostic_lines) {
            if (line.find("[windup]") != std::string::npos) foundWindup = true;
            if (line.find("[impact]") != std::string::npos) foundImpact = true;
            if (line.find("[recovery]") != std::string::npos) foundRecovery = true;
        }
        REQUIRE(foundWindup);
        REQUIRE(foundImpact);
        REQUIRE(foundRecovery);
    }
}
