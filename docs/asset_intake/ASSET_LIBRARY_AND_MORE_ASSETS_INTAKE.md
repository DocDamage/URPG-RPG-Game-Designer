# Asset Library And More Assets Intake

Status Date: 2026-05-01

This document records the current asset-library vertical slice and the local `more assets/` intake that was unpacked, cleaned, indexed, and kept as non-release raw quarantine.

## Current Scope

The asset-library slice is a conservative intake and inspection lane. It does not promote any template or subsystem to `READY`, and it does not delete duplicate production assets automatically.

Landed code:

- `engine/core/assets/asset_library.*`
- `engine/core/assets/asset_cleanup_planner.*`
- `engine/core/assets/asset_provenance.h`
- `engine/core/assets/asset_import_session.*`
- `engine/core/assets/global_asset_library_store.*`
- `engine/core/assets/global_asset_promotion_service.*`
- `engine/core/assets/project_asset_attachment_service.*`
- `editor/assets/asset_library_model.*`
- `editor/assets/asset_library_panel.*`
- `tools/ci/check_asset_library_governance.ps1`
- `tools/assets/global_asset_import.py`
- `tools/assets/ingest_more_assets.ps1`
- `tools/assets/plan_more_assets_dedupe.py`

## Global Asset Library Import Phase 1

Status: complete for the managed folder/file/ZIP import foundation as of 2026-05-01.

The first-pass global import lane now supports:

- managed global library roots under `.urpg/asset-library/`
- loose file, folder, and ZIP quarantine import
- bounded ZIP extraction with path traversal, absolute path, malformed archive, file-count, and byte-count diagnostics
- source manifests under `.urpg/asset-library/sources/<session>/source_manifest.json`
- catalog mirror manifests under `.urpg/asset-library/catalog/import_sessions/<session>.json`
- import session serialization, review rows, and summary counts
- explicit review-row preview metadata, including stable no-preview diagnostics for source/tool/unsupported records
- image/audio/source/tool classification for first-pass asset review
- PNG, GIF, BMP, and JPEG dimensions where headers expose them without external dependencies
- WAV duration metadata
- duplicate detection by SHA-256 within the import session
- unsupported extractor diagnostics for RAR/7z when no external extractor command is configured
- `GlobalAssetLibraryStore` as the facade over the Phase 1 layout and existing `.urpg/asset-index/asset_catalog.db` path
- Asset Library model exposure for import sessions and review queues

Phase 1 intentionally did not claim native file-dialog UX, conversion workflows, optional RAR/7z extraction,
aggregate animation sequence assembly, or richer RPG Maker folder convention mapping. Later phases now cover the
non-dialog tooling surfaces.

## Global Asset Library Promotion Phase 2

Status: 100% complete for review-gated promotion as of 2026-05-01.

The promotion lane now supports:

- deterministic promotion planning from import-session records
- governed `AssetPromotionManifest` output and ingestion
- license/attribution-note gating for runtime/export promotion
- blocked promotion diagnostics for conversion-needed, duplicate, unsupported, source-only, missing-license, and
  missing-normalized-path records
- single-record promotion from editor model entrypoints
- selected-record batch promotion with per-record result rows and aggregate promoted/blocked/missing counts
- selected-record global promotion that copies eligible quarantined payloads into `.urpg/asset-library/promoted`
  and writes governed per-asset promotion manifests
- Asset Library action rows that expose promoted payload, diagnostics, and next actions

This phase still does not execute conversions. Records that need conversion remain blocked with diagnostics and Phase 4
conversion handoff metadata until a later converter runner produces a supported runtime payload.

## Global Asset Library Project Attachment Phase 3

Status: 100% complete as of 2026-05-01.

The project attachment lane now supports:

- runtime-ready promoted asset validation before project attachment
- copying selected promoted payloads into `content/assets/imported/<asset-id>/`
- writing project-local manifests under `content/assets/manifests/`
- single-asset and selected-asset attachment model entrypoints with per-record diagnostics
- reloading project-local attachment manifests into the Asset Library model
- project-local picker rows for Level Builder, sprite selectors, audio selectors, and UI/theme selectors
- attachable and project-attached quick filters and counts
- export/package discovery that includes attached project assets and excludes `.urpg/asset-library` quarantine/promoted
  roots
- Level Builder prop placement consumption of level-builder-targeted attached project assets

Level Builder prop placement now consumes the Asset Library model's project-local picker rows. Audio and UI/theme picker
surfaces can use the same row contract when their dedicated selector controls are expanded.

## Global Asset Library Advanced Packs Phase 4

Status: 100% complete as of 2026-05-01.

The advanced-pack lane now supports:

- RPG Maker `img/characters`, `img/tilesets`, `img/faces`, `img/parallaxes`, `img/pictures`, `img/animations`, and
  `img/system` folders map into URPG `sprite`, `tileset`, `portrait`, `background`, `vfx`, and `ui` categories.
- RPG Maker `audio/bgm`, `audio/bgs`, `audio/me`, and `audio/se` folders map into URPG audio categories.
- conversion-needed audio records carry deterministic `conversionRequired`, `conversionTargetPath`, and
  `conversionCommand` handoff metadata while remaining blocked from runtime promotion until conversion succeeds.
- optional local external extractor commands can ingest `.rar` and `.7z` sources into managed quarantine without shell
  execution, with bounded output validation and stable failure diagnostics.
- numbered animation/image-frame drops assemble into deterministic `sequenceGroups`, with per-frame sequence metadata
  on import records.

## Global Asset Library Wizard Workflow Phase 5

Status: 100% complete as of 2026-05-01.

The model-level wizard contract now supports:

- ordered Add Source, Review, Promote, Attach, and Package workflow steps under `import_wizard`
- current-step and overall status values for empty, review-required, ready-to-attach, and package-ready states
- action IDs, enablement flags, eligible counts, and disabled reasons for add-source, promote-selected,
  attach-selected, and package-validation affordances
- deterministic Add Source handoff requests with `tools/assets/global_asset_import.py` command arguments and expected
  import-session manifest paths
- package-validation readiness once promoted assets have been attached into project-local asset roots

Native file-dialog wiring remains outside this slice; the model contract is stable for an editor UI to render the
Project Import Wizard, collect a chosen source path, and call the existing import/promote/attach entrypoints without
making editor/runtime code execute Python directly.

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

The raw source and extracted intake paths are intentionally ignored local quarantine, not repository payload:

- `more assets/**`
- `imports/raw/more_assets/**`
- `imports/raw/itch_assets/loose/**`

Reports and curated promotion records remain eligible for normal Git tracking under `imports/reports/`, `imports/manifests/`, and `imports/normalized/`. Promote only selected, governed assets into those paths; do not re-add the full raw extraction trees or source archive drops.

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
