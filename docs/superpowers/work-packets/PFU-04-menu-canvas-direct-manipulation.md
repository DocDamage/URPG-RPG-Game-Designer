# PFU-04 Work Packet: native Menu Studio direct canvas manipulation

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add native direct manipulation to the Menu Preview canvas. Visible panes may be
dragged on the design canvas; their final position snaps to a visible grid and
commits one layout edit through `MenuInspectorModel`, existing local history,
runtime apply, and the project-owned menu document.

## Contract

1. Drag state is preview-local until mouse release. One completed drag records
   exactly one native layout history entry rather than one entry per frame.
2. The rendered canvas shows design-grid guides and uses the same serialized
   pane rectangles/layers as runtime preview.
3. A drag may update only a visible, valid pane; values remain constrained by
   `MenuPaneLayout` validation and model application.
4. The preview owns no persistence or alternate document. Its callback applies
   accepted changes to the existing native Menu Studio model/runtime owner.

## Acceptance and verification

- Dragging a pane visibly changes its snapped rectangle after release.
- Undo/redo returns/reapplies one completed drag as one menu edit.
- The project dirty/recovery/save path observes the committed graph change.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][editor][menu]" --reporter compact`

`git diff --check`

## Rollback and limits

Removing the preview drag callback and state restores numeric-only layout
editing. Resize handles, multi-select, alignment/distribution guides,
responsive anchors/constraints, reusable components/templates, and
target-size/manual qualification remain separate F05 work.

## Implementation evidence

2026-07-15: `MenuPreviewPanel` now renders an adjustable 4-64 px canvas grid
and uses invisible pane hit regions to initiate a local drag state. Pointer
movement previews a clamped, snapped pane rectangle and guide lines without
touching the model. Mouse release incorporates the final pointer position,
then calls the Diagnostics-owned layout callback once; that callback performs
the existing validated model mutation and runtime apply, which records one
undoable project edit. Snapshot panes now include their stable pane index. No
build, test, or formatting command was run because the user explicitly
directed implementation to continue without further test activity.
