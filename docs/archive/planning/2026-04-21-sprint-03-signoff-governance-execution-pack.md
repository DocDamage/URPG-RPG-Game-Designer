# Sprint 03 Signoff Governance Execution Pack

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans`. Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** turn the remaining release-signoff discipline gaps into explicit governed artifacts instead of leaving them as scattered prose.

**Sprint theme:** compat bridge exit signoff and human-review-gated readiness discipline.

**Primary outcomes for this sprint:**
- add an explicit compat bridge exit signoff artifact parallel to the existing Wave 1 closure signoff docs
- wire signoff artifacts into readiness/truth checks where the canonical docs depend on human review
- reconcile remaining compat and governance wording so signoff-required lanes are explicit, conservative, and resumable

**Non-goals for this sprint:**
- no reopening closed Wave 1 implementation lanes
- no promotion of `compat_bridge_exit` or `governance_foundation` to `READY` without real human approval
- no speculative new template/product feature epics

---

## Canonical Sources

Read these before starting Ticket `S03-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TRUTH_ALIGNMENT_RULES.md`
- `docs/COMPAT_EXIT_CHECKLIST.md`
- `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`
- `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`
- `content/readiness/readiness_status.json`

Reference implementation surfaces:

- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `docs/PROJECT_AUDIT.md`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Update docs in the same change as governance/script changes.
- Human-review-gated status language must stay conservative. A signoff artifact records evidence and residual gaps; it does not by itself promote a lane to `READY`.
- Do not check human signoff boxes on behalf of real owners.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open `docs/superpowers/plans/2026-04-21-sprint-03-signoff-governance-task-board.md`.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [x] Run the preflight command block:

```powershell
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
```

---

## Ticket Order

### S03-T01 Compat bridge exit: formal signoff artifact and readiness linkage

**Goal:** add a governed signoff artifact for `compat_bridge_exit` and connect it to the readiness truth surfaces.

**Primary files:**
- `docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/COMPAT_EXIT_CHECKLIST.md`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

### S03-T02 Signoff governance: matrix/rule enforcement for human-review-gated lanes

**Goal:** make human-review-required readiness language enforceable where canonical docs already rely on signoff-based promotion rules.

**Primary files:**
- readiness rules/docs/scripts discovered during implementation

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

### S03-T03 Compat truth-maintenance follow-through and sprint closeout

**Goal:** reconcile status/docs/checklists and close the sprint cleanly after the signoff-governance slice lands.

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
