# PFU-04 Work Packet: native Menu Studio pane-layout templates

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add four reusable, native pane-layout templates to the selected Menu Studio
pane: Compact List, Centered Dialog, Bottom Overlay, and Full Canvas.

## Contract

1. Templates derive only a pane rectangle from the persisted native design
   canvas and apply through `MenuInspectorModel::UpdatePaneLayout`; they do not
   create a sidecar, alter the menu schema, or establish a second document
   owner.
2. A template preserves the selected pane's stable identity, commands,
   visibility/activation state, z layer, and focus order. It leaves every
   unselected pane unchanged.
3. Each non-no-op template application is one existing local-history mutation,
   invokes the existing runtime apply callback, and then follows the existing
   project dirty, save, close-guard, recovery, and atomic `menus.json` path.
4. Small canvases produce positive, valid pane dimensions. Existing layout
   diagnostics remain responsible for any author-selected overlap or overflow
   outside this bounded template set.

## Acceptance and verification

- The four templates appear for the selected pane in the native inspector.
- Undo/redo restores a template application as one edit, and applying an
  already matching template creates no history or dirty mutation.
- Saved menu graphs retain the resulting ordinary pane layout and round-trip
  through the existing native serializer.

Verification command (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][editor][menu]" --reporter compact`

## Rollback and limits

Removing the template controls and model entry point leaves numeric editing,
alignment, presets, canvas dragging, resizing, and project persistence intact.
This is reusable layout preset coverage only; component definitions,
parameterized template import/export, multi-select/distribution, responsive
constraints, and manual target-size qualification remain separate F05 work.
