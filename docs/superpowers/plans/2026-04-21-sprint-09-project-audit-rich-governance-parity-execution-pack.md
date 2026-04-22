# S09 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Bring ProjectAudit diagnostics and export parity up to the richer governance detail already emitted by the CLI.

**Sprint theme:** project audit governance parity

**Primary outcomes for this sprint:**
- preserve structured signoff and template-spec governance detail through `ProjectAuditPanel` and `DiagnosticsWorkspace`
- add end-to-end parity tests for nested governance payloads
- keep ProjectAudit docs and sprint artifacts truthful while the parity seam lands

**Non-goals for this sprint:**
- no new repo-wide scanner behavior
- no broad governance-roadmap expansion
- no unrelated diagnostics refactors

---

## Canonical Sources

Read these before starting `S09-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Add sprint-specific sources below before starting implementation:

- `docs/PROJECT_AUDIT.md`
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

### S09-T01: ProjectAudit Signoff Governance Parity

**Goal:** carry the richer `signoffArtifacts` governance payload from the CLI into the panel snapshot and diagnostics export path without flattening the structured contract state.

**Primary files:**
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`

**Implementation targets:**
- preserve `expectedArtifacts` and structured signoff-contract state in the panel snapshot model
- export the same nested signoff governance detail through `DiagnosticsWorkspace::exportAsJson()`
- keep field names and conservative meaning aligned with the CLI report model

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][project_audit]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][integration][project_audit]" --reporter compact
```

**Done when:**
- the signoff governance detail survives CLI -> panel -> workspace export without flattening
- focused panel/workspace regressions assert the nested payload

---

### S09-T02: ProjectAudit Template-Spec Governance Parity

**Goal:** preserve the richer `templateSpecArtifacts` parity payload through the same diagnostics/export path so docs and rendered data stay aligned.

**Primary files:**
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_cli.cpp`

**Implementation targets:**
- carry nested template-spec governance detail, including parity status and mismatches, into diagnostics snapshots
- keep export JSON in parity with the richer CLI report shape
- avoid inventing new audit checks beyond the existing CLI contract

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][project_audit]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][integration][project_audit]" --reporter compact
```

**Done when:**
- template-spec governance detail survives end to end without field loss or renaming drift
- focused regressions prove parity for the nested payload

---

### S09-T03: Audit Truth And Sprint Closeout

**Goal:** reconcile ProjectAudit docs/status/sprint artifacts with the richer diagnostics parity slice and leave a truthful handoff or closeout state.

**Primary files:**
- `docs/PROJECT_AUDIT.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s09-task-board.md`
- `WORKLOG.md`

**Implementation targets:**
- update only the touched ProjectAudit parity language
- keep the active-sprint handoff contract current
- close the sprint cleanly if the focused and broader validation snapshots are green

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- canonical docs describe the landed parity depth accurately
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
