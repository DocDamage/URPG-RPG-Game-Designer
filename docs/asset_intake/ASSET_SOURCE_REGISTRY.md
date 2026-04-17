# URPG Asset Source Registry

> Registry of direct-ingest and discovery sources for private-use asset intake.  
> See `URPG_private_asset_intake_plan.md` and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` (P3-03, Phase 4 / Workstream 4.2).

---

## Status Legend

| Type | Meaning |
|------|---------|
| `direct_asset_pack` | May enter controlled staging, audit, normalization, and selective promotion |
| `discovery_index` | Tracked sourcing backlog input; do not ingest directly |
| `utility_pack` | Tools/scripts related to assets; evaluate case by case |

---

## Direct-Ingest Sources

These repos may be staged, audited, normalized, and promoted into URPG product lanes.

| # | Source ID | Repo Name | Source URL | Snapshot Commit | Snapshot Date | Type | Category Tags | Intended Use | Status |
|---|-----------|-----------|------------|-----------------|---------------|------|---------------|--------------|--------|
| 1 | `SRC-001` | AscensionGameDev/Intersect-Assets | `https://github.com/AscensionGameDev/Intersect-Assets` | `TBD` | `TBD` | `direct_asset_pack` | tilesets, characters, animations, UI, sounds, music, tools | Environment kit bootstrapping; character placeholder coverage; UI prototyping; audio resolver testing; animation/VFX placeholder coverage | Staged |
| 2 | `SRC-002` | GDQuest/game-sprites | `https://github.com/GDQuest/game-sprites` | `TBD` | `TBD` | `direct_asset_pack` | sprites, items, grid actors | Combat prototype actors; inventory/item placeholders; grid-based and interaction tests; early map/entity validation | Staged |
| 3 | `SRC-003` | Calinou/kenney-interface-sounds | `https://github.com/Calinou/kenney-interface-sounds` | `TBD` | `TBD` | `direct_asset_pack` | UI SFX, feedback sounds | Menu confirm/cancel; button hover/click; panel open/close; inventory and notification feedback; editor shell feedback sounds | Staged |

## Discovery / Source-Mining Sources

These repos are indexes and directories. They feed the acquisition backlog, not the direct-import lane.

| # | Source ID | Repo Name | Source URL | Type | Category Tags | Intended Use | Status |
|---|-----------|-----------|------------|------|---------------|--------------|--------|
| 4 | `SRC-004` | madjin/awesome-cc0 | `https://github.com/madjin/awesome-cc0` | `discovery_index` | textures, materials, HDRIs, 3D models, CC0 sources | Sourcing environment materials; finding future UI/icon/audio/3D references; building a vetted internal source list | Backlog |
| 5 | `SRC-005` | HotpotDesign/Game-Assets-And-Resources | `https://github.com/HotpotDesign/Game-Assets-And-Resources` | `discovery_index` | broad directory | Finding category-specific asset sources; locating emergency fill-ins for missing content classes; maintaining a future acquisition backlog | Backlog |

---

## Acquisition Backlog (from Discovery Sources)

Targets mined from `awesome-cc0` and `Game-Assets-And-Resources` for future intake.

| Priority | Category | Candidate Source | Notes | Status |
|----------|----------|------------------|-------|--------|
| P1 | Environment textures/materials | TBD | To be mined from `awesome-cc0` | Backlog |
| P1 | Icon packs | TBD | To be mined from `awesome-cc0` | Backlog |
| P2 | Fantasy UI frames | TBD | To be mined from discovery indexes | Backlog |
| P2 | Battle VFX sheets | TBD | To be mined from discovery indexes | Backlog |
| P2 | Ambient/background audio | TBD | To be mined from discovery indexes | Backlog |
| P3 | 3D materials/references | TBD | To be mined from `awesome-cc0` | Backlog |

---

## Manifest Schema

Each direct-ingest source must have a manifest under `imports/manifests/asset_sources/<source_id>.json`:

```json
{
  "source_id": "SRC-001",
  "repo_name": "AscensionGameDev/Intersect-Assets",
  "snapshot_commit": "abc1234",
  "snapshot_date": "2026-04-17",
  "source_type": "direct_asset_pack",
  "category_tags": ["tilesets", "characters", "ui", "sfx", "music", "vfx"],
  "intended_use": "Environment kit bootstrapping and placeholder coverage",
  "internal_notes": "Do not ingest entire repo as flat dump; promote curated subsets only."
}
```

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial registry created from `URPG_private_asset_intake_plan.md` |
