# PFU-04 Work Packet: native Menu Studio canvas alignment guides

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add creator-visible magnetic alignment feedback to the existing native Menu
Studio canvas for one pane move or bottom-right resize operation.

## Contract

1. After the existing grid snap, a dragged pane can snap within eight design
   pixels to another valid visible pane's leading edge, center, or trailing
   edge on either axis. A resize can similarly snap its changed trailing edge.
2. A green full-canvas guide appears only while an eligible alignment is being
   previewed. It is not serialized and has no runtime meaning.
3. The preview remains local until pointer release. The existing layout-change
   callback still performs the sole validated Menu model mutation, local
   history entry, runtime application, dirty update, and project persistence.
4. Invalid, hidden, and the currently dragged panes are excluded as guide
   sources; a guide cannot move or resize outside the existing canvas bounds.

## Limits

This provides single-pane alignment feedback, not multi-select distribution,
persisted responsive anchors, components, or target-device qualification.

Verification is deferred by user instruction.
