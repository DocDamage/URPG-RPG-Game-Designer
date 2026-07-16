# PFU-04 Work Packet: gameplay recipe project persistence

**Status:** Implemented in worktree; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Persist the bounded native `GameplayRecipeProjectDocument` at one owned
project path: `content/gameplay/recipes.json`. This packet joins the existing
editor project-session, dirty-state, recovery, and atomic-write flow. It does
not persist templates, execute plugins, introduce arbitrary project JSON
edits, or claim that generic gameplay WYSIWYG features are fully integrated
with their specialist domain owners.

## Traceability and owners

- **Source outcomes:** F06 Gameplay Recipe Gallery; I07 typed configuration;
  F01 project lifecycle/recovery.
- **Authoritative typed owner:**
  `engine/core/gameplay/gameplay_recipe_document.*`.
- **Persistence integration:** `apps/editor/main.cpp` and the existing
  `EditorDirtyStateRegistry` / `EditorRecoveryService`.
- **Schema impact:** the existing `urpg.gameplay_recipe_project.v1` JSON is
  persisted at the owned project path, with no change to `project.json`.

## Contract

1. The document validates its schema, feature IDs, feature documents, receipt
   IDs, receipt targets, and receipt/feature consistency before publication.
2. Loading a missing owned file yields a clean empty document; malformed or
   invalid present files are diagnosed and never partially applied.
3. Apply/revert marks one existing dirty surface. Save publishes exactly one
   typed document atomically; a failed save leaves that surface dirty.
4. Dirty recipe work is included in the existing private recovery snapshot,
   and project switches use the existing save/discard/cancel navigation guard.
5. Project switching loads the new project document; close clears it. No
   template, plugin, or cross-project document state is reused implicitly.

## Acceptance and verification

- A recipe apply saves and reloads its feature plus receipt.
- Revert saves the removed receipt/target state without deleting unrelated
  gameplay document features.
- Invalid/malformed persisted data is not loaded into the active document.
- A save failure leaves the dirty surface and recovery draft intact.

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[gameplay][recipe]" --reporter compact
git diff --check
```

## Rollback and limits

Deleting the owned `content/gameplay/recipes.json` restores the previous
empty-project behavior. General templates, menus, dialogue, quest migration,
plugin settings, and specialist runtime integration remain separate work.

## Implementation evidence

2026-07-15: `GameplayRecipeProjectDocument::validate()` now checks the typed
project schema, feature-map IDs, feature validity, receipt IDs, receipt target
existence, and receipt-target equivalence. The editor loads the owned document
only after that validation, otherwise retains a clean document and exposes the
load diagnostic in the Ability workspace. Apply/revert tracks
`gameplay.recipes` through `EditorDirtyStateRegistry`; its save callback
publishes `content/gameplay/recipes.json` with the existing atomic text writer,
and private recovery snapshots capture the same owned relative path. Project
switches load the incoming document and clear stale dirty state; close clears
the temporary document. No build, test, or formatting command was run after
this increment because the user explicitly directed implementation to continue
without further test activity.
