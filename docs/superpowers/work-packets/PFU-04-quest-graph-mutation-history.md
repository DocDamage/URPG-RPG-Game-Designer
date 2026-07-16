# PFU-04 Work Packet: native quest graph mutation and local history

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Extend the active project's native Quest Authoring surface from initial
three-node creation to bounded graph mutation. Authors can add/remove nodes and
links, add typed conditions and rewards, inspect validation/flow diagnostics,
and undo/redo the last 64 graph mutations through the existing project draft.

## Contract

1. All graph mutations retain the existing `QuestObjectiveGraphDocument`,
   dirty-state, save, close guard, recovery, preview, and runtime-application
   owner. No parallel quest sidecar is introduced.
2. Node removal removes its incident links atomically and cannot remove the
   last start node. Link creation requires distinct existing node IDs and
   rejects exact duplicates.
3. Local history captures complete pre-mutation documents, clears redo after a
   new edit, and retains the most recent 64 states.
4. `analyzeFlow` is an authoring diagnostic supplement: it reports missing
   completion nodes, unreachable nodes, and reachable nodes without a path to
   completion while preserving the existing compatibility validator and runtime
   preview contract.
5. Quest nodes may carry optional authoring-only canvas coordinates in the
   existing v1 document. The Map workspace renders node cards and links,
   loads a clicked node into the typed graph controls, and treats a direct
   node drag as one local-history mutation. Graphs without stored coordinates
   retain a deterministic temporary layout until a node is moved.
6. Node localization keys are selected only from valid native project locale
   bundles, can be changed through a stable-ID-preserving node update, and are
   checked as authoring diagnostics when a catalog is available. These checks
   do not change runtime preview or application semantics.
7. Conditions and rewards support exact add/remove mutations. Addition rejects
   duplicates; removal matches the typed ID/value tuple. Each successful
   mutation uses the existing single-step local history path.

## Acceptance and verification

- Node, link, condition, and reward edits are durable project mutations and
  restore as one undo/redo operation.
- Quest diagnostics expose both structural validation and graph-flow feedback.
- A creator can select and directly reposition a graph node without creating a
  parallel layout sidecar or changing quest runtime semantics.
- A creator can select and update a native project localization reference for
  a quest node, with a diagnostic if its catalog key later disappears.
- Preview/application remains blocked by the existing structural validator;
  flow analysis is visible author guidance, not an unproven runtime claim.

Packet-local verification commands (current pass not implied):

`./build/dev-ninja-debug/urpg_tests.exe "[quest][graph][editor]" --reporter compact`

## Rollback and limits

Removing the mutation controls leaves quest graph creation, save, and preview
unchanged. Dialogue graph authoring, localization/voice/caption linking,
stable-reference pickers, migrations beyond the existing schema, full runtime
softlock enforcement, and collaborative history remain separate PFU-04 work.
