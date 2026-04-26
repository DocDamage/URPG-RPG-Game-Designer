# Asset Library And More Assets Intake

Status Date: 2026-04-25

This document records the current asset-library vertical slice and the local `more assets/` intake that was unpacked, cleaned, indexed, and prepared for repository tracking.

## Current Scope

The asset-library slice is a conservative intake and inspection lane. It does not promote any template or subsystem to `READY`, and it does not delete duplicate production assets automatically.

Landed code:

- `engine/core/assets/asset_library.*`
- `engine/core/assets/asset_cleanup_planner.*`
- `engine/core/assets/asset_provenance.h`
- `editor/assets/asset_library_model.*`
- `editor/assets/asset_library_panel.*`
- `tools/ci/check_asset_library_governance.ps1`
- `tools/assets/ingest_more_assets.ps1`
- `tools/assets/plan_more_assets_dedupe.py`

## More Assets Intake

Source folder:

- `more assets/`

Raw extraction folder:

- `imports/raw/more_assets/`

Generated reports:

- `imports/reports/more_assets/more_assets_intake_manifest.json`
- `imports/reports/more_assets/more_assets_duplicate_cleanup_preview.csv`
- `imports/reports/more_assets/more_assets_duplicate_cleanup_preview_summary.json`
- `imports/reports/more_assets/more_assets_safe_dedupe_plan.csv`
- `imports/reports/more_assets/more_assets_safe_dedupe_applied.csv`
- `imports/reports/more_assets/more_assets_safe_dedupe_summary.json`

Intake results:

- 39 zip archives were inventoried.
- 39 extracted archive roots are present under `imports/raw/more_assets/`.
- 3 installers were cataloged and intentionally not executed.
- Extracted payload after junk cleanup: 109,622 files, approximately 6.53 GB.
- Raw source folder payload: 84 files, approximately 7.02 GB.

The raw source and extracted intake paths are tracked through Git LFS using path-specific `.gitattributes` rules:

- `more assets/**`
- `imports/raw/more_assets/**`

## Hygiene Results

After extraction, `tools/assets/asset_hygiene.py --write-reports --prune-junk` removed only OS/archive junk:

- `.DS_Store`
- `Thumbs.db`
- `desktop.ini`
- `__MACOSX`

The follow-up non-pruning scan reported:

- `file_count`: 159,443
- `junk_file_count`: 0
- `junk_dir_count`: 0
- `oversize_count`: 4
- `duplicate_groups`: 43,300
- `duplicate_file_count`: 95,117
- `duplicate_waste_bytes`: 3,428,021,085

## Duplicate Policy

The new raw-intake duplicate planner is deliberately conservative:

- It only plans exact duplicates inside the same extracted archive root.
- It preserves cross-archive duplicates to avoid collapsing provenance or license boundaries.
- It preserves license/readme/credits style files.
- It defaults to report-only mode.

The current report-only plan found:

- 28,456 same-archive duplicate candidates.
- 44,131,398 bytes of possible savings.
- 0 files deleted.

Because the savings are small compared with the value of preserving raw archive fidelity, duplicate deletion remains unapplied.

## Asset Catalog

`tools/assets/asset_db.py` now includes `imports/raw/more_assets` in its default roots and infers:

- `category = more-assets-raw`
- `pack = <extracted archive root folder>`

The catalog can now search the new intake by pack and media type, for example:

```powershell
python .\tools\assets\asset_db.py find --category more-assets-raw --pack modernexteriors --limit 10
python .\tools\assets\asset_db.py find --kind audio --category more-assets-raw --limit 10
```

## Validation

Focused validation commands used for this slice:

```powershell
cmake --build --preset dev-debug --target urpg_tests
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]" --reporter compact
powershell -ExecutionPolicy Bypass -File .\tools\ci\check_asset_library_governance.ps1
python .\tools\assets\asset_hygiene.py --write-reports
python .\tools\assets\plan_more_assets_dedupe.py
python .\tools\assets\asset_db.py index
python .\tools\assets\asset_db.py stats
```
