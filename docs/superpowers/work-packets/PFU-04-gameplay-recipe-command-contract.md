# PFU-04 Work Packet: native gameplay recipe command contract

**Status:** Implemented bounded F06 foundation

**Date:** 2026-07-15

## Scope

Introduce a versioned native gameplay-recipe artifact for the existing typed
`GameplayWysiwygDocument` runtime primitive. A recipe is previewed, validated,
applied through a typed project document, and reverted through its recorded
receipt. This first slice deliberately owns only authored gameplay WYSIWYG
features; it does not execute MZ plugins, evaluate scripts, mutate arbitrary
project JSON, or replace Menu, dialogue, and quest domain owners.

## Traceability and owners

- **Source outcomes:** F06 Gameplay Recipe Gallery; F04 boundary separation;
  I07 typed configuration.
- **Authoritative runtime primitive:**
  `engine/core/gameplay/gameplay_wysiwyg_system.*`.
- **New domain owner:**
  `engine/core/gameplay/gameplay_recipe_document.*`.
- **Editor projection:** `editor/gameplay/gameplay_recipe_panel.*`.
- **Schema impact:** `urpg.gameplay_recipe.v1` and
  `urpg.gameplay_recipe_project.v1`; both are in-memory typed documents in
  this slice, without migration of the general project manifest.

## Contract

1. A recipe has a non-empty stable ID and version and contains one validated
   `GameplayWysiwygDocument` target.
2. Preview validates the recipe and exposes the target runtime preview without
   mutating the project document.
3. Apply inserts the target only when its ID is unowned. Repeating an identical
   applied recipe is idempotent; a different recipe or version at that target
   fails as a conflict.
4. Apply records the recipe ID, version, target ID, and canonical target
   document. Revert succeeds only when the receipt still owns an unchanged
   target. It then removes that target and receipt; edits made after apply fail
   closed and are never erased.
5. The editor panel is a projection over this owner. It cannot execute plugin
   code or turn an MZ plugin recipe into native support.

## Acceptance and verification

- A built-in typed template previews and applies to the native runtime
  primitive.
- Reapplying does not duplicate content.
- A target collision and an externally modified target are diagnosed and left
  intact.
- Revert removes only the recipe-owned target and leaves unrelated targets.

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[gameplay][recipe]" --reporter compact
git diff --check
```

## Rollback and limits

The original command implementation is removable as an isolated
document/service/panel/test surface. Its later owned persistence extension is
tracked separately in
`docs/superpowers/work-packets/PFU-04-gameplay-recipe-project-persistence.md`.
Menu canvas, dialogue and quest graph editing, plugin lock/trust UX, and
migrations remain separate PFU-04 packets.

## Implementation evidence

2026-07-15: `GameplayRecipeService` now owns versioned recipe validation,
preview, idempotent application, conflict rejection, and receipt-guarded
revert for `GameplayWysiwygDocument` targets. `GameplayRecipePanel` is
reachable as the in-session Gameplay Recipe Gallery inside the existing
Ability workspace, where the built-in native template exposes preview, apply,
revert, receipt state, and diagnostics. It never executes plugin code; its
later project-document persistence and recovery integration are tracked in the
dedicated persistence packet. The focused `[gameplay][recipe]`
lane previously passed 30 assertions across template/runtime preview,
duplicate-apply, collision, modified-target revert, and panel-state cases.
