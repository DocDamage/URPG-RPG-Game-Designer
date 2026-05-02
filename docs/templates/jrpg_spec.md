# JRPG Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `jrpg`

## Purpose

The `jrpg` template covers classic Japanese-style RPGs: party management, turn-based battle, persistent world progression, and narrative-driven dialogue. It is the Wave 1 native-first baseline template and the primary target for the first publicly demonstrable engine slice.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `ui_menu_core` | Party management, inventory, main menu, equip/status screens. |
| `message_text_core` | Dialogue, battle messages, cutscene narration, and name-insertion. |
| `battle_core` | Turn-based combat resolution, party/enemy state, and action dispatch. |
| `save_data_core` | Persistent progression, autosave recovery, and multi-slot management. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `AccessibilityAuditor` label/focus/contrast tests, live adapters, renderer-derived contrast ingestion, and release top-level panel contrast coverage evidence are governed by `test_accessibility_auditor` and `check_accessibility_governance.ps1`. |
| Audio | `READY` | `AudioMixPresets` validator coverage, live backend smoke diagnostics, backend/device matrix fixtures, muted release fallback diagnostics, and `AudioMixPanel` matrix exposure are governed by `test_audio_mix_presets` and `check_audio_governance.ps1`. |
| Input | `READY` | The JRPG starter manifest and generated project input subsystem now declare the full menu/battle action set (`Move*`, `Confirm`, `Cancel`, `Menu`, `PageLeft`, `PageRight`, `BattleAttack`, `BattleSkill`, `BattleItem`, `BattleDefend`, `BattleEscape`) with keyboard and controller defaults. `test_template_acceptance` verifies manifest and generator closure, and `test_input_remap_store` proves the action names survive save/load. Touch remains a hit-test UI/world path and reports `touch_binding_unsupported` for remap profiles. |
| Localization | `READY` | The JRPG starter manifest declares required menu, battle, and system keys with en-US and ja-JP locale bundles plus Latin/CJK font profiles. `TemplateLocalizationAudit` and `test_template_bar_quality` verify manifest-driven required-key completeness and font-profile coverage. |
| Performance | `READY` | JRPG frame-time, arena-usage, and active-entity budgets are governed by template-specific performance budget diagnostics in `test_template_bar_quality`, presentation runtime arena allocation, and `profile_arena` tests for the claimed bounded starter scope. |

## Safe Scope Today

Wave 1 native-first JRPG starter projects using the four required subsystems (`ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core`). Bounded demos covering narrative dialogue, menu-driven party management, turn-based combat, save/load loops, governed input, manifest-audited localization, governed accessibility coverage, audio backend/muted-fallback evidence, WYSIWYG proof descriptors, and bounded performance budgets are supported within current evidence.

## Main Blockers

None for the claimed template scope.

## Promotion Path

This template may advance to `READY` only when:

- All required subsystems (`ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are `READY` for the claimed bounded starter scope.
- Template-level acceptance tests (edit → preview → export) are landed and green.
- Required subsystem signoffs are approved or not required for the claimed template scope.
