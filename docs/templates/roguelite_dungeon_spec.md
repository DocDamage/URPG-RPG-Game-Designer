# Roguelite Dungeon Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `roguelite_dungeon`

## Purpose

The `roguelite_dungeon` template covers deterministic run-based dungeon RPGs with seeded floor generation, encounter rooms, loot rewards, boss gates, meta progression, and export validation for replayable starter projects.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Room preview, dungeon traversal, and run-state presentation. |
| `battle_core` | Encounter and boss-room combat execution. |
| `save_data_core` | Meta progression, run summary, and recovery data. |
| `gameplay_ability_framework` | Reward abilities, tags, costs, and run modifiers. |
| `export_validator` | Seed, loot-table, and boss-drop validation before packaging. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines run seed, floor depth, loot choice, and exit portal labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines run-start, room-clear, loot-drop, and boss-warning cues. |
| Input | `READY` | Starter projects expose run setup, procedural floor, loot reward, and meta-progression save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `run.start`, `loot.choose`, `floor.depth`, and `boss.warning`. |
| Performance | `READY` | Starter profile defines bounded room budgets and deterministic floor seed data. |

## Safe Scope Today

Seeded roguelite dungeon starter projects with deterministic room stitching, encounter tables, weighted loot, boss rooms, meta-progression save data, WYSIWYG dungeon/VFX/save preview surfaces, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
