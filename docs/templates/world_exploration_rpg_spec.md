# World Exploration RPG Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `world_exploration_rpg`

## Purpose

The `world_exploration_rpg` template covers travel-forward RPGs with route planning, fast travel, biome events, quest objectives, landmarks, world-map zoom, and export validation for connected route graphs.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | World map, route preview, biome events, and travel presentation. |
| `message_text_core` | Quest objective text, biome labels, landmark text, and prompts. |
| `save_data_core` | Discovered landmarks, fast-travel unlocks, and quest journal state. |
| `gameplay_ability_framework` | Travel actions, route tags, biome effects, and quest marker effects. |
| `export_validator` | Connected route graph, discoverable landmark, and quest-marker validation. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines world marker, route node, biome badge, and quest objective labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines route select, fast travel, biome enter, and quest update cues. |
| Input | `READY` | Starter projects expose world route, fast travel, biome event, and quest journal save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `world.marker`, `travel.fast`, `biome.name`, and `quest.objective`. |
| Performance | `READY` | Starter profile defines bounded biome, route, and quest-marker data. |

## Safe Scope Today

World exploration starter projects with route planning, fast travel, biome events, quest journal saves, WYSIWYG world/route/zoom surfaces, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
