# Action RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `arpg`

## Purpose

The `arpg` template covers real-time action RPG games with movement-driven combat, growth loops, and responsive input. It depends on `presentation_runtime` for real-time update cycles and `save_data_core` for persistent character progression.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Real-time render loop, movement update cycles, and spatial camera management. |
| `save_data_core` | Persistent character stats, equipment, and session state. |
| `gameplay_ability_framework` | Action skills, growth loop hooks, and combat ability integration. |
| `input_runtime` | Real-time action input mapping for attack, dodge, interact, and quick item. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines `health_bar`, `stamina_bar`, `quick_slot`, and `target_lock` labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines swing, dodge, hit-confirm, and low-health cues. |
| Input | `READY` | `TemplateRuntimeProfile` defines light attack, heavy attack, dodge, interact, and quick item actions. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `combat.dodge`, `combat.quick_item`, and `prompt.interact`. |
| Performance | `READY` | Starter profile defines `frame_budget_16ms` for real-time combat. |

## Safe Scope Today

Action RPG starter projects with real-time combat states, stamina/dodge/attack actions, closure visibility cues, growth loop, and save loop.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, and `TemplateCertification`.
