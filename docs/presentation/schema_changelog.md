# Presentation Core - Schema Changelog

## [1] — 2026-04-16
Initial schema versions for Presentation Core Phase 1 & 2.

### Added Schemas:
- `ProjectPresentationSettings` (v1): Global feature enablement and tier thresholds.
- `SpatialMapOverlay` (v1): 3D-aware map data with elevation and prop arrays.
- `ElevationGrid` (v1): Deterministic tile-based elevation data.
- `PropInstance` (v1): Transform and variant data for 3D environment assets.
- `LightProfile` (v1): Standardized light properties (Color, Intensity, Falloff).
- `FogProfile` (v1): Volumetric and distance-based atmospheric settings.
- `PostFXProfile` (v1): Color correction and filter parameters for read-only interpreters.
- `MaterialResponseProfile` (v1): Tier-aware material overrides for performance.
- `TierFallbackDeclaration` (v1): Explicit logic for downgrade-path resource resolution.
- `ActorPresentationProfile` (v1): Skeletal and transform offsets for map/battle actors.
- `CameraProfile` (v1): Target framing, pitch limits, and zoom constraints.
- `PresentationRegistry` (v1): Centralized resource indexing and cold-boot hydration.
- `StreamingManifest` (v1): LOD and chunk-based loading priorities.
- `ShaderPipelineState` (v1): Immutable hash-based GPU state tracking (ADR-007).

