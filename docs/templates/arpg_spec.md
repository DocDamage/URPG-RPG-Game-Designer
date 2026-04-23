# Action RPG Template Spec

Status Date: 2026-04-23
Authority: canonical template spec for `arpg`

## Purpose

The `arpg` template covers real-time action RPG games with movement-driven combat, growth loops, and responsive input. It depends on `presentation_runtime` for real-time update cycles and `save_data_core` for persistent character progression.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Real-time render loop, movement update cycles, and spatial camera management. |
| `save_data_core` | Persistent character stats, equipment, and session state. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `PLANNED` | No arpg-specific accessibility governance yet; baseline auditor applies. |
| Audio | `PLANNED` | Real-time audio cue contracts for combat and movement are undefined. |
| Input | `PARTIAL` | Real-time input dispatch present in `presentation_runtime`; arpg-specific remap governance is bounded. |
| Localization | `PLANNED` | Baseline message/menu localization patterns apply; arpg-specific completeness check pending. |
| Performance | `PARTIAL` | Presentation runtime arena/profile evidence in `test_presentation_runtime.cpp`; real-time combat perf at production scale is untested. |

## Safe Scope Today

Bounded arpg demos using `presentation_runtime` real-time update cycles and `save_data_core` for character state. Current evidence supports basic movement and growth loop proofs, but production-grade arpg template closure requires additional closure-visibility artifacts and accessibility/audio governance.

## Main Blockers

1. **Closure visibility** — Template-level audit checks, spec-to-readiness parity, and matrix row are not fully established.
2. **Movement contracts** — Canonical dash/roll/attack movement lane is not landed.
3. **Cross-cutting readiness bars** — accessibility, audio, and localization bars remain PLANNED.

## Promotion Path

This template may advance to `PARTIAL` once:

- `presentation_runtime` has arpg-relevant movement and update-cycle evidence.
- Closure-visibility artifacts (audit contract, spec, readiness row) are all consistent.

It may advance to `READY` only when all required subsystems are `READY`, all cross-cutting bars are at least `PARTIAL`, and arpg-specific movement and growth loop evidence is landed.
