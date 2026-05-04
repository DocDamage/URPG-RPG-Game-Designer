# AAA Production Release Execution Plan

Status Date: 2026-05-04

This plan converts the latest senior release-engineering audit into ordered remediation work. Tasks are grouped so a developer or coding agent can execute them one by one without guessing. The current blocker is that the canonical `build/dev-ninja-debug` tree is not a valid CMake build directory, so release evidence must be regenerated after build/test discovery is restored.

## Phase 0 - Build Blockers And App-Breaking Issues

### P0-01 - Restore canonical CMake debug build tree

- Task ID: `P0-01`
- Title: Restore canonical CMake debug build tree
- Files to edit: `CMakePresets.json`, `CMakeLists.txt` if configure failure proves config drift
- Files to inspect: `build/dev-ninja-debug`, `.cache/fetchcontent`, CMake configure logs
- Dependencies: None
- Risk level: Blocker
- Exact implementation steps:
  1. Confirm `build/dev-ninja-debug` contains no user-authored artifacts.
  2. Remove or quarantine the invalid generated contents.
  3. Run `cmake --preset dev-ninja-debug`.
  4. Resolve dependency, compiler, FetchContent, or preset errors.
  5. Confirm `CMakeCache.txt`, `build.ninja`, and `CTestTestfile.cmake` exist.
  6. Run `cmake --build --preset dev-debug`.
- Acceptance criteria: Canonical debug preset configures and builds from a clean state.
- Verification command or manual test: `cmake --preset dev-ninja-debug`; then `cmake --build --preset dev-debug`

### P0-02 - Restore test discovery for release gates

- Task ID: `P0-02`
- Title: Restore test discovery for release gates
- Files to edit: `CMakeLists.txt`, `CMakePresets.json`
- Files to inspect: generated `CTestTestfile.cmake`, test labels in `CMakeLists.txt`, `docs/agent/QUALITY_GATES.md`
- Dependencies: `P0-01`
- Risk level: Blocker
- Exact implementation steps:
  1. Run `ctest --test-dir build/dev-ninja-debug -N`.
  2. If zero tests are listed, inspect CMake test registration and `enable_testing()`.
  3. Ensure `dev-pr`, `dev-export`, `dev-spatial`, `dev-snapshot`, and related labels map to registered tests.
  4. Reconfigure after any CMake changes.
  5. Run the PR preset and confirm it executes real tests.
- Acceptance criteria: `ctest -N` lists the expected CTest suite and label presets execute real tests.
- Verification command or manual test: `ctest --test-dir build/dev-ninja-debug -N`; then `ctest --preset dev-pr --output-on-failure`

### P0-03 - Regenerate stale release signoff evidence

- Task ID: `P0-03`
- Title: Regenerate stale release signoff evidence
- Files to edit: `docs/*_CLOSURE_SIGNOFF.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`, `docs/release/AAA_RELEASE_READINESS_REPORT.md`
- Files to inspect: `docs/RELEASE_SIGNOFF_WORKFLOW.md`, project audit JSON output
- Dependencies: `P0-01`, `P0-02`
- Risk level: Blocker
- Exact implementation steps:
  1. Find every signoff that claims `PASS` for commands run against `build/dev-ninja-debug`.
  2. Rerun each command on the current commit.
  3. Update commit SHA, date, command, and result.
  4. Mark any unrunnable command as blocked, not passed.
  5. Re-run project audit after documentation updates.
- Acceptance criteria: Release docs no longer cite commands that currently discover zero tests.
- Verification command or manual test: `./build/dev-ninja-debug/urpg_project_audit.exe --json`

## Phase 1 - Unwired UI, Routes, Handlers, And Feature Surfaces

### P1-01 - Implement real runtime chat text entry

- Task ID: `P1-01`
- Title: Implement real runtime chat text entry
- Files to edit: `engine/core/input/*`, `engine/core/platform/sdl_surface.*`, `engine/core/scene/map_scene.*`
- Files to inspect: `engine/core/message/chatbot_component.h`, runtime input tests
- Dependencies: `P0-01`
- Risk level: High
- Exact implementation steps:
  1. Add text-input event storage to `InputCore`.
  2. Forward SDL text input, backspace, and editing events into the input layer.
  3. Replace the mocked text-entry branch in `MapScene::handleInput`.
  4. Handle submit, cancel, backspace, empty submit, and focus state.
  5. Add headless-friendly tests for typed chat submission.
- Acceptance criteria: Runtime chat input accepts real typed text and sends it to `ChatbotComponent`.
- Verification command or manual test: `ctest --preset dev-all -R "input|MapScene|Chatbot" --output-on-failure`

### P1-02 - Add cross-platform asset import picker fallback

- Task ID: `P1-02`
- Title: Add cross-platform asset import picker fallback
- Files to edit: `editor/assets/asset_library_panel.*`
- Files to inspect: asset import model tests, platform UI helpers, packaging docs
- Dependencies: `P0-01`
- Risk level: Medium
- Exact implementation steps:
  1. Keep the current Windows native dialog path.
  2. Add a non-Windows fallback path or ImGui/path-entry import flow.
  3. Surface unsupported, cancelled, and invalid-path states distinctly.
  4. Add tests for picker availability snapshot on each platform mode.
- Acceptance criteria: Asset import source selection is usable or explicitly path-driven on Linux and macOS.
- Verification command or manual test: `ctest --preset dev-all -R "AssetLibrary|Asset.*Import" --output-on-failure`

## Phase 2 - Incomplete Implementations, Stubs, Placeholders, Mock Data

### P2-01 - Apply AI animation commands to runtime animation state

- Task ID: `P2-01`
- Title: Apply AI animation commands to runtime animation state
- Files to edit: `engine/core/scene/map_scene.*`, animation runtime files under `engine/core/animation/`
- Files to inspect: `engine/core/animation/animation_ai_bridge.*`, animation tests
- Dependencies: `P1-01` optional for interactive testing
- Risk level: Medium
- Exact implementation steps:
  1. Define a real API for applying parsed keyframes to the player animation path.
  2. Replace the commented `applySequence` placeholder with real behavior.
  3. Record diagnostics for unsupported target/entity cases.
  4. Add regression coverage proving a parsed command changes animation state.
- Acceptance criteria: AI animation commands visibly affect runtime animation or produce actionable diagnostics.
- Verification command or manual test: `ctest --preset dev-all -R "animation|MapScene|AI" --output-on-failure`

### P2-02 - Remove placeholder global state source file

- Task ID: `P2-02`
- Title: Remove placeholder global state source file
- Files to edit: `engine/core/global_state_hub.cpp`, possibly `CMakeLists.txt`
- Files to inspect: `engine/core/global_state_hub.h`, state tests
- Dependencies: `P0-01`
- Risk level: Low
- Exact implementation steps:
  1. Decide whether `global_state_hub.cpp` should own validation logic.
  2. If yes, implement validation and diagnostic hooks.
  3. If no, remove the placeholder source from CMake and keep the header as owner.
  4. Update tests and docs if ownership changes.
- Acceptance criteria: No release-owned core source file exists only as a placeholder.
- Verification command or manual test: `ctest --preset dev-pr --output-on-failure`

### P2-03 - Make placeholder rendering fail-loud in release mode

- Task ID: `P2-03`
- Title: Make placeholder rendering fail-loud in release mode
- Files to edit: `engine/core/platform/opengl_renderer.*`, `engine/core/scene/map_scene.*`, runtime preflight files
- Files to inspect: renderer snapshot tests, asset validation tests
- Dependencies: `P0-01`
- Risk level: High
- Exact implementation steps:
  1. Add runtime/export mode awareness for unresolved texture IDs.
  2. Keep deterministic placeholders only for tests/dev diagnostics.
  3. In release mode, promote missing player, tile, and battle visuals to blocking diagnostics or explicit missing-asset overlays.
  4. Update snapshot expectations.
- Acceptance criteria: Release runtime cannot silently hide missing core visuals with hashed quads.
- Verification command or manual test: `ctest --preset dev-all -R "MapScene|AssetLibrary|renderer-backed|preflight" --output-on-failure`

## Phase 3 - Error Handling, Validation, Loading States, Empty States

### P3-01 - Harden save/load UI feedback for map-scene failures

- Task ID: `P3-01`
- Title: Harden save/load UI feedback for map-scene failures
- Files to edit: `engine/core/scene/map_scene.*`, `engine/core/save/*`
- Files to inspect: runtime title continue flow, save recovery tests
- Dependencies: `P4-01`
- Risk level: Medium
- Exact implementation steps:
  1. Replace boolean-only `saveGame` and `loadGame` outcomes with structured result data.
  2. Preserve and surface failure reason, path, and recovery tier.
  3. Add scene/runtime diagnostics for failed save/load.
  4. Update call sites and tests.
- Acceptance criteria: Save/load failures are visible to UI and tests, not just returned as `false`.
- Verification command or manual test: `ctest --preset dev-all -R "save|load|MapScene|RuntimeTitleScene" --output-on-failure`

### P3-02 - Align release legal state messaging

- Task ID: `P3-02`
- Title: Align release legal state messaging
- Files to edit: `EULA.md`, `docs/release/LEGAL_REVIEW_SIGNOFF.md`, `README.md`
- Files to inspect: `PRIVACY_POLICY.md`, `THIRD_PARTY_NOTICES.md`
- Dependencies: None
- Risk level: Blocker
- Exact implementation steps:
  1. Resolve the contradiction between placeholder EULA text and public-release waiver text.
  2. Either replace EULA with production text or explicitly scope packages as non-public.
  3. Update README and signoff notes to match the chosen scope.
  4. Re-run legal/readiness text scans.
- Acceptance criteria: No public release document says distribution is both allowed and forbidden.
- Verification command or manual test: `rg -n "placeholder|must not be distributed publicly|WAIVED_BY_RELEASE_OWNER" EULA.md README.md docs/release/LEGAL_REVIEW_SIGNOFF.md`

## Phase 4 - Persistence/API/Storage/Data Integrity

### P4-01 - Route map save/load through project-root-aware save services

- Task ID: `P4-01`
- Title: Route map save/load through project-root-aware save services
- Files to edit: `engine/core/scene/map_scene.*`, `engine/core/save/*`
- Files to inspect: `apps/runtime/main.cpp`, `engine/core/save/runtime_save_startup.*`
- Dependencies: `P0-01`
- Risk level: High
- Exact implementation steps:
  1. Add project-root/settings context to `MapScene` save/load calls or delegate to runtime save service.
  2. Remove hardcoded relative `saves/slot_N.usr`.
  3. Ensure title continue and map save use the same slot catalog.
  4. Add round-trip tests.
- Acceptance criteria: Saves created from map scene are discoverable by title continue after restart.
- Verification command or manual test: `ctest --preset dev-all -R "save|runtime|RuntimeTitleScene|MapScene" --output-on-failure`

### P4-02 - Keep cloud sync hidden unless real provider exists

- Task ID: `P4-02`
- Title: Keep cloud sync hidden unless real provider exists
- Files to edit: `engine/core/social/cloud_service.h`, docs/UI that mention sync
- Files to inspect: AI sync coordinator, chatbot docs, README cloud references
- Dependencies: None
- Risk level: Medium
- Exact implementation steps:
  1. Audit surfaced cloud/sync claims.
  2. Mark `LocalInMemoryCloudService` as test/dev-only in UI-facing paths.
  3. Add guards preventing production UI from presenting it as remote sync.
  4. Optionally add a future provider-interface task for real implementation.
- Acceptance criteria: No production-visible flow claims cross-device or remote persistence using process-local memory.
- Verification command or manual test: `rg -n "cloud|sync|LocalInMemoryCloudService|cross-device" README.md docs engine editor`

## Phase 5 - Asset, Manifest, Packaging, And Release-Readiness Fixes

### P5-01 - Replace muted audio fallback with governed release audio or explicit silent scope

- Task ID: `P5-01`
- Title: Replace muted audio fallback with governed release audio or explicit silent scope
- Files to edit: `content/fixtures/project_governance_fixture.json`, `imports/manifests/asset_bundles/*.json`, audio runtime/editor files if assets are promoted
- Files to inspect: `tools/ci/check_release_required_assets.ps1`, audio manifests/reports
- Dependencies: `P0-01`
- Risk level: High
- Exact implementation steps:
  1. Decide whether release scope includes audio.
  2. If yes, promote license-cleared non-LFS audio assets and wire them to UI/runtime surfaces.
  3. If no, update product docs to explicitly state silent release scope.
  4. Ensure release asset gate enforces the chosen policy.
- Acceptance criteria: Audio release state is intentional and validated, not a placeholder fallback by accident.
- Verification command or manual test: `./tools/ci/check_release_required_assets.ps1`

### P5-02 - Finalize package vendor/contact metadata

- Task ID: `P5-02`
- Title: Finalize package vendor/contact metadata
- Files to edit: `cmake/packaging.cmake`, release docs
- Files to inspect: generated CPack package metadata
- Dependencies: `P3-02`
- Risk level: High
- Exact implementation steps:
  1. Replace pending package contact with approved support/release contact.
  2. Confirm package vendor, homepage, and version metadata are final.
  3. Rebuild package smoke output.
- Acceptance criteria: Generated package metadata is storefront/release suitable.
- Verification command or manual test: `./tools/ci/check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke`

### P5-03 - Reconcile release asset placeholder visuals

- Task ID: `P5-03`
- Title: Reconcile release asset placeholder visuals
- Files to edit: `content/fixtures/project_governance_fixture.json`, `imports/manifests/asset_bundles/*.json`, runtime asset references
- Files to inspect: `imports/normalized/`, renderer snapshots, release asset docs
- Dependencies: `P2-03`
- Risk level: High
- Exact implementation steps:
  1. List all release-required surfaces still using prototype or placeholder assets.
  2. Either promote final-quality assets or explicitly downgrade release claim.
  3. Update map/title/battle asset references.
  4. Re-run asset and visual gates.
- Acceptance criteria: AAA release claim is not backed by prototype/placeholder art unless scope is explicitly non-final.
- Verification command or manual test: `ctest --preset dev-snapshot --output-on-failure`; then `./tools/ci/check_release_required_assets.ps1`

## Phase 6 - Tests, Regression Coverage, And Final Verification

### P6-01 - Add regression tests for fixed runtime data flows

- Task ID: `P6-01`
- Title: Add regression tests for fixed runtime data flows
- Files to edit: tests under `tests/unit`, `tests/integration`, `tests/snapshot`
- Files to inspect: changed runtime/editor modules from Phases 1 through 5
- Dependencies: `P1-01`, `P2-01`, `P3-01`, `P4-01`
- Risk level: High
- Exact implementation steps:
  1. Add chat text-entry tests.
  2. Add AI animation application tests.
  3. Add project-root save/load round-trip tests.
  4. Add release placeholder/missing-asset behavior tests.
- Acceptance criteria: Each fixed release blocker has a focused regression.
- Verification command or manual test: `ctest --preset dev-all --output-on-failure`

### P6-02 - Rerun complete local release gates

- Task ID: `P6-02`
- Title: Rerun complete local release gates
- Files to edit: none unless failures occur
- Files to inspect: failed logs and generated reports
- Dependencies: all prior tasks
- Risk level: Blocker
- Exact implementation steps:
  1. Configure clean debug and release builds.
  2. Build both presets.
  3. Run PR, export, spatial, snapshot, project-audit, presentation, install, package, and release-candidate gates.
  4. Fix failures or document true blockers.
- Acceptance criteria: All canonical local release gates pass without stale evidence.
- Verification command or manual test: `./tools/ci/run_local_gates.ps1`; then `./tools/ci/run_presentation_gate.ps1`; then `./tools/ci/run_release_candidate_gate.ps1`

### P6-03 - Produce final release readiness evidence

- Task ID: `P6-03`
- Title: Produce final release readiness evidence
- Files to edit: `docs/APP_RELEASE_READINESS_MATRIX.md`, `docs/release/AAA_RELEASE_READINESS_REPORT.md`, signoff docs
- Files to inspect: CI logs, package smoke output, project audit JSON
- Dependencies: `P6-02`
- Risk level: Blocker
- Exact implementation steps:
  1. Record exact commit SHA.
  2. Record exact commands and results.
  3. Record package artifact paths.
  4. Record legal, package, and contact decisions.
  5. Confirm `releaseBlockerCount: 0` only after real gates pass.
- Acceptance criteria: Release readiness docs match current executable evidence.
- Verification command or manual test: `./build/dev-ninja-debug/urpg_project_audit.exe --json`
