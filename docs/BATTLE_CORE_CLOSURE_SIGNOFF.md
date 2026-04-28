# Battle Core — Wave 1 Closure Sign-off

> **Status:** `READY`
> **Purpose:** Approved closure artifact for Wave 1 battle-core scope.
> **Date:** 2026-04-28
> **Rule:** This document records release-owner approval for the bounded Wave 1 scope. Future scope expansion requires new evidence.

---

## 1. Runtime Ownership

Battle Core provides deterministic, native-owned battle flow control:

| Component | Responsibility | Evidence |
|-----------|---------------|----------|
| `BattleFlowController` | Phase management (Start → Input → Action → TurnEnd → Victory/Defeat/Abort), turn counting, escape tracking | `engine/core/battle/battle_core.h` |
| `BattleActionQueue` | Speed/priority ordered action scheduling with snapshot support | `engine/core/battle/battle_core.h` |
| `BattleRuleResolver` | Damage formula resolution and escape ratio calculation | `engine/core/battle/battle_core.h` |

All runtime types are header-defined, use C++20 `enum class`, and carry no hidden state.

---

## 2. Editor Surfaces

Three editor panels provide live inspection and preview:

| Surface | Key Capability | Header |
|---------|---------------|--------|
| `BattleInspectorModel` | Loads from runtime controller + queue; produces summary, action rows, and issue list | `editor/battle/battle_inspector_model.h` |
| `BattlePreviewPanel` | Binds to flow controller; previews physical/magical damage and escape ratios | `editor/battle/battle_preview_panel.h` |
| `BattleInspectorPanel` | Orchestrates model + preview; supports subject filter and issues-only mode | `editor/battle/battle_inspector_panel.h` |

---

## 3. Schema Contracts

Native JSON schemas are enforced under `content/schemas/`:

- **`battle_troops.schema.json`** — Contract for enemy groups, member placement, and phase triggers.
- **`battle_actions.schema.json`** — Contract for skill/item execution, scope, cost, and effects.

Both schemas define required fields, enumerated scope values, and effect type constraints. `battle_troops.schema.json` now also accepts recursive grouped phase conditions (`and` / `or`) plus `_compat_condition_fallbacks` records when a source condition tree cannot be represented exactly.

---

## 4. Migration Mapping

`BattleMigration` (`engine/core/battle/battle_migration.h`) provides static migration helpers:

- `migrateTroop()` — Maps RPG Maker MV/MZ troop JSON to native `TRP_*` format with member positioning, phase condition mapping (turn-based, enemy HP threshold, switch-based, **actor-present**, and grouped boolean `and` / `or` trees), and **event command effect mapping** (Show Text → message, Common Event → common_event, Change State → state_change, Force Action → force_action, Change Enemy HP/MP → state_change, **Change Switches → change_switches, Change Variables → change_variables, Change Gold → change_gold, Change Items → change_items, Change Weapons → change_weapons, Change Armors → change_armors, Transfer Player → transfer_player, Game Over → game_over**). Unsupported condition-tree nodes and unsupported event commands now emit explicit structured warnings/effects, and unrepresentable condition branches are preserved in `_compat_condition_fallbacks`.
- `migrateAction()` — Maps MV/MZ skill/item records to native battle actions with scope translation and cost mapping. **Scope mapping now covers all 12 RPG Maker scope codes** (single/random/all enemy/ally/dead/user combinations).

Progress tracking (`total_enemies`, `total_troops`, `total_actions`, `warnings`, `errors`) is surfaced for diagnostics.

---

## 5. Diagnostics Integration

- **Live scene diagnostics preview**: `BattleInspectorPanel` binds directly to a live `BattleFlowController` and `BattleActionQueue`, enabling real-time action order validation and damage preview.
- **Migration warnings**: `BattleMigration` emits honest warnings for unmapped scopes, unsupported troop event commands, and unsupported non-damage effects, which propagate into the `MigrationWizardModel` report.

---

## 6. Test Coverage

| Layer | Count | Sources |
|-------|-------|---------|
| Unit | 8 | `test_battle_core`, `test_battlemgr`, `test_battle_effect_cues`, `test_battle_scene_native`, `test_battle_inspector_model`, `test_battle_inspector_panel`, `test_battle_preview_panel`, `test_battle_migration` |
| Integration | 2 + closure suite | `test_battle_save_integration`, `test_integration_runtime_recovery`, `test_wave1_closure_integration` |

Key cross-subsystem assertions include:
- Battle metadata survives `RuntimeSaveLoader` round-trips.
- Migration runner does not drop `_battle_state` during unrelated field renames.
- Menu and message runtime states survive save/load boundaries.

---

## 7. Closed Scope And Future Work

1. **Bounded in-tree battle contract**: the claimed Wave 1 battle-core contract is native-owned and covered. Future feature work such as broader formula semantics, transition-type visual routing, deeper troop-page scripting, and networked/async battle does not block this scope.
2. **Compat import/migration completion**: `BattleMigration` maps troop structure, member placement, phase conditions, event commands, and action scopes for the claimed scope, with unsupported future/plugin-specific constructs preserved through explicit fallback diagnostics.
3. **Plugin authority validation**: battle plugin behavior remains a future corpus-expansion lane, not a blocker for native battle-core readiness.

---

*Sign-off approved by release-owner review for the bounded Wave 1 battle-core scope on 2026-04-28.*
