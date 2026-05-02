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

These repos are approved for controlled capture. As of 2026-04-23, TD Sprint 04 has promoted one bounded visual lane from `SRC-002` and one bounded UI-audio lane from `SRC-003`; the remaining rows are active intake implementation candidates.

| # | Source ID | Repo Name | Source URL | Snapshot Commit | Snapshot Date | Type | Category Tags | Intended Use | Capture State | Handling Path | Legal Disposition | Promotion Status |
|---|-----------|-----------|------------|-----------------|---------------|------|---------------|--------------|---------------|---------------|-------------------|------------------|
| 1 | `SRC-001` | AscensionGameDev/Intersect-Assets | `https://github.com/AscensionGameDev/Intersect-Assets` | `—` | `—` | `direct_asset_pack` | tilesets, characters, animations, UI, sounds, music, tools | Environment kit bootstrapping; character placeholder coverage; UI prototyping; audio resolver testing; animation/VFX placeholder coverage | `cataloged_not_mirrored` | `direct_ingest_when_captured` | `mixed_asset_pack_reference_only_until_per-asset_attribution_is_captured` | `not_started` |
| 2 | `SRC-002` | GDQuest/game-sprites | `https://github.com/GDQuest/game-sprites` | `ea05c63eb1d88af928d2d9a7445879500c9ece3f` | `2026-04-23` | `direct_asset_pack` | sprites, items, grid actors | Combat prototype actors; inventory/item placeholders; grid-based and interaction tests; early map/entity validation | `normalized` | `direct_ingest_when_captured` | `cc0_candidate_recorded_for_private_use_intake` | `promoted` |
| 3 | `SRC-003` | Calinou/kenney-interface-sounds | `https://github.com/Calinou/kenney-interface-sounds` | `4596a49eaf5a533948d49a47467f606bcdea70ff` | `2026-04-23` | `direct_asset_pack` | UI SFX, feedback sounds | Menu confirm/cancel; button hover/click; panel open/close; inventory and notification feedback; editor shell feedback sounds | `normalized` | `direct_ingest_when_captured` | `cc0_candidate_recorded_for_private_use_intake` | `promoted` |
| 4 | `SRC-006` | DocDamage/Unity-Duelyst-Animations | `https://github.com/DocDamage/Unity-Duelyst-Animations` | `—` | `—` | `direct_asset_pack` | sprites, unit animations, pixel art, battle actors | Battle actor animation prototypes; sprite pipeline atlas/preview stress testing; tactics/card-battler/monster-collector demo candidates | `cataloged_not_mirrored` | `direct_ingest_when_captured` | `cc0_candidate_recorded_for_private_use_intake` | `not_started` |
| 5 | `SRC-007` | Local URPG asset drop | `local://imports/raw/urpg_stuff` | `—` | `2026-04-28` | `direct_asset_pack` | side-scroller sprites, characters, pixel art, UI audio, voice audio, source art, isometric models, maps, generator tools | Raw asset cataloging; side-scroller animation source review; map/model/archive/tooling candidate review; generator review for editor panels and generated-game runtime features; UI/audio candidate review after OGG-only normalization | `mirrored` | `direct_ingest_when_captured` | `user_attested_free_for_game_use_requires_per_pack_attribution_and_tool_bundle_review` | `cataloged_local` |
| 6 | `SRC-008` | Local isometric animation and background drop | `local://imports/raw/urpg_stuff/assets_to_ingest_20260429` | `—` | `2026-04-29` | `direct_asset_pack` | isometric characters, sideview battlers, animation frames, sprite sheets, backgrounds, Lua metadata, source archives | Editor asset browser discovery for large animation-frame packs; runtime library candidates for isometric actors, sideview battlers, monsters, drones, trees, and backgrounds; animation metadata review | `mirrored` | `direct_ingest_when_captured` | `user_attested_free_for_game_use_requires_per_pack_attribution_and_bundle_review` | `in_review` |
| 7 | `SRC-009` | URPG repo-generated release starter skin | `local://repo/generated/release_skin` | `—` | `2026-05-02` | `direct_asset_pack` | UI, VFX, release skin | Bounded Phase 9 release starter frame/chrome, VFX, and cohesive skin metadata | `normalized` | `direct_ingest_when_captured` | `proprietary_distribution_rights_confirmed_project_owned_repo_generated_release_assets` | `promoted` |
| 8 | `SRC-010` | Local more assets to ingest drop | `local://imports/raw/more_assets_to_ingest` | `—` | `2026-05-02` | `direct_asset_pack` | tilesets, sprites, portraits, UI, VFX, audio, fonts, 3D models, maps, source art, metadata, archives | Template polish candidates for expanded RPG/non-RPG starters; editor asset browser discovery; WYSIWYG showcase candidate review | `mirrored` | `direct_ingest_when_captured` | `mixed_quarantine_with_bounded_cc0_starter_promotion` | `partially_promoted` |
| 9 | `SRC-011` | Local AssetsForMyGame drop | `local://imports/raw/assets_for_my_game` | `—` | `2026-05-02` | `direct_asset_pack` | tilesets, sprites, characters, UI, VFX, backgrounds, source art, archives | Editor asset browser discovery; RPG/non-RPG template polish candidates; curated future bundle promotion after attribution review | `mirrored` | `direct_ingest_when_captured` | `local_quarantine_requires_per_pack_attribution_and_bundle_review` | `cataloged_local` |
| 10 | `SRC-012` | Local GoDoGenUI asset library drop | `local://imports/raw/godogenui_assets` | `—` | `2026-05-02` | `direct_asset_pack` | tilesets, sprites, characters, portraits, UI, VFX, audio, backgrounds, source art | Editor asset browser discovery from large expanded local asset library; RPG/non-RPG template polish candidates; curated future bundle promotion after attribution review | `mirrored` | `direct_ingest_when_captured` | `mixed_quarantine_with_bounded_cc0_starter_promotion` | `partially_promoted` |

## Discovery / Source-Mining Sources

These repos are indexes and directories. They feed the acquisition backlog, not the direct-import lane.

| # | Source ID | Repo Name | Source URL | Type | Category Tags | Intended Use | Capture State | Handling Path | Status |
|---|-----------|-----------|------------|------|---------------|--------------|---------------|---------------|--------|
| 11 | `SRC-004` | madjin/awesome-cc0 | `https://github.com/madjin/awesome-cc0` | `discovery_index` | textures, materials, HDRIs, 3D models, CC0 sources | Sourcing environment materials; finding UI/icon/audio/3D references; building a vetted internal source list | `cataloged_not_mirrored` | `discovery_backlog_only` | Active acquisition source |
| 12 | `SRC-005` | HotpotDesign/Game-Assets-And-Resources | `https://github.com/HotpotDesign/Game-Assets-And-Resources` | `discovery_index` | broad directory | Finding category-specific asset sources; locating emergency fill-ins for missing content classes; maintaining an active acquisition queue | `cataloged_not_mirrored` | `discovery_backlog_only` | Active acquisition source |

---

## Acquisition Backlog (from Discovery Sources)

Targets mined from `awesome-cc0` and `Game-Assets-And-Resources` for current intake implementation.

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

- Canonical source manifests: `imports/manifests/asset_sources/SRC-001.json` through `SRC-012.json`
- Canonical status report: `imports/reports/asset_intake/source_capture_status.json`
- Current recorded state: 12 cataloged sources, 5 normalized/promoted proof lanes, 6 local catalog-normalized raw quarantine sources, 4 cataloged implementation candidates
- SRC-007 local catalog-normalization: `imports/reports/asset_intake/urpg_stuff_promotion_catalog.json` plus category shards under `imports/reports/asset_intake/urpg_stuff_promotion_catalog/` record 77,509 supported non-audio records, 56,096 canonical exact-hash records, 21,413 exact-duplicate recovered RAR records, inferred categories/tags, and preview paths for editor/library discovery. Raw binaries remain local under `imports/raw/urpg_stuff`.
- SRC-007 recovered RAR extraction: `imports/reports/asset_intake/src007_rar_extraction_report.json` records the old-intake HoriHori icon, Generic Pixel Dropship, and royal props RAR recovery into ignored raw quarantine under `imports/raw/urpg_stuff/refresh_20260428_2206/__rar_extracted/`; all three archives extracted successfully with 21,413 files present. These records remain local-use-only and export-ineligible until curated attribution and bundle promotion are completed.
- SRC-007 duplicate pruning: `imports/reports/asset_intake/urpg_stuff_duplicate_prune_report.json` records 30,495 exact duplicate raw files removed after preserving canonical copies.
- SRC-007 generator candidates: `imports/reports/asset_intake/urpg_stuff_generator_candidates.json` records `SpriteGenerator`, `pixel planet maker`, `SpaceBackground/BackgroundGenerator`, and `tiled-map editor` as engineering-review candidates for both URPG Maker editor integration and generated-game runtime use.
- SRC-007 local source archive: `imports/reports/asset_intake/local_source_drop_archive_20260429.json` records the already-ingested `urpg stuff` source drop moved out of the project root into `.urpg/archive/source_drops/20260429/`; the catalog source of truth remains `imports/raw/urpg_stuff`.
- SRC-008 aggregate animation/background catalog: `imports/reports/asset_intake/assets_to_ingest_20260429_promotion_catalog.json` plus category shards under `imports/reports/asset_intake/assets_to_ingest_20260429_promotion_catalog/` record 35 editor/library pack records over 662,009 local raw files. Raw files moved out of the project root into `imports/raw/urpg_stuff/assets_to_ingest_20260429`, no audio was present, and export remains blocked pending per-pack attribution and curated bundle promotion.
- SRC-010 mixed archive/loose catalog: `imports/reports/more_assets_to_ingest/more_assets_to_ingest_catalog.json` plus category shards under `imports/reports/more_assets_to_ingest/more_assets_to_ingest_catalog/` record 20,229 quarantine assets from 407 archive pack roots. ZIP and RAR payloads are extracted under `imports/raw/more_assets_to_ingest`. The full drop is also catalog-normalized for local editor/library use at `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog.json` plus category shards under `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog/`; these records are local-use-only and export-ineligible. The first curated review queue is recorded in `imports/reports/more_assets_to_ingest/src010_curated_promotion_plan.md`; `BND-004` promotes only 14 selected CC0/public-domain starter assets under `imports/normalized/src010_cc0_starter_pack/`, while the rest of SRC-010 remains blocked pending per-pack attribution and curated bundle promotion.
- SRC-011 local AssetsForMyGame catalog: `imports/reports/asset_intake/assets_for_my_game_promotion_catalog.json` plus category shards under `imports/reports/asset_intake/assets_for_my_game_promotion_catalog/` record 12,985 local-use records after extracting 58 ZIP/RAR archives with zero failures. Raw binaries remain local under `imports/raw/assets_for_my_game`; export remains blocked pending per-pack attribution and curated bundle promotion.
- SRC-012 local GoDoGenUI catalog: `imports/reports/asset_intake/godogenui_assets_promotion_catalog.json` plus category shards under `imports/reports/asset_intake/godogenui_assets_promotion_catalog/` record 29,516 supported local-use records from 46,285 mirrored files. Unsupported source/tool/development files are reported but not exposed as usable editor assets. `BND-005` promotes a bounded 6-file CC0/public-domain tiles/VFX subset under `imports/normalized/src012_cc0_tiles_vfx/`; all remaining raw binaries stay local under `imports/raw/godogenui_assets` and export-blocked pending per-pack attribution plus curated bundle promotion.
- Third-party/itch local index: `imports/reports/asset_intake/third_party_itch_ingest_summary.json` records the asset DB ingest over local raw quarantine roots including `imports/raw/third_party_assets/itch-assets`, source-only RPG Maker/Aseprite/root-drop archives, `imports/raw/third_party_assets/huggingface`, `imports/raw/third_party_assets/external-repos`, `imports/raw/third_party_assets/github_assets`, and `imports/raw/itch_assets/loose`. The bulk raw/source roots are ignored local working payloads, not repository payload or release promotion.
- First promoted visual lane: `imports/normalized/prototype_sprites/gdquest_blue_actor.svg` via `imports/manifests/asset_bundles/BND-001.json`
- First audio proof lane: `imports/manifests/asset_bundles/BND-002.json`; the former WAV payload was removed from GitHub and newly promoted audio must use OGG output from `tools/assets/convert_audio_to_ogg.py`
- Phase 9 release starter skin lane: `imports/manifests/asset_bundles/BND-003.json` promotes repo-generated UI frame/chrome, battle VFX proof, and cohesive skin metadata from `SRC-009`.
- SRC-010 CC0 starter lane: `imports/manifests/asset_bundles/BND-004.json` promotes a bounded non-release-required starter subset with attribution record `imports/reports/asset_intake/attribution/BND-004_src010_cc0_starter_pack.json`.
- SRC-012 CC0 tiles/VFX lane: `imports/manifests/asset_bundles/BND-005.json` promotes a bounded non-release-required starter subset with attribution record `imports/reports/asset_intake/attribution/BND-005_src012_cc0_tiles_vfx.json`.
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
| 2026-04-29 | Archived already-ingested local source-drop folders from the project root into `.urpg/archive/source_drops/20260429/` and repointed generator cataloging at the ingested raw refresh path. |
| 2026-04-29 | Indexed the third-party and itch asset folders into the local asset DB and recorded a tracked third-party/itch intake summary report. |
| 2026-04-29 | Added `SRC-008` for the local `assets to ingest` drop, moved it under ignored raw intake storage, and aggregate-cataloged 662,009 raw animation/background files as 35 editor/library pack records. |
| 2026-04-30 | Removed current Git tracking for source-only third-party/itch raw intake baggage while preserving local working files; curated RPG Maker plugin drop-ins, release plugin reports, Hugging Face fixtures, and tiny external/github README anchors remain tracked. |
| 2026-05-02 | Added `SRC-009` and `BND-003` for repo-generated Phase 9 release starter skin coverage. |
| 2026-05-02 | Added `SRC-010` for the local `more assets to ingest` drop, extracted ZIP/RAR payloads into raw quarantine, pruned only OS/archive junk, and cataloged 20,229 non-release candidate assets. |
| 2026-05-02 | Added the SRC-010 curated promotion review plan with CC0/public-domain first-pass candidates and attribution/redistribution review holds. |
| 2026-05-02 | Promoted `BND-004`, a 14-file SRC-010 CC0 starter subset covering UI chrome, isometric tiles, space/action visuals, and one font; broader SRC-010 quarantine remains blocked. |
| 2026-05-02 | Catalog-normalized all 20,229 SRC-010 records into the editor-ingestible local promotion catalog so quarantined assets are browseable and usable locally without becoming release/export assets. |
| 2026-05-02 | Recovered the three old-intake SRC-007 RAR archives into ignored raw quarantine, recorded `src007_rar_extraction_report.json`, and regenerated the SRC-007 local promotion catalog while excluding the separate SRC-008 aggregate drop. |
| 2026-05-02 | Added `SRC-011` and `SRC-012` from the computer-wide asset scan, mirrored both into ignored raw quarantine, extracted the `SRC-011` ZIP/RAR archives, and catalog-normalized both drops for local editor/library use only. |
| 2026-05-02 | Promoted `BND-005`, a 6-file SRC-012 CC0/public-domain tiles/VFX subset from Ansimuz/GoDoGenUI evidence packs; broader SRC-011/SRC-012 quarantine remains blocked pending per-pack attribution and curated review. |
