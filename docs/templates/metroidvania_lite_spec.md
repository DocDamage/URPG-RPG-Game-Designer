# Metroidvania Lite Template Spec

Status Date: 2026-04-28
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
| Accessibility | `READY` | `TemplateRuntimeProfile` defines `map_region`, `ability_gate`, `checkpoint`, and `upgrade_pickup` labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines `ability_unlock`, `gate_open`, `checkpoint`, and `low_health` cues. |
| Input | `READY` | Starter projects expose traversal, ability-gate, map-unlock, and save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `ability.dash`, `ability.wall_jump`, `region.locked`, and `map.checkpoint`. |
| Performance | `READY` | Starter profile defines `room_budget` with bounded region/gate data. |

## Safe Scope Today

Metroidvania-lite starter projects with dash/wall-jump/double-jump traversal gates, region unlock data, map loop, and save loop.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, and `TemplateCertification`.
