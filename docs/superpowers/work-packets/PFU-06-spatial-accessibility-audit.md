# PFU-06 Work Packet: spatial Map accessibility audit ingestion

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Include the existing native Perspective 2D spatial-control snapshots in the
creator's Accessibility Audit. This covers the current Elevation Brush and
Prop Placement controls, including the selected/last-added prop state already
represented by the native spatial accessibility adapter.

## Contract

1. The Map workspace continues to own its canvas and tool snapshots.
2. The existing `AccessibilitySpatialAdapter` converts those snapshots into
   structured auditor elements; the main creator route now ingests them with
   the existing Audio Mix and battle-preview elements.
3. The audit remains read-only. It does not alter Map state, focus, input
   routing, or the accessibility findings themselves.

## Limits

This is evidence ingestion, not keyboard-only Map operation or controller
qualification. Canvas alternatives for tile/event placement, focus traversal,
assistive-technology review, device hot-plug, and manual graphical evidence
remain PFU-06 work.

Verification is deferred by user instruction.
