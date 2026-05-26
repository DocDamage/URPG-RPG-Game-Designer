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

## Self-Review

- Spec coverage: This plan covers tile/layer ergonomics, promoted asset palette rows, palette selection, event/object rows, editable event command rows, persistence, live playtest readiness, export readiness, and docs truth for the first P2D depth slice. It does not claim full RPG Maker parity.
- Placeholder scan: No TBD/TODO placeholders are used.
- Type consistency: All new APIs are named on `SpatialAuthoringWorkspace` and are referenced consistently across test and implementation tasks.
