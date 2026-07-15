# Creator Product Cohesion Implementation Plan

> **For implementers:** Complete tasks in dependency order and keep the checkboxes current. Use focused tests while iterating, then run each milestone gate exactly as written. Do not promote a deferred panel or widen a release claim merely because its model compiles or its headless snapshot passes.

**Goal:** Turn the existing broad URPG engine/editor implementation into a cohesive creator product that takes a new user from project creation through asset selection, map authoring, playtest, recovery, and packaging without requiring knowledge of URPG's internal subsystem boundaries.

**Product promise:** A creator can make a small, visually distinct RPG with their own governed assets, play it quickly, recover safely from mistakes, diagnose blockers, and package the exact reviewed project.

**Architecture:** Preserve the existing deterministic native runtime, ImGui shell, registry exposure rules, project-data contracts, asset-governance boundary, and headless snapshot coverage. Integrate existing models behind shared project, map, asset, playtest, and safety contexts. Keep `level_builder` and `spatial_authoring` as valid release routes, but make them deep links into one coherent Map workspace instead of unrelated destinations.

**Tech stack:** C++20, ImGui, OpenGL/headless rendering, CMake/Ninja, Catch2, nlohmann JSON, PowerShell gates, Python asset tooling, local SQLite asset index, existing no-shell `ProcessRunner`, and governed project manifests.

---

## Success Measures

The program is complete only when the following creator outcomes are demonstrated from a clean local install:

| Outcome | Target |
| --- | --- |
| First playable project | A new user reaches a controllable starter map in 10 minutes or less without editing JSON. |
| External asset discovery | A configured library of 100,000 metadata rows becomes searchable without copying its payload into a project or Git. |
| Asset search response | Cached text/filter queries return the first visible page in 150 ms or less at the 95th percentile on the reference Windows machine. |
| Preview responsiveness | Only visible thumbnails are decoded; scrolling does not grow texture memory without bound and keeps the editor interactive. |
| Map workflow | Tile, part, prop, event, ability, validation, playtest, and package actions are reachable from one Map workspace. |
| Playtest latency | With binaries already built, current-map playtest begins in 3 seconds or less on the reference project. |
| Recovery | After a forced editor termination, the next launch offers a recoverable snapshot no older than 5 minutes without overwriting the last manual save. |
| Vertical slice | One governed example proves map transfer, dialogue, quest progression, combat, inventory, ability use, save/load, menus, and package validation. |
| Release truth | Raw external assets never become package eligible without explicit import, license metadata, promotion, and project attachment. |

Performance targets are budgets, not reasons to hide an error. When a target is missed, emit a measured diagnostic and keep the feature truthful.

## Scope Guardrails

- Do not copy or extract all of `G:\All 2D Assets Stay Here` into the repository or a project.
- Do not treat an indexed archive as licensed, promoted, attached, or release eligible.
- Do not make every compiled editor panel top-level navigation.
- Do not remove either `level_builder` or `spatial_authoring` from release routing without an explicit release-owner decision and aligned registry/docs/tests.
- Do not replace deterministic native behavior with hidden AI-generated content or an online-only dependency.
- Do not claim full RPG Maker parity, final art direction, public legal approval, platform signing, or cross-platform verification from this program alone.
- Do not add a second persistence format when an existing project contract can be extended compatibly.
- Treat `https://github.com/DocDamage/capybara_2d_engine` as an MIT-licensed design reference, initially `reference_only`, not as a vendored runtime or architectural replacement.
- Any Capybara-derived production code must enter through URPG-owned native facades, preserve required fork/upstream provenance and license notices, and pass the existing external-intake governance gate.
- Do not import Capybara's TypeScript/browser runtime, npm pathfinding dependency, DOM widgets/loading gate, cloud SDK, or external asset-generation service. URPG's native pathfinding, save/recovery, input, UI, deterministic runtime, and governed asset pipeline remain authoritative.

## Dependency Order

```text
M0 Evidence baseline
  -> M1 Project session and creator shell
      -> M2 Virtual asset catalog
          -> M3 Preview, archive, promotion, and attachment workflow
      -> M4 Unified Map workspace
          -> M5 First-run playable project
          -> M6 Fast playtest loop
      -> M7 Autosave and recovery
  -> M8 Contextual authoring integrations
      -> M9 Governed vertical slice
          -> M10 Visual/accessibility/performance polish
              -> M11 Release qualification
```

## Milestone Summary

| Milestone | Deliverable | Estimated focused effort | Exit condition |
| --- | --- | ---: | --- |
| M0 | Creator-journey baseline and automated scorecard | 3-5 days | Current friction and timing are recorded reproducibly. |
| M1 | Persistent startup shell and project session | 1-2 weeks | Recent/open/create/switch/close project paths work safely. |
| M2 | Searchable virtual external asset catalog | 2-3 weeks | External image/archive metadata is paged and searchable in the native Assets panel. |
| M3 | Preview, archive inspection, promotion, attachment, drag/drop | 3-5 weeks | A selected external sprite can safely reach a map without bulk copying. |
| M4 | Unified Map workspace | 3-5 weeks | Both release map routes share project, selection, history, diagnostics, and lifecycle state. |
| M5 | First-run playable project | 2-3 weeks | Wizard creates and opens a valid starter project that immediately playtests. |
| M6 | Fast edit/playtest/return loop | 2-4 weeks | Unsaved current-map state can be tested and diagnostics return to the editor. |
| M7 | Autosave, crash recovery, and relinking | 2-3 weeks | Forced-termination recovery and missing-asset repair are proven. |
| M8 | Contextual gameplay authoring and stable gameplay primitives | 5-8 weeks | Required deep editors open in context, share bounded runtime primitives, and satisfy the WYSIWYG done rule. |
| M9 | Governed vertical slice | 3-5 weeks | The example completes and packages through creator-facing controls. |
| M10 | Visual, accessibility, and performance polish | 3-5 weeks | Manual graphical review and automated budgets pass. |
| M11 | Release qualification | 1-3 weeks plus external approvals | Fresh gates pass; external blockers remain explicitly reported. |

The estimates are planning ranges for one focused engineer and exclude platform certificates, legal review turnaround, and creation of bespoke final art.

## Current Implementation Checkpoint (2026-07-13)

The current `development` implementation has completed the bounded M0-M5 foundation at commit `d607eecc`, but the milestone exit conditions are not all release-qualified. Checked task boxes below mean the named code/test contract exists; they do not substitute for M10 graphical review, M11 release qualification, or an unchecked breadth item.

| Milestone | Current state | Remaining qualification or breadth |
| --- | --- | --- |
| M0 | Implemented baseline/report gate | The smoke remains a mixed passed/partial/deferred baseline; it is not an end-to-end playable journey. |
| M1 | Implemented creator shell, project session, settings, and Grid Parts-triggered Map Save All guard | Aggregate Perspective 2D-only dirty state and register the remaining durable non-Map authoring surfaces with the shared owner. |
| M2 | Implemented metadata-only virtual catalog and 100,000-row performance coverage | Re-run against the creator's configured live library when qualifying a release machine. |
| M3 | Implemented preview, archive selection, promotion/attachment, and typed drag payloads | Attached drops on Map Tiles now atomically register and place a tile with one Perspective 2D undo; Props remain palette enrollment. Other contextual consumers and complete drop-history integration remain open. |
| M4 | Implemented unified Map routing, shared context/history, layout, diagnostics, and paired atomic save | Complete the remaining shortcut breadth and record graphical route/layout equivalence. |
| M5 | Implemented atomic starter-project wizard and durable creator checklist | The wizard now offers a starter-map playtest handoff; full milestone evidence still needs native runtime execution and requalification. |
| M6 | Initial implementation | Editor-owned process lifecycle, private current-map overlay, authored player-start or selected-part target (with blank-starter fallback), visible target/elapsed/exit/overlay state, F5 stop/restart, document/catalog launch blocking, and bounded JSONL diagnostics with matching GridParts-object focus are implemented. Broader runtime blocker policy, map-entrance/checkpoint targets, generic diagnostic focus, and safe reload remain open. |
| M7 | Initial implementation | Checksummed recovery snapshot primitives, orderly-session markers, five-minute bounded private capture of dirty Grid Parts, Perspective 2D, ability, Character Creator, quest, database, vendor, and audio-mix drafts, plus in-editor count, safe restore-to-new-folder, and clean-session open-recovered-project actions are implemented. Broader durable-surface coverage and asset relinking remain open. |
| M8 | Initial implementation | Map can open the active project's durable Character Creator draft or Ability Inspector while preserving project/Map context and shared save, navigation, and recovery ownership. Its Perspective 2D event form creates an event, first page, and a supported native command including text/choice dialogue, transfer, inventory, and encounter launch in the normal Map document; running an authored encounter-launch event opens the native Battle Inspector with the active character and ability draft queued, while its contextual encounter control provides the same direct preview route; its contextual audio control applies and saves a project mix preset through the native audio core and assigns an attached audio asset with owner-local undo/redo; its quest form creates, previews, saves, and recovers a typed objective graph; and its database/vendor form creates project items and database-validated vendor stock. Battle-result, broader audio cue routing, and WYSIWYG breadth remain open. |
| M9 | Draft seeded | The governed vertical-slice scenario has a native-wizard two-map draft seed and strict report gate, but no creator-authored passed runtime/package report exists. |
| M10-M11 | Planned | Manual polish/review and clean target qualification remain open. |

Focused evidence recorded for this checkpoint includes the creator-journey gate, 25 passing Python asset-import/catalog tests with one platform skip, thumbnail and Map-history tests, all three `dev-spatial` tests, 73 selected M1/M5 CTests, a successful editor headless frame, and successful `urpg_editor`/`urpg_tests` builds. Re-run the milestone commands before using that evidence for a new release target.

### PFU-I1 Qualification Contract Record (2026-07-15)

`development` is the selected integration base for the next bounded creator-product increment. The target-state contract is intentionally separate from the historical M0 baseline:

- `content/fixtures/creator_journey_qualification_spec.json` defines the required PFU-I1 creator steps, evidence kinds, diagnostic policy, deterministic budgets, and Debug/Release provenance requirements.
- `tools/ci/check_creator_journey_qualification.ps1` accepts only a complete passed report with clean, commit-matched build and evidence provenance; it rejects partial/deferred status, duplicate/missing/unknown steps, fallback diagnostics, missing artifacts, and evidence from another commit.
- `tools/ci/check_pfu_i1_qualification.ps1` invokes the target command set and emits a commit-stamped manifest only after the strict report, M9 slice, spatial, governed-asset, recovery, accessibility/input, package, install, and Debug/Release provenance checks pass.

This contract and wrapper are gate definitions, not qualification results. The historical creator-journey report remains the current mixed passed/partial/deferred baseline until native creator workflows produce the required target evidence.

---

## M0 - Establish The Creator-Journey Baseline

### Task M0.1 - Define the reference journey and evidence format

**Create:**
- `docs/product/CREATOR_JOURNEY.md`
- `docs/product/CREATOR_JOURNEY_BASELINE.md`
- `content/fixtures/creator_journey_spec.json`

- [x] Define the reference journey: launch, create project, discover external assets, attach one sprite, paint a map, add an event, choose a spawn, playtest, return, save, validate, and package.
- [x] Record expected creator actions, maximum acceptable clicks, elapsed-time budget, required diagnostics, and completion evidence for each step.
- [x] Record current behavior honestly, including routes that are deferred or require manual paths.
- [x] Separate automated evidence from required manual graphical observations.

**Acceptance:** The journey has stable step IDs and no step depends on editing project JSON manually.

### Task M0.2 - Add a deterministic creator-journey smoke report

**Create:**
- `tests/integration/test_creator_journey.cpp`
- `tools/ci/check_creator_journey.ps1`

**Modify:**
- `CMakeLists.txt`
- `docs/agent/QUALITY_GATES.md`

- [x] Drive existing headless shell, asset, map, playtest, snapshot, and package seams in journey order, recording deferred seams honestly.
- [x] Emit a JSON report containing step ID, status, duration, diagnostic codes, and artifact paths.
- [ ] Fail on missing steps, hidden fallbacks, or release-ineligible asset selection.
- [x] Keep visual-layout judgment in the manual checklist rather than fabricating headless proof.

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -R "creator journey" --output-on-failure
.\tools\ci\check_creator_journey.ps1 -BuildDirectory build/dev-ninja-debug
```

**Milestone gate:** Baseline report is checked in, deterministic smoke passes, and the manual baseline lists every visible friction point that later milestones intend to close.

---

## M1 - Create A Persistent Creator Shell And Project Session

### Task M1.1 - Introduce one editor project-session owner

**Create:**
- `editor/project/editor_project_session.h`
- `editor/project/editor_project_session.cpp`
- `tests/unit/test_editor_project_session.cpp`

**Modify:**
- `apps/editor/main.cpp`
- `CMakeLists.txt`

- [x] Own the active project root, project ID, display name, schema version, open state, dirty-surface summaries, and last project diagnostic in one session object.
- [x] Validate a project before switching the app shell to editor mode.
- [x] Make project open/switch/close explicit commands with structured success and failure results.
- [x] Reject malformed or missing project roots without discarding the currently open valid project.
- [x] Notify bound panels of a committed project switch through existing bind/set-root seams.

**Acceptance:** `apps/editor/main.cpp` no longer independently distributes an unchecked path as implicit global project state.

### Task M1.2 - Wire the existing Main Menu and New Project Wizard into startup

**Modify:**
- `apps/editor/main.cpp`
- `editor/project/main_menu_panel.*`
- `editor/project/new_project_wizard_panel.*`
- `editor/project/new_project_wizard_model.*`
- `engine/core/editor/editor_panel_registry.cpp`
- `tests/unit/test_main_menu_panel.cpp`
- `tests/unit/test_new_project_wizard.cpp`
- `tests/unit/test_editor_app_panels.cpp`

- [x] Show the creator shell when no project is supplied or the configured project cannot be opened.
- [x] Support Continue, New Project, Open Project, Settings, locate missing project, and return to main menu.
- [x] Keep the wizard nested in the startup flow; do not add it as unrelated top-level workspace navigation.
- [x] Provide visible disabled/error/remediation states for unavailable native pickers and invalid destinations.

### Task M1.3 - Persist recent projects and creator preferences

**Modify:**
- `engine/core/settings/app_settings.*`
- `engine/core/settings/app_settings_store.*`
- `editor/project/main_menu_panel.*`
- `tests/unit/test_app_settings_store.cpp`
- `tests/unit/test_main_menu_panel.cpp`

- [x] Persist the last project, up to ten recent projects, pinned projects, hidden missing projects, onboarding preference, help-tip preference, and asset-browser layout.
- [x] Normalize paths for duplicate detection without breaking display casing.
- [x] Prune neither missing nor moved projects silently; offer Locate or Hide.
- [x] Write settings atomically and preserve the last valid settings document after malformed loads.

### Task M1.4 - Add shared Save All, dirty-state, and navigation guard behavior

**Create:**
- `editor/project/editor_dirty_state_registry.h`
- `editor/project/editor_dirty_state_registry.cpp`
- `tests/unit/test_editor_dirty_state_registry.cpp`

**Modify:**
- `apps/editor/main.cpp`
- map, ability, character, and other already-persistent authoring binders as needed

- [ ] Register each durable authoring surface with stable document ID, dirty state, save command, and focus route.
- [x] Add Save, Save All, and close/switch guards with explicit Save, Discard, and Cancel outcomes.
- [x] Do not report a surface clean until its atomic write succeeds.
- [x] Preserve per-surface failure diagnostics and focus the failing editor.

**Verification:**

```powershell
ctest --preset dev-all -R "main menu|new project|project session|dirty state|settings|editor app panels" --output-on-failure
```

**Milestone gate:** A user can launch without CLI project arguments, open or locate a project, switch safely, persist recents, and never lose dirty work because of an implicit route change.

---

## M2 - Expose The External Sprite Collection As A Virtual Catalog

### Task M2.1 - Finish and harden external indexing

**Modify:**
- `tools/assets/asset_db.py`
- `tools/assets/tests/test_asset_db.py`
- `tools/assets/README.md`

- [x] Complete image/archive discovery for absolute external roots, including PNG, JPG/JPEG, GIF, BMP, SVG, ASE/ASEPRITE, ZIP, RAR, and 7z classification.
- [x] Resume interrupted scans from committed checkpoints without hiding records from other roots or media kinds.
- [x] Store source-root identity, relative virtual path, size, timestamp, media kind, archive kind, and optional hash.
- [x] Keep discovery mode fast with deferred hashing; expose a separate integrity/deduplication pass.
- [x] Emit clear unreadable-file, inaccessible-directory, malformed-archive, and extractor-unavailable diagnostics.

### Task M2.2 - Export a stable, bounded catalog interchange

**Create:**
- `tools/assets/catalog_interchange.py`
- `tools/assets/tests/test_catalog_interchange.py`
- `engine/core/assets/local_asset_catalog.h`
- `engine/core/assets/local_asset_catalog.cpp`
- `tests/unit/test_local_asset_catalog.cpp`

**Modify:**
- `CMakeLists.txt`

- [x] Atomically export `.urpg/asset-index/catalog_meta.json` plus capped JSONL metadata shards from the local SQLite database.
- [x] Version the interchange schema and include scan completeness, root states, counts, and generation timestamp.
- [x] Never include file payloads in the interchange.
- [x] Load shards lazily or incrementally in C++; index normalized filename, virtual path, extension, pack, category, and tags.
- [x] Page results so the UI never materializes every row as ImGui widgets.
- [x] Reject incompatible schema versions with remediation that names the regeneration command.

### Task M2.3 - Bind virtual-catalog query state into AssetLibraryModel

**Modify:**
- `editor/assets/asset_library_model.*`
- `editor/assets/asset_library_panel.*`
- `apps/editor/main.cpp`
- `tests/unit/test_asset_library_model.cpp`
- `tests/unit/test_asset_library_panel.cpp`

- [x] Add configured catalog roots, scan status, text search, media-kind filter, extension filter, pack/category filter, archive-only filter, sort, page, and page-size state.
- [x] Keep virtual external records distinct from promoted global-library records and project attachments.
- [x] Show counts for discovered, hash-pending, duplicate-candidate, archived, promoted, attached, and release eligible states.
- [x] Add Refresh Index and Open Source Location actions with no-shell execution and truthful disabled states.
- [x] Persist the external library root in local app settings, not project or release manifests.

### Task M2.4 - Prove scale and cancellation behavior

**Create:**
- `tests/perf/test_local_asset_catalog_perf.cpp`

- [x] Generate a deterministic 100,000-row metadata fixture without checking payloads into Git.
- [x] Measure load, query, filter, sort, and page latency.
- [x] Ensure rescan/export cancellation leaves the prior valid interchange readable.
- [x] Bound query result and diagnostic sizes.

**Verification:**

```powershell
python -m unittest tools.assets.tests.test_asset_db tools.assets.tests.test_catalog_interchange -v
ctest --preset dev-all -R "LocalAssetCatalog|AssetLibrary.*virtual|asset catalog perf" --output-on-failure
python .\tools\assets\asset_db.py stats
```

**Milestone gate:** `G:\All 2D Assets Stay Here` is searchable inside the native Assets workspace without copying or extracting the collection.

---

## M3 - Complete Preview, Archive, Promotion, Attachment, And Drag/Drop

### Task M3.1 - Add bounded editor thumbnail loading

**Create:**
- `editor/assets/editor_thumbnail_cache.h`
- `editor/assets/editor_thumbnail_cache.cpp`
- `tests/unit/test_editor_thumbnail_cache.cpp`

**Modify:**
- `editor/assets/asset_library_panel.*`
- `apps/editor/main.cpp`
- `CMakeLists.txt`

- [x] Decode only visible PNG/JPEG/BMP/GIF-static-frame thumbnails through the existing image stack.
- [x] Cache by canonical path, size, timestamp, and requested preview dimensions.
- [x] Use an LRU memory budget and release OpenGL textures on eviction and shutdown.
- [x] Render a deterministic fallback card for unsupported, corrupt, missing, or hash-pending assets.
- [x] Keep decode work off the render-critical path and cancel requests for rows that leave the viewport.

### Task M3.2 - Integrate animation and spritesheet inspection

**Modify:**
- `editor/sprite/sprite_animation_preview_panel.*`
- `editor/assets/asset_library_panel.*`
- `engine/core/editor/editor_panel_registry.cpp`
- related sprite preview tests

- [x] Open Sprite Animation Preview contextually from a selected asset rather than promoting it to top-level navigation.
- [x] Preview GIF animation and recognized frame/atlas metadata.
- [x] Offer explicit grid slicing for loose spritesheets with frame width, height, rows, columns, direction, loop, and frame-duration controls.
- [x] Save slicing metadata only to an import/promotion manifest or project attachment, never beside an untouched external source file.
- [x] Route ASE/ASEPRITE assets through an explicit conversion workflow when a supported converter is configured.

### Task M3.3 - Browse archives without bulk extraction

**Create:**
- `engine/core/assets/archive_catalog.h`
- `engine/core/assets/archive_catalog.cpp`
- `tests/unit/test_archive_catalog.cpp`

**Modify:**
- `tools/assets/global_asset_import.py`
- `editor/assets/asset_library_model.*`
- `editor/assets/asset_library_panel.*`

- [x] List ZIP entries natively where supported and use the configured 7z-compatible extractor for RAR/7z listing.
- [x] Cache an archive-entry manifest keyed by archive size, timestamp, and hash when available.
- [x] Reject absolute paths, traversal, links, device names, excessive entry counts, excessive expansion, and unsupported encryption.
- [x] Extract only user-selected entries to isolated staging before preview or import.
- [x] Make archive browse status different from promotion and release eligibility.

### Task M3.4 - Deliver one governed Attach To Project workflow

**Modify:**
- `editor/assets/asset_library_model.*`
- `editor/assets/asset_library_panel.*`
- `engine/core/assets/asset_import_session.*`
- `engine/core/assets/global_asset_library_store.*`
- `engine/core/assets/project_asset_attachment_service.*`
- corresponding asset tests

- [x] For an external asset, guide the creator through source review, license selection, optional conversion/slicing, promotion, and project attachment in one resumable workflow.
- [x] Require explicit license/attribution fields or a documented private-project-only classification before promotion.
- [x] Copy only the selected normalized payload and its manifest.
- [x] Reuse hashes to detect already-promoted and already-attached assets.
- [x] Provide Replace, Keep Both, Relink Existing, and Cancel conflict outcomes.
- [x] Verify package policy still rejects raw external paths and unpromoted staging paths.

### Task M3.5 - Add typed editor drag/drop payloads

**Create:**
- `editor/assets/editor_asset_drag_payload.h`
- `editor/assets/editor_asset_drag_payload.cpp`
- `tests/unit/test_editor_asset_drag_payload.cpp`

**Modify:**
- `editor/assets/asset_library_panel.*`
- `editor/spatial/spatial_authoring_workspace.*`
- `editor/spatial/grid_part_palette_panel.*`
- `editor/character/character_creator_panel.*`

- [x] Drag promoted/attached assets with stable asset ID, project path, kind, dimensions, and provenance state.
- [x] Refuse raw external payload drops into durable project documents and offer Attach To Project as remediation.
- [ ] Accept appropriate payloads in tile palette, prop placement, event sprite, character appearance, UI skin, and animation contexts.
- [ ] Record each accepted drop in undo/redo history and dirty-state tracking.

**Verification:**

```powershell
ctest --preset dev-all -R "thumbnail|sprite animation|archive catalog|AssetLibrary|asset drag|project attachment|export" --output-on-failure
.\tools\ci\check_release_required_assets.ps1
```

**Milestone gate:** A creator can find a sprite inside a loose folder or archive, preview it, supply governance metadata, attach only that asset, and drag it into an authoring surface.

---

## M4 - Make Level Builder And Perspective 2D One Map Experience

### Task M4.1 - Add a shared map authoring context

**Create:**
- `editor/spatial/map_authoring_context.h`
- `editor/spatial/map_authoring_context.cpp`
- `tests/unit/test_map_authoring_context.cpp`

**Modify:**
- `editor/spatial/level_builder_workspace.*`
- `editor/spatial/spatial_authoring_workspace.*`
- `apps/editor/main.cpp`

- [x] Own project root, active map ID, selected layer/object/event/part, viewport focus, active tool, validation summary, and playtest/package state.
- [x] Keep grid-part and Perspective 2D documents explicit; bridge related selections and lifecycle commands without silently converting or discarding either representation.
- [x] Route undo/redo through one visible history while preserving the responsible document owner.
- [x] Mark all affected documents dirty for cross-document commands.

### Task M4.2 - Compose one MapAuthoringWorkspace

**Create:**
- `editor/spatial/map_authoring_workspace.h`
- `editor/spatial/map_authoring_workspace.cpp`
- `tests/unit/test_map_authoring_workspace.cpp`

**Modify:**
- `apps/editor/main.cpp`
- `engine/core/editor/editor_panel_registry.cpp`
- `tests/unit/test_editor_panel_registry.cpp`
- `tests/unit/test_editor_app_panels.cpp`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`

- [x] Provide Canvas, Tiles, Parts, Props, Events, Abilities, World, Validate, Playtest, and Package modes inside one workspace.
- [x] Use the existing child panels rather than duplicating their models.
- [x] Route `level_builder` to the Parts/Build perspective and `spatial_authoring` to the Canvas/Tiles perspective of the same workspace.
- [x] Keep both registry IDs release-valid and add a visible mode switch instead of separate state islands.
- [x] Share palette search, selection, inspector, diagnostics, project references, save/load, and readiness state.

### Task M4.3 - Make the layout creator-oriented

**Modify:**
- `apps/editor/main.cpp`
- `editor/spatial/map_authoring_workspace.*`

- [x] Implement a stable layout: tool strip, left palette/library, central canvas, right inspector, bottom diagnostics/timeline.
- [x] Allow panels to collapse and persist proportions per project/user settings.
- [x] Keep the canvas usable at 1280x720 and scale cleanly at high DPI in the bounded layout model; graphical DPI review remains M10.
- [ ] Add keyboard shortcuts for Save, Save All, Undo, Redo, Playtest, palette search, delete, duplicate, focus selection, and command palette.
- [x] Display the current map, tool, layer, dirty state, validation state, and playtest target at all times.

### Task M4.4 - Unify save, validation, and package readiness

- [x] Save all map-owned documents atomically or leave the previous valid versions intact.
- [x] Aggregate diagnostics with stable focus targets that switch mode and select the offending object.
- [x] Make package readiness explain the next creator action rather than expose only raw counters.
- [x] Keep release-asset, accessibility, performance, and human-review evidence separate and truthful.

**Verification:**

```powershell
ctest --preset dev-spatial --output-on-failure
ctest --test-dir build/dev-ninja-debug -R "MapAuthoring|Level Builder|Spatial Authoring|Editor panel registry|editor app panels" --output-on-failure
```

**Manual verification:** Launch `urpg_editor`, open both map routes, and confirm they show the same active map, selected object, dirty state, undo history, and diagnostics while preserving the requested initial mode.

**Milestone gate:** The creator experiences one Map editor; the two release routes are useful shortcuts, not competing workspaces.

---

## M5 - Deliver A First-Run Playable Project

### Task M5.1 - Add an atomic project-creation service

**Create:**
- `engine/core/project/project_creation_service.h`
- `engine/core/project/project_creation_service.cpp`
- `tests/unit/test_project_creation_service.cpp`

**Modify:**
- `engine/core/project/project_template_generator.*`
- `editor/project/new_project_wizard_model.*`

- [x] Accept template, project ID/name, destination, display settings, input preset, starter-map choice, and optional external library root.
- [x] Validate destination and project ID before writing.
- [x] Write to a temporary sibling directory, run project audit, then atomically publish or roll back.
- [x] Generate the minimum runtime, map, database, input, save-profile, and project manifest data required by preflight.
- [x] Never leave a half-created project after failure or cancellation.

### Task M5.2 - Turn the wizard into a guided creator flow

**Modify:**
- `editor/project/new_project_wizard_panel.*`
- `editor/project/new_project_wizard_model.*`
- `apps/editor/main.cpp`
- wizard tests and snapshots

- [x] Steps: Project, Template, Visual Style, Asset Library, Starter Map, Review, Create.
- [x] Provide JRPG, action RPG, tactics, visual novel, and blank/native presets only where current template certification supports them.
- [x] Preview what each template creates and report unsupported/deferred capabilities honestly.
- [x] Detect the already-indexed external library and allow Skip/Configure Later.
- [x] After success, open the new project directly in the unified Map workspace with a highlighted next action.

### Task M5.3 - Add an optional first-project checklist

**Create:**
- `editor/project/creator_checklist.h`
- `editor/project/creator_checklist.cpp`
- `tests/unit/test_creator_checklist.cpp`

- [x] Guide: choose hero art, paint/edit map, place player start, create NPC event, preview dialogue, playtest, save, and validate.
- [x] Derive completion from durable project state rather than button-click flags.
- [x] Allow dismissal and restoration from Help.
- [x] Persist per-project completion without affecting runtime or release payloads.

**Verification:**

```powershell
ctest --preset dev-all -R "project creation|new project|template|creator checklist|preflight|startup" --output-on-failure
.\tools\ci\check_creator_journey.ps1 -BuildDirectory build/dev-ninja-debug
```

**Milestone gate:** From an empty startup shell, a new user can create a valid project, enter a starter map, and begin playtest without command-line arguments or JSON edits.

---

## M6 - Shorten Edit, Playtest, Diagnose, And Return

### Task M6.1 - Add a playtest session controller

**Create:**
- `editor/playtest/playtest_session_controller.h`
- `editor/playtest/playtest_session_controller.cpp`
- `tests/unit/test_playtest_session_controller.cpp`

**Modify:**
- `apps/editor/main.cpp`
- `editor/spatial/grid_part_playtest_panel.*`
- `editor/spatial/spatial_authoring_workspace.*`
- `editor/spatial/map_authoring_workspace.*`

- [ ] Launch the already-built runtime through `ProcessRunner` with explicit project, map, spawn, and session-manifest arguments.
- [ ] Serialize unsaved map state into an ignored `.urpg/playtest/<session>` overlay; never pretend it is a manual save.
- [ ] Refuse launch on blockers that would make the result misleading; allow reviewed nonblocking warnings.
- [ ] Track starting, running, stopping, exited, crashed, and returned states.
- [ ] Stop only the process owned by the session and clean stale overlays safely.

### Task M6.2 - Return structured runtime diagnostics to editor objects

**Modify:**
- runtime CLI/startup and diagnostics seams
- `editor/playtest/playtest_session_controller.*`
- `editor/spatial/map_authoring_workspace.*`
- related runtime and diagnostics tests

- [ ] Write versioned JSONL diagnostics with map ID, object/event ID, source file, diagnostic code, severity, and timestamp.
- [ ] Stream or ingest diagnostics without blocking the editor frame.
- [ ] On return, show the playtest summary and focus the selected diagnostic in the correct map mode.
- [ ] Keep stdout/stderr available as bounded secondary evidence.

### Task M6.3 - Add safe data hot reload in bounded stages

- [ ] Stage 1: fast stop/relaunch preserving editor selection and playtest target.
- [ ] Stage 2: reload explicitly supported JSON resources—dialogue, event pages, abilities, and map metadata—at a runtime safe point.
- [ ] Version each reload request and return accepted/rejected resource IDs with reasons.
- [ ] Require relaunch for native code, plugin graph, schema, renderer, or incompatible map topology changes.
- [ ] Never label a rejected reload as applied.

### Task M6.4 - Add playtest controls and shortcuts

- [ ] Play from player start, selected tile/object, map entrance, and saved checkpoint.
- [ ] Add Play/Stop/Restart and `F5`/`Shift+F5` bindings with remapping support.
- [ ] Display runtime version, active overlay, target map/spawn, elapsed time, and last exit status.
- [ ] Preserve unsaved editor state after Return To Editor.

**Verification:**

```powershell
ctest --preset dev-all -R "playtest session|grid part playtest|Perspective.*playtest|runtime diagnostics|ProcessRunner" --output-on-failure
```

**Milestone gate:** Current-map playtest uses the creator's latest state, starts quickly, returns safely, and turns runtime failures into focusable editor diagnostics.

---

## M7 - Add Autosave, Crash Recovery, And Asset Relinking

### Task M7.1 - Extend ProjectSnapshotStore for editor recovery

**Modify:**
- `engine/core/project/project_snapshot_store.*`
- `editor/project/editor_dirty_state_registry.*`
- `apps/editor/main.cpp`
- snapshot-store tests

**Create:**
- `editor/project/editor_recovery_service.h`
- `editor/project/editor_recovery_service.cpp`
- `tests/unit/test_editor_recovery_service.cpp`

- [ ] Write ignored recovery snapshots on a configurable interval only when durable surfaces are dirty.
- [ ] Use atomic manifests with project identity, source revisions, dirty document IDs, timestamp, app version, and checksums.
- [ ] Retain a bounded rolling set by count and total bytes.
- [ ] Place a session marker on successful project open and clear it only on orderly close.
- [ ] Detect an unclean previous session and offer Preview, Restore Copy, Replace Current, Discard, or Later.

### Task M7.2 - Keep autosave separate from manual save

- [ ] Autosave recovery data under `.urpg/recovery/`; do not overwrite authored project files silently.
- [ ] Report the last manual save and last recovery snapshot separately.
- [ ] Pause autosave during project switch, package publication, and atomic manual writes.
- [ ] Surface per-document snapshot failures without blocking unrelated documents.

### Task M7.3 - Add missing-asset relinking

**Create:**
- `engine/core/assets/asset_relink_service.h`
- `engine/core/assets/asset_relink_service.cpp`
- `editor/assets/asset_relink_panel.h`
- `editor/assets/asset_relink_panel.cpp`
- `tests/unit/test_asset_relink_service.cpp`

- [ ] Match moved assets by promotion ID/hash first, then size/name as an explicitly lower-confidence suggestion.
- [ ] Preview every reference that will change.
- [ ] Apply relinks as one reversible command and update project attachment manifests atomically.
- [ ] Never relink automatically to an unpromoted external source.

**Verification:**

```powershell
ctest --preset dev-all -R "snapshot|recovery|autosave|dirty state|asset relink|project attachment" --output-on-failure
```

**Manual verification:** Force-terminate the editor after dirtying multiple surfaces, relaunch, restore to a copy, and compare the recovered map/ability/assets with the pre-termination state.

**Milestone gate:** A crash or moved governed asset produces a guided recovery path rather than silent loss or fallback content.

---

## M8 - Integrate Deep Editors Contextually

### Task M8.1 - Add contextual routing instead of navigation sprawl

**Create:**
- `editor/ui/editor_context_action.h`
- `editor/ui/editor_context_action.cpp`
- `tests/unit/test_editor_context_actions.cpp`

**Modify:**
- `engine/core/editor/editor_panel_registry.*`
- `apps/editor/main.cpp`
- `editor/spatial/map_authoring_workspace.*`

- [ ] Define context actions with stable route, object kind/ID, project path, selection payload, and return route.
- [ ] Open deep editors as inspector tabs, modal workflows, or docked child surfaces.
- [ ] Preserve map selection and viewport when entering and returning.
- [ ] Keep panels Deferred until their complete integration meets the WYSIWYG done rule.

### Task M8.2 - Wave A: event, dialogue, character, and database authoring

**Modify:**
- `editor/events/*`
- `editor/dialogue/*`
- `editor/character/*`
- `editor/database/*`
- relevant runtime loaders and tests

- [ ] Event: author pages, conditions, commands, movement, sprite, and trigger from selected map event.
- [ ] Dialogue: author graph/text/choices, preview, localization keys, and bind the result back to the event command.
- [ ] Character: choose attached appearance assets, animation/slicing profile, actor data, and runtime preview.
- [ ] Database: edit referenced actors, items, switches, variables, common events, encounters, and starting party with reference validation.
- [ ] Each surface must provide visual authoring, live preview, saved project data, runtime execution, diagnostics, undo/redo, and tests.

### Task M8.3 - Wave B: quest, battle, ability, inventory, and economy loops

**Modify:**
- `editor/quest/*`
- `editor/battle/*`
- `editor/ability/*`
- `editor/items/*`
- `editor/shop/*`
- related runtime and tests

- [ ] Bind quest steps to map events, dialogue outcomes, item changes, and battle results.
- [ ] Launch battle preview from an encounter/event context and return results to the authoring surface.
- [ ] Bind existing ability assets through the map/character/battle inspectors.
- [ ] Validate item/vendor/loot references and expose runtime preview state.
- [ ] Avoid top-level promotion unless repeated creator testing proves a separate workspace is necessary.

### Task M8.4 - Wave C: audio, accessibility, input, and export context

**Modify:**
- `editor/audio/*`
- `editor/accessibility/*`
- `editor/input/*`
- `editor/export/*`
- `editor/spatial/map_authoring_workspace.*`

- [ ] Choose map/event/battle audio from attached project assets with preview and bus controls.
- [ ] Run accessibility checks against the active map/menu/dialogue and focus the violating control/object.
- [ ] Preview keyboard/controller bindings from the current project profile.
- [ ] Open exact-ship export preview and package blockers from the Map Package mode.

### Task M8.5 - Add stable gameplay facade, Perspective 2D ordering, and bounded NPC/prop primitives

**Reference:**
- `https://github.com/DocDamage/capybara_2d_engine`
- Review concepts from its single `src/Game.ts` public facade, feet-anchor/Y-sort contract, NPC primitives, stateful map overlays, and focused gameplay recipes.
- Reimplement only approved behavior in native URPG code; do not copy the browser runtime wholesale.

**Create:**
- `engine/core/gameplay/gameplay_runtime_facade.h`
- `engine/core/gameplay/gameplay_runtime_facade.cpp`
- `engine/core/presentation/perspective2d_render_order.h`
- `engine/core/presentation/perspective2d_render_order.cpp`
- `engine/core/npc/npc_runtime_primitives.h`
- `engine/core/npc/npc_runtime_primitives.cpp`
- `engine/core/map/map_prop_state_set.h`
- `engine/core/map/map_prop_state_set.cpp`
- `tests/unit/test_gameplay_runtime_facade.cpp`
- `tests/unit/test_perspective2d_render_order.cpp`
- `tests/unit/test_npc_runtime_primitives.cpp`
- `tests/unit/test_map_prop_state_set.cpp`
- bounded creator recipes under `docs/product/recipes/`

**Modify:**
- `engine/core/scene/map_scene.*`
- `engine/core/events/event_runtime.*`
- `engine/core/level/path_request_router.*`
- `editor/spatial/spatial_authoring_workspace.*`
- `editor/spatial/map_authoring_workspace.*`
- `tools/urpg_mcp/server.py` and its tool manifests/tests as warranted
- `docs/external-intake/repo-watchlist.md`
- `docs/external-intake/license-matrix.md`

- [ ] Expose one compact, thread-documented URPG gameplay facade for stable spawn, query, event, input, navigation, resource, and contextual-authoring operations; delegate to existing subsystem owners rather than duplicating their models.
- [ ] Keep agent/MCP access bounded: read-only inspection by default, structured preview for durable changes, explicit apply where already authorized, stable diagnostics, and no arbitrary runtime or filesystem mutation.
- [ ] Define one Perspective 2D ground-anchor contract used consistently by spawn placement, collision footprints, navigation destinations, interaction distance, and render ordering; prevent callers from repeatedly converting between sprite top-left and feet coordinates.
- [ ] Add deterministic `ground`, `occluder`, and `prop` render layers, ordered by authored layer semantics, feet/render Y, and stable entity/object ID as the final tie-breaker.
- [ ] Add typed NPC operations for move-to point/location, stop, face player, proximity observation, bounded thought/bark display, and current activity; return structured success/failure codes and reuse the existing path router/event runtime.
- [ ] Keep NPC behavior deterministic and authored. Do not introduce hidden model calls, autonomous online simulation, or unbounded generated dialogue into the runtime loop.
- [ ] Add stateful map-prop sets for doors, gates, chests, crops, switches, and similar objects, where each state can select an attached sprite, collision footprint, render layer, interaction metadata, and emitted event.
- [ ] Make prop-state authoring contextual from the Map inspector, participate in shared undo/redo and dirty tracking, validate asset/event references, and round-trip through project data into runtime behavior.
- [ ] Add concise recipes for NPC patrol/proximity, stateful doors/containers, map placement, quest/inventory hookups, HUD visibility, and save-safe stable IDs using only public URPG facades.
- [ ] Record Capybara's fork/upstream provenance and MIT disposition before copying any expression-level implementation; prefer independent URPG-native implementations of the concepts.
- [ ] Prove that the adoption adds no npm, browser DOM, Capybara cloud SDK, online-only asset generation, or second save/pathfinding/runtime stack to shipping URPG targets.

**Acceptance:** A creator can place a Perspective 2D NPC and stateful door through the Map workspace, preview correct feet-based occlusion and collision, trigger deterministic movement/state changes, and playtest the result through the same native project contracts. The local MCP/AI surface can inspect and invoke only the bounded approved facade operations with truthful diagnostics.

**Verification:**

```powershell
ctest --preset dev-all -R "gameplay runtime facade|Perspective.*render order|NPC runtime primitive|map prop state|pathfinding|event runtime" --output-on-failure
.\tools\ci\check_phase4_intake_governance.ps1
```

### Task M8.6 - Promotion review for each deferred surface

- [ ] Record owner, creator entry point, saved contract, runtime consumer, preview, diagnostics, tests, and empty/error/disabled evidence.
- [ ] Update registry exposure only after those fields are complete.
- [ ] Prefer Nested exposure for contextual tools.
- [ ] Add registry and app-shell regression coverage for every exposure change.

**Verification:**

```powershell
ctest --preset dev-all -R "event|dialogue|character|database|quest|battle|ability|inventory|vendor|audio|accessibility|input|export|WYSIWYG" --output-on-failure
.\tools\ci\truth_reconciler.ps1
```

**Milestone gate:** The vertical-slice-required gameplay data can be created from project/map context without JSON editing or a maze of unrelated top-level panels, and NPC/prop behavior reaches runtime through one deterministic native facade and shared Perspective 2D anchor/order contract.

---

## M9 - Build One Governed Creator Vertical Slice

### Task M9.1 - Define the slice and asset manifest

**Create:**
- `content/examples/creator_vertical_slice/README.md`
- `content/examples/creator_vertical_slice/acceptance.json`
- governed project files and manifests under the same example root

- [ ] Use only repository-approved proof assets or specifically selected, attributed, promoted, and hydrated assets.
- [ ] Include two connected maps, one player character, two NPCs, one quest, one item reward, one vendor interaction, one encounter, two abilities, one save point, menus, and an ending state.
- [ ] Keep the story small enough to replay in under 15 minutes.
- [ ] Document every asset source and redistribution status.

### Task M9.2 - Author the slice through creator-facing workflows

- [ ] Create/open the project through the startup shell.
- [ ] Attach assets through the Assets workflow.
- [ ] Build maps, events, dialogue, character, quest, encounter, ability bindings, and audio through contextual editors.
- [ ] Record any required out-of-band JSON edit as a product blocker; do not normalize it as acceptable process.
- [ ] Fix the editor workflow before marking the slice complete.

### Task M9.3 - Add end-to-end runtime and packaging evidence

**Create:**
- `tests/integration/test_creator_vertical_slice.cpp`
- `tools/ci/check_creator_vertical_slice.ps1`

- [ ] Validate new game, map transfer, dialogue choice, quest activation, combat result, inventory reward, ability use, vendor interaction, save, load, ending, and return to title.
- [ ] Build a deterministic package and compare its inventory with the governed project attachments.
- [ ] Prove no raw source, external absolute path, ignored local DB, recovery snapshot, or playtest overlay ships.
- [ ] Emit a bounded JSON completion report.

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -R "creator vertical slice" --output-on-failure
.\tools\ci\check_creator_vertical_slice.ps1 -BuildDirectory build/dev-ninja-debug
.\tools\ci\check_release_required_assets.ps1
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

**Milestone gate:** The example can be recreated, played, saved, loaded, validated, and packaged through the same workflows offered to users.

---

## M10 - Apply Visual, Accessibility, And Performance Polish

### Task M10.1 - Establish editor design tokens and reusable UI patterns

**Create:**
- `editor/ui/editor_theme.h`
- `editor/ui/editor_theme.cpp`
- `editor/ui/editor_widgets.h`
- `editor/ui/editor_widgets.cpp`
- `docs/product/EDITOR_VISUAL_LANGUAGE.md`

**Modify:**
- `apps/editor/main.cpp`
- release top-level and required nested panels

- [ ] Centralize color, spacing, typography scale, icon sizing, control height, severity, selection, focus, and disabled-state styling.
- [ ] Add reusable status banner, empty state, diagnostic row, asset card, inspector section, command button, progress, and destructive confirmation widgets.
- [ ] Use labels and icons together for important commands.
- [ ] Keep themes readable at 100%, 125%, 150%, and 200% scale.

### Task M10.2 - Run a complete graphical UX pass

- [ ] Review startup, wizard, Assets, unified Map, each contextual vertical-slice editor, playtest return, recovery, diagnostics, and package preview.
- [ ] Check clipping, overlap, tiny targets, inconsistent terms, ambiguous destructive actions, empty space, focus loss, and missing tooltips.
- [ ] Test 1280x720, 1920x1080, ultrawide, and high-DPI layouts.
- [ ] Record before/after screenshots and close every P0/P1 visual issue.

### Task M10.3 - Complete keyboard and accessibility behavior

- [ ] Provide deterministic tab/focus order and visible keyboard focus.
- [ ] Ensure every icon-only affordance has an accessible label and tooltip.
- [ ] Meet the existing contrast policy for text, selection, diagnostics, and disabled states.
- [ ] Make core journey steps possible without precise mouse dragging.
- [ ] Respect reduced-motion and animation-preview pause preferences.

### Task M10.4 - Enforce UI and asset performance budgets

**Create:**
- `tests/perf/test_editor_creator_workflow_perf.cpp`

- [ ] Measure frame time while scrolling large asset results, painting, switching modes, opening inspectors, and streaming playtest diagnostics.
- [ ] Bound background jobs, queued thumbnails, texture memory, JSON snapshot size, and diagnostic rows.
- [ ] Move expensive scans/decodes off the frame path with cancellation and generation IDs.
- [ ] Expose a creator-facing slow-operation diagnostic rather than freezing silently.

**Verification:**

```powershell
ctest --preset dev-all -R "accessibility|editor creator workflow perf|thumbnail|MapAuthoring|editor app panels" --output-on-failure
.\tools\ci\check_accessibility_governance.ps1
.\tools\ci\run_presentation_gate.ps1
```

**Milestone gate:** The full reference journey passes graphical review, keyboard/accessibility review, and performance budgets on the reference hardware.

---

## M11 - Qualify The Improved Product For Distribution

### Task M11.1 - Reconcile public behavior and release scope

**Modify as warranted by actual completed behavior:**
- `README.md`
- `CHANGELOG.md`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- `docs/agent/ARCHITECTURE_MAP.md`
- `docs/agent/KNOWN_DEBT.md`

- [ ] Document the startup, virtual asset, unified map, playtest, recovery, contextual authoring, and vertical-slice surfaces only after their gates pass.
- [ ] Keep virtual/cataloged, promoted, attached, release-required, and shipped asset terms distinct.
- [ ] Record remaining unsupported formats, hot-reload boundaries, platform picker state, and external approval blockers.

### Task M11.2 - Run clean-clone and package qualification

- [ ] Test a clean non-sparse clone with required LFS payloads hydrated.
- [ ] Configure and build Debug and Release from empty build directories.
- [ ] Run creator journey, vertical slice, PR, nightly, snapshot, spatial, export, project-audit, and package/install gates.
- [ ] Inspect the installed/package contents and execute the packaged runtime.
- [ ] Verify version metadata and exact shipping manifest.

### Task M11.3 - Close or report external distribution requirements

- [ ] Compile and exercise native pickers on Windows, macOS, and Linux runners.
- [ ] Run sanitizer evidence with a working supported toolchain.
- [ ] Supply signing/notarization credentials through the existing release profile; never check them into the repository.
- [ ] Obtain qualified legal/privacy review or record the release-owner decision without mislabeling it as counsel approval.
- [ ] Make the final release/tag decision only after fresh evidence is attached.

**Final verification:**

```powershell
cmake --preset dev-ninja-release
cmake --build --preset dev-release
ctest --preset dev-pr --output-on-failure
ctest --preset dev-spatial --output-on-failure
ctest --preset dev-snapshot --output-on-failure
ctest --preset dev-export --output-on-failure
ctest --preset dev-project-audit --output-on-failure
.\tools\ci\check_creator_journey.ps1 -BuildDirectory build/dev-ninja-release
.\tools\ci\check_creator_vertical_slice.ps1 -BuildDirectory build/dev-ninja-release
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1
```

**Final exit:** Engineering gates pass from a clean release build, manual creator and graphical walkthroughs are signed off, exact package contents are reviewed, and every external requirement is either completed or truthfully blocks the corresponding distribution claim.

---

## Cross-Cutting Implementation Rules

### Data and migration

- Extend project schemas compatibly and version every new durable contract.
- Provide migrations for existing projects and retain the previous valid document on migration failure.
- Use temp-file plus atomic rename for settings, projects, manifests, autosaves, and catalog interchange.
- Keep user-local paths under ignored local state, never inside portable project manifests.

### Diagnostics

- Every disabled action names why it is disabled and what unlocks it.
- Every failure result includes a stable code, creator-readable message, responsible subsystem, and focus/remediation target when possible.
- Bound logs, snapshots, and report arrays; summarize overflow counts.
- Never substitute placeholder assets or silently discard malformed authored data in release workflows.

### Asset safety and governance

- Discovery is read-only against the external source library.
- Archive inspection and selected extraction remain containment-audited.
- Promotion requires provenance and license state; project attachment requires a promoted payload.
- Export accepts project-selected governed paths only and rejects external absolute paths, raw intake, staging, local DB, recovery, and playtest files.

### External implementation references

- Classify external repositories before adoption and keep the repo watchlist/license matrix synchronized with the implementation plan.
- Default Capybara 2D Engine to `reference_only`; elevate an individual concept to `production_candidate` only with a named URPG-owned facade, bounded scope, provenance, license disposition, tests, and removal/rollback path.
- Prefer native reimplementation of small contracts and algorithms. If source is copied or adapted, retain all required MIT notices and identify the exact upstream/fork revision in the intake record.
- Never allow a reference implementation to create a parallel runtime, persistence, pathfinding, input, UI, asset-governance, or online-service authority.

### Testing

- Add unit tests for every new model/service state transition.
- Add headless render snapshots for empty, loading, ready, disabled, and error states.
- Add integration tests at workflow boundaries instead of only testing isolated panels.
- Require manual graphical verification for layout, tooltips, focus visibility, drag/drop feel, and animation quality.
- Run the narrowest relevant gate during implementation and the milestone gate before checking the milestone complete.

## Recommended Sprint Order

Do not begin with visual re-skinning or broad panel promotion. Execute in this order:

1. Finish the current external asset index and land its tested tooling changes.
2. Complete M0 and M1 so every later workflow has a stable project/session owner.
3. Complete M2 and M3 to turn the user's sprite library into a safe authoring advantage.
4. Complete M4 before expanding map-adjacent panels.
5. Complete M5, M6, and M7 to make the core creator loop fast and safe.
6. Promote only the M8 contextual integrations and bounded gameplay primitives required by the vertical slice; complete the Capybara-reference disposition before implementation-level reuse.
7. Use M9 to expose remaining workflow defects.
8. Perform M10 polish after the workflow stabilizes.
9. Run M11 qualification and make a fresh release-owner decision.

## Definition Of Done For Every Milestone

A milestone is done only when:

- its acceptance behavior works through the actual app shell;
- saved project data round-trips into runtime behavior;
- errors and disabled states are visible and actionable;
- focused automated tests pass;
- required manual graphical verification is recorded;
- canonical documentation reflects actual behavior;
- no release claim is widened beyond the evidence; and
- `git diff --check` passes with no generated/local artifacts added to Git.
