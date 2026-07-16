# PFU-05 Work Packet: active Map attached-asset replacement

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Provide one owner-scoped replacement operation for attached visual assets in
the currently active native Perspective 2D Map.

## Contract

1. The operation is initiated from an attached image in Assets by dropping a
   second attached image onto its explicit active-Map replacement target.
2. The replacement payload must be an attached image whose project path is
   contained by its stable imported-asset directory. It then passes the
   existing durable-drop admission and attachment-manifest revision check
   immediately before mutation.
3. The Perspective 2D Map owner replaces its supported attached tile-palette,
   painted-tile, prop-palette, prop-instance, and authored event-metadata
   references in one local undo/redo operation. It updates the active document
   dirty state but does not save it implicitly.
4. The operation rejects missing IDs, same-asset replacement, a missing Map
   target, and a source with no supported active-Map reference. The source
   asset remains attached; no attachment manifest or media file is deleted.

## Limits

This is neither a project-wide replacement nor a safe delete/rename API. It
does not rewrite other Maps, Dialogue, Character Creator, Grid Part, Audio,
or any unindexed owner; it does not render event sprites or establish
project-wide transaction/history semantics. The existing removal-impact query
remains read-only for all other owners.

Automated verification was audited on 2026-07-16; see the active PFU evidence. Manual/package qualification remains outside this bounded packet.
