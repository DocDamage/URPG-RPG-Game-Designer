# PFU-04 Work Packet: native gameplay recipe gallery templates

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Turn the bounded F06 surface into an actual gallery of selectable, native
templates. Add one additional typed `GameplayWysiwygDocument` template beside
the starter quest-choice recipe and expose selection from the existing Ability
workspace. Both templates remain governed by the same parameter, preview,
receipt, persistence, and recovery owners.

## Traceability and owners

- **Source outcomes:** F06 Gameplay Recipe Gallery; I07 typed configuration.
- **Authoritative owner:** `engine/core/gameplay/gameplay_recipe_document.*`.
- **Editor projection:** `editor/gameplay/gameplay_recipe_panel.*` and
  `apps/editor/main.cpp`.
- **Schema impact:** no project-document schema change; the built-in catalog is
  compiled native data, while an applied concrete feature remains persisted at
  `content/gameplay/recipes.json`.

## Contract

1. Each gallery item is a stable native recipe ID/version with a distinct
   target ID and validated typed target document.
2. Selection replaces only the panel's template/configuration state; it never
   mutates the project document.
3. Switching templates preserves existing project features and receipts. A
   template already applied at its own ID remains receipt-guarded.
4. The gallery contains no MZ-plugin discovery, dynamic code loading, or
   arbitrary JSON recipe import.

## Acceptance and verification

- The gallery lists both built-in templates and reports their display names.
- Selecting either template previews before project mutation.
- Applying/reverting one template leaves another applied template intact.
- Template selection does not create a dirty project document by itself.

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[gameplay][recipe]" --reporter compact
git diff --check
```

## Rollback and limits

Removing the additional compiled template and selector restores the
single-template surface without changing stored project documents. Template
authoring/import, specialist domain integration, dynamic catalogs, and MZ
compatibility remain separate work.

## Implementation evidence

2026-07-15: `builtInGameplayRecipeTemplates()` now contains the parameterized
Starter Quest Choice, Town Event Signal, and Camp Rest Recovery templates. They
use distinct stable recipe/target IDs and native `quest_choice_consequence`,
`world_state_timeline`, and `companion_banter` WYSIWYG feature types,
respectively. Camp Rest Recovery binds only the named camp-status variable and
party-health resource field through the same typed parameter contract. The Ability
workspace derives its selector from that compiled catalog and asks
`GameplayRecipePanel` to select a template without changing the project
document. No build, test, or formatting command was run after this increment
because the user explicitly directed implementation to continue without
further test activity.
