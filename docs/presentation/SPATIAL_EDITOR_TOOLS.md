# Spatial Editor Tools Implementation (2026-04-16)

> **Status: Incubating / Header-Only.** `editor/spatial/` currently contains only header files (`elevation_brush_panel.h`, `prop_placement_panel.h`). No `.cpp` implementations are registered in [CMakeLists.txt](../../CMakeLists.txt), so these panels are not compiled into product targets. They are exercised by `tests/unit/test_spatial_editor.cpp` as an anchored future productization path.

## Overview
Added native editor panels to support the `SpatialMapOverlay` system as part of the Phase 3/7 presentation upgrade. These tools allow non-technical authors to modify elevation and place 3D props without editing JSON schemas directly.

## Tools Added

### ElevationBrush (`ElevationBrushPanel`)
- **Path**: `editor/spatial/elevation_brush_panel.h`
- **Purpose**: Modifies the `ElevationGrid` within a `SpatialMapOverlay`.
- **Functions**:
  - `ApplyBrush(x, y, level)`: Updates a specific grid tile to a new elevation level.
  - **Multi-tile Support**: Respects `m_brushSize` for painting larger areas simultaneously.
  - Integration: Binds directly to the `SpatialMapOverlay::elevation` schema.

### PropPlacement (`PropPlacementPanel`)
- **Path**: `editor/spatial/prop_placement_panel.h`
- **Purpose**: Manages the `std::vector<PropInstance>` list for a map.
- **Functions**:
  - `AddProp(assetId, x, y, z)`: Places a new prop instance into the spatial world.
  - `TryProjectScreenToGround(...)`: Projects editor screen coordinates into world-space placement coordinates and samples height from the active `ElevationGrid`.
  - `AddPropFromScreen(...)`: Adds a prop directly from projected screen-space placement input.
  - **Asset Binding**: Currently supports direct `assetId` mapping for 3D meshes.
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
  - direct prop insertion
  - screen-to-world projection
  - elevation-aware projected placement
  - rejection of invalid or out-of-bounds placement
  - camera-ray intersection against elevated terrain
