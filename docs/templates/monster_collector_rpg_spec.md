# Monster Collector RPG Template Spec

Status Date: 2026-04-20  
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
| Accessibility | `PLANNED` | No template-grade accessibility governance yet. |
| Audio | `PARTIAL` | Basic audio hooks exist; collection-specific audio bars are undefined. |
| Input | `PARTIAL` | Menu and battle input are covered; collection-specific interactions are not. |
| Localization | `PLANNED` | Planned once message and menu localization patterns are finalized. |
| Performance | `PARTIAL` | General RPG budgets apply; large collection scaling is untested. |

## Safe Scope Today

Exploration of collection-driven combat and party assembly using landed Wave 1 subsystems (`ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core`) plus the `gameplay_ability_framework`. Bounded demos that rely on existing menu, message, save, and ability surfaces are safe, provided they do not claim production-grade collection governance.

## Main Blockers

1. **Dedicated collection schema** — No canonical schema for entity collection, evolution, or storage yet.
2. **Capture mechanics** — Capture logic and success/failure contracts are not defined or landed.
3. **Template-grade governance** — Cross-cutting bars, promotion rules, and audit checks for this template are not yet enforced.

## Promotion Path

This template may advance to `PARTIAL` once:

- `gameplay_ability_framework` reaches `READY` or `PARTIAL` with collection-relevant evidence.
- A collection schema draft exists and is referenced in the readiness records.

It may advance to `READY` only when all required subsystems are `READY`, all cross-cutting bars are at least `PARTIAL`, and template-specific audit checks are landed.
