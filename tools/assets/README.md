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
  - `imports/root-drop/archives`

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
python .\tools\assets\asset_db.py index --roots third_party/itch-assets/packs imports/root-drop/archives
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
