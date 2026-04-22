# Spatial Editor Tools Implementation (2026-04-20)

> **Status: Compiled authoring lane.** `editor/spatial/` now includes `elevation_brush_panel.cpp` and `prop_placement_panel.cpp`, both registered in [CMakeLists.txt](../../CMakeLists.txt) and covered by `tests/unit/test_spatial_editor.cpp`. This remains a focused authoring lane, not a blanket claim that the entire broader presentation subsystem is fully productized.

## Overview
Added native editor panels to support the `SpatialMapOverlay` system as part of the Phase 3/7 presentation upgrade. These tools allow non-technical authors to modify elevation and place 3D props without editing JSON schemas directly.

## Tools Added

### ElevationBrush (`ElevationBrushPanel`)
- **Path**: `editor/spatial/elevation_brush_panel.h`, `editor/spatial/elevation_brush_panel.cpp`
- **Purpose**: Modifies the `ElevationGrid` within a `SpatialMapOverlay`.
- **Functions**:
  - `ApplyBrush(x, y, level)`: Updates a specific grid tile to a new elevation level.
  - **Multi-tile Support**: Respects `m_brushSize` for painting larger areas simultaneously.
  - **Snapshot State**: Exposes target-grid metadata and brush settings through a lightweight render snapshot for tests and diagnostics.
  - Integration: Binds directly to the `SpatialMapOverlay::elevation` schema.

### PropPlacement (`PropPlacementPanel`)
- **Path**: `editor/spatial/prop_placement_panel.h`, `editor/spatial/prop_placement_panel.cpp`
- **Purpose**: Manages the `std::vector<PropInstance>` list for a map.
- **Functions**:
  - `AddProp(assetId, x, y, z)`: Places a new prop instance into the spatial world.
  - `TryProjectScreenToGround(...)`: Projects editor screen coordinates into world-space placement coordinates and samples height from the active `ElevationGrid`.
  - `AddPropFromScreen(...)`: Adds a prop directly from projected screen-space placement input.
  - **Asset Binding**: Currently supports direct `assetId` mapping for 3D meshes.
  - **Snapshot State**: Exposes selected asset id, prop counts, and the latest placed prop through a lightweight render snapshot.
  - **Projection Modes**:
    - simplified top-down editor projection using viewport size, camera center, and world-units-per-pixel settings
    - camera-aware ray projection using `ViewportRect`, `CameraState`, `CameraProfile`, and `SpatialProjection`
  - Future: Support for `rotY` and `scale` adjustments via Gizmos.

## Shared Integration
- Both panels inherit from `EditorPanel` for standard ImGui lifecycle management.
- They use `SetTarget()` to bind to the active map's overlay data.

## Verification
- Unit tests implemented in `tests/unit/test_spatial_editor.cpp` cover:
  - elevation editing and bounds safety
  - brush snapshot state
  - direct prop insertion
  - prop snapshot state
  - screen-to-world projection
  - elevation-aware projected placement
  - rejection of invalid or out-of-bounds placement
  - camera-ray intersection against elevated terrain
