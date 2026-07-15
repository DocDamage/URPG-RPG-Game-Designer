# PFU-05 Reviewed Native Grid Part Rectangle Fill

## Scope

Expose the existing native Grid Part bulk-placement command as a labelled Level
Builder control. This is a bounded rectangle fill for one selected catalog part,
not a freeform canvas brush or a new shared history owner.

## Contract

- The creator enters two rectangle corners and explicitly reviews the selected
  catalog part before applying it.
- Review checks document/catalog ownership, selection, rectangle bounds, and
  every derived part footprint. It reports the exact placement count without
  mutation.
- Apply repeats the review and then uses the existing `BulkGridPartCommand`,
  preserving its all-or-nothing behavior and one local undo/redo entry.
- The Accessibility Audit reports labelled review/apply virtual actions for the
  selected Grid Part rectangle route, so the operation is not represented as a
  canvas-only control.

## Limits

This does not add freehand/multi-instance brush strokes, cross-owner history,
runtime/package behavior, controller qualification, or manual accessibility
evidence. Builds and tests are deferred under the user instruction for this
phase.
