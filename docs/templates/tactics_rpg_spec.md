# Tactics RPG Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `tactics_rpg`

## Purpose

The `tactics_rpg` template covers grid-based tactical combat games with scenario authoring, map-driven unit placement, and deterministic turn-order progression. It extends battle_core and presentation_runtime with grid mechanics and scenario authoring workflows.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `battle_core` | Turn-based combat execution, including turn-order, action dispatch, and outcome processing. |
| `presentation_runtime` | Grid rendering, unit sprites, map layers, and camera management. |
| `save_data_core` | Persistent scenario progress, unit roster, and progression state. |
| `accessibility_auditor` | Template-specific grid, roster, cursor, and action-menu labels. |
| `audio_mix_presets` | Template-specific unit, movement, attack-preview, and turn-start cue ownership. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines tactics grid labels, unit roster labels, grid cursor labels, action-menu labels, and grid contrast mode. |
| Audio | `READY` | `TemplateRuntimeProfile` defines `unit_select`, `move_confirm`, `attack_preview`, and `turn_start` cues. |
| Input | `READY` | Starter projects expose `tactical_battle_loop` and `scenario_authoring_loop` with grid/action commands. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `scenario.title`, `objective.primary`, `action.move`, and `action.attack`. |
| Performance | `READY` | Starter profile defines a bounded 12x10 grid and `turn_budget_16ms`. |

## Safe Scope Today

Grid tactics starter projects with scenario authoring, deployment zones, deterministic turn-order, cross-cutting bars, and save loop.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, and `TemplateCertification`.
