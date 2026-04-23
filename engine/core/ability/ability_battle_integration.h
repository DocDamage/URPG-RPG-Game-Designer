#pragma once

/**
 * @file ability_battle_integration.h
 * @brief S24-T03/T04: Ability command queue wiring into BattleFlowController.
 *
 * Provides AbilityBattleQueue — a bounded command queue that sits between the
 * ability framework and the battle flow controller.  During the BattleFlowPhase::Action
 * phase the battle loop dequeues pending ability commands, resolves them through
 * the matching AbilitySystemComponent, and records deterministic execution outcomes.
 *
 * Design constraints:
 *  - The queue is strictly ordered by (priority DESC, speed DESC, insertion order ASC).
 *  - Execution outcomes are deterministic given the same queue contents and
 *    AbilitySystemComponent state.  No RNG is introduced here; callers supply
 *    pre-resolved targets.
 *  - Diagnostics (AbilityBattleDiagnosticsSnapshot) are produced after each
 *    flush and can be serialised to JSON for diagnostics panels and release
 *    validation tests.
 */

#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/battle/battle_core.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace urpg::ability {

// ---------------------------------------------------------------------------
// Pending ability command
// ---------------------------------------------------------------------------

/// A queued request to activate one ability during the current battle action phase.
struct AbilityBattleCommand {
    /// Subject that is activating the ability.
    std::string subject_id;
    /// Target(s) of the ability.  May be empty for self-targeting abilities.
    std::vector<std::string> target_ids;
    /// Ability identifier to activate on the subject's AbilitySystemComponent.
    std::string ability_id;
    /// Speed determines ordering (higher = earlier in the turn).
    int32_t speed = 0;
    /// Priority overrides speed ties (higher = earlier).
    int32_t priority = 0;
};

// ---------------------------------------------------------------------------
// Execution outcome
// ---------------------------------------------------------------------------

/// Result of resolving one ability command against an AbilitySystemComponent.
struct AbilityBattleOutcome {
    std::string subject_id;
    std::string ability_id;
    /// "activated", "blocked", "cooldown", "insufficient_mp", "not_found", "error"
    std::string result;
    std::string reason;
    std::string detail;
    float mp_before = 0.0f;
    float mp_after = 0.0f;
    float cooldown_after = 0.0f;
    size_t active_effect_count = 0;
};

// ---------------------------------------------------------------------------
// Diagnostics snapshot
// ---------------------------------------------------------------------------

/// Produced after each flush() call.  Captures the complete ordered execution
/// log for the action phase so tests and diagnostics panels can verify
/// deterministic turn-by-turn behavior.
struct AbilityBattleDiagnosticsSnapshot {
    int32_t turn_number = 0;
    size_t commands_received = 0;
    size_t commands_executed = 0;
    size_t commands_blocked = 0;
    std::vector<AbilityBattleOutcome> outcomes;

    [[nodiscard]] nlohmann::json toJson() const {
        nlohmann::json j;
        j["turn_number"] = turn_number;
        j["commands_received"] = commands_received;
        j["commands_executed"] = commands_executed;
        j["commands_blocked"] = commands_blocked;
        nlohmann::json outcomes_arr = nlohmann::json::array();
        for (const auto& o : outcomes) {
            nlohmann::json oj;
            oj["subject_id"] = o.subject_id;
            oj["ability_id"] = o.ability_id;
            oj["result"] = o.result;
            oj["reason"] = o.reason;
            oj["detail"] = o.detail;
            oj["mp_before"] = o.mp_before;
            oj["mp_after"] = o.mp_after;
            oj["cooldown_after"] = o.cooldown_after;
            oj["active_effect_count"] = o.active_effect_count;
            outcomes_arr.push_back(std::move(oj));
        }
        j["outcomes"] = std::move(outcomes_arr);
        return j;
    }
};

// ---------------------------------------------------------------------------
// Ability battle queue
// ---------------------------------------------------------------------------

/// Sits between AbilitySystemComponent and BattleFlowController.
/// Holds pending ability commands for the current action phase and resolves
/// them deterministically when flush() is called.
class AbilityBattleQueue {
public:
    /// Enqueue an ability command.  Commands may be added during BattleFlowPhase::Input
    /// and are resolved during BattleFlowPhase::Action.
    void enqueue(AbilityBattleCommand cmd) {
        pending_.push_back(std::move(cmd));
    }

    /// Remove all pending commands without executing them.
    void clear() { pending_.clear(); }

    /// Number of pending commands.
    [[nodiscard]] size_t size() const { return pending_.size(); }
    [[nodiscard]] bool empty() const { return pending_.empty(); }

    /// Execute all pending commands in deterministic order against the provided
    /// AbilitySystemComponent map.  Returns a snapshot of every execution outcome.
    ///
    /// @param asc_map   Maps subject_id → AbilitySystemComponent pointer.
    ///                  Subjects absent from this map produce an "error" outcome.
    /// @param turn_number  Current battle turn count (for diagnostics).
    [[nodiscard]] AbilityBattleDiagnosticsSnapshot flush(
        const std::vector<std::pair<std::string, AbilitySystemComponent*>>& asc_map,
        int32_t turn_number = 0)
    {
        AbilityBattleDiagnosticsSnapshot snap;
        snap.turn_number = turn_number;
        snap.commands_received = pending_.size();

        // Sort: priority DESC, speed DESC, then preserve insertion order (stable).
        std::stable_sort(pending_.begin(), pending_.end(),
            [](const AbilityBattleCommand& a, const AbilityBattleCommand& b) {
                if (a.priority != b.priority) return a.priority > b.priority;
                return a.speed > b.speed;
            });

        for (const auto& cmd : pending_) {
            AbilityBattleOutcome outcome;
            outcome.subject_id = cmd.subject_id;
            outcome.ability_id = cmd.ability_id;

            // Find subject's ASC
            AbilitySystemComponent* asc = nullptr;
            for (const auto& [id, ptr] : asc_map) {
                if (id == cmd.subject_id) {
                    asc = ptr;
                    break;
                }
            }

            if (!asc) {
                outcome.result = "error";
                outcome.reason = "subject_not_found";
                outcome.detail = "No AbilitySystemComponent registered for subject: " + cmd.subject_id;
                snap.commands_blocked++;
                snap.outcomes.push_back(std::move(outcome));
                continue;
            }

            // Find the ability on the ASC by scanning the granted ability list
            GameplayAbility* ability_ptr = nullptr;
            for (const auto& ab : asc->getAbilities()) {
                if (ab && ab->getId() == cmd.ability_id) {
                    ability_ptr = ab.get();
                    break;
                }
            }

            if (!ability_ptr) {
                outcome.result = "not_found";
                outcome.reason = "ability_not_registered";
                outcome.detail = "Ability '" + cmd.ability_id +
                                 "' not registered on subject '" + cmd.subject_id + "'";
                snap.commands_blocked++;
                snap.outcomes.push_back(std::move(outcome));
                continue;
            }

            // Capture pre-activation MP (MP attribute; default 9999 when unset)
            outcome.mp_before = asc->getAttribute("MP", 9999.0f);

            // Activation check (returns ActivationCheckResult with reason/detail)
            const auto check = ability_ptr->evaluateActivation(*asc);
            if (!check.allowed) {
                outcome.result = "blocked";
                outcome.reason = check.reason;
                outcome.detail = check.detail;
                outcome.cooldown_after = check.cooldown_remaining;
                outcome.mp_after = outcome.mp_before;
                snap.commands_blocked++;
                snap.outcomes.push_back(std::move(outcome));
                continue;
            }

            // Execute via ASC (handles commit, cooldown, effects)
            asc->tryActivateAbility(*ability_ptr);
            outcome.result = "activated";
            outcome.mp_after = asc->getAttribute("MP", 9999.0f);
            outcome.cooldown_after = asc->getCooldownRemaining(cmd.ability_id);
            outcome.active_effect_count = asc->getActiveEffectCount();
            snap.commands_executed++;
            snap.outcomes.push_back(std::move(outcome));
        }

        pending_.clear();
        return snap;
    }

    /// Returns a snapshot of pending commands in current (unsorted) insertion order.
    [[nodiscard]] std::vector<AbilityBattleCommand> snapshot() const {
        return pending_;
    }

private:
    std::vector<AbilityBattleCommand> pending_;
};

} // namespace urpg::ability
