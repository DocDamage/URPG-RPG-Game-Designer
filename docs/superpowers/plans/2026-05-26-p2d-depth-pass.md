# P2D Depth Pass Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move Perspective 2D from a composed first-class surface to a durable map-authoring workflow with layers, palette painting, event rows, persistence, playtest readiness, and export readiness.

**Architecture:** Keep the first depth pass inside `SpatialAuthoringWorkspace` so it composes with the existing elevation, props, parts, ability, terrain, region, procedural, and environment panels. Add a lightweight P2D authoring document model to the workspace snapshot and command API, with JSON save/load as the persistence contract and readiness diagnostics as the playtest/export gate.

**Tech Stack:** C++20, ImGui editor panel snapshots, `nlohmann::json`, Catch2 focused editor tests.

---

## File Map

- Modify: `editor/spatial/spatial_authoring_workspace.h`
  - Add P2D layer, tile palette, event row, project state, and save/load/playtest/export result snapshot structs.
  - Add command methods for layer management, tile selection/painting, event placement, draft save/load, playtest, and export.
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
  - Implement P2D document state, JSON serialization, layer validation, projection-to-tile painting, event placement, readiness diagnostics, and toolbar mode routing.
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
  - Add focused tests for RPG Maker-style tile/layer/event/persistence workflow and playtest/export readiness gates.
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
  - Record that the top-level P2D surface now owns the first durable tile/layer/event authoring slice.
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
  - Keep the status bounded and update the P2D evidence wording after tests pass.
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
  - Add a dated checkpoint for the P2D depth-pass slice.

## Task 1: P2D Layer, Palette, Event, And Persistence Spine

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`

- [x] **Step 1: Write failing persistence and authoring test**

Add a Catch2 test that creates a `SpatialAuthoringWorkspace`, adds `ground` and `events` layers, selects `tileset_overworld/grass_a`, paints two tiles through the canvas projection, adds one event through the canvas projection, saves the P2D draft JSON, loads it into a second workspace, and asserts the layer rows, palette state, event row, tile counts, dirty flag, and saved JSON contract.

- [x] **Step 2: Run focused test and verify red**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected: compile failure or test failure because the P2D layer/palette/event/persistence API does not exist yet.

- [x] **Step 3: Implement minimal P2D authoring document**

Add `ToolMode::Tiles`, snapshot structs, in-memory layer/tile/event rows, `AddPerspectiveLayer`, `SelectPerspectiveLayer`, `SetPerspectiveLayerVisible`, `SetPerspectiveLayerLocked`, `MovePerspectiveLayer`, `SelectPerspectiveTile`, `PaintPerspectiveTileFromScreen`, `AddPerspectiveEventFromScreen`, `SavePerspectiveMapDraft`, and `LoadPerspectiveMapDraft`. Persist JSON with:

```json
{
  "document_kind": "urpg.perspective_2d.map",
  "version": 1,
  "map_id": "map_id",
  "width": 10,
  "height": 10,
  "selected_layer_id": "ground",
  "selected_tileset_id": "tileset_overworld",
  "selected_tile_id": "grass_a",
  "layers": [],
  "tiles": [],
  "events": []
}
```

- [x] **Step 4: Run focused test and verify green**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected: the new persistence and authoring test passes.

## Task 2: P2D Playtest And Export Readiness Gates

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`

- [x] **Step 1: Write failing readiness test**

Add a Catch2 test that proves playtest/export are blocked until a target scene and overlay are bound, at least one visible unlocked tile layer exists, at least one tile is painted, and there are no events on hidden/locked layers. Assert playtest success sets export eligibility, and export returns the saved P2D JSON.

- [x] **Step 2: Run focused test and verify red**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected: readiness result API is missing or returns the default failure state.

- [x] **Step 3: Implement readiness diagnostics**

Add `RunPerspectiveMapPlaytest` and `ExportPerspectiveMap`. Playtest validates target binding, layer availability, painted tiles, event layer visibility/lock state, and map bounds. Export requires the latest playtest to pass and serializes the same P2D document JSON.

- [x] **Step 4: Run focused test and verify green**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected: both P2D depth tests pass.

## Task 3: Documentation Truth Update

**Files:**
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Update docs from evidence**

Record that P2D now has a first durable tile/layer/event/persistence/playtest/export-readiness slice, while keeping the broader product-depth claim bounded.

- [x] **Step 2: Run doc-safe checks**

Run:

```powershell
git diff --check
```

Expected: no whitespace errors.

## Task 4: Brush, Layer, And Event Command Ergonomics

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing brush/layer/event command tests**

Added focused `[editor][spatial][p2d_depth]` coverage for brush-size painting, erasing, layer duplicate/clear/delete, locked-layer protection, event movement, event command rows, and command persistence through draft save/load.

- [x] **Step 2: Run focused test and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile failed because the brush/layer/event command APIs and event command snapshot fields did not exist.

- [x] **Step 3: Implement the ergonomic slice**

Added brush-size painting/erasing, layer delete/duplicate/clear operations, event movement, event command rows, JSON command serialization, and snapshot command counts.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 5: Promoted Asset Palette And Editable Event Command Rows

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing promoted-palette and command-row tests**

Added focused `[editor][spatial][p2d_depth]` coverage for promoted tile palette option rows, palette option selection, selected option persistence, event command row update, event command row removal, and command persistence through draft save.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile failed because the promoted palette option snapshots/APIs and event command update/remove APIs did not exist.

- [x] **Step 3: Implement promoted palette and editable command rows**

Added `Perspective2DPaletteOption`, palette option snapshots, `SetPerspectiveTilePaletteOptions`, `SelectPerspectiveTilePaletteOption`, selected palette option JSON persistence, `UpdatePerspectiveEventCommand`, and `RemovePerspectiveEventCommand`.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 6: RPG Maker-Style Event Pages And Conditions

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing event-page tests**

Add focused `[editor][spatial][p2d_depth]` coverage for event page rows, selected page state, page conditions, page command rows, active page preview, and JSON draft save/load.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the new test fails because event page snapshots/APIs and JSON persistence are not present yet.

- [x] **Step 3: Implement event page authoring**

Add P2D event page structs, page condition structs, page command rows, selected-page editing APIs, condition preview state, active-page resolution, and JSON serialization/load while preserving the existing single-command-row event API.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 7: Event Page Editing Ergonomics

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing event-page editing tests**

Add focused `[editor][spatial][p2d_depth]` coverage for page reorder, duplicate, delete, page command update/remove, page condition update/remove, selected page preservation, and JSON draft save.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the new test fails because page editing APIs are not present yet.

- [x] **Step 3: Implement event page editing APIs**

Add explicit page reorder/duplicate/delete commands and page command/condition update/remove APIs, keeping existing event page JSON ordering and selected page behavior deterministic.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 8: Promoted Tile Palette Search, Filter, And Preview Metadata

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing palette UX tests**

Add focused `[editor][spatial][p2d_depth]` coverage for promoted tile option preview metadata, search text filtering, tileset filtering, category filtering, selected option visibility, empty filtered-state counts, and clearing filters.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the new test fails because palette filter state and preview metadata are not present yet.

- [x] **Step 3: Implement palette filter and preview state**

Add thumbnail/category metadata to promoted tile palette options, persistent filter snapshot state, visible option counts, and filter APIs that preserve direct selection while reporting whether the selected option is visible in the current filter.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 9: Multi-Layer Editing Ergonomics

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing multi-layer tests**

Add focused `[editor][spatial][p2d_depth]` coverage for bulk layer selection, selected-layer counts, selected row flags, bulk visible/locked toggles, duplicate-selected layers, delete-selected layers, and deterministic active-layer fallback.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the new test fails because multi-layer editing APIs and snapshot fields are not present yet.

- [x] **Step 3: Implement multi-layer editing**

Add bulk layer selection state, selected-layer snapshot counts, bulk visible/locked operations, selected-layer duplication with copied tiles/events, selected-layer deletion, order renumbering, and active-layer fallback.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 10: Runtime Playtest And Export Manifest Proof

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing runtime manifest tests**

Add focused `[editor][spatial][p2d_depth]` coverage proving playtest/export produce runtime-facing manifests that include visible unlocked tile layers, painted tiles, active event pages, active page commands, and package metadata.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because playtest/export runtime manifest fields are not present yet.

- [x] **Step 3: Implement runtime manifest serialization**

Add a runtime manifest serializer used by playtest and export results. Keep draft JSON unchanged, but add explicit playtest/export manifest fields that filter hidden/locked layers and resolve active event pages from condition preview state.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 11: Richer Event Page Condition Rules

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing comparison-condition tests**

Add focused `[editor][spatial][p2d_depth]` coverage for event page conditions with explicit comparison operators, including variable threshold checks, active-page switching, snapshots, draft JSON, and runtime manifest output.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because comparison condition APIs and JSON fields are not present yet.

- [x] **Step 3: Implement comparison condition rules**

Add a comparison field to P2D event page conditions, keep the existing equality condition API as a compatibility wrapper, evaluate string equality/inequality and numeric greater/less comparisons, and serialize comparison metadata through draft/runtime snapshots.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 12: Conditional Branch Event Commands

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing branch-command tests**

Add focused `[editor][spatial][p2d_depth]` coverage for authoring conditional branch commands inside event pages, adding true/false child commands, snapshot counts, draft JSON, and runtime manifest JSON.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because branch command APIs and nested command JSON are not present yet.

- [x] **Step 3: Implement branch command support**

Add branch condition metadata and true/false child command lists to P2D event commands, expose page branch authoring APIs, and serialize nested branch commands through snapshots, draft JSON, and runtime manifests.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 13: Runtime Event Execution Preview Trace

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing event execution preview tests**

Add focused `[editor][spatial][p2d_depth]` coverage for previewing a Perspective 2D event execution trace that resolves the active page, evaluates conditional branch command values, flattens the chosen true/false command path, reports missing-event blockers, and serializes a runtime-facing trace JSON document.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile failed because the event execution preview API and render snapshot result were not present yet.

- [x] **Step 3: Implement execution preview handoff**

Add `PreviewPerspectiveEventExecution`, execution-step/result snapshots, active-page resolution, branch condition evaluation, chosen-branch command flattening, JSON trace serialization, and render snapshot surfacing.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass with clean build output.

## Task 14: Playtest Event Execution Handoff

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing playtest handoff tests**

Add focused `[editor][spatial][p2d_depth]` coverage proving `RunPerspectiveMapPlaytest` emits runtime event execution traces for active events, with selected branch paths resolved from current event condition values.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because playtest does not yet expose event execution handoff traces.

- [x] **Step 3: Implement playtest execution handoff**

Promote event execution traces from one-off preview output into playtest results, runtime manifest metadata, and render snapshots so playtest has a concrete runtime-facing command stream for each active event.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 15: Package Inventory And Signature Proof

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing package inventory tests**

Add focused `[editor][spatial][p2d_depth]` coverage proving export returns a deterministic package file inventory with draft, runtime manifest, execution trace bundle, package manifest, and a stable package signature.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because export does not yet expose package file inventory/signature metadata.

- [x] **Step 3: Implement package inventory and signature**

Add deterministic package file rows, byte counts, content hashes, and a package signature to the P2D export result and package manifest.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 16: Creator UX Readiness Summary

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing creator UX summary tests**

Add focused `[editor][spatial][p2d_depth]` coverage proving render snapshots expose creator-facing workflow readiness: layer, palette, event page, branch, playtest, export, and release-asset gate states with actionable next steps.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because there is no consolidated creator UX readiness summary.

- [x] **Step 3: Implement creator readiness summary**

Add compact workflow summary fields to the P2D project snapshot, sourced from existing authoring state and latest playtest/export/release-gate results.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Task 17: Current-Checkout Release Asset Gate

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write failing release asset gate tests**

Add focused `[editor][spatial][p2d_depth]` coverage proving P2D can record the current release asset/LFS policy state, distinguish release-required assets from optional deferred LFS payloads, and carry that decision into playtest/export readiness snapshots.

- [x] **Step 2: Run focused build and verify red**

Ran:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile fails or the test fails because P2D has no release asset gate result.

- [x] **Step 3: Implement release asset gate result**

Add a P2D release asset gate API/result that marks release-required assets as verified, optional LFS payloads as deferred, blocks only release-required failures, and serializes this policy into export package metadata.

- [x] **Step 4: Run focused test and verify green**

Ran:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: all P2D depth tests pass.

## Self-Review

- Spec coverage: This plan covers tile/layer ergonomics, multi-layer editing, promoted asset palette rows, palette search/filter/preview metadata, palette selection, event/object rows, editable event command rows, RPG Maker-style event pages/conditions, richer comparison condition rules, conditional branch commands, page editing ergonomics, persistence, live playtest readiness, runtime manifest proof, event execution preview traces, playtest execution handoff traces, deterministic package inventory/signature proof, creator UX readiness summaries, release asset/LFS policy gating, export readiness, and docs truth for the first P2D depth slice. It does not claim full RPG Maker parity.
- Placeholder scan: No TBD/TODO placeholders are used.
- Type consistency: All new APIs are named on `SpatialAuthoringWorkspace` and are referenced consistently across test and implementation tasks.
