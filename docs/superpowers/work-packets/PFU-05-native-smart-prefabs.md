# PFU-05 Native Smart Prefabs

## Scope

Provide the first native, versioned smart-prefab operation group through the
existing Grid Part Map owner. This replaces no existing catalog or Map format.

## Contract

- A Grid Part catalog may declare optional `smartPrefabs` alongside its stable
  part definitions. Each prefab has a stable ID, explicit version, declared
  part dependencies, conflict tags, parameters, and operations.
- The native Map preflight rejects unknown dependencies, unknown/missing or
  out-of-range parameters, duplicate operation IDs, unknown operation parts,
  unresolved parameter references, duplicate instance IDs, out-of-bounds
  footprints, conflicting tags, and non-permitted overlaps.
- Each accepted operation is materialized as a catalog-owned `PlacedPartInstance`
  with stable prefab provenance (`id`, `version`, operation, anchor, conflict
  tags, and resolved parameters). A `BulkGridPartCommand` applies or rolls back
  the complete group as one local undo/redo action.
- The Level Builder exposes the catalog-backed prefabs, editable declared
  parameter values, explicit review, and apply status. Review status identifies
  preflight-ready and blocked operation IDs, while a blocked group remains
  wholly unapplied. The initial bundled `town.vendor_stall` prefab is an
  auditable example, not a generic generator.

## Limits

This is native Grid Part authoring only. It does not create a project-wide
prefab store, cross-owner transaction, runtime/package claim, procedural
generation feature, collaboration/branch history, or a safe delete/rename
operation. Verification is deferred under the user's instruction not to run
builds or tests during this implementation phase.
