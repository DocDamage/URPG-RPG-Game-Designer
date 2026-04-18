# Presentation Core - Work Log (WORKLOG.md)

## Current Status
**Phase:** 5
**Active Task:** Phase 5 presentation-polish milestone complete
**Cross-Cutting Governance:** cross-program debt, truthfulness, and intake-governance tracking lives in `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`

---

## Entries

### 2026-04-18 — Canonical Planning Linkage Sync
- **Action**: Updated `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` to absorb PGMMV/native-absorption planning governance, including canonical roadmap-alignment requirements and Definition-of-Done hooks.
- **Action**: Updated `docs/PROGRAM_COMPLETION_STATUS.md` and `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` to publish the canonical planning chain and identify standalone PGMMV/native-absorption files as planning annexes rather than parallel authorities.
- **Action**: Updated `README.md` documentation links so remediation, roadmap, status, and detailed planning annexes are distinguished clearly.
- **Action**: Re-labeled `URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`, `URPG_PGMMV_SUPPORT_PLAN.md`, and `URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md` as execution-detail/reference planning inputs that route back through the remediation hub.
- **Result**: The planning/status/remediation authority chain is now explicit across the repo instead of being split across parallel roadmap files.

### 2026-04-18 — AudioManager Runtime Closure and Documentation Sync (Workstream 2.3)
- **Action**: Added focused `AudioManager` regressions covering deterministic playback-position progression, duration-based duck/unduck ramps, applied master/bus volume scaling, and expanded QuickJS bridge routing.
- **Action**: Implemented deterministic playback-position advancement in `AudioChannel::update()` and replaced immediate duck/unduck snaps with frame-based volume ramps.
- **Action**: Applied master/bus volume changes to active BGM/BGS/ME/SE compat playback and separated source clip volume from effective mixed volume.
- **Action**: Expanded `AudioManager::registerAPI()` so the QuickJS-facing `AudioManager` object now routes live compat state for BGM/BGS/ME/SE control, current-BGM metadata, master/bus volume, and ducking helpers instead of leaving those paths stubbed or absent.
- **Action**: Reconciled `audio_manager.h/.cpp` comments, compat deviations, and linked docs so AudioManager now consistently describes a deterministic compat harness rather than a live mixer/backend.
- **Action**: Published `docs/COMPAT_EXIT_CHECKLIST.md` and updated remediation/status docs to point at the real checklist artifact.
- **Result**: `[audio_manager]` passes with expanded coverage, and the audio compat/doc truthfulness lane is materially more complete.

### 2026-04-18 — Battle Manager Runtime Debt Closure (Workstream 2.2)
- **Action**: Implemented `DataManager::gainExp()` with real EXP progression: looks up actor/class data, adds EXP, handles level-up with carry-over, clamps to `maxLevel`, and learns new skills from `skillsToLearn`.
- **Action**: Added minimal fields to data structures: `ActorData.exp`, `ActorData.skills`, `ClassData.maxLevel`, `ClassData.expTable`, `ClassData.skillsToLearn`, `SkillDamage`, `ItemDamage`, and `EffectData`.
- **Action**: Implemented `BattleManager::applySkill()` and `BattleManager::applyItem()` to resolve database records and apply damage/healing/state effects.
- **Action**: Updated `BattleManager::processAction()` to route SKILL and ITEM actions through the new `applySkill`/`applyItem` paths.
- **Action**: Upgraded compat status registry entries for `applySkill`, `applyItem`, and `applyExp` from `STUB`/`PARTIAL` to `PARTIAL` with honest deviation notes.
- **Action**: Added JS-friendly helpers (`isBattleTest`, `canLose`, `onEscapeSuccess`, `onEscapeFailure`, `changeBattleBackground`, `changeBattleBgm`, `changeVictoryMe`, `changeDefeatMe`, `isStateActive`, `processAction` overload, `queueActionByIndices`).
- **Action**: Rewired `BattleManager::registerAPI()` JS bindings for `setup`, `startBattle`, `abortBattle`, `endBattle`, `isBattleTest`, `canEscape`, `canLose`, `onEscapeSuccess`, `onEscapeFailure`, `changeBattleBackground`, `changeBattleBgm`, `changeVictoryMe`, `changeDefeatMe`, `getPhase`, `getTurnCount`, `processEscape`, `processAction`, `queueAction`, `getNextAction`, `clearActions`, `checkTurnCondition`, `forceAction`, `addState`, `removeState`, `isStateActive`.
- **Action**: Added focused regressions in `test_battlemgr.cpp` for `applySkill` damage/healing, `applyItem` healing, `applyExp` level-up, and JS bindings returning non-default values.
- **Action**: Added `gainExp` level-up regression in `test_data_manager.cpp`.
- **Result**: `urpg_core` builds cleanly; `[battlemgr]` and `[data_manager]` tests pass. Phase 2 Workstream 2.2 battle manager runtime debt is CLOSED.

### 2026-04-18 — Window Compat Rendering Debt Closure (Workstream 2.4)
- **Action**: Implemented `Window_Base::drawIcon()` with MZ icon-set atlas lookup (16 icons/row, 32×32, 512×512 sheet) emitting a `SpriteCommand` to the render layer with correct source UV coordinates.
- **Action**: Updated `Window_Base::drawGauge()` gradient limitation comment to explicitly document that `GradientRectCommand` is not yet available and color2 is pending.
- **Action**: Fixed `Window_Base::drawCharacter()` sprite-sheet math to use proper MZ standing-frame source rect: 4 columns × 2 rows of characters, 48×48 cells, with the standing frame at offset (+1, +0) within each character's 3×4 block.
- **Action**: Implemented `Window_Command::drawItem()` to call `drawText()` with the command name, computed item rect, and `normalColor()`/`dimColor()` based on enabled state. Added `normalColor()` and `dimColor()` helpers to `Window_Base`.
- **Action**: Reconciled method-status registry: upgraded `drawActorHp`, `drawActorMp`, `drawActorTp`, and `lineHeight` from `STUB` to `PARTIAL`.
- **Action**: Replaced stub JS bindings in `Window_Selectable::registerAPI()` and `Window_Command::registerAPI()` with real dispatch to instance methods for `index`, `select`, `topRow`, `maxItems`, `maxCols`, `itemHeight`, `itemWidth`, `cursorDown`, `cursorUp`, `cursorRight`, `cursorLeft`, `isCursorMovable`, `isHandled`, `processOk`, `processCancel`, `addCommand`, `clearCommandList`, `selectSymbol`, `findSymbol`, `ext`, and `makeCommandList`.
- **Action**: Added/extended unit tests for `drawIcon` render command, `drawCharacter` 48×48 sheet math, `Window_Command::drawItem` telemetry, status registry values, and JS binding non-nil returns.
- **Result**: `urpg_core` builds cleanly; `[compat][window]` tests pass. Phase 2 Workstream 2.4 window compat rendering debt is CLOSED.

### 2026-04-16 â€” Initialization
- **Action**: Created `PLAN.md` based on `urpg_first_class_presentation_architecture_plan_v2.md`.
- **Action**: Created directory structure for `docs/presentation/` and `docs/adr/`.
- **Action**: Created ADR skeletons (ADR-001 through ADR-010).
- **Action**: Created Scene-Family Contracts (Map, Battle, Menu, Overlay).
- **Action**: Scaffolded `engine/core/presentation/` directory and initial headers (`presentation_types.h`, `presentation_runtime.h`).
- **Action**: Initialized `RISKS.md` and `RELEASE_CHECKLIST.md`.
- **Action**: Initialized `docs/presentation/performance_budgets.md` with placeholder targets.
- **Result**: Phase 0 â€” Program Setup is COMPLETE.
- **Action**: Implemented `PresentationMode` resolver in `presentation_types.h`.
- **Action**: Created `presentation_schema.h` with `ProjectPresentationSettings` and `PresentationAuthoringData`.
- **Action**: Created `presentation_context.h` for runtime state tracking.
- **Action**: Updated `presentation_runtime.h` with `BuildPresentationFrame` contract and `PresentationFrameIntent`.
- **Action**: Added `DiagnosticSeverity` and `DiagnosticEntry` to `presentation_types.h`.
- **Action**: Added diagnostic collection methods to `PresentationRuntime`.
- **Action**: Created `capability_matrix.h` with `FeatureStatus` and `PresentationCapabilityMatrix`.
- **Action**: Created `fallback_router.h` with `PresentationFallbackRouter`.
- **Action**: Created `scene_translator.h` base contract.
- **Action**: Created `scene_adapters.h` with Map, Battle, Menu, and Overlay translator interfaces.
- **Action**: Created `presentation_camera.h` with `CameraProfile` schema and `PresentationCamera` skeleton.
- **Action**: Created `presentation_arena.h` for linear per-frame allocations (ADR-010).
- **Action**: Performed Task 22 profiling. Representative MapScene frame fits well within 2.0MB Tier 1 budget (~75KB used).
- **Result**: Phase 1 â€” Runtime Spine is COMPLETE.
- **Action**: Added `SpatialMapOverlay` and `ElevationGrid` schemas to `presentation_schema.h`.
- **Action**: Added `PropInstance` and `LightProfile` schemas.
- **Action**: Added `FogProfile`, `PostFXProfile`, and `MaterialResponseProfile` schemas.
- **Action**: Added `TierFallbackDeclaration` and `ActorPresentationProfile` schemas to `presentation_schema.h`.
- **Action**: Verified all Phase 2 schemas with `test_schema.cpp`.
- **Action**: Created `docs/presentation/schema_changelog.md` and recorded initial versions.
- **Result**: Phase 2 â€” Spatial Data Foundation is COMPLETE.
- **Action**: Created `map_scene_state.h` to define the input state for MapScene translation.
- **Action**: Implemented `MapSceneTranslatorImpl` in `map_scene_translator.h` providing the structure for elevation-aware actor anchoring and fallback routing.
- **Next**: Task 32 â€” Refining Actor anchoring math and depth policy.

### 2026-04-16 â€” Phase 3 & 4 Completion
- **Action**: Implemented `MapSceneTranslatorImpl` with actor anchoring logic and spatial position calculation via `ElevationGrid`.
- **Action**: Implemented `BattleCore` native integration with `BattleFormation` logic (Staged, Linear, Surround) in `BattleSceneTranslatorImpl`.
- **Action**: Expanded `GlobalStateHub` with "Diff-First" state updates to trigger events only on value change.
- **Action**: Implemented `StateDrivenAudioResolver` to automate BGM transitions based on map and battle state tags.
- **Action**: Created Editor tools: `ElevationBrushPanel` (with multi-tile interpolation) and `PropPlacementPanel` in `editor/spatial/`.
- **Action**: Finalized `SpatialMapOverlay` serialization (JSON To/From) in `PresentationMigrationTool`.
- **Action**: Implemented `MenuSceneTranslator` for UI background and Post-FX orchestration.
- **Action**: Integrated `PresentationRuntime::BuildPresentationFrame` spine for global environment and camera intent emission.
- **Action**: Added unit tests for state notifications and spatial editor data round-trips.
- **Result**: Phase 3 and Phase 4 â€” Core Integration is COMPLETE.

### 2026-04-16 â€” Phase 5 Bootstrap and Fixture Enablement
- **Action**: Completed Phase 5 bootstrap for environment intent emission by wiring dynamic light commands, fog, Post-FX, and shadow proxies into the map presentation path.
- **Action**: Added and wired `test_spatial_editor.cpp` plus targeted presentation/runtime assertions for Tier 1 environment behavior.
- **Action**: Added curated Hugging Face fixture ingestion for TMX, Visual Novel Maker, and Godot corpora, with manifest-driven refresh tooling under `tools/assets/`.
- **Action**: Integrated `third_party/huggingface/` into asset catalog, hygiene, and duplicate-planning workflows.
- **Action**: Added `test_huggingface_curated_assets.cpp` to keep the curated manifest and vendored fixture set aligned.
- **Action**: Synced `PLAN.md` with the latest GitHub plan update, including supporting enablement, AI-tooling follow-ons, and the asset reality check.
- **Result**: Task 41 is COMPLETE, Task 42 is IN PROGRESS, and Task 43 is the next active presentation task.

### 2026-04-16 â€” Task 43 Completion: Prop Gizmo Projection
- **Action**: Extended `PropPlacementPanel` with screen-to-world projection helpers for both simplified editor-view placement and camera-aware ray projection via `SpatialProjection`.
- **Action**: Added spatial editor coverage for center-screen projection, elevation sampling, out-of-bounds rejection, and perspective camera ray placement into authored elevation grids.
- **Action**: Fixed `test_spatial_editor.cpp` to use the existing `Vector2f` math type so the newer spatial projection tests compile and validate alongside the prop placement coverage.
- **Result**: Task 43 â€” Screen-to-World coordinate projection for Prop Gizmos is COMPLETE. Task 42 â€” Fog and Post-FX profile blending remains IN PROGRESS.

### 2026-04-16 â€” Task 42 Completion: Environment Profile Blending
- **Action**: Added weighted fog and Post-FX blending helpers to `PresentationRuntime` and a `ResolveEnvironmentCommands()` pass that collapses multiple environment commands into a single resolved fog command and a single resolved Post-FX command.
- **Action**: Extended `RenderBackendMock` pipeline state to surface resolved fog density, fog distances, exposure, bloom threshold, and saturation for verification.
- **Action**: Added unit coverage for weighted environment blending and dialogue readability/Post-FX override resolution in `tests/unit/test_spatial_editor.cpp`.
- **Result**: Task 42 â€” Fog and Post-FX profile blending is COMPLETE. Phase 5 presentation-polish tasks 41â€“43 are now COMPLETE.

### 2026-04-16 â€” Presentation Runtime Hardening Milestone
- **Action**: Removed the duplicate inline `PresentationRuntime::BuildPresentationFrame()` body from the header so runtime behavior now routes through the single compiled implementation in `presentation_runtime.cpp`.
- **Action**: Added `engine/core/presentation/presentation_runtime.cpp` to the `urpg_core` build target so runtime presentation behavior is exercised through the real build graph rather than a header-only fallback.
- **Action**: Added direct `BuildPresentationFrame()` assertions to `tests/unit/test_spatial_editor.cpp` and updated `release_validation.cpp` to count actor/environment commands in a way that matches the current Phase 5 environment pipeline.
- **Result**: Presentation runtime build-graph and validation hardening milestone is COMPLETE.

### 2026-04-16 â€” Dedicated Presentation Runtime Test Lane
- **Action**: Split runtime-focused presentation assertions out of `test_spatial_editor.cpp` into a dedicated `tests/unit/test_presentation_runtime.cpp`.
- **Action**: Added focused coverage for weighted environment blending, `PresentationRuntime::BuildPresentationFrame()`, dialogue readability/Post-FX override resolution, and `PresentationBridge` frame generation against an active scene.
- **Action**: Registered the new runtime test file in `CMakeLists.txt` and revalidated both the presentation-tagged lane and the editor/spatial lane independently.
- **Result**: Dedicated presentation runtime test-lane milestone is COMPLETE.

### 2026-04-16 â€” Presentation Release Validation Harness Wired
- **Action**: Added `urpg_presentation_release_validation` as a first-class CMake executable target backed by `engine/core/presentation/release_validation.cpp`.
- **Action**: Registered the harness as a named CTest entry with `nightly` and `presentation` labels.
- **Action**: Built and ran the executable directly, then verified the CTest entry succeeds end-to-end.
- **Result**: Presentation release-validation harness milestone is COMPLETE.

### 2026-04-16 â€” Presentation Focused Gate
- **Action**: Added a named `urpg_presentation_unit_lane` CTest entry that runs `urpg_tests "[presentation]"`.
- **Action**: Rebuilt the test graph and verified the focused presentation unit lane and the standalone release-validation harness run together as a presentation-specific gate.
- **Result**: Presentation focused-gate milestone is COMPLETE.

### 2026-04-16 â€” Presentation Validation Docs Alignment
- **Action**: Added `docs/presentation/VALIDATION.md` to document the focused presentation unit lane, release-validation harness, and combined presentation gate.
- **Action**: Updated `RELEASE_CHECKLIST.md` and `docs/PROGRAM_COMPLETION_STATUS.md` so release/readiness docs point at the same focused presentation validation commands now wired into CTest.
- **Result**: Presentation validation documentation milestone is COMPLETE.

### 2026-04-16 â€” Spatial Authoring Gate
- **Action**: Added `urpg_spatial_editor_lane` as a named CTest entry that runs `urpg_tests "[editor][spatial]"`.
- **Action**: Verified the focused presentation gate now runs runtime unit coverage, release validation, and spatial editor authoring coverage together.
- **Action**: Updated `docs/presentation/VALIDATION.md`, `RELEASE_CHECKLIST.md`, and `docs/PROGRAM_COMPLETION_STATUS.md` to reflect the expanded three-part presentation gate.
- **Result**: Spatial authoring gate milestone is COMPLETE.

### 2026-04-16 â€” One-Command Presentation Gate Runner
- **Action**: Added `tools/ci/run_presentation_gate.ps1` to build the presentation targets and execute the full focused presentation gate from one command.
- **Action**: Updated `docs/presentation/VALIDATION.md` and `RELEASE_CHECKLIST.md` to point at the helper script alongside the raw `ctest` command.
- **Action**: Ran the script successfully against `build-local` / `Debug` and verified all three presentation gate entries pass end-to-end.
- **Result**: One-command presentation gate runner milestone is COMPLETE.

### 2026-04-16 â€” Local Gates Integration for Presentation
- **Action**: Integrated the focused presentation gate into `tools/ci/run_local_gates.ps1` so broader local validation now has a subsystem-specific presentation check before the repo-wide labeled suites.

### 2026-04-17 â€” Migration Wizard Workspace Actions
- **Action**: Promoted migration wizard workflow actions up to `DiagnosticsWorkspace`, adding workspace-level forwarding for subsystem selection, next/previous navigation, re-run, selective clear, report export, and report save/load.
- **Action**: Added focused integration coverage in `tests/unit/test_diagnostics_workspace.cpp` so the top-level diagnostics surface now proves it can drive the wizard workflow without reaching through panel internals.
- **Result**: Migration wizard workspace-actions milestone is COMPLETE. The diagnostics workspace now exposes the richer wizard workflow state it already exports.

### 2026-04-17 â€” Migration Wizard Snapshot Coherence
- **Action**: Taught `DiagnosticsWorkspace` to refresh the migration wizard render snapshot immediately after workspace-driven wizard actions when the wizard tab is the active visible diagnostics surface.
- **Action**: Added focused regression coverage proving that selection, rerun, selective clear, and full clear all update `exportAsJson()` without requiring a separate manual `render()` call.
- **Result**: Migration wizard snapshot-coherence milestone is COMPLETE. Workspace-owned wizard actions now keep the exported diagnostics state truthful in the same interaction step.

### 2026-04-17 â€” Migration Wizard Failed-Load Truthfulness
- **Action**: Fixed `DiagnosticsWorkspace::loadMigrationWizardReportFromFile()` to refresh the active wizard snapshot after failed loads as well as successful ones, matching the model behavior that clears itself on invalid report input.
- **Action**: Added focused integration coverage proving that a bad workspace-level wizard report import immediately clears the exported diagnostics snapshot instead of leaving stale migration state behind.
- **Result**: Migration wizard failed-load truthfulness milestone is COMPLETE. Workspace export now stays aligned with the wizard model after invalid import attempts.

### 2026-04-17 â€” Migration Wizard Workspace File Round-Trip Coverage
- **Action**: Added focused `DiagnosticsWorkspace` integration coverage for the happy-path wizard report save/load round-trip, including selected-subsystem preservation and exported report JSON equality after reload.
- **Result**: Migration wizard workspace file round-trip coverage is COMPLETE. The top-level diagnostics surface now has both success-path and failure-path regression anchors for wizard report I/O.

### 2026-04-17 â€” Audio Snapshot Coherence
- **Action**: Taught `DiagnosticsWorkspace` to refresh the audio inspector render snapshot immediately after workspace-driven audio bind/clear actions when the audio tab is the active visible diagnostics surface.
- **Action**: Added focused regression coverage proving that active-tab audio exports update immediately after clear and rebind without requiring a separate manual `render()` call.
- **Result**: Audio snapshot-coherence milestone is COMPLETE. Workspace-owned audio runtime actions now keep exported diagnostics state truthful in the same interaction step.

### 2026-04-17 â€” Event Authority Snapshot Coherence
- **Action**: Taught `DiagnosticsWorkspace` to refresh the event-authority panel snapshot immediately after workspace-driven diagnostics ingest/clear actions when the event-authority tab is the active visible diagnostics surface.
- **Action**: Added the missing `has_data` field to the event-authority active-tab export payload so workspace JSON reflects the same truthfulness signal already present in the panel snapshot.
- **Action**: Added focused regression coverage proving that active-tab event-authority exports update immediately after ingest and clear without requiring a separate manual `render()` call.
- **Result**: Event-authority snapshot-coherence milestone is COMPLETE. Workspace-owned event diagnostics actions now keep exported state truthful in the same interaction step.

### 2026-04-17 â€” Snapshot-Backed Tab Activation Truthfulness
- **Action**: Added a workspace-level helper that refreshes snapshot-backed tabs (`event_authority`, `audio`, `migration_wizard`) immediately when they become the active visible tab.
- **Action**: Updated `setActiveTab()` and `setVisible()` to invoke that helper, so switching onto a snapshot-backed tab no longer requires a later manual `render()` before `exportAsJson()` becomes truthful.
- **Action**: Added focused integration coverage proving audio tab activation now exports fresh detail immediately after a tab switch.
- **Result**: Snapshot-backed tab activation truthfulness is COMPLETE. Diagnostics workspace exports now stay honest across both workspace actions and tab activation for the snapshot-backed tabs currently in use.

### 2026-04-17 â€” Hidden-to-Visible Snapshot Coverage
- **Action**: Added focused integration coverage proving that a hidden active snapshot-backed tab refreshes its exported detail immediately when the workspace becomes visible again, without requiring a separate manual `render()` call.
- **Result**: Hidden-to-visible snapshot coverage is COMPLETE. The generalized activation helper now has an explicit regression anchor for both tab switches and visibility restoration.

### 2026-04-17 â€” Event Authority Workspace Actions
- **Action**: Promoted event-authority workflow actions up to `DiagnosticsWorkspace`, adding workspace-level filtering, clear-filters, row selection, and next/previous row navigation instead of requiring callers to reach through the panel directly.
- **Action**: Split event-authority snapshot updates into a refresh+render path for data/filter changes and a render-only path for selection/navigation so workspace-owned row selection preserves the userâ€™s current selection instead of resetting it.
- **Action**: Added focused integration coverage proving the top-level diagnostics surface can drive event-authority filters and row navigation while keeping exported state truthful.
- **Result**: Event-authority workspace-actions milestone is COMPLETE. The diagnostics workspace now exposes a first-class event-authority workflow surface rather than only a panel escape hatch.

### 2026-04-17 â€” Message Inspector Workspace Actions
- **Action**: Promoted message-inspector workflow actions up to `DiagnosticsWorkspace`, adding workspace-level route filtering, â€œissues onlyâ€ toggling, filter clearing, and visible-row selection.
- **Action**: Updated `MessageInspectorModel` to preserve selected page id across filter and refresh rebuilds when the selected page remains visible, instead of always clearing selection on every filter change.
- **Action**: Added focused integration coverage proving the top-level diagnostics surface can drive message filtering and page selection while keeping the exported `selected_page_id` truthful.
- **Result**: Message-inspector workspace-actions milestone is COMPLETE. The diagnostics workspace now exposes a first-class message-inspector workflow surface, and message selection is more stable through normal filtering changes.
- **Action**: Added an explicit `PresentationConfiguration` parameter and `SkipPresentationGate` escape hatch to keep the local gate runner configurable.
- **Action**: Updated `docs/presentation/VALIDATION.md` and `docs/DEVELOPMENT_KICKOFF.md` to reflect the new relationship between `run_presentation_gate.ps1` and `run_local_gates.ps1`.
- **Action**: Performed a parse-level sanity check on `run_local_gates.ps1` after the integration change.
- **Result**: Presentation gate integration into the broader local-gates workflow is COMPLETE.

### 2026-04-16 â€” Presentation Gate CI Integration
- **Action**: Added an explicit `Run focused presentation gate` step to `.github/workflows/ci-gates.yml` inside the `gate1-pr` job, reusing `tools/ci/run_presentation_gate.ps1` against `build/ci` / `Release`.
- **Action**: Updated `docs/presentation/VALIDATION.md` and `docs/PROGRAM_COMPLETION_STATUS.md` to reflect that the focused presentation gate now runs in CI on PR/push/workflow-dispatch paths.
- **Action**: Performed a structural sanity check on the edited `ci-gates.yml` step block to confirm the new workflow insertion is in the expected location after the shared build step.
- **Result**: Presentation gate CI-integration milestone is COMPLETE.

### 2026-04-16 â€” Presentation Gate README Surfacing
- **Action**: Added a focused presentation validation section to `README.md` with both the helper-script command and the direct combined CTest gate command.
- **Action**: Added a top-level documentation link from `README.md` to `docs/presentation/VALIDATION.md`.
- **Result**: Presentation gate onboarding/discoverability milestone is COMPLETE.

### 2026-04-16 â€” Contributor Workflow Alignment for Presentation
- **Action**: Updated `CONTRIBUTING.md` so presentation runtime, spatial editor, and presentation-facing rendering changes explicitly call for `tools/ci/run_presentation_gate.ps1` before opening a PR.
- **Result**: Presentation gate contributor-workflow milestone is COMPLETE.

### 2026-04-16 â€” PR Review Surface for Presentation Validation
- **Action**: Added `.github/PULL_REQUEST_TEMPLATE.md` with a verification checklist that explicitly calls for `tools/ci/run_presentation_gate.ps1` when a PR touches presentation runtime, spatial editor, or presentation-facing rendering behavior.
- **Action**: Updated `CONTRIBUTING.md` and `README.md` to point contributors at the PR template for verification reporting.
- **Result**: Presentation gate PR-review milestone is COMPLETE.

### 2026-04-16 â€” Presentation Docs Hub
- **Action**: Added `docs/presentation/README.md` as a single index for presentation validation, spatial tooling, budgets, schema docs, and scene contracts.
- **Action**: Updated `README.md` to link to the new presentation docs hub from the top-level documentation section.
- **Result**: Presentation documentation hub milestone is COMPLETE.

### 2026-04-16 â€” Presentation Contract Index
- **Action**: Added `docs/presentation/test_matrix/README.md` as a dedicated landing page for presentation scene-family contracts.
- **Action**: Linked the new contract index from `docs/presentation/README.md` so the presentation docs now have both a subsystem hub and a contracts hub.
- **Result**: Presentation contract-index milestone is COMPLETE.

### 2026-04-16 â€” Presentation Docs Integrity Check
- **Action**: Added `tools/docs/check-presentation-doc-links.ps1` to validate local Markdown links in the main presentation docs hub, validation guide, and contracts index.
- **Action**: Ran the checker successfully and added a pointer to it from `docs/presentation/README.md`.
- **Result**: Presentation docs-integrity milestone is COMPLETE.

### 2026-04-16 â€” Presentation Docs Check in Gates
- **Action**: Integrated `tools/docs/check-presentation-doc-links.ps1` into `tools/ci/run_local_gates.ps1`.
- **Action**: Added the same docs-link validation step to `.github/workflows/ci-gates.yml` inside `gate1-pr`.
- **Action**: Updated `docs/presentation/VALIDATION.md` and `CONTRIBUTING.md` to reflect that presentation docs integrity is now part of the standard gate path.
- **Action**: Re-ran the presentation docs checker successfully after the workflow updates.
- **Result**: Presentation docs-check gating milestone is COMPLETE.

### 2026-04-16 â€” Self-Contained Presentation Gate
- **Action**: Integrated `tools/docs/check-presentation-doc-links.ps1` directly into `tools/ci/run_presentation_gate.ps1` so the focused presentation gate validates its own documentation surfaces without relying on broader wrappers.
- **Action**: Updated `docs/presentation/VALIDATION.md` to describe the focused gate as self-contained.
- **Action**: Re-ran `tools/ci/run_presentation_gate.ps1 -SkipBuild` successfully and verified docs-link validation plus all three presentation test entries pass together.
- **Result**: Self-contained presentation gate milestone is COMPLETE.

### 2026-04-16 â€” Presentation Docs Tooling Surfaced
- **Action**: Updated `tools/docs/README.md` to document `check-presentation-doc-links.ps1`, its scope, where it runs, and the policy for handling presentation-doc link drift.
- **Result**: Presentation docs-tooling discoverability milestone is COMPLETE.

### 2026-04-16 â€” Menu Diagnostics Workspace Integration
- **Action**: Added a diagnostics workspace integration test that binds menu runtime data, verifies menu-tab visibility/state export, and proves menu diagnostics clear cleanly.
- **Action**: Added `MenuInspectorModel::Clear()` and wired `DiagnosticsWorkspace::clearMenuRuntime()` to reset stale menu inspector state instead of leaving previous runtime data resident.
- **Action**: Revalidated the focused diagnostics integration lane and the menu inspector model lane after the runtime clear-path fix.
- **Result**: UI/Menu diagnostics-workspace integration milestone is COMPLETE.

### 2026-04-16 â€” Menu Legacy Import Mapping Enriched
- **Action**: Extended `MenuSceneSerializer::ImportLegacy()` to prefer explicit `mainMenu.commands` metadata when present, carrying native route targets, fallback routes, custom route IDs, and command-state rules into the imported graph.
- **Action**: Normalized legacy route parsing so rich menu import accepts the native lower-case route identifiers already used in schema-backed menu data.
- **Action**: Added legacy import coverage that asserts fallback-route and visibility/enable rule preservation, then revalidated the broader `[ui][menu]` suite.
- **Result**: UI/Menu legacy-import mapping milestone is COMPLETE.

### 2026-04-16 â€” Menu Round-Trip Serialization and Preview Workflow
- **Action**: Added `MenuSceneGraph::getRegisteredScenes()` and implemented `MenuSceneSerializer::Serialize()` so native menu graphs can now emit a non-empty serialized scene definition.
- **Action**: Added round-trip coverage that serializes a native menu graph, deserializes it, and asserts the restored scene/pane/route structure matches the authored source.
- **Action**: Surfaced `MenuPreviewPanel` through `DiagnosticsWorkspace` so the menu diagnostics tab now carries both the inspector and preview workflow surfaces together.
- **Action**: Revalidated the focused menu diagnostics integration lane and the full `[ui][menu]` suite after the serializer and preview-workflow changes.
- **Result**: UI/Menu round-trip serialization and preview-workflow milestone is COMPLETE.

### 2026-04-16 â€” Plugin API Stub Truthfulness Pass
- **Action**: Updated `engine/core/editor/plugin_api.h` section comments so entity, global-state, and input exports are explicitly labeled as `STUB` or scratch-state bridges rather than implied live engine integrations.
- **Action**: Updated `engine/core/editor/plugin_api.cpp` inline comments so the disconnected routing behavior is called out at each stubbed export site.
- **Result**: Plugin API public-surface truthfulness milestone is COMPLETE.

### 2026-04-16 â€” Cloud Sync Documentation Truthfulness Pass
- **Action**: Updated `docs/AI_COPILOT_GUIDE.md` so `AISyncCoordinator` and cloud-sync guidance are described as stub-backed plumbing rather than an operational cross-device persistence path.
- **Action**: Updated `docs/AI_SUBSYSTEM_CLOSURE_CHECKLIST.md` with an explicit note that the in-tree `CloudServiceStub` only covers local in-memory plumbing and does not constitute production cloud sync.
- **Action**: Re-checked repository language for cloud-sync claims to confirm the remaining in-tree references now consistently describe stub-backed behavior.
- **Result**: Cloud sync documentation truthfulness milestone is COMPLETE.

### 2026-04-16 â€” Async Plugin Callback Threading Contract Surfaced
- **Action**: Updated `runtimes/compat_js/plugin_manager.h` so `executeCommandAsync()` explicitly documents that callbacks currently run on the worker thread and are not safe to treat as main-thread UI/editor/gameplay callbacks.
- **Action**: Added matching inline contract commentary at the async callback invocation site in `runtimes/compat_js/plugin_manager.cpp`.
- **Result**: Async plugin callback threading-contract documentation milestone is COMPLETE.

### 2026-04-16 â€” Async Plugin Callback Main-Thread Dispatch
- **Action**: Added `PluginManager::dispatchPendingAsyncCallbacks()` and a completed-callback queue so async command execution still happens on the worker thread while callback delivery is deferred to an explicit caller-thread drain point.
- **Action**: Updated async plugin manager tests to prove callbacks do not fire before dispatch, then execute in FIFO order once the caller drains the pending callback queue.
- **Action**: Updated the compat-status deviation text and public API comments so the async execution contract now matches the implemented marshalling path.
- **Result**: Async plugin callback main-thread dispatch milestone is COMPLETE.

### 2026-04-16 â€” MapScene Audio Service Injection
- **Action**: Replaced `MapScene::processAiAudioCommands()` static-local `AudioCore` usage with a scene-owned audio service reference that can be overridden through `MapScene::setAudioCore()`.
- **Action**: Added read-only `AudioCore` inspection accessors for current BGM and active-source count so scene/audio integration can be verified without reaching into private state.
- **Action**: Added `MapScene` coverage proving AI audio commands drive the injected audio service and revalidated the broader `[scene]` and `[plugin_manager]` lanes afterward.
- **Result**: P3-01 audio-service injection milestone is COMPLETE. The render-layer dirty-flag/incremental rebuild follow-on was closed later the same day.

### 2026-04-16 â€” MapScene Render-Layer Dirty-Flag Cache
- **Action**: Added a retained tile-command cache to `MapScene` so unchanged updates reuse existing tile render commands instead of rebuilding tile command objects every frame.
- **Action**: Marked the retained tile cache dirty from `setTile()` and `setTilePassable()` so authored map edits rebuild the cached tile command set on the next update.
- **Action**: Added scene coverage that proves unchanged frames keep the same tile render commands while a tile edit forces a rebuilt command with the updated tile index.
- **Action**: Revalidated the broader `[scene]` lane after the render-cache change.
- **Result**: P3-01 render-layer dirty-flag/incremental rebuild milestone is COMPLETE.

### 2026-04-16 â€” AssetLoader Negative Cache for Missing Textures
- **Action**: Added a missing-texture negative cache to `AssetLoader` so repeated requests for the same absent texture path return `nullptr` without logging the same warning over and over.
- **Action**: Added asset coverage that captures `std::cerr` and proves a repeated missing-path lookup only emits a single loader warning.
- **Action**: Revalidated the focused `[assets]` lane after the loader change.
- **Result**: Missing-texture warning-spam reduction milestone is COMPLETE.

### 2026-04-16 â€” Compat Report Duplicate Test Cleanup
- **Action**: Removed the stale unregistered duplicate file `tests/unit/test_compat_reportPanel.cpp`, leaving `tests/unit/test_compat_report_panel.cpp` as the single active compat-report panel test surface referenced by `CMakeLists.txt`.
- **Action**: Rechecked repo references so remediation/docs now point at a resolved duplicate-test cleanup instead of an active ambiguity in the tree.
- **Result**: P2-04 stale duplicate compat-report test cleanup milestone is COMPLETE.

### 2026-04-16 â€” BattleScene Optional Default Battleback
- **Action**: Made the placeholder default battleback load in `BattleScene::onStart()` optional so headless/unit-test runs no longer ask `AssetLoader` to log a missing `Grassland.png` placeholder.
- **Action**: Added battle-scene coverage that captures `std::cerr` and proves startup stays quiet when the default battleback asset is absent.
- **Action**: Revalidated the focused `[battle][scene]` and broader `[scene]` lanes after the change.
- **Result**: BattleScene startup warning cleanup milestone is COMPLETE.

### 2026-04-16 â€” Diagnostics Workspace 9-Tab Export Alignment
- **Action**: Expanded `DiagnosticsWorkspace::allTabSummaries()` so the exported workspace shape now includes the existing `audio` and `migration_wizard` tabs alongside the previously surfaced seven-tab set.
- **Action**: Updated diagnostics integration coverage to assert the full 9-tab serialized workspace order (`compat`, `save`, `event_authority`, `message_text`, `battle`, `menu`, `audio`, `migration_wizard`, `abilities`).
- **Action**: Revalidated the focused diagnostics integration lane after a clean rebuild to ensure the exported workspace shape matches the declared tab model.
- **Result**: Diagnostics workspace export-shape alignment milestone is COMPLETE.

### 2026-04-16 â€” AudioManager SE Channel Lifetime Fix
- **Action**: Added a focused audio-manager regression that proves a one-shot SE channel is reclaimed after playback completes and `update()` runs, without requiring an explicit `stopSe()` call.
- **Action**: Implemented deterministic one-shot SE completion inside `AudioChannel::update()` so compat audio cleanup can reclaim SE channels automatically.
- **Action**: Revalidated the focused `AudioManager: SE channels are reclaimed after playback completion` regression and the broader `[audio_manager]` lane.
- **Result**: P1-03 audio SE channel lifetime leak milestone is COMPLETE.

### 2026-04-16 â€” BattleManager Turn-Condition Cadence Fix
- **Action**: Reworked `BattleManager::checkTurnCondition()` so exact-turn checks stay exclusive to `span == 0`, negative spans fail fast, and positive spans use threshold-gated modulo cadence instead of the previous incorrect special cases.
- **Action**: Added a focused battle-manager regression that walks turn progression and proves exact-turn, every-turn-after-threshold, and every-other-turn cadence behavior.
- **Action**: Revalidated both the named cadence regression and the broader `[battlemgr]` lane after rebuilding `urpg_tests`.
- **Result**: P1-04 battle turn-condition correctness milestone is COMPLETE.

### 2026-04-16 â€” Diagnostics Audio/Migration Tab Render Reachability
- **Action**: Restored `DiagnosticsWorkspace::render()` coverage for the `audio` and `migration_wizard` tabs so those exported tabs now participate in the active-tab render path instead of being commented out.
- **Action**: Added lightweight render snapshots to `AudioInspectorPanel` and `MigrationWizardPanel` so workspace-level tests can prove those panels rendered when visible.
- **Action**: Extended diagnostics, audio-inspector, and migration-wizard coverage to activate both tabs through the workspace and assert their rendered snapshots.
- **Result**: P2-01 is PARTIALLY remediated: `audio` and `migration_wizard` now render honestly, while `event_authority` remains the open tab-body gap.

### 2026-04-16 â€” Diagnostics Runtime Clear/Rebind Fix
- **Action**: Implemented real `clearAudioRuntime()` and `clearAbilityRuntime()` reset paths so diagnostics detach clears projected runtime state instead of leaving stale audio/ability summaries behind.
- **Action**: Tightened the audio inspector model to retain projected `AudioCore` active-source count and master volume, making audio runtime binding truthfully observable and resettable.
- **Action**: Added a focused diagnostics workspace regression that clears and rebinds audio and ability runtimes, proving summaries and projected tag/ability state reset cleanly between bindings.
- **Result**: P2-02 diagnostics runtime stale-state milestone is COMPLETE.

### 2026-04-17 â€” Event Authority Diagnostics Render Reachability
- **Action**: Added lightweight render snapshots to `EventAuthorityPanel` so the panel now records visible-row counts, severity counts, and selection presence when rendered while visible.
- **Action**: Extended `test_event_authority_panel.cpp` with a focused visible-render regression and updated diagnostics workspace integration coverage to assert the `event_authority` tab really renders when active.
- **Action**: Revalidated the focused `[events][panel]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-01 diagnostics workspace truthfulness milestone is COMPLETE. Remaining audio/migration/event-authority productization work stays tracked under P2-03.

### 2026-04-17 â€” Audio Inspector Live Runtime Projection
- **Action**: Added read-only active-source snapshots to `AudioCore` so editor diagnostics can inspect live handle, asset-id, category, and channel-state data without reaching into mixer internals.
- **Action**: Reworked `AudioInspectorModel` to project real `AudioCore` active sources into `AudioHandleRow` entries instead of leaving the row list empty while only reporting aggregate counts.
- **Action**: Upgraded `test_audio_inspector.cpp` to assert live row projection and revalidated the focused `[editor][audio]` and `[editor][diagnostics]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 audio inspector fidelity milestone is COMPLETE. Migration wizard productization remains the primary open diagnostics-scaffolding follow-on.

### 2026-04-17 â€” Migration Wizard Subsystem Execution Reporting
- **Action**: Extended `MigrationWizardModel` to run message migration alongside the existing menu and battle passes, accumulating per-subsystem summary logs instead of only emitting a generic completion message.
- **Action**: Expanded `MigrationWizardPanel` render snapshots to surface summary-log count and a headline so workspace-level tests can verify which migration run completed.
- **Action**: Upgraded `test_migration_wizard.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the focused `[editor][diagnostics][wizard]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard execution-reporting milestone is COMPLETE. Richer wizard workflow/productization remains the next follow-on.

### 2026-04-17 â€” Migration Wizard Rendered Summary Text Surfaced
- **Action**: Extended `MigrationWizardPanel::RenderSnapshot` to carry the rendered summary-log lines themselves, not just the log count and headline.
- **Action**: Tightened the migration wizard and diagnostics workspace coverage to assert the rendered `Menu migration ...` summary text is present in the active wizard snapshot.
- **Action**: Revalidated the focused `[editor][diagnostics][wizard]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard rendered-summary milestone is COMPLETE. The remaining open work is deeper wizard interaction/product workflow rather than snapshot truthfulness.

### 2026-04-17 â€” Migration Wizard Reset Path
- **Action**: Added `MigrationWizardModel::clear()` and `MigrationWizardPanel::clear()` so the wizard can reset both its accumulated report and its rendered snapshot back to an empty state.
- **Action**: Surfaced that reset through `DiagnosticsWorkspace::clearMigrationWizardRuntime()` so the top-level diagnostics workspace can clear wizard state consistently with other panel-backed runtimes.
- **Action**: Added focused reset regressions in `test_migration_wizard.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the `[editor][diagnostics][wizard]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard resettable-state milestone is COMPLETE. Remaining work in this lane is now deeper interaction/workflow productization rather than reporting or reset truthfulness.

### 2026-04-17 â€” Migration Wizard Structured Subsystem Results
- **Action**: Added typed per-subsystem migration results to `MigrationWizardModel::ProgressReport` for the message, menu, and battle passes, including processed-count and warning/error totals.
- **Action**: Threaded those structured subsystem results through `MigrationWizardPanel::RenderSnapshot` so future wizard UI/productization can consume typed state instead of parsing summary strings.
- **Action**: Extended `test_migration_wizard.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the `[editor][diagnostics][wizard]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard structured-results milestone is COMPLETE. The remaining follow-on is richer interaction/workflow UI built on top of this now-typed wizard state.

### 2026-04-17 â€” Migration Wizard Subsystem Selection State
- **Action**: Added subsystem selection APIs to `MigrationWizardModel` so the wizard can select a typed subsystem result and resolve the current selected row back into structured state.
- **Action**: Threaded the selected subsystem id through `MigrationWizardPanel::RenderSnapshot`, giving follow-on wizard UI a concrete interaction state to render.
- **Action**: Extended `test_migration_wizard.cpp` and revalidated the `[editor][diagnostics][wizard]` plus `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard selection-state milestone is COMPLETE. The next follow-on is richer workflow UI built on top of the now-typed, selectable wizard state.

### 2026-04-17 â€” Migration Wizard Default Selection and Detail Snapshot
- **Action**: Updated `MigrationWizardModel` so a completed migration run auto-selects the first available subsystem result instead of leaving selection empty after successful work.
- **Action**: Expanded `MigrationWizardPanel::RenderSnapshot` with selected-subsystem detail fields (`display_name`, `processed_count`) so a follow-on UI can render meaningful details immediately from the selected row.
- **Action**: Extended `test_migration_wizard.cpp` and revalidated the `[editor][diagnostics][wizard]` plus `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard selected-detail milestone is COMPLETE. The next follow-on is fuller wizard workflow UI built on top of the now-typed, selectable, detail-bearing state.

### 2026-04-17 â€” Migration Wizard Selected Status Snapshot
- **Action**: Expanded `MigrationWizardPanel::RenderSnapshot` so the currently selected subsystem now carries warning-count, error-count, and completion-state fields alongside its identity and processed-count details.
- **Action**: Tightened `test_migration_wizard.cpp` around the selected subsystem status payload and revalidated the `[editor][diagnostics][wizard]` plus `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard selected-status milestone is COMPLETE. The next follow-on is fuller workflow UI built on top of the now-typed, selectable, detail-and-status-bearing wizard state.

### 2026-04-17 â€” Migration Wizard Typed Summary Lines
- **Action**: Added a `summary_line` field directly to each typed `MigrationWizardModel::SubsystemResult`, so subsystem summaries now travel with the structured result instead of living only in the loose `summary_logs` list.
- **Action**: Expanded `MigrationWizardPanel::RenderSnapshot` with the selected subsystem's own `summary_line`, so follow-on UI can render the selected summary directly from selected-state snapshot data.
- **Action**: Tightened `test_migration_wizard.cpp` around typed subsystem summaries and revalidated the `[editor][diagnostics][wizard]` plus `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: P2-03 migration wizard typed-summary milestone is COMPLETE. The next follow-on is fuller workflow UI built on top of the now-typed, selectable, detail-, status-, and summary-bearing wizard state.

### 2026-04-17 â€” Event Authority Snapshot Context and Wizard Result Navigation
- **Action**: Expanded `EventAuthorityPanel` render snapshots with selection, filter, and navigation state so the tab now preserves its current browsing context instead of only severity counts.
- **Action**: Threaded event-authority selection/filter state and richer active-tab detail through diagnostics workspace coverage, and extended audio workspace assertions to prove live handle rows survive through the rendered panel snapshot.
- **Action**: Added next/previous subsystem-result navigation to `MigrationWizardModel` and `MigrationWizardPanel`, including explicit `can_select_next_subsystem` / `can_select_previous_subsystem` snapshot state and focused wizard navigation regressions.
- **Result**: Phase 3 diagnostics productization advanced again: event-authority is more actionable at both panel and workspace levels, audio snapshot fidelity is asserted through the workspace, and migration wizard workflow now supports ordered result navigation.

### 2026-04-17 â€” Event Authority Row Navigation and Selected-Row Detail
- **Action**: Added next/previous row navigation to `EventAuthorityPanelModel` and `EventAuthorityPanel`, including explicit `canSelectNextRow()` / `canSelectPreviousRow()` behavior for filtered result sets.
- **Action**: Expanded `EventAuthorityPanel::RenderSnapshot` and the `DiagnosticsWorkspace` event-authority export detail with the selected row's full detail payload (`event_id`, `block_id`, mode, operation, error code, message, summary) plus row-navigation state.
- **Action**: Tightened `test_event_authority_panel.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the focused `[events][panel]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: Event-authority diagnostics now behave more like a browsable workflow surface rather than a count-only selection snapshot.

### 2026-04-17 â€” Event Authority Visible-Row Body Surfaced
- **Action**: Expanded `EventAuthorityPanel::RenderSnapshot` with the visible projected row entries themselves so the panel snapshot now carries a real row body, not just counts and selected-row metadata.
- **Action**: Threaded those visible row entries through `DiagnosticsWorkspace::exportAsJson()` under the `event_authority` active-tab detail payload.
- **Action**: Tightened `test_event_authority_panel.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the focused `[events][panel]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: Event-authority diagnostics now expose a fuller panel body for downstream UI/workflow consumers instead of only summary metadata.

### 2026-04-17 â€” Event Authority Severity Filtering
- **Action**: Added severity-level filtering (`warn` / `error`) to `EventAuthorityPanelModel` and `EventAuthorityPanel`, alongside the existing event-id filter.
- **Action**: Surfaced the active severity filter through `EventAuthorityPanel::RenderSnapshot` and the `DiagnosticsWorkspace` event-authority export detail payload.
- **Action**: Tightened `test_event_authority_panel.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the focused `[events][panel]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: Event-authority diagnostics now support a more practical browse-and-narrow workflow instead of forcing consumers to inspect the full mixed-severity row set.

### 2026-04-17 â€” Event Authority Mode Filtering
- **Action**: Added mode filtering (`compat` / `mixed`) to `EventAuthorityPanelModel` and `EventAuthorityPanel`, alongside the existing event-id and severity filters.
- **Action**: Surfaced the active mode filter through `EventAuthorityPanel::RenderSnapshot` and the `DiagnosticsWorkspace` event-authority export detail payload.
- **Action**: Tightened `test_event_authority_panel.cpp` and `test_diagnostics_workspace.cpp`, then revalidated the focused `[events][panel]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: Event-authority diagnostics now support a fuller browse-and-narrow workflow across id, severity, and mode rather than a single mixed row set.

### 2026-04-17 â€” Agent Swarm Remediation Passes
- **Action**: First agent swarm pass closed:
  - Compat status honesty downgraded inflated `FULL` claims to `PARTIAL`/`STUB` in `audio_manager.cpp`, `battle_manager.cpp`, `data_manager.cpp`, and `window_compat.cpp`.
  - Verified QuickJS scope is explicitly documented as a fixture-backed compat harness.
  - Fixed test/build registration drift: removed duplicate `test_engine_shell.cpp`, registered missing `test_menu_orchestration.cpp`, added SE channel-growth regression test, fixed scene-stack navigation test.
  - Established external-repo intake governance (`docs/external-intake/`) and private-use asset intake governance (`docs/asset_intake/`) with scaffolded directory structure.
  - Aligned documentation truth for presentation/spatial build graph and cloud-sync/plugin scaffolding claims.
- **Action**: Second agent swarm pass closed:
  - Downgraded all 79 inflated `FULL` labels to `PARTIAL` in `input_manager.cpp`.
  - Deepened migration wizard productization with `rerunSubsystem(id, project_data)` action, panel snapshot exposure, and `bindMigrationWizardRuntime()` wiring.
  - Implemented real `loadDatabase()` orchestration in `data_manager.cpp` with seeded records; wired up all stubbed JS bindings; implemented real `get*AsValue()` serializers.
  - Synchronized intake governance artifacts into `URPG_repo_intake_plan.md`, `URPG_private_asset_intake_plan.md`, `TECHNICAL_DEBT_REMEDIATION_PLAN.md`, and `PROGRAM_COMPLETION_STATUS.md`.
- **Result**: `urpg_core` builds cleanly; `urpg_tests` passes 400 test cases (5,098 assertions).

### 2026-04-17 Ã¢â‚¬â€ Menu Diagnostics Workspace Actions
- **Action**: Promoted menu-inspector workflow actions up to `DiagnosticsWorkspace`, adding workspace-level command-id filtering, filter clearing, â€œissues onlyâ€ toggling, and row selection instead of requiring callers to reach through the menu model directly.
- **Action**: Updated `MenuInspectorModel` to preserve the selected command across filter rebuilds when that command remains visible, so workspace-driven narrowing no longer drops selection unnecessarily.
- **Action**: Expanded `DiagnosticsWorkspace::exportAsJson()` for the `menu` tab to include `command_id_filter`, `show_issues_only`, and a structured `selected_row` payload alongside the existing summary, visible rows, issues, and preview state.
- **Result**: Menu diagnostics now expose a first-class workspace workflow surface rather than only passive export data plus placeholder panel chrome.

### 2026-04-17 - Local Toolchain Hardening
- **Action**: Hardened SDL resolution in `CMakeLists.txt` so MinGW only accepts SDL2 from the active compiler root, falls back to vendored SDL when that package is missing, and disables the broken vendored Windows joystick/haptic stack on MinGW header sets that already define `XINPUT_CAPABILITIES_EX`.
- **Action**: Hardened the MSVC lane to skip host `find_package(SDL2)` discovery entirely, preventing MSYS2 MinGW headers from leaking into Visual Studio projects through an incompatible SDL package.
- **Action**: Added missing standard-library includes surfaced by stricter toolchains (`<cstdint>` in frame/plugin headers and `<cmath>` in `AISystem`), and restored the missing test support headers `engine/core/testing/headless_play_mode.h` and `engine/core/testing/snapshot_validator.h`.
- **Action**: Hardened `tools/ci/run_presentation_gate.ps1` so it reconfigures stale local build trees when the cached generator no longer matches the selected local profile.
- **Result**: `urpg_core` now builds in both `dev-mingw-debug` and `dev-vs2022`, `urpg_tests` builds in both lanes, and the focused presentation gate passes locally on the Visual Studio profile without MSYS header contamination.

### 2026-04-17 â€” Menu Diagnostics Panel Snapshot and Hidden-Selection Persistence
- **Action**: Added `MenuInspectorPanel::RenderSnapshot` plus focused panel coverage so the menu diagnostics surface now records rendered summary, visible rows, issues, filter state, and selected-command detail instead of relying on placeholder panel sections.
- **Action**: Replaced the remaining placeholder registry, scene-graph, and selection sections in `MenuInspectorPanel` with model-backed detail, including selected-command state and actionable row-level detail.
- **Action**: Updated `MenuInspectorModel` to preserve selected command identity even when a selected command becomes filtered or hidden, then refreshed `DiagnosticsWorkspace` menu export from the panel snapshot so workspace JSON stays aligned with the rendered surface.
- **Action**: Revalidated the focused `[ui][editor][menu_inspector]` and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: Phase 3 menu diagnostics moved past export-only plumbing; the menu tab now has a verified rendered snapshot and retains actionable selection context across filter changes.

### 2026-04-17 â€” Menu Preview Snapshot Export
- **Action**: Added `MenuPreviewPanel::RenderSnapshot` plus focused preview-panel coverage so the menu workflow tab now records active scene id, visible pane state, selected command ids, and pane command lists instead of exposing preview only as title/visibility chrome.
- **Action**: Updated `DiagnosticsWorkspace::exportAsJson()` and active-menu snapshot refresh so the `menu.preview` payload comes from the rendered preview snapshot rather than a stub shell.
- **Action**: Revalidated the focused `[ui][editor][menu_preview][panel]`, `[ui][editor][menu_inspector]`, and `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: The menu diagnostics tab now exposes both inspector and preview workflow state as real, test-backed rendered surfaces instead of pairing one actionable panel with one mostly opaque preview shell.

### 2026-04-17 â€” Menu Preview Workspace Actions
- **Action**: Added `DiagnosticsWorkspace::dispatchMenuPreviewAction(InputAction)` so the menu diagnostics tab can drive preview-side `MoveUp`/`MoveDown`/`MoveLeft`/`MoveRight`/`Confirm`/`Cancel` flows against the bound `MenuSceneGraph` instead of only reading preview state.
- **Action**: Rebound the menu preview path to a mutable `MenuSceneGraph`, refreshed the menu inspector model after preview actions, and extended `MenuPreviewPanel::RenderSnapshot`/workspace export with `last_blocked_command_id` and `last_blocked_reason`.
- **Action**: Added focused diagnostics coverage proving workspace-level preview actions update selected commands, switch active panes, and surface blocked confirm reasons through exported preview state.
- **Action**: Revalidated the focused `[editor][diagnostics][integration][menu_preview_actions]`, broader `[editor][diagnostics][integration]`, and `[ui][editor][menu_preview][panel]` lanes after rebuilding `urpg_tests`.
- **Result**: The menu diagnostics tab is now actionable on both sides: the inspector can filter/select rows and the preview can be driven through real navigation/confirm flows with blocked-command diagnostics exported from the live graph.

### 2026-04-17 â€” Menu Inspector/Preview Selection Sync
- **Action**: Added `MenuInspectorModel::SelectCommandById()` so inspector selection can follow a command id directly instead of depending only on a current visible-row index.
- **Action**: Updated `DiagnosticsWorkspace` to reselect the live active preview command in the inspector after preview-side navigation, keeping exported `selected_command_id` and `selected_row` aligned with the mutable `MenuSceneGraph`.
- **Action**: Tightened diagnostics integration coverage so preview-side `MoveDown` now must also update the inspector-side selected command export instead of leaving it null.
- **Action**: Revalidated the focused `[editor][diagnostics][integration][menu_preview_actions]`, broader `[editor][diagnostics][integration]`, and `[ui][editor][menu_inspector]` lanes after rebuilding `urpg_tests`.
- **Result**: The two halves of the menu diagnostics tab now stay synchronized under preview-driven navigation rather than behaving like independent observers of the same graph.

### 2026-04-17 â€” Menu Inspector-to-Preview Reverse Sync
- **Action**: Updated `DiagnosticsWorkspace::selectMenuRow()` to project the selected inspector row back into the bound `MenuSceneGraph`, activating the corresponding pane and command in the live preview workflow.
- **Action**: Cleared stale blocked-command state on inspector-driven selection changes so preview export reflects the new selected command rather than a prior blocked confirm.
- **Action**: Tightened the existing menu workspace-actions regression so inspector-side row selection must also update the exported preview `selected_command_id`.
- **Action**: Revalidated the focused `[editor][diagnostics][integration][menu_actions]`, `[editor][diagnostics][integration][menu_preview_actions]`, and broader `[editor][diagnostics][integration]` lanes after rebuilding `urpg_tests`.
- **Result**: The menu diagnostics tab now has bidirectional coherence: preview actions update inspector selection, and inspector selection updates the live preview state.

### 2026-04-17 â€” Menu Duplicate-Command Pane Disambiguation
- **Action**: Added `MenuInspectorModel::SelectCommandRow(pane_id, command_id)` so workspace sync can target a concrete pane/command pair instead of ambiguously matching only on command id.
- **Action**: Updated preview-to-inspector synchronization in `DiagnosticsWorkspace` to use the active preview pane id together with the selected command id, fixing the case where multiple panes share the same command id.
- **Action**: Tightened the menu preview-actions regression so pane navigation must now select the `side_pane` inspector row after `MoveRight`, not the first matching `main_pane` row.
- **Action**: Revalidated the focused `[editor][diagnostics][integration][menu_preview_actions]`, broader `[editor][diagnostics][integration]`, and `[ui][editor][menu_inspector]` lanes after rebuilding `urpg_tests`.
- **Result**: Menu diagnostics selection sync is now stable even when the same command id appears in multiple panes.

### 2026-04-17 â€” Migration Wizard Last-Result Clear Truthfulness
- **Action**: Fixed `MigrationWizardModel::clearSubsystemResult()` so clearing the final remaining subsystem result routes through a full model reset instead of leaving the wizard marked complete with zero subsystem results.
- **Action**: Added focused wizard and diagnostics-workspace regressions covering the â€œclear last remaining subsystemâ€ path, including empty-state snapshot/export expectations and disabled save/clear affordances.
- **Action**: Revalidated the focused `[editor][diagnostics][wizard]`, `[editor][diagnostics][integration][wizard_actions]`, `[editor][diagnostics][integration][wizard_snapshot]`, and `[editor][diagnostics][integration][wizard_file_roundtrip]` lanes after rebuilding `urpg_tests`.
- **Result**: Migration wizard selective clear is now truthful at both model and workspace levels; removing the last subsystem no longer leaves behind a fake completed report shell.

### 2026-04-17 â€” Migration Wizard Selection-Scoped Actions
- **Action**: Added `rerunSelectedSubsystem(project_data)` and `clearSelectedSubsystemResult()` to `MigrationWizardModel` and forwarded them through `MigrationWizardPanel` plus `DiagnosticsWorkspace`, so the wizard can act directly on its current selected subsystem instead of forcing callers to pass the selected id back in manually.
- **Action**: Hardened the model wrappers to copy the selected subsystem id before dispatching into mutating id-based operations, avoiding aliasing against selection state that can be reset during the action.
- **Action**: Added focused wizard and workspace regressions covering selection-scoped rerun/clear behavior, including selection fallback to the remaining subsystem and empty-state affordances after the last selected subsystem is cleared.
- **Action**: Revalidated the focused `[selected_actions]`, `[wizard_selected_actions]`, `[editor][diagnostics][wizard]`, `[editor][diagnostics][integration][wizard_actions]`, `[editor][diagnostics][integration][wizard_snapshot]`, and `[editor][diagnostics][integration][wizard_file_roundtrip]` lanes after rebuilding `urpg_tests`.
- **Result**: Migration wizard workflow actions now match the selected-state surface they expose; callers can operate on the active subsystem directly without reconstructing ids from snapshot state.

### 2026-04-17 â€” Migration Wizard Imported-Selection Repair
- **Action**: Fixed `MigrationWizardModel::loadReportFromFile()` to repair orphaned `selected_subsystem_id` values when a saved report references a subsystem that is not present in the loaded `subsystem_results`.
- **Action**: Added focused model, panel, and diagnostics-workspace regressions covering this stale-selection import path so snapshots/export now fall back to the first real subsystem instead of exposing a fake selected id with no backing result.
- **Action**: Revalidated the focused orphan-selection regressions plus `[editor][diagnostics][wizard][file]`, `[editor][diagnostics][integration][wizard_file_failure]`, and `[editor][diagnostics][integration][wizard_file_roundtrip]` after rebuilding `urpg_tests`.
- **Result**: Migration wizard report import is now selection-safe; stale saved selections no longer leave the wizard exporting impossible selected-subsystem state after load.

### 2026-04-17 â€” Migration Wizard Aggregate Export Fidelity
- **Action**: Updated `DiagnosticsWorkspace::exportAsJson()` so the `migration_wizard` active-tab detail now includes the panel snapshot's aggregate `total_files_processed`, `warning_count`, and `error_count` fields instead of forcing consumers to infer them from tab summary or subsystem rows.
- **Action**: Added focused diagnostics-workspace regressions covering both the live-run export path and the loaded-report export path for those aggregate counts.
- **Action**: Forced a clean rebuild of `test_diagnostics_workspace.cpp.obj` / `urpg_tests.exe` after an incremental-build stale-object issue, then revalidated the focused wizard export and file-related integration lanes.
- **Result**: Migration wizard workspace export now carries the same top-level aggregate counts the panel snapshot already had, for both freshly run and report-loaded wizard states.

### 2026-04-17 â€” Migration Wizard Bound-Runtime Rerun Flow
- **Action**: Taught `MigrationWizardPanel` to retain bound project data from `onProjectUpdateRequested(...)` and expose no-argument rerun actions (`rerunBoundProject()`, `rerunBoundSelectedSubsystem()`), replacing part of the prior one-shot wrapper behavior.
- **Action**: Surfaced that retained-binding state through `MigrationWizardPanel::RenderSnapshot` with `has_bound_project_data`, `can_rerun_bound_migration`, and `can_rerun_bound_selected_subsystem`, then exported the same fields from `DiagnosticsWorkspace`.
- **Action**: Added workspace-level forwards `rerunBoundMigrationWizard()` and `rerunBoundSelectedMigrationWizardSubsystem()`, plus focused panel/workspace regressions proving that a cleared subsystem can be rebuilt from the retained bound runtime without resupplying project JSON.
- **Action**: Revalidated the focused `[bound_runtime]`, `[wizard_bound_runtime]`, `[editor][diagnostics][wizard]`, `[editor][diagnostics][integration][wizard_actions]`, `[editor][diagnostics][integration][wizard_selected_actions]`, and `[editor][diagnostics][integration][wizard_file_roundtrip]` lanes after rebuilding `urpg_tests`.
- **Result**: The migration wizard is no longer only a one-shot `project_data` trigger; its bound runtime can now drive real rerun workflow actions directly through the panel and diagnostics workspace.

### 2026-04-17 â€” Migration Wizard File-Load Runtime Detachment
- **Action**: Updated `MigrationWizardPanel::loadReportFromFile()` to clear any previously retained bound project data before importing a report file, so file-loaded wizard state does not inherit stale bound-runtime affordances from an earlier live binding.
- **Action**: Added focused panel and diagnostics-workspace regressions proving that `has_bound_project_data`, `can_rerun_bound_migration`, and `can_rerun_bound_selected_subsystem` all drop to `false` after loading a file on top of a previously bound wizard, and that bound rerun actions then fail as expected.
- **Action**: Forced a clean rebuild of the wizard-related test objects and `urpg_tests.exe` after another stale incremental-build issue, then revalidated the focused file/bound-runtime cases plus the broader wizard file lanes.
- **Result**: Migration wizard file import now cleanly detaches from prior live bindings; loaded reports no longer advertise impossible bound-runtime rerun affordances.

### 2026-04-17 â€” Migration Wizard Failed-Load Bound-Runtime Coverage
- **Action**: Tightened the existing `wizard_file_failure` diagnostics-workspace regression so an invalid report load must also clear the exported bound-runtime affordances and make bound rerun actions fail, not only clear report rows.
- **Action**: Revalidated the focused wizard file-failure, file-roundtrip, and file-selection-repair integration lanes after rebuilding `urpg_tests`.
- **Result**: The failed-load path is now explicitly covered for both empty-report state and bound-runtime detachment, closing the remaining test gap in that file-import branch.

### 2026-04-17 â€” Migration Wizard Save-Preserves-Binding Coverage
- **Action**: Added focused panel and diagnostics-workspace regressions proving that `saveReportToFile()` is non-destructive with respect to the retained bound runtime: save leaves `has_bound_project_data` and the bound rerun affordances intact, and bound rerun actions still succeed immediately afterward.
- **Action**: Revalidated the focused `[save_binding]`, `[wizard_file_save_binding]`, and broader wizard file integration lanes after rebuilding `urpg_tests`.
- **Result**: The wizard file workflow is now explicitly covered as asymmetric in the intended way: save preserves bound runtime, while load/failed-load detach it.

### 2026-04-17 â€” Migration Wizard Panel Failed-Load Binding Coverage
- **Action**: Added a focused panel-level regression proving that `MigrationWizardPanel::loadReportFromFile()` clears retained bound-runtime affordances on invalid input, not only through the workspace export path.
- **Action**: Revalidated the broader `[editor][diagnostics][wizard][file]` and `[editor][diagnostics][integration][wizard_file_failure]` lanes after rebuilding `urpg_tests`.
- **Result**: The wizard's failed-load detachment contract is now covered at both panel and workspace levels, closing the last obvious panel-local gap in the file/binding lane.

### 2026-04-18 — Agent Swarm Pass 4
- **Action**: Agent swarm pass 4 launched with 4 parallel agents.
- **Action**: Window compat rendering closure: drawIcon, drawGauge, drawCharacter, Window_Command::drawItem, status reconciliation, JS bindings.
- **Action**: Battle manager runtime closure: gainExp level-up, applySkill, applyItem.
- **Action**: Message inspector panel productization: RenderSnapshot, render(), workspace snapshot coherence.
- **Action**: Compat exit checklist publication: created docs/COMPAT_EXIT_CHECKLIST.md with import-confidence and migration-confidence pass criteria, signed-off-by section, and related-document links.
- **Result**: Compat exit checklist artifact is published. Documentation alignment updates applied to docs/PROGRAM_COMPLETION_STATUS.md, docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md, and WORKLOG.md.
