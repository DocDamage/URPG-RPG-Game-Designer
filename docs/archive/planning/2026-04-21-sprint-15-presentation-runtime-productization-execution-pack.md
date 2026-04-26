# S15 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Productize the presentation runtime by proving a real authoring→runtime consumption path from spatial editing into the presentation frame pipeline, tightening release validation around that path, and reconciling ADR/status wording.

**Sprint theme:** presentation runtime productization

**Primary outcomes for this sprint:**
- One end-to-end test proves spatial editing output (elevation + props) flows into `PresentationRuntime` frame generation.
- Release validation harness covers the spatial-authoring→runtime-consumption path.
- ADR-011 and canonical status docs truthfully reflect compiled spatial panels and runtime-backed evidence.

**Non-goals for this sprint:**
- No new renderer backend integration.
- No new presentation targets or capability tiers.
- No unrelated subsystem work.

---

## Canonical Sources

Read these before starting `S15-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/adr/ADR-011-presentation-spatial-status.md`
- `docs/presentation/VALIDATION.md`

Add sprint-specific sources below before starting implementation:

- `engine/core/presentation/presentation_runtime.h`
- `engine/core/presentation/presentation_runtime.cpp`
- `engine/core/presentation/presentation_bridge.h`
- `engine/core/presentation/presentation_bridge.cpp`
- `engine/core/presentation/release_validation.cpp`
- `editor/spatial/elevation_brush_panel.h`
- `editor/spatial/elevation_brush_panel.cpp`
- `editor/spatial/prop_placement_panel.h`
- `editor/spatial/prop_placement_panel.cpp`
- `tests/unit/test_presentation_runtime.cpp`
- `tests/unit/test_presentation_bridge.cpp`
- `tests/unit/test_spatial_editor.cpp`

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

### S15-T01: Richer Authoring/Runtime Proof Path

**Goal:** Add one end-to-end test connecting spatial editing output to presentation runtime consumption.

**Primary files:**
- `tests/unit/test_spatial_editor.cpp`
- `tests/unit/test_presentation_runtime.cpp`

**Implementation targets:**
- Add a new focused test case (or extend existing `[editor][spatial]` test) that:
  1. Creates a `SpatialMapOverlay` with a flat elevation grid.
  2. Uses `ElevationBrushPanel` to raise a hill at a specific coordinate.
  3. Uses `PropPlacementPanel` to place props on the raised terrain.
  4. Feeds the edited overlay into `PresentationAuthoringData`.
  5. Calls `PresentationRuntime::BuildPresentationFrame` with a matching `PresentationContext`.
  6. Verifies the resulting `PresentationFrameIntent` contains `DrawActor` and `DrawProp` commands with Y-positions resolved from the edited elevation (not flat ground).
- Tag the test `[presentation][spatial][e2e]`.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[presentation][spatial][e2e]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][spatial]" --reporter compact
```

**Done when:**
- The new test passes.
- Existing presentation and spatial tests remain green.

---

### S15-T02: Tighten Release-Validation Coverage

**Goal:** Extend the release validation harness to cover the spatial-authoring→runtime-consumption path.

**Primary files:**
- `engine/core/presentation/release_validation.cpp`

**Implementation targets:**
- Add a new validation section to `RunReleaseValidation` that:
  1. Creates a `SpatialMapOverlay`, edits it via brush and prop placement.
  2. Builds a `PresentationFrameIntent` from the edited overlay.
  3. Asserts that prop commands have Y-positions reflecting the edited elevation.
  4. Asserts that actor commands walking on the raised terrain also resolve to the correct Y.
- Keep the existing 100-actor stress test and battle effect cue envelope intact.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_presentation_release_validation.exe
```

**Done when:**
- The release validation executable passes.
- No assertions are relaxed or removed.

---

### S15-T03: Reconcile ADR/Status Wording

**Goal:** Update ADR-011, validation docs, and readiness records to reflect the now-runtime-backed spatial editor and presentation pipeline.

**Primary files:**
- `docs/adr/ADR-011-presentation-spatial-status.md`
- `docs/presentation/VALIDATION.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`

**Implementation targets:**
- Update ADR-011:
  - Correct the `editor/spatial/` status from incubating to productized for the compiled panel scope.
  - Correct the `engine/core/presentation/` status to reflect compiled runtime, bridge, and release validation.
  - Keep honest residual gaps (e.g., `profile_arena.cpp` not yet registered, renderer backend is mock).
- Update `VALIDATION.md` to include the new `[presentation][spatial][e2e]` test in the spatial editor authoring lane description.
- Update `readiness_status.json` for `presentation_runtime`:
  - Set `diagnostics` and `testsValidation` evidence to `true`.
  - Narrow `mainGaps` to honest residual scope.
- Update `RELEASE_READINESS_MATRIX.md`, `PROGRAM_COMPLETION_STATUS.md`, and `WORKLOG.md`.

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
.\tools\ci\run_presentation_gate.ps1
```

**Done when:**
- Readiness gates pass.
- Presentation gate passes.
- Canonical docs are truthful.

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
.\tools\ci\run_presentation_gate.ps1
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
