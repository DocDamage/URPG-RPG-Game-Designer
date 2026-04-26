# AAA Release Execution Plan

Status Date: 2026-04-25

Scope: Convert the senior release audit into ordered implementation work for moving URPG toward a production-quality release. Tasks are grouped so a developer or coding agent can complete them sequentially without guessing.

## Phase 0 - Build Blockers And App-Breaking Issues

### P0-01 - Add Production App Targets

- Task ID: P0-01
- Title: Add Production App Targets
- Files to edit: `CMakeLists.txt`, `apps/editor/main.cpp`, `apps/runtime/main.cpp`
- Files to inspect: `engine/core/engine_shell.h`, `engine/core/platform/sdl_surface.*`, `engine/core/platform/opengl_renderer.*`
- Dependencies: none
- Risk level: High
- Exact implementation steps:
  1. Add `urpg_editor` and `urpg_runtime` executable targets to `CMakeLists.txt`.
  2. Create app entry points that initialize SDL surface and renderer.
  3. Start `EngineShell` from the runtime app.
  4. Start the editor shell from the editor app.
  5. Load a default project or default boot scene.
  6. Add install/package rules for both binaries.
- Acceptance criteria: app binaries build and launch without relying on tests or tools.
- Verification command or manual test: `cmake --build --preset dev-debug --target urpg_editor urpg_runtime`

### P0-02 - Fix Project-Owned Compiler Warnings

- Task ID: P0-02
- Title: Fix Project-Owned Compiler Warnings
- Files to edit: `engine/core/presentation/effects/effect_cue.h`
- Files to inspect: active compile output from `urpg_core`
- Dependencies: none
- Risk level: Low
- Exact implementation steps:
  1. Remove invalid `constexpr` from `effectCueLess` or make the comparison constexpr-safe.
  2. Rebuild `urpg_core`.
  3. Run a strict warnings build.
- Acceptance criteria: zero project-owned warnings.
- Verification command or manual test: `cmake -S . -B build/dev-warnings -DURPG_WARNINGS_AS_ERRORS=ON && cmake --build build/dev-warnings --target urpg_core`

### P0-03 - Stabilize Local Gate Runtime

- Task ID: P0-03
- Title: Stabilize Local Gate Runtime
- Files to edit: `tools/ci/run_local_gates.ps1`
- Files to inspect: `CMakePresets.json`, `.github/workflows/ci-gates.yml`
- Dependencies: P0-02
- Risk level: Medium
- Exact implementation steps:
  1. Ensure local gates do not launch overlapping builds in the same build tree.
  2. Add clear timeout handling.
  3. Ensure `ctest -L pr` reports exact failures or timeout causes.
  4. Update command docs if behavior changes.
- Acceptance criteria: one local command gives deterministic pass/fail output.
- Verification command or manual test: `.\tools\ci\run_local_gates.ps1`

### P0-04 - Fix Project-Owned Test Warnings

- Task ID: P0-04
- Title: Fix Project-Owned Test Warnings
- Files to edit: `tests/unit/test_plugin_compatibility_score.cpp`
- Files to inspect: active compile output from `urpg_tests`
- Dependencies: P0-02
- Risk level: Low
- Exact implementation steps:
  1. Replace warning-prone temporary string references in plugin compatibility score tests.
  2. Keep assertions and test coverage unchanged.
  3. Rebuild `urpg_tests`.
- Acceptance criteria: `urpg_tests` builds without project-owned warnings from `test_plugin_compatibility_score.cpp`.
- Verification command or manual test: `cmake --build --preset dev-debug --target urpg_tests`

## Phase 1 - Unwired UI, Routes, Handlers, And Feature Surfaces

### P1-01 - Implement Real Editor Shell

- Task ID: P1-01
- Title: Implement Real Editor Shell
- Files to edit: `engine/core/editor/editor_shell.h`, `apps/editor/main.cpp`, relevant `editor/**` registration files
- Files to inspect: all `editor/*/*panel.*`
- Dependencies: P0-01
- Risk level: High
- Exact implementation steps:
  1. Implement ImGui frame lifecycle.
  2. Register editor panels.
  3. Add top-level menu/navigation.
  4. Route panel visibility and selection.
  5. Wire project save/load context.
  6. Wire runtime preview context.
- Acceptance criteria: editor launches and panels are reachable.
- Verification command or manual test: launch `urpg_editor`; open diagnostics, assets, ability, mod, and analytics panels.

### P1-02 - Wire Ability Inspector UI

- Task ID: P1-02
- Title: Wire Ability Inspector UI
- Files to edit: `editor/ability/ability_inspector_panel.*`
- Files to inspect: `editor/ability/ability_inspector_model.*`, `engine/core/ability/*`
- Dependencies: P1-01
- Risk level: Medium
- Exact implementation steps:
  1. Replace `std::cout` rendering with ImGui rendering.
  2. Add ability selection.
  3. Add preview, validate, activate, save, and load controls.
  4. Surface diagnostics in the UI.
  5. Keep model snapshot tests deterministic.
- Acceptance criteria: no console-only ability inspector behavior remains.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[ability][editor]"`

### P1-03 - Wire Pattern Field Editor UI

- Task ID: P1-03
- Title: Wire Pattern Field Editor UI
- Files to edit: `editor/ability/pattern_field_panel.*`
- Files to inspect: `editor/ability/pattern_field_model.*`, `engine/core/ability/pattern_field.*`
- Dependencies: P1-01
- Risk level: Medium
- Exact implementation steps:
  1. Implement visual grid canvas.
  2. Add point toggling.
  3. Add validation display.
  4. Add zoom/pan if supported by the current model.
  5. Persist model changes.
- Acceptance criteria: users can edit and persist pattern fields visually.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[ability][pattern]"`

### P1-04 - Add Mod Manager Actions

- Task ID: P1-04
- Title: Add Mod Manager Actions
- Files to edit: `editor/mod/mod_manager_panel.*`, `engine/core/mod/mod_loader.*`
- Files to inspect: `engine/core/mod/mod_registry.*`
- Dependencies: P1-01
- Risk level: High
- Exact implementation steps:
  1. Add import/register action.
  2. Add activate/deactivate actions.
  3. Add reload action.
  4. Display sandbox policy.
  5. Display loader and validation errors.
- Acceptance criteria: editor can perform full manifest lifecycle.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[mod]"`

### P1-05 - Add Analytics Consent And Controls

- Task ID: P1-05
- Title: Add Analytics Consent And Controls
- Files to edit: `editor/analytics/analytics_panel.*`, `engine/core/analytics/*`
- Files to inspect: `content/schemas/analytics_config.schema.json`
- Dependencies: P1-01
- Risk level: Medium
- Exact implementation steps:
  1. Add opt-in toggle.
  2. Add clear queued events action.
  3. Add flush/upload control.
  4. Add retention display.
  5. Add privacy status and disabled-upload messaging.
- Acceptance criteria: no analytics upload or flush state is ambiguous.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[analytics]"`

## Phase 2 - Incomplete Implementations, Stubs, Placeholders, Mock Data

### P2-01 - Replace Mock Dialogue Startup

- Task ID: P2-01
- Title: Replace Mock Dialogue Startup
- Files to edit: `engine/core/engine_shell.h`, `engine/core/message/*`
- Files to inspect: `content/schemas/dialogue_*.json`, `tests/unit/test_message_*`
- Dependencies: P0-01
- Risk level: High
- Exact implementation steps:
  1. Load dialogue from project data.
  2. Remove production startup dependency on `MockDialogueBuilder::populate()`.
  3. Keep mock dialogue as test-only fixture support.
  4. Add diagnostics for missing or invalid dialogue data.
- Acceptance criteria: production runtime starts from project content only.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[message]"`

### P2-02 - Implement Dialogue Command Handlers

- Task ID: P2-02
- Title: Implement Dialogue Command Handlers
- Files to edit: `engine/core/engine_shell.h`, `engine/core/quest/*`, `engine/core/ecs/health_system.h`, inventory-related files
- Files to inspect: `engine/core/message/message_core.*`
- Dependencies: P2-01
- Risk level: High
- Exact implementation steps:
  1. Route `GIVE_ITEM` into the inventory system.
  2. Route `HEAL_PLAYER` into the health system.
  3. Route `QUEST_ADVANCE` into the quest system.
  4. Return structured command results.
  5. Surface command failures through diagnostics.
- Acceptance criteria: dialogue commands mutate gameplay state and failures are visible.
- Verification command or manual test: add and run integration test for dialogue-to-gameplay commands.

### P2-03 - Complete Runtime Audio Backend

- Task ID: P2-03
- Title: Complete Runtime Audio Backend
- Files to edit: `engine/core/audio/*`, `runtimes/compat_js/audio_manager.*`
- Files to inspect: `engine/core/platform/*`, `editor/audio/*`
- Dependencies: P0-01
- Risk level: High
- Exact implementation steps:
  1. Select and integrate the audio backend.
  2. Implement decode/play/stop/fade/channel state.
  3. Route BGM, SE, ME, and BGS.
  4. Connect mix presets.
  5. Connect compat audio calls.
  6. Add device and asset failure diagnostics.
- Acceptance criteria: BGM/SE/ME/BGS audibly play and lifecycle is tracked.
- Verification command or manual test: audio smoke app/manual playback plus `.\build\dev-ninja-debug\urpg_tests.exe "[audio]"`

### P2-04 - Implement Spatial Audio Hardware Mix

- Task ID: P2-04
- Title: Implement Spatial Audio Hardware Mix
- Files to edit: `engine/core/audio/audio_mixer.h` or new `engine/core/audio/audio_mixer.cpp`
- Files to inspect: `engine/core/audio/audio_core.h`
- Dependencies: P2-03
- Risk level: Medium
- Exact implementation steps:
  1. Route computed gain to backend channels.
  2. Route computed pan to backend channels.
  3. Update moving sources each frame.
  4. Add diagnostics for missing backend/source handles.
- Acceptance criteria: listener movement affects pan/gain.
- Verification command or manual test: new spatial audio unit or integration test.

### P2-05 - Promote QuickJS Harness Toward Live Runtime

- Task ID: P2-05
- Title: Promote QuickJS Harness Toward Live Runtime
- Files to edit: `runtimes/compat_js/quickjs_runtime.*`
- Files to inspect: `runtimes/compat_js/plugin_manager.*`, `tests/compat/*`
- Dependencies: P0-01
- Risk level: High
- Exact implementation steps:
  1. Initialize a real QuickJS context.
  2. Load and evaluate JavaScript source.
  3. Expose host bindings.
  4. Preserve deterministic budget and memory limits.
  5. Preserve structured error diagnostics.
  6. Keep fixture directive support only where explicitly test-scoped.
- Acceptance criteria: real JavaScript plugin fixtures execute without `@urpg-*` directives.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_compat_tests.exe`

### P2-06 - Complete Plugin Manager Live Loading

- Task ID: P2-06
- Title: Complete Plugin Manager Live Loading
- Files to edit: `runtimes/compat_js/plugin_manager.*`, `runtimes/compat_js/plugin_manager_status.cpp`
- Files to inspect: `third_party/rpgmaker-mz/**`, `tests/compat/fixtures/**`
- Dependencies: P2-05
- Risk level: High
- Exact implementation steps:
  1. Load real plugin files from disk.
  2. Execute plugin entry points through QuickJS.
  3. Track commands, dependencies, permissions, and errors.
  4. Update method statuses to match real behavior.
  5. Preserve deterministic diagnostics for unsupported APIs.
- Acceptance criteria: fixture-backed status no longer overstates live loading.
- Verification command or manual test: weekly compat suite plus curated plugin drop-in validation.

### P2-07 - Implement Mod Script Execution

- Task ID: P2-07
- Title: Implement Mod Script Execution
- Files to edit: `engine/core/mod/mod_loader.*`, `runtimes/compat_js/*`
- Files to inspect: `content/fixtures/mod_sdk_sample/*`
- Dependencies: P2-05, P1-04
- Risk level: High
- Exact implementation steps:
  1. Load mod entrypoint.
  2. Enforce sandbox policy.
  3. Execute lifecycle hooks.
  4. Roll back registration/activation on failure.
  5. Surface sandbox and script errors to editor diagnostics.
- Acceptance criteria: sample mod changes observable runtime state.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[mod]"`

## Phase 3 - Error Handling, Validation, Loading States, Empty States

### P3-01 - Replace Swallowed AI Sync Errors

- Task ID: P3-01
- Title: Replace Swallowed AI Sync Errors
- Files to edit: `engine/core/message/ai_sync_coordinator.*`
- Files to inspect: `engine/core/social/cloud_service.h`, `tests/unit/test_ai_sync_coordinator.cpp`
- Dependencies: none
- Risk level: Medium
- Exact implementation steps:
  1. Replace ambiguous bool returns with structured results or companion diagnostics.
  2. Distinguish offline, missing key, invalid JSON, schema mismatch, and restore failure.
  3. Update tests for each failure path.
- Acceptance criteria: parse failures are not silent.
- Verification command or manual test: focused AI sync tests.

### P3-02 - Add Editor Empty/Disconnected States

- Task ID: P3-02
- Title: Add Editor Empty/Disconnected States
- Files to edit: `editor/mod/*`, `editor/analytics/*`, `editor/audio/*`, `editor/assets/*`
- Files to inspect: panel snapshots and panel tests
- Dependencies: P1-01
- Risk level: Medium
- Exact implementation steps:
  1. Add missing-model state.
  2. Add missing-service state.
  3. Add no-project-loaded state.
  4. Disable unsafe actions.
  5. Add actionable status messages.
- Acceptance criteria: unbound panels never look successful or blank without explanation.
- Verification command or manual test: panel unit tests for null/unbound states.

### P3-03 - Normalize Console Logging To Diagnostics

- Task ID: P3-03
- Title: Normalize Console Logging To Diagnostics
- Files to edit: `engine/core/editor/security_manager.cpp`, `engine/core/editor/plugin_api.cpp`, platform/render/audio diagnostic sites
- Files to inspect: output of `rg "std::cout|std::cerr" engine editor runtimes`
- Dependencies: none
- Risk level: Medium
- Exact implementation steps:
  1. Introduce or reuse diagnostics sink.
  2. Route production runtime/editor logs into diagnostics.
  3. Keep CLI stdout/stderr only for tools.
  4. Add tests where diagnostics affect UI state.
- Acceptance criteria: production app does not depend on console output for user feedback.
- Verification command or manual test: grep plus diagnostics tests.

## Phase 4 - Persistence/API/Storage/Data Integrity

### P4-01 - Implement Runtime Bundle Signature Enforcement

- Task ID: P4-01
- Title: Implement Runtime Bundle Signature Enforcement
- Files to edit: `engine/core/export/runtime_bundle_loader.*`, `engine/core/export/export_validator.*`, `engine/core/tools/export_packager_bundle_writer.*`
- Files to inspect: `docs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md`
- Dependencies: P5-01
- Risk level: High
- Exact implementation steps:
  1. Parse bundle header and manifest with strict bounds.
  2. Verify bundle signature before exposing payloads.
  3. Verify per-entry integrity before decompression/deobfuscation.
  4. Add structured diagnostics for malformed bundle classes.
  5. Share byte-contract helpers or fixtures with `ExportValidator`.
- Acceptance criteria: runtime rejects tampered bundles.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[export][runtime_bundle]"`

### P4-02 - Add Atomic Bundle Publication

- Task ID: P4-02
- Title: Add Atomic Bundle Publication
- Files to edit: `engine/core/tools/export_packager_bundle_writer.*`
- Files to inspect: export packager tests
- Dependencies: P4-01
- Risk level: Medium
- Exact implementation steps:
  1. Write `data.pck.tmp`.
  2. Flush and close the temp file.
  3. Validate the temp file.
  4. Atomically replace `data.pck`.
  5. Remove temp file on failure.
  6. Report structured export errors.
- Acceptance criteria: interrupted or failed export never leaves a corrupt final bundle.
- Verification command or manual test: new tests for failed write and validation paths.

### P4-03 - Persist Runtime-Created Character State

- Task ID: P4-03
- Title: Persist Runtime-Created Character State
- Files to edit: `engine/core/character/*`, save serialization files
- Files to inspect: `tests/unit/test_character_creation_screen.cpp`, save tests
- Dependencies: none
- Risk level: Medium
- Exact implementation steps:
  1. Serialize created protagonist identity.
  2. Serialize attributes, class, portrait, body, and appearance.
  3. Restore through save/load.
  4. Validate against character schema.
- Acceptance criteria: created character survives save/load.
- Verification command or manual test: character-save integration test.

## Phase 5 - Asset, Manifest, Packaging, And Release-Readiness Fixes

### P5-01 - Replace Smoke Export Payload

- Task ID: P5-01
- Status: Completed on 2026-04-25
- Title: Replace Smoke Export Payload
- Files to edit: `engine/core/tools/export_packager_payload_builder.cpp`
- Files to inspect: `content/schemas/export_config.schema.json`, `tools/pack/pack_cli.cpp`
- Dependencies: P0-01
- Risk level: High
- Exact implementation steps:
  1. Remove smoke contract metadata.
  2. Include real project scene/config data.
  3. Include real project assets.
  4. Include real script policy.
  5. Fail closed for unsupported script export modes.
- Acceptance criteria: exported bundle contains actual project content and no placeholder policy strings.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_tests.exe "[export][packager]"`

### P5-02 - Add One-Command Platform Export Matrix

- Task ID: P5-02
- Status: Completed on 2026-04-25
- Title: Add One-Command Platform Export Matrix
- Files to edit: `tools/ci/check_platform_exports.ps1`, `tools/ci/run_local_gates.ps1`, `.github/workflows/ci-gates.yml`
- Files to inspect: `engine/core/export/export_validator.*`
- Dependencies: P5-01
- Risk level: Medium
- Exact implementation steps:
  1. Add defaults or wrapper for Windows, Linux, macOS, and Web targets.
  2. Emit JSON results.
  3. Integrate into local gates.
  4. Integrate into CI gates.
  5. Make unavailable platform targets explicit skips, not silent passes.
- Acceptance criteria: no manual parameters are required for standard release validation.
- Verification command or manual test: `.\tools\ci\check_platform_exports.ps1` or the new wrapper command.

### P5-03 - Regenerate Asset Hygiene Reports

- Task ID: P5-03
- Status: Completed on 2026-04-25
- Title: Regenerate Asset Hygiene Reports
- Files to edit: generated `imports/reports/*`
- Files to inspect: `tools/assets/asset_hygiene.py`, `docs/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`
- Dependencies: none
- Risk level: Low
- Exact implementation steps:
  1. Rerun asset hygiene in the current repo.
  2. Remove stale absolute paths.
  3. Update docs if counts change.
  4. Ensure reports are deterministic enough for review.
- Acceptance criteria: reports reference `C:\dev\URPG Maker` and current scan data.
- Verification command or manual test: `python .\tools\assets\asset_hygiene.py --write-reports`

### P5-04 - Promote License-Cleared Asset Set

- Task ID: P5-04
- Status: Completed on 2026-04-25
- Title: Promote License-Cleared Asset Set
- Files to edit: `imports/manifests/**`, `imports/reports/**`, `content/**`
- Files to inspect: `imports/raw/**`, `docs/asset_intake/*`
- Dependencies: P5-03
- Risk level: High
- Exact implementation steps:
  1. Select release asset candidates.
  2. Capture attribution.
  3. Validate license terms.
  4. Normalize promoted assets.
  5. Create bundle manifests.
  6. Exclude raw intake from release packaging.
- Acceptance criteria: shipped package includes only promoted, attributed assets.
- Verification command or manual test: asset governance script plus export package inspection.

### P5-05 - Add Native Signing/Notarization Plan Hooks

- Task ID: P5-05
- Status: Completed on 2026-04-25
- Title: Add Native Signing/Notarization Plan Hooks
- Files to edit: packaging scripts, release docs
- Files to inspect: export docs and CI workflows
- Dependencies: P5-02
- Risk level: High
- Exact implementation steps:
  1. Define signing inputs.
  2. Add unsigned/dev mode.
  3. Add release mode that fails without credentials.
  4. Add notarization hooks where applicable.
  5. Add release artifact reporting.
- Acceptance criteria: release packaging cannot silently ship unsigned artifacts.
- Verification command or manual test: release package dry run.

### P5-06 - Resolve RPG Maker Plugin Drop-In Collisions

- Task ID: P5-06 (completed)
- Title: Resolve RPG Maker Plugin Drop-In Collisions
- Files to edit: `third_party/rpgmaker-mz/steam-dlc/plugin-dropins/**`, `third_party/rpgmaker-mz/steam-dlc/reports/**`, release packaging manifests
- Files to inspect: `tools/rpgmaker/validate-plugin-dropins.ps1`, `third_party/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_issues.csv`
- Dependencies: P5-04
- Risk level: High
- Exact implementation steps:
  1. Review duplicate stem and duplicate plugin key collisions reported by the drop-in validator.
  2. Classify each collision as identical copy, localized sample copy, or conflicting plugin implementation.
  3. Preserve all source files in intake storage; do not delete vendor DLC content.
  4. Promote one deterministic release candidate per plugin key into the release manifest.
  5. Record excluded duplicate sources and the reason they are not packaged.
  6. Regenerate drop-in validation reports.
- Acceptance criteria: release packaging has one deterministic source per plugin key and plugin drop-in validation reports `ERROR_COUNT 0`.
- Verification command or manual test: `.\tools\rpgmaker\validate-plugin-dropins.ps1 -FailOnError`

## Phase 6 - Tests, Regression Coverage, And Final Verification

### P6-01 - Register All Python Tool Tests

- Task ID: P6-01 (completed)
- Title: Register All Python Tool Tests
- Files to edit: `CMakeLists.txt`, `.github/workflows/ci-gates.yml`
- Files to inspect: `tools/audio/tests/*.py`, `tools/retrieval/tests/*.py`, `tools/vision/tests/*.py`
- Dependencies: none
- Risk level: Medium
- Exact implementation steps:
  1. Register all committed Python tests in CTest or pytest.
  2. Add optional dependency skips with explicit reasons.
  3. Add CI coverage for the registered tests.
- Acceptance criteria: all committed tool tests are run or explicitly skipped with reason.
- Verification command or manual test: `ctest --preset dev-all -L pr --output-on-failure`

### P6-02 - Add C++ Format/Static Analysis Gate (completed)

- Task ID: P6-02
- Title: Add C++ Format/Static Analysis Gate
- Files to edit: `.pre-commit-config.yaml`, `.github/workflows/ci-gates.yml`
- Files to inspect: current style docs and CMake target layout
- Dependencies: P0-02
- Risk level: Medium
- Exact implementation steps:
  1. Add clang-format check.
  2. Add clang-tidy or equivalent static analysis profile.
  3. Add local command documentation.
  4. Enforce in CI.
- Acceptance criteria: CI rejects C++ style/static-analysis regressions.
- Verification command or manual test: `pre-commit run --all-files`

### P6-03 - Add End-To-End Editor Smoke Test

- Task ID: P6-03
- Title: Add End-To-End Editor Smoke Test
- Files to edit: tests or app smoke harness
- Files to inspect: `apps/editor/main.cpp`, editor shell
- Dependencies: P1-01
- Risk level: High
- Exact implementation steps:
  1. Launch editor in headless/smoke mode.
  2. Open key panels.
  3. Load a sample project.
  4. Save project state.
  5. Exit cleanly.
  6. Register as CTest.
- Acceptance criteria: editor startup workflow is covered in CI.
- Verification command or manual test: new `ctest` labeled `pr;editor_smoke`

### P6-04 - Add Exported Runtime Smoke Test

- Task ID: P6-04
- Title: Add Exported Runtime Smoke Test
- Files to edit: export tests, `tools/pack/pack_cli.cpp`
- Files to inspect: `tools/export/export_smoke_app.cpp`
- Dependencies: P4-01, P5-01
- Risk level: High
- Exact implementation steps:
  1. Package sample project.
  2. Launch exported runtime.
  3. Verify signed bundle load.
  4. Tamper with bundle.
  5. Verify runtime rejection.
  6. Register as nightly CTest.
- Acceptance criteria: export path proves runtime load, not only validator output.
- Verification command or manual test: new `ctest` labeled `nightly;export`

### P6-05 - Final Release Gate

- Task ID: P6-05
- Title: Final Release Gate
- Files to edit: none unless failures are found
- Files to inspect: all generated reports and logs
- Dependencies: all prior tasks
- Risk level: High
- Exact implementation steps:
  1. Run local gates.
  2. Run pre-commit.
  3. Run all CTest labels.
  4. Run export matrix.
  5. Run visual regression.
  6. Run asset governance.
  7. Confirm release docs are current.
- Acceptance criteria: zero release blockers, zero export blockers, no warnings, all release docs current.
- Verification command or manual test:

```powershell
.\tools\ci\run_local_gates.ps1
pre-commit run --all-files
ctest --preset dev-all --output-on-failure
.\tools\ci\check_release_readiness.ps1
.\tools\ci\check_platform_exports.ps1
```
