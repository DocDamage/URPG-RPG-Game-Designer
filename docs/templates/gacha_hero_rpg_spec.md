# Gacha Hero RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `gacha_hero_rpg`

## Purpose

The `gacha_hero_rpg` template covers offline-configured hero collection RPGs with summon banners, pity counters, roster management, party battle loops, and export validation for rates and saved pity state.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Summon, roster, rate detail, and party setup menus. |
| `message_text_core` | Banner copy, hero text, rate details, and summon-result messages. |
| `battle_core` | Party battle execution for collected heroes. |
| `save_data_core` | Roster, pity state, currencies, and duplicate conversion state. |
| `export_validator` | Rate totals, featured-unit references, and pity persistence validation. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines summon button, rate details, pity counter, and hero roster slot labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines summon start, rarity reveal, SSR pull, and party-ready cues. |
| Input | `READY` | Starter projects expose summon banner, hero roster, party battle, and offline pity save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `summon.pull`, `rates.details`, `hero.roster`, and `pity.counter`. |
| Performance | `READY` | Starter profile defines bounded roster, banner, and simulation data. |

## Safe Scope Today

Local/offline gacha hero starter projects with authored banners, pity simulation, roster persistence, party battle, WYSIWYG summon/roster surfaces, and export validation. This does not claim live payments or external store services.

## Main Blockers

None for the claimed local/offline starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
