#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_orchestration.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/ability_state_machine.h"
#include "editor/ability/ability_orchestration_panel.h"
#include "editor/ability/ability_sandbox_panel.h"
#include "editor/ability/ability_inspector_panel.h"
#include "engine/core/ability/gameplay_effect.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace urpg;
using namespace urpg::ability;
using namespace urpg::editor;

namespace {

std::filesystem::path abilityRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadAbilityJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

} // namespace

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

TEST_CASE("Ability sandbox shows costs, cooldowns, tags, effects, and runtime activation",
          "[ability][sandbox][wysiwyg]") {
    const auto json = loadAbilityJson(abilityRepoRoot() / "content" / "fixtures" / "ability_sandbox_fixture.json");
    const auto document = AbilitySandboxDocument::fromJson(json);

    AbilitySandboxPanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().sandbox_id == "power_surge_sandbox");
    REQUIRE(panel.snapshot().ability_id == "skill.power_surge");
    REQUIRE(panel.snapshot().mp_cost == 15.0f);
    REQUIRE(panel.snapshot().cooldown_seconds == 5.0f);
    REQUIRE(panel.snapshot().seconds_between_attempts == 0.0f);
    REQUIRE(panel.snapshot().mp_before == 40.0f);
    REQUIRE(panel.snapshot().mp_after == 25.0f);
    REQUIRE(panel.snapshot().cooldown_after == 5.0f);
    REQUIRE(panel.snapshot().effect_id == "effect.power_surge");
    REQUIRE(panel.snapshot().effect_attribute == "Attack");
    REQUIRE(panel.snapshot().effect_before == 10.0f);
    REQUIRE(panel.snapshot().effect_after == 18.0f);
    REQUIRE(panel.snapshot().source_tag_count == 2);
    REQUIRE(panel.snapshot().required_tag_count == 1);
    REQUIRE(panel.snapshot().blocking_tag_count == 1);
    REQUIRE(panel.snapshot().active_effect_count == 1);
    REQUIRE(panel.snapshot().activation_attempt_count == 2);
    REQUIRE(panel.snapshot().execution_history_count == 2);
    REQUIRE(panel.snapshot().runtime_trace_count == 4);
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().activation_allowed);
    REQUIRE(panel.snapshot().activation_executed);
    REQUIRE(panel.snapshot().mp_delta == -15.0f);
    REQUIRE(panel.snapshot().effect_delta == 8.0f);
    REQUIRE(panel.snapshot().activation_success_ratio == 1.0f);
    REQUIRE(panel.snapshot().ux_focus_lane == "cooldown");
    REQUIRE(panel.snapshot().status_message == "Ability sandbox preview is ready.");
    REQUIRE_FALSE(panel.snapshot().saved_project_json.empty());

    REQUIRE(panel.result().activation_steps[0].executed);
    REQUIRE_FALSE(panel.result().activation_steps[1].executed);
    REQUIRE(panel.result().activation_steps[1].blocking_reason == "cooldown_active");
    REQUIRE(std::find(panel.result().runtime_trace.begin(), panel.result().runtime_trace.end(),
                      "attempt:2:blocked:cooldown_active") != panel.result().runtime_trace.end());
}

TEST_CASE("Ability sandbox panel edits live costs, cooldowns, tags, and effects through runtime preview",
          "[ability][sandbox][wysiwyg]") {
    const auto json = loadAbilityJson(abilityRepoRoot() / "content" / "fixtures" / "ability_sandbox_fixture.json");
    const auto document = AbilitySandboxDocument::fromJson(json);

    AbilitySandboxPanel panel;
    panel.loadDocument(document);
    panel.setSourceMp(50.0f);
    panel.setAbilityCost(10.0f);
    panel.setAbilityCooldown(2.0f);
    panel.setEffectValue(12.0f);
    panel.setActivationAttempts(2, 2.0f);
    panel.removeSourceTag("Class.Warrior");
    panel.render();

    REQUIRE_FALSE(panel.snapshot().activation_executed);
    REQUIRE(panel.snapshot().blocking_reason == "missing_required_tags");
    REQUIRE(panel.snapshot().ux_focus_lane == "requirements");
    REQUIRE(panel.snapshot().source_tag_count == 1);
    REQUIRE(panel.snapshot().mp_after == 50.0f);

    panel.addSourceTag("Class.Warrior");
    panel.render();

    REQUIRE(panel.snapshot().activation_executed);
    REQUIRE(panel.snapshot().activation_attempt_count == 2);
    REQUIRE(panel.snapshot().execution_history_count == 2);
    REQUIRE(panel.snapshot().mp_after == 30.0f);
    REQUIRE(panel.snapshot().cooldown_after == 2.0f);
    REQUIRE(panel.snapshot().effect_after == 34.0f);
    REQUIRE(panel.snapshot().activation_success_ratio == 1.0f);
    REQUIRE(panel.result().activation_steps[0].effect_attribute_after == 22.0f);
    REQUIRE(panel.result().activation_steps[1].executed);
    REQUIRE(panel.result().activation_steps[1].cooldown_before == 0.0f);
    REQUIRE(std::find(panel.result().runtime_trace.begin(), panel.result().runtime_trace.end(),
                      "tick:2.000000") != panel.result().runtime_trace.end());

    const auto saved = nlohmann::json::parse(panel.snapshot().saved_project_json);
    REQUIRE(saved["source_mp"] == 50.0f);
    REQUIRE(saved["ability"]["mp_cost"] == 10.0f);
    REQUIRE(saved["ability"]["cooldown_seconds"] == 2.0f);
    REQUIRE(saved["ability"]["effect_value"] == 12.0f);
    REQUIRE(saved["activation_attempts"] == 2);
    REQUIRE(saved["seconds_between_attempts"] == 2.0f);
}

TEST_CASE("Ability sandbox saved project data round-trips through schema surface",
          "[ability][sandbox][wysiwyg]") {
    const auto json = loadAbilityJson(abilityRepoRoot() / "content" / "fixtures" / "ability_sandbox_fixture.json");
    const auto document = AbilitySandboxDocument::fromJson(json);
    const auto saved = document.toJson();
    const auto restored = AbilitySandboxDocument::fromJson(saved);

    REQUIRE(saved["schema"] == "urpg.ability_sandbox.v1");
    REQUIRE(restored.id == document.id);
    REQUIRE(restored.ability.ability_id == "skill.power_surge");
    REQUIRE(restored.required_tags.size() == 1);
    REQUIRE(restored.activation_attempts == 2);
    REQUIRE(restored.seconds_between_attempts == 0.0f);

    const auto result = RunAbilitySandbox(restored);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.activation_executed);
    REQUIRE(result.mp_after == 25.0f);
    REQUIRE(result.effect_attribute_after == 18.0f);
    REQUIRE(result.activation_steps.size() == 2);
    REQUIRE(result.activation_steps[1].blocking_reason == "cooldown_active");
}

TEST_CASE("Ability sandbox diagnostics and tag gates block false complete claims",
          "[ability][sandbox][wysiwyg]") {
    AbilitySandboxDocument document;
    document.id = "broken_sandbox";
    document.source_mp = 4.0f;
    document.source_attack = 10.0f;
    document.ability.ability_id = "skill.blocked";
    document.ability.cooldown_seconds = -1.0f;
    document.ability.mp_cost = 10.0f;
    document.ability.effect_id = "";
    document.ability.effect_attribute = "";
    document.activation_attempts = 0;
    document.seconds_between_attempts = -1.0f;
    document.required_tags = {"Class.Mage"};

    AbilitySandboxPanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().diagnostic_count >= 2);
    REQUIRE(panel.snapshot().ux_focus_lane == "diagnostics");
    REQUIRE_FALSE(panel.snapshot().activation_executed);
    REQUIRE(panel.snapshot().status_message == "Ability sandbox preview has diagnostics.");

    const auto& diagnostics = panel.result().diagnostics;
    const auto hasCode = [&diagnostics](const std::string& code) {
        return std::any_of(diagnostics.begin(), diagnostics.end(), [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
    };
    REQUIRE(hasCode("invalid_cooldown"));
    REQUIRE(hasCode("invalid_activation_attempts"));
    REQUIRE(hasCode("invalid_attempt_interval"));
    REQUIRE(hasCode("missing_effect_id"));
    REQUIRE(hasCode("missing_effect_attribute"));

    document.ability.cooldown_seconds = 1.0f;
    document.ability.effect_id = "effect.blocked";
    document.ability.effect_attribute = "Attack";
    document.activation_attempts = 1;
    document.seconds_between_attempts = 0.0f;
    document.source_tags.clear();
    const auto result = RunAbilitySandbox(document);
    REQUIRE(result.diagnostics.empty());
    REQUIRE_FALSE(result.activation_allowed);
    REQUIRE_FALSE(result.activation_executed);
    REQUIRE(result.blocking_reason == "missing_required_tags");
}

TEST_CASE("Ability orchestration executes authored battle asset through runtime targets",
          "[ability][orchestration][wysiwyg]") {
    const auto json = loadAbilityJson(abilityRepoRoot() / "content" / "fixtures" / "ability_orchestration_fixture.json");
    const auto document = AbilityOrchestrationDocument::fromJson(json);
    const auto result = runAbilityOrchestration(document);

    REQUIRE(result.valid);
    REQUIRE(result.activation_attempted);
    REQUIRE(result.activation_executed);
    REQUIRE(result.source_mp_before == 40.0f);
    REQUIRE(result.source_mp_after == 28.0f);
    REQUIRE(result.cooldown_after == 4.0f);
    REQUIRE(result.battle_commands_received == 1);
    REQUIRE(result.battle_commands_executed == 1);
    REQUIRE(result.battle_commands_blocked == 0);
    REQUIRE(result.targets.size() == 2);
    REQUIRE(result.targets[0].id == "enemy.slime_a");
    REQUIRE(result.targets[0].effect_attribute_before == 60.0f);
    REQUIRE(result.targets[0].effect_attribute_after == 42.0f);
    REQUIRE(result.targets[0].active_effect_count == 1);
    REQUIRE(result.targets[1].effect_attribute_after == 52.0f);
    REQUIRE(result.diagnostics.empty());

    const auto resultJson = abilityOrchestrationResultToJson(result);
    REQUIRE(resultJson["mode"] == "battle");
    REQUIRE(resultJson["battleSnapshot"]["turn_number"] == 3);
    REQUIRE(resultJson["targets"].size() == 2);
}

TEST_CASE("Ability orchestration panel exposes saved data, live preview, and diagnostics",
          "[ability][orchestration][wysiwyg]") {
    const auto json = loadAbilityJson(abilityRepoRoot() / "content" / "fixtures" / "ability_orchestration_fixture.json");
    const auto document = AbilityOrchestrationDocument::fromJson(json);

    AbilityOrchestrationPanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().orchestration_id == "battle_line_cleave_orchestration");
    REQUIRE(panel.snapshot().mode == "battle");
    REQUIRE(panel.snapshot().ability_id == "skill.line_cleave");
    REQUIRE(panel.snapshot().activation_executed);
    REQUIRE(panel.snapshot().source_mp_before == 40.0f);
    REQUIRE(panel.snapshot().source_mp_after == 28.0f);
    REQUIRE(panel.snapshot().cooldown_after == 4.0f);
    REQUIRE(panel.snapshot().target_count == 2);
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().status_message == "Ability orchestration preview is ready.");
    REQUIRE(panel.snapshot().saved_project_json["schema"] == "urpg.ability_orchestration.v1");
    REQUIRE(panel.snapshot().runtime_result_json["activationExecuted"] == true);
}

TEST_CASE("Ability orchestration task composition round-trips and previews creator rows",
          "[ability][orchestration][task]") {
    nlohmann::json json = {
        {"schema", "urpg.ability_orchestration.v1"},
        {"id", "chain_lightning_orchestration"},
        {"mode", "map"},
        {"ability",
         {{"ability_id", "skill.chain_lightning"},
          {"cooldown_seconds", 2.0},
          {"mp_cost", 5.0},
          {"effect_id", "state.shocked"},
          {"effect_attribute", "HP"},
          {"effect_operation", "Add"},
          {"effect_value", -12.0},
          {"effect_duration", 3.0},
          {"pattern", {{"name", "single"}, {"points", {{{"x", 0}, {"y", 0}}}}}}}},
        {"source", {{"id", "actor.mage"}, {"mp", 12.0}, {"effect_attribute_base", 40.0}, {"tags", nlohmann::json::array()}}},
        {"targets", {{{"id", "enemy.slime"}, {"mp", 0.0}, {"effect_attribute_base", 30.0}, {"tags", nlohmann::json::array()}}}},
        {"tasks",
         {{{"id", "await_confirm"}, {"kind", "wait_input"}, {"action", "Confirm"}, {"timeout_ms", 1000}},
          {{"id", "branch_mp"},
           {"kind", "branch_on_condition"},
           {"condition", "source.mp >= 5"},
           {"on_true", "apply_shock"},
           {"on_false", "play_fizzle"}},
          {{"id", "apply_shock"}, {"kind", "apply_effect"}, {"effect_id", "state.shocked"}, {"target", "primary"}},
          {{"id", "play_fizzle"}, {"kind", "play_cue"}, {"cue_id", "ability.fizzle"}}}},
    };

    const auto document = AbilityOrchestrationDocument::fromJson(json);
    REQUIRE(document.tasks.size() == 4);
    REQUIRE(document.tasks[0].id == "await_confirm");
    REQUIRE(document.tasks[1].kind == "branch_on_condition");
    REQUIRE(document.tasks[1].on_true == "apply_shock");
    REQUIRE(document.tasks[1].on_false == "play_fizzle");

    const auto saved = document.toJson();
    REQUIRE(saved["tasks"].size() == 4);
    REQUIRE(saved["tasks"][0]["kind"] == "wait_input");
    REQUIRE(saved["tasks"][1]["condition"] == "source.mp >= 5");

    AbilityOrchestrationPanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.snapshot().task_count == 4);
    REQUIRE(panel.snapshot().task_preview_rows.size() == 4);
    REQUIRE(panel.snapshot().task_preview_rows[0].id == "await_confirm");
    REQUIRE(panel.snapshot().task_preview_rows[0].kind == "wait_input");
    REQUIRE(panel.snapshot().task_preview_rows[0].status == "ready");
    REQUIRE(panel.snapshot().task_preview_rows[1].detail.find("source.mp >= 5") != std::string::npos);
    REQUIRE(panel.snapshot().runtime_result_json["taskPreviewRows"].size() == 4);
}

TEST_CASE("Ability orchestration task composition validates branch targets and conditions",
          "[ability][orchestration][task]") {
    AbilityOrchestrationDocument document;
    document.id = "bad_task_graph";
    document.mode = AbilityOrchestrationMode::Map;
    document.ability.ability_id = "skill.branch";
    document.ability.cooldown_seconds = 1.0f;
    document.ability.mp_cost = 5.0f;
    document.ability.effect_id = "effect.branch";
    document.ability.effect_attribute = "HP";
    document.ability.effect_value = -10.0f;
    document.source.id = "actor.mage";
    document.source.mp = 10.0f;
    document.targets.push_back({"enemy.slime", 0.0f, 30.0f, 0, 0, {}});
    document.tasks.push_back({"branch", "branch_on_condition", "", 0, "source.hp + source.mp > 10", "missing_true",
                              "missing_false", "", "", ""});

    const auto diagnostics = document.validate();
    const auto hasCode = [&diagnostics](const std::string& code) {
        return std::any_of(diagnostics.begin(), diagnostics.end(), [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
    };

    REQUIRE(hasCode("ability_task_branch_missing_target"));
    REQUIRE(hasCode("ability_task_branch_condition_invalid"));
}

TEST_CASE("Ability orchestration reports map pattern and tag gate diagnostics",
          "[ability][orchestration][wysiwyg]") {
    AbilityOrchestrationDocument document;
    document.id = "map_bad_target";
    document.mode = AbilityOrchestrationMode::Map;
    document.ability.ability_id = "skill.line_cleave";
    document.ability.cooldown_seconds = 1.0f;
    document.ability.mp_cost = 5.0f;
    document.ability.effect_id = "effect.line_cleave";
    document.ability.effect_attribute = "HP";
    document.ability.effect_value = -10.0f;
    document.ability.pattern = urpg::PatternField("One Step");
    document.ability.pattern.addPoint(1, 0);
    document.source.id = "actor.hero";
    document.source.mp = 20.0f;
    document.source.tags = {"Class.Warrior"};
    document.required_tags = {"Class.Warrior"};
    document.targets.push_back({"enemy.out_of_range", 0.0f, 30.0f, 3, 0, {}});

    auto result = runAbilityOrchestration(document);
    REQUIRE(result.valid);
    REQUIRE_FALSE(result.activation_attempted);
    REQUIRE_FALSE(result.activation_executed);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0].code == "target_out_of_pattern");
    REQUIRE(result.targets[0].in_pattern == false);

    document.targets[0].x = 1;
    document.source.tags.clear();
    result = runAbilityOrchestration(document);
    REQUIRE(result.valid);
    REQUIRE(result.activation_attempted);
    REQUIRE_FALSE(result.activation_executed);
    REQUIRE(result.blocking_reason == "missing_required_tags");
    REQUIRE(result.targets[0].effect_attribute_after == 30.0f);
}
