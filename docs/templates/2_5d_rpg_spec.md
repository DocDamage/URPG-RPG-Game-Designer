# 2.5D RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `2_5d_rpg`

## Purpose

The `2_5d_rpg` template covers games that use a raycast-based or faux-3D presentation mode layered over 2D RPG mechanics. It depends on the 2.5D presentation lane and is a mandatory first-class template target for projects that want depth-cue exploration without a full 3D engine.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Raycast rendering, camera, and spatial updates for the 2.5D mode. |
| `save_data_core` | Persistent progression, settings, and session recovery. |
| `raycast_renderer` | Raycast frame casting and blocking-cell authoring adapter. |
| `spatial_projection` | Spatial presentation profile and projection metadata. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines `depth_cue`, `navigation_prompt`, `interact_target`, and `minimap` labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines `footstep_near`, `door_open`, `spatial_ambience`, and `interact_prompt` cues. |
| Input | `READY` | Starter projects expose spatial navigation, raycast authoring, and save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `area.name`, `prompt.forward`, `prompt.turn`, and `prompt.interact`. |
| Performance | `READY` | Starter profile defines `raycast_budget` and 640x480/fov starter config. |

## Safe Scope Today

2.5D RPG starter projects with raycast authoring adapter, blocking-cell validation, spatial navigation, export validation contract, and save loop.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, and `TemplateCertification`.
