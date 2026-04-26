# AAA Release Readiness Execution Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Convert the verified AAA release-readiness audit into ordered implementation tasks that remove release blockers from the shipped runtime, editor, packaging, and validation paths.

**Architecture:** Fix release readiness from the outside in: first make the build, CLI, and validation gates trustworthy; then wire user-facing runtime/editor surfaces; then close implementation gaps, error states, persistence, packaging, and final regression coverage. Do not promote audit-gap claims to defects until the inspection task proves them.

**Tech Stack:** C++20, CMake 3.23+, Catch2 v3, PowerShell CI scripts, Git LFS, ImGui editor, SDL/headless runtime surfaces.

**Source Audit:** `docs/release/AAA_RELEASE_READINESS_REPORT.md`

---

## File-By-File Release Blockers And Incomplete Work

- `CMakeLists.txt`
  - Verified blocker: `project(urpg LANGUAGES C CXX)` has no project version.
  - Verified blocker: install rules only install `urpg_runtime`, `urpg_editor`, and `urpg_audio_smoke` to `bin`.
  - Verified blocker: Catch2 acquisition and test targets are not gated behind a dedicated test-build option.
  - Verified blocker: no verified `CPack` configuration.
  - Inspect before edit: lines around project declaration, options, dependency fetches, install rules, test target definitions, CTest labels.

- `apps/runtime/main.cpp`
  - Verified blocker: runtime boots directly into `MapScene("RuntimeBoot", 16, 12)`.
  - Verified blocker: no verified title/new/continue/options/exit startup flow.
  - Verified blocker: no verified save/load flow from the shipped entry point.
  - Verified blocker: no references to `AudioCore`, `AssetLoader`, `RuntimeBundleLoader`, `LocaleCatalog`, `PerfProfiler`, or `InputManager`.
  - Verified blocker: no `--help` or `--version`.
  - Verified blocker: unknown CLI options are ignored.
  - Verified blocker: non-headless loop has no verified frame cap in the entry point.
  - Verified blocker: no verified pause/focus handling in the entry point.
  - Verified blocker: startup failures are console-only.
  - Verified blocker: no verified runtime settings persistence.
  - Verified blocker: no verified runtime asset directory validation.

- `apps/editor/main.cpp`
  - Verified blocker: editor registers only `diagnostics`, `assets`, `ability`, `patterns`, `mod`, and `analytics` as top-level panels.
  - Verified blocker: smoke-required panel list omits `patterns` and does not cover compiled user-facing panels.
  - Verified blocker: no `--help` or `--version`.
  - Verified blocker: unknown CLI options are ignored.
  - Verified blocker: `ImGui::GetIO().IniFilename` and `LogFilename` are set to `nullptr`.
  - Verified gap: analytics objects are constructed, but first-run consent was not verified.

- `engine/core/presentation/release_validation.cpp`
  - Verified blocker: release validation uses `<cassert>` and `assert(...)`, so checks can compile out in `NDEBUG` builds.

- `editor/spatial/procedural_map_panel.cpp`
  - Verified blocker: lowercase `render()` implementation is empty.

- `editor/spatial/region_rules_panel.cpp`
  - Verified blocker: lowercase `render()` implementation is empty.

- `editor/**/*.cpp` and `editor/**/*.h`
  - Unverified audit gap: button handlers, disabled states, empty states, loading states, and error states have not been exhaustively audited.
  - Required verification: enumerate user-facing panels, inspect each interactive control, and record missing handlers/states before editing.

- `runtimes/compat_js/**`
  - Unverified audit gap: orphaned event listeners, repeated timers, and unhandled JavaScript promise rejection behavior have not been verified.
  - Required verification: inspect runtime timer/event/promise handling and add failing tests before making changes.

- `imports/staging/plugin_intake/UM7_TerrainMesh/`
  - Backlog intake candidate: `UM7_TerrainMesh.js` is an RPG Maker MZ UltraMode7 terrain mesh plugin.
  - Current status: not integrated, not release-blocking, license unverified.
  - Required verification before promotion: confirm source URL/license, decide whether support is JS-compat-only or native terrain mesh behavior, and add compatibility/native tests before implementation.

- `docs/`
  - Verified blocker: no root-level or docs-level `THIRD_PARTY_NOTICES.md`, `EULA.md`, `PRIVACY_POLICY.md`, `CREDITS.md`, or `CHANGELOG.md`.
  - Verified blocker: release readiness governance is split across docs without an app-level release matrix for shipped-product readiness.

- `third_party/`, `imports/`, `more assets/`, `.gitattributes`, `.gitignore`
  - Verified blocker: release validation depends on Git LFS hydration; fresh clone and CI LFS access are unverified while remote LFS budget was observed exhausted.
  - Verified mitigation already done: duplicate ZIP archives and duplicate asset tree removals were completed in prior cleanup.
  - Required verification: fresh clone or CI runner must prove required LFS objects hydrate.

- Platform metadata paths to create or inspect
  - Verified blocker: no app-level icon or platform metadata wiring was verified.
  - Required inspection: search for `.rc`, `.ico`, `.icns`, `.desktop`, manifests, bundle metadata, and install packaging references.

---

## Phase 0 - Build Blockers And App-Breaking Issues

### P0-001 - Add Project Version Metadata And Shared App Version Output

**Files to edit:**
- `CMakeLists.txt`
- `cmake/urpg_version.h.in`
- `engine/core/version.h`
- `apps/runtime/main.cpp`
- `apps/editor/main.cpp`
- `apps/audio_smoke/main.cpp`
- `CHANGELOG.md`

**Files to inspect:**
- `CMakePresets.json`
- `tools/ci/run_local_gates.ps1`
- Existing generated-header patterns under `cmake/`, `engine/`, and `tools/`

**Dependencies:** None.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Add `VERSION 0.1.0` to the root `project(urpg ...)` declaration.
- [x] Create `cmake/urpg_version.h.in` defining `URPG_VERSION_MAJOR`, `URPG_VERSION_MINOR`, `URPG_VERSION_PATCH`, and `URPG_VERSION_STRING`.
- [x] Add `configure_file(cmake/urpg_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/urpg_version.h @ONLY)`.
- [x] Add `${CMAKE_CURRENT_BINARY_DIR}/generated` to include paths for app targets that print version data.
- [x] Create `engine/core/version.h` with a small inline API that returns the generated version string.
- [x] Add `--version` output to `urpg_runtime`, `urpg_editor`, and `urpg_audio_smoke`.
- [x] Create `CHANGELOG.md` with an `Unreleased` section and the current release-readiness cleanup scope.
- [x] Run configure, build the three app targets, and run each executable with `--version`.

**Acceptance criteria:**
- `project(urpg VERSION 0.1.0 LANGUAGES C CXX)` or equivalent versioned declaration exists.
- All shipped executables print the same version string and exit `0` for `--version`.
- `CHANGELOG.md` exists and references unreleased release-readiness work.

**Verification command or manual test:**
- `cmake --preset dev-ninja-debug`
- `cmake --build --preset dev-debug --target urpg_runtime urpg_editor urpg_audio_smoke`
- `.\build\dev-ninja-debug\apps\runtime\urpg_runtime.exe --version`
- `.\build\dev-ninja-debug\apps\editor\urpg_editor.exe --version`
- `.\build\dev-ninja-debug\apps\audio_smoke\urpg_audio_smoke.exe --version`

**Verification evidence (2026-04-26):**
- `cmake --preset dev-ninja-debug` completed.
- `cmake --build --preset dev-debug --target urpg_runtime urpg_editor urpg_audio_smoke` is blocked by a pre-existing MinGW SDL `windres` path-with-space failure in `C:\dev\URPG Maker`.
- No-space headless verification completed with `cmake -S . -B C:\dev\urpg-build-ci-nospace -G Ninja -DCMAKE_BUILD_TYPE=Release -DURPG_SKIP_OPENGL=ON -DFETCHCONTENT_BASE_DIR=C:\dev\urpg-fetchcontent-nospace -DURPG_USE_SCCACHE=ON` and `cmake --build C:\dev\urpg-build-ci-nospace --target urpg_runtime urpg_editor urpg_audio_smoke`.
- `C:\dev\urpg-build-ci-nospace\urpg_runtime.exe --version` printed `URPG Runtime 0.1.0`.
- `C:\dev\urpg-build-ci-nospace\urpg_editor.exe --version` printed `URPG Editor 0.1.0`.
- `C:\dev\urpg-build-ci-nospace\urpg_audio_smoke.exe --version` printed `URPG Audio Smoke 0.1.0`.

### P0-002 - Replace Release Validation `assert()` With Runtime-Failing Checks

**Files to edit:**
- `engine/core/presentation/release_validation.cpp`

**Files to inspect:**
- `CMakeLists.txt`
- `tests/unit/*presentation*.cpp`
- `docs/presentation/VALIDATION.md`

**Dependencies:** None.

**Risk level:** High.

**Exact implementation steps:**
- [x] Remove `#include <cassert>`.
- [x] Add a local validation helper that records failures with file/section context and returns a non-zero process exit code.
- [x] Replace each `assert(condition)` with `check(condition, "specific failure message")`.
- [x] Ensure success prints a concise pass summary.
- [x] Ensure any failed check prints all failure messages before returning non-zero.
- [x] Build and run `urpg_presentation_release_validation` in Debug and Release configurations.

**Acceptance criteria:**
- No `assert(` remains in `engine/core/presentation/release_validation.cpp`.
- Release builds fail if a validation condition fails.
- The validation executable produces actionable diagnostics instead of silent assertion behavior.

**Verification command or manual test:**
- `rg -n "assert\\(|<cassert>" engine/core/presentation/release_validation.cpp`
- `cmake --build --preset dev-debug --target urpg_presentation_release_validation`
- `.\build\dev-ninja-debug\urpg_presentation_release_validation.exe`
- `cmake --build --preset dev-release --target urpg_presentation_release_validation`
- `.\build\dev-ninja-release\urpg_presentation_release_validation.exe`

**Verification evidence (2026-04-26):**
- `rg -n "assert\\(|<cassert>" engine/core/presentation/release_validation.cpp` returned no matches.
- `cmake --build --preset dev-debug --target urpg_presentation_release_validation` completed.
- `.\build\dev-ninja-debug\urpg_presentation_release_validation.exe` printed `Release Validation Suite: ALL TESTS PASSED`.
- `cmake --build --preset dev-release --target urpg_presentation_release_validation` completed after forcing vendored SDL's `WINDRES` cache variable to the path-safe MinGW wrapper.
- `.\build\dev-ninja-release\urpg_presentation_release_validation.exe` printed `Release Validation Suite: ALL TESTS PASSED`.

### P0-003 - Make Runtime And Editor CLI Parsing Strict

**Files to edit:**
- `apps/runtime/main.cpp`
- `apps/editor/main.cpp`
- Optional if repeated logic grows: `engine/core/app_cli.h`
- Tests under `tests/unit/` or `tests/integration/`

**Files to inspect:**
- `apps/audio_smoke/main.cpp`
- Existing CLI parsing tests under `tests/`

**Dependencies:** P0-001 for `--version` output.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Extract runtime option parsing into a testable function if it currently only exists inside `main`.
- [x] Extract editor option parsing into a testable function if it currently only exists inside `main`.
- [x] Add `--help` output for runtime and editor listing every supported option and expected value.
- [x] Return non-zero for unknown flags.
- [x] Return non-zero for missing values after flags such as `--frames`, `--width`, `--height`, and `--project-root`.
- [x] Preserve existing successful behavior for valid options.
- [x] Add tests for help, version, unknown flag, missing value, and valid option parsing.

**Acceptance criteria:**
- `urpg_runtime --help` exits `0` and lists supported options.
- `urpg_editor --help` exits `0` and lists supported options.
- Unknown or incomplete options exit non-zero and print the invalid argument.
- Existing headless runtime launch still works.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_editor urpg_tests`
- `.\build\dev-ninja-debug\apps\runtime\urpg_runtime.exe --help`
- `.\build\dev-ninja-debug\apps\runtime\urpg_runtime.exe --bad-option`
- `.\build\dev-ninja-debug\apps\editor\urpg_editor.exe --help`
- `ctest --preset dev-all -R "cli|runtime|editor"`

**Verification evidence (2026-04-26):**
- `cmake --build --preset dev-debug --target urpg_tests urpg_runtime urpg_editor` completed.
- `.\build\dev-ninja-debug\urpg_tests.exe "[cli]"` passed 8 test cases and 535 assertions.
- `.\build\dev-ninja-debug\urpg_runtime.exe --help` exited `0` and listed runtime options.
- `.\build\dev-ninja-debug\urpg_runtime.exe --version` exited `0` and printed `URPG Runtime 0.1.0`.
- `.\build\dev-ninja-debug\urpg_runtime.exe --bogus` exited `2` and printed `unknown option: --bogus`.
- `.\build\dev-ninja-debug\urpg_editor.exe --help` exited `0` and listed editor options.
- `.\build\dev-ninja-debug\urpg_editor.exe --version` exited `0` and printed `URPG Editor 0.1.0`.
- `.\build\dev-ninja-debug\urpg_editor.exe --width` exited `2` and printed `missing value after --width`.
- `.\build\dev-ninja-debug\urpg_runtime.exe --headless --frames 0` exited `0`.
- `.\build\dev-ninja-debug\urpg_editor.exe --headless --frames 0` exited `0`.
- `ctest --preset dev-all -R "Runtime CLI|Editor CLI" --output-on-failure` passed 6 tests.
- `ctest --preset dev-all -R "CLI|cli" --output-on-failure` is not a valid focused P0-003 check because it also matches unrelated Project Audit CLI tests; that broader run failed on existing governance report assertions in tests 548 and 563.

### P0-004 - Gate Test Dependencies Behind An Explicit Build Option

**Files to edit:**
- `CMakeLists.txt`
- `CMakePresets.json`

**Files to inspect:**
- `tools/ci/run_presentation_gate.ps1`
- `docs/presentation/VALIDATION.md`

**Dependencies:** None.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Add `option(URPG_BUILD_TESTS "Build URPG test executables" ON)`.
- [x] Gate Catch2 `FetchContent` or `find_package(Catch2)` behind `if(URPG_BUILD_TESTS)`.
- [x] Gate test executable definitions and `catch_discover_tests` calls behind `if(URPG_BUILD_TESTS)`.
- [x] Keep `urpg_presentation_release_validation` available if it is a release validation executable, or explicitly document and gate it if it is test-only.
- [x] Add a release preset or cache variable path with `URPG_BUILD_TESTS=OFF`.
- [x] Confirm CI presets keep tests enabled.

**Acceptance criteria:**
- Configuring with `-DURPG_BUILD_TESTS=OFF` does not fetch Catch2 and does not define Catch2-linked test targets.
- CI/local validation still builds and runs tests with `URPG_BUILD_TESTS=ON`.

**Verification command or manual test:**
- `cmake --preset release-no-tests`
- `ninja -C build\release-no-tests -t targets`
- `cmake --build --preset release-no-tests --target urpg_runtime urpg_editor urpg_presentation_release_validation`
- `cmake --preset dev-ninja-debug`
- `cmake --build --preset dev-debug --target urpg_tests`
- `ctest --test-dir build\release-no-tests -R urpg_presentation_release_validation --output-on-failure`
- `ctest --preset dev-all -R "Runtime CLI|Editor CLI" --output-on-failure`

**Verification evidence (2026-04-26):**
- `cmake --preset release-no-tests` completed from a clean `build\release-no-tests` directory with `URPG_BUILD_TESTS=OFF` and `URPG_FETCH_CATCH2=OFF`.
- `ninja -C build\release-no-tests -t targets` listed `urpg_runtime`, `urpg_editor`, and `urpg_presentation_release_validation`; it did not list `urpg_tests`, `urpg_integration_tests`, `urpg_snapshot_tests`, `urpg_compat_tests`, or Catch2 targets.
- `Select-String -Path build\release-no-tests\CMakeCache.txt -Pattern "URPG_BUILD_TESTS|URPG_FETCH_CATCH2|Catch2"` showed `URPG_BUILD_TESTS:BOOL=OFF` and `URPG_FETCH_CATCH2:BOOL=OFF`.
- `cmake --build --preset release-no-tests --target urpg_runtime urpg_editor urpg_presentation_release_validation` completed.
- `build\release-no-tests\urpg_runtime.exe --version` printed `URPG Runtime 0.1.0`.
- `build\release-no-tests\urpg_editor.exe --version` printed `URPG Editor 0.1.0`.
- `build\release-no-tests\urpg_presentation_release_validation.exe` printed `Release Validation Suite: ALL TESTS PASSED`.
- `ctest --test-dir build\release-no-tests -R urpg_presentation_release_validation --output-on-failure` passed 1 test.
- `cmake --preset dev-ninja-debug` completed with default `URPG_BUILD_TESTS=ON`.
- `cmake --build --preset dev-debug --target urpg_tests` completed.
- `build\dev-ninja-debug\urpg_tests.exe "[cli]"` passed 8 test cases and 535 assertions.
- `ctest --preset dev-all -R "Runtime CLI|Editor CLI" --output-on-failure` passed 6 tests.
- During verification, a clean MinGW fallback build in `C:\dev\URPG Maker` hit SDL's raw `windres` path-with-space failure. `CMakeLists.txt` now removes vendored SDL Windows `.rc` resource sources for MinGW fallback builds under space-containing build paths, and the same clean `release-no-tests` build then completed.

---

## Phase 1 - Unwired UI, Routes, Handlers, And Feature Surfaces

### P1-001 - Replace Hardcoded Runtime Map Boot With A Real Startup Flow

**Files to edit:**
- `apps/runtime/main.cpp`
- `engine/core/scene/runtime_title_scene.h`
- `engine/core/scene/runtime_title_scene.cpp`
- `tests/unit/test_scene_manager.cpp`
- `CMakeLists.txt`

**Files to inspect:**
- `engine/core/scene/*`
- `engine/core/save/*`
- `engine/core/ui/*`
- Existing `MapScene`, scene manager, menu, and title/menu-like classes.

**Dependencies:** P0-003.

**Risk level:** High.

**Exact implementation steps:**
- [x] Inspect existing scene classes for title, menu, options, save slot, and map transition support.
- [x] If no title scene exists, add a minimal `RuntimeTitleScene` with explicit actions for New Game, Continue, Options, and Exit.
- [x] Replace direct `MapScene("RuntimeBoot", 16, 12)` construction in `apps/runtime/main.cpp` with startup scene construction.
- [x] Wire New Game to start the existing map path.
- [x] Wire Exit to request runtime shutdown through the existing shell/scene mechanism.
- [x] Add a headless smoke path that can run a fixed number of frames without requiring manual UI input.
- [x] Add tests proving the runtime startup stack starts with the title flow instead of the hardcoded map.

**Acceptance criteria:**
- Runtime no longer clears directly into a hardcoded `RuntimeBoot` map in normal startup.
- New Game reaches a playable map or documented runtime scene.
- Continue and Options are visible or explicitly disabled with reason text until their backing tasks land.
- Headless smoke still exits after `--frames`.

**Verification command or manual test:**
- `rg -n "RuntimeBoot|MapScene\\(" apps/runtime/main.cpp`
- `cmake --build --preset dev-debug --target urpg_runtime urpg_tests urpg_integration_tests`
- `.\build\dev-ninja-debug\apps\runtime\urpg_runtime.exe --headless --frames 3`
- Manual non-headless test unverified until a developer confirms title/new/exit visually.

**Verification evidence (2026-04-26):**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_tests urpg_integration_tests` completed.
- `build\dev-ninja-debug\urpg_tests.exe "[scene][runtime][title]"` passed 5 test cases and 50 assertions.
- `ctest --preset dev-all -R "RuntimeTitleScene|runtime title|SceneManager: Basic" --output-on-failure` passed 6 tests.
- `build\dev-ninja-debug\urpg_runtime.exe --headless --frames 3` exited `0` and printed `URPG runtime exited after 3 frame(s).`
- `rg -n "RuntimeBoot|MapScene\(" apps\runtime\main.cpp engine\core\scene\runtime_title_scene.h engine\core\scene\runtime_title_scene.cpp tests\unit\test_scene_manager.cpp` shows `RuntimeBoot` only in the New Game transition callback and its unit test, not as the initial runtime scene.
- Manual non-headless visual confirmation remains unverified; it requires launching the runtime with a window and selecting New Game and Exit interactively.

### P1-002 - Wire Runtime Save/Load Into Startup Continue Flow

**Files to edit:**
- `apps/runtime/main.cpp`
- Runtime startup scene files from P1-001
- Existing or new files under `engine/core/save/`
- Tests under `tests/integration/`

**Files to inspect:**
- `engine/core/save/*`
- `tests/unit/*save*.cpp`
- `tests/integration/*save*.cpp`
- Save recovery docs and save runtime APIs.

**Dependencies:** P1-001.

**Risk level:** High.

**Exact implementation steps:**
- [x] Identify the canonical save metadata API and save slot load API.
- [x] Add startup save-slot discovery using `RuntimeOptions.project_root` or the configured save directory.
- [x] Make Continue enabled only when at least one valid save exists.
- [x] Make Continue load the newest valid save or open a slot selector if one exists.
- [x] On corrupted save, show or log a structured failure and fall back to recovery flow if one exists.
- [x] Add integration tests for no saves, valid save, and corrupted save metadata.

**Acceptance criteria:**
- Runtime startup can distinguish no-save and has-save states.
- Continue path is covered by automated tests.
- Corrupt save failure is actionable and does not crash the runtime.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_integration_tests`
- `ctest --preset dev-all -R "save|runtime"`
- Manual save-slot selection is unverified until exercised in the running editor/runtime.

**Verification evidence (2026-04-26):**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_tests urpg_integration_tests` completed.
- `build\dev-ninja-debug\urpg_integration_tests.exe "[integration][runtime][save]"` passed 4 test cases and 39 assertions.
- `build\dev-ninja-debug\urpg_tests.exe "[scene][runtime][title]"` passed 5 test cases and 50 assertions.
- `build\dev-ninja-debug\urpg_runtime.exe --headless --frames 3` exited `0` and printed `URPG runtime exited after 3 frame(s).`
- Initial `ctest --preset dev-all -R "Runtime startup save|RuntimeTitleScene|save|runtime" --output-on-failure` exposed a stale documentation test path for `docs/specs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md`; the lookup was updated.
- Final `ctest --preset dev-all -R "Runtime startup save|RuntimeTitleScene|save|runtime" --output-on-failure` passed 130 tests with 0 failures.
- Manual save-slot selection remains unverified because the current implementation loads the newest valid slot directly; selector UI does not exist yet.

### P1-003 - Prove Or Wire Runtime Startup Subsystems

**Files to edit:**
- `apps/runtime/main.cpp`
- Existing subsystem initialization files under `engine/core/audio/`, `engine/core/assets/`, `engine/core/input/`, `engine/core/localization/`, `engine/core/profiling/`
- Tests under `tests/integration/`

**Files to inspect:**
- `engine/core/audio/*`
- `engine/core/assets/*`
- `engine/core/input/*`
- `engine/core/localization/*`
- `engine/core/profiling/*`
- `apps/audio_smoke/main.cpp`

**Dependencies:** P0-003.

**Risk level:** High.

**Exact implementation steps:**
- [x] Inspect each subsystem for its intended startup API and ownership model.
- [x] Add a `RuntimeStartupServices` object or equivalent local startup sequence if no central startup object exists.
- [x] Initialize or explicitly skip with documented reason: audio, asset bundle validation, localization catalog, profiler, and input manager.
- [x] Ensure startup failures use Phase 3 diagnostics, not raw crashes.
- [x] Add smoke tests proving each required subsystem is reached during runtime startup.

**Acceptance criteria:**
- The runtime entry path either initializes each listed subsystem or references a verified downstream initializer.
- Missing asset/localization/audio config produces targeted diagnostics.
- Tests fail if startup stops invoking a required subsystem.

**Verification command or manual test:**
- `rg -n "AudioCore|AssetLoader|RuntimeBundleLoader|LocaleCatalog|PerfProfiler|InputManager" apps/runtime engine/core`
- `cmake --build --preset dev-debug --target urpg_runtime urpg_integration_tests`
- `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler"`

**Verification evidence (2026-04-26):**
- `rg -n "AudioCore|AssetLoader|RuntimeBundleLoader|LocaleCatalog|PerfProfiler|InputManager" apps/runtime engine/core` shows runtime startup coverage through `engine/core/runtime_startup_services.cpp` and `engine/core/engine_shell.h`.
- `cmake --build --preset dev-debug --target urpg_runtime urpg_tests urpg_integration_tests` completed.
- `build\dev-ninja-debug\urpg_integration_tests.exe "[integration][runtime][startup]"` passed 4 test cases and 25 assertions.
- `build\dev-ninja-debug\urpg_tests.exe "[input]"` passed 21 test cases and 103 assertions.
- `build\dev-ninja-debug\urpg_runtime.exe --headless --frames 3` exited `0` and printed the targeted missing-locale startup warning.
- Initial broad CTest surfaced that `imports/normalized/ui_sfx/kenney_click_001.wav` is a Git LFS pointer because the remote LFS budget is exceeded; the export test now accepts either a hydrated `RIFF` WAV or an explicit LFS pointer with `oid`/`size` evidence.
- Final `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` passed 121 tests with 0 failures.

### P1-004 - Register Intended Top-Level Editor Panels Through A Discoverable Navigation Model

**Status:** Completed on 2026-04-26.

**Files to edit:**
- `apps/editor/main.cpp`
- Existing editor panel registry or shell files under `editor/`
- Tests under `tests/unit/` or `tests/integration/`

**Files to inspect:**
- `editor/**/*panel.h`
- `editor/**/*panel.cpp`
- `editor/diagnostics/*`
- `CMakeLists.txt` editor source lists

**Dependencies:** P0-003.

**Risk level:** High.

**Exact implementation steps:**
- [x] Generate a list of compiled editor panels from `CMakeLists.txt` and `editor/**/*panel.*`.
- [x] Classify each panel as top-level, nested workspace-only, internal, or intentionally disabled.
- [x] Add a single editor panel registry table with ID, title, category, factory, and exposure status.
- [x] Register every top-level panel through `EditorShell::addPanel`.
- [x] Add an allowlist for internal or intentionally hidden panels with explicit reason strings.
- [x] Ensure `patterns` remains registered and covered.

**Acceptance criteria:**
- Every user-facing compiled panel is reachable from top-level navigation or documented as nested/internal.
- The registry is the single source of truth for panel smoke coverage.
- No panel is silently omitted.

**Verification command or manual test:**
- `rg -n "addPanel|requiredPanelIds|PanelDescriptor|panel registry" apps/editor editor tests`
- `cmake --build --preset dev-debug --target urpg_editor urpg_tests`
- Manual editor navigation verification is unverified until a developer launches `urpg_editor` and confirms every top-level category.

**Verification results:**
- `rg -n "addPanel|requiredPanelIds|PanelDescriptor|panel registry" apps/editor editor tests` confirmed top-level shell registration now flows through the registry and registry tests.
- `cmake --build --preset dev-debug --target urpg_editor urpg_tests` passed.
- `build\dev-ninja-debug\urpg_tests.exe "[editor][panel][registry]"` passed 3 test cases and 436 assertions.
- `build\dev-ninja-debug\urpg_editor.exe --headless --smoke --project-root .\content\fixtures\editor_smoke_project --smoke-output .\build\dev-ninja-debug\editor_smoke\editor_state.json --smoke-snapshot-root .\build\dev-ninja-debug\editor_smoke\snapshots` passed and wrote an error-free smoke report.
- `ctest --preset dev-all -R "editor|panel|smoke" --output-on-failure` passed 34 tests with 0 failures.
- Manual editor navigation remains unverified until a developer launches the graphical editor.

### P1-005 - Expand Editor Smoke Coverage To All Registered Panels

**Status:** Completed on 2026-04-26.

**Files to edit:**
- `apps/editor/main.cpp`
- Editor smoke tests under `tests/`

**Files to inspect:**
- Existing editor smoke test code.
- Panel registry from P1-004.

**Dependencies:** P1-004.

**Risk level:** Medium.

**Exact implementation steps:**
- [x] Replace hardcoded smoke-required panel IDs with IDs from the registry.
- [x] Include `patterns` in smoke coverage immediately.
- [x] Add a test that fails if a registered top-level panel is missing from the smoke-required list.
- [x] Add a test that fails if an intentionally hidden panel lacks a reason.
- [x] Run editor-focused tests.

**Acceptance criteria:**
- Smoke coverage covers all registered top-level panels.
- Hidden/internal panels are explicit and reviewed.
- Adding a new panel without coverage fails tests.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_tests`
- `ctest --preset dev-all -R "editor|panel|smoke"`
- `.\tools\ci\run_presentation_gate.ps1`

**Verification results:**
- `rg -n "smokeRequiredEditorPanelIds|requiredTopLevelPanelIds|patterns" apps/editor engine/core/editor tests/unit/test_editor_panel_registry.cpp` confirmed smoke coverage is registry-backed and includes `patterns`.
- `cmake --build --preset dev-debug --target urpg_editor urpg_tests` passed.
- `build\dev-ninja-debug\urpg_tests.exe "[editor][panel][registry]"` passed 4 test cases and 438 assertions.
- `ctest --preset dev-all -R "editor|panel|smoke" --output-on-failure` passed 35 tests with 0 failures.
- `.\tools\ci\run_presentation_gate.ps1` passed presentation and visual regression gates.

### P1-006 - Implement Or Explicitly Disable Empty Spatial Panel Render Paths

**Files to edit:**
- `editor/spatial/procedural_map_panel.cpp`
- `editor/spatial/region_rules_panel.cpp`
- Spatial editor tests under `tests/`

**Files to inspect:**
- `editor/spatial/*.h`
- `editor/spatial/*.cpp`
- Spatial editor model/state classes.

**Dependencies:** P1-004.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Inspect the uppercase/lowercase render conventions used by spatial panels.
- [ ] If the lowercase `render()` path is expected to draw UI, implement visible controls or a disabled state with the exact reason.
- [ ] If lowercase `render()` is obsolete, remove it or forward it to the canonical render method.
- [ ] Add smoke coverage that calls the path used by editor navigation.
- [ ] Confirm no empty user-facing render path remains.

**Acceptance criteria:**
- `procedural_map_panel.cpp` and `region_rules_panel.cpp` no longer expose empty user-facing render functions.
- Smoke tests prove each panel can draw a visible or intentionally disabled state.

**Verification command or manual test:**
- `rg -n "void .*::render\\(\\)\\s*\\{\\s*\\}" editor/spatial`
- `cmake --build --preset dev-debug --target urpg_tests`
- `ctest --preset dev-all -R "spatial|editor"`

---

## Phase 2 - Incomplete Implementations, Stubs, Placeholders, Mock Data

### P2-001 - Audit And Replace User-Facing Editor Stubs

**Files to edit:**
- Files discovered by the inspection under `editor/`
- Tests under `tests/unit/` or `tests/integration/`

**Files to inspect:**
- `editor/**/*.cpp`
- `editor/**/*.h`

**Dependencies:** P1-004, P1-005.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Run searches for empty render functions, placeholder labels, mock data strings, disabled buttons with no reason, and no-op handlers.
- [ ] Record each verified stub in `docs/release/RELEASE_READINESS_MATRIX.md` or the app-level matrix from P5-005.
- [ ] For each user-facing stub, either implement the real behavior or convert it to an explicit disabled state with owner, reason, and unlock condition.
- [ ] Add a focused test for each implemented behavior or disabled-state contract.
- [ ] Do not edit audit-gap items until the search proves the specific file and behavior.

**Acceptance criteria:**
- No user-facing panel has a silent no-op, empty view, or placeholder data path unless documented as intentionally disabled.
- Every changed panel has a regression test or smoke assertion.

**Verification command or manual test:**
- `rg -n "TODO|TBD|placeholder|mock|stub|coming soon|not implemented|\\{\\s*\\}" editor`
- `cmake --build --preset dev-debug --target urpg_tests`
- `ctest --preset dev-all -R "editor"`

### P2-002 - Verify Editor Button Handlers And Disabled States

**Files to edit:**
- Files discovered by this task under `editor/`
- Tests under `tests/`

**Files to inspect:**
- `editor/**/*.cpp`
- `editor/**/*.h`

**Dependencies:** P1-004, P2-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Enumerate every `ImGui::Button`, menu item, checkbox, combo, drag, slider, and selectable control in user-facing panels.
- [ ] For each command button, confirm there is a handler that mutates state, opens a modal, dispatches a command, or is visibly disabled.
- [ ] For every disabled control, confirm the UI shows a reason or tooltip.
- [ ] Fix verified no-op handlers one at a time with tests.
- [ ] Mark this task unverified until the enumeration is committed as an artifact.

**Acceptance criteria:**
- Every user-facing editor command has an observable behavior or explicit disabled reason.
- A control inventory artifact exists for reviewer use.

**Verification command or manual test:**
- `rg -n "ImGui::(Button|MenuItem|Checkbox|Combo|Selectable|Slider|Drag)" editor`
- Manual verification is required for visual disabled-state affordances in `urpg_editor`.

### P2-003 - Verify QuickJS Runtime Timer, Event, And Promise Gaps

**Files to edit:**
- Files discovered under `runtimes/compat_js/`
- Tests under `tests/compat/`

**Files to inspect:**
- `runtimes/compat_js/**`
- `tests/compat/**`

**Dependencies:** None.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Inspect timer registration, event listener registration, and promise rejection handling in the QuickJS compatibility runtime.
- [ ] Add tests for listener removal, repeated timer cleanup, runtime shutdown cleanup, and unhandled promise rejection reporting.
- [ ] Implement fixes only for failing or missing behavior proven by tests.
- [ ] Add diagnostics for rejected promises that include plugin/source context where available.

**Acceptance criteria:**
- Timer/listener resources do not leak across runtime teardown.
- Unhandled promise rejections are surfaced as diagnostics.
- Compat tests cover the verified behavior.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_compat_tests`
- `ctest -L weekly`
- This remains unverified until the inspection and tests prove whether the audit-gap claims are real defects.

### P2-004 - Evaluate UM7 Terrain Mesh Plugin For Future Compatibility Or Native Support

**Files to edit:**
- `imports/staging/plugin_intake/UM7_TerrainMesh/manifest.json`
- Optional if promoted later: `tests/compat/fixtures/plugins/UM7_TerrainMesh.json`
- Optional if native support is approved later: files under `engine/core/presentation/`, `engine/core/scene/`, and `editor/spatial/`

**Files to inspect:**
- `imports/staging/plugin_intake/UM7_TerrainMesh/UM7_TerrainMesh.js`
- `runtimes/compat_js/**`
- `tests/compat/fixtures/plugins/**`
- `engine/core/presentation/**`
- `editor/spatial/**`

**Dependencies:** P2-003 for compatibility-runtime behavior, P5-002 for third-party notices/license handling.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Verify the plugin source URL, license, and redistribution terms.
- [ ] Confirm whether UltraMode7 itself is available, licensed, and intended as a compatibility target.
- [ ] Classify desired support as JavaScript plugin compatibility, native terrain mesh feature, or both.
- [ ] If JavaScript compatibility is desired, add a compat fixture manifest that records globals patched by the plugin and expected dependency handling.
- [ ] If native support is desired, write a focused design note for terrain-tag elevation, region passability overrides, vehicle/airship elevation behavior, and editor preview implications before touching engine code.
- [ ] Do not move the plugin out of `imports/staging/plugin_intake/` until licensing and support classification are complete.

**Acceptance criteria:**
- The plugin has verified provenance and license status before promotion.
- UltraMode7 dependency handling is explicit.
- Any implementation path has tests defined before code changes begin.
- The plugin remains non-release-blocking unless the release owner explicitly promotes it into scope.

**Verification command or manual test:**
- `Get-Content imports/staging/plugin_intake/UM7_TerrainMesh/manifest.json`
- `rg -n "UM7_TerrainMesh|UltraMode7|um7_terrain_mesh" imports docs tests runtimes engine editor`
- Promotion remains unverified until source/license and implementation path are approved.

---

## Phase 3 - Error Handling, Validation, Loading States, Empty States

### P3-001 - Add Structured Startup Diagnostics And Persistent Logs

**Files to edit:**
- `apps/runtime/main.cpp`
- `apps/editor/main.cpp`
- Existing or new logging files under `engine/core/diagnostics/`
- Tests under `tests/unit/` or `tests/integration/`

**Files to inspect:**
- `engine/core/diagnostics/*`
- `editor/diagnostics/*`
- Existing logging, report, or crash handling utilities.

**Dependencies:** P0-003.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Identify existing diagnostics/logging APIs and reuse them.
- [ ] Add a startup diagnostics sink for runtime and editor.
- [ ] Write startup failures to a stable per-project or per-user log path.
- [ ] For non-headless app startup, route critical startup failures to a visible error surface if the surface can be initialized.
- [ ] For headless mode, print the log path and structured error code to stderr.
- [ ] Add tests for missing project root and invalid startup config.

**Acceptance criteria:**
- Startup failures are not console-only.
- The user gets an actionable message and a log path.
- CI can assert specific error codes/messages.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_editor urpg_tests`
- `.\build\dev-ninja-debug\apps\runtime\urpg_runtime.exe --headless --project-root C:\definitely-missing-urpg-root --frames 1`
- `ctest --preset dev-all -R "diagnostic|startup|error"`

### P3-002 - Validate Runtime Project And Asset Directories Before Startup

**Files to edit:**
- `apps/runtime/main.cpp`
- Existing asset/runtime bundle validation files under `engine/core/assets/`
- Tests under `tests/integration/`

**Files to inspect:**
- `engine/core/assets/*`
- `imports/`
- Runtime project manifest conventions.

**Dependencies:** P1-003, P3-001.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Define the minimum required runtime project files and directories.
- [ ] Add preflight validation before scene startup.
- [ ] Return non-zero with diagnostics if required directories or manifests are missing.
- [ ] Add a test fixture for valid minimal project data.
- [ ] Add a test fixture for missing assets/manifests.

**Acceptance criteria:**
- Missing project assets fail early with named missing paths.
- Valid minimal project data reaches runtime startup.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_integration_tests`
- `ctest --preset dev-all -R "asset|runtime|preflight"`

### P3-003 - Add Editor Empty, Loading, And Error State Coverage

**Files to edit:**
- Files discovered under `editor/`
- Tests under `tests/`

**Files to inspect:**
- `editor/**/*.cpp`
- `editor/**/*.h`

**Dependencies:** P1-004, P2-002.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] For every top-level panel, identify data dependencies and async/loading paths.
- [ ] Add explicit empty state text for no data.
- [ ] Add explicit loading state for deferred or background operations.
- [ ] Add explicit error state for failed load/parse/validation.
- [ ] Add tests or smoke assertions for the highest-risk panels first: assets, save, export, analytics, mod manager, battle, and spatial.

**Acceptance criteria:**
- Top-level panels do not render blank content for empty/error/loading states.
- Error states include a concrete remediation or log reference.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_tests`
- `ctest --preset dev-all -R "editor|empty|loading|error"`
- Manual verification remains required for visual quality in `urpg_editor`.

---

## Phase 4 - Persistence/API/Storage/Data Integrity

### P4-001 - Add Runtime And Editor Settings Persistence

**Files to edit:**
- `apps/runtime/main.cpp`
- `apps/editor/main.cpp`
- Existing or new settings files under `engine/core/settings/` or nearest existing config namespace
- Tests under `tests/unit/`

**Files to inspect:**
- Existing config/settings code under `engine/`, `editor/`, and `tools/`
- `nlohmann::json` usage patterns.

**Dependencies:** P3-001.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Locate or create a settings store using existing JSON conventions.
- [ ] Persist runtime window size, fullscreen/window mode if supported, audio volume, input mapping path, and accessibility preferences.
- [ ] Persist editor ImGui settings to a project-local or user-local path instead of `nullptr`.
- [ ] Persist editor layout/workspace settings if a custom workspace store exists.
- [ ] Add migration/default behavior for missing or malformed settings files.
- [ ] Add tests for load defaults, save, reload, and malformed JSON recovery.

**Acceptance criteria:**
- Runtime/editor settings survive restart.
- Malformed settings do not crash startup.
- ImGui settings persistence is intentionally configured or replaced by tested custom persistence.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_runtime urpg_editor urpg_tests`
- `ctest --preset dev-all -R "settings|persistence|imgui"`
- Manual restart verification is required for visible editor layout persistence.

### P4-002 - Add Analytics Privacy Consent And Disable Path

**Files to edit:**
- `apps/editor/main.cpp`
- Analytics/privacy files under `editor/` or `engine/core/analytics/`
- Tests under `tests/unit/`
- `PRIVACY_POLICY.md`

**Files to inspect:**
- `editor/**/*analytics*`
- `engine/**/*analytics*`
- Existing privacy controller APIs.

**Dependencies:** P4-001.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Inspect the analytics dispatcher/uploader/privacy controller wiring in `apps/editor/main.cpp`.
- [ ] Ensure analytics upload is disabled by default until consent is recorded.
- [ ] Add a first-run consent prompt or settings panel control.
- [ ] Add a persistent opt-out path.
- [ ] Document collected fields in `PRIVACY_POLICY.md`.
- [ ] Add tests for default-disabled, opt-in, opt-out, and persisted preference.

**Acceptance criteria:**
- No analytics upload can occur before explicit consent.
- User can disable analytics after opting in.
- Privacy policy matches implemented data flow.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_editor urpg_tests`
- `ctest --preset dev-all -R "analytics|privacy|consent"`
- Manual first-run prompt verification remains required in `urpg_editor`.

### P4-003 - Prove Save Data Integrity Through Runtime Continue Path

**Files to edit:**
- Runtime save/load files touched in P1-002
- Tests under `tests/integration/` and `tests/unit/`

**Files to inspect:**
- `engine/core/save/*`
- `tests/unit/*save*.cpp`
- `tests/integration/*save*.cpp`

**Dependencies:** P1-002, P3-001.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Add or extend tests for valid save, corrupt metadata, corrupt payload, autosave fallback, and safe skeleton fallback if supported.
- [ ] Ensure runtime Continue surfaces recovery result to the user or log.
- [ ] Ensure save write failures return actionable diagnostics.
- [ ] Confirm save format version/checksum behavior is exercised by tests.

**Acceptance criteria:**
- Continue cannot silently load corrupt state.
- Recovery path is deterministic and tested.
- Save write failures are reported.

**Verification command or manual test:**
- `cmake --build --preset dev-debug --target urpg_tests urpg_integration_tests`
- `ctest --preset dev-all -R "save|recovery|checksum|autosave"`

---

## Phase 5 - Asset, Manifest, Packaging, And Release-Readiness Fixes

### P5-001 - Complete Install Rules For A Distributable App Layout

**Files to edit:**
- `CMakeLists.txt`
- Packaging/install docs under `docs/`
- Tests or scripts under `tools/ci/`

**Files to inspect:**
- `imports/`
- `third_party/`
- Runtime shader/plugin/template/data directories.
- License files for vendored dependencies.

**Dependencies:** P0-001, P1-003, P3-002.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Define the expected installed directory layout for runtime, editor, data, project templates, plugins, shaders, licenses, and notices.
- [ ] Add `install(FILES ...)` and `install(DIRECTORY ...)` rules for required non-binary runtime data.
- [ ] Add install components if the project distinguishes runtime, editor, dev, and docs.
- [ ] Add a CI script or CTest that runs `cmake --install` into a temporary directory.
- [ ] Validate the installed runtime can start headless from the install directory.

**Acceptance criteria:**
- `cmake --install` produces a runnable app layout.
- Required data, notices, and runtime files are present in the install tree.

**Verification command or manual test:**
- `cmake --build --preset dev-release`
- `cmake --install build/dev-release --prefix build/install-smoke`
- `.\build\install-smoke\bin\urpg_runtime.exe --headless --frames 1`

### P5-002 - Create Required Legal And Release Documents

**Files to edit:**
- `THIRD_PARTY_NOTICES.md`
- `EULA.md`
- `PRIVACY_POLICY.md`
- `CREDITS.md`
- `CHANGELOG.md`
- `docs/release/RELEASE_READINESS_MATRIX.md` or app-level matrix from P5-005

**Files to inspect:**
- `LICENSE`
- `third_party/**`
- `imports/**`
- `.gitattributes`
- Asset license manifests and intake docs.

**Dependencies:** P0-001, P4-002.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Inventory third-party code and asset sources from `third_party/`, `imports/`, and existing intake docs.
- [ ] Create `THIRD_PARTY_NOTICES.md` with dependency/source, license, path, and distribution notes.
- [ ] Create `CREDITS.md` covering engine, assets, plugins, and third-party packs where attribution is required.
- [ ] Create `EULA.md` suitable for the intended distribution mode, or mark it as internal-only if external legal review is required before publication.
- [ ] Complete `PRIVACY_POLICY.md` from P4-002 analytics behavior.
- [ ] Keep `CHANGELOG.md` current with release-readiness changes.

**Acceptance criteria:**
- Required docs exist and are referenced from packaging/install rules.
- Third-party notices cover all shipped third-party paths.
- Any legal text requiring external legal review is marked unverified with reviewer requirement.

**Verification command or manual test:**
- `Test-Path .\THIRD_PARTY_NOTICES.md, .\EULA.md, .\PRIVACY_POLICY.md, .\CREDITS.md, .\CHANGELOG.md`
- `python .\tools\assets\asset_hygiene.py --write-reports`
- Legal sufficiency remains unverified until reviewed by a qualified legal reviewer.

### P5-003 - Add CPack Or Document The Supported Packaging System

**Files to edit:**
- `CMakeLists.txt`
- `cmake/packaging.cmake`
- `docs/packaging.md`
- `tools/ci/run_local_gates.ps1`

**Files to inspect:**
- Existing deployment/package scripts under `tools/`
- CMake install rules from P5-001.

**Dependencies:** P5-001, P5-002.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Decide whether official packaging uses CPack.
- [ ] If using CPack, add `CPACK_PACKAGE_NAME`, version fields from the CMake project, vendor/contact fields, license/readme resources, and generator configuration.
- [ ] Include `include(CPack)` after install rules.
- [ ] If not using CPack, document the supported packaging command and add CI validation for it.
- [ ] Add a package smoke step to local gates or a dedicated packaging script.

**Acceptance criteria:**
- The repo has one documented, validated package creation path.
- Package metadata uses the same version as the app binaries.

**Verification command or manual test:**
- CPack path: `cmake --build --preset dev-release && cpack --config build/dev-release/CPackConfig.cmake`
- Non-CPack path: run the documented packaging command and verify produced artifact contents.

### P5-004 - Add Platform Metadata And App Icons

**Files to edit:**
- `CMakeLists.txt`
- Platform metadata files to create under `packaging/` or `resources/`
- Install/package docs

**Files to inspect:**
- Existing image/icon assets.
- Existing Windows `.rc`, manifest, macOS bundle, Linux desktop metadata files.

**Dependencies:** P5-001.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Search for existing `.ico`, `.icns`, `.png`, `.rc`, `.manifest`, and `.desktop` assets.
- [ ] Select or create app icons for runtime and editor.
- [ ] Add Windows resource metadata for product name, version, company/project name, and icon.
- [ ] Add Linux `.desktop` metadata if Linux packages are supported.
- [ ] Add macOS bundle metadata only if macOS app bundle packaging is supported.
- [ ] Include metadata files in install/package rules.

**Acceptance criteria:**
- Supported release platforms have explicit app identity metadata.
- Installed package includes icon and metadata resources.

**Verification command or manual test:**
- `rg -n "\\.ico|\\.icns|\\.desktop|VERSIONINFO|app manifest|CPACK" CMakeLists.txt packaging resources`
- Platform-specific visual verification remains unverified until tested on each supported OS.

### P5-005 - Prove Fresh Clone And LFS Asset Hydration

**Files to edit:**
- `.gitattributes`
- `.gitignore`
- `tools/assets/asset_hygiene.py` if checks need updates
- `docs/release/RELEASE_READINESS_MATRIX.md`

**Files to inspect:**
- `third_party/`
- `imports/`
- `more assets/`
- Git LFS tracking state

**Dependencies:** Previous asset cleanup already completed.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Run `git lfs ls-files` and capture required LFS object count and largest paths.
- [ ] Run asset hygiene and review oversize/duplicate reports.
- [ ] Perform a fresh clone with LFS enabled in a temporary directory or CI runner.
- [ ] Run `git lfs pull` in the fresh clone.
- [ ] Build or run a minimal asset preflight from the fresh clone.
- [ ] If LFS budget blocks hydration, either increase remote LFS capacity, move release assets to an accessible artifact store, or reduce required LFS footprint further.

**Acceptance criteria:**
- A fresh clone can hydrate required assets without relying on local cache.
- CI/release machines can access all required LFS objects.
- Duplicate archives remain ignored and untracked.

**Verification command or manual test:**
- `git lfs ls-files`
- `python .\tools\assets\asset_hygiene.py --write-reports`
- Fresh-clone verification is currently unverified until run outside the local cached repo.

### P5-006 - Create App-Level Release Readiness Matrix

**Files to edit:**
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `docs/release/AAA_RELEASE_READINESS_REPORT.md`
- `docs/status/PROGRAM_COMPLETION_STATUS.md`

**Files to inspect:**
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/release/KNOWN_BREAK_WAIVERS.md`
- `docs/status/PROGRAM_COMPLETION_STATUS.md`
- `docs/archive/planning/NATIVE_FEATURE_ABSORPTION_PLAN.md`

**Dependencies:** P1-P5 tasks can update this incrementally.

**Risk level:** Low.

**Exact implementation steps:**
- [ ] Create an app-level matrix with rows for boot flow, save/load, settings, audio, input, localization, asset validation, editor navigation, analytics consent, install/package, legal docs, LFS hydration, and final gates.
- [ ] For each row, include owner file, status, evidence command, and blocking task ID.
- [ ] Link the matrix from `AAA_RELEASE_READINESS_REPORT.md`.
- [ ] Update `PROGRAM_COMPLETION_STATUS.md` to reference the app-level matrix as the current release-readiness tracker.

**Acceptance criteria:**
- Release governance no longer depends on scattered docs alone.
- Every critical/high audit finding maps to a matrix row and task ID.

**Verification command or manual test:**
- `rg -n "APP_RELEASE_READINESS_MATRIX|P[0-6]-[0-9]{3}" docs`
- `.\tools\docs\check-presentation-doc-links.ps1` if it covers changed docs; otherwise run the repo markdown link checker if available.

---

## Phase 6 - Tests, Regression Coverage, And Final Verification

### P6-001 - Add End-To-End Release Candidate Gate Script

**Files to edit:**
- `tools/ci/run_release_candidate_gate.ps1`
- `tools/ci/run_local_gates.ps1`
- `.github/workflows/ci-gates.yml`
- `docs/APP_RELEASE_READINESS_MATRIX.md`

**Files to inspect:**
- `tools/ci/run_local_gates.ps1`
- `tools/ci/run_presentation_gate.ps1`
- `.github/workflows/ci-gates.yml`

**Dependencies:** P0-P5 tasks.

**Risk level:** Medium.

**Exact implementation steps:**
- [ ] Create a release-candidate gate script that configures release build, builds runtime/editor/validation targets, runs PR tests, runs presentation validation, runs install/package smoke, and checks required docs exist.
- [ ] Include LFS hydration check or skip only with an explicit failure message and waiver reference.
- [ ] Add the gate to CI as manual `workflow_dispatch`.
- [ ] Add matrix row evidence pointing to the gate.

**Acceptance criteria:**
- One command exercises the objective release-candidate exit criteria.
- CI can run the gate manually.
- The gate fails on missing docs, missing package output, failed validation, or broken tests.

**Verification command or manual test:**
- `.\tools\ci\run_release_candidate_gate.ps1`
- GitHub Actions verification remains unverified until the workflow is run remotely.

### P6-002 - Run Full Regression And Close The Readiness Report

**Files to edit:**
- `docs/release/AAA_RELEASE_READINESS_REPORT.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `docs/status/PROGRAM_COMPLETION_STATUS.md`
- `CHANGELOG.md`

**Files to inspect:**
- All files touched by P0-P6.
- CI logs.
- Fresh clone verification logs.

**Dependencies:** P6-001.

**Risk level:** High.

**Exact implementation steps:**
- [ ] Run `pre-commit run --all-files`.
- [ ] Run `.\tools\ci\run_local_gates.ps1`.
- [ ] Run `.\tools\ci\run_presentation_gate.ps1`.
- [ ] Run `.\tools\ci\run_release_candidate_gate.ps1`.
- [ ] Run fresh-clone LFS hydration and install/package smoke outside the local cached repo.
- [ ] Update the readiness report from `NOT RELEASE-READY` only if every critical finding is closed with evidence.
- [ ] Create a signed or annotated prerelease tag only after all gates pass.

**Acceptance criteria:**
- All critical findings are closed or explicitly waived with issue URL, owner, scope, and non-expired date.
- No high finding remains without a linked follow-up issue and release decision.
- A reproducible commit SHA, package artifact, and release notes exist.

**Verification command or manual test:**
- `pre-commit run --all-files`
- `.\tools\ci\run_local_gates.ps1`
- `.\tools\ci\run_presentation_gate.ps1`
- `.\tools\ci\run_release_candidate_gate.ps1`
- `git status --short`
- `git tag -l`

---

## Current Verification Status

- Verified: The source audit exists at `docs/release/AAA_RELEASE_READINESS_REPORT.md`.
- Verified: The plan maps every critical, high, and medium finding in the report to at least one task.
- Verified: Audit-gap items are marked unverified and require inspection/tests before implementation.
- Unverified: Any code fix in this plan. This document is an execution plan only; implementation tasks still need to be performed and verified.
- Unverified: Fresh-clone LFS hydration. It requires a clone that does not rely on the local Git LFS cache.
- Unverified: Legal sufficiency of EULA/privacy/third-party notices. It requires qualified legal review.
