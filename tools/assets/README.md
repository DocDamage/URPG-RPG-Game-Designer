# Asset Catalog

Robust local asset index for this repo using SQLite + FTS5.

## What it does
- Incremental indexing by file `size + mtime`.
- SHA-256 hashing for duplicate detection.
- Image metadata (`width`, `height`) for common formats (`png`, `jpg/jpeg`, `gif`, `bmp`, `webp`).
- Audio/video duration via `ffprobe` when available.
- Auto tags (`kind:*`, `ext:*`, `category:*`, `pack:*`, `name:*`).
- Full text search over paths, filenames, pack/category, and tags.

## Database
- Default DB path: `.urpg/asset-index/asset_catalog.db`
- Default roots:
  - `imports/raw/third_party_assets/itch-assets/packs`
  - `imports/raw/third_party_assets/itch-assets/loose-files`
  - `imports/raw/third_party_assets/rpgmaker-mz`
  - `imports/raw/third_party_assets/huggingface`
- `imports/root-drop/archives`
- `imports/raw/more_assets`
- `imports/raw/more_assets_to_ingest`
- `imports/raw/assets_for_my_game`
- `imports/raw/godogenui_assets`

Most broad raw/source roots are intentionally ignored local quarantine. The catalog can index them when they exist in a developer checkout, but reports and promoted manifests are the repository source of truth.

## Usage
```powershell
python .\tools\assets\asset_db.py init
python .\tools\assets\asset_db.py index
python .\tools\assets\asset_db.py index --roots imports/raw/third_party_assets/itch-assets/packs imports/raw/third_party_assets/itch-assets/loose-files imports/raw/third_party_assets/rpgmaker-mz imports/raw/third_party_assets/huggingface imports/raw/third_party_assets/aseprite imports/raw/third_party_assets/external-repos imports/raw/third_party_assets/github_assets imports/raw/itch_assets/loose
python .\tools\assets\asset_db.py stats
python .\tools\assets\asset_db.py dupes --show-paths --limit 20
python .\tools\assets\asset_db.py find --query "dragon" --limit 25
python .\tools\assets\asset_db.py find --kind image --ext png --pack "Monster Mega"
python .\tools\assets\report_third_party_itch_ingest.py
```

## Custom roots
```powershell
python .\tools\assets\asset_db.py index --roots imports/raw/third_party_assets/itch-assets/packs imports/raw/third_party_assets/huggingface imports/root-drop/archives
```

`imports/raw/urpg_stuff` is included in the default roots when the local drop exists.
`imports/raw/more_assets_to_ingest` is included for the SRC-010 zip/RAR/loose-file drop when that local quarantine exists.
`imports/raw/assets_for_my_game` and `imports/raw/godogenui_assets` are included for the SRC-011/SRC-012 local quarantine drops when present.

`report_third_party_itch_ingest.py` writes a tracked summary report to `imports/reports/asset_intake/third_party_itch_ingest_summary.json` after a third-party/itch indexing pass. The SQLite DB remains local under `.urpg/asset-index/`.

## Convenience wrappers
```powershell
.\tools\assets\asset-index.ps1
.\tools\assets\asset-find.ps1 -Query "dragon AND kind:image" -Limit 25
```

## Hygiene pass
Scan for duplicate files, oversized files, and removable OS junk.

```powershell
python .\tools\assets\asset_hygiene.py --write-reports
python .\tools\assets\asset_hygiene.py --write-reports --prune-junk
```

Reports are written to `imports/reports/`:
- `asset_hygiene_summary.json`
- `asset_hygiene_duplicates.csv`
- `asset_hygiene_oversize.csv`
- `asset_hygiene_junk.csv`
- `asset_hygiene_hash_skips.csv`

## Hugging Face curated imports
Curated third-party samples and manifest-only references from selected Hugging Face datasets.

```powershell
.\tools\assets\ingest_huggingface_curated.ps1
.\tools\assets\ingest_huggingface_curated.ps1 -Refresh
```

Inputs and reports:
- `tools/assets/huggingface_curated_manifest.json`
- `imports/reports/huggingface_curated_inventory.json`

## More-assets archive intake
Inventory and safely extract local zip archives from `more assets/` into a quarantined raw intake folder.
Installers are cataloged but never executed.

```powershell
.\tools\assets\ingest_more_assets.ps1
.\tools\assets\ingest_more_assets.ps1 -InventoryOnly
```

Inputs and outputs:
- Source: `more assets/`
- Extracted files: `imports/raw/more_assets/`
- Manifest: `imports/reports/more_assets/more_assets_intake_manifest.json`

## Duelyst animation metadata intake
Convert a local checkout of `DocDamage/Unity-Duelyst-Animations` into URPG `sprite_atlas` manifests.
The tool does not clone the source repo and does not promote generated content for release.

```powershell
git clone https://github.com/DocDamage/Unity-Duelyst-Animations.git ..\Unity-Duelyst-Animations
python .\tools\assets\ingest_duelyst_animations.py `
  --source-root ..\Unity-Duelyst-Animations `
  --output-root .\imports\normalized\duelyst_animations `
  --limit 10
```

Add `--copy-textures` only for local/non-release preview work. Generated reports and atlas metadata are normalized intake output with `export_eligible=false` until a promotion manifest and attribution review approve a curated subset.

## Audio OGG conversion
Convert intake audio assets to OGG/Vorbis and write an audit manifest.
Build/cache/vendor folders are skipped by default.

```powershell
python .\tools\assets\convert_audio_to_ogg.py --dry-run
python .\tools\assets\convert_audio_to_ogg.py --delete-source
```

The report is written to `imports/reports/audio_conversion/audio_to_ogg_manifest.json`.

## Local URPG asset catalog-normalization
Build an editor-loadable catalog for the local `SRC-007` raw asset drop without copying the binary payload into a second tree.

```powershell
python .\tools\assets\promote_urpg_stuff_assets.py
python .\tools\assets\promote_urpg_stuff_assets.py --exclude-audio
```

Outputs:
- `imports/reports/asset_intake/urpg_stuff_promotion_catalog.json`
- `imports/reports/asset_intake/urpg_stuff_promotion_catalog/*.json`
- `imports/reports/asset_intake/urpg_stuff_promotion_summary.json`

The top-level catalog is a manifest plus duplicate index; category shards record stable virtual normalized paths, source paths, inferred categories, packs, tags, preview paths, SHA-256 hashes, image/audio/model/map metadata, and exact duplicate markers. Use `--exclude-audio` when a refresh should ignore audio. The editor asset library loads the manifest and shards from the canonical report directory when present. The catalog is local-discovery ready but remains `export_eligible=false` until curated subsets get per-pack attribution and promoted bundle manifests.

Category shards are capped at 5,000 records by default and split into `*-part-###.json` files when needed. Keep the default cap for Git-tracked reports so regenerated catalogs do not create oversized JSON shards.

Exact duplicates can be removed from the ignored raw intake after a catalog has been generated:

```powershell
python .\tools\assets\prune_urpg_stuff_duplicates.py
python .\tools\assets\prune_urpg_stuff_duplicates.py --apply
python .\tools\assets\promote_urpg_stuff_assets.py --exclude-audio
```

The duplicate pruner only removes cataloged `duplicate_source_paths` under `imports/raw/urpg_stuff`, preserves canonical files, and writes `imports/reports/asset_intake/urpg_stuff_duplicate_prune_report.json`.

## SRC-010 local catalog-normalization
Build the editor-loadable local catalog for the `more assets to ingest` drop after archive extraction and raw catalog generation:

```powershell
python .\tools\assets\catalog_more_assets_to_ingest.py
python .\tools\assets\asset_db.py index --roots imports/raw/more_assets_to_ingest --force
```

Outputs:
- `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog.json`
- `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog/*.json`
- `imports/reports/asset_intake/more_assets_to_ingest_promotion_summary.json`

These records make SRC-010 browseable/searchable in local editor/library surfaces through stable `asset://src-010/...` ids and raw preview paths. They are local-use-only records: `export_eligible=false` and `release_use_allowed=false` until a curated subset is copied into `imports/normalized/` and receives bundle plus attribution manifests.

## Generic local-drop catalog-normalization
Build editor-loadable local catalogs for additional raw quarantine drops:

```powershell
python .\tools\assets\catalog_local_asset_drop.py `
  --source-id SRC-011 `
  --source-root imports/raw/assets_for_my_game `
  --output-catalog imports/reports/asset_intake/assets_for_my_game_promotion_catalog.json `
  --output-summary imports/reports/asset_intake/assets_for_my_game_promotion_summary.json `
  --output-shard-root imports/reports/asset_intake/assets_for_my_game_promotion_catalog

python .\tools\assets\catalog_local_asset_drop.py `
  --source-id SRC-012 `
  --source-root imports/raw/godogenui_assets `
  --output-catalog imports/reports/asset_intake/godogenui_assets_promotion_catalog.json `
  --output-summary imports/reports/asset_intake/godogenui_assets_promotion_summary.json `
  --output-shard-root imports/reports/asset_intake/godogenui_assets_promotion_catalog
```

These catalogs are local-use-only. They provide stable virtual asset ids and preview paths for editor/library browsing, but do not make raw files release/export eligible.

The generic cataloger also caps shards at 5,000 records and removes stale shard files before rewriting the shard directory.

`BND-005` is the first governed promotion from these local drops:

- `imports/manifests/asset_bundles/BND-005.json`
- `imports/reports/asset_intake/attribution/BND-005_src012_cc0_tiles_vfx.json`
- `imports/normalized/src012_cc0_tiles_vfx/`

It promotes only six selected `SRC-012` CC0/public-domain tiles/VFX PNGs. All other `SRC-011` and `SRC-012` records remain local-use-only until curated attribution review.

## Promoted grid-part generation
Generate editor-ready Level Builder thumbnails and catalogs from already promoted bundle manifests.

```powershell
python .\tools\assets\generate_promoted_grid_parts.py `
  --repo-root . `
  --bundle imports\manifests\asset_bundles\BND-006.json `
  --bundle imports\manifests\asset_bundles\BND-007.json `
  --bundle imports\manifests\asset_bundles\BND-008.json `
  --output-root content\assets\gameplay\promoted_legacy `
  --catalog-root content\part_catalogs\generated\promoted_legacy `
  --report-output imports\reports\asset_intake\promoted_legacy_grid_part_generation_report.json
```

The generator uses only manifest records that are image assets, license-cleared, and release-eligible. It writes small thumbnails for editor browsing and leaves the original promoted payloads under `imports/normalized`.

## CuteSCKR full grid-part generation
Extract all CuteSCKR archives from the 2026-05-04 drop into raw quarantine:

```powershell
python .\tools\assets\extract_local_asset_archives.py `
  --repo-root . `
  --source-id SRC-015 `
  --source-root "ingest stuff 5-4-26\CuteSCKR" `
  --extract-root imports\raw\cutesckr_all `
  --report imports\reports\asset_intake\cutesckr_full_archive_extraction_report.json
```

Generate Level Builder slices and aggregate includes:

```powershell
python .\tools\assets\generate_cutesckr_grid_parts.py `
  --repo-root . `
  --source-root imports\raw\cutesckr_all\__archive_extracted `
  --output-root content\assets\gameplay\cutesckr_all `
  --catalog-root content\part_catalogs\generated\cutesckr_all `
  --aggregate-catalog content\part_catalogs\generated\cutesckr_all_parts.json `
  --tile-size 48 `
  --max-tiles-per-sheet 96 `
  --report-output imports\reports\asset_intake\cutesckr_full_grid_part_generation_report.json
```

The generator writes source-bundle metadata from each extracted archive root so the Level Builder source filter can narrow the 90-pack import.

## Individual image game-maker catalogs
Generate game-maker catalog entries from folders of standalone images such as portraits.

```powershell
python .\tools\assets\generate_image_folder_grid_parts.py `
  --repo-root . `
  --source-root imports\raw\human_rpg_portraits\__archive_extracted `
  --output-root content\assets\gameplay\human_rpg_portraits `
  --catalog-output content\part_catalogs\generated\human_rpg_portraits_parts.json `
  --catalog-id human_rpg_portraits `
  --display-name "Human RPG Portraits" `
  --part-prefix human_portrait `
  --category Npc `
  --source-bundle-id human_rpg_portraits `
  --max-thumbnail-size 96 `
  --report-output imports\reports\asset_intake\human_rpg_portrait_grid_part_generation_report.json
```

ModernUI app-control skinning is out of scope for this path. The promoted ModernUI portrait-generator PNGs can still be exposed for game-maker use with `generate_promoted_grid_parts.py` and `BND-010`.

The default editor catalog remains the smaller `content/part_catalogs/base_jrpg_parts.json` for startup performance. Use `content/part_catalogs/game_maker_all_parts.json` when a run needs the complete generated game-maker library.

## UI theme manifest generation
Mirror candidate UI theme folders, copy runtime UI art, and generate completeness metadata plus validation reports.

```powershell
python .\tools\assets\generate_ui_theme_manifest.py `
  --repo-root . `
  --source-root "F:\Complete_UI_Essential_Pack_Free" `
  --theme-id complete_ui_essential_flat `
  --display-name "Complete UI Essential Flat" `
  --raw-output imports\raw\ui_themes\complete_ui_essential_flat `
  --asset-output content\assets\ui_themes\complete_ui_essential_flat `
  --manifest-output content\ui_themes\complete_ui_essential_flat.json `
  --report-output imports\reports\ui_theme_validation\complete_ui_essential_flat.json `
  --surface game_ui_theme
```

Generated manifests conform to `content/schemas/ui_theme.schema.json` version 2 and distinguish `game_ui_theme` from `editor_theme`. As of 2026-05-05:

- `complete_ui_essential_flat` ingests 102 assets and passes `gameUiReady`.
- `wenrexa_hologram` ingests 48 assets and remains `candidate` because it is missing bar/progress assets.
- Neither pack is editor-theme-ready; editor theme use still requires hand-authored color tokens, font pairing, semantic icon aliases, nine-slice metadata, scale policy, and missing editor-control states.

Template-scoped browsing, previews, and onboarding are now wired into the editor model/panel flow. The shipped path uses bounded starter indexes by default, keeps `content/part_catalogs/game_maker_all_parts.json` opt-in, and exposes preview metadata plus Level Builder handoff through the asset browser. Follow-up details are tracked in `docs/superpowers/plans/2026-05-05-game-maker-asset-browser-onboarding-plan.md` and `docs/superpowers/plans/2026-05-06-game-maker-onboarding-finish-plan.md`.

## Local generator/tool candidate catalog
Catalog generator and tool source folders from a local drop for engineering review without executing them.

```powershell
python .\tools\assets\catalog_urpg_generators.py --source-root "imports/raw/urpg_stuff/refresh_20260428_2206"
```

Output:
- `imports/reports/asset_intake/urpg_stuff_generator_candidates.json`

Current candidate kinds include Godot sprite/background generators, source archive generators, and Tiled map-editor interop/reference code. These records are review inputs only; integration into URPG Maker requires license, dependency, sandboxing, and product-surface review. Candidate records include target surfaces for both editor panels and generated-game runtime use where applicable.

## Safe duplicate prune wave
Conservative duplicate cleanup for extracted working copies (`imports/raw/itch_assets/unzipped`) when canonical copies already exist.

```powershell
python .\tools\assets\prune_safe_duplicates.py
python .\tools\assets\prune_safe_duplicates.py --apply
```

Reports are written to `imports/reports/`:
- `asset_safe_dedupe_plan.csv`
- `asset_safe_dedupe_applied.csv`
- `asset_safe_dedupe_summary.json`

## More-assets raw duplicate planning
Report-only duplicate cleanup planning for `imports/raw/more_assets`.
The planner only considers exact duplicates inside the same extracted archive root and preserves license/readme files.

```powershell
python .\tools\assets\plan_more_assets_dedupe.py
python .\tools\assets\plan_more_assets_dedupe.py --apply --max-removals 100
```

Reports are written to `imports/reports/more_assets/`:
- `more_assets_safe_dedupe_plan.csv`
- `more_assets_safe_dedupe_applied.csv`
- `more_assets_safe_dedupe_summary.json`
