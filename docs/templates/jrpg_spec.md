# JRPG Template Spec

Status Date: 2026-04-23  
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
| Accessibility | `PARTIAL` | Baseline accessibility governance (label checks, focus-order audits, contrast rules) is exercised via `AccessibilityAuditor` unit tests and the accessibility panel adapter. JRPG-specific text-speed control and input-hold alternatives for menu navigation are scaffolded; production enforcement is not yet landed. |
| Audio | `PARTIAL` | Audio mix preset governance and validator rules are exercised. JRPG-specific battle jingle and BGM-transition bars are scaffolded in `AudioMixPresets` but not fully validated end-to-end. |
| Input | `PARTIAL` | Menu and battle input paths are covered by unit tests. Controller-binding governance and cross-template remap validation are tested via `test_input_remap_store`. Full closure requires end-to-end remap regression for all menu/battle screens. |
| Localization | `PARTIAL` | Baseline message and menu localization patterns are exercised via `message_text_core` tests. Completeness metrics, font-set coverage, and per-region test fixtures are not yet landed. |
| Performance | `PARTIAL` | General RPG frame-time budgets are enforced through presentation runtime arena allocation and `profile_arena` tests. Large-world streaming and extended-play battle-sequence consistency are not production-validated. |

## Safe Scope Today

Wave 1 native-first JRPG projects using the four required subsystems (`ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core`). Bounded demos covering narrative dialogue, menu-driven party management, turn-based combat, and save/load loops are supported within current evidence. Projects must not claim full production-grade accessibility, audio mix, or localization completeness until the corresponding bars close.

## Main Blockers

1. **Battle and save closure remain open** — `battle_core` and `save_data_core` remain `PARTIAL` with human-review gates. JRPG cannot advance to `READY` until both close.
2. **Cross-cutting readiness bars are not complete** — All five bars (`accessibility`, `audio`, `input`, `localization`, `performance`) are `PARTIAL`. Production-grade claims require each bar to reach `READY` or explicitly bounded `PARTIAL` with accepted scope limits.

## Promotion Path

This template may advance to `READY` only when:

- All required subsystems (`ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core`) are `READY`.
- All cross-cutting bars are at least `PARTIAL` with accepted bounded-scope wording, and any bar remaining `PARTIAL` has explicit residual scope documented.
- Template-level acceptance tests (edit → preview → export) are landed and green.
- A human reviewer has completed the relevant signoff closure for each human-review-gated required subsystem.
