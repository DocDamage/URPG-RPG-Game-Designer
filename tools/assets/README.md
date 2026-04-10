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
