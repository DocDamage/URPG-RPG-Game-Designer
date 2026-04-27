# S20 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the mod registry diagnostics gap with a bounded validation runtime, a focused CI gate script, and reconciled governance.

**Sprint theme:** mod registry validation pipeline

**Primary outcomes for this sprint:**
- A real `ModRegistryValidator` runtime checks bounded manifest-quality rules.
- `ModManagerPanel` surfaces validator-backed diagnostics in its snapshot.
- A focused CI gate script validates the mod artifact contract.
- ProjectAudit governance and readiness docs reflect the validation evidence.

**Non-goals for this sprint:**
- No live mod loading implementation.
- No script sandbox runtime.
- No mod marketplace/distribution features.

---

## Canonical Sources

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `engine/core/mod/mod_registry.h`
- `engine/core/mod/mod_registry.cpp`
- `editor/mod/mod_manager_panel.cpp`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_mod_registry.cpp`
- `tests/unit/test_mod_manager_panel.cpp`

---

## Ticket Order

### S20-T01: Mod Registry Validator Runtime + Panel Diagnostics

- Build `ModRegistryValidator`.
- Surface validation issues through `ModManagerPanel`.
- Add focused mod validator and panel tests.

### S20-T02: CI Gate Script for Canonical Mod Governance Fixture

- Add `tools/ci/check_mod_governance.ps1`.
- Add `content/fixtures/mod_manifest_fixture.json`.
- Add a PowerShell-invocation test and wire the script into `tools/ci/run_local_gates.ps1`.

### S20-T03: Thread Evidence into ProjectAudit/Governance

- Add `modArtifacts` + `modArtifactIssueCount` to `urpg_project_audit`.
- Update ProjectAudit contract tests and panel parsing.
- Reconcile readiness/docs/worklog status.
