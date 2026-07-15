# PFU-03 Work Packet: deterministic atlas-metadata revision contract

**Status:** Implemented bounded foundation slice

**Date:** 2026-07-15

## Scope

Add the first native PFU-03 transform revision path. `AssetTransformRevisionService` creates a deterministic, immutable atlas-metadata manifest for one reviewed, runtime-ready promoted image. This packet creates metadata only: it does not decode, crop, scale, slice, palette-convert, or otherwise alter source media.

## Traceability and owners

- **Source outcomes:** F03 Asset Transformation Studio; F21 Personal Asset Vault; I01-I02 and I09 deterministic/provenance workflow.
- **Native owner:** `engine/core/assets/asset_transform_revision_service.*`.
- **Source owner:** a validated `AssetPromotionManifest` whose promoted payload is runtime-ready and package-eligible.
- **Artifact location:** caller-selected derived asset root, under `<asset-id>/revisions/<derived-revision>.json`.
- **Consumer boundary:** this packet does not attach, assign, preview, or package a derived revision. Existing promotion and attachment owners remain the only governed entry points for those actions.

## Contract

1. An atlas plan supplies an operation ID, validated promoted source, output root, and positive atlas/frame dimensions. Atlas dimensions must divide exactly by frame dimensions.
2. The service hashes the actual promoted payload with SHA-256 for the source revision. It hashes the canonical plan identity (schema, operation, operation ID, source asset ID/revision, and dimensions) for the derived revision.
3. The manifest records the schema, operation, operation ID, source identity/path/revision, derived revision, dimensions, and calculated frame count.
4. Publication stages the manifest beside its final path and publishes with a single rename. An identical existing manifest is reused; divergent content at the deterministic revision path fails closed as a collision.
5. The source promoted payload is never modified. No raw external input path or mutable transform state becomes authoritative project data.
6. An un-attached derived manifest can be removed only by its safe asset ID and deterministic revision ID. The service verifies the manifest identity before deletion and leaves source media unchanged.

## Safety limits

- This is metadata, not a raster transformation or visual-preview claim.
- The initial atlas-metadata slice had no creator-facing transform panel, cancellation/resume, operation journal, attachment integration, reference-aware removal, or automatic package inclusion. Later bounded slices below add transforms and selected attachment/assignment owners; the remaining limits are stated with each slice.
- Crop/slice/scale/palette/tileset pixel output, audio revisions and QA, collections/favorites/comparison, and end-to-end contextual assignment remain open PFU-03 work.
- A derived revision is content/provenance-addressed, but it is not an attachment receipt and does not by itself establish runtime or release eligibility.

## Acceptance and verification

- A valid atlas plan produces a manifest with stable source/derived revisions.
- Repeating the same plan reuses the manifest without changing the source payload.
- Invalid non-divisible dimensions are rejected before publication.

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library][asset_transform]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library][asset_attachment]" --reporter compact
```

2026-07-15 implementation evidence: the focused transform lane covers deterministic creation/reuse, source preservation, safe removal, missing-revision rejection, and invalid dimensions. The attachment regression lane remains included to guard the existing governed downstream seam.

## Subsequent PFU-03 curation slice

User-scoped favorites and named collections now persist through `EditorSettings` as opaque stable asset keys: promoted assets use their promoted asset ID; catalog-only assets use a SHA-256 key derived from their canonical catalog identity. This curation is explicitly non-authoritative: it does not promote, attach, license, package, or retain an external filesystem path. `AssetLibraryModel` exposes favorite and collection membership actions plus a snapshot for native panel rendering, and the Assets workspace provides collection creation, favorite toggling, and collection-membership toggling. The same workspace now offers a bounded, read-only two-up comparison of governed rows (identity, kind, dimensions, category, and cached image previews); it cannot mutate either asset. The editor loads/saves curation with existing user settings.

## Subsequent PFU-03 pixel transform slice

`AssetTransformRevisionService` now implements a native `image_crop_scale` operation. It decodes a runtime-ready promoted image, validates an in-bounds crop rectangle, applies deterministic nearest-neighbor scaling, stages a PNG, and publishes a content/provenance-linked revision manifest. Repeating the plan reuses only a matching manifest plus output; malformed, missing, out-of-bounds, and colliding inputs fail closed. The promoted source is never modified. This is a first real image operation, not support for palette/tileset transforms, visual editing/preview, cancellation, reference-aware removal, or derived-revision attachment.

`AssetTransformRevisionService` also implements `image_palette`: every decoded RGBA pixel maps to the nearest color in an explicitly ordered two-to-256 entry RGBA palette, using deterministic squared-distance and first-entry tie breaking. The ordered palette is in the revision identity and manifest; duplicate palettes are rejected. The focused transform fixture decodes the resulting PNG and verifies the output pixels, reuse, and rejection behavior. This is fixed-palette reduction, not automatic palette extraction, dithering, color-profile conversion, or tileset slicing.

`AssetTransformRevisionService` implements optional Floyd-Steinberg dithering for both explicit `image_palette` and automatic `image_palette_extract` revisions. The fixed scan/distribution order stores error in signed fixed-sixteenth RGBA units, so it has no floating-point or platform-dependent rounding path. For extraction, exact decoded RGBA values rank by frequency descending and RGBA ascending tie break before selecting up to the requested two-to-256 color limit. The selected ordered palette, selection policy, and dither mode are all part of immutable revision identity and manifest. Both paths reuse matching complete output, and the single PNG is accepted by the existing reviewed derived-revision attachment path. Clustering, color-profile conversion, and perceptual color-space selection remain open.

`AssetTransformRevisionService` now implements `tileset_slice`: an exact positive tile grid with optional uniform margin and spacing becomes a staged deterministic directory of individual PNG tiles, named in row-major zero-padded order. The manifest records every output path and grid dimensions. Reuse requires both a matching manifest and tile directory. Un-attached removal verifies every manifest-listed tile is a PNG directly inside the revision’s expected tile directory, deletes those tiles, requires that directory to be empty, then removes the manifest. This is a pixel-producing slice, not collision metadata authoring or direct Map placement by itself.

## Subsequent PFU-03 audio revision slice

`AssetTransformRevisionService` now also implements `audio_trim_fade_gain_pcm16` for reviewed, runtime-ready PCM16 WAV promotions. The plan uses explicit source-frame trim bounds, optional frame-based fades, bounded integer gain, and optional derived-output loop bounds. It writes a deterministic WAV and provenance manifest with sample rate, channel count, duration, 64 waveform peaks, and sample-peak/RMS dBFS metrics (explicitly not LUFS). It reuses only a matching complete revision and never alters the promoted source. Un-attached removal now deletes a manifest-declared PNG or WAV only after checking it is the deterministic sibling of the requested revision; it never trusts an arbitrary output path. Unsupported codecs, LUFS/loudness conformance, loop-seam listening, spectrograms, captions, voice take/locale/rights metadata, cancellation, reference-aware removal after attachment, and project attachment/assignment are deliberately outside this slice. The focused transform lane passes 37 assertions including valid creation/reuse, source preservation, manifest metadata, safe payload cleanup, and range rejection.

## Subsequent PFU-03 derived-revision attachment slice

`ProjectAssetAttachmentService` now accepts a checked `ProjectDerivedAssetAttachmentRequest` for the immutable single-file PNG/WAV revisions produced by `image_crop_scale`, `image_palette`, `image_palette_extract`, and `audio_trim_fade_gain_pcm16`. Before it reuses the existing project attachment transaction, it validates the transform schema, source asset and source content hash, deterministic revision ID, operation, output sibling/path, and output file. It stages a sidecar attachment-reference entry before the project transaction and finalizes it only after attachment; the transform remover fails closed while that entry is prepared or attached. The resulting project asset has a stable revision-qualified ID and retains source/derived revision provenance in `authoredMetadata`; it remains subject to the existing review revision, operation receipt, conflict, rollback, and recovery rules. `AssetLibraryModel`, `AssetLibraryPanel`, and the Assets workspace expose the same review/confirm flow, then reload the project asset into its normal contextual picker with the library’s existing kind-based target selection. Atlas metadata remains deliberately rejected because it needs a dedicated metadata owner. Actual Map placement, undo/history, runtime preview, package qualification, cancellation, and detach/recovery tooling for prepared references remain outside this bounded slice.

`ProjectAssetAttachmentService` also owns checked `tileset_slice` assignment. Its review derives a deterministic bundle revision from the promoted source, immutable slice manifest/grid, and SHA-256 of every ordered tile; confirmation rechecks that revision, stages every expected row-major PNG, deterministically nearest-neighbor packs arbitrary exact source cell sizes into a bounded project-owned 48-by-48-cell `atlas.png`, and publishes the directory plus a project-owned `content/tilesets/<tileset-id>.json` provenance manifest under the selected conflict policy. The manifest records the atlas path, hash, dimensions, and runtime cell dimensions. It writes a separate idempotent operation receipt and the same fail-closed derived-reference marker, so a prepared or assigned bundle blocks global revision removal. The Assets workspace exposes review/confirm controls and reports the project tileset ID and grid. This establishes project custody and bounded runtime atlas data, not automatic tile collision authoring, crash recovery, or detach tooling.

The Assets workspace also previews the current source-scoped derived image revision through the existing render-thread thumbnail cache. It accepts only an immutable transform manifest with a SHA-256-shaped revision ID and the exact expected sibling PNG (`image_crop_scale`, `image_palette`, or `image_palette_extract`), or the canonical first row-major tile of a `tileset_slice` directory. When the source row remains image-previewable, the workspace renders it beside that validated derived result. A malformed, missing, unexpected, or non-image output stays unavailable; the comparison never writes, promotes, attaches, or changes the revision. This is a bounded result preview, not a crop-selection canvas, tileset-page Map insertion, or runtime rendering claim.

An assigned tileset bundle of any exact sliced source-cell size can now be imported into the active Perspective 2D Map as one undoable native document change. Assignment normalizes those cells to a declared 48-by-48 runtime atlas; the Map owner revalidates the project assignment schema, grid, canonical project tileset manifest location, deterministic tile list/index/filename, each required PNG, and the packed atlas path/hash/runtime-cell dimensions before adding one tileset page, palette options, and default per-tile metadata using the existing map persistence/history owner. Re-import replaces only that bundle’s page/options/definitions. The Map workspace exposes a selected-palette tile inspector for solid collision and four persisted passage directions; the same owner records each edit in local history. The bound `MapScene` now projects visible unlocked derived tiles onto the registered atlas and applies the same default or edited boolean collision metadata to movement; clearing, undo/redo, hiding, and draft load remove stale projections. This is bounded Map/runtime consumption, not a directional collision contract, cross-map propagation, or detached-bundle recovery.

The Assets workspace now also exposes creator-facing transform controls: a promoted image can be given a stable operation ID plus crop/nearest-neighbor-scale dimensions, an explicitly authored ordered two-to-256 color RGBA palette, an exact-RGBA frequency-extraction bound, an exact tileset grid, or atlas/frame metadata; a PCM16 WAV can be given exact trim/fade/gain and optional derived-loop frame parameters. Image and audio single-output revisions fill the reviewed derived-manifest field for the existing attachment confirmation flow. Tileset slicing emits its deterministic row-major PNG directory and manifest, then enters its dedicated project tileset review/confirm flow; atlas metadata remains a validated non-media revision with no assignment owner. These are parameter-entry controls, not yet a visual crop canvas, waveform/spectrogram editor, codec conversion, cancellation/resume job surface, atlas assignment, or direct Map placement command.
