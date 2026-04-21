# Sprint 01 Task Board

> Keep this file short and current. It is the sprint control surface for handoff between LLM sessions.

## Status Legend

- `TODO`
- `IN PROGRESS`
- `BLOCKED`
- `DONE`

## Resume From Here

- Current ticket: `SPRINT 01 CLOSED`
- Last known green command: `ctest --preset dev-all --output-on-failure`, `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure`, `powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1`, `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1`, and `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1`
- First failing command: `none`
- Touched files: `engine/core/battle/battle_migration.h`, `engine/core/save/save_migration.h`, `engine/core/save/save_migration.cpp`, `engine/core/editor/plugin_api.h`, `engine/core/editor/plugin_api.cpp`, `editor/diagnostics/migration_wizard_model.h`, `tests/unit/test_battle_migration.cpp`, `tests/unit/test_save_migration.cpp`, `tests/unit/test_save_runtime.cpp`, `tests/unit/test_audio_manager.cpp`, `tests/unit/test_battlemgr.cpp`, `tests/unit/test_plugin_api.cpp`, `tests/unit/test_window_compat.cpp`, `tests/integration/test_integration_runtime_recovery.cpp`, `tools/ci/check_release_readiness.ps1`, `tools/ci/truth_reconciler.ps1`, `content/readiness/readiness_status.json`, `docs/COMPAT_EXIT_CHECKLIST.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/RELEASE_READINESS_MATRIX.md`, `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`, `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`, `docs/SCHEMA_CHANGELOG.md`, `docs/SUBSYSTEM_STATUS_RULES.md`, `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`, `docs/TEMPLATE_LABEL_RULES.md`, `docs/TEMPLATE_READINESS_MATRIX.md`, `docs/TRUTH_ALIGNMENT_RULES.md`, `WORKLOG.md`, `docs/superpowers/plans/2026-04-21-sprint-01-closure-execution-pack.md`, `docs/superpowers/plans/2026-04-21-sprint-01-task-board.md`
- Notes: `Sprint 01 is closed. Root-level 'ctest -L pr' reported no tests in this workspace layout, so the equivalent build-dir command ('ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure') was used for closeout evidence.`

## Ticket Board

| ID | Title | Status | Depends On | Primary Files | Verification |
|---|---|---|---|---|---|
| S01-T01 | Battle compat migration: boolean condition trees | DONE | none | `engine/core/battle/battle_migration.h`, `tests/unit/test_battle_migration.cpp` | `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "battle_migration\|BattleMigration"` |
| S01-T02 | Battle compat migration: remaining event-command coverage | DONE | S01-T01 | `engine/core/battle/battle_migration.h`, `tests/unit/test_battle_migration.cpp` | `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "battle_migration\|BattleMigration"` |
| S01-T03 | Save/Data end-to-end compat import closure | DONE | S01-T02 | `engine/core/save/save_migration.h`, `engine/core/save/save_runtime.h`, save tests | `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "save_migration\|save\|integration.*save\|runtime_recovery"` |
| S01-T04 | Compat exit checklist evidence closure | DONE | S01-T03 | `docs/COMPAT_EXIT_CHECKLIST.md`, compat tests, diagnostics workspace | `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "compat\|audio_manager\|battlemgr\|data_manager\|window_compat\|DiagnosticsWorkspace"` |
| S01-T05 | Replace stubbed PluginAPI bridge with live engine wiring | DONE | S01-T04 | `engine/core/editor/plugin_api.h`, `engine/core/editor/plugin_api.cpp`, `tests/unit/test_plugin_api.cpp` | `.\build\dev-ninja-debug\urpg_tests.exe "[plugin_api]" --reporter compact` and `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "plugin_api\|global_state\|input"` |
| S01-T06 | Release-readiness enforcement hardening | DONE | S01-T05 | `tools/ci/check_release_readiness.ps1`, `tools/ci/truth_reconciler.ps1`, readiness/docs | `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` and `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` |

## Definition of Done Per Ticket

- Tests added or updated for the claimed behavior
- Targeted verification command passes
- Canonical docs updated in the same change
- Task status updated here
- Resume note updated for the next session

## Blocker Log

| Date | Ticket | Blocker | Next Action |
|---|---|---|---|
| none | none | none | none |

## Sprint Closeout Checklist

- [x] All tickets are `DONE` or explicitly moved out of sprint with rationale.
- [x] `docs/PROGRAM_COMPLETION_STATUS.md` is aligned.
- [x] `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` is aligned.
- [x] `content/readiness/readiness_status.json` is aligned.
- [x] `WORKLOG.md` records the sprint slices.
- [x] Final validation commands were run or explicitly waived with reason.
