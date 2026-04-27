# Sprint 01 Closure Execution Pack

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans`. Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Execute one full sprint focused on the highest-value remaining closure work without reopening broad roadmap scope.

**Sprint theme:** turn the current `PARTIAL` Wave 1 and governance leftovers into evidence-backed, resumable implementation work.

**Primary outcomes for this sprint:**
- finish the remaining Battle compat migration gaps that block honest promotion
- complete the Save/Data end-to-end compat import closure work
- close the unchecked compat exit evidence items that still require code or tests
- replace the stubbed `PluginAPI` bridge with real engine wiring for ECS, global state, and input
- harden release-readiness enforcement so future `READY` claims are evidence-gated

**Non-goals for this sprint:**
- no new template epics
- no broad Wave 2 expansion
- no speculative refactors outside the listed tickets
- no relabeling a subsystem as `READY` without passing the listed evidence gates

---

## Canonical Sources

Read these before starting Ticket S01-T01:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `docs/COMPAT_EXIT_CHECKLIST.md`
- `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`
- `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`
- `content/readiness/readiness_status.json`

Reference implementation surfaces:

- `engine/core/battle/battle_migration.h`
- `engine/core/save/save_migration.h`
- `engine/core/save/save_runtime.h`
- `engine/core/editor/plugin_api.h`
- `engine/core/editor/plugin_api.cpp`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation. At minimum keep these aligned when a ticket lands:
  - `docs/PROGRAM_COMPLETION_STATUS.md`
  - `docs/PROGRAM_COMPLETION_STATUS.md`
  - `docs/COMPAT_EXIT_CHECKLIST.md` when compat evidence changes
  - `content/readiness/readiness_status.json` when subsystem status or main gaps change
  - `WORKLOG.md`
- Unsupported source behavior must become structured diagnostics or explicit waivers, never silent loss.
- Do not widen sprint scope by “cleaning up nearby code” unless the cleanup is necessary for the current ticket to compile or test.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open `docs/superpowers/plans/2026-04-21-sprint-01-task-board.md`.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [x] Run the preflight command block:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest --preset dev-all --output-on-failure
```

If the full suite is too expensive before the first code change, run at minimum:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### S01-T01 Battle compat migration: boolean condition trees

**Goal:** preserve source troop-page logic with explicit `AND`/`OR` semantics instead of flattening or dropping branches.

**Primary files:**
- `engine/core/battle/battle_migration.h`
- `tests/unit/test_battle_migration.cpp`
- `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`

**Implementation targets:**
- add a native representation for grouped conditions
- preserve precedence and nesting during migration
- emit typed warnings when exact representation is impossible

**Suggested test additions:**
- single leaf condition remains unchanged
- `AND` group migration
- `OR` group migration
- nested `AND`/`OR` group migration
- unsupported tree shape emits warning plus fallback record

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "battle_migration|BattleMigration"
```

**Done when:**
- no source condition branch is silently dropped
- migration diagnostics identify lossy fallbacks
- battle migration tests pass

---

### S01-T02 Battle compat migration: remaining event-command coverage

**Goal:** map remaining high-value RPG Maker troop event commands or emit explicit unsupported-command artifacts.

**Primary files:**
- `engine/core/battle/battle_migration.h`
- `tests/unit/test_battle_migration.cpp`
- `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`

**Implementation targets:**
- enumerate remaining unmapped command codes used by the curated corpus
- map each to a native effect or structured unsupported placeholder
- carry command code and reason through diagnostics

**Edge cases to cover:**
- mixed supported and unsupported commands in one page
- nested command lists
- commands that mutate both battle and global state

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "battle_migration|BattleMigration"
```

**Done when:**
- every known command code ends in a native mapping or explicit unsupported record
- docs narrow the residual gap honestly

---

### S01-T03 Save/Data end-to-end compat import closure

**Goal:** import real MV/MZ save payloads into native runtime-owned save state, not just metadata upgrades.

**Primary files:**
- `engine/core/save/save_migration.h`
- `engine/core/save/save_runtime.h`
- corresponding `.cpp` files if present
- `tests/unit/test_save_migration.cpp`
- `tests/integration/test_battle_save_integration.cpp`
- `tests/integration/test_integration_runtime_recovery.cpp`
- `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`

**Implementation targets:**
- parse compat save payloads into native save/runtime structures
- preserve unknown plugin-owned data under explicit compat payload retention
- ensure imported saves can load through the normal native runtime path

**Required diagnostics:**
- corrupted but recoverable input
- missing database references
- unsupported plugin blobs
- lossy field mapping

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "save_migration|save|integration.*save|runtime_recovery"
```

**Done when:**
- at least one real compat fixture imports and loads successfully
- import diagnostics capture every lossy conversion
- imported data survives native re-save/reload without structural corruption

---

### S01-T04 Compat exit checklist evidence closure

**Goal:** turn the remaining unchecked items in `docs/COMPAT_EXIT_CHECKLIST.md` into passing evidence or explicit waivers.

**Primary files:**
- `docs/COMPAT_EXIT_CHECKLIST.md`
- `tests/compat/test_compat_plugin_fixtures.cpp`
- `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- `tests/unit/test_audio_manager.cpp`
- `tests/unit/test_battlemgr.cpp`
- `tests/unit/test_data_manager.cpp`
- `tests/unit/test_window_compat.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`

**Implementation targets:**
- add missing regression anchors for compat subsystems with planning weight
- verify export surfaces match what panels actually render
- ensure partial harness behavior is described consistently in tests and docs

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "compat|audio_manager|battlemgr|data_manager|window_compat|DiagnosticsWorkspace"
```

**Done when:**
- each unchecked checklist item is either checked with evidence or converted into an explicit, justified waiver

---

### S01-T05 Replace stubbed PluginAPI bridge with live engine wiring

**Goal:** remove the current scratch-state/synthetic-id behavior from the editor plugin bridge.

**Primary files:**
- `engine/core/editor/plugin_api.h`
- `engine/core/editor/plugin_api.cpp`
- related ECS and input files discovered during implementation
- `tests/unit/test_plugin_api.cpp`

**Implementation targets:**
- route entity creation/destruction to the live ECS world
- route switch/variable reads and writes to `GlobalStateHub`
- route input queries to the active input manager

**Hard constraints:**
- define clear invalid-handle behavior
- do not introduce hidden thread-affinity violations
- preserve truthful comments and status language

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[plugin_api]" --reporter compact
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "plugin_api|global_state|input"
```

**Done when:**
- tests prove PluginAPI is no longer placeholder-backed for the claimed surfaces
- header comments are updated to match real behavior

---

### S01-T06 Release-readiness enforcement hardening

**Goal:** make `READY` and closure language evidence-gated in CI and docs, not descriptive prose.

**Primary files:**
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `content/readiness/readiness_status.json`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`

**Implementation targets:**
- fail CI when `READY` lacks required evidence fields
- fail CI when status docs claim closure while readiness records remain contradictory
- require signoff artifacts where the repo already uses signoff-based promotion language

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- contradictory readiness/doc states fail locally
- sprint tickets can only promote status through evidence-backed updates

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
ctest --preset dev-all --output-on-failure
```

If a command is too expensive or blocked, record the reason in the task board and final sprint note.

---

## Commit Rhythm

Preferred commit shape:

1. tests that expose the missing behavior
2. implementation
3. docs/readiness/worklog alignment

Suggested commit subject style:

- `test: add battle migration or-condition coverage`
- `feat: preserve grouped troop page conditions during migration`
- `feat: import compat save payloads into native runtime state`
- `feat: wire plugin api to live ecs and input state`
- `ci: enforce evidence-gated readiness promotion`

---

## Resume Protocol

When handing off to a new LLM session:

- update the paired task board status
- add a short “resume from here” note at the top of the task board
- list the last green command that ran
- list the first failing command that still needs work
- list touched files
- list docs already updated

Use the handoff template in:

- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

---

## Acceptance Summary

This sprint is complete only when all of the following are true:

- Battle compat migration no longer drops grouped boolean logic silently.
- Save/Data compat import is end-to-end through native runtime load behavior.
- Compat exit checklist remaining items are closed by evidence or explicit waivers.
- PluginAPI is live-wired for the surfaces it claims to expose.
- Readiness enforcement is stronger at the CI/document layer than it was at sprint start.
- Canonical docs and readiness records are aligned with what actually landed.
