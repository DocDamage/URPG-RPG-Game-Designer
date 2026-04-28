# Survival Horror RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `survival_horror_rpg`

## Purpose

The `survival_horror_rpg` template covers exploration-heavy RPGs with lighting pressure, scarce resources, puzzle locks, safe rooms, threat pacing, and diagnostics that prevent softlocks in starter projects.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Lighting, weather, visibility, and map-preview execution. |
| `message_text_core` | Clues, puzzle prompts, item descriptions, and dialogue preview. |
| `battle_core` | Threat encounters and scarce-resource combat outcomes. |
| `save_data_core` | Safe-room saves, inventory persistence, and recovery state. |
| `accessibility_auditor` | Readability and warning-label governance for low-visibility scenes. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines flashlight meter, inventory slots, safe-room prompt, and threat-warning labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines heartbeat, distant threat, safe-room theme, and puzzle unlock cues. |
| Input | `READY` | Starter projects expose exploration, scarce-resource, puzzle-unlock, and safe-room save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `item.battery`, `prompt.hide`, `safe_room.save`, and `puzzle.locked`. |
| Performance | `READY` | Starter profile defines bounded lighting and resource-pressure data for preview scenes. |

## Safe Scope Today

Survival horror RPG starter projects with flashlight/lighting preview, scarce resource tables, safe-room saves, puzzle lock diagnostics, dialogue clue preview, and reachable export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
