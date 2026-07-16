# PFU-07 Native Reviewed Prop Command

## Scope

Extend the developer-only creator-intent surface by one authoritative native
domain: placing already-attached Perspective 2D prop assets. This increment
does not grant a planner generic project-document mutation authority.

## Contract

- The deterministic `place prop` request contains one planned asset ID and one
  tile target. The UI resolves that planned ID explicitly to an existing,
  Perspective-2D-targeted attached prop-palette entry before review.
- The review captures the active Perspective 2D document revision. Apply
  rejects a changed document, an absent/changed palette binding, an empty or
  out-of-bounds target, a duplicate planned operation, or a stable instance-ID
  collision before mutating the Map.
- The Map owner derives the stable instance ID and places every validated prop
  at the target tile center. It records the group as one existing local
  undo/redo entry and marks the Map dirty; no detached JSON is written.
- Tile-only review remains separate. Mixed tile-and-prop plans and all event
  logic plans remain explicitly unavailable.

## Limits

This is developer-only, deterministic, local planning with dry-run provider
transport. It does not expose arbitrary props, enroll raw assets, create event
logic, add image/sketch authority, support multi-turn tool use, or establish
runtime/package/assistant qualification. Automated verification was audited on 2026-07-16; see the active PFU evidence. Manual/package qualification remains outside this bounded packet.
