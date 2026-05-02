# Visual Novel Template Spec

Status Date: 2026-05-01
Authority: canonical template spec for `visual_novel`

## Purpose

The `visual_novel` template covers dialogue-heavy narrative games: branching story paths, character portraits, persistent choice tracking, and save-anywhere progression. It is the lightest-weight Wave 1 native template, depending only on the message and save subsystems, and is the target for story-first or visual-novel-style jam projects.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `message_text_core` | Dialogue display, name box, portrait integration, branching choice menus, and cutscene narration. |
| `save_data_core` | Persistent choice history, chapter markers, save-anywhere, and multi-slot management. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `READY` | `AccessibilityAuditor` label/focus/contrast tests, live adapters, renderer-derived contrast ingestion, and release top-level panel contrast coverage evidence are governed by `test_accessibility_auditor` and `check_accessibility_governance.ps1`. |
| Audio | `READY` | `AudioMixPresets` validator coverage, live backend smoke diagnostics, backend/device matrix fixtures, muted release fallback diagnostics, and `AudioMixPanel` matrix exposure are governed by `test_audio_mix_presets` and `check_audio_governance.ps1`. |
| Input | `READY` | Dialogue-advance, menu-navigation, auto-advance, skip-read, backlog, and text-speed controls are governed by `test_input_core` plus `VisualNovelPacingDocument` runtime preview commands and `VisualNovelPacingPanel` snapshots in `test_template_bar_quality`. |
| Localization | `READY` | The visual novel starter manifest declares required menu, dialogue control, speaker-name, and name-interpolation keys with en-US and ja-JP locale bundles plus Latin/CJK font profiles. `TemplateLocalizationAudit` and `test_template_bar_quality` verify manifest-driven required-key completeness and font-profile coverage. |
| Performance | `READY` | Visual-novel frame-time, arena-usage, and active-entity budgets are governed by template-specific dialogue-heavy performance budget diagnostics in `test_template_bar_quality`, presentation runtime arena allocation, and `profile_arena` tests for the claimed bounded starter scope. |

## Safe Scope Today

Dialogue-heavy visual-novel starter projects within the current native `message_text_core` and `save_data_core` scope. Branching narratives with persistent choice state, portrait sequences, chapter-level save points, backlog review, auto-advance, skip-read, text-speed controls, speaker-name interpolation, manifest-audited localization, governed accessibility coverage, audio backend/muted-fallback evidence, and bounded performance budgets are supported.

## Main Blockers

None for the claimed template scope.

## Promotion Path

This template may advance to `READY` only when:

- Both required subsystems (`message_text_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are `READY` for the claimed bounded starter scope.
- Template-level acceptance tests (dialogue flow → choice → save/load) are landed and green.
- Canonical spec governance checks pass (spec drift, authority line, status date, bar parity).
