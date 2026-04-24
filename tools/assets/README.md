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
- Default DB path: `third_party/asset-index/asset_catalog.db`
- Default roots:
  - `third_party/itch-assets/packs`
  - `third_party/itch-assets/loose-files`
  - `third_party/rpgmaker-mz`
  - `third_party/huggingface`
  - `imports/root-drop/archives`
  - `imports/raw/more_assets`

## Usage
```powershell
python .\tools\assets\asset_db.py init
python .\tools\assets\asset_db.py index
python .\tools\assets\asset_db.py stats
python .\tools\assets\asset_db.py dupes --show-paths --limit 20
python .\tools\assets\asset_db.py find --query "dragon" --limit 25
python .\tools\assets\asset_db.py find --kind image --ext png --pack "Monster Mega"
```

## Custom roots
```powershell
python .\tools\assets\asset_db.py index --roots third_party/itch-assets/packs third_party/huggingface imports/root-drop/archives
```

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

## Safe duplicate prune wave
Conservative duplicate cleanup for extracted working copies (`itch/unzipped`) when canonical copies already exist.

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
