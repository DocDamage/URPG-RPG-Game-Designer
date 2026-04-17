# ADR-010: Presentation Core Architectural Completion

**Status**: [Approved]
**Date**: 2026-04-16

## Context
The URPG engine requires a "First-Class" presentation layer to transition from 2D heritage logic to a modern spatial subsystem capable of scaling across capability tiers (Legacy Mobile to High-End Desktop).

## Decision
We have implemented a decoupled, contract-first Presentation Core that manages all visual intent. This ADR finalizes the architectural patterns used during Phases 0-17 of the expansion.

### Key Architectural Pillars:
1. **Scene-to-Intent Translation**: All scenes (Map, Battle, Menu, Dialogue) are translated into an immutable `PresentationFrameIntent` buffer. The Game Thread never speaks directly to the Renderer.
2. **Capability-Tiered Scaling**: All visual features are bucketed into four tiers (0-3). The `TierDetector` automatically assigns these based on hardware probing and runtime performance.
3. **Z-Layered Spatial Maps**: Maps are no longer flat. The `SpatialMapOverlay` provides an elevation grid and 3D prop placement, while maintaining a deterministic fallback path for heritage hardware.
4. **Authoritative Registry**: All profiles (Actors, Camera, Environment) are stored in a centralized `PresentationRegistry` with full hot-reloading support.
5. **Readability Guarantees**: UI and Dialogue translators have the authority to override environmental effects (Desaturation/Blur) to ensure text clarity.

## Consequences
- **Positive**: Complete decoupling of gameplay logic from rendering APIs. Consistent scaling across diverse hardware. High authoring productivity via hot-reload.
- **Neutral**: Requires strict adherence to the command-buffer pattern. Direct "renderer-only" hacks are now architectural violations.
- **Negative**: Slight overhead for the translation pass (mitigated by 2MB Arena Allocator).
