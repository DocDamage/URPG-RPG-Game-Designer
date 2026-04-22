# Sprint 04 Release Signoff Execution Pack

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans`. Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** move governance_foundation beyond scattered artifact and signoff checks by giving the repo a canonical release-signoff workflow and the first enforcement around it.

**Sprint theme:** explicit release-signoff workflow discipline.

**Primary outcomes for this sprint:**
- add a canonical release-signoff workflow artifact for the governed readiness/signoff stack
- wire that workflow artifact into readiness/truth checks
- reconcile governance_foundation wording so the remaining gap narrows honestly

**Non-goals for this sprint:**
- no promotion of `governance_foundation` to `READY`
- no fake human approvals or machine-written signoff claims
- no reopening closed Sprint 02/03 governance slices except where the release-signoff workflow needs to reference them

---

## Canonical Sources

Read these before starting Ticket `S04-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TRUTH_ALIGNMENT_RULES.md`
- `docs/PROJECT_AUDIT.md`
- `content/readiness/readiness_status.json`

Reference implementation surfaces:

- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Keep human approval out of automation. Workflow artifacts may describe required review steps, but they must not claim those steps happened unless a real owner did them.
- Update docs in the same change as governance/script edits.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open `docs/superpowers/plans/2026-04-21-sprint-04-release-signoff-task-board.md`.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [ ] Run the preflight command block:

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

## Ticket Order

### S04-T01 Governance foundation: canonical release-signoff workflow artifact

**Goal:** add a governed release-signoff workflow artifact and connect it to the readiness/truth chain.

**Primary files:**
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- canonical status docs

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Status:** DONE 2026-04-21. `docs/RELEASE_SIGNOFF_WORKFLOW.md` is now present, and both governance gates require its date-aligned, non-promoting workflow language.

---

### S04-T02 Project audit / governance surfaces: release-signoff workflow parity

**Goal:** keep project-audit/governance surfaces aligned with the new release-signoff workflow artifact where appropriate.

**Status:** DONE 2026-04-21. The audit CLI, panel snapshot, diagnostics export, and governance gates now keep the release-signoff workflow artifact in parity.

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

### S04-T03 Release-signoff follow-through and sprint closeout

**Goal:** align status/docs and close the sprint cleanly once the release-signoff workflow slice lands.

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Status:** DONE 2026-04-21. Sprint closeout validation is green on the current tree.
