# ADR-011: Presentation and Spatial Subsystem Status — Incubating

**Status:** Approved  
**Date:** 2026-04-17  
**Supersedes / Clarifies:** ADR-010-presentation-completion (architectural patterns approved; build coverage and productization status explicitly downscoped)

Historical note: this ADR preserves the status correction recorded on 2026-04-17. For current execution and release truth, defer to the canonical program/remediation documents.

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

1. **`editor/spatial/` — Incubating**
   - The spatial editor panels (`ElevationBrushPanel`, `PropPlacementPanel`) exist as design headers and are exercised by `tests/unit/test_spatial_editor.cpp`, but they have no compiled `.cpp` implementation in the build graph.
   - They are **not** productized. They will be labeled incubating in all program documentation until `.cpp` sources are written, registered in `CMakeLists.txt`, and linked into an editor target.

2. **`engine/core/presentation/` — Incubating as a subsystem**
   - The directory is predominantly header-only architectural scaffolding.
   - The only actively compiled surfaces are:
     - `presentation_runtime.cpp` (part of `urpg_core`)
     - `release_validation.cpp` (standalone validation executable)
   - `profile_arena.cpp` is intentionally left out of `urpg_core` while the subsystem remains in incubation.
   - The existing registered tests (`test_presentation_runtime.cpp`, `test_spatial_editor.cpp`) were retained as anchor points for the future productization path.

## Rationale

A subsystem is not considered landed in URPG unless it is built, test-registered, and reachable from a real runtime or editor entry point (see Remediation Principle #2 in `TECHNICAL_DEBT_REMEDIATION_PLAN.md`). Because the bulk of the presentation/spatial code is headers-only and the editor panels are not compiled, claiming productized status would overstate completion and undermine planning trust. Retaining the compiled runtime surface (`presentation_runtime.cpp`) and the standalone release validator keeps the anchor points alive without misrepresenting the maturity of the overall subsystem.

## Consequences

- **Positive:** Program status documents now accurately reflect build-graph evidence. Stakeholders can plan around the need for future implementation work rather than assuming the spatial editor is already shipped.
- **Neutral:** The architectural direction from ADR-010 remains valid; this ADR only changes the claimed productization status.
- **Negative:** None. The headers, tests, and compiled surfaces remain untouched. When `.cpp` implementations are added and registered, the status can be upgraded to productized via a follow-up ADR or checklist closure.
