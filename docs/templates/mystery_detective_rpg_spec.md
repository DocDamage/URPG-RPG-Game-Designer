# Mystery Detective RPG Template Spec

Status Date: 2026-04-28
Authority: canonical template spec for `mystery_detective_rpg`

## Purpose

The `mystery_detective_rpg` template covers clue-driven RPGs with case boards, interrogations, evidence presentation, deduction puzzles, branching dialogue, and diagnostics that prevent dead-end investigations.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Case board, clue list, evidence detail, and journal menus. |
| `message_text_core` | Interrogation dialogue, choices, variables, and clue text. |
| `save_data_core` | Case progress, discovered clues, deductions, and resolution state. |
| `gameplay_ability_framework` | Evidence effects, clue tags, and deduction rule composition. |
| `export_validator` | Reachable clue, resolution path, and dialogue variable validation. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `TemplateRuntimeProfile` defines clue card, case thread, evidence prompt, and deduction result labels. |
| Audio | `READY` | `TemplateRuntimeProfile` defines clue found, contradiction, case update, and case solved cues. |
| Input | `READY` | Starter projects expose clue board, interrogation dialogue, deduction puzzle, and case resolution save loops. |
| Localization | `READY` | `TemplateRuntimeProfile` requires `clue.found`, `case.thread`, `prompt.present`, and `case.solved`. |
| Performance | `READY` | Starter profile defines bounded clue graph and deduction path data. |

## Safe Scope Today

Mystery detective starter projects with case boards, evidence tags, interrogation previews, deduction puzzle logic, WYSIWYG dialogue/quest/puzzle surfaces, and export validation.

## Main Blockers

None for the claimed starter-template scope.

## Promotion Path

This template is `READY` for the starter-template scope implemented by `TemplateRuntimeProfile`, `ProjectTemplateGenerator`, the starter manifest, WYSIWYG showcase coverage, and template acceptance tests.
