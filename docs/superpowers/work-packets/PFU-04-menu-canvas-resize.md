# PFU-04 Work Packet: native Menu Studio canvas resize handles

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add a bounded bottom-right resize handle to each visible native Menu Studio
pane. Resizing previews a snapped valid rectangle and commits one model edit on
release through the same history/runtime/project-document path as dragging.

## Contract

1. The handle is distinct from the pane move target and affects width/height
   only; it does not reinterpret focus/layer or create an editor-only size.
2. Preview dimensions snap to the visible canvas grid, remain at least 32 px,
   and remain inside the native design canvas.
3. One completed resize is one local history entry and one project dirty
   mutation; cancelled/no-op resize commits nothing.

## Acceptance and verification

- Resizing changes the serialized pane rectangle only after release.
- Undo/redo restores/reapplies one completed resize.
- The preview does not permit out-of-canvas or non-positive dimensions.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][editor][menu]" --reporter compact`

## Rollback and limits

Removing the handle restores move-only direct manipulation. Multi-edge resize,
multi-select, alignment/distribution, responsive constraints, templates, and
target-size qualification remain separate F05 work.
