# PFU-05 Work Packet: direct placement of attached Map event metadata

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Extend the native Map canvas asset-drop route so an attached image dropped in
Events mode creates one native Perspective 2D event with a durable asset ID and
project-path reference.

## Contract

1. Only an existing governed attached image payload is accepted in Events mode.
   Map Events mode selects the matching native Perspective 2D Events toolbar
   state; ordinary canvas clicks remain non-mutating.
   Editor-originated attached payloads carry the immutable attachment manifest
   source SHA-256 and are rejected if that revision changes before placement.
2. The drop selects the current visible, unlocked event/object layer when
   possible, otherwise the first visible, unlocked event/object layer. It fails
   without mutation when none exists or the drop cannot be projected to a map
   tile.
3. The native owner creates a unique `asset_event_<asset-id>` event ID, records
   the asset ID/project path with the event, and captures placement plus layer
   selection as one Perspective 2D undo/redo action.
4. The event asset reference round-trips in the Perspective 2D authoring
   document and render snapshot. The bound native `MapScene` now receives the
   sprite projection through the separate PFU-05 runtime-sprite slice; event
   execution semantics and package support remain separate.

## Acceptance and verification

- Dropping an attached image in Map Events mode creates an event on an editable
  event/object layer and shows its stable asset reference in the native event
  inspector.
- Undo removes the event and redo restores its ID, tile, asset ID, and project
  path together.
- Older documents without event asset metadata continue to load.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[spatial][map_authoring][assets][history]" --reporter compact`

## Rollback and limits

Removing the Events route restores the former Tiles/Props-only canvas-drop
behavior. Asset-revision/staleness checks, collision policy, authored event
pages/commands, cross-owner history, and runtime/export asset support remain
separate PFU-05 work.
