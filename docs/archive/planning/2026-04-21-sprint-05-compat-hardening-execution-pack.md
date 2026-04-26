# Sprint 05 Compat Hardening Execution Pack

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans`. Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** turn post-closure compat hardening into a concrete maintenance sprint with explicit corpus-depth, failure-parity, and truth-maintenance tickets.

**Sprint theme:** honest compat maintenance after Phase 2 closure.

**Primary outcomes for this sprint:**
- deepen curated compat corpus confidence where the current corpus is still thin
- keep any new compat failure operations locked to JSONL/report/panel parity
- reconcile compat docs and status language whenever the claimed scope changes

**Non-goals for this sprint:**
- no relabeling compat surfaces to `READY` or `FULL` without real evidence
- no reopening baseline Phase 2 runtime closure as if it were still unfinished
- no broad new compat runtime feature expansion unless it directly closes a documented hardening gap

---

## Canonical Sources

Read these before starting Ticket `S05-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/COMPAT_EXIT_CHECKLIST.md`
- `docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md`
- `content/readiness/readiness_status.json`

Reference implementation surfaces:

- `tests/compat/test_compat_plugin_fixtures.cpp`
- `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- `tools/ci/run_compat_weekly_regression.ps1`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Treat compat as an import, validation, and migration bridge unless new evidence truly expands that bounded claim.
- Update docs in the same change as compat test or diagnostics changes.

---

## Session Start Checklist

- [ ] Read the canonical sources above.
- [ ] Open `docs/superpowers/plans/2026-04-21-sprint-05-compat-hardening-task-board.md`.
- [ ] Mark exactly one ticket as `IN PROGRESS`.
- [ ] Reconfirm the active local build profile.
- [ ] Run the preflight command block:

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

## Ticket Order

### S05-T01 Compat hardening: curated corpus depth follow-through

**Goal:** expand or tighten curated compat corpus evidence for the active migration-planning profiles without overstating runtime parity.

**Primary files:**
- `tests/compat/*`
- `docs/COMPAT_EXIT_CHECKLIST.md`
- canonical status docs

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_compat_tests.exe --reporter compact
ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure
```

**Status:** DONE 2026-04-21. The weekly compat lane now includes a directory re-import plus orchestration rerun anchor for the curated all-profile corpus.

---

### S05-T02 Compat hardening: new failure-path parity follow-through

**Goal:** keep new compat failure operations locked to JSONL export, report ingestion/export, and panel projection parity.

**Status:** DONE 2026-04-21. The failure lane now includes a real curated by-name dependency-gating anchor in addition to the existing direct-dispatch and parse-failure coverage.

**Primary files:**
- `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- compat diagnostics/report surfaces touched by the new failure operation
- canonical status docs

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_compat_tests.exe "[compat][diagnostics]" --reporter compact
```

---

### S05-T03 Compat truth-maintenance and sprint closeout

**Goal:** reconcile compat checklist/status language with the latest evidence and close the sprint cleanly.

**Status:** DONE 2026-04-21. Sprint closeout validation is green on the current tree.

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
