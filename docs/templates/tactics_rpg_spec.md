# Tactics RPG Template Spec

Status Date: 2026-04-23
Authority: canonical template spec for `tactics_rpg`

## Purpose

The `tactics_rpg` template covers grid-based tactical combat games with scenario authoring, map-driven unit placement, and deterministic turn-order progression. It extends battle_core and presentation_runtime with grid mechanics and scenario authoring workflows.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `battle_core` | Turn-based combat execution, including turn-order, action dispatch, and outcome processing. |
| `presentation_runtime` | Grid rendering, unit sprites, map layers, and camera management. |
| `save_data_core` | Persistent scenario progress, unit roster, and progression state. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `PLANNED` | No tactics-specific accessibility governance yet; baseline auditor applies. |
| Audio | `PLANNED` | Tactics-specific audio cues and spatial audio contracts are undefined. |
| Input | `PARTIAL` | Grid navigation and battle action input are bounded; see `test_input_remap_store.cpp` and `test_input_core.cpp`. |
| Localization | `PLANNED` | Baseline message/menu localization patterns apply; tactics-specific completeness check pending. |
| Performance | `PARTIAL` | General presentation_runtime performance evidence in `test_presentation_runtime.cpp`; large-map tactic battles untested. |

## Safe Scope Today

Bounded tactics demos that use `battle_core`, `presentation_runtime`, and `save_data_core` within their current landed scope. Grid-based combat authoring using existing turn-order and action-dispatch surfaces is safe provided it does not claim production-grade scenario authoring or full accessibility governance.

## Main Blockers

1. **Scenario authoring** — No canonical scenario authoring format or editor workflow exists.
2. **Progression framework** — Template-level campaign/chapter progression is not defined.
3. **Cross-cutting readiness bars** — accessibility, audio, and localization bars remain PLANNED.

## Promotion Path

This template may advance to `PARTIAL` once:

- `battle_core` and `presentation_runtime` have tactics-relevant evidence.
- A scenario authoring schema draft is referenced in readiness records.

It may advance to `READY` only when all required subsystems are `READY`, all cross-cutting bars are at least `PARTIAL`, and tactics-specific scenario authoring evidence is landed.
