# Digital Board Game Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `board_game_digital`

## Purpose

The `board_game_digital` template covers bounded Digital Board Game starter projects with turn_order, pieces, cards, local_multiplayer, generated starter data, runtime profile evidence, diagnostics, and export validation.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Required for bounded Digital Board Game starter project generation, preview, save, and export evidence. |
| `message_text_core` | Required for bounded Digital Board Game starter project generation, preview, save, and export evidence. |
| `save_data_core` | Required for bounded Digital Board Game starter project generation, preview, save, and export evidence. |
| `presentation_runtime` | Required for bounded Digital Board Game starter project generation, preview, save, and export evidence. |
| `export_validator` | Required for bounded Digital Board Game starter project generation, preview, save, and export evidence. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Audio | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Input | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Localization | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Performance | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |

## Safe Scope Today

Digital Board Game starter projects with turn_order, pieces, cards, local_multiplayer, WYSIWYG showcase coverage, saved project data, runtime hooks, diagnostics, and export validation.

## Main Blockers

None for the claimed template scope.

## Promotion Path

This template remains `READY` while its runtime profile, starter manifest, readiness row, spec bars, certification loops, and export validation evidence stay aligned under the template truth gates.
