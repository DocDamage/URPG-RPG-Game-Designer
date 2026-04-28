# Cozy Life RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `cozy_life_rpg`

## Purpose

The `cozy_life_rpg` template covers dialogue-heavy life-simulation games with scheduling, social relationships, and low-stakes progression. It prioritizes narrative depth, character interaction, and persistent world state over combat or action mechanics.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Menus for inventory, relationships, calendar, and world navigation. |
| `message_text_core` | Heavy reliance on dialogue, choice prompts, and narrative text. |
| `save_data_core` | Persistent world state, schedule snapshots, and social flag tracking. |
| `crafting` | Starter recipes and craft preview loop. |
| `economy` | Daily route, rewards, costs, and balance simulation. |
| `shop` | Vendor stock and buying/selling starter flow. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines `calendar`, `relationship_log`, `recipe_list`, and `vendor_stock` labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines `day_start`, `craft_complete`, `friendship_up`, and `shop_open` cues. |
| Input | `READY` | Starter projects expose daily life, relationship, crafting, economy, and save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `day.morning`, `activity.craft`, `relationship.gift`, and `shop.buy`. |
| Performance | `READY` | Starter profile defines `day_tick_budget` and bounded schedule/social data. |

## Safe Scope Today

Cozy life starter projects with day phases, NPC routines, relationships, crafting, economy, vendors, and save loop.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, and `TemplateCertification`.
