# ADR-011: Presentation and Spatial Subsystem Status — Productized with Residual Gaps

**Status:** Approved  
**Date:** 2026-04-17  
**Updated:** 2026-04-21  
**Supersedes / Clarifies:** ADR-010-presentation-completion (architectural patterns approved; build coverage and productization status explicitly downscoped)

Historical note: this ADR preserves the status correction recorded on 2026-04-17. The specific `editor/spatial/*` header-only limitation described in the original Context was remediated on 2026-04-20 when compiled `.cpp` panel sources were added and registered in the build graph. The subsystem was further productized on 2026-04-21 with a runtime-backed spatial-authoring→presentation-consumption proof path. For current execution and release truth, defer to the canonical program/remediation documents.

## Context

ADR-010 established the architectural patterns for the Presentation Core (scene-to-intent translation, capability tiers, spatial overlays, and registry hot-reloading). However, the build graph does not reflect the completion posture implied by that architecture and by recent program-status documents. Specifically:

- `editor/spatial/` contains only header files (`elevation_brush_panel.h`, `prop_placement_panel.h`). No `.cpp` sources are registered in `CMakeLists.txt`, so these panels are not compiled into any product target.
- `engine/core/presentation/` contains ~25 files, of which only three are `.cpp` sources:
  - `presentation_runtime.cpp` — registered in `urpg_core`
  - `release_validation.cpp` — registered as its own executable (`urpg_presentation_release_validation`)
  - `profile_arena.cpp` — **not** registered in any build target
- The majority of `engine/core/presentation/` is header-only abstraction (translators, states, routers, adapters). Until those abstractions are backed by compiled implementation and linked into runtime/editor targets, the subsystem cannot be considered productized.

This ADR recorded an explicit status correction for these directories so the planning set at that time would remain trustworthy.

## Decision

1. **`editor/spatial/` — Productized for compiled panel scope**
   - The spatial editor panels (`ElevationBrushPanel`, `PropPlacementPanel`) have compiled `.cpp` implementations registered in `CMakeLists.txt` and linked into `urpg_core`.
   - They are exercised by `tests/unit/test_spatial_editor.cpp` and the new `tests/unit/test_spatial_editor.cpp` `[presentation][spatial][e2e]` test proving authoring output flows into `PresentationRuntime` frame generation.
   - Residual gap: deeper prop library integration and real renderer-backend preview remain future work.

2. **`engine/core/presentation/` — Productized with residual gaps**
   - The directory now has actively compiled and tested surfaces:
     - `presentation_runtime.cpp` (part of `urpg_core`, tested by `[presentation][runtime]` and `[presentation][spatial][e2e]`)
     - `presentation_bridge.cpp` (part of `urpg_core`, tested by `[presentation][bridge]`)
     - `release_validation.cpp` (standalone validation executable, now covers spatial-authoring→runtime consumption)
   - `profile_arena.cpp` remains intentionally out of `urpg_core` until the profile hot-reload path is fully implemented.
   - The renderer backend consumed by the presentation intent is still a mock/stub (`RenderBackendMock`); real OpenGL backend integration is a residual gap.

## Rationale

A subsystem is considered landed in URPG when it is built, test-registered, and reachable from a real runtime or editor entry point (see Remediation Principle #2 in `TECHNICAL_DEBT_REMEDIATION_PLAN.md`). The presentation/spatial subsystem now meets these criteria: compiled panels, compiled runtime and bridge, end-to-end authoring→runtime tests, and release-validation coverage. The remaining gaps (mock renderer backend, unregistered `profile_arena.cpp`) are explicitly scoped as residual future work rather than blocking productization claims.

## Consequences

- **Positive:** Program status documents now accurately reflect build-graph evidence including the compiled spatial panels, runtime-backed tests, and end-to-end authoring proof path.
- **Neutral:** The architectural direction from ADR-010 remains valid; this ADR updates the claimed productization status to match the current evidence.
- **Negative:** None. The honest residual gaps (mock renderer, profile arena) are documented and do not silently overclaim.
