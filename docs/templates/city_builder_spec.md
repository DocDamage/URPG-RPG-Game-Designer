# City Builder Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `city_builder`

## Purpose

The `city_builder` template covers bounded City Builder starter projects with zoning, resources, placement, services, generated starter data, runtime profile evidence, diagnostics, and export validation.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Required for bounded City Builder starter project generation, preview, save, and export evidence. |
| `message_text_core` | Required for bounded City Builder starter project generation, preview, save, and export evidence. |
| `save_data_core` | Required for bounded City Builder starter project generation, preview, save, and export evidence. |
| `presentation_runtime` | Required for bounded City Builder starter project generation, preview, save, and export evidence. |
| `export_validator` | Required for bounded City Builder starter project generation, preview, save, and export evidence. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Audio | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Input | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Localization | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |
| Performance | `READY` | Governed by TemplateRuntimeProfile evidence, starter manifest data, certification loops, and template truth gates for the claimed starter scope. |

## Safe Scope Today

City Builder starter projects with zoning, resources, placement, services, WYSIWYG showcase coverage, saved project data, runtime hooks, diagnostics, and export validation.

## Main Blockers

None for the claimed template scope.

## Promotion Path

This template remains `READY` while its runtime profile, starter manifest, readiness row, spec bars, certification loops, and export validation evidence stay aligned under the template truth gates.
