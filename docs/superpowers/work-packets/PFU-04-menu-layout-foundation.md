# PFU-04 Work Packet: native Menu Studio layout foundation

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Add the first persisted native layout contract for Menu Studio: a design
canvas, per-pane rectangle, layer, and deterministic focus-order metadata.
The contract is owned by the native menu scene graph and is consumed by the
serializer, inspector, and preview. It is deliberately a foundation for later
drag editing, guides, constraints, templates, drafts, and recovery rather than
an editor-only visual overlay.

## Traceability and owners

- **Source outcomes:** F05 WYSIWYG Menu Studio; I04 safe menu persistence;
  I05 configuration validation.
- **Authoritative owner:** `engine/core/ui/menu_scene_graph.*` and
  `engine/core/ui/menu_serializer.*`.
- **Editor projections:** `editor/ui/menu_inspector_*` and
  `editor/ui/menu_preview_panel.*`.
- **Schema impact:** backward-compatible native menu JSON adds an explicit
  layout version, design canvas, and optional pane layout objects.

## Contract

1. New scenes use a bounded, explicit design canvas; old documents receive the
   native default canvas and pane layout without migration loss.
2. Each visible pane has a finite rectangle and z layer. A non-negative focus
   order overrides insertion order; otherwise legacy insertion order remains
   the navigation fallback.
3. Invalid canvas/pane values and overlapping visible rectangles are surfaced
   as inspector diagnostics. The runtime does not reinterpret invalid data as
   an implicit layout.
4. Serialization round-trips the same native graph data used by runtime focus
   traversal and preview. No MZ sidecar or HTML export is involved.

## Acceptance and verification

- A menu graph round-trips canvas and pane layout metadata.
- Focus traversal follows explicit focus order while legacy panes retain their
  existing insertion-order behavior.
- Inspector and preview snapshots expose layout/focus data and layout issues.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][menu]" --reporter compact`

`git diff --check`

## Rollback and limits

Removing the layout fields restores the legacy stacked preview and
insertion-order focus behavior. Interactive drag handles, snapping/guides,
responsive constraints, reusable components/templates, draft/history UI,
atomic project publication, and target-size/manual evidence remain separate
F05 work.

## Implementation evidence

2026-07-15: `MenuScene` now owns a bounded 1280x720 default design canvas;
each `MenuPane` carries a validated rectangle, z layer, and optional explicit
focus order. The native serializer emits `layout_version: 1`, canvas, and pane
layout fields, rejects malformed/unknown layout versions, and gives pre-layout
documents deterministic legacy pane placement at load time. `MenuSceneGraph`
uses explicit focus order for horizontal pane traversal while retaining legacy
insertion order when no order is supplied. Inspector rows/summaries and
diagnostics expose the data and report invalid, outside-canvas, and overlapping
layouts. The preview renders the native canvas at the serialized pane
rectangles/layers. The existing Menu Inspector exposes bounded numeric canvas,
rectangle, layer, and focus-order edits and applies accepted changes through
the runtime graph owner. No build, test, or formatting command was run after
this increment because the user explicitly directed implementation to continue
without further test activity.
