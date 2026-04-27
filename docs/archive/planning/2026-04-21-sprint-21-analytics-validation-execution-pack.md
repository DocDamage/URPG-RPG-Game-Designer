# S21 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the analytics dispatcher diagnostics gap with a bounded validation runtime, a focused CI gate script, and reconciled governance.

**Sprint theme:** analytics dispatcher validation pipeline

**Primary outcomes for this sprint:**
- A real analytics validator checks bounded dispatcher/config event-quality rules.
- `AnalyticsPanel` surfaces validator-backed diagnostics in its snapshot.
- A focused CI gate script validates the analytics artifact contract.
- ProjectAudit governance and readiness docs reflect the validation evidence.

**Non-goals for this sprint:**
- No telemetry upload implementation.
- No cross-session aggregation service.
- No privacy workflow beyond bounded governance evidence.

---

## Canonical Sources

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `engine/core/analytics/analytics_dispatcher.h`
- `engine/core/analytics/analytics_dispatcher.cpp`
- `editor/analytics/analytics_panel.cpp`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_analytics_dispatcher.cpp`
- `tests/unit/test_analytics_panel.cpp`

---

## Ticket Order

### S21-T01: Analytics Validator Runtime + Panel Diagnostics

- Build a bounded analytics validator/runtime diagnostic slice.
- Surface validation issues through `AnalyticsPanel`.
- Add focused analytics validator and panel tests.

### S21-T02: CI Gate Script for Canonical Analytics Governance Fixture

- Add a focused analytics governance script.
- Add a canonical analytics fixture.
- Add a PowerShell-invocation test and wire the script into `tools/ci/run_local_gates.ps1`.

### S21-T03: Thread Evidence into ProjectAudit/Governance

- Add analytics governance artifacts and issue-count tracking to `urpg_project_audit`.
- Update ProjectAudit contract tests and panel parsing.
- Reconcile readiness/docs/worklog status.

## Swarm Split

- Worker 1 owns S21-T01 and S21-T02 plus any analytics runtime, panel, validator, fixture, and governance-script files.
- Worker 2 owns S21-T03 only and is limited to the audit/readiness/docs surface.
- Final closeout depends on both workers landing without reverting each other.

---

## Sprint-End Validation

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[analytics]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_analytics_governance.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
```

If the validator/governance files are still in flight from worker 1, record the blocker instead of overclaiming a green closeout.
