# PFU-06 Work Packet: spatial Map accessibility audit ingestion

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Include the existing native Perspective 2D spatial-control snapshots in the
creator's Accessibility Audit. This covers the current Elevation Brush, Prop
Placement, and Grid Part Placement controls, including the selected/last-added
prop state and selected smart-prefab review/apply alternatives represented by
the native spatial accessibility adapter, plus the Map coordinator's current
mode and project-context state. The same audit run also ingests the existing
native Menu Inspector's visible focus rows without changing the menu owner or
its runtime behavior.

## Contract

1. The Map workspace continues to own its canvas and tool snapshots.
2. The existing `AccessibilitySpatialAdapter` converts those snapshots into
   structured auditor elements. It also exposes the current available Map
  modes, active Map context, and selected Grid Part smart-prefab review/apply
   actions as labelled virtual elements. Current reviewed native creator-plan
   review/apply state is also exposed as labelled virtual actions; the main
   creator route now ingests them with the existing Audio Mix and battle-preview
   elements.
3. The audit remains read-only. It does not alter Map state, focus, or the
   accessibility findings themselves.
4. The unified Map mode selector has a focused keyboard route: `Alt+1` through
   `Alt+0` activate its ordered Canvas-through-Package modes through the same
   native Map owner. Shortcuts are suppressed while a text field is active.
5. The same audit run includes the existing Menu Inspector's visible command
   rows as focus evidence. It reports only the currently modeled native menu
   state and does not claim editor-wide keyboard or controller qualification.
6. Aggregation assigns each owner snapshot a non-overlapping focus-order range
   while preserving its local ordering and local duplicate-order findings. This
   avoids treating independent surfaces that both start at order one as a
   cross-surface focus conflict.

## Limits

This is evidence ingestion and labelled virtual mode/context alternatives, not
keyboard-only Map operation or controller qualification. Canvas alternatives
for tile/event placement, broader focus traversal, assistive-technology review,
device hot-plug, and manual graphical evidence remain PFU-06 work.

Verification is deferred by user instruction.
