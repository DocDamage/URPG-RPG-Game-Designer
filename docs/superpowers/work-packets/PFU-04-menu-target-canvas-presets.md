# PFU-04 Work Packet: native Menu Studio target-canvas presets

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Provide bounded 1280x720, 1920x1080, and 800x600 design-canvas presets in the
native Menu Inspector. Each preset uses the same model mutation, local history,
runtime application, dirty state, project document, and recovery path as a
numeric canvas edit.

## Contract

1. A preset changes only the persisted native design canvas; it neither scales
   nor silently reflows pane rectangles.
2. Existing inspector diagnostics remain the truthful outcome for panes that
   extend outside a newly selected target canvas.
3. One changed preset is one local undo/redo entry. Selecting the current
   target is a no-op.

## Acceptance and verification

- Preset selection updates the native preview canvas and durable project graph.
- Undo/redo restores one preset selection.
- Overflow is diagnosed rather than hidden or automatically corrected.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][editor][menu]" --reporter compact`

## Rollback and limits

Removing the three buttons leaves numeric canvas entry unchanged. Responsive
constraints, automatic reflow, device certification, and manual target-size
evidence remain separate F05 work.
