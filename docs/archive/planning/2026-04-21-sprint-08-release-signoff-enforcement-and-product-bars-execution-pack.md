# S08 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Advance URPG from first-slice governance artifacts toward executable release-signoff enforcement and one real product-bar gate without widening roadmap scope.

**Sprint theme:** release-signoff enforcement and executable product bars

**Primary outcomes for this sprint:**
- encode a bounded machine-checked release-signoff contract
- wire one existing cross-cutting product bar to executable gate evidence
- keep audit/readiness/docs truthful while the enforcement seam lands

**Non-goals for this sprint:**
- no broad Wave 2 feature expansion
- no unsafe automatic promotion to READY
- no unrelated refactors outside governance/audit/bar enforcement

---

## Canonical Sources

Read these before starting `S08-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Add sprint-specific sources below before starting implementation:

- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `docs/TRUTH_ALIGNMENT_RULES.md`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `tools/audit/urpg_project_audit.cpp`
- `engine/core/testing/visual_regression_harness.h`
- `engine/core/testing/visual_regression_harness.cpp`
- `tests/unit/test_visual_regression_harness.cpp`
- `tools/visual_regression/approve_golden.ps1`

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

### S08-T01: Release-Signoff Contract Encoding

**Goal:** turn the current artifact-backed release-signoff discipline into a bounded machine-checked contract without inventing unsafe auto-approval.

**Primary files:**
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `tools/audit/urpg_project_audit.cpp`
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`

**Implementation targets:**
- encode a small canonical signoff decision shape for the currently human-review-gated subsystem lanes
- enforce that readiness records, release matrix wording, and audit/report output agree on that shape
- keep the workflow conservative: machine checks may validate state and evidence presence, but must not imply automatic promotion to `READY`

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
.\build\dev-ninja-debug\urpg_project_audit.exe --json
```

**Done when:**
- the governed signoff lanes have a machine-checked contract
- readiness/truth gates enforce the contract
- audit output reflects the contract truthfully without overclaiming approval

---

### S08-T02: Visual Regression Executable Gate

**Goal:** wire one already-landed cross-cutting product bar to executable gate evidence by giving the visual regression lane a real golden-gate path.

**Primary files:**
- `engine/core/testing/visual_regression_harness.h`
- `engine/core/testing/visual_regression_harness.cpp`
- `tests/unit/test_visual_regression_harness.cpp`
- `tools/visual_regression/approve_golden.ps1`
- `tools/ci/run_local_gates.ps1`
- `docs/RELEASE_READINESS_MATRIX.md`

**Implementation targets:**
- add a bounded golden-gate workflow suitable for local/CI validation
- keep the scope conservative and repo-truthful; no fake “full rendering capture” claims unless real capture is added
- update readiness/docs to reflect the exact gate now supported

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[testing][visual_regression]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/visual_regression/approve_golden.ps1 -TestName sample -SnapshotId sample
```

**Done when:**
- visual regression has an executable golden-gate path rather than only helper tooling
- the release-readiness docs describe that gate accurately
- focused validation covers the new gate behavior

---

### S08-T03: Audit And Status Closeout

**Goal:** reconcile audit/readiness/status language with the Sprint 08 enforcement seam so product claims stay evidence-gated.

**Primary files:**
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROJECT_AUDIT.md`
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s08-task-board.md`
- `WORKLOG.md`

**Implementation targets:**
- update only the touched release-signoff and executable-bar language
- leave a truthful resume note or close the sprint cleanly
- preserve the active-sprint handoff contract for the next LLM session

**Verification:**

```powershell
ctest -L pr --output-on-failure
```

**Done when:**
- canonical docs describe the landed enforcement depth accurately
- the sprint artifacts are current
- the sprint can be resumed or closed without hidden assumptions

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
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
