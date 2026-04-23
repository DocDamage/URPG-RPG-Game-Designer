# Metroidvania Lite Template Spec

Status Date: 2026-04-23  
Authority: canonical template spec for `metroidvania_lite`

## Purpose

The `metroidvania_lite` template covers 2D action-exploration games with ability-gated progression, map unlock systems, and traversal mechanics. It sits at the intersection of the `presentation_runtime` and `gameplay_ability_framework`, requiring real-time movement and environmental interaction.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Real-time rendering, camera, and spatial updates for action exploration. |
| `save_data_core` | Persistent map unlock state, ability unlocks, and checkpoint recovery. |
| `gameplay_ability_framework` | Traversal abilities (dash, double-jump, wall-jump) and combat skills. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `PLANNED` | No template-grade accessibility governance yet. |
| Audio | `PLANNED` | Action-audio requirements are undefined. |
| Input | `PARTIAL` | Basic action input exists; traversal-specific input contracts are not landed. |
| Localization | `PLANNED` | Planned once message localization patterns are finalized. |
| Performance | `PARTIAL` | General budgets exist; real-time action and large-map scaling are untested. |

## Safe Scope Today

2D action exploration using `presentation_runtime` and `gameplay_ability_framework`. Current evidence supports real-time movement and simple combat slices, but production-grade traversal, map-gating, and progression contracts are still required before broader release-facing claims.

## Main Blockers

1. **Traversal mechanics** — Dash, wall-jump, and ability-gated movement are not defined or landed.
2. **Map unlock system** — Room-unlock and map-reveal mechanics are not established.
3. **Ability-gated progression** — Progression gates driven by `gameplay_ability_framework` remain future work.

## Promotion Path

This template may advance to `PARTIAL` once:

- `presentation_runtime` reaches `READY` or `PARTIAL` with action-relevant evidence.
- A traversal or map-gating lane reaches `PARTIAL` with canonical evidence.

It may advance to `READY` only when all required subsystems are `READY`, all cross-cutting bars are at least `PARTIAL`, and action-specific editor flows exist.
