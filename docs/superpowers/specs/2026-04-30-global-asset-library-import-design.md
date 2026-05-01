# Global Asset Library Import Design

Status: approved design direction; Phases 1-5 implemented
Date: 2026-04-30

## Goal

Add an easy external asset ingestion system where a creator can point URPG Maker at a file, folder, or archive, have URPG copy the contents into a managed global library, classify and prepare the usable assets, then attach selected assets to one or more projects.

The first implementation should make user-owned asset packs practical without bundling third-party game assets in the URPG release. It should be safer than blind auto-import, but much easier than manual folder wrangling.

## Product Shape

The feature is a Global Asset Library plus Project Import Wizard.

Creators build a reusable library once, then attach approved assets to projects as needed. URPG owns the managed copy, metadata, dedupe state, thumbnails, diagnostics, and promotion records. Projects should not depend on the user's original download folder staying in place.

The wizard should use smart defaults, but promotion remains review-gated. URPG can preselect likely-safe assets and suggest conversions, but the user explicitly approves what becomes project/runtime/export-ready.

## Non-Goals

- Do not auto-promote every discovered file.
- Do not execute imported tools, scripts, installers, or generators.
- Do not treat raw archive contents as release-ready.
- Do not require bundled third-party game assets for URPG itself to ship.
- Do not solve every asset format in the first pass.
- Do not make legal claims beyond user-provided license/attribution metadata.

## Supported First-Pass Inputs

Phase 1 should support:

- folders
- loose files
- `.zip` archives
- common image files used for sprites, UI, textures, tiles, and previews
- WAV audio for runtime-ready playback
- broader audio files as cataloged or conversion-needed records, when metadata tooling can inspect them

Phase 1 may catalog but should not fully promote:

- `.rar` and `.7z` archives unless a local extractor is configured
- source art files such as `.psd`, `.kra`, `.aseprite`, `.blend`
- executables, installers, scripts, project generators, plugins, and tools
- very large animation-frame drops that need aggregate sequence handling before promotion

RAR and 7z are handled as pluggable extractor support. If a local extractor command is configured, URPG uses it through
a bounded extraction adapter. If not configured, the wizard and importer show clear unsupported-extractor diagnostics.

## User Flow

1. Add Source

   The user chooses a folder, loose file, or archive. URPG creates a new library import session with source name, timestamp, original path, selected import mode, and destination managed-library root.

2. Quarantine Copy Or Extract

   URPG copies loose/folder inputs or extracts archives into a managed global library quarantine area. The original source remains untouched. Extraction must reject path traversal, absolute paths, oversized outputs, and executable auto-run behavior.

3. Scan And Classify

   URPG walks the quarantined copy and records file metadata:

   - relative path
   - normalized path candidate
   - extension and media kind
   - size
   - hash
   - dimensions for images
   - duration/waveform metadata where available
   - likely category
   - pack/group hints
   - duplicate state
   - unsupported/junk/source-only state

4. Normalize

   URPG creates stable asset IDs and clean internal paths without immediately making assets runtime-ready. The normalization stage should be deterministic and preserve source provenance.

5. Review

   The Asset Library UI shows grouped review rows:

   - Ready to promote
   - Needs conversion
   - Duplicate
   - Missing license note
   - Unsupported
   - Source/tooling only
   - Error

   Each row exposes recommended action, preview state, reason codes, and whether it can be promoted into the active project.

6. Promote To Global Library

   The user promotes selected records into the global curated library. Promotion requires at least a source record, hash, managed path, category, and user-provided license/attribution note. Runtime-ready promotion requires a supported runtime payload or a successful conversion.

7. Attach To Project

   The user attaches selected global assets to the active project. URPG copies project-ready payloads into the project asset area and writes project-local promotion manifests or bundle references. Attached assets become visible to relevant pickers such as Level Builder, sprite selectors, audio selectors, UI/theme selectors, and export validation.

8. Package And Validate

   Project export and release validation consume only attached, promoted, package-eligible assets. Raw quarantine files and unapproved library records are not packaged.

## Architecture

### Global Library Roots

Use a managed root under `.urpg/asset-library/` for local development and user installs by default. The exact user-install location can be platform-specific later, but the model should not assume the repo root is the permanent user data path.

Suggested first-pass structure:

```text
.urpg/asset-library/
  sources/
    <source-id>/
      original/
      extracted/
      source_manifest.json
  catalog/
    asset_catalog.db
    import_sessions/
      <session-id>.json
  promoted/
    <asset-id-or-pack-id>/
      payloads/
      previews/
      asset_promotion_manifest.json
```

Reuse `.urpg/asset-index/asset_catalog.db` through a new `GlobalAssetLibraryStore` facade for Phase 1. Do not move the database format in the first pass. The facade becomes the stable boundary so a later migration to `.urpg/asset-library/catalog/asset_catalog.db` does not affect editor or project-attachment callers.

### Core Components

`AssetImportSession`
: Tracks one import run: source path, managed root, extractor used, counts, diagnostics, and generated catalog paths.

`AssetSourceIngestor`
: Copies folders/files and dispatches archive extraction. It owns quarantine safety, path normalization, and source manifest creation.

`ArchiveExtractor`
: Interface for archive support. ZIP is built in; RAR/7z import is available through configured external extractor
  commands with bounded output validation and stable diagnostics.

`AssetScanner`
: Walks quarantined files, records metadata, hashes, dimensions, duration, and basic media type.

`AssetClassifier`
: Assigns category, media kind, source-only/tooling-only flags, and likely promotion lane from paths, extensions, dimensions, and folder names.

`AssetNormalizer`
: Creates stable normalized IDs and proposed managed library paths.

`AssetPromotionPlanner`
: Builds review rows and recommended actions. It does not mutate project content.

`GlobalAssetLibraryStore`
: Persists global catalog records, import sessions, dedupe groups, previews, provenance, and promoted library records.

`ProjectAssetAttachmentService`
: Copies or references promoted global assets into the active project and writes project-local manifests.

### UI Surfaces

Add or extend the existing Asset Library panel with:

- Add Source button
- Import Sessions list
- Review queue grouped by readiness state
- filters for media kind, category, status, duplicate state, and project-attached state
- previews for images and audio metadata
- Promote selected
- Attach selected to current project
- diagnostics panel with actionable reason codes

The first version exposes visible Asset Library panel workflow state and accepts source paths through model/CLI/test
entrypoints. The panel now also wires Add Source through an injectable picker with a Windows native file/folder dialog
implementation, keeping the feature demonstrable in headless tests while allowing native source selection in editor
builds.

## Data Model

The implementation should reuse existing concepts where possible:

- `AssetLibrary`
- `AssetRecord`
- `AssetPromotionManifest`
- asset action rows
- promotion catalogs
- source and bundle manifests

New data should fill the gap between raw external input and promotion:

```json
{
  "schemaVersion": "1.0.0",
  "sessionId": "import_20260430_001",
  "sourceKind": "zip",
  "sourcePath": "C:/Users/Creator/Downloads/fantasy_pack.zip",
  "managedSourceRoot": ".urpg/asset-library/sources/import_20260430_001/extracted",
  "status": "review_ready",
  "createdAt": "2026-04-30T00:00:00Z",
  "summary": {
    "filesScanned": 1234,
    "readyCount": 412,
    "needsConversionCount": 18,
    "duplicateCount": 104,
    "unsupportedCount": 23,
    "sourceOnlyCount": 6,
    "errorCount": 0
  },
  "diagnostics": []
}
```

Project attachment creates project-owned records that remain valid even if the global library is later moved. Phase 1 copies payloads into the active project instead of storing global-library references.

## Format Policy

Images:

- promote common raster images that the runtime renderer can load
- preserve source dimensions and preview metadata
- classify sprites, tiles, UI, backgrounds, portraits, and VFX by path and dimensions where possible

Audio:

- promote WAV as runtime-ready
- catalog OGG/MP3/FLAC/M4A as conversion-needed unless runtime decode support is added and verified
- execute stored conversion handoffs through the Asset Library model before runtime/export promotion

Archives:

- ZIP first
- RAR/7z through optional external extractor configuration and detection
- nested archives should be cataloged but not recursively extracted by default

External extractor configuration:

- `URPG_ASSET_ARCHIVE_EXTRACTOR` is the current deterministic local configuration hook for the command-line importer
  and editor Add Source request builder.
- Store extractor commands as an argv vector shape. Shell command strings are only parsed at the current environment
  variable boundary for local configuration compatibility.
- A future editor/user settings surface should persist that argv vector directly and feed the same wizard
  `extractor_configuration` snapshot, including source, command, and RAR/7z support status.

Source/tool files:

- catalog as source-only or tooling-only
- never package unless a later explicit workflow supports that class

## Safety And Diagnostics

The system must be fail-closed:

- Block path traversal during extraction.
- Reject absolute archive paths.
- Enforce max file count and max extracted bytes per import session.
- Do not execute imported files.
- Detect duplicates by hash.
- Preserve provenance to the original source session.
- Require user license/attribution notes before runtime/export promotion.
- Require supported runtime format before project attachment.
- Surface missing files, unsupported formats, conversion-needed state, and duplicate state as explicit reason codes.

Every failed or skipped asset should have a stable diagnostic code. This keeps the editor UI, tests, and future chatbot guidance aligned.

## Integration Points

Existing code and tools that should be reused or wrapped:

- `tools/assets/asset_db.py` for SQLite cataloging and duplicate/hash metadata
- `tools/assets/asset_hygiene.py` for duplicate, oversize, and junk concepts
- `tools/assets/ingest_more_assets.ps1` for archive intake precedent
- `tools/assets/promote_urpg_stuff_assets.py` for catalog-normalized records
- `tools/assets/catalog_animation_asset_drop.py` as prior art for aggregate animation-frame handling
- `tools/assets/convert_audio_to_ogg.py` as prior art for audio conversion handoff flows
- `engine/core/assets/asset_library.*`
- `engine/core/assets/asset_promotion_manifest.*`
- `editor/assets/asset_library_model.*`
- `editor/assets/asset_library_panel.*`
- `tools/ci/check_release_required_assets.ps1`

The design should not make engine/runtime code depend on Python tooling. Tooling can generate reports and manifests; engine/editor C++ should consume structured manifests and catalog records.

## Testing Strategy

Unit tests:

- import session serialization
- classifier categories
- duplicate planning
- promotion planner reason codes
- project attachment manifest output
- archive extraction safety paths

Integration tests:

- folder import with images and duplicates
- ZIP import with nested folders
- unsupported RAR without extractor produces clear diagnostic
- WAV asset can be promoted and attached
- OGG asset is cataloged as conversion-needed unless runtime support lands
- missing license note blocks runtime/export promotion

Editor/model tests:

- Asset Library panel exposes import session state
- review rows group assets by status
- promote selected updates global library records
- attach selected updates project asset records
- empty/error states are explicit

Gate tests:

- raw quarantine files are not packaged
- only attached project-ready assets enter package manifests
- existing release-required asset checks continue to pass when no user assets are bundled

## Phased Delivery

### Phase 1: Managed Folder/ZIP Import Foundation

- Add import session model and schema.
- Add managed global library root.
- Implement folder/file copy and ZIP extraction with safety checks.
- Produce catalog records for images/audio/source-only files.
- Add duplicate and unsupported diagnostics.
- Expose session/review rows in Asset Library model tests.

Implementation status: complete as of the 2026-05-01 development slice. Phase 1 is covered by
`AssetImportSession`, `GlobalAssetLibraryStore`, `tools/assets/global_asset_import.py`, editor model
session/review rows with explicit preview/no-preview metadata, and targeted CTest/Python coverage. Later slices added
the panel-native source picker, promotion, project attachment, conversion execution, optional external archive
extraction, sequence assembly, and RPG Maker pack mapping.

### Phase 2: Review-Gated Promotion

- Add promotion planner.
- Reuse `AssetPromotionManifest` for promoted global records.
- Require license/attribution note for runtime/export promotion.
- Add promote selected behavior and diagnostics.
- Add editor snapshot coverage for review states.

Implementation status: 100% complete as of the 2026-05-01 development slice. Phase 2 is covered by
`planAssetPromotionManifest`, governed `AssetPromotionManifest` ingestion, single-record and selected-record
promotion model APIs, materialized selected promotion into `.urpg/asset-library/promoted`, per-record diagnostics,
license-gated runtime promotion, and editor snapshot tests.

### Phase 3: Project Attachment

- Add project attachment service.
- Copy selected promoted assets into project-local asset roots.
- Write project-local manifests.
- Make Level Builder and relevant pickers consume attached assets.
- Ensure package/export validation sees only attached, package-eligible records.

Implementation status: 100% complete as of the 2026-05-01 development slice.
`ProjectAssetAttachmentService` copies runtime-ready promoted assets into `content/assets/imported`, writes
project-local manifests under `content/assets/manifests`, exposes single-asset and selected-asset model APIs, reloads
project attachment manifests, exposes project-local picker rows for Level Builder and selector surfaces, and keeps
export/package discovery scoped to attached project assets while excluding raw global-library quarantine. The Level
Builder prop placement, sprite preview, audio inspector, and UI/theme builder selector surfaces consume attached project
picker rows for their targeted asset kinds.

### Phase 4: Conversions And Advanced Packs

- Add audio conversion workflow.
- Add optional 7-Zip-backed `.rar`/`.7z` extraction.
- Add aggregate animation sequence assembly.
- Add RPG Maker folder convention mapping.
- Add richer tileset/sprite/portrait/UI classification.

Implementation status: 100% complete as of the 2026-05-01 development slice. Conversion-needed audio records now carry
deterministic conversion handoff metadata, and the Asset Library model can execute the stored conversion command from
the managed source root, verify the converted output, clear conversion diagnostics, and make the converted record
promotable. The Project Import Wizard exposes Convert Selected for conversion-needed records, with panel dispatch into
the model runner. Optional local external extractor commands can ingest `.rar` and `.7z` sources into bounded quarantine
with clear diagnostics, and Add Source handoff metadata can pass configured extractor commands through to
`global_asset_import.py`. Extractor commands support either appended source/destination arguments or explicit
`{source}` and `{destination}` placeholders, including embedded placeholder forms such as `-o{destination}`, for native
tool argument templates. The command-line importer also reads `URPG_ASSET_ARCHIVE_EXTRACTOR` when no explicit extractor
command is provided, and the editor Add Source request builder uses the same environment variable as its default
external extractor handoff. The wizard snapshot exposes the current extractor configuration source, environment
variable, parsed argv vector, and whether RAR/7z support is configured before Add Source is requested. Numbered
animation/image-frame drops assemble into deterministic sequence groups, and RPG Maker `img/*` plus `audio/*` folder
conventions map into URPG review categories during managed import.

### Phase 5: Wizard Workflow Contract

- Expose explicit Add Source, Review, Promote, Attach, and Package workflow state.
- Surface action enablement and disabled reasons from the Asset Library model.
- Emit deterministic Add Source command handoff payloads for the external import tool.
- Keep the wizard contract testable while allowing native source selection through the panel picker.
- Provide package-validation readiness once project assets are attached.

Implementation status: 100% complete as of the 2026-05-01 development slice. The Asset Library model now emits an
`import_wizard` snapshot with ordered workflow steps, current step, status, action IDs, eligible counts, disabled
reasons, pending Add Source import requests, deterministic `tools/assets/global_asset_import.py` command arguments,
expected import-session manifest paths, and package-validation readiness. The Asset Library panel also exposes a typed
wizard render snapshot for steps, actions, disabled reasons, pending requests, and package-validation readiness. Panel
dispatchers now route Add Source, Promote Selected, Attach Selected, and Package Validate through the existing model or
export-packager entrypoints and keep the wizard snapshot current without making engine/runtime code depend on Python
tooling. Add Source picker wiring is available through an injectable panel picker with a Windows native `IFileOpenDialog`
implementation for file/archive or folder source selection.

## Success Criteria

- A user can import a ZIP or folder into a managed global library.
- URPG preserves the original source and does not mutate it.
- URPG detects duplicates, unsupported files, and source-only/tooling-only files.
- The Asset Library shows reviewable rows with previews or clear no-preview diagnostics.
- The user can promote selected safe assets with license notes.
- The user can attach selected promoted assets to the active project.
- Attached assets are visible to project authoring surfaces.
- Release/package gates exclude unapproved quarantine content.
- URPG remains releasable with no bundled third-party game assets.

## Implementation Defaults

- Store global catalog records through a `GlobalAssetLibraryStore` facade over the existing `.urpg/asset-index/asset_catalog.db` in Phase 1.
- Copy attached project assets under `content/assets/imported/<pack-or-session-id>/`.
- Write project-local promotion manifests under `content/assets/manifests/`.
- Implement ZIP extraction as a bounded tooling helper first, reusing repository scripting patterns; C++ editor/runtime code consumes the generated session manifests and never imports Python modules directly.
- Classify first-pass images by path and dimensions into `sprite`, `tileset`, `ui`, `background`, `portrait`, `vfx`, or `image/uncategorized`.
- Classify first-pass audio by path into `audio/bgm`, `audio/bgs`, `audio/se`, `audio/me`, `audio/ui`, or `audio/uncategorized`.
- Expose Phase 1 in the Asset Library panel as import-session and review-row state; later phases added native source
  picker wiring, promotion, attachment, conversion, external archive handoff, and package-validation actions.
