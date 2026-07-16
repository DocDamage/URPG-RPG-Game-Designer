# PFU-07 Native Creator Tile Command

## Scope

This bounded developer-only slice removes durable creator-plan mutation from detached project JSON. It adds one reviewed native domain route: a `paint_tile` intent may apply only to the active Perspective 2D Map.

## Native ownership and safeguards

- `CreatorCommandPanel` supplies a reviewed plan plus explicit bindings from every planned layer/tile ID to an existing Perspective 2D palette entry.
- The native Perspective 2D Map workspace exposes this as a clearly labelled developer-only review/apply surface. It builds one explicit binding from the selected active tile layer and palette option, uses the local deterministic planner, and keeps provider transport dry-run only.
- `SpatialAuthoringWorkspace::applyNativeTileEdits` owns validation, mutation, dirty state, snapshot capture, and one local undo entry.
- The panel requires the plan's selected map to be the active Map, then the command checks the reviewed document revision, active map bounds, a visible unlocked tile layer, palette membership, and duplicate destination cells before changing the document.
- Props and event logic are rejected with an explicit unavailable state. They are not converted into a parallel project JSON authority.
- The legacy `applyCreatorCommandPlan` remains a non-mutating compatibility preview and reports `creator_generic_project_mutation_removed` for otherwise-valid plans.

## Deliberate limits

This does not promote the assistant, provider transport, RAG, multi-turn tool loop, or non-tile intent plans to shipping capability. It does not establish runtime/playtest qualification or complete WYSIWYG evidence. Automated verification was audited on 2026-07-16; see the active PFU evidence. Manual/package qualification remains outside this bounded packet.
