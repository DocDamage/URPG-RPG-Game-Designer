# Monster Tamer Arena Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `monster_tamer_arena`

## Purpose

The `monster_tamer_arena` template covers team_rules, league_ladder, breeding, evolution starter projects with authored data, WYSIWYG preview surfaces, saved project state, runtime execution, diagnostics, and export validation.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Template menus and creator-facing setup surfaces. |
| `message_text_core` | Dialogue, prompts, labels, and localization text. |
| `battle_core` | Encounter, challenge, match, or conflict resolution where the template uses gameplay outcomes. |
| `save_data_core` | Persistent progression, authored state, and recovery data. |
| `export_validator` | Starter-data completeness and package validation for the template scope. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | TemplateRuntimeProfile defines required labels for the claimed starter scope. |
| Audio | `READY` | TemplateRuntimeProfile defines opening, confirm, success, and warning cues. |
| Input | `READY` | Starter projects expose the required loops: team_build_loop, arena_battle_loop, evolution_training_loop, save_loop. |
| Localization | `READY` | TemplateRuntimeProfile requires title, action, status, and warning keys. |
| Performance | `READY` | Starter profile defines bounded authored data and a template budget marker. |

## Safe Scope Today

Monster Tamer Arena starter projects with team_rules, league_ladder, breeding, evolution, WYSIWYG showcase coverage, saved project data, runtime hooks, diagnostics, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by TemplateRuntimeProfile, ProjectTemplateGenerator, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
