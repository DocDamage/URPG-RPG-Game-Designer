# S14 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Wire the export validator into an emitted-artifact pack-and-validate pipeline so the export lane is backed by post-export evidence, not only preflight fixture checks.

**Historical note:** Sprint 14 landed the emitted-artifact validation slice. A later bounded follow-through added one real Windows launch smoke path, so this file should be read as a sprint-local execution plan rather than the final export truth source.

**Sprint theme:** export validator to real export pipeline

**Primary outcomes for this sprint:**
- `ExportPackager::runExport()` writes an emitted export tree to disk that `ExportValidator` can validate post-export.
- `check_platform_exports.ps1` emits the same structured result shape as the C++ validator.
- Diagnostics panel and readiness docs reflect the new emitted-artifact contract.

**Non-goals for this sprint:**
- No real asset bundling and no broad cross-platform executable synthesis beyond the bounded emitted-artifact slice.
- No new export targets.
- No unrelated subsystem work.

---

## Canonical Sources

Read these before starting `S14-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`

Add sprint-specific sources below before starting implementation:

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
- Do not widen sprint scope by "cleaning up nearby code" unless the cleanup is necessary for the current ticket to compile or test.

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

### S14-T01: Pack and Validate Emitted Export Tree

**Goal:** Add one emitted-artifact "pack and validate export tree" path instead of only preflight fixture checks.

**Primary files:**
- `engine/core/tools/export_packager.h`
- `engine/core/tools/export_packager.cpp`
- `tests/unit/test_export_packager.cpp`

**Implementation targets:**
- Make `ExportPackager::bundleAssets` actually write `data.pck` into the output directory.
- Add `synthesizeExecutable` private helper that writes the target-appropriate executable artifact(s) into the output directory:
  - Windows: `game.exe`
  - Linux: `game`
  - macOS: `MyGame.app` directory
  - Web: `index.html`, `game.wasm`, `game.js`
- Call `synthesizeExecutable` from `runExport` after asset bundling.
- Add a new focused test `[export][packager]` that:
  1. Creates a fixture directory (so preflight passes).
  2. Calls `runExport`.
  3. Verifies `ExportValidator::validateExportDirectory` passes on the now-emitted tree.
  4. Asserts the emitted files exist on disk.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[export][packager]" --reporter compact
```

**Done when:**
- The new test passes.
- Existing export packager tests remain green.

---

### S14-T02: Wire Result Shape into check_platform_exports.ps1

**Goal:** Make `check_platform_exports.ps1` emit the same structured result shape as `ExportValidator::buildReportJson`.

**Primary files:**
- `tools/ci/check_platform_exports.ps1`
- `tests/unit/test_export_validator.cpp`

**Implementation targets:**
- Add an optional `-Json` switch to `check_platform_exports.ps1`.
- When `-Json` is present, emit a JSON object with `target`, `passed`, and `errors` array instead of plain text.
- Keep the existing plain-text output as the default for human readability.
- Add a focused test in `test_export_validator.cpp` (or a new PowerShell-invocation test) that invokes the script with `-Json` and asserts the emitted shape matches the C++ validator contract.

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_platform_exports.ps1 -ExportDir <temp> -Target Windows_x64 -Json
.\build\dev-ninja-debug\urpg_tests.exe "[export][validation]" --reporter compact
```

**Done when:**
- The script emits matching JSON shape under `-Json`.
- The script still passes/fails correctly for valid and invalid export directories.

---

### S14-T03: Promote Diagnostics and Docs to the New Contract

**Goal:** Reconcile diagnostics panel and readiness records with the emitted-artifact contract.

**Primary files:**
- `editor/export/export_diagnostics_panel.h`
- `editor/export/export_diagnostics_panel.cpp`
- `tests/unit/test_export_diagnostics_panel.cpp`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`

**Implementation targets:**
- Update `ExportDiagnosticsPanel::render` to include an `emittedArtifacts` array in the snapshot when a post-export tree is present, showing which files were validated after packing.
- Add focused test coverage for the new `emittedArtifacts` field.
- Update `readiness_status.json` for `export_validator` to reflect the new real-pipeline evidence.
- Update `RELEASE_READINESS_MATRIX.md` and `PROGRAM_COMPLETION_STATUS.md` with Sprint 14 slice note.
- Update `WORKLOG.md`.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[export][editor][panel]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- Diagnostics snapshot carries `emittedArtifacts` when applicable.
- Readiness gates pass.
- Canonical docs are truthful.

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
4. Leave blockers explicit, even if the blocker is "full suite too expensive right now".

Use the handoff template in:

- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

---

## Acceptance Summary

This sprint is complete only when all of the following are true:

- every ticket is `DONE` or explicitly moved out of sprint with rationale
- targeted verification passed or is explicitly waived with reason
- canonical docs and readiness records are aligned with the landed changes
- `WORKLOG.md` records the sprint slices
