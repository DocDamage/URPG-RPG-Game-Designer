# S10 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Harden the export validation pipeline around the already-landed validator, packager integration, diagnostics surface, and CI script without overclaiming a full real export-artifact pipeline.

**Sprint theme:** export validation pipeline hardening

**Primary outcomes for this sprint:**
- add a bounded executable gate for the validator-to-packager export path
- keep export diagnostics and CI/local gate behavior aligned on the same synthetic export-fixture contract
- update readiness/status wording so the landed export evidence is sharper but still conservative

**Non-goals for this sprint:**
- no real production export pipeline
- no broad release-readiness promotion for export lanes
- no unrelated refactors outside export validation, packager integration, diagnostics parity, and closeout docs

---

## Canonical Sources

Read these before starting `S10-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Add sprint-specific sources below before starting implementation:

- `docs/RELEASE_READINESS_MATRIX.md`
- `engine/core/export/export_validator.h`
- `engine/core/export/export_validator.cpp`
- `engine/core/tools/export_packager.h`
- `engine/core/tools/export_packager.cpp`
- `editor/export/export_diagnostics_panel.h`
- `editor/export/export_diagnostics_panel.cpp`
- `tools/ci/check_platform_exports.ps1`
- `tests/unit/test_export_validator.cpp`
- `tests/unit/test_export_packager.cpp`
- `tests/unit/test_export_diagnostics_panel.cpp`

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

### S10-T01: Export Validator And Packager Gate

**Goal:** add a bounded executable gate that proves the existing export validator and `ExportPackager::validateBeforeExport()` path agree on synthetic export-root fixtures.

**Primary files:**
- `engine/core/export/export_validator.h`
- `engine/core/export/export_validator.cpp`
- `engine/core/tools/export_packager.h`
- `engine/core/tools/export_packager.cpp`
- `tests/unit/test_export_validator.cpp`
- `tests/unit/test_export_packager.cpp`
- `tools/ci/check_platform_exports.ps1`

**Implementation targets:**
- add fixture-backed pass/fail coverage that exercises validator and packager integration on the same export-root contract
- tighten `check_platform_exports.ps1` only around the already-landed synthetic validator seam
- avoid implying that this validates a real shipped export-artifact pipeline

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[export][validation]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[export][packager]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_platform_exports.ps1
```

**Done when:**
- validator and packager integration share one executable synthetic-fixture contract
- the focused export CI script passes on the current tree

---

### S10-T02: Export Diagnostics Parity

**Goal:** keep `ExportDiagnosticsPanel` and export-report JSON aligned with the sharpened validator/packager contract.

**Primary files:**
- `editor/export/export_diagnostics_panel.h`
- `editor/export/export_diagnostics_panel.cpp`
- `tests/unit/test_export_diagnostics_panel.cpp`
- `tests/unit/test_export_validator.cpp`
- `tests/unit/test_export_packager.cpp`

**Implementation targets:**
- ensure diagnostics snapshots reflect the same missing-file and ready-to-export conditions the validator/packager path now proves
- add focused parity regressions instead of broad export-surface redesign
- keep report vocabulary conservative and consistent with the validator JSON contract

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[export][editor][panel]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[export][validation]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[export][packager]" --reporter compact
```

**Done when:**
- diagnostics snapshots stay in parity with the sharpened export validation contract
- focused panel and export tests are green together

---

### S10-T03: Export Truth And Sprint Closeout

**Goal:** reconcile readiness/status/sprint artifacts with the sharpened export-validation slice and close the sprint cleanly if the broader validation remains green.

**Primary files:**
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s10-task-board.md`
- `WORKLOG.md`

**Implementation targets:**
- update only the touched export-validation wording
- keep the lane explicitly below claims about a real export-artifact pipeline
- leave a truthful closeout or resume note for the next session

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- canonical docs describe the landed export evidence accurately
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
