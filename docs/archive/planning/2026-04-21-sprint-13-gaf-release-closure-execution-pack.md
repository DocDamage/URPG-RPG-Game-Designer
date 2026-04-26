# S13 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the Gameplay Ability Framework to release-grade evidence with an end-to-end activation/commit/cooldown/effect/editor test, exposed runtime/editor diagnostics, and reconciled readiness/signoff artifacts.

**Sprint theme:** gameplay ability framework release closure

**Primary outcomes for this sprint:**
- One end-to-end test proves activation, commit, cooldown, effect application, and editor projection coherence.
- Runtime and editor diagnostics expose structured ability-state snapshots for release-facing observability.
- Readiness records and signoff artifacts are truthful and aligned with the new evidence.

**Non-goals for this sprint:**
- No new ability runtime features beyond diagnostics and test coverage.
- No unrelated subsystem work.
- No promotion to `READY` without human review.

---

## Canonical Sources

Read these before starting `S13-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`

Add sprint-specific sources below before starting implementation:

- `engine/core/ability/gameplay_ability.h`
- `engine/core/ability/gameplay_ability.cpp`
- `engine/core/ability/ability_state_machine.h`
- `engine/core/ability/ability_state_machine.cpp`
- `engine/core/ability/ability_system_component.h`
- `engine/core/ability/pattern_field.h`
- `engine/core/ability/pattern_field.cpp`
- `editor/ability/ability_inspector_model.h`
- `editor/ability/ability_inspector_model.cpp`
- `editor/ability/ability_inspector_panel.h`
- `editor/ability/ability_inspector_panel.cpp`
- `tests/unit/test_ability_activation.cpp`
- `tests/test_ability_state_machine.cpp`
- `tests/test_ability_tasks.cpp`
- `tests/test_ability_pattern_integration.cpp`
- `tests/test_ability_inspector.cpp`
- `tests/unit/test_wave3_gaf.cpp`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation.
- Unsupported source behavior must become structured diagnostics or explicit waivers, never silent loss.
- Do not widen sprint scope by "cleaning up nearby code" unless the cleanup is necessary for the current ticket to compile or test.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open the paired sprint task board.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [ ] Run the sprint preflight command block.

Suggested preflight:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### S13-T01: End-to-End Release-Grade Ability Flow Test

**Goal:** Add one end-to-end test proving activation, commit, cooldown, effect application, and editor projection stay coherent across the full ability lifecycle.

**Primary files:**
- `tests/unit/test_ability_activation.cpp`
- `tests/test_ability_state_machine.cpp`
- `tests/test_ability_tasks.cpp`
- `tests/test_ability_pattern_integration.cpp`
- `tests/test_ability_inspector.cpp`
- `tests/unit/test_wave3_gaf.cpp`

**Implementation targets:**
- Create a new focused test case (or extend an existing one) that exercises:
  1. Ability activation with structured check result.
  2. `commitAbility` deducting MP and starting cooldown.
  3. Cooldown preventing re-activation until elapsed.
  4. Gameplay effect application (modifier applied, attribute changed).
  5. State machine transitions (windup → impact → recovery) recording execution history.
  6. Inspector model/panel snapshot coherence after each stage.
- Ensure the test is tagged `[ability][e2e]` and registered in `CMakeLists.txt` if needed.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ability][e2e]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[ability]" --reporter compact
```

**Done when:**
- The new test passes and covers the full lifecycle.
- Existing ability tests remain green.

---

### S13-T02: Runtime/Editor Diagnostics for Release-Facing Closure

**Goal:** Expose structured runtime and editor diagnostics so the ability lane can be observed in diagnostics exports and project audit.

**Primary files:**
- `engine/core/ability/ability_system_component.h`
- `editor/ability/ability_inspector_model.h`
- `editor/ability/ability_inspector_model.cpp`
- `editor/ability/ability_inspector_panel.h`
- `editor/ability/ability_inspector_panel.cpp`
- `tests/test_ability_inspector.cpp`

**Implementation targets:**
- Add `AbilityDiagnosticsSnapshot` struct to `ability_inspector_model.h` carrying:
  - `ability_count`, `active_effect_count`, `active_cooldown_count`
  - `last_execution_sequence_id`
  - `ability_states` array (id, can_activate, cooldown_remaining, blocking_reason)
  - `active_effects` array (id, duration, elapsed, stack_count)
- Add `AbilityInspectorModel::buildDiagnosticsSnapshot(const AbilitySystemComponent&)` method.
- Add `AbilityInspectorPanel::getDiagnosticsSnapshot()` accessor.
- Update `tests/test_ability_inspector.cpp` to assert the diagnostics snapshot shape.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[ability][editor]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[ability]" --reporter compact
```

**Done when:**
- Diagnostics snapshot is reachable from tests and carries structured data.
- Inspector panel does not silently drop ability-state context.

---

### S13-T03: Readiness Wording and Signoff Evidence Reconciliation

**Goal:** Reconcile readiness claims and produce a signoff artifact so the GAF lane is truthfully represented.

**Primary files:**
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `docs/GAF_CLOSURE_SIGNOFF.md` (new)

**Implementation targets:**
- Create `docs/GAF_CLOSURE_SIGNOFF.md` following the pattern of `BATTLE_CORE_CLOSURE_SIGNOFF.md` and `SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`.
- Update `readiness_status.json`:
  - Set `diagnostics` evidence to `true` for `gameplay_ability_framework`.
  - Update `mainGaps` to reflect closed diagnostics gap and remaining honest scope limits.
- Update `RELEASE_READINESS_MATRIX.md` row for `gameplay_ability_framework` to reflect diagnostics closure.
- Update `PROGRAM_COMPLETION_STATUS.md` with Sprint 13 slice note.

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- Signoff artifact exists and follows the established pattern.
- Readiness gates pass.
- Canonical docs are truthful.

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

If a command is too expensive or blocked, record the reason in the task board and final sprint note.

---

## Commit Rhythm

Preferred commit shape:

1. tests that expose the missing behavior
2. implementation
3. docs/readiness/worklog alignment

Suggested commit subject style:

- `test: add <ticket> regression coverage`
- `feat: implement <ticket behavior>`
- `docs: reconcile sprint status and readiness evidence`

---

## Resume Protocol

When handing off to a new LLM session:

1. Update the task board `Resume From Here` section.
2. Record the current ticket status and exact next command.
3. Keep the touched-files list current.
4. Leave blockers explicit, even if the blocker is "full suite too expensive right now".

Use the handoff template in:

- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

---

## Acceptance Summary

This sprint is complete only when all of the following are true:

- every ticket is `DONE` or explicitly moved out of sprint with rationale
- targeted verification passed or is explicitly waived with reason
- canonical docs and readiness records are aligned with the landed changes
- `WORKLOG.md` records the sprint slices
