# PFU-05 Work Packet: direct placement of attached Map props

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Extend the native Map canvas asset-drop route and the existing selected-project
prop tool so an attached image creates a placed Perspective 2D prop immediately,
rather than only enrolling the asset in a palette.

## Contract

1. The operation accepts only the existing governed attached-asset payload and
   projects the drop through the existing bounded map-surface projection.
2. A successful drop writes one stable asset ID prop instance, its persisted
   project-path palette binding, and its transform as one Perspective 2D
   history action. Undo/redo restores or removes both together.
   Attached Tile drops use an explicit conservative conflict policy: they add
   an empty cell or idempotently reuse the same attached tile, but reject a
   different existing tile before changing a palette or map document.
3. Prop instances round-trip through the native Perspective 2D document. Older
   documents without a `props` field retain their legacy overlay behavior.
4. Invalid or out-of-canvas projection, unavailable owner, unsupported Map
   mode, raw asset, missing project path, and stale or missing attached
   revisions fail without mutation. Legacy programmatic payloads without an
   open project context remain accepted for compatibility.

## Acceptance and verification

- Tiles and governed Props both place immediately from the Map canvas or the
  selected-project Prop tool.
- A prop drop is dirty, save/recovery eligible, and stored in the project-owned
  Perspective 2D document rather than a palette-only/editor-only sidecar.
- The serialized prop includes instance ID, stable asset ID, position, rotation,
  and scale; malformed prop arrays/placements are rejected before the existing
  loader mutates its child document state.

Packet-local verification commands (current pass not implied):

`./build/dev-ninja-debug/urpg_tests.exe "[spatial][map_authoring][assets][history]" --reporter compact`

## Rollback and limits

Removing the direct-prop route restores palette-only prop drops. Grid Parts
prefabs, multi-instance brush placement, stale attachment resolution, shared
cross-owner history, and target/runtime package evidence remain separate PFU-05
work.
