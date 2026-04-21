# Battle Core — Wave 1 Closure Sign-off

> **Status:** `PARTIAL`  
> **Purpose:** Evidence-gathering artifact for Wave 1 closure review.  
> **Date:** 2026-04-20  
> **Rule:** This document does **not** promote `readiness_status.json` status. Human review is required for promotion to `READY`.

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

Both schemas define required fields, enumerated scope values, and effect type constraints.

---

## 4. Migration Mapping

`BattleMigration` (`engine/core/battle/battle_migration.h`) provides static migration helpers:

- `migrateTroop()` — Maps RPG Maker MV/MZ troop JSON to native `TRP_*` format with member positioning, phase condition mapping (turn-based, enemy HP threshold, switch-based, **actor-present**), and **event command effect mapping** (Show Text → message, Common Event → common_event, Change State → state_change, Force Action → force_action, Change Enemy HP/MP → state_change, **Change Gold → change_gold, Change Items → change_items, Change Weapons → change_weapons, Change Armors → change_armors, Transfer Player → transfer_player, Game Over → game_over**). Event command lists with unmapped codes emit partial-mapping warnings.
- `migrateAction()` — Maps MV/MZ skill/item records to native battle actions with scope translation and cost mapping. **Scope mapping now covers all 12 RPG Maker scope codes** (single/random/all enemy/ally/dead/user combinations).

Progress tracking (`total_enemies`, `total_troops`, `total_actions`, `warnings`, `errors`) is surfaced for diagnostics.

---

## 5. Diagnostics Integration

- **Live scene diagnostics preview**: `BattleInspectorPanel` binds directly to a live `BattleFlowController` and `BattleActionQueue`, enabling real-time action order validation and damage preview.
- **Migration warnings**: `BattleMigration` emits honest warnings for unmapped scopes and unsupported non-damage effects, which propagate into the `MigrationWizardModel` report.

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

## 7. Remaining Residual Gaps (Honest Scope Limits)

1. **Compat import/migration completion**: `BattleMigration` maps troop structure, member placement, phase conditions (turn, enemy HP, switch, **actor present**), event commands (Show Text, Common Event, Change State, Force Action, Change Enemy HP/MP, **Change Gold, Change Items/Weapons/Armors, Transfer Player, Game Over**), and action scopes. Complex multi-condition trees with OR semantics and remaining niche event commands are not yet migrated. This is tracked as "Compat import/migration completion remains open" in `readiness_status.json`.
2. **Plugin authority validation**: Battle-specific RPG Maker MZ plugins are validated at the manifest level, but deep behavioral sandbox tests for every battle plugin variant are not yet at 100 % coverage.
3. **Networked / async battle**: Not in Wave 1 scope.

---

*Sign-off prepared by governance agent. Promotion to `READY` requires human review of the residual gaps above.*
