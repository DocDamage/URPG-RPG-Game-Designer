# P2D RPG Maker Grade Tiles And Database Integration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Promote Perspective 2D from basic tile painting and local event state into an RPG Maker-grade tile metadata surface with project/database reference integration.

**Architecture:** Keep this slice inside `SpatialAuthoringWorkspace` so it stays consistent with the existing P2D authoring, runtime, playtest, export, and snapshot APIs. Add explicit tile-system metadata for tileset pages A-Z, autotiles, animation, passability, terrain tags, region IDs, priority/star behavior, collision previews, and project/database references for actors, items, switches, variables, common events, maps, starting party, transfers, encounters, assets, and save/load state.

**Tech Stack:** C++20, ImGui editor model snapshots, nlohmann::json serialization, Catch2 focused `[editor][spatial][p2d_depth]` tests, CMake/Ninja `urpg_spatial_unit_tests`.

---

## Task 1: RPG Maker-Grade Tile Metadata

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`

- [x] **Step 1: Write the failing tile-system test**

Add a focused P2D test that registers pages A-Z, records tile metadata for an autotile/animated/star-priority/passability tile, paints it, previews tile metadata at the painted coordinate, and verifies playtest/export runtime manifests carry the tile-system metadata.

- [x] **Step 2: Run focused build and verify red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile failure because the tile-system APIs and snapshots do not exist.

- [x] **Step 3: Implement tile-system metadata**

Add page and tile-definition structs, APIs for setting pages and tile metadata, a tile preview query, snapshot projection, and runtime/export JSON serialization for pages, autotile kind, animation frames, passability, terrain tags, region IDs, priority, star passability, and preview paths.

- [x] **Step 4: Run focused P2D tests**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
```

Expected green: P2D depth tests pass.

## Task 2: Project/Database Integration

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`

- [x] **Step 1: Write the failing project-integration test**

Add a focused P2D test that binds project/database references for actors, items, switches, variables, common events, maps, starting party, transfers, encounters, assets, and save/load state, then verifies event commands validate against those references and playtest/export manifests serialize the integration.

- [x] **Step 2: Run focused build and verify red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile failure because project/database integration APIs and snapshots do not exist.

- [x] **Step 3: Implement project/database integration**

Add project reference structs, command APIs, validation diagnostics, snapshot counts/readiness, and runtime/export JSON serialization for database/project references and save/load state.

- [x] **Step 4: Run focused verification**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_depth]" --reporter compact
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial]" --reporter compact
git diff --check
```

Expected green: focused P2D and broader spatial editor tests pass with no whitespace errors.

## MCP Recommendation

Building a URPG MCP makes sense after the editor/runtime APIs are stable enough to expose. The first MCP should be local-only and wrap existing deterministic commands: inspect project status, list maps/events/tilesets/assets, run focused gates, invoke P2D tile/event/database commands, run playtest/export checks, and return structured diagnostics. It should not bypass source-control, release gates, or asset licensing checks.

## Self-Review

- Spec coverage: The plan covers autotiles, animated tiles, passage/collision flags, terrain tags, region IDs, tileset pages A-Z, priority/star passability, tile previews, actors, items, switches, variables, common events, maps list, starting party, transfers, encounters, assets, save/load, and an MCP recommendation.
- Placeholder scan: No TBD/TODO placeholders are used.
- Type consistency: All new APIs live on `SpatialAuthoringWorkspace` and flow into the existing snapshot/playtest/export results.
