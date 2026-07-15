# PFU-04 Work Packet: native Menu Studio local history

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add bounded in-memory undo/redo for native Menu Studio document edits. The
history is owned by `MenuInspectorModel`, captures the menu canvas and all
pane/command data before each accepted mutation, and applies through the same
runtime graph/project dirty owner as ordinary edits.

## Contract

1. Command label/route/add/remove and pane/canvas edits each record one prior
   native document state and clear redo after a new mutation.
2. Undo/redo restores only the active menu document model, preserves selected
   command identity where possible, and has no independent file-write path.
3. History resets on runtime bind/clear. Save does not discard it; undoing a
   saved change simply makes the project-owned document dirty again.
4. History is intentionally local and bounded. Cross-session history,
   collaboration/merge, and generic JSON overwrite are outside this slice.

## Acceptance and verification

- Each supported edit can be undone and redone without changing another pane.
- Applying an undo/redo changes the preview/runtime graph and project dirty
  state through existing owners.
- Invalid layout input records no history entry.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][editor][menu]" --reporter compact`

`git diff --check`

## Rollback and limits

Removing model history and inspector controls restores the previous immediate
editing behavior. Persistent history, operation receipts, selective replay,
and branch/merge remain PFU-05/F20 work.

## Implementation evidence

2026-07-15: `MenuInspectorModel` now stores up to 64 native document states
(canvas plus panes/commands) before each accepted label, route, add/remove,
pane-layout, or canvas mutation. New changes clear redo. Undo/redo restores a
state, rebuilds diagnostics, preserves selected command identity where it
still exists, and leaves runtime application to the existing inspector apply
handler. The Menu tab exposes Undo Menu Edit and Redo Menu Edit controls; the
project-owned graph serialization then drives normal dirty/save/recovery
behavior. Deferred unit coverage records label/layout undo/redo and runtime
application. No build, test, or formatting command was run because the user
explicitly directed implementation to continue without further test activity.
