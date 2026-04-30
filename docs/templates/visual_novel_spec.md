# Visual Novel Template Spec

Status Date: 2026-04-28
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
| Accessibility | `PARTIAL` | Baseline accessibility audits (label, contrast, focus) cover dialogue and menu UI elements through `AccessibilityAuditor`. Visual-novel-specific pacing controls now have a saved WYSIWYG backlog/auto-advance/skip/read-speed panel and diagnostics; full renderer-derived coverage for every VN UI surface remains broader accessibility backlog. |
| Audio | `PARTIAL` | Audio mix preset governance is active. Visual-novel-specific ambient BGM layer management and voice-over duck rules are scaffolded; end-to-end validation is not yet landed. |
| Input | `PARTIAL` | Dialogue-advance and menu-navigation input paths are covered. Auto-advance and skip-read behavior are now governed through `VisualNovelPacingDocument`, runtime preview commands, and `VisualNovelPacingPanel` snapshots. |
| Localization | `READY` | The visual novel starter manifest declares required menu, dialogue control, speaker-name, and name-interpolation keys with en-US and ja-JP locale bundles plus Latin/CJK font profiles. `TemplateLocalizationAudit` and `test_template_bar_quality` verify manifest-driven required-key completeness and font-profile coverage. |
| Performance | `PARTIAL` | Presentation runtime and `profile_arena` tests provide frame-time budget coverage. Sustained dialogue-heavy session load and branching-graph traversal at production scale are not production-validated. |

## Safe Scope Today

Dialogue-heavy projects within the current native `message_text_core` and `save_data_core` scope. Branching narratives with persistent choice state, portrait sequences, chapter-level save points, backlog review, auto-advance, skip-read, text-speed controls, speaker-name interpolation, and manifest-audited localization are supported. Projects must not claim full production-grade accessibility coverage across every visual surface or voice-over integration until the corresponding bars close.

## Main Blockers

1. **Template-grade governance is not complete** — Canonical spec, bar-level acceptance tests, and machine-checked template governance (spec drift detection, project-audit coverage) are not yet fully landed.

## Promotion Path

This template may advance to `READY` only when:

- Both required subsystems (`message_text_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are at least `PARTIAL` with accepted bounded-scope wording.
- Template-level acceptance tests (dialogue flow → choice → save/load) are landed and green.
- Canonical spec governance checks pass (spec drift, authority line, status date, bar parity).
