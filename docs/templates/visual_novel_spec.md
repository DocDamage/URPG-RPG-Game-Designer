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
| Accessibility | `PARTIAL` | Baseline accessibility audits (label, contrast, focus) cover dialogue and menu UI elements through `AccessibilityAuditor`. Visual-novel-specific pacing controls (text auto-advance toggle, read-speed override, backlog reader) are scaffolded but not production-enforced. |
| Audio | `PARTIAL` | Audio mix preset governance is active. Visual-novel-specific ambient BGM layer management and voice-over duck rules are scaffolded; end-to-end validation is not yet landed. |
| Input | `PARTIAL` | Dialogue-advance and menu-navigation input paths are covered. Accessibility-input alternatives (auto-advance, skip) are scaffolded but not yet fully governed. |
| Localization | `PARTIAL` | Baseline message localization patterns from `message_text_core` tests apply. Visual-novel-specific multi-voice character name insertion and per-language font-set coverage are not yet measured. |
| Performance | `PARTIAL` | Presentation runtime and `profile_arena` tests provide frame-time budget coverage. Sustained dialogue-heavy session load and branching-graph traversal at production scale are not production-validated. |

## Safe Scope Today

Dialogue-heavy projects within the current native `message_text_core` and `save_data_core` scope. Branching narratives with persistent choice state, portrait sequences, and chapter-level save points are supported. Projects must not claim production-grade accessibility (e.g. backlog or auto-advance), voice-over integration, or locale-complete localization until the corresponding bars close.

## Main Blockers

1. **Template-grade governance is not complete** — Canonical spec, bar-level acceptance tests, and machine-checked template governance (spec drift detection, project-audit coverage) are not yet fully landed.

## Promotion Path

This template may advance to `READY` only when:

- Both required subsystems (`message_text_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are at least `PARTIAL` with accepted bounded-scope wording.
- Template-level acceptance tests (dialogue flow → choice → save/load) are landed and green.
- Canonical spec governance checks pass (spec drift, authority line, status date, bar parity).
