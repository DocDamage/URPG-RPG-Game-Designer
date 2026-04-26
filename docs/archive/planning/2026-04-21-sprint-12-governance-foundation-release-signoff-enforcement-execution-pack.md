# S12 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Lock the shipped ProjectAudit governance contract into the readiness/truth gates and close the remaining localization-evidence parity gap without widening ProjectAudit into a full product scanner.

**Sprint theme:** governance foundation audit contract lock-in

**Primary outcomes for this sprint:**
- tighten machine-checked enforcement around the already-shipped ProjectAudit governance contract
- carry localization-evidence parity through ProjectAudit diagnostics surfaces and exports
- leave canonical readiness and audit docs truthful and resumable for follow-on work

**Non-goals for this sprint:**
- no new broad product scanner behavior
- no unrelated feature-lane implementation outside governance foundation
- no release-promotion claims that outrun executable evidence

---

## Canonical Sources

Read these before starting `S12-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Add sprint-specific sources below before starting implementation:

- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROJECT_AUDIT.md`
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `tools/audit/urpg_project_audit.cpp`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`

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

### S12-T01: ProjectAudit Contract Lock-In

**Goal:** tighten the readiness and truth gates so already-shipped ProjectAudit governance sections and counts cannot silently regress.

**Primary files:**
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `docs/PROJECT_AUDIT.md`

**Implementation targets:**
- require the newer shipped governance sections and issue counts in the readiness gate
- tighten the truth gate so `PROJECT_AUDIT.md` must describe the shipped richer contract precisely
- keep the enforcement grounded in the existing CLI contract rather than adding a new broad scanner rule

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- the shipped ProjectAudit contract is machine-checked by the readiness/truth gates
- the canonical audit docs stay truthful under that stronger enforcement

---

### S12-T02: Localization Evidence Diagnostics Parity

**Goal:** carry the already-shipped `localizationEvidence` ProjectAudit contract through panel snapshots and diagnostics export so the diagnostics path matches the CLI.

**Primary files:**
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`
- `docs/PROJECT_AUDIT.md`

**Implementation targets:**
- add structured `localizationEvidence` snapshot/export support
- preserve the missing localization-related issue counts through the diagnostics path where applicable
- keep diagnostics parity bounded to the CLI contract already emitted by `urpg_project_audit`

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][project_audit]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][integration][project_audit]" --reporter compact
```

**Done when:**
- CLI, panel, and diagnostics export stay in parity for `localizationEvidence`
- `PROJECT_AUDIT.md` truthfully describes the emitted governance surface

---

### S12-T03: Governance Truth And Sprint Closeout

**Goal:** reconcile the canonical governance wording with the narrowed ProjectAudit contract-lock/parity slice and close the sprint cleanly if the broader validation remains green.

**Primary files:**
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s12-task-board.md`
- `WORKLOG.md`

**Implementation targets:**
- update only the touched governance wording
- keep claims below full release-ready productization
- leave a truthful closeout or resume note for the next session

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- canonical docs describe the strengthened signoff evidence accurately
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
