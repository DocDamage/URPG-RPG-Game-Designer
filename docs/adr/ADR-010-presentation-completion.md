# ADR-010: Presentation Core Architectural Completion

**Status**: [Approved]
**Date**: 2026-04-16

Historical note: this ADR records the architectural completion claim as it was written on 2026-04-16. Later canonical status and follow-up ADRs, especially `ADR-011-presentation-spatial-status`, clarified that the broader presentation/spatial subsystem remained incubating rather than fully productized. Use the canonical program/remediation docs for current branch truth.

## Context
The URPG engine requires a "First-Class" presentation layer to transition from 2D heritage logic to a modern spatial subsystem capable of scaling across capability tiers (Legacy Mobile to High-End Desktop).

## Decision
This ADR approved a decoupled, contract-first Presentation Core direction intended to manage visual intent. It recorded the architectural patterns the program was aiming to finalize during the presentation expansion effort; it does not supersede later status corrections about build-graph coverage or productization maturity.

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
