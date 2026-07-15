# PFU-05 Work Packet: Perspective 2D asset-reference index

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Provide a read-only native index of stable asset references in saved Perspective
2D map documents and other owner-specific project documents as the first bounded
impact-analysis owner.

## Contract

1. The core index scans `content/maps/*.p2d.json` and `content/dialogues/*.json`
   and emits deterministic inbound references for tile palettes, prop palettes,
   prop instances, authored event asset metadata, and dialogue voice assets.
2. Character Creator asset slots are also indexed from
   `content/characters/*.json`: portrait, field/battle sprites, and layered
   parts. The saved audio-mix encounter-preview asset is indexed from
   `config/audio_mix_presets.json`. Grid Part catalogs under
   `content/part_catalogs/*.json` contribute their declared `assetId` fields,
   keyed by stable `partId`. Each edge records the asset ID, owning document
   path, owner kind, stable local ID, and an available stored project path.
3. Invalid saved map documents are reported as diagnostics and never mutated.
   The index does not change asset attachment, map loading, runtime rendering,
   or deletion behavior.
   It exposes inbound-by-asset and outbound-by-document queries over the same
   deterministic edge set.
4. The core can report attached-manifest IDs with no inbound edge from its
   currently indexed owners. This is an orphan candidate query only; it is not
   authorization to remove an asset while other owners remain unindexed.
5. A removal-impact preflight returns the same inbound edges and index
   diagnostics for one asset ID. It always reports `removal_authorized: false`:
   this is evidence for a future owner-specific remove/replace operation, not
   a deletion API.

## Limits

Quests, menus, database records, extensions, saves, safe rename/delete plans,
and richer impact UI remain later reference-graph increments. The existing
Assets surface exposes this bounded index through an explicit Project
References query and a read-only Removal Impact query.
Verification is deferred by user instruction.
