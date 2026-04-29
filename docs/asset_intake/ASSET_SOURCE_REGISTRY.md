# URPG Asset Source Registry

> Registry of direct-ingest and discovery sources for private-use asset intake.
> See [URPG_private_asset_intake_plan.md](../archive/planning/asset_intake__URPG_private_asset_intake_plan.md) and [PROGRAM_COMPLETION_STATUS.md](../archive/planning/PROGRAM_COMPLETION_STATUS.md) (P3-03, Phase 4 / Workstream 4.2).

---

## Status Legend

| Type | Meaning |
|------|---------|
| `direct_asset_pack` | May enter controlled staging, audit, normalization, and selective promotion |
| `discovery_index` | Tracked sourcing backlog input; do not ingest directly |
| `utility_pack` | Tools/scripts related to assets; evaluate case by case |

---

## Direct-Ingest Sources

These repos are approved for controlled capture. As of 2026-04-23, TD Sprint 04 has promoted one bounded visual lane from `SRC-002` and one bounded UI-audio lane from `SRC-003`; the remaining rows stay cataloged future intake candidates.

| # | Source ID | Repo Name | Source URL | Snapshot Commit | Snapshot Date | Type | Category Tags | Intended Use | Capture State | Handling Path | Legal Disposition | Promotion Status |
|---|-----------|-----------|------------|-----------------|---------------|------|---------------|--------------|---------------|---------------|-------------------|------------------|
| 1 | `SRC-001` | AscensionGameDev/Intersect-Assets | `https://github.com/AscensionGameDev/Intersect-Assets` | `—` | `—` | `direct_asset_pack` | tilesets, characters, animations, UI, sounds, music, tools | Environment kit bootstrapping; character placeholder coverage; UI prototyping; audio resolver testing; animation/VFX placeholder coverage | `cataloged_not_mirrored` | `direct_ingest_when_captured` | `mixed_asset_pack_reference_only_until_per-asset_attribution_is_captured` | `not_started` |
| 2 | `SRC-002` | GDQuest/game-sprites | `https://github.com/GDQuest/game-sprites` | `ea05c63eb1d88af928d2d9a7445879500c9ece3f` | `2026-04-23` | `direct_asset_pack` | sprites, items, grid actors | Combat prototype actors; inventory/item placeholders; grid-based and interaction tests; early map/entity validation | `normalized` | `direct_ingest_when_captured` | `cc0_candidate_recorded_for_private_use_intake` | `promoted` |
| 3 | `SRC-003` | Calinou/kenney-interface-sounds | `https://github.com/Calinou/kenney-interface-sounds` | `4596a49eaf5a533948d49a47467f606bcdea70ff` | `2026-04-23` | `direct_asset_pack` | UI SFX, feedback sounds | Menu confirm/cancel; button hover/click; panel open/close; inventory and notification feedback; editor shell feedback sounds | `normalized` | `direct_ingest_when_captured` | `cc0_candidate_recorded_for_private_use_intake` | `promoted` |
| 4 | `SRC-006` | DocDamage/Unity-Duelyst-Animations | `https://github.com/DocDamage/Unity-Duelyst-Animations` | `—` | `—` | `direct_asset_pack` | sprites, unit animations, pixel art, battle actors | Battle actor animation prototypes; sprite pipeline atlas/preview stress testing; tactics/card-battler/monster-collector demo candidates | `cataloged_not_mirrored` | `direct_ingest_when_captured` | `cc0_candidate_recorded_for_private_use_intake` | `not_started` |
| 5 | `SRC-007` | Local URPG asset drop | `local://imports/raw/urpg_stuff` | `—` | `2026-04-28` | `direct_asset_pack` | side-scroller sprites, characters, pixel art, UI audio, voice audio, source art, isometric models, maps, generator tools | Raw asset cataloging; side-scroller animation source review; map/model/archive/tooling candidate review; generator review for editor panels and generated-game runtime features; UI/audio candidate review after OGG-only normalization | `mirrored` | `direct_ingest_when_captured` | `user_attested_free_for_game_use_requires_per_pack_attribution_and_tool_bundle_review` | `cataloged_local` |

## Discovery / Source-Mining Sources

These repos are indexes and directories. They feed the acquisition backlog, not the direct-import lane.

| # | Source ID | Repo Name | Source URL | Type | Category Tags | Intended Use | Capture State | Handling Path | Status |
|---|-----------|-----------|------------|------|---------------|--------------|---------------|---------------|--------|
| 6 | `SRC-004` | madjin/awesome-cc0 | `https://github.com/madjin/awesome-cc0` | `discovery_index` | textures, materials, HDRIs, 3D models, CC0 sources | Sourcing environment materials; finding future UI/icon/audio/3D references; building a vetted internal source list | `cataloged_not_mirrored` | `discovery_backlog_only` | Acquisition backlog source |
| 7 | `SRC-005` | HotpotDesign/Game-Assets-And-Resources | `https://github.com/HotpotDesign/Game-Assets-And-Resources` | `discovery_index` | broad directory | Finding category-specific asset sources; locating emergency fill-ins for missing content classes; maintaining a future acquisition backlog | `cataloged_not_mirrored` | `discovery_backlog_only` | Acquisition backlog source |

---

## Acquisition Backlog (from Discovery Sources)

Targets mined from `awesome-cc0` and `Game-Assets-And-Resources` for future intake.

| Priority | Category | Candidate Source Class | Notes | Status |
|----------|----------|------------------------|-------|--------|
| P1 | Environment textures/materials | `awesome-cc0` material and texture candidates | Register each selected downstream source under a new `SRC-*` record before capture | Discovery backlog only |
| P1 | Icon packs | `awesome-cc0` UI/icon candidates | Prefer permissive packs with clear attribution metadata | Discovery backlog only |
| P2 | Fantasy UI frames | Discovery-index UI pack candidates | Require style-fit review before any capture | Discovery backlog only |
| P2 | Battle VFX sheets | Discovery-index VFX candidates | Capture only after bundle manifest and promotion path exist | Discovery backlog only |
| P2 | Ambient/background audio | Discovery-index audio candidates | Favor small curated subsets over broad dumps | Discovery backlog only |
| P3 | 3D materials/references | `awesome-cc0` 3D material candidates | Presentation experiments only until a product lane requests them | Discovery backlog only |

---

## Manifest Schema

Each direct-ingest source must have a manifest under `imports/manifests/asset_sources/<source_id>.json` and conform to `imports/manifests/asset_sources/asset_source.schema.json`:

```json
{
  "source_id": "SRC-001",
  "repo_name": "AscensionGameDev/Intersect-Assets",
  "source_url": "https://github.com/example/example-assets",
  "capture_state": "cataloged_not_mirrored",
  "snapshot_commit": null,
  "snapshot_date": null,
  "source_type": "direct_asset_pack",
  "category_tags": ["tilesets", "characters", "ui", "sfx", "music", "vfx"],
  "intended_use": ["Environment kit bootstrapping and placeholder coverage"],
  "handling_path": "direct_ingest_when_captured",
  "legal_disposition": "mixed_asset_pack_reference_only_until_per-asset_attribution_is_captured",
  "promotion_status": "not_started",
  "notes": ["Do not ingest entire repo as flat dump; promote curated subsets only."]
}
```

## Current Capture Snapshot

- Canonical source manifests: `imports/manifests/asset_sources/SRC-001.json` through `SRC-007.json`
- Canonical status report: `imports/reports/asset_intake/source_capture_status.json`
- Current recorded state: 7 cataloged sources, 2 normalized/promoted proof lanes, 1 local catalog-normalized raw quarantine source, 4 cataloged future candidates
- SRC-007 local catalog-normalization: `imports/reports/asset_intake/urpg_stuff_promotion_catalog.json` plus category shards under `imports/reports/asset_intake/urpg_stuff_promotion_catalog/` record 56,096 supported non-audio/tool records, 56,096 canonical exact-hash records, 0 duplicate groups, inferred categories/tags, and preview paths for editor/library discovery. Raw binaries remain local under `imports/raw/urpg_stuff`.
- SRC-007 duplicate pruning: `imports/reports/asset_intake/urpg_stuff_duplicate_prune_report.json` records 30,495 exact duplicate raw files removed after preserving canonical copies.
- SRC-007 generator candidates: `imports/reports/asset_intake/urpg_stuff_generator_candidates.json` records `SpriteGenerator`, `pixel planet maker`, `SpaceBackground/BackgroundGenerator`, and `tiled-map editor` as engineering-review candidates for both URPG Maker editor integration and generated-game runtime use.
- First promoted visual lane: `imports/normalized/prototype_sprites/gdquest_blue_actor.svg` via `imports/manifests/asset_bundles/BND-001.json`
- First audio proof lane: `imports/manifests/asset_bundles/BND-002.json`; the former WAV payload was removed from GitHub and future promoted audio must use OGG output from `tools/assets/convert_audio_to_ogg.py`
- Release attribution records: `imports/reports/asset_intake/attribution/SRC-002_gdquest_blue_actor.json` and `imports/reports/asset_intake/attribution/SRC-003_kenney_click_001.json`

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial registry created from `docs/asset_intake/URPG_private_asset_intake_plan.md` |
| 2026-04-19 | Replaced placeholder staged-state rows with concrete capture-state, handling-path, legal-disposition, and promotion-status records linked to the canonical manifests and intake report. |
| 2026-04-23 | TD Sprint 04 promoted the first bounded visual (`SRC-002`) and UI-audio (`SRC-003`) proof lanes with source snapshots, normalized assets, bundle manifests, and smoke-proof reporting. |
| 2026-04-25 | Added per-asset release attribution records for the promoted visual and UI-audio lanes and tightened export discovery so promoted normalized assets ship through bundle manifests rather than generic discovery. |
| 2026-04-27 | Deferred tracked WAV payloads from release-required GitHub packaging while retaining the governed BND-002 attribution record for local/non-release validation. |
| 2026-04-28 | Added `SRC-006` for `DocDamage/Unity-Duelyst-Animations` plus a normalized metadata importer for Duelyst unit animation atlases; no release promotion was made. |
| 2026-04-28 | Added `SRC-007` for the local `urpg stuff` raw asset drop under `imports/raw/urpg_stuff`; user attested the assets are free for game use, non-OGG audio is excluded from the raw drop, and promotion still requires per-pack attribution plus tool-bundle review. |
| 2026-04-28 | Removed tracked WAV/MP3 payloads from GitHub, converted the refreshed local audio drop to OGG, copied OGG outputs into `SRC-007`, and archived local WAV/MP3 sources under `.urpg/local_audio_archive`. |
| 2026-04-28 | Catalog-normalized all supported `SRC-007` files into an editor-loadable local promotion catalog with stable virtual asset ids, category/tag inference, preview paths, and exact duplicate grouping; release export remains blocked pending per-pack attribution and bundle manifests. |
| 2026-04-28 | Refreshed `SRC-007` with non-audio assets from `C:\dev\URPG Maker\urpg stuff`, expanded cataloging for maps/models/archives/tooling, ignored audio per user instruction, and recorded generator/tool integration candidates. |
| 2026-04-29 | Pruned 30,495 exact duplicate files from the ignored `SRC-007` raw intake, regenerated the non-audio catalog with zero duplicate groups, and marked generator candidates for editor plus generated-game runtime integration review. |
