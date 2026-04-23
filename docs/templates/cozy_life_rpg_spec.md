# Cozy Life RPG Template Spec

Status Date: 2026-04-23  
Authority: canonical template spec for `cozy_life_rpg`

## Purpose

The `cozy_life_rpg` template covers dialogue-heavy life-simulation games with scheduling, social relationships, and low-stakes progression. It prioritizes narrative depth, character interaction, and persistent world state over combat or action mechanics.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Menus for inventory, relationships, calendar, and world navigation. |
| `message_text_core` | Heavy reliance on dialogue, choice prompts, and narrative text. |
| `save_data_core` | Persistent world state, schedule snapshots, and social flag tracking. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `PARTIAL` | Baseline accessibility governance applies; cozy-life-specific UI flows remain deferred pending scheduling/social lane proof. |
| Audio | `PARTIAL` | Ambient and music hooks exist; life-sim-specific audio bars are undefined. |
| Input | `PARTIAL` | Menu and dialogue input are covered; life-sim-specific interactions are not. |
| Localization | `PARTIAL` | Baseline message/menu localization patterns apply; scheduling and economy-specific completeness remains deferred. |
| Performance | `PARTIAL` | General RPG budgets apply; dialogue-heavy life-sim scheduling remains untested. |

## Safe Scope Today

Dialogue-heavy life-sim exploration within the current message, save, and menu scope. Projects that rely primarily on `message_text_core`, `save_data_core`, and `ui_menu_core` for narrative and persistence are safe, provided they do not claim production-grade scheduling or social mechanics.

## Main Blockers

1. **Scheduling system** — No time-of-day or calendar-driven event system is defined.
2. **Social relationship mechanics** — Relationship graph and social-state persistence are not landed.
3. **Crafting/economy lanes** — Item crafting, economy balance, and shop systems remain future work.

## Promotion Path

This template may advance to `PARTIAL` once:

- A scheduling or social lane reaches `PARTIAL` with canonical evidence.
- Template-specific governance and cross-cutting bars are introduced.

It may advance to `READY` only when all required subsystems are `READY`, all cross-cutting bars are at least `PARTIAL`, and life-sim-specific editor flows exist.
