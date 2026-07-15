# PFU-04 Work Packet: project-owned native Menu Studio document

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Bind Menu Studio to a real project-owned native `MenuScene` graph. Store it at
`content/ui/menus.json`, load it through the staged graph serializer, expose it
through the Diagnostics Menu surface, and register it with the existing dirty,
save, close, and private-recovery lifecycle.

## Traceability and owners

- **Source outcomes:** F05 WYSIWYG Menu Studio; F01 project lifecycle; I04
  safe menu persistence; I08 editor usability.
- **Runtime owner:** `engine/core/scene/menu_scene.*` and its native graph/
  command registry.
- **Project lifecycle owner:** `apps/editor/main.cpp` plus existing dirty and
  recovery services.
- **Document schema:** `content/ui/menus.json` using `urpg.menu_graph.v1`.

## Contract

1. Opening a project loads its menu graph before binding the native Menu
   Inspector/Preview. A missing file receives a deterministic native starter
   graph; malformed files do not become partial live state.
2. The Menu Studio dirty surface compares canonical graph serialization with
   its last persisted value. Save uses the existing editor atomic writer.
3. Unsaved menu changes participate in Save All, close guards, and private
   recovery snapshots, without treating preview focus movement as authoring.
4. Closing a project clears the bound menu runtime and its transient editor
   state. Native baseline commands are registered separately from MZ plugins.

## Acceptance and verification

- Project open/load/save/close preserves a graph and active scene.
- Layout or command edits become dirty, save atomically, and enter recovery.
- Missing and malformed project menu documents have explicit creator status.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][menu]" --reporter compact`

`ctest --preset dev-pr --output-on-failure`

## Rollback and limits

Removing the project binding returns Menu Studio to diagnostics-only runtime
inspection. Templates/components, drag constraints/guides, complete command
authoring, migration UI, local history/undo, target-size qualification, and
manual accessibility evidence remain separate F05 work.

## Implementation evidence

2026-07-15: `EditorPanelRuntime` now owns a native `MenuScene`, and project
switching loads `content/ui/menus.json` through the staged graph serializer
before binding the Diagnostics Menu inspector/preview. Missing files receive a
MainMenu starter graph containing registered native Item, Status, Options, and
Save commands. Invalid files are replaced only by that explicit starter state
and reported in the Menu tab. The `menu.studio` dirty surface compares graph
serialization against the last persisted value, saves with the editor's atomic
writer, participates in Save All/close guards, and writes an exact
`content/ui/menus.json` private recovery draft. Closing clears the menu runtime
and diagnostic binding. No build, test, or formatting command was run because
the user explicitly directed implementation to continue without further test
activity.
