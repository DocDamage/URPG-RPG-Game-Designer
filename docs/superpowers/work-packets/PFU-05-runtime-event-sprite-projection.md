# PFU-05 Work Packet: Native Map Event Sprite Projection

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Project the existing Perspective 2D event image reference and opt-in movement
blocker into the bound native `MapScene` without adding a second event store,
changing event command execution, or claiming package/runtime qualification.

## Contract

- The Perspective 2D document remains the only persistent owner of an event's
  stable ID, asset ID, project path, and tile position.
- Every authoring snapshot synchronizes visible-layer image events into a
  complete validated `MapScene` sprite set. A missing image reference, missing
  layer, hidden layer, or out-of-bounds event is not projected.
- `MapScene` accepts a replacement batch only when every event ID, asset ID,
  asset path, and tile coordinate is valid, and a shared stable asset ID never
  names conflicting paths. Invalid batches leave its prior runtime projection
  intact; accepted sprites are sorted by event ID for deterministic command
  order.
- `MapScene` registers the governed asset path as renderer metadata and emits
  one 48-pixel sprite command at the authored tile with a z-order above the
  player. It does not load or mutate an asset catalog.
- Perspective 2D undo, redo, draft load, layer visibility, and active-map
  rebinding reuse the existing document/history route, so the runtime sprite
  projection follows the same authoritative state.
- An event's persisted `blocks_movement` default and its page-level
  `blocks_movement` overrides are edited through Event Authoring and project
  only for visible, in-bounds event layers. The `MapScene` replaces the
  validated collider batch deterministically by event ID and evaluates the
  final matching page with the existing switch/integer-variable/event-local
  self-switch condition authority; direct movement and path planning use the
  same collision predicate.

## Limits

This does not add animation/sheet slicing, page-conditional visibility,
interaction binding, event execution changes, texture packing, package
evidence, or release qualification. Builds and test execution remain deferred
under the user instruction for this phase.
