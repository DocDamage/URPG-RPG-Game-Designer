# Sprint 05 Task Board

> Keep this file short and current. It is the sprint control surface for handoff between LLM sessions.

## Status Legend

- `TODO`
- `IN PROGRESS`
- `BLOCKED`
- `DONE`

## Resume From Here

- Current ticket: `SPRINT 05 CLOSED`
- Last known green command: `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure`, `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1`, and `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1`
- First failing command: `none yet`
- Touched files: `tests/compat/test_compat_plugin_fixtures.cpp`, `tests/compat/test_compat_plugin_failure_diagnostics.cpp`, `docs/COMPAT_EXIT_CHECKLIST.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, `WORKLOG.md`, `docs/superpowers/plans/2026-04-21-sprint-05-compat-hardening-task-board.md`, `docs/superpowers/plans/2026-04-21-sprint-05-compat-hardening-execution-pack.md`
- Notes: `Sprint 05 is closed. The compat maintenance lane now has both a directory re-import corpus-depth anchor and a by-name dependency-gating failure-parity anchor, and the weekly, PR, and governance lanes are green on the current tree.`

## Ticket Board

| ID | Title | Status | Depends On | Primary Files | Verification |
|---|---|---|---|---|---|
| S05-T01 | Compat hardening: curated corpus depth follow-through | DONE | none | `tests/compat/*`, `docs/COMPAT_EXIT_CHECKLIST.md`, canonical status docs | focused compat tests and `ctest -L weekly` or equivalent local compat lane |
| S05-T02 | Compat hardening: new failure-path parity follow-through | DONE | S05-T01 | compat diagnostics/report/panel surfaces and tests | focused compat failure diagnostics tests |
| S05-T03 | Compat truth-maintenance and sprint closeout | DONE | S05-T02 | canonical status docs, checklist, sprint artifacts | `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` plus relevant compat/governance lanes |

## Definition of Done Per Ticket

- Tests or validation updated for the claimed behavior
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
- [x] `docs/PROGRAM_COMPLETION_STATUS.md` is aligned.
- [x] `docs/COMPAT_EXIT_CHECKLIST.md` is aligned.
- [x] `WORKLOG.md` records the sprint slices.
- [x] Final validation commands were run or explicitly waived with reason.
