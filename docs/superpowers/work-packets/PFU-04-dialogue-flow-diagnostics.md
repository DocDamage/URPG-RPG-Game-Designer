# PFU-04 Work Packet: native dialogue graph flow diagnostics

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Add read-only structural and topology diagnostics to the existing native
`DialogueGraph` and expose them in the existing `DialogueGraphPanel` snapshot.

## Contract

1. Structural validation reports an empty graph, an absent/invalid start node,
   missing or duplicate choice IDs, and missing choice targets. Flow analysis
   runs only after structural validation succeeds.
2. Flow analysis reports an absent ending node, nodes unreachable from the
   start node, reachable nodes without an ending path, and non-ending nodes
   with no choices.
3. The diagnostics are authoring-only. They do not alter `previewRoute`, node
   insertion, serialization, or runtime dialogue behavior for existing graphs.
4. The panel preserves its graph snapshot and appends serializable diagnostic
   rows with stable code, message, node ID, and choice ID fields. It creates no
   project document, sidecar, or alternative persistence authority.

## Acceptance and verification

- A valid branching graph that reaches an ending reports no diagnostics.
- Broken targets and invalid start state report structural diagnostics without
  secondary topology output.
- Valid but orphaned, dead-end, cyclic, or end-less graphs report their
  corresponding flow codes in the panel snapshot.

Packet-local verification command (current pass not implied):

`./build/dev-ninja-debug/urpg_tests.exe "[dialogue][narrative]" --reporter compact`

## Rollback and limits

Removing the diagnostics leaves the prior dialogue model, preview, and panel
snapshot fields intact. Dialogue mutation, project-owned persistence/recovery,
undo, stable reference pickers, localization/voice/caption assignment,
migration, and runtime authored-graph proof remain separate F07 work.
