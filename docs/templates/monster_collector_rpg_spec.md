# Monster Collector RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `monster_collector_rpg`

## Purpose

The `monster_collector_rpg` template covers games where players explore a world, collect creatures or entities into a party, and engage in combat driven by those collections. It extends the baseline RPG template with collection mechanics, party assembly, and ability-driven combat.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Party management, collection inventory, and menu-driven assembly flows. |
| `message_text_core` | Dialogue, collection flavor text, and narrative context. |
| `battle_core` | Turn-based or hybrid combat using collected entities. |
| `save_data_core` | Persistent collection state, party snapshots, and progression. |
| `gameplay_ability_framework` | Collection mechanics, capture abilities, and entity-specific skills. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines `creature_roster`, `capture_button`, `party_slot`, and `ability_grid` labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines `encounter_start`, `capture_throw`, `capture_success`, and `capture_breakout` cues. |
| Input | `READY` | Starter projects expose `capture_loop`, `party_assembly_loop`, `battle_loop`, and `save_loop`. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `creature.name`, `capture.success`, `capture.failed`, and `party.swap`. |
| Performance | `READY` | Starter profile defines bounded species, party, and encounter-budget data. |

## Safe Scope Today

Monster collector starter projects with species definitions, capture formula, party assembly, battle loop, and save loop.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, and `TemplateCertification`.
