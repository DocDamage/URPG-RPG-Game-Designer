# Asset Library And More Assets Intake

Status Date: 2026-05-02

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

Phase 1 intentionally stayed limited to the managed folder/file/ZIP foundation. Later phases now cover native picker
wiring, conversion execution, optional RAR/7z extraction through configured extractor commands, aggregate animation
sequence assembly, and richer RPG Maker folder convention mapping.

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

Phase 2's promotion gate still blocks conversion-needed records until Phase 4's conversion runner produces a supported
runtime payload and clears the conversion diagnostics.

## Global Asset Library Project Attachment Phase 3

Status: 100% complete as of 2026-05-01.

The project attachment lane now supports:

- runtime-ready promoted asset validation before project attachment
- copying selected promoted payloads into `content/assets/imported/<asset-id>/`
- writing project-local manifests under `content/assets/manifests/`
- single-asset and selected-asset attachment model entrypoints with per-record diagnostics
- reloading project-local attachment manifests into the Asset Library model
- project-local picker rows consumed by Level Builder, sprite preview, audio inspector, and UI/theme selector surfaces
- attachable and project-attached quick filters and counts
- export/package discovery that includes attached project assets and excludes `.urpg/asset-library` quarantine/promoted
  roots
- Level Builder prop placement consumption of level-builder-targeted attached project assets

Level Builder prop placement, sprite preview, audio inspector, and UI/theme builder surfaces now consume the Asset
Library model's project-local picker rows through selector-specific option filters.

## Global Asset Library Advanced Packs Phase 4

Status: 100% complete as of 2026-05-01.

The advanced-pack lane now supports:

- RPG Maker `img/characters`, `img/tilesets`, `img/faces`, `img/parallaxes`, `img/pictures`, `img/animations`, and
  `img/system` folders map into URPG `sprite`, `tileset`, `portrait`, `background`, `vfx`, and `ui` categories.
- RPG Maker `audio/bgm`, `audio/bgs`, `audio/me`, and `audio/se` folders map into URPG audio categories.
- conversion-needed audio records carry deterministic `conversionRequired`, `conversionTargetPath`, and
  `conversionCommand` handoff metadata.
- the Asset Library model can run the stored conversion handoff from the managed source root, verify the expected
  output, update the import record to the converted runtime-ready payload, clear conversion diagnostics, and make the
  record promotable. Conversion execution now uses an explicit argv process runner and working directory instead of
  mutating the editor process's current directory.
- the Project Import Wizard exposes Convert Selected as an enabled action for conversion-needed records, and the Asset
  Library panel dispatches selected conversions through the model runner.
- optional local external extractor commands can ingest `.rar` and `.7z` sources into managed quarantine without shell
  execution, with bounded output validation and stable failure diagnostics.
- fatal archive/import failures are fail-closed: unsafe ZIP paths and failed external extractor runs do not surface
  partial copied or extracted payloads as review records.
- Add Source handoff metadata can carry a configured external extractor command into `global_asset_import.py` for
  `.rar` and `.7z` sources. Extractor handoff commands can either accept the source and destination as appended final
  arguments or use explicit `{source}` and `{destination}` placeholders, including embedded placeholders such as
  `-o{destination}`, for tools that require templated argument positions. The CLI also reads
  `URPG_ASSET_ARCHIVE_EXTRACTOR` when `--external-extractor-command` is not supplied, and the C++ Add Source request
  builder uses the same variable as the default external extractor command when the editor request does not supply one.
- the Project Import Wizard snapshot exposes external extractor configuration status before Add Source is requested,
  including the configuration source, environment variable name, parsed argv vector, and whether RAR/7z handoff support
  is currently configured. Malformed extractor configuration is surfaced through
  `external_extractor_command_parse_error` and is not treated as configured support.
- the editor model keeps the Add Source importer command prefix configurable so packaged builds can resolve the Python
  runtime and importer script without depending on the process working directory.
- native source picker availability is explicit in panel diagnostics; Windows reports `native_import_source_picker_available`,
  while platforms without a native picker report `native_import_source_picker_unsupported`.
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
- typed Asset Library panel render snapshots for wizard steps, actions, disabled reasons, and package-validation
  readiness
- Asset Library panel action dispatchers for Add Source, Promote Selected, Attach Selected, and Package Validate that
  call the existing model or export-packager entrypoints and keep the wizard render snapshot current
- Add Source picker wiring through an injectable panel picker, with a Windows native `IFileOpenDialog` implementation
  for file/archive or folder source selection and deterministic test injection for headless runs

The picker supplies the chosen source path to the existing Add Source dispatcher without making editor/runtime code
execute Python directly.

## External Extractor Configuration

`URPG_ASSET_ARCHIVE_EXTRACTOR` is the current deterministic local configuration hook for RAR/7z import. It is parsed
into an argv vector, not executed through a shell, and supports `{source}` and `{destination}` placeholders. A future
editor/user settings surface should persist the same argv vector shape so the wizard can populate the same
`extractor_configuration` snapshot without relying on shell command strings.

## More Assets Intake

Source folder:

- `more assets/`
- `more assets to ingest/`

Raw extraction folder:

- `imports/raw/more_assets/`
- `imports/raw/more_assets_to_ingest/`
- `imports/raw/assets_for_my_game/`
- `imports/raw/godogenui_assets/`

Generated reports:

- `imports/reports/more_assets/more_assets_intake_manifest.json`
- `imports/reports/more_assets/more_assets_duplicate_cleanup_preview.csv`
- `imports/reports/more_assets/more_assets_duplicate_cleanup_preview_summary.json`
- `imports/reports/more_assets/more_assets_safe_dedupe_plan.csv`
- `imports/reports/more_assets/more_assets_safe_dedupe_applied.csv`
- `imports/reports/more_assets/more_assets_safe_dedupe_summary.json`
- `imports/reports/more_assets_to_ingest/more_assets_intake_manifest.json`
- `imports/reports/more_assets_to_ingest/source_inventory_with_rar_and_loose.json`
- `imports/reports/more_assets_to_ingest/rar_extraction_report.json`
- `imports/reports/more_assets_to_ingest/hygiene/asset_hygiene_summary.json`
- `imports/reports/more_assets_to_ingest/more_assets_to_ingest_catalog.json`
- `imports/reports/more_assets_to_ingest/more_assets_to_ingest_catalog/*.json`
- `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog.json`
- `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog/*.json`
- `imports/reports/asset_intake/src007_rar_extraction_report.json`
- `imports/reports/asset_intake/local_computer_asset_candidate_inventory.json`
- `imports/reports/asset_intake/local_computer_asset_candidate_inventory_refined.json`
- `imports/reports/asset_intake/src011_archive_extraction_report.json`
- `imports/reports/asset_intake/assets_for_my_game_promotion_catalog.json`
- `imports/reports/asset_intake/assets_for_my_game_promotion_catalog/*.json`
- `imports/reports/asset_intake/godogenui_assets_promotion_catalog.json`
- `imports/reports/asset_intake/godogenui_assets_promotion_catalog/*.json`

Intake results:

- 39 zip archives were inventoried.
- 39 extracted archive roots are present under `imports/raw/more_assets/`.
- 3 installers were cataloged and intentionally not executed.
- Extracted payload after junk cleanup: 109,622 files, approximately 6.53 GB.
- Raw source folder payload: 84 files, approximately 7.02 GB.
- The newer `more assets to ingest/` drop contains 407 archive files: 319 ZIP and 88 RAR.
- ZIP and RAR payloads were extracted into `imports/raw/more_assets_to_ingest/` with zero extraction failures.
- The newer drop catalog records 20,229 quarantine assets from 407 archive pack roots.
- The newer drop includes 4,495 inferred tilesets, 2,209 sprite/character records, 1,222 UI records, 626 VFX records, 479 portraits, 553 3D model records, 344 map/tileset-data records, 273 fonts, 55 audio records, and 148 license/readme records.
- The raw catalog remains quarantine-only, but `BND-004` now promotes 14 selected CC0/public-domain starter assets into `imports/normalized/src010_cc0_starter_pack/` with checksums and attribution metadata.
- The full raw catalog is also converted into an editor-ingestible local promotion catalog with 20,229 records. These records are local-use-only, previewable/searchable, and export-ineligible.
- The promotion candidate review ranked 135 useful packs and produced a curated SRC-010 plan under `imports/reports/more_assets_to_ingest/src010_curated_promotion_plan.md`.
- The remaining recommended review cohort is restricted to packs with CC0/public-domain evidence; attribution/redistribution-restricted packs remain review holds.
- Three old-intake SRC-007 RAR archives that were missed before RAR support was configured were extracted into `imports/raw/urpg_stuff/refresh_20260428_2206/__rar_extracted/` with zero failures and 21,413 files present.
- The SRC-007 local promotion catalog was regenerated with SRC-008 excluded, producing 77,509 local-use records, 56,096 canonical exact-hash records, and 21,413 exact-duplicate recovered RAR records.
- A computer-wide read-only scan identified two high-signal additional local asset drops: `C:\Users\Doc\Downloads\AssetsForMyGame` and `C:\Users\Doc\Desktop\Projects\GoDoGenUI\godogenwithui-master\assets`.
- `SRC-011` mirrors `AssetsForMyGame` into ignored raw quarantine, extracts 58 ZIP/RAR archives with zero failures and 13,313 extracted files present, and catalog-normalizes 12,985 supported local-use records.
- `SRC-012` mirrors the GoDoGenUI asset folder into ignored raw quarantine, intentionally skips duplicate ProgramData/All Users mirrors, and catalog-normalizes 29,516 supported local-use records from 46,285 mirrored files.
- `BND-005` promotes 6 selected `SRC-012` CC0/public-domain tiles/VFX assets into `imports/normalized/src012_cc0_tiles_vfx/` with checksums and attribution metadata. The rest of `SRC-011` and `SRC-012` remains quarantine-only.

The raw source and extracted intake paths are intentionally ignored local quarantine, not repository payload:

- `more assets/**`
- `more assets to ingest/**`
- `imports/raw/more_assets/**`
- `imports/raw/more_assets_to_ingest/**`
- `imports/raw/urpg_stuff/**`
- `imports/raw/src014_20260504_extracted/**`
- `imports/raw/assets_for_my_game/**`
- `imports/raw/godogenui_assets/**`
- `imports/raw/itch_assets/loose/**`
- `ingest stuff 5-4-26/**`

Reports and curated promotion records remain eligible for normal Git tracking under `imports/reports/`, `imports/manifests/`, and `imports/normalized/`. Promote only selected, governed assets into those paths; do not re-add the full raw extraction trees or source archive drops.

## SRC-010 Promotion Plan

`imports/reports/more_assets_to_ingest/src010_curated_promotion_plan.json` and `.md` are review guidance. `BND-004` is the first governed result from that plan, promoting a small CC0 starter subset while leaving the full raw drop ignored and quarantined.

Before any additional SRC-010 bundle can be marked ready:

- Copy only the curated subset into `imports/normalized/src010_<bundle-scope>/`.
- Create a governed `BND-*` bundle manifest with exact file checksums.
- Create per-pack or per-bundle attribution records under `imports/reports/asset_intake/attribution/`.
- Update the source registry and rerun the asset governance gate.

The first promoted lane is CC0/public-domain evidence only: Lucifer UI, Screaming Brain isometric tiles, Foozle Void space/action assets, and a GGBotNet CC0 font. Packs with credit, redistribution, NFT/metaverse/AI, commercial-use-only, or unclear terms stay quarantined until those terms are explicitly reviewed.

## SRC-011 / SRC-012 Promotion Plan

`BND-005` is the first governed promotion from the computer-wide scan follow-up, promoting a small Ansimuz/GoDoGenUI CC0/public-domain subset while leaving both new raw drops ignored and quarantined.

Before any additional `SRC-011` or `SRC-012` bundle can be marked ready:

- Copy only the curated subset into `imports/normalized/src011_<bundle-scope>/` or `imports/normalized/src012_<bundle-scope>/`.
- Create a governed `BND-*` bundle manifest with exact file checksums.
- Create per-pack or per-bundle attribution records under `imports/reports/asset_intake/attribution/`.
- Keep Patreon/supporter, no-redistribution, CraftPix web-license-only, ROM-derived, and unclear-license packs quarantined until their terms are explicitly reviewed.
- Update the source registry and rerun the asset governance gate.

## SRC-014 ModernUI And Archive Intake

`SRC-014` catalogs the local `ingest stuff 5-4-26/` drop for editor/library browsing. `BND-009` catalogs the ModernUI static style/gamepad sheets at 16x16, 32x32, and 48x48 plus the trash-button animation GIFs under `imports/normalized/src014_modern_ui/`. These assets are unsliced source sheets, not runtime-ready app UI: they remain `distribution: "deferred"` until atlas slice, nine-patch, and control-state metadata exists.

The remaining loose ModernUI portrait-generator files are promoted through `BND-010`: 953 PNG/Aseprite portrait part and source-art assets under `imports/normalized/src014_modernui_portrait_generator/`. The bundle is release-eligible but uses `distribution: "deferred"` so default exports stay bounded until a project explicitly selects the portrait assets.

The CuteSCKR and Human RPG Portrait Pack archives are fully extracted into ignored raw quarantine:

- `imports/reports/asset_intake/src014_archive_extraction_report.json`
- `imports/raw/src014_20260504_extracted/__archive_extracted/`

The extracted archive catalog records 1,397 supported image assets with zero unsupported files:

- `imports/reports/asset_intake/ingest_20260504_extracted_promotion_catalog.json`
- `imports/reports/asset_intake/ingest_20260504_extracted_promotion_catalog/*.json`
- `imports/reports/asset_intake/ingest_20260504_extracted_promotion_summary.json`

Those extracted records remain local-use-only catalog entries until a future curated bundle selects specific CuteSCKR/Human assets for normalized promotion.

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

`tools/assets/asset_db.py` now includes `imports/raw/more_assets` and `imports/raw/more_assets_to_ingest` in its default roots and infers:

- `category = more-assets-raw`
- `pack = <extracted archive root folder>`

The editor/library catalog can load `imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog.json` automatically through the existing `*_promotion_catalog.json` scan. Those records carry stable virtual ids such as `asset://src-010/...`, point previews back at ignored raw files, set `local_use_allowed=true`, and keep `release_use_allowed=false`.

The local SQLite catalog can also search the new intake by pack and media type, for example:

```powershell
python .\tools\assets\asset_db.py find --category more-assets-raw --pack modernexteriors --limit 10
python .\tools\assets\asset_db.py find --category more-assets-to-ingest-raw --query "lucifer" --limit 10
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
