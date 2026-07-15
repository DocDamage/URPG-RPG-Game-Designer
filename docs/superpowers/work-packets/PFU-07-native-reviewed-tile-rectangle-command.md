# PFU-07 Reviewed Native Tile Rectangle Command

## Scope

Extend the developer-only deterministic `paint_tile` / `stamp_tile` route from
one cell to a bounded axis-aligned rectangle. The active Perspective 2D Map
remains the only mutation owner.

## Contract

- The creator explicitly supplies the anchor, tile ID, and a width/height from
  1 through 64 cells in the existing reviewed tile-plan controls.
- The planner rejects invalid dimensions and rectangles outside the current
  Map before producing edits. It expands only to same-tile terrain edits.
- Existing native palette/layer binding, source-revision recheck, Map bounds
  checks, and one local owner history action remain mandatory for apply.
- Provider transport stays dry-run only. The rectangle is deterministic input,
  never provider-authored geometry.

## Limits

This does not add arbitrary shapes, freehand painting, mixed-domain plans,
provider execution, controller qualification, or runtime/package evidence.
Builds and tests are deferred under the user instruction for this phase.
