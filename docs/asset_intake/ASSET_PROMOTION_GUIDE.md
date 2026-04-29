# URPG Asset Promotion Guide

> Guide for staging → normalization → promotion workflow for private-use asset intake.
> See [URPG_private_asset_intake_plan.md](../archive/planning/asset_intake__URPG_private_asset_intake_plan.md) and [PROGRAM_COMPLETION_STATUS.md](../archive/planning/PROGRAM_COMPLETION_STATUS.md) (P3-03, Phase 4 / Workstream 4.2).

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
- Release-required assets must be explicitly marked in bundle manifests with `release_required: true`, `release_surfaces`, `license_cleared: true`, and `distribution: "bundled"`. The project-level release manifest in `content/fixtures/project_governance_fixture.json` lists every current title, map, battle, UI, audio, icon, and font surface consumed by the release candidate gate.
- `tools/ci/check_release_required_assets.ps1` validates that required repo-local assets exist, are hydrated, are not raw/vendor paths, and have license-cleared manifest metadata.
- Local WAV proofs under `imports/normalized/ui_sfx/` are ignored and excluded from GitHub until binary hosting is restored or an approved non-LFS release audio asset is promoted. Release UI/audio surfaces must use an explicit fallback entry or a hydrated bundled asset.

### Local Bulk Catalog-Normalization

Large local drops can enter the editor library before release promotion by producing a catalog-normalized report instead of copying every binary into `imports/normalized/`.

```powershell
python .\tools\assets\promote_urpg_stuff_assets.py
python .\tools\assets\promote_urpg_stuff_assets.py --exclude-audio
```

This writes:
- `imports/reports/asset_intake/urpg_stuff_promotion_catalog.json`
- `imports/reports/asset_intake/urpg_stuff_promotion_catalog/*.json`
- `imports/reports/asset_intake/urpg_stuff_promotion_summary.json`

The top-level catalog is a small manifest plus duplicate index; category shards give each raw asset a stable virtual `asset://` normalized path, inferred category, pack, tags, preview path, checksum, and duplicate marker. The editor asset library loads this optional catalog from the canonical report directory. These records are usable for local browsing, preview, and curation, but are not release/export eligible until a curated subset receives bundle manifests, attribution records, and copied or otherwise hydrated promoted payloads.

After catalog-normalization, exact raw duplicates can be pruned from the ignored `SRC-007` raw root while preserving the canonical catalog copy:

```powershell
python .\tools\assets\prune_urpg_stuff_duplicates.py
python .\tools\assets\prune_urpg_stuff_duplicates.py --apply
python .\tools\assets\promote_urpg_stuff_assets.py --exclude-audio
```

This writes `imports/reports/asset_intake/urpg_stuff_duplicate_prune_report.json`. Always regenerate the promotion catalog after applying the prune so duplicate counts and category shards reflect the cleaned raw tree.

Tooling and generator folders must be cataloged separately from game-art promotion:

```powershell
python .\tools\assets\catalog_urpg_generators.py --source-root "urpg stuff"
```

This writes `imports/reports/asset_intake/urpg_stuff_generator_candidates.json`. Treat those records as engineering-review candidates only; do not execute or bundle generator code until license, dependency, sandboxing, and product-surface review are complete. Candidate generators may target both URPG Maker editor panels and generated-game runtime features when the review approves that surface.

### Promotion Manifest Schema Example

```json
{
  "bundle_id": "BND-001",
  "bundle_name": "prototype_sprite_01",
  "source_id": "SRC-002",
  "assets": [
    {
      "original_relative_path": "sprites/hero.svg",
      "promoted_relative_path": "prototype_sprites/hero.svg",
      "category": "prototype_sprite",
      "status": "promoted",
      "release_required": true,
      "release_surfaces": ["title", "map", "battle"],
      "license_cleared": true,
      "distribution": "bundled"
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
| 2026-04-17 | Initial guide created from `docs/asset_intake/URPG_private_asset_intake_plan.md` |
| 2026-04-23 | TD Sprint 04 exercised the guide with `BND-001` and `BND-002`, proving one visual and one UI-audio promoted lane through source manifests, normalized assets, smoke-proof reporting, and export bundle staging. |
| 2026-04-25 | Added release attribution records for the promoted proof assets and clarified that shipped normalized assets must enter exports through promoted bundle manifests, not broad raw-intake or normalized-root discovery. |
| 2026-04-27 | Added release-required asset manifest metadata and CI validation for title, map, battle, UI, audio, icons, and font fallback surfaces. |
| 2026-04-27 | Deferred tracked WAV payloads from release packaging; local UI SFX proofs are ignored while release UI/audio surfaces use explicit fallback policy entries. |
| 2026-04-28 | Added local bulk catalog-normalization for `SRC-007` so large raw drops can be tagged, deduped, preview-addressed, and loaded by the editor asset library without duplicating or committing binary payloads. |
| 2026-04-28 | Added non-audio refresh handling and generator/tool candidate cataloging for `SRC-007`, including map/model/archive/tooling categories. |
| 2026-04-29 | Added exact duplicate pruning for the ignored `SRC-007` raw intake and clarified generator candidates can be reviewed for both editor and generated-game runtime surfaces. |
