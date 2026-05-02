# Character Identity - Release Closure Sign-off

> **Status:** `READY`
> **Purpose:** Approved closure artifact for the bounded Phase 4 Character Identity and appearance-part authoring scope.
> **Date:** 2026-05-01
> **Rule:** This document records release-owner approval for the claimed character identity engine/editor scope. Final-quality portrait art, cohesive character art direction, and unreviewed raw/source asset packs remain content backlog.

---

## Runtime And Persistence

Character Identity has native ownership for identity data, creation rules, ECS attachment, deterministic spawning, runtime-created protagonist save/load persistence, and layered portrait/field/battle composition.

| Area | Evidence |
| --- | --- |
| Identity runtime and serialization | `engine/core/character/character_identity.*`, `content/schemas/character_identity.schema.json` |
| Creation rules and runtime creator screen | `engine/core/character/character_creation_rules.*`, `engine/core/character/character_creation_screen.*` |
| ECS and spawner integration | `engine/core/character/character_identity_system.*` |
| Save/load persistence | `engine/core/character/character_save_state.*`, `content/schemas/created_protagonist_save.schema.json` |
| Layered appearance composition | `engine/core/character/character_appearance_composition.*`, `content/schemas/character_appearance_composition.schema.json` |

## Editor And Import Authoring

The editor exposes deterministic character authoring snapshots for validation, preview, save persistence diagnostics, promoted appearance selection, and part-library management.

| Area | Evidence |
| --- | --- |
| Character creator model and panel | `editor/character/character_creator_model.*`, `editor/character/character_creator_panel.*` |
| Appearance-part import rows | `engine/core/assets/asset_import_session.*` |
| Governed asset action rows | `engine/core/assets/asset_action_view.*` |
| RPG Maker field/battle category mapping | `tools/assets/global_asset_import.py` |

Creator-supplied portrait, field, and battle appearance parts now surface source metadata, normalized asset ids, dimensions, attribution state, blocked reasons, and management actions before assignment. Promotion targets remain under the governed promoted asset library; raw intake paths stay quarantine-only.

## Verification

| Command | Result |
| --- | --- |
| Focused character/asset CTest lane | PASS |
| `python -m unittest tools.assets.tests.test_global_asset_import` | PASS |

Focused character/asset CTest command:

```powershell
ctest --preset dev-all -R "Character|character|AssetLibrary" --output-on-failure
```

## Release Decision

The bounded Phase 4 `character_identity` scope is approved by release-owner review.

This approval does not claim shipped final-quality character art. It approves the native identity runtime, editor authoring, governed appearance-part import path, diagnostics, persistence, preview, and focused validation evidence listed above.
