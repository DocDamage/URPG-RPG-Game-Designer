# PFU-04 Work Packet: native dialogue project mutation and local history

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Make the existing native `DialogueGraph` a project-owned Map-authoring draft
that creators can create, mutate, preview, save, undo/redo, and recover.

## Contract

1. Dialogue documents are native `urpg.dialogue_graph.v1` JSON at
   `content/dialogues/<dialogue-id>.json`. Save uses the editor's existing
   same-directory atomic writer; project switching reloads the current draft
   only after strict shape parsing succeeds.
2. The Map workspace owns creation, node add/update/remove, start-node
   selection, choice add/update/remove, and choice condition/effect add/remove.
   Node and choice updates preserve their stable IDs. Node removal is rejected
   for the active start node and atomically removes inbound choices. Choice
   addition/update requires existing source and target node IDs and a
   source-local unique choice ID. Conditions and effects require existing
   choices, non-empty keys (plus an operator for conditions), and reject exact
   duplicates.
3. Every non-no-op mutation stores one complete pre-mutation `DialogueGraph`
   in a 64-state local history, clears redo after a new edit, marks the native
   dialogue dirty surface, and uses the existing close guard, save, and private
   recovery snapshot paths. No JSON sidecar or second dialogue owner exists.
4. Node authoring requires stable node, speaker, and localization IDs. The
   authoring UI collects keys only from valid native locale bundles in
   `content/localization` and offers them as start/node localization selectors;
   it similarly derives speaker IDs from valid native character drafts under
   `content/characters`, using each file stem as the stable ID and its display
   name as a label. An absent or malformed document yields no selectable
   reference. When a valid locale catalog supplies keys, an authoring-only
   diagnostic identifies every node key absent from that catalog. The
   structural/flow/reference diagnostics do not modify preview traversal or
   runtime dialogue semantics. Each node can additionally store optional
   `voice_asset_id` and `caption_localization_key` fields in the existing v1
   document: voice selection accepts only an attached audio drag payload, and
   captions use the same valid project localization-key picker. On project
   bind, voice-reference diagnostics admit only valid attachment manifests
   that identify audio and retain a project-imported payload directory.
   Loading, replacing, and clearing either reference is one local-history
   mutation.
5. Dialogue nodes may carry optional authoring-only canvas coordinates in the
   existing v1 document. The Map workspace renders node cards and their choice
   links, loads a clicked node into the existing typed editor controls, and
   persists direct node dragging as one local-history mutation. Older graphs
   without coordinates receive a deterministic temporary layout until moved.

## Acceptance and verification

- A creator can select native project character, localization, and attached
  audio references; create a graph; add and update a localized node; connect
  and update a conditional choice with effects; select a start node; remove a
  non-start node; drag a node on the canvas; undo/redo each edit; save; and
  reopen the project through the native authoring route.
- Invalid document shapes or field types do not partially replace the active
  dialogue draft.
- The saved document round-trips the graph's existing schema without a new
  compatibility or export format.

Packet-local verification commands (current pass not implied):

`./build/dev-ninja-debug/urpg_tests.exe "[dialogue][narrative][editor]" --reporter compact`

## Rollback and limits

Removing this Map-owned draft route leaves the legacy dialogue graph and its
read-only panel intact. Catalog breadth/completeness workflow, document
migration, authored runtime execution of voice/captions, and manual creator
evidence remain separate F07 work.
