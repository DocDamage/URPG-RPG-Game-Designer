# URPG Asset Promotion Guide

> Guide for staging → normalization → promotion workflow for private-use asset intake.  
> See `URPG_private_asset_intake_plan.md` and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` (P3-03, Phase 4 / Workstream 4.2).

---

## Overview

This document defines the canonical workflow for moving external assets from capture into URPG product lanes while preserving provenance, avoiding content-root drift, and maintaining license clarity.

Every promoted asset subset must retain a provenance chain made of:
- source manifest
- bundle manifest
- promotion record
- attribution/provenance linkage back to the captured source

---

## Phase A — Source Capture

1. Clone or download the direct-ingest repo into `third_party/github_assets/<repo-name>/`.
2. Record source metadata (repo name, branch/commit, snapshot date, intended use, source category).
3. Build a small manifest per source under `imports/manifests/asset_sources/<source_id>.json`.
4. Keep original folder structures intact during capture.

### Exit Criteria
- All direct-ingest repos are mirrored locally.
- Every source has a manifest entry.
- Provenance is recoverable without git archaeology.

---

## Phase B — Initial Audit

1. Run `tools/assets/asset_hygiene.py` against the new roots.
2. Produce reports for:
   - Duplicate files
   - Oversize files
   - Junk artifacts
   - Suspicious naming collisions
3. Manually inspect top-level structure of each pack.
4. Bucket assets into rough classes:
   - tilesets
   - characters
   - UI
   - animations / VFX
   - SFX
   - BGM
   - tools / source files

### Exit Criteria
- Hygiene report exists for each new pack.
- Category breakdown exists.
- Obvious junk and low-value noise are identified.

---

## Phase C — Controlled Normalization

1. Normalize names and folder structure for promoted subsets.
2. Preserve source pack identity in metadata.
3. Split promoted assets into logical URPG buckets under `imports/normalized/`:
   - `imports/normalized/ui_sfx/`
   - `imports/normalized/prototype_sprites/`
   - `imports/normalized/fantasy_environment/`
   - `imports/normalized/placeholder_characters/`
   - `imports/normalized/music/`
   - `imports/normalized/vfx/`
4. Generate per-bundle manifests under `imports/manifests/asset_bundles/`:
   - `original_source`
   - `original_relative_path`
   - `promoted_target_path`
   - `category`
   - `notes`

### Exit Criteria
- Normalized roots exist with clean substructure.
- Promoted assets are not anonymous.
- Source-to-target mapping is explicit.
- Every promoted subset has a promotion record that links the source manifest and bundle manifest entries used to justify promotion.
- ExportPackager may stage bundle manifests plus promoted assets into `data.pck`, but only for bundle manifests with `bundle_state: "promoted"` and asset rows with `status: "promoted"` that resolve to existing repo-local files under `imports/normalized/`.

### Promotion Manifest Schema Example

```json
{
  "bundle_id": "BND-001",
  "bundle_name": "prototype_ui_sfx_01",
  "source_id": "SRC-003",
  "assets": [
    {
      "original_relative_path": "audio/click.wav",
      "promoted_relative_path": "ui_sfx/click_01.wav",
      "category": "ui_sfx",
      "status": "normalized"
    }
  ]
}
```

---

## Phase D — Fast-Win Integration

Target integrations by priority:

### D1. UI Sound Pass (`kenney-interface-sounds`)
- Editor button click
- Panel open/close
- Command confirm/cancel
- Notification / toast feedback
- Menu navigation sounds

**Acceptance criteria:**
- Editor shell and one runtime menu path both use real UI sounds.
- Audio events are mapped through the state-driven audio layer, not ad hoc file calls.

### D2. Prototype Sprite Pass (`GDQuest/game-sprites`)
- Placeholder actors in map scene
- Simple combat entity visuals
- Item/weapon placeholders
- Test scenes for sprite atlas packing

**Acceptance criteria:**
- Prototype sprite set is visible in at least one map flow and one battle flow.
- Sprite pipeline can process the chosen subset cleanly.

### D3. Fantasy Environment Pass (`Intersect-Assets` subsets)
- One curated environment tileset bundle
- One placeholder character family
- One UI subset if it visually fits
- One VFX/animation subset
- One audio subset for environmental realism

**Acceptance criteria:**
- One representative vertical-slice scene uses real environment assets.
- One character/monster placeholder set is usable in-scene.
- One VFX subset is loaded and tested.

---

## Phase E — Source Mining Expansion

1. Review `awesome-cc0` by category.
2. Pull only the highest-value candidate sources into `ASSET_SOURCE_REGISTRY.md`.
3. Review `Game-Assets-And-Resources` for category gaps only.
4. Add follow-up targets by category, not by random browsing.

### Exit Criteria
- Discovery repos are converted into a structured acquisition backlog.
- No raw directory repo is mistaken for a direct asset pack.

---

## Do Not

- Do not ingest the entire repo as one flat asset dump.
- Do not merge raw content directly into production roots (`engine/`, `editor/`, `content/` production lanes).
- Do not assume naming/organization is already aligned with URPG conventions.
- Do not scatter raw audio file calls everywhere.
- Do not treat discovery directories as direct asset packs.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial guide created from `URPG_private_asset_intake_plan.md` |
