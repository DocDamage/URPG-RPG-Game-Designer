# Gameplay Ability Framework — Release Closure Sign-off

> **Status:** `PARTIAL`  
> **Purpose:** Evidence-gathering artifact for GAF release closure review.  
> **Date:** 2026-04-21  
> **Rule:** This document does **not** promote `readiness_status.json` status. Human review is required for promotion to `READY`.

---

## 1. Runtime Ownership

The Gameplay Ability Framework provides deterministic, native-owned ability execution, tag-based gating, effect stacking, and cooldown management:

| Component | Responsibility | Evidence |
|-----------|---------------|----------|
| `GameplayAbility` | Base class for executable actions with tag-based activation checks, explicit unsupported scripted-condition diagnostics, structured failure reasons, MP cost resolution, and cooldown commit | `engine/core/ability/gameplay_ability.h` |
| `AbilitySystemComponent` | Tag/effect/cooldown owner, attribute modification with additive/multiplicative/override stacking, and bounded deterministic execution history | `engine/core/ability/ability_system_component.h` |
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

No native JSON schema is enforced for ability definitions under `content/schemas/` at this time. The GAF relies on header-defined C++ structures and in-memory JSON serialization for pattern fields. Schema definition and validation are tracked as residual gaps.

---

## 4. Migration Mapping

No compat-to-native migration mapping exists for RPG Maker MV/MZ skills or items into the GAF shape. The GAF is a native-first advanced capability and does not yet provide an upgrader from legacy plugin-shaped ability systems. This is tracked as residual gap.

---

## 5. Diagnostics Integration

- **Execution history**: `AbilitySystemComponent` records bounded deterministic execution history for blocked, executed, and state-machine transition outcomes, including MP before/after, cooldown remaining, and active effect count.
- **Inspector replay log**: `AbilityInspectorPanel::rebuildSnapshot` renders replay-log lines from execution history with sequence IDs, stages, outcomes, state names, and reasons.
- **Structured diagnostics snapshot**: `AbilityInspectorModel::buildDiagnosticsSnapshot` exposes `ability_count`, `active_effect_count`, `active_cooldown_count`, `last_execution_sequence_id`, and per-ability state vectors for diagnostics export and project audit consumption.

---

## 6. Test Coverage

| Layer | Count | Sources |
|-------|-------|---------|
| Unit | 8 | `test_ability_activation`, `test_ability_e2e`, `test_ability_state_machine`, `test_ability_tasks`, `test_ability_pattern_integration`, `test_ability_inspector`, `test_wave3_gaf`, `test_effect_modifiers` |
| Integration | 1 + closure suite | `test_wave1_closure_integration` |

Key cross-subsystem assertions include:
- End-to-end activation → commit → cooldown → effect → inspector coherence.
- State machine windup → impact → recovery transitions with deterministic history.
- Pattern field integration with ability targeting and editor preview.
- Effect stacking policies (refresh and stack) with attribute resolution.

---

## 7. Remaining Residual Gaps (Honest Scope Limits)

1. **Schema contracts**: No canonical JSON schema exists for ability definitions, activation info, or state machine phases. Pattern field has JSON serialization but no schema enforcement.
2. **Migration mapping**: No compat-to-native upgrader maps RPG Maker MV/MZ skills, items, or troop actions into `GameplayAbility` / `AbilityStateMachine` shapes.
3. **Gameplay task backend**: `AbilityTask` and `AbilityTask_WaitTime` exist as async task scaffolding, but deeper task backends (input-wait, event-wait, projectile collision) are not yet implemented.
4. **Scripted condition evaluator**: `activeCondition` and `passiveCondition` strings are intentionally not evaluated in-tree. Non-empty `activeCondition` values fail with `active_condition_unsupported`, and `passiveCondition` is out of runtime scope unless future work deliberately reopens this lane.
5. **Real gameplay loop integration**: The GAF is exercised through unit tests and inspector snapshots, but integration into a live `BattleScene` or `MapScene` ability command queue is future work.

---

*Sign-off prepared by governance agent. Promotion to `READY` requires human review of the residual gaps above.*
