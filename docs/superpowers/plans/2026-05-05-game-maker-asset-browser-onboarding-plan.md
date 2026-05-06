# Game Maker Asset Browser And Onboarding Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the Mario Maker-style asset browsing, preview, template-scoped loading, and onboarding flow that makes the newly ingested URPG game-maker asset library usable without loading the whole library into memory.

**Architecture:** Keep the existing fast default `base_jrpg_parts` catalog for startup and add an indexed discovery layer that can page, filter, preview, and lazily attach larger catalogs. Templates select small asset sets by default; the full library remains available through a collapsible browser and opt-in full-library scope. UI theme packs remain game UI theme metadata until hand-authored editor-theme completeness metadata exists.

**Tech Stack:** C++20, ImGui editor panels, existing `AssetLibraryModel`, existing `GridPartCatalog`/`GridPartDocument`, JSON manifests under `content/`, SQLite/FTS asset index under `.urpg/asset-index/`, Python asset tooling, Catch2 tests, CMake/Ninja.

---

## Current Baseline

- Default startup uses `content/part_catalogs/base_jrpg_parts.json` to avoid loading the complete generated library.
- The complete generated gameplay library is available through `content/part_catalogs/game_maker_all_parts.json`.
- Generated Level Builder catalogs now cover promoted legacy assets, CuteSCKR full slices, Human RPG portraits, and ModernUI portrait-generator game-maker parts.
- UI theme ingestion now records:
  - `content/ui_themes/complete_ui_essential_flat.json`: `game_ui_ready`
  - `content/ui_themes/wenrexa_hologram.json`: `candidate`, missing bars
- The editor still lacks the cohesive asset browser, project onboarding, template-scoped loading, preview drawer, and lazy load/unload behavior required for the product flow.

## File Structure

- Create `content/schemas/asset_library_index.schema.json`: persisted index metadata shape for project/library discovery records.
- Create `content/schemas/game_template_manifest.schema.json`: template manifest shape, selected catalogs, default questions, and recommended project settings.
- Create `content/templates/game_maker/*.json`: starter template manifests for JRPG, action RPG, tactical RPG, visual novel hybrid, cozy/life sim, monster collector, and platform/adventure variants.
- Modify `engine/core/assets/asset_library.*`: add paged indexed query contracts and preview metadata.
- Modify `engine/core/map/grid_part_catalog_loader.*`: support catalog groups and lazy include resolution without full expansion.
- Modify `editor/assets/asset_library_model.*`: expose folder/category tree, search, preview rows, pinned favorites, recent projects, hidden missing projects, and template scopes.
- Modify `editor/assets/asset_library_panel.*`: render the left-side collapsible browser and preview drawer.
- Modify `editor/spatial/level_builder_workspace.*`: connect palette filtering, drag/paint placement, and selected asset scope to the indexed browser.
- Modify `apps/editor/main.cpp`: add main menu and onboarding entry routing before opening the editor workspace.
- Create `editor/project/onboarding_wizard.*`: robust new-project wizard that adapts questions by game type and can be disabled in settings.
- Create `editor/project/main_menu_panel.*`: continue/open/new/template/recent/pinned/missing-project handling.
- Modify `engine/core/app_settings.*` or current settings store: add onboarding/tips/browser layout toggles.
- Modify `tools/assets/asset_db.py`: ensure indexed metadata can feed editor queries for images, spritesheets, UI themes, audio, fonts, unknown files, and future downloaded/community templates.
- Modify docs and tests alongside each feature slice.

---

### Task 1: Asset Index Contracts

**Files:**
- Create: `content/schemas/asset_library_index.schema.json`
- Modify: `tools/assets/asset_db.py`
- Test: `tools/assets/tests/test_asset_db.py` or a new focused `tools/assets/tests/test_asset_library_index.py`

- [x] **Step 1: Write a failing schema/tool test**

Create a fixture with PNG, Aseprite, JSON manifest, unknown file, and UI theme manifest records. Assert the generated index record includes stable ID, display name, source path, preview kind, media kind, category, pack, dimensions when known, and unloadable payload hint.

- [x] **Step 2: Run the focused Python test**

Run:

```powershell
python .\tools\assets\tests\test_asset_library_index.py
```

Expected: fail because the new index metadata contract does not exist yet.

- [x] **Step 3: Implement the index output**

Add an export command to `asset_db.py` or a small wrapper that emits a JSON index shard from SQLite query results. Keep the SQLite DB local under `.urpg/asset-index/`; only deterministic manifest/shard outputs should be tracked.

- [x] **Step 4: Verify**

Run:

```powershell
python .\tools\assets\tests\test_asset_library_index.py
python .\tools\assets\asset_db.py stats
```

Expected: focused test passes; stats still reads the current local DB.

Status 2026-05-05: complete. `content/schemas/asset_library_index.schema.json` exists, `tools/assets/asset_db.py export-index` emits deterministic preview records, and `tools/assets/tests/test_asset_library_index.py` covers PNG, Aseprite, game UI theme manifest, and unknown-file metadata.

### Task 2: Template Manifest Foundation

**Files:**
- Create: `content/schemas/game_template_manifest.schema.json`
- Create: `content/templates/game_maker/jrpg_starter.json`
- Create: `content/templates/game_maker/action_rpg_starter.json`
- Create: `content/templates/game_maker/tactical_rpg_starter.json`
- Create: `content/templates/game_maker/visual_novel_hybrid_starter.json`
- Create: `content/templates/game_maker/cozy_life_starter.json`
- Create: `content/templates/game_maker/monster_collector_starter.json`
- Create: `content/templates/game_maker/platform_adventure_starter.json`
- Test: new Catch2 or Python schema validation test

- [x] **Step 1: Write failing validation coverage**

Add tests that validate each template manifest and verify each template references only a bounded default catalog subset, not `game_maker_all_parts.json`.

- [x] **Step 2: Create schema and starter manifests**

Each manifest must include `templateId`, `displayName`, `gameType`, `questionProfile`, `defaultWorldSize`, `recommendedMechanics`, `defaultCatalogs`, `optionalCatalogs`, `uiThemes`, and `futureCommunityTemplateSlot`.

- [x] **Step 3: Verify**

Run:

```powershell
ctest --test-dir build\dev-ninja-debug -R "template|schema" --output-on-failure
```

Expected: new template manifest tests pass.

Status 2026-05-05: complete. `content/schemas/game_template_manifest.schema.json` exists, and `content/templates/game_maker/` contains bounded starter manifests for JRPG, action RPG, tactical RPG, visual novel hybrid, cozy/life, monster collector, and platform/adventure onboarding lanes. `tools/assets/tests/test_game_template_manifest.py` verifies required onboarding fields, bounded default catalogs, optional full-library catalogs, SQLite index policy, default browser layout, and game UI theme metadata.

### Task 3: Lazy Catalog Loading

**Files:**
- Modify: `engine/core/map/grid_part_catalog_loader.h`
- Modify: `engine/core/map/grid_part_catalog_loader.cpp`
- Modify: `tests/unit/test_grid_part_catalog.cpp`

Status 2026-05-05: backend scope-switch primitive started. `GridPartCatalogScope` and `LoadGridPartCatalogScopeFromProject` can load an active catalog scope, identify full-library scopes, and switch back to a starter scope without retaining inactive full-library rows. `AssetLibraryModel` now exposes an `asset_browser_scope` snapshot with template scope, full-library opt-in, active catalogs, favorites, recent projects, and hidden missing projects. Remaining work: wire real catalog loading into the asset browser panel, preserve selected placed asset IDs through browser interactions, and expose active-scope diagnostics in the visible UI.

- [ ] **Step 1: Write failing tests for scoped load/unload**

Add tests for loading a starter scope, switching to the full library, returning to starter scope, and preserving selected/favorite IDs without keeping full-library parts active.

- [ ] **Step 2: Implement catalog group loading**

Add a lightweight group model that can resolve includes on demand and return only the active catalog rows needed for the current browser or template scope.

- [ ] **Step 3: Verify**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[grid_part]"
```

Expected: existing grid-part tests and new scope tests pass.

### Task 4: Left-Side Asset Browser Model

**Files:**
- Modify: `editor/assets/asset_library_model.h`
- Modify: `editor/assets/asset_library_model.cpp`
- Test: `tests/unit/test_asset_library_model.cpp` or existing asset-library tests

- [ ] **Step 1: Write failing model tests**

Cover folder/category tree generation, search, source/pack filter, pinned favorites, last 10 recent projects, missing project prompt state, hidden missing project state, and selected layout mode.

Status 2026-05-05: partially covered. `AssetLibraryModel` now has tests for selected layout mode, template/full-library scope state, pinned favorites, last-10 recent projects, hidden missing projects, asset-library-index ingestion from memory and disk, game-template manifest binding through `assetIndexPath`, folder/category/pack facet trees, text search, source/pack/category filter controls, selected asset details, and active row paging. `AssetLibraryPanel` now exposes an indexed browser render snapshot, can load a game-template manifest directly, and renders the indexed browser as a left-side drawer with rows and preview in the main content area. `NewProjectWizardModel` can discover game-maker template manifests, auto-select the first template, select a requested template, expose the selected manifest path/defaults for onboarding, and create a project folder containing `project.json`, onboarding audit output, and the selected game-template manifest. `NewProjectWizardPanel` can start the selected template through an asset-browser callback. `MainMenuModel`/`MainMenuPanel` now cover Continue/New/Open/Recent/Pinned/Missing state, and the visible editor startup path routes main menu -> onboarding/template picker -> created project -> asset browser.

- [ ] **Step 2: Implement model snapshots**

Expose deterministic render snapshots for:

- collapsible folder tree
- active filter and search text
- visible rows with preview metadata
- selected asset details
- pinned favorites
- recently opened projects
- missing/deleted project remediation actions
- active scope: template, project, or full library

- [ ] **Step 3: Verify**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"
```

Expected: asset-library model tests pass.

### Task 5: Preview Drawer

**Files:**
- Modify: `editor/assets/asset_library_panel.cpp`
- Modify: `editor/assets/asset_library_panel.h`
- Modify: image/texture helper files already used by Level Builder previews
- Test: model snapshot tests plus guarded editor startup

- [ ] **Step 1: Add preview snapshot expectations**

Test image preview, spritesheet preview metadata, UI theme preview metadata, audio placeholder preview, font metadata, JSON manifest preview, and unknown-file fallback.

- [ ] **Step 2: Implement drawer/panel rendering**

Render a left-side collapsible browser by default and a preview drawer that can show the selected file metadata without blocking the editor if a preview loader fails.

- [ ] **Step 3: Verify**

Run:

```powershell
.\tools\launcher\start_editor_guarded.ps1 -Visible -VisibleFrames 120
```

Expected: editor opens, browser renders, no crash during preview selection.

### Task 6: Level Builder Drag/Paint Integration

**Files:**
- Modify: `editor/spatial/level_builder_workspace.cpp`
- Modify: `editor/spatial/grid_part_palette_panel.cpp`
- Modify: `editor/spatial/grid_part_placement_panel.cpp`
- Modify: `tests/unit/test_grid_part_editor.cpp`

- [ ] **Step 1: Write failing interaction tests**

Cover selecting a browser part, painting a single tile, click-drag painting a stroke, right-click erase/context behavior, and preview thumbnail presence.

- [ ] **Step 2: Implement input routing**

Wire browser selection into the current `GridPartDocument` placement command path. Keep undo/redo and diagnostics intact.

- [ ] **Step 3: Verify**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[grid_part][editor]"
.\tools\launcher\start_editor_guarded.ps1 -Visible -VisibleFrames 120
```

Expected: headless interactions pass and visible startup remains stable.

Status 2026-05-06: bridge and visible handoff complete. `LevelBuilderWorkspace::SelectAssetBrowserRecord` accepts an indexed browser row and uses `stableId` as the grid-part catalog id, then routes selection through palette, placement, and supporting spatial panels. `AssetLibraryPanel` now exposes a visible "Use in Level Builder" action for the selected browser record and dispatches it through an app-level callback; `apps/editor/main.cpp` selects the record in `LevelBuilderWorkspace` and opens the Level Builder panel when the handoff succeeds. Level Builder now exposes command-path helpers for painting a selected grid rectangle and erasing the top part at a grid cell, with coverage for browser-row selection, single placement, drag-style rectangle fill, erase, and undo of both operations. Verified with `[assets][asset_library]`, `[grid_part][editor][level_builder][assets]`, `[grid_part][editor]`, `urpg_editor --project-root . --safe-mode --list-panels`, and `tools/launcher/start_editor_guarded.ps1 -Visible -VisibleFrames 120`.

### Task 7: Main Menu

**Files:**
- Create: `editor/project/main_menu_panel.h`
- Create: `editor/project/main_menu_panel.cpp`
- Modify: `apps/editor/main.cpp`
- Modify: CMake registration
- Test: new Catch2 tests for main menu model state

- [x] **Step 1: Write failing main-menu tests**

Cover Continue Last Project, New Project, Open Project, Recent Projects, Pinned Favorites, missing project locate prompt, hide missing project, settings, and bypass onboarding.

- [x] **Step 2: Implement deterministic main-menu model**

Keep the first visible app surface a real main menu, not a marketing page. It should route to onboarding for new projects and into the editor for existing projects.

- [x] **Step 3: Verify**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project][main_menu]"
.\tools\launcher\start_editor_guarded.ps1 -Visible -VisibleFrames 120
```

Expected: menu tests pass and editor startup is stable.

Status 2026-05-05: partial app wiring complete. `editor/project/main_menu_panel.*` exists with deterministic model snapshots for Continue/New/Open/Recent/Pinned/Missing/hidden-missing state. `apps/editor/main.cpp` renders the main menu as the first visible interactive surface unless an explicit panel route or render-all mode is requested, and New Project now routes to the visible wizard branch. The wizard start callback loads the selected game-template manifest into `AssetLibraryPanel`, enters the editor route, and opens the Assets panel. Verified with `[project][main_menu]`, `[project][editor][panel][onboarding]`, `[assets][asset_library]`, `urpg_editor --project-root . --safe-mode --list-panels`, and `tools/launcher/start_editor_guarded.ps1 -Visible -VisibleFrames 120`. Remaining Task 7 gap: real OS file picker/open-project integration plus visible locate/settings actions.

Status 2026-05-06: editor settings now persist `onboarding_enabled`, `help_tips_enabled`, and `asset_browser_layout`. Startup applies `onboarding_enabled` to the main-menu route decision and applies `asset_browser_layout` to `AssetLibraryModel`; shutdown writes the current browser layout back through `saveEditorSettings`. Verified with `[settings][persistence]`, `[assets][asset_library]`, `[project][editor][panel][onboarding]`, `urpg_editor --project-root . --safe-mode --list-panels`, and `tools/launcher/start_editor_guarded.ps1 -Visible -VisibleFrames 120`.

Status 2026-05-06: `MainMenuModel` now exposes a visible Settings route with deterministic onboarding/help-tip toggles and browser layout selection. `MainMenuPanel` renders the Settings route, startup initializes it from `EditorSettings`, frame sync applies browser-layout changes to `AssetLibraryModel`, and shutdown persists the selected values. Verified with `[project][main_menu]`, `[settings][persistence]`, `[assets][asset_library]`, `urpg_editor --project-root . --safe-mode --list-panels`, and `tools/launcher/start_editor_guarded.ps1 -Visible -VisibleFrames 120`.

### Task 8: Adaptive Onboarding Wizard

**Files:**
- Create: `editor/project/onboarding_wizard.h`
- Create: `editor/project/onboarding_wizard.cpp`
- Modify: settings store for onboarding/tip toggles
- Test: new Catch2 tests for wizard branching

- [ ] **Step 1: Write failing wizard tests**

Cover JRPG, action RPG, tactical RPG, visual novel hybrid, cozy/life, monster collector, and platform/adventure question flows. Verify the wizard asks fewer or more questions based on game type, includes mechanic-depth choices, recommends defaults with reasons, and allows override.

- [ ] **Step 2: Implement wizard state machine**

Persist selected template, project name/path, world size, mechanics, asset scope, UI theme, and whether onboarding/help tips are enabled.

- [ ] **Step 3: Verify**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project][onboarding]"
```

Expected: onboarding branching tests pass.

### Task 9: Lazy Load/Unload Performance Guard

**Files:**
- Modify: asset browser model and catalog loader touched above
- Create or modify: tests that assert row caps and active scope counts

- [ ] **Step 1: Write failing performance-guard tests**

Assert default startup uses starter counts, full library is loaded only after explicit scope switch, row snapshots are clipped/paged, and returning to template scope unloads full-library active rows.

- [ ] **Step 2: Implement row caps and active scope accounting**

Keep unfiltered full-library snapshot caps, use `ImGuiListClipper`, and expose active-count diagnostics so future regressions are visible.

- [ ] **Step 3: Verify**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"
.\build\dev-ninja-debug\urpg_tests.exe "[grid_part]"
.\build\dev-ninja-debug\urpg_editor.exe --project-root . --safe-mode --list-panels
```

Expected: default startup stays bounded; explicit full catalog remains opt-in.

Status 2026-05-06: row caps and active-scope accounting are now explicit in `AssetLibraryModel`. Browser page limits clamp to 200 rows, `asset_browser_scope` reports active/default/optional catalog counts plus active row limit, and performance coverage verifies a 250-record index still returns at most 200 visible rows while template/full-library scope switching keeps active catalog counts bounded. Verified with `[assets][asset_library]`, `[grid_part][editor]`, `urpg_editor --project-root . --safe-mode --list-panels`, and `tools/launcher/start_editor_guarded.ps1 -Visible -VisibleFrames 120`.

### Task 10: Documentation And Release Truth

**Files:**
- Modify: `README.md`
- Modify: `docs/agent/INDEX.md`
- Modify: `docs/agent/KNOWN_DEBT.md`
- Modify: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`
- Modify: `tools/assets/README.md`
- Modify: `docs/SCHEMA_CHANGELOG.md`

- [ ] **Step 1: Update docs with actual status**

Document the default vs full catalog split, UI theme completeness results, onboarding/browser plan, and remaining gaps.

- [ ] **Step 2: Verify docs mention canonical paths**

Run:

```powershell
rg -n "game_maker_all_parts|ui_theme|asset browser|onboarding" README.md docs tools/assets/README.md
```

Expected: canonical plan and current status are discoverable.

### Task 11: Final Verification

**Files:**
- No new files unless fixing failures.

- [ ] **Step 1: Run focused build and tests**

Run:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug --target urpg_editor urpg_tests
ctest --test-dir build\dev-ninja-debug -R "urpg_generate_(promoted|cutesckr|image_folder|ui_theme_manifest)_tool_test" --output-on-failure
.\build\dev-ninja-debug\urpg_tests.exe "[grid_part]"
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"
.\build\dev-ninja-debug\urpg_tests.exe "[theme]"
.\tools\launcher\start_editor_guarded.ps1 -Visible -VisibleFrames 120
git diff --check
```

Expected: all commands pass. If the visible editor guard fails, capture the crash/log path and fix before release-facing claims.

## Definition Of Done

- New project starts at a main menu.
- New project flow can launch adaptive onboarding and create a template-scoped project.
- Default browser layout is left-side collapsible folders/categories.
- Asset preview works for images, spritesheets, UI theme pieces, audio/font/manifest metadata, and unknown-file fallback.
- Level Builder can select, preview, click-paint, drag-paint, and erase/context assets without crashing.
- Templates load a bounded subset by default; the full library is opt-in and unloads when leaving full-library scope.
- Settings can disable onboarding/help tips and choose browser layout.
- Recent projects keep the last 10, support pinned favorites, and prompt to locate missing/deleted projects before hiding them.
- Complete UI Essential is selectable as a game UI theme; Wenrexa remains candidate-only until missing bars or equivalent mapping exists.
- Docs and schema changelog reflect the actual shipped state.

## Self-Review

- Spec coverage: covers asset/theme library index, browser, preview, templates, onboarding, main menu, recent/pinned/missing projects, settings toggles, lazy loading, drag/paint, and UI theme readiness policy.
- Placeholder scan: no intentional `TBD` or open-ended “add tests” steps; each task has concrete files and commands.
- Type consistency: uses stable terms `asset_library_index`, `game_template_manifest`, `game_ui_theme`, `editor_theme`, `game_maker_all_parts`, and `base_jrpg_parts` consistently.
