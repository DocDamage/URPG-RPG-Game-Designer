# PFU-04 Work Packet: native Menu Studio persistence safety

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Make the existing native Menu Studio graph export/import path failure-safe. A
menu document will identify its native schema and active scene, publish through
a same-directory atomic replacement, and deserialize into a staged graph before
the live runtime graph is replaced.

## Traceability and owners

- **Source outcomes:** F05 WYSIWYG Menu Studio; I04 safe menu persistence;
  I05 configuration validation.
- **Authoritative data owner:** `engine/core/ui/menu_scene_graph.*` and
  `engine/core/ui/menu_serializer.*`.
- **Persistence projection:**
  `editor/diagnostics/diagnostics_workspace_serialization.cpp`.
- **Schema impact:** backward-compatible `urpg.menu_graph.v1` root metadata
  wraps the existing `scenes` array.

## Contract

1. Save writes a complete serialized graph to a temporary file beside the
   target, then atomically replaces the target. A failed write never truncates
   the prior published document.
2. Load parses and validates every scene in a separate graph first. Invalid
   JSON/schema/scene data leaves the live graph untouched.
3. A valid graph records and restores its active scene without using normal
   push/pop audio side effects. Older documents without root metadata remain
   importable.
4. Duplicate scene IDs and an invalid declared active scene fail closed rather
   than silently overwriting or opening an ambiguous menu graph.

## Acceptance and verification

- Failed save/load leaves the prior target/live graph intact.
- A round trip preserves all scenes, layout metadata, and active scene.
- Old native graph JSON remains loadable.

Verification commands (deferred):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][menu]" --reporter compact`

`git diff --check`

## Rollback and limits

Removing this packet's changes restores the previous diagnostics-only direct
file export/import behavior. Project-owned menu draft registration, dirty
tracking, close/recovery integration, undo/history, and editor-level conflict
handling remain separate F05 work.

## Implementation evidence

2026-07-15: `SerializeGraph()` now emits `urpg.menu_graph.v1` and the active
scene when one exists. `DeserializeGraph()` validates schema, scene identity,
duplicates, and active-scene references in a staging graph, then clears and
replaces the live graph only after validation succeeds. The diagnostics save
path writes a temporary file beside the target and atomically publishes it;
its load path routes both graph and legacy single-scene documents through the
staged importer before refreshing the native inspector. Deferred unit coverage
now records active-scene/layout round-trip, replacement, and invalid-import
non-mutation. No build, test, or formatting command was run because the user
explicitly directed implementation to continue without further test activity.
