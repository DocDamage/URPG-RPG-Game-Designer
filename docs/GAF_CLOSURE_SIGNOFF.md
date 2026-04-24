# Gameplay Ability Framework — Release Closure Sign-off

> **Status:** `PARTIAL`  
> **Purpose:** Evidence-gathering artifact for GAF release closure review.  
> **Date:** 2026-04-23  
> **Rule:** This document does **not** promote `readiness_status.json` status. Human review is required for promotion to `READY`.

---

## 1. Runtime Ownership

The Gameplay Ability Framework provides deterministic, native-owned ability execution, tag-based gating, effect stacking, and cooldown management:

| Component | Responsibility | Evidence |
|-----------|---------------|----------|
| `GameplayAbility` | Base class for executable actions with tag-based activation checks, explicit unsupported scripted-condition diagnostics, structured failure reasons, MP cost resolution, and cooldown commit | `engine/core/ability/gameplay_ability.h` |
| `AbilitySystemComponent` | Tag/effect/cooldown owner, attribute modification with additive/multiplicative/override stacking, and bounded deterministic execution history | `engine/core/ability/ability_system_component.h` |
| `AbilityTask` backends | Deterministic async task surface with `WaitTime`, `WaitInput`, `WaitEvent`, and `WaitProjectileCollision`; granted abilities tick task progress through `AbilitySystemComponent::update()` | `engine/core/ability/ability_task.h` |
| `AbilityStateMachine` | Multi-phase ability orchestration (windup → impact → recovery) with per-state tags, enter/tick/exit logic, and deterministic transition diagnostics | `engine/core/ability/ability_state_machine.h` |
| `PatternField` | 2D coordinate pattern authoring for AoE/range requirements with origin validation, radius bounds, and JSON serialization | `engine/core/ability/pattern_field.h` |

All runtime types use C++20 `enum class`, carry no hidden state, and record replay-safe execution history.

---

## 2. Editor Surfaces

Two editor panels provide live ability inspection and diagnostics:

| Surface | Key Capability | Header |
|---------|---------------|--------|
| `AbilityInspectorModel` | Projects abilities, active tags, cooldowns, and blocking reasons from a live `AbilitySystemComponent` | `editor/ability/ability_inspector_model.h` |
| `AbilityInspectorPanel` | Orchestrates model + render snapshot; exposes replay-log-oriented diagnostic lines and structured `AbilityDiagnosticsSnapshot` | `editor/ability/ability_inspector_panel.h` |

---

## 3. Schema Contracts

Native JSON schemas are enforced under `content/schemas/`:

- **`gameplay_ability.schema.json`** (S24-T01) — JSON Schema draft-07 contract for `AuthoredAbilityAsset` definitions. Enforces required fields (`ability_id`, `cooldown_seconds`, `mp_cost`, `effect_id`, `effect_attribute`, `effect_operation`, `effect_value`, `effect_duration`), enumerated `effect_operation` values (`Add`, `Multiply`, `Override`), and an optional embedded `patternField` sub-schema. Unknown fields are preserved in `unsupported_fields` for deterministic fallback handling.

---

## 4. Migration Mapping

`AbilityCompatMapper` (`engine/core/ability/ability_compat_mapper.h`, S24-T02) provides static compat-to-native mapping:

- `mapMzSkillToNativeAbility()` — Maps an RPG Maker MZ/MV skill JSON object to a native `AuthoredAbilityAsset`. Deterministic cost mapping (mpCost, tpCost, tpGain), effect type inference from `damage.type`, and `_fallback_payload` preservation for unrecognised fields. Returns `AbilityCompatMapResult { AuthoredAbilityAsset ability; bool hadUnmappedFields; std::string warnings; }`.
- `mapMzSkillArrayToNativeAbilities()` — Batch wrapper that maps an array of MZ skill objects and accumulates per-entry warnings.

---

## 5. Diagnostics Integration

- **Execution history**: `AbilitySystemComponent` records bounded deterministic execution history for blocked, executed, and state-machine transition outcomes, including MP before/after, cooldown remaining, and active effect count.
- **Inspector replay log**: `AbilityInspectorPanel::rebuildSnapshot` renders replay-log lines from execution history with sequence IDs, stages, outcomes, state names, and reasons.
- **Structured diagnostics snapshot**: `AbilityInspectorModel::buildDiagnosticsSnapshot` exposes `ability_count`, `active_effect_count`, `active_cooldown_count`, `last_execution_sequence_id`, and per-ability state vectors for diagnostics export and project audit consumption.

---

## 6. Test Coverage

| Layer | Count | Sources |
|-------|-------|---------|
| Unit | 8 + S24 | `test_ability_activation`, `test_ability_e2e`, `test_ability_state_machine`, `test_ability_tasks`, `test_ability_pattern_integration`, `test_ability_inspector`, `test_wave3_gaf`, `test_effect_modifiers`, `test_ability_battle_integration` (S24-T05, 18 tests) |
| Integration | 1 + closure suite | `test_wave1_closure_integration` |

Key cross-subsystem assertions include:
- End-to-end activation → commit → cooldown → effect → inspector coherence.
- State machine windup → impact → recovery transitions with deterministic history.
- WaitInput, WaitEvent, and WaitProjectileCollision task completion, mismatch rejection, timeout behavior, and ASC update-loop ticking.
- Pattern field integration with ability targeting and editor preview.
- Effect stacking policies (refresh and stack) with attribute resolution.

---

## 7. Remaining Residual Gaps (Honest Scope Limits)

1. **Schema contracts (partial closure)**: `gameplay_ability.schema.json` now enforces the `AuthoredAbilityAsset` JSON contract. Activation info, state machine phase schemas, and pattern field schema enforcement remain future work.
2. **Migration mapping (partial closure)**: `AbilityCompatMapper` maps RPG Maker MZ/MV skills to native ability assets with deterministic fallback. Troop actions, item compat, and `AbilityStateMachine` phase shapes are not yet mapped from legacy data.
3. **Gameplay task orchestration depth**: `AbilityTask_WaitInput`, `AbilityTask_WaitEvent`, and `AbilityTask_WaitProjectileCollision` are now implemented and covered with deterministic completion, mismatch, timeout, and ASC update-loop tests. Broader authored battle/map content that composes these tasks into designer-facing ability flows remains future product-depth work.
4. **Scripted condition evaluator**: `activeCondition` and `passiveCondition` strings are intentionally not evaluated in-tree. Non-empty `activeCondition` values fail with `active_condition_unsupported`, and `passiveCondition` is out of runtime scope unless future work deliberately reopens this lane.
5. **Scene-runtime breadth**: live `BattleScene` participant activation plus authored-ability handoff into a mutable `MapScene` player ASC are now landed, but richer async task-driven orchestration beyond the current queue/update-loop scope remains future work.

---

*Sign-off prepared by governance agent. Promotion to `READY` requires human review of the residual gaps above.*
