# PFU-04 Work Packet: native Menu Studio multi-select distribution

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Provide bounded multi-pane horizontal and vertical distribution in the native
Menu Studio preview.

## Contract

1. A creator Ctrl+clicks valid visible panes to select or deselect them. A
   selected pane has a distinct preview border; selection is panel-local and
   never serialized.
2. Horizontal or vertical distribution retains the outermost selected pane
   bounds and evenly distributes the selected panes' gaps on the chosen axis.
   The operation rejects overlapping outer bounds that cannot yield a
   non-negative gap.
3. The preview submits all changed pane rectangles through one batch callback.
   `MenuInspectorModel` validates every unique target index before recording
   one local history state, updates all layouts, rebuilds the model, and then
   applies the existing native runtime/project persistence route.
4. Invalid, hidden, or stale selected panes are removed from panel-local
   selection before an operation. No layout data outside the selected panes is
   changed.

## Limits

This is not responsive anchoring, arbitrary multi-pane move/resize, reusable
components, or target-device qualification. It provides only deterministic
same-axis gap distribution for selected visible panes.

Verification is deferred by user instruction.
