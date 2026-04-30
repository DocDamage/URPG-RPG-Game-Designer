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

Exact duplicates can be removed from the ignored raw intake after a catalog has been generated:

```powershell
python .\tools\assets\prune_urpg_stuff_duplicates.py
python .\tools\assets\prune_urpg_stuff_duplicates.py --apply
python .\tools\assets\promote_urpg_stuff_assets.py --exclude-audio
```

The duplicate pruner only removes cataloged `duplicate_source_paths` under `imports/raw/urpg_stuff`, preserves canonical files, and writes `imports/reports/asset_intake/urpg_stuff_duplicate_prune_report.json`.

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
