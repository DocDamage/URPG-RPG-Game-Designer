# MapScene Presentation Contract

## Purpose
Defines the visual translation rules for the primary exploratory world state.

## Minimum Requirements
- **Terrain/World Composition**: Must resolve tile-based or mesh-based terrain intent.
- **Elevation-Aware Traversal**: Visual representation of "up" vs "down" (stairs, cliffs).
- **Props and Occlusion**: Resolve prop placement and visibility (masking, transparency).
- **Actor Anchoring**: Rules for where sprite pivots/feet touch the world (Depth Policy).
- **Environmental Volumes**: Support for local fog/light/post-FX zones in 3D space.
- **Camera Anchors**: Standardized camera behaviors (Follow, Fixed, Interpolated).
- **Streaming Viewport Hints**: Must provide hints to the asset system about upcoming visual needs.

## Fallback Policy
- Must degrade to a valid "Classic 2D" sprite-based representation if Tier 0 is active.
- Elevation becomes a purely visual layering (Z-Order) without 3D projection.
