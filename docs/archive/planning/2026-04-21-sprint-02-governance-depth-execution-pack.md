# Sprint 02 Governance Depth Execution Pack

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans`. Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** deepen the governance/template-readiness lane so product-facing audit and readiness claims are backed by concrete cross-cutting evidence rather than first-slice scaffolding.

**Sprint theme:** turn the current governance-foundation baseline into richer project-audit and readiness-gate proof.

**Primary outcomes for this sprint:**
- expand `ProjectAudit` to report cross-cutting governance gaps for accessibility, audio, and performance
- keep diagnostics/export snapshots aligned with the richer audit contract
- harden readiness CI so the deeper governance evidence is enforced instead of advisory only
- reconcile canonical docs and readiness records to the new governance depth honestly

**Non-goals for this sprint:**
- no reopening closed Wave 1 subsystem implementation slices
- no new broad template feature epics
- no relabeling a subsystem or template as `READY` without passing the tightened evidence bars
- no speculative refactors outside the listed governance files

---

## Canonical Sources

Read these before starting Ticket `S02-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `docs/TEMPLATE_LABEL_RULES.md`
- `content/readiness/readiness_status.json`

Reference implementation surfaces:

- `tools/audit/urpg_project_audit.cpp`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation. At minimum keep these aligned when a ticket lands:
  - `docs/PROGRAM_COMPLETION_STATUS.md`
  - `docs/PROGRAM_COMPLETION_STATUS.md`
  - `docs/RELEASE_READINESS_MATRIX.md`
  - `docs/TEMPLATE_READINESS_MATRIX.md`
  - `content/readiness/readiness_status.json`
  - `WORKLOG.md`
- Governance findings must stay evidence-backed. If a required artifact or bar is only partially represented, record it as a structured audit result instead of optimistic prose.
- Do not widen sprint scope by starting new template gameplay implementation.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open `docs/superpowers/plans/2026-04-21-sprint-02-task-board.md`.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [x] Run the preflight command block:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
```

---

## Ticket Order

### S02-T01 Project audit: cross-cutting bar governance checks

**Goal:** expand project-audit output so accessibility, audio, and performance bars produce concrete governance findings instead of remaining implicit in matrix prose.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tests/unit/test_project_audit_panel.cpp`

**Implementation targets:**
- add explicit audit sections for accessibility, audio, and performance governance artifacts
- make summary/output counts include those new sections
- preserve stable JSON contracts for diagnostics consumers

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "project_audit"
```

**Done when:**
- audit output names the new governance sections explicitly
- focused CLI/panel tests cover the richer contract
- canonical docs narrow the governance-foundation remaining gap honestly

---

### S02-T02 Project audit: diagnostics/export parity for richer governance snapshots

**Goal:** keep diagnostics workspace exports and active-tab snapshots aligned with the deeper audit contract.

**Primary files:**
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_panel.cpp`

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "project_audit|DiagnosticsWorkspace"
```

---

### S02-T03 Release-readiness enforcement: governance-depth CI checks

**Goal:** turn the richer governance evidence into enforced release-readiness checks instead of informational output only.

**Primary files:**
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- readiness/docs surfaces discovered during implementation

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

### S02-T04 Governance/template truth reconciliation and sprint closeout

**Goal:** align the canonical docs, readiness records, and sprint control files with the new governance depth and close the sprint cleanly.

**Primary files:**
- canonical status docs
- `content/readiness/readiness_status.json`
- sprint artifacts
- `WORKLOG.md`

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
