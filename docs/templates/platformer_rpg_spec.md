# Platformer RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `platformer_rpg`

## Purpose

The `platformer_rpg` template covers side-view action RPGs with authored jump physics, checkpoints, hazards, traversal upgrades, side-action combat, and export validation for safe spawns and reachable pickups.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Side-view camera, platformer movement preview, hazards, and map execution. |
| `battle_core` | Side-action combat resolution and hit feedback. |
| `save_data_core` | Checkpoints, unlocked traversal abilities, and recovery state. |
| `gameplay_ability_framework` | Dash, wall-slide, double-jump, and side-action abilities. |
| `export_validator` | Spawn, checkpoint, reachability, and physics budget validation. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines jump prompt, checkpoint, hazard warning, and ability gate labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines jump, land, checkpoint, and hazard-hit cues. |
| Input | `READY` | Starter projects expose platformer traversal, side-action combat, checkpoint upgrade, and save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `platform.jump`, `checkpoint.saved`, `hazard.warning`, and `ability.dash`. |
| Performance | `READY` | Starter profile defines bounded physics data, checkpoints, and traversal diagnostics. |

## Safe Scope Today

Platformer RPG starter projects with authored physics, side-action combat, traversal gates, checkpoints, WYSIWYG physics/combat/gate surfaces, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
