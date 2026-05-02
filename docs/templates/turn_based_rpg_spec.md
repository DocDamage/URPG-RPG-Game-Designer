# Turn-Based RPG Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `turn_based_rpg`

## Purpose

The `turn_based_rpg` template covers turn-based tactical or strategic RPGs that lean on combat and progression without requiring the full party-management and narrative surface area of the JRPG template. It is a conservative template slice for projects that want a lean combat-and-save loop without a mandatory main-menu or full dialogue system.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `message_text_core` | Battle log messages, combat narration, and minimal status text. |
| `battle_core` | Turn-based combat resolution, action dispatch, and unit state management. |
| `save_data_core` | Persistent progression, autosave, and session recovery. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `AccessibilityAuditor` label/focus/contrast tests, live adapters, renderer-derived contrast ingestion, and release top-level panel contrast coverage evidence are governed by `test_accessibility_auditor` and `check_accessibility_governance.ps1`. |
| Audio | `READY` | `AudioMixPresets` validator coverage, live backend smoke diagnostics, backend/device matrix fixtures, muted release fallback diagnostics, and `AudioMixPanel` matrix exposure are governed by `test_audio_mix_presets` and `check_audio_governance.ps1`. |
| Input | `READY` | The turn-based RPG starter manifest and generated project input subsystem now declare the full navigation/menu/battle action set (`Move*`, `Confirm`, `Cancel`, `Menu`, `PageLeft`, `PageRight`, `BattleAttack`, `BattleSkill`, `BattleItem`, `BattleDefend`, `BattleEscape`) with keyboard and controller defaults. `test_template_acceptance` verifies manifest and generator closure, and `test_input_remap_store` proves the action names survive save/load. Touch remains a hit-test UI/world path and reports `touch_binding_unsupported` for remap profiles. |
| Localization | `READY` | The turn-based RPG starter manifest declares required command, battle, result, turn-order, and status-effect keys with en-US and ja-JP locale bundles plus Latin/CJK font profiles. `TemplateLocalizationAudit` and `test_template_bar_quality` verify manifest-driven required-key completeness and font-profile coverage. |
| Performance | `PARTIAL` | Presentation runtime arena and `profile_arena` frame-time budget tests apply. Battle-sequence scalability under extended play and large-unit-count scenarios are not production-validated. |

## Safe Scope Today

A conservative turn-based template slice using `message_text_core`, `battle_core`, and `save_data_core`. Combat-focused projects without requiring a full main menu or dialogue system are within scope, including manifest-audited battle/status localization, governed accessibility coverage, and audio backend/muted-fallback evidence. Projects must not claim full production-grade performance until that bar closes.

## Main Blockers

1. **Cross-cutting readiness bars and dedicated template governance remain incomplete** — Accessibility, audio, input, and localization are `READY`; `performance` remains `PARTIAL`. Machine-checked template governance (spec drift detection, project-audit coverage) is not yet fully landed.

## Promotion Path

This template may advance to `READY` only when:

- All required subsystems (`message_text_core`, `battle_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are at least `PARTIAL` with accepted bounded-scope wording.
- Template-level acceptance tests (combat loop → save → reload) are landed and green.
- Required subsystem signoffs are approved or not required for the claimed template scope.
