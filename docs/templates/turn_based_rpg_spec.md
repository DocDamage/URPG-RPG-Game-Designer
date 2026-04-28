# Turn-Based RPG Template Spec

Status Date: 2026-04-28
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
| Accessibility | `PARTIAL` | Baseline accessibility governance (label audits, focus-order, contrast) is active via `AccessibilityAuditor`. Turn-based-RPG-specific expectations (battle-menu hold alternatives, status-screen readability, colorblind unit highlighting) are scaffolded but not fully enforced. |
| Audio | `PARTIAL` | Audio mix preset governance applies. Turn-based-RPG-specific battle audio transitions and victory fanfare sequencing are scaffolded in `AudioMixPresets` but not fully validated. |
| Input | `PARTIAL` | Battle and menu input paths are covered by unit tests. Controller remap and cross-template binding governance via `test_input_remap_store` apply. Full binding closure for all battle actions is pending. |
| Localization | `PARTIAL` | Baseline message localization patterns from `message_text_core` apply. Per-template completeness metrics, battle-text, and status-effect localization are not yet measured. |
| Performance | `PARTIAL` | Presentation runtime arena and `profile_arena` frame-time budget tests apply. Battle-sequence scalability under extended play and large-unit-count scenarios are not production-validated. |

## Safe Scope Today

A conservative turn-based template slice using `message_text_core`, `battle_core`, and `save_data_core`. Combat-focused projects without requiring a full main menu or dialogue system are within scope. Projects must not claim full production-grade accessibility, localization completeness, or audio mix fidelity until the corresponding bars close.

## Main Blockers

1. **Cross-cutting readiness bars and dedicated template governance remain incomplete** — All five bars (`accessibility`, `audio`, `input`, `localization`, `performance`) are `PARTIAL`. Machine-checked template governance (spec drift detection, project-audit coverage) is not yet fully landed.

## Promotion Path

This template may advance to `READY` only when:

- All required subsystems (`message_text_core`, `battle_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are at least `PARTIAL` with accepted bounded-scope wording.
- Template-level acceptance tests (combat loop → save → reload) are landed and green.
- A human reviewer has completed the signoff closure for each human-review-gated required subsystem (`battle_core`, `save_data_core`).
