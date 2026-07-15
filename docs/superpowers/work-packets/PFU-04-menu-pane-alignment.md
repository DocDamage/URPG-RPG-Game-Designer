# PFU-04 Work Packet: native Menu Studio pane alignment

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add horizontal and vertical alignment controls for the inspector's selected
pane: left/center/right and top/center/bottom relative to the persisted native
design canvas.

## Contract

1. Alignment changes only the selected pane's x or y coordinate through the
   existing `UpdatePaneLayout` model mutation and runtime apply callback.
2. Width, height, layer, focus, command data, and every unselected pane remain
   unchanged.
3. If a pane exceeds the corresponding canvas dimension, the command anchors
   it at origin and existing diagnostics remain responsible for reporting the
   overflow; alignment never hides or resizes it.

## Acceptance and verification

- Each alignment command creates at most one local history entry and persists
  through the Menu Studio project document.
- Undo/redo restores the aligned coordinate as a single edit.
- No-op commands do not create history or dirty state.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][editor][menu]" --reporter compact`

## Rollback and limits

Removing the inspector controls leaves numeric and canvas-drag editing intact.
Multi-select/distribution, constraint systems, responsive reflow, and manual
target-size qualification remain separate F05 work.
