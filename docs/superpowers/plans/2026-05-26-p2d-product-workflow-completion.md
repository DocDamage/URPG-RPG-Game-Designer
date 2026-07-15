# P2D Product Workflow Completion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Turn the completed Perspective 2D depth slice into a stronger product workflow boundary with analyzer-backed readiness, editor route summaries, RPG Maker-style command coverage checks, and fixture-backed playable proof.

**Architecture:** Keep existing `SpatialAuthoringWorkspace` APIs stable, but extract product-level analysis into a focused `Perspective2DProductWorkflow` service that consumes the existing draft/runtime/export JSON contracts. The workspace remains the editor coordinator while the new service owns map/event/tile workflow summaries, transfer graph validation, command coverage diagnostics, and playable sample evidence.

**Tech Stack:** C++20, nlohmann::json, Catch2, CMake/Ninja, existing `urpg_spatial_unit_tests`.

---

## File Map

- Create: `editor/spatial/perspective_2d_product_workflow.h`
- Create: `editor/spatial/perspective_2d_product_workflow.cpp`
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `CMakeLists.txt`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

## Task 1: Product Workflow Analyzer Boundary

**Files:**
- Create: `editor/spatial/perspective_2d_product_workflow.h`
- Create: `editor/spatial/perspective_2d_product_workflow.cpp`
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write the failing analyzer test**

Add a focused `[editor][spatial][p2d_product]` test that builds a P2D draft, playtest runtime manifest, and export package through `SpatialAuthoringWorkspace`, then analyzes those JSON contracts with `Perspective2DProductWorkflow::Analyze`. Assert the report contains map counts, tile/event counts, active command coverage, package file coverage, transfer graph edges, and no blockers.

- [x] **Step 2: Verify red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Observed red: the first build attempt was blocked by the pre-existing MinGW `-loldnames` link failure before compilation reached the new missing header. After adding the build-local MinGW compatibility shim, the next red came from the workspace snapshot field missing in Task 2.

- [x] **Step 3: Implement the analyzer**

Create a dependency-light analyzer that parses the existing JSON strings, reports:
- `ready`
- `map_id`
- `draft_layer_count`
- `draft_tile_count`
- `draft_event_count`
- `runtime_layer_count`
- `runtime_tile_count`
- `runtime_event_count`
- `export_package_file_count`
- `package_signature_present`
- `transfer_edge_count`
- `supported_command_count`
- `unsupported_command_count`
- `blockers`

- [x] **Step 4: Verify green**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_product]" --reporter compact
```

Expected green: product analyzer test passes.

## Task 2: Workspace Product Summary Snapshot

**Files:**
- Modify: `editor/spatial/spatial_authoring_workspace.h`
- Modify: `editor/spatial/spatial_authoring_workspace.cpp`
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`

- [x] **Step 1: Write the failing workspace summary test**

Extend the product test to assert `lastRenderSnapshot().perspective_2d_product` exposes the analyzer report after playtest/export.

- [x] **Step 2: Verify red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_spatial_unit_tests
```

Expected red: compile failure because `Perspective2DProductWorkflowSnapshot` is not part of `RenderSnapshot`.

- [x] **Step 3: Wire analyzer output into the workspace**

Add a compact product workflow snapshot to `RenderSnapshot`, update it from the latest save/playtest/export JSON, and keep the result conservative until all three contracts are present.

- [x] **Step 4: Verify green**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_product]" --reporter compact
```

Expected green: product analyzer and workspace snapshot pass.

## Task 3: Playable Sample Proof

**Files:**
- Modify: `tests/unit/test_spatial_editor_canvas_workspace.cpp`
- Modify: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [x] **Step 1: Write the failing playable sample test**

Add a test that creates a tiny playable P2D map with tiles, event pages, switch/variable changes, transfer, item/gold mutation, starting party, save/load profile, release asset gate, playtest, runtime execution, export, and product workflow analysis.

- [x] **Step 2: Verify red**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_product]" --reporter compact
```

Observed red: compile failure because `RenderSnapshot` did not expose `perspective_2d_product`.

- [x] **Step 3: Complete product proof behavior**

Use the analyzer and existing workspace commands to prove all product workflow gates are represented in a single report. Do not claim full RPG Maker parity; the proof is for the implemented command set.

- [x] **Step 4: Update docs from evidence**

Record the analyzer boundary and playable sample proof in the release inventory and status docs.

- [x] **Step 5: Verify focused and broader lanes**

Run:

```powershell
.\build\dev-ninja-debug\urpg_spatial_unit_tests.exe "[editor][spatial][p2d_product],[editor][spatial][p2d_depth]" --reporter compact
ctest --preset dev-spatial --output-on-failure
git diff --check
```

Expected green: product and depth tests pass, spatial lane passes, and whitespace is clean.

## Self-Review

- Spec coverage: This plan covers a product workflow boundary, editor summary wiring, command/transfer/package analysis, a tiny playable P2D proof, and documentation truth updates. It intentionally does not promise full engine-wide RPG Maker parity beyond the implemented P2D command set.
- Placeholder scan: No TBD/TODO placeholders are used.
- Type consistency: The analyzer is consistently named `Perspective2DProductWorkflow`; workspace exposure is consistently named `perspective_2d_product`.
