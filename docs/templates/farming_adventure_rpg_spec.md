# Farming Adventure RPG Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `farming_adventure_rpg`

## Purpose

The `farming_adventure_rpg` template covers farming and town-life RPGs with crop calendars, weather effects, crafting/economy loops, relationship progression, tool-gated adventure areas, and save/export validation.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Farm, inventory, shop, relationship, and calendar menus. |
| `message_text_core` | NPC dialogue, gifting, weather text, and tutorial prompts. |
| `save_data_core` | Day, crop, relationship, inventory, and economy persistence. |
| `gameplay_ability_framework` | Tool gates, adventure abilities, and authored effects. |
| `export_validator` | Crop calendar, vendor stock, localization, and starter-data checks. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines crop stage, tool slot, weather badge, and relationship heart labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines crop water, harvest, shop open, and heart-up cues. |
| Input | `READY` | Starter projects expose crop day, forage adventure, crafting economy, and relationship save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `crop.ready`, `tool.upgrade`, `weather.rain`, and `friend.gift`. |
| Performance | `READY` | Starter profile defines bounded day, crop, vendor, and relationship data. |

## Safe Scope Today

Farming adventure starter projects with seasons, crop growth, weather preview, crafting/economy, relationship progression, tool-gated exploration, WYSIWYG map/crafting/social surfaces, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
