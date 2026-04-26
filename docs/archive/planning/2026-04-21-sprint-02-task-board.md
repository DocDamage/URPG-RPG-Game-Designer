# Sprint 02 Task Board

> Keep this file short and current. It is the sprint control surface for handoff between LLM sessions.

## Status Legend

- `TODO`
- `IN PROGRESS`
- `BLOCKED`
- `DONE`

## Resume From Here

- Current ticket: `SPRINT 02 CLOSED`
- Last known green command: `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure`, `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1`, and `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1`
- First failing command: `none yet`
- Touched files: `tools/audit/urpg_project_audit.cpp`, `editor/diagnostics/project_audit_panel.h`, `editor/diagnostics/project_audit_panel.cpp`, `editor/diagnostics/diagnostics_workspace.cpp`, `tests/unit/test_project_audit_cli.cpp`, `tests/unit/test_project_audit_panel.cpp`, `tests/unit/test_diagnostics_workspace.cpp`, `tools/ci/check_release_readiness.ps1`, `tools/ci/truth_reconciler.ps1`, `docs/PROJECT_AUDIT.md`, `docs/RELEASE_READINESS_MATRIX.md`, `docs/TRUTH_ALIGNMENT_RULES.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`, `content/readiness/readiness_status.json`, `WORKLOG.md`, `docs/superpowers/plans/2026-04-21-sprint-02-task-board.md`, `docs/superpowers/plans/2026-04-21-sprint-02-governance-depth-execution-pack.md`
- Notes: `Sprint 02 is closed. Governance-depth project-audit coverage, diagnostics/export parity, and readiness/truth enforcement all passed their focused validation lanes and the PR lane is green.`

## Ticket Board

| ID | Title | Status | Depends On | Primary Files | Verification |
|---|---|---|---|---|---|
| S02-T01 | Project audit: cross-cutting bar governance checks | DONE | none | `tools/audit/urpg_project_audit.cpp`, `editor/diagnostics/project_audit_panel.*`, project audit tests | `.\build\dev-ninja-debug\urpg_tests.exe "[project_audit]" --reporter compact` and `.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact` |
| S02-T02 | Project audit: diagnostics/export parity for richer governance snapshots | DONE | S02-T01 | `editor/diagnostics/diagnostics_workspace.cpp`, diagnostics tests | `.\build\dev-ninja-debug\urpg_tests.exe "[project_audit],[editor][diagnostics][integration][project_audit]" --reporter compact` |
| S02-T03 | Release-readiness enforcement: governance-depth CI checks | DONE | S02-T02 | `tools/ci/check_release_readiness.ps1`, `tools/ci/truth_reconciler.ps1`, readiness/docs | `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` and `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` |
| S02-T04 | Governance/template truth reconciliation and sprint closeout | DONE | S02-T03 | canonical docs, readiness records, sprint artifacts | `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` and governance gates |

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
