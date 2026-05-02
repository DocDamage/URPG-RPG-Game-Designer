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
| Input | `PARTIAL` | Dialogue-advance and menu-navigation input paths are covered. Auto-advance and skip-read behavior are now governed through `VisualNovelPacingDocument`, runtime preview commands, and `VisualNovelPacingPanel` snapshots. |
| Localization | `READY` | The visual novel starter manifest declares required menu, dialogue control, speaker-name, and name-interpolation keys with en-US and ja-JP locale bundles plus Latin/CJK font profiles. `TemplateLocalizationAudit` and `test_template_bar_quality` verify manifest-driven required-key completeness and font-profile coverage. |
| Performance | `PARTIAL` | Presentation runtime and `profile_arena` tests provide frame-time budget coverage. Sustained dialogue-heavy session load and branching-graph traversal at production scale are not production-validated. |

## Safe Scope Today

Dialogue-heavy projects within the current native `message_text_core` and `save_data_core` scope. Branching narratives with persistent choice state, portrait sequences, chapter-level save points, backlog review, auto-advance, skip-read, text-speed controls, speaker-name interpolation, manifest-audited localization, governed accessibility coverage, and audio backend/muted-fallback evidence are supported. Projects must not claim full production-grade input or performance until those bars close.

## Main Blockers

1. **Template-grade governance is not complete** — Accessibility, audio, and localization are `READY`; input and performance remain `PARTIAL`. Canonical spec, bar-level acceptance tests, and machine-checked template governance (spec drift detection, project-audit coverage) are not yet fully landed.

## Promotion Path

This template may advance to `READY` only when:

- Both required subsystems (`message_text_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are at least `PARTIAL` with accepted bounded-scope wording.
- Template-level acceptance tests (dialogue flow → choice → save/load) are landed and green.
- Canonical spec governance checks pass (spec drift, authority line, status date, bar parity).
