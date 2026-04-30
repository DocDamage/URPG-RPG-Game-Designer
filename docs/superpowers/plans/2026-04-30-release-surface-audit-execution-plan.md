# Release Surface Audit Remediation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the confirmed release-surface audit findings and add repeatable gates that prevent unwired UI, debug exposure, placeholder surfaces, broken persistence, asset/reference drift, and release packaging regressions from re-entering the app.

**Architecture:** Treat `engine/core/editor/editor_panel_registry.*` as the release navigation contract and `apps/editor/main.cpp` as the app-shell wiring contract. Keep production surfaces reachable only when they have real render factories, handlers, state snapshots, persistence paths, diagnostics, and tests; otherwise demote them with explicit reasons.

**Tech Stack:** C++20, CMake/Ninja, Catch2, ImGui/headless editor smoke, PowerShell CI scripts, nlohmann/json.

**Current progress:** P0-001 through P2-002 are implemented and verified on branch `codex/release-surface-p0`. Editor startup/list/smoke, `level_builder` app-shell routing, compiled-panel exposure ownership, release placeholder governance, and fixture/mock readiness overclaim governance are now covered by tests or CI scripts. Next planned task is P3-001.

---

## Phase 0 - Build Blockers And App-Breaking Issues

### Task P0-001: Restore editor startup by reconciling release panel registry with app-shell factories

**Task ID:** P0-001

**Title:** Editor release navigation must only include panels that the app can register and render.

**Files to edit:**
- `engine/core/editor/editor_panel_registry.cpp`
- `tests/unit/test_editor_panel_registry.cpp`
- `apps/editor/main.cpp`
- `tests/unit/test_app_cli.cpp` or a new focused app-shell registration test if app-main helpers are split out
- `docs/release/EDITOR_CONTROL_INVENTORY.md`

**Files to inspect:**
- `apps/editor/main.cpp`
- `engine/core/editor/editor_panel_registry.h`
- `engine/core/editor/editor_shell.h`
- `docs/agent/KNOWN_DEBT.md`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`

**Dependencies:** None.

**Risk level:** Critical. The editor currently fails before listing panels or running smoke.

**Exact implementation steps:**
- [x] Run `rg -n "EditorPanelExposure::ReleaseTopLevel|renderFactories|registerEditorPanels|smokeRequiredEditorPanelIds" engine/core/editor apps/editor tests/unit docs/release`.
- [x] Add a small test seam in `apps/editor/main.cpp` if needed: move the render-factory key list into a helper such as `std::vector<std::string> editorAppRegisteredPanelFactoryIds()` in a new `apps/editor/editor_app_panels.*` pair, then use it from `registerEditorPanels`.
- [x] Write a failing test that compares `topLevelEditorPanels()` with the app factory ids and reports every missing id. The test must fail before registry reconciliation.
- [x] Change `engine/core/editor/editor_panel_registry.cpp` so release top-level ids match the canonical implemented shell set for this task: `diagnostics`, `assets`, `ability`, `patterns`, `mod`, and `analytics`.
- [x] Demote `level_builder` to `Deferred` for P0-001 with an explicit reason that P1-002 would promote it only after a real app-shell factory was added; P1-002 has now promoted it.
- [x] Demote every other currently unwired `ReleaseTopLevel` entry to `Deferred`, `Nested`, or `DevOnly` with a reason that names its promotion gate.
- [x] Update registry tests so they assert the canonical release set and hidden entries have non-empty reasons, not the previous 146-entry release list.
- [x] Update `docs/release/EDITOR_CONTROL_INVENTORY.md` and `docs/APP_RELEASE_READINESS_MATRIX.md` to match the resulting release set and verification commands.

**Acceptance criteria:**
- `urpg_editor --headless --list-panels --frames 1 --project-root .` exits 0 and lists only reachable release panels.
- `urpg_editor_smoke` exits 0.
- Every `ReleaseTopLevel` registry id has an app render factory. `level_builder` is promoted by P1-002 after its app-shell factory was wired.
- Every non-release compiled panel has a promotion/defer/dev-only reason.

**Verification command or manual test:**
- `ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|urpg_editor_smoke|urpg_editor_list_panels" --output-on-failure`
- `.\build\dev-ninja-debug\urpg_editor.exe --headless --list-panels --frames 1 --project-root .`

### Task P0-002: Keep developer debug overlay out of release navigation

**Task ID:** P0-002

**Title:** Reclassify the developer debug overlay as dev-only while preserving dev-mode feature tests.

**Files to edit:**
- `engine/core/editor/editor_panel_registry.cpp`
- `tests/unit/test_editor_panel_registry.cpp`
- `tests/unit/test_community_wysiwyg_features.cpp`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`

**Files to inspect:**
- `engine/core/community/community_wysiwyg_feature.cpp`
- `content/fixtures/developer_debug_overlay_fixture.json`
- `editor/community/community_wysiwyg_panel.cpp`

**Dependencies:** P0-001 if both tasks touch registry expectations. Can be done in the same patch as P0-001.

**Risk level:** High. A dev console/battle test/map transfer/save surface is currently classified as production navigation.

**Exact implementation steps:**
- [x] Change `developer_debug_overlay` exposure from `ReleaseTopLevel` to `DevOnly`.
- [x] Keep the existing reason text, but prefix it with a clear release boundary such as `Dev-only overlay; excluded from release navigation.`
- [x] Update `test_editor_panel_registry.cpp` so `developer_debug_overlay` is expected in `DevOnly` coverage, not `smokeRequiredEditorPanelIds()`.
- [x] Update `test_community_wysiwyg_features.cpp`: keep the dev-mode runtime gating test, but change the registry hook assertion for this feature to expect `EditorPanelExposure::DevOnly`.
- [x] Update `EDITOR_CONTROL_INVENTORY.md` so the Production Panel Exposure Map lists `developer_debug_overlay` under `DevOnly`.

**Acceptance criteria:**
- `developer_debug_overlay` cannot appear in `requiredTopLevelPanelIds()` or editor smoke required ids.
- Dev-mode feature behavior remains tested through `content/fixtures/developer_debug_overlay_fixture.json`.

**Verification command or manual test:**
- `ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|Community WYSIWYG|urpg_editor_smoke" --output-on-failure`

### Task P0-003: Stop compat report failure dashboards from counting stale execution events

**Task ID:** P0-003

**Title:** Separate compat execution diagnostics from failure-dashboard assertions.

**Files to edit:**
- `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- `editor/compat/compat_report_panel.cpp` if product behavior should change rather than test setup
- `editor/compat/compat_report_panel.h` if adding filtered query helpers

**Files to inspect:**
- `runtimes/compat_js/plugin_manager.h`
- `runtimes/compat_js/plugin_manager.cpp`
- `editor/compat/compat_report_panel.cpp`

**Dependencies:** None.

**Risk level:** Medium. The failure is test-visible and may also confuse users by mixing success and failure timeline entries without buckets.

**Exact implementation steps:**
- [x] Decide the intended behavior: for this failure-lifecycle test, the panel dashboard should verify only failure diagnostics.
- [x] In the test setup at the start of the save-data lifecycle test, call both `pm.clearFailureDiagnostics()` and `pm.clearExecutionDiagnostics()` after `pm.unloadAllPlugins()`.
- [x] Immediately before `panel.refresh()`, also clear execution diagnostics if the preceding manual `CompatReportModel` ingest intentionally consumed only failure diagnostics.
- [x] Confirmed no filtered model helper is needed for this task because the product panel can still ingest both event types; only the failure-focused test needed stale execution diagnostics cleared before refresh.
- [x] Keep `CompatReportPanel::refresh()` clearing both diagnostics buffers after ingestion so repeated refreshes do not duplicate events.

**Acceptance criteria:**
- The save-data lifecycle compat test observes exactly two missing-command warning events in its failure dashboard assertion.
- Successful execution diagnostics remain available in tests that explicitly cover execution diagnostics.
- Repeated `CompatReportPanel::refresh()` calls do not duplicate events from the same plugin-manager buffers.

**Verification command or manual test:**
- `ctest --test-dir build\dev-ninja-debug -R "curated save-data lifecycle failures|Compat fixtures.*presentation lifecycle|Compat fixtures.*save-data" --output-on-failure`

## Phase 1 - Unwired UI, Routes, Handlers, And Feature Surfaces

### Task P1-001: Add a registry-to-factory coverage gate for editor navigation

**Task ID:** P1-001

**Title:** Prevent release panels from being registered without app-shell routes.

**Files to edit:**
- `apps/editor/editor_app_panels.h`
- `apps/editor/editor_app_panels.cpp`
- `apps/editor/main.cpp`
- `tests/unit/test_editor_app_panels.cpp`
- `CMakeLists.txt`

**Files to inspect:**
- `engine/core/editor/editor_panel_registry.cpp`
- `engine/core/editor/editor_shell.h`
- Existing test registration blocks in `CMakeLists.txt`

**Dependencies:** P0-001.

**Risk level:** Medium. Adds a new guard around editor startup wiring.

**Exact implementation steps:**
- [x] Extract the panel factory ids from `apps/editor/main.cpp` into `editorAppRegisteredPanelFactoryIds()`.
- [x] Add `editorAppMissingReleasePanelFactoryIds()` that returns release top-level ids absent from the factory-id set.
- [x] Use the helper from `registerEditorPanels()` to preserve runtime behavior.
- [x] Add a Catch2 test asserting `editorAppMissingReleasePanelFactoryIds().empty()`.
- [x] Register the new test source in `CMakeLists.txt`.

**Acceptance criteria:**
- A future `ReleaseTopLevel` registry entry without a factory fails unit tests before app smoke.
- The helper returns stable, sorted ids for readable failure output.

**Verification command or manual test:**
- `ctest --test-dir build\dev-ninja-debug -R "editor app panels|Editor panel registry|urpg_editor_smoke" --output-on-failure`

### Task P1-002: Wire Level Builder into the editor shell

**Task ID:** P1-002

**Title:** Make `level_builder` reachable from production navigation.

**Files to edit:**
- `apps/editor/main.cpp`
- `tests/unit/test_editor_app_panels.cpp`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`

**Files to inspect:**
- `editor/spatial/level_builder_workspace.h`
- `editor/spatial/level_builder_workspace.cpp`
- `tests/unit/test_grid_part_editor.cpp`
- `tests/unit/test_editor_panel_registry.cpp`

**Dependencies:** P0-001.

**Risk level:** High. Level Builder is documented as the shippable native map editor, but it was not in the app factory set.

**Exact implementation steps:**
- [x] Add `urpg::editor::LevelBuilderWorkspace level_builder_workspace;` or the correct namespace/type to `EditorPanelRuntime`.
- [x] Initialize it with `project_root` and default grid-part/catalog state using existing Level Builder APIs.
- [x] Add a `renderFactories` entry for `"level_builder"` that calls the workspace render/snapshot method used by tests.
- [x] Ensure startup without a project-specific catalog produces an explicit disabled/empty state, not a crash.
- [x] Add or update app factory coverage tests to include `level_builder`.
- [x] Update inventory verification evidence to include `ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure`.

**Acceptance criteria:**
- `--open-panel level_builder` succeeds.
- `urpg_editor_smoke` renders `level_builder`.
- The panel exposes save/load/export/playtest/package actions in the snapshot or a documented disabled state.

**Verification command or manual test:**
- `.\build\dev-ninja-debug\urpg_editor.exe --headless --open-panel level_builder --frames 2 --project-root .`
- `ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure`

### Task P1-003: Inventory implemented panels that are compiled but not reachable

**Task ID:** P1-003

**Title:** Classify implemented editor panels as release, nested, deferred, or dev-only with no ambiguous surfaces.

**Files to edit:**
- `engine/core/editor/editor_panel_registry.cpp`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`

**Files to inspect:**
- All files from `rg --files editor -g '*panel.cpp' -g '*workspace.cpp'`
- `CMakeLists.txt`
- `tests/unit/test_*panel*.cpp`

**Dependencies:** P0-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Generate the compiled panel list with `rg --files editor -g '*panel.cpp' -g '*workspace.cpp'`.
- [x] For each compiled panel, verify it has a registry entry or is intentionally nested under an existing workspace.
- [x] For each registry entry, verify the target file exists or the reason states why it has no app route.
- [x] Promote only panels with app factories, handlers, persistence, diagnostics, and tests.
- [x] Demote feature-doc-only panels to `Deferred` with promotion criteria.
- [x] Update the inventory table with the final exposure map.

**Acceptance criteria:**
- Every compiled user-facing panel has exactly one exposure decision.
- No `ReleaseTopLevel` entry points to a missing or unwired panel.
- No implemented release-grade panel is omitted from either navigation or nested workspace documentation.

**Verification command or manual test:**
- `rg -n "EditorPanelExposure::ReleaseTopLevel|EditorPanelExposure::Deferred|EditorPanelExposure::Nested|EditorPanelExposure::DevOnly" engine/core/editor/editor_panel_registry.cpp`
- `ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|editor app panels" --output-on-failure`

## Phase 2 - Incomplete Implementations, Stubs, Placeholders, Mock Data

### Task P2-001: Replace release-facing placeholder/stub language or demote the surface

**Task ID:** P2-001

**Title:** Ensure release surfaces do not expose stub, fake, mock, or placeholder behavior as production.

**Files to edit:**
- `engine/core/editor/editor_panel_registry.cpp`
- Any release-surface source found by the audit query
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md` if status labels change

**Files to inspect:**
- `engine/core/scene/map_scene.cpp`
- `engine/core/ui/ui_window.cpp`
- `engine/core/ui/ui_command_list.cpp`
- `engine/core/tools/export_packager_executable_staging.cpp`
- `engine/core/ability/gameplay_ability.cpp`
- `runtimes/compat_js/window_compat.cpp`
- `tools/docs/incubator/doc_generator.cpp`

**Dependencies:** P1-003.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Run `rg -n "TODO|FIXME|stub|placeholder|fake|mock|For testing|dummy|dev bootstrap" engine editor apps runtimes tools -g '*.cpp' -g '*.h' -g '*.hpp'`.
- [x] For every hit under a release or package path, choose one outcome: implement real behavior, add explicit diagnostic fallback, or demote/defer the feature.
- [x] For map-scene testing dialogue or diagnostic placeholder rendering, ensure the user-facing snapshot states the fallback reason.
- [x] For export-packager bootstrap placeholders, ensure docs and package manifests label them as dev bootstrap unless production signing/artifact paths are implemented.
- [x] Add a CI script or unit test that fails on banned placeholder terms in `ReleaseTopLevel` owner files unless the line is in an allowlist with a documented reason.

**Acceptance criteria:**
- No release top-level surface silently uses fake/mock/stub behavior.
- Allowed placeholder paths are documented as diagnostic, headless, incubator, or dev bootstrap.

**Verification command or manual test:**
- `rg -n "TODO|FIXME|stub|placeholder|fake|mock|For testing|dummy|dev bootstrap" engine editor apps runtimes tools -g '*.cpp' -g '*.h' -g '*.hpp'`
- New placeholder-governance test or script added by this task.

### Task P2-002: Gate mock data and fixture-backed claims from release status

**Task ID:** P2-002

**Title:** Prevent fixture-only systems from being described as release-ready.

**Files to edit:**
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`
- Relevant tests under `tests/unit/test_*readiness*.cpp`

**Files to inspect:**
- `docs/agent/KNOWN_DEBT.md`
- `content/readiness/wysiwyg_done_rule.json`
- `tools/ci/truth_reconciler.ps1`
- `tools/docs/check-agent-knowledge.ps1`

**Dependencies:** P2-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Search readiness docs for `READY`, `VERIFIED`, `fixture`, `mock`, `partial`, and `dev bootstrap`.
- [x] For each fixture-only lane, verify status is below ready or has explicit release-owner acceptance language.
- [x] Update readiness JSON and docs so status labels match implementation evidence.
- [x] Add or update truth-reconciliation coverage to flag `READY` rows that mention fixture-only or mock-only evidence.

**Acceptance criteria:**
- Release docs cannot overclaim mock-only or fixture-only behavior.
- Truth reconciliation fails on future status drift.

**Verification command or manual test:**
- `.\tools\ci\truth_reconciler.ps1`
- `.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug`

## Phase 3 - Error Handling, Validation, Loading States, Empty States

### Task P3-001: Audit release panels for disabled, loading, empty, and error states

**Task ID:** P3-001

**Title:** Every release panel must expose actionable UI state for missing data, loading, empty collections, and errors.

**Files to edit:**
- Release panel files from the final registry set
- `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Focused tests for each panel, such as `tests/unit/test_asset_library_panel.cpp`, `tests/unit/test_analytics_panel.cpp`, `tests/unit/test_grid_part_editor.cpp`

**Files to inspect:**
- `editor/analytics/analytics_panel.cpp`
- `editor/assets/asset_library_panel.cpp`
- `editor/ability/ability_inspector_panel.cpp`
- `editor/ability/pattern_field_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `editor/mod/mod_manager_panel.cpp`
- `editor/spatial/level_builder_workspace.cpp`

**Dependencies:** P0-001, P1-002.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Run `rg -n "RenderSnapshot|disabled|empty|error|loading|lastRenderSnapshot|recordCommandResult" editor tests/unit`.
- [x] For each release panel, add or verify snapshot fields for `status`, `disabledReason`, `emptyReason`, `errorMessage`, and enabled/disabled action states using the local naming pattern already present in that panel.
- [x] For every button or command in `EDITOR_CONTROL_INVENTORY.md`, verify disabled-state evidence exists.
- [x] Add failing tests for any missing empty or error state before implementation.
- [x] Implement the minimal snapshot/status changes to pass tests.

**Acceptance criteria:**
- Missing runtime bindings, missing project roots, empty datasets, failed loads, and disabled handlers are visible in snapshots and user-facing panel state.
- No release panel returns a silent `{}` snapshot.

**Verification command or manual test:**
- `ctest --preset dev-all -R "editor|empty|loading|error" --output-on-failure`
- `ctest --test-dir build\dev-ninja-debug -R "AssetLibrary|Analytics|Ability|Pattern|grid_part" --output-on-failure`

**Progress note (2026-04-30):** Implemented and verified release-panel state evidence for Ability, Pattern, and Mod snapshots, and documented state evidence for all release top-level panels. Focused verification passes:
- `ctest --test-dir build\dev-ninja-debug -R "ModManagerPanel|PatternFieldPanel|Ability execution history captures|Panel snapshot exposes empty runtime" --output-on-failure`
- `ctest --test-dir build\dev-ninja-debug -R "AssetLibrary|Analytics|Ability|Pattern|grid_part" --output-on-failure`

Follow-up broad-gate cleanup resolved on 2026-04-30:
- Updated Maker and Gameplay WYSIWYG registry tests to assert `Deferred` for feature-family panels after release navigation demotion.
- Updated `DataManager: database loading and accessors` to use strict VisuMZ sample assertions only when the optional external sample data is present, and otherwise assert the seeded database contract.
- `ctest --preset dev-all -R "editor|empty|loading|error" --output-on-failure` now passes.

### Task P3-002: Normalize handler feedback for buttons, menu items, and commands

**Task ID:** P3-002

**Title:** Every actionable control must record success/failure feedback and a disabled reason.

**Files to edit:**
- Release panel files from P3-001
- `docs/release/EDITOR_CONTROL_INVENTORY.md`

**Files to inspect:**
- Output of `rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor`
- `tests/unit/test_*panel*.cpp`

**Dependencies:** P3-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Run the ImGui control query from `EDITOR_CONTROL_INVENTORY.md`.
- [x] For each release-surface control, verify there is a handler path and a test that proves it mutates state, records feedback, or is disabled with a reason.
- [x] Add missing command-result fields using existing panel patterns such as `recordCommandResult`.
- [x] Add tests for controls that currently render but do not mutate state.
- [x] Update inventory rows with the verified behavior and disabled-state evidence.

**Acceptance criteria:**
- No release control exists without a handler or disabled-state contract.
- Every action has visible feedback through snapshot or panel state.

**Verification command or manual test:**
- `rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor`
- `ctest --test-dir build\dev-ninja-debug -R "Panel|Inspector|Workspace|grid_part" --output-on-failure`

## Phase 4 - Persistence, API, Storage, And Data Integrity

### Task P4-001: Verify save/load/export paths for release authoring surfaces

**Task ID:** P4-001

**Title:** Release authoring actions must persist through project data and recover cleanly.

**Files to edit:**
- `apps/editor/main.cpp`
- `editor/spatial/level_builder_workspace.cpp`
- `editor/ability/ability_inspector_panel.cpp`
- `engine/core/settings/app_settings_store.cpp` only if settings path defects are found
- Focused tests under `tests/unit/`

**Files to inspect:**
- `engine/core/settings/app_settings.*`
- `engine/core/save/*`
- `engine/core/project/project_snapshot_store.*`
- `engine/core/ability/authored_ability_asset*`
- `engine/core/map/grid_part_serializer.*`

**Dependencies:** P1-002, P3-002.

**Risk level:** High. Save/load defects can corrupt authoring work.

**Exact implementation steps:**
- [x] List release authoring actions: ability save/load/apply, pattern edit, Level Builder save/load/export/playtest/package, analytics consent/settings.
- [x] For each action, write or update a round-trip test that saves to a temp project root, reloads into a fresh panel/runtime, and asserts semantic equality.
- [x] Ensure file writes create parent directories and report path-specific errors.
- [x] Ensure failed loads preserve the last valid in-memory document unless the panel explicitly clears with feedback.
- [x] Ensure settings persistence updates are saved after editor shutdown and do not depend on window-size mutation in headless mode.

**Acceptance criteria:**
- All release authoring surfaces have deterministic save/load/export tests.
- Failed persistence paths emit errors without data loss.

**Verification command or manual test:**
- `ctest --preset dev-all -R "settings|persistence|save|load|grid_part|Ability" --output-on-failure`

**Progress note (2026-04-30):** Completed P4-001 persistence coverage for release authoring actions. Ability draft save/load/project-content paths now expose `last_io` with operation, success, path, and message fields while preserving the existing boolean APIs. Authored ability asset file IO reports path-specific write/read/parse/schema errors and still creates parent directories. Added regression coverage for nested draft saves, malformed-load preservation of the previous draft, project-content save paths, and optional DataManager fixture fallback contracts. Existing Level Builder save/load/export/playtest/package, analytics local JSONL/settings, pattern edit, and app settings persistence coverage passed under the required gate:
- `cmake --build --preset dev-debug --target urpg_tests`
- `git diff --check`
- `ctest --test-dir build\dev-ninja-debug -R "DiagnosticsWorkspace - ability draft IO|DiagnosticsWorkspace - ability draft state save|selected project ability asset" --output-on-failure`
- `ctest --test-dir build\dev-ninja-debug -R "AnalyticsPanel reports release local export handler|Level Builder saves canonical|Level Builder loads canonical|Level Builder load command rejects|Level Builder exposes native validation playtest" --output-on-failure`
- `ctest --preset dev-all -R "settings|persistence|save|load|grid_part|Ability" --output-on-failure`

### Task P4-002: Audit input consistency, pause/resume, and scene transition persistence

**Task ID:** P4-002

**Title:** Runtime/editor navigation should behave consistently across keyboard/controller/touch abstractions and pause/resume transitions.

**Files to edit:**
- `engine/core/input/*`
- `engine/core/scene/*`
- `apps/runtime/main.cpp`
- Tests under `tests/unit/test_input_core.cpp`, `tests/unit/test_scene_manager.cpp`, and related runtime startup tests

**Files to inspect:**
- `engine/core/input/input_remap_profile.cpp`
- `engine/core/action/controller_binding_runtime.cpp`
- `engine/core/scene/runtime_title_scene.*`
- `engine/core/scene/map_scene.cpp`
- `engine/core/scene/scene_manager.cpp`

**Dependencies:** P4-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Trace confirm/cancel/menu/pause inputs from runtime startup through title, map, menu, and editor preview.
- [ ] Add tests for consistent confirm/cancel handling across at least title and menu/map flows.
- [ ] Add or verify pause/resume state restoration tests for active scene stack and input state.
- [ ] Add diagnostics for unsupported controller/touch bindings instead of silent fallback.

**Acceptance criteria:**
- Keyboard/controller/touch abstractions have either implemented mappings or explicit unsupported diagnostics.
- Pause/resume does not lose active scene or stale input state.

**Verification command or manual test:**
- `ctest --preset dev-all -R "startup|settings|input|SceneManager|RuntimeTitleScene" --output-on-failure`

## Phase 5 - Asset, Manifest, Packaging, And Release-Readiness Fixes

### Task P5-001: Verify connected assets exist and unused release assets are classified

**Task ID:** P5-001

**Title:** Asset references must resolve through manifests or produce release-blocking diagnostics.

**Files to edit:**
- `tools/ci/check_release_required_assets.ps1`
- Asset manifests under `content/`, `data/`, or `imports/normalized/` as needed
- `docs/release/RELEASE_PACKAGING.md`

**Files to inspect:**
- `.gitattributes`
- `resources/icons/*.png`
- `content/schemas/asset_pipeline_manifest.schema.json`
- `tools/ci/run_release_candidate_gate.ps1`
- `engine/core/project/runtime_project_preflight.*`

**Dependencies:** P2-001.

**Risk level:** High for packaging/export.

**Exact implementation steps:**
- [ ] Run `.\tools\ci\check_release_required_assets.ps1`.
- [ ] Run `rg -n "asset_id|assetId|texture_path|preview_asset|bgm_asset|bgs_asset|promoted_relative_path" content data engine editor tools -g '*.json' -g '*.cpp' -g '*.h' -g '*.ps1'`.
- [ ] For each connected release asset reference, verify the file exists and is not an unresolved LFS pointer.
- [ ] For each missing asset, either add/promote the asset, retarget to an existing asset, or mark the owning feature deferred.
- [ ] Add a report under the repo’s existing report folder if the scan produces non-actionable unused source assets.

**Acceptance criteria:**
- Release-required connected assets exist in the expected tree.
- Missing assets fail preflight with exact paths.
- Unused assets are classified as source-only, deferred, or connected.

**Verification command or manual test:**
- `.\tools\ci\check_release_required_assets.ps1`
- `ctest --preset dev-all -R "AssetLoader|Runtime map asset|preflight|asset" --output-on-failure`

### Task P5-002: Re-run install/package smoke and reconcile metadata/legal release blockers

**Task ID:** P5-002

**Title:** Packaging must include runtime/editor binaries, metadata, legal notices, and accurate release status.

**Files to edit:**
- `cmake/packaging.cmake`
- `CMakeLists.txt`
- `docs/release/RELEASE_PACKAGING.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `THIRD_PARTY_NOTICES.md`, `CREDITS.md`, `EULA.md`, `PRIVACY_POLICY.md` if package contents or status change

**Files to inspect:**
- `tools/ci/check_install_smoke.ps1`
- `tools/ci/check_package_smoke.ps1`
- `resources/windows/*.rc.in`
- `engine/core/version.h`
- `cmake/urpg_version.h.in`

**Dependencies:** P5-001.

**Risk level:** High for release distribution.

**Exact implementation steps:**
- [ ] Run install smoke and package smoke against the release build directory.
- [ ] Verify version/build metadata is present in generated headers, Windows resources, package manifests, and CLI `--version`.
- [ ] Verify package docs include license, credits, third-party notices, privacy policy, and EULA.
- [ ] Keep legal status `PARTIAL` or `BLOCKED` unless qualified review or waiver exists.
- [ ] Update packaging docs if any installed/package file list changes.

**Acceptance criteria:**
- Install/package smoke pass.
- Release package metadata and legal docs match readiness matrices.
- Public-release blockers remain visible until explicitly cleared.

**Verification command or manual test:**
- `.\tools\ci\check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke`
- `.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke`

## Phase 6 - Tests, Regression Coverage, And Final Verification

### Task P6-001: Add targeted regression tests for every confirmed audit finding

**Task ID:** P6-001

**Title:** Convert audit findings into durable tests.

**Files to edit:**
- `tests/unit/test_editor_panel_registry.cpp`
- `tests/unit/test_editor_app_panels.cpp`
- `tests/unit/test_community_wysiwyg_features.cpp`
- `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- `CMakeLists.txt`

**Files to inspect:**
- Failed test output from Phase 0
- `docs/agent/QUALITY_GATES.md`

**Dependencies:** P0-001, P0-002, P0-003, P1-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Add regression coverage for: no missing app factories, debug overlay dev-only, release docs/registry canonical set, and compat panel diagnostic bucket counts.
- [ ] Ensure each test fails on the original audited state.
- [ ] Ensure test names are discoverable by `ctest -R "Editor panel registry|editor app panels|Community WYSIWYG|curated save-data lifecycle"`.
- [ ] Update `docs/agent/QUALITY_GATES.md` if a new narrow command is introduced.

**Acceptance criteria:**
- Each confirmed audit finding has a focused test that would catch recurrence.
- New tests are registered in CTest.

**Verification command or manual test:**
- `ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|editor app panels|Community WYSIWYG|curated save-data lifecycle" --output-on-failure`

### Task P6-002: Run final release-surface verification gates

**Task ID:** P6-002

**Title:** Prove the remediated app passes editor, packaging, asset, and release-surface gates.

**Files to edit:**
- None unless verification exposes failures.

**Files to inspect:**
- `docs/agent/QUALITY_GATES.md`
- `tools/ci/run_local_gates.ps1`
- `tools/ci/run_release_candidate_gate.ps1`
- Generated reports under existing report folders

**Dependencies:** All prior tasks.

**Risk level:** High because this is the final release-surface confidence gate.

**Exact implementation steps:**
- [ ] Run the narrow gates first: editor smoke, registry/app panel tests, compat diagnostics test, grid-part Level Builder tests, asset preflight.
- [ ] Run install/package smoke.
- [ ] Run PR-level tests.
- [ ] Run full local gates if time and environment allow.
- [ ] Record any skipped command with the exact reason and the narrower command used instead.

**Acceptance criteria:**
- No editor startup failures.
- No release top-level panel lacks a route/factory.
- No dev-only/debug surface appears in release navigation.
- Compat report diagnostics are bucketed or cleared correctly.
- Asset, install, and package smoke gates pass.
- Any remaining legal/public-release blockers are documented, not silently cleared.

**Verification command or manual test:**
- `ctest --preset dev-all -L pr --output-on-failure`
- `ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|editor app panels|urpg_editor_smoke|urpg_editor_list_panels|curated save-data lifecycle" --output-on-failure`
- `ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure`
- `.\tools\ci\check_release_required_assets.ps1`
- `.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke`
- `.\tools\ci\run_local_gates.ps1`

---

## Execution Order

1. P0-001
2. P0-002
3. P0-003
4. P1-001
5. P1-002
6. P1-003
7. P2-001
8. P2-002
9. P3-001
10. P3-002
11. P4-001
12. P4-002
13. P5-001
14. P5-002
15. P6-001
16. P6-002

## Self-Review

- Spec coverage: The four confirmed findings are covered in P0-001, P0-002, P0-003, P1-001, and P6-001. The broader audit areas are covered by P1 through P5 and final gates in P6.
- Placeholder scan: No task uses open-ended instructions without naming files, commands, and acceptance criteria.
- Type consistency: The plan uses existing names from the repo where known: `topLevelEditorPanels`, `requiredTopLevelPanelIds`, `smokeRequiredEditorPanelIds`, `CompatReportPanel::refresh`, `clearFailureDiagnostics`, and `clearExecutionDiagnostics`.
