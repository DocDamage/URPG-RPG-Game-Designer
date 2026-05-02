# Card Battler RPG Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `card_battler_rpg`

## Purpose

The `card_battler_rpg` template covers RPGs built around deck construction, visible enemy intent, card costs, reward drafts, collection persistence, and export validation for starter decks and reward pools.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Deck builder, collection, reward draft, and card detail menus. |
| `battle_core` | Card-battle turns, enemy intent, damage, block, and resolution. |
| `save_data_core` | Deck, collection, run reward, and campaign progression persistence. |
| `gameplay_ability_framework` | Card tags, costs, cooldown-like resource rules, and effects. |
| `export_validator` | Starter deck, reward pool, and card localization validation. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines card name, cost, enemy intent, and draw-pile count labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines card draw, card play, block gain, and enemy intent cues. |
| Input | `READY` | Starter projects expose deck build, card battle, reward draft, and collection save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `card.strike`, `card.guard`, `battle.intent`, and `reward.choose`. |
| Performance | `READY` | Starter profile defines bounded deck, hand, reward, and turn-budget data. |

## Safe Scope Today

Card battler RPG starter projects with deck building, hand/resource rules, visible enemy intent, reward drafts, collection saves, WYSIWYG ability/VFX/export preview surfaces, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
