# S11 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Add a real localization completeness gate and thread that same repo-local evidence through ProjectAudit and the canonical status stack without widening the audit into a full project scanner.

**Sprint theme:** localization completeness executable gate

**Primary outcomes for this sprint:**
- add a canonical localization consistency gate using the already-landed localization runtime helpers
- surface localization completeness evidence through ProjectAudit without widening scanner scope
- keep localization governance wording truthful and aligned across local gates, ProjectAudit, and canonical status docs

**Non-goals for this sprint:**
- no broad localization productization effort
- no new full repo scanner behavior
- no unrelated governance or diagnostics refactors outside the localization completeness seam

---

## Canonical Sources

Read these before starting `S11-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Add sprint-specific sources below before starting implementation:

- `docs/PROJECT_AUDIT.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `engine/core/localization/locale_catalog.h`
- `engine/core/localization/locale_catalog.cpp`
- `engine/core/localization/completeness_checker.h`
- `engine/core/localization/completeness_checker.cpp`
- `content/schemas/localization_bundle.schema.json`
- `tools/audit/urpg_project_audit.cpp`
- `tools/ci/run_local_gates.ps1`
- `tests/unit/test_locale_catalog.cpp`
- `tests/unit/test_completeness_checker.cpp`
- `tests/unit/test_project_audit_cli.cpp`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation.
- Unsupported source behavior must become structured diagnostics or explicit waivers, never silent loss.
- Do not widen sprint scope by “cleaning up nearby code” unless the cleanup is necessary for the current ticket to compile or test.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open the paired sprint task board.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [ ] Run the sprint preflight command block.

Suggested preflight:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### S11-T01: Localization Consistency Gate

**Goal:** add a canonical localization completeness/consistency gate using `LocaleCatalog` and `CompletenessChecker` against repo-local localization bundles and the existing schema contract.

**Primary files:**
- `engine/core/localization/locale_catalog.h`
- `engine/core/localization/locale_catalog.cpp`
- `engine/core/localization/completeness_checker.h`
- `engine/core/localization/completeness_checker.cpp`
- `content/schemas/localization_bundle.schema.json`
- `tests/unit/test_locale_catalog.cpp`
- `tests/unit/test_completeness_checker.cpp`
- `tools/ci/check_localization_consistency.ps1`
- `tools/ci/run_local_gates.ps1`

**Implementation targets:**
- add a focused PowerShell gate for localization bundle validity and completeness expectations
- reuse existing localization runtime helpers instead of re-inventing the contract in docs alone
- wire the new check into the local gate stack conservatively

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[localization]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_localization_consistency.ps1
```

**Done when:**
- the repo has a real localization consistency gate
- the focused localization tests and the new CI script agree on the same bounded contract

---

### S11-T02: ProjectAudit Localization Evidence

**Goal:** surface the new localization gate and missing-key/completeness evidence through ProjectAudit without turning it into a broad full-repo scanner.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `docs/PROJECT_AUDIT.md`

**Implementation targets:**
- upgrade the localization artifact/audit section so it reflects the now-landed consistency gate
- keep the audit scoped to repo-local canonical evidence rather than generic crawling
- add focused CLI regressions for the new localization governance detail

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
.\build\dev-ninja-debug\urpg_project_audit.exe --json
```

**Done when:**
- ProjectAudit exposes the landed localization evidence truthfully
- focused CLI coverage proves the new localization governance contract

---

### S11-T03: Localization Truth And Sprint Closeout

**Goal:** reconcile the localization readiness/audit wording with the landed executable gate and close the sprint cleanly if the broader validation remains green.

**Primary files:**
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s11-task-board.md`
- `WORKLOG.md`

**Implementation targets:**
- update only the touched localization governance wording
- keep localization claims below broad productization promises
- leave a truthful closeout or resume note for the next session

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- canonical docs describe the landed localization evidence accurately
- sprint artifacts are current and resumable
- the sprint can be resumed or closed without hidden assumptions

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

If a command is too expensive or blocked, record the reason in the task board and final sprint note.

---

## Commit Rhythm

Preferred commit shape:

1. tests that expose the missing behavior
2. implementation
3. docs/readiness/worklog alignment

Suggested commit subject style:

- `test: add <ticket> regression coverage`
- `feat: implement <ticket behavior>`
- `docs: reconcile sprint status and readiness evidence`

---

## Resume Protocol

When handing off to a new LLM session:

1. Update the task board `Resume From Here` section.
2. Record the current ticket status and exact next command.
3. Keep the touched-files list current.
4. Leave blockers explicit, even if the blocker is “full suite too expensive right now”.

Use the handoff template in:

- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

---

## Acceptance Summary

This sprint is complete only when all of the following are true:

- every ticket is `DONE` or explicitly moved out of sprint with rationale
- targeted verification passed or is explicitly waived with reason
- canonical docs and readiness records are aligned with the landed changes
- `WORKLOG.md` records the sprint slices
