# AAA Release-Readiness Report - URPG Engine

**Report date:** 2026-04-26
**Repository audited:** `C:\dev\URPG after mess up\URPG-RPG-Game-Designer`
**Purpose:** Authoritative release-readiness audit for the current application entry points, packaging metadata, editor surface exposure, and release governance files.
**Verdict:** **NOT RELEASE-READY**

This report replaces the prior broad audit with a stricter evidence standard. Findings below are limited to facts verified from the repository on 2026-04-26. Items that were not exhaustively audited are listed separately under "Audit Gaps" and must not be treated as proven defects until verified.

## Evidence Basis

The following checks were performed against the local tree:

```powershell
Get-Content .\CMakeLists.txt -TotalCount 90
Select-String .\CMakeLists.txt -Pattern 'project\(|install\(|FetchContent|Catch2|CPack|VERSION|urpg_presentation_release_validation'
Get-Content .\apps\runtime\main.cpp
Get-Content .\apps\editor\main.cpp -TotalCount 460
Get-Content .\engine\core\presentation\release_validation.cpp
Get-ChildItem .\docs
Get-ChildItem .\third_party
git tag -l
git describe --tags --always --dirty
rg -n -- '--help|--version|AudioCore|AssetLoader|RuntimeBundleLoader|LocaleCatalog|PerfProfiler|InputManager|SaveRuntime|TitleScreen|MenuScene|SDL_GL_SetSwapInterval|fullscreen|VSync' .\apps
```

Important repository condition: after branch cleanup and history rewrite, the audited branch is clean on `development` at commit `f9d74b2dca0423dafcce2cd9e009cab4ce8555e3`. The earlier index-only mass deletion state was resolved, then the tracked asset deletions were restored from `HEAD` and hydrated from the local Git LFS cache with `git lfs checkout`. The absence of tags and `git describe` output are still reported because they were directly observed.

## Verified Summary

| Severity | Count | Meaning |
|---|---:|---|
| Critical | 10 | Blocks a credible release or release candidate from this tree |
| High | 9 | Major release risk or user-facing incompleteness |
| Medium | 6 | Release hardening, packaging, or governance gap |

## Critical Findings

### C-1: No Project Version Metadata

**Evidence:** [CMakeLists.txt](../../CMakeLists.txt) line 2 contains:

```cmake
project(urpg LANGUAGES C CXX)
```

There is no `VERSION` argument in the CMake project declaration. Direct checks also found no root or docs-level `VERSION`, `version.txt`, or `CHANGELOG.md`.

**Impact:** Release builds cannot be tied to a stable semantic version from CMake metadata, and binaries cannot reliably expose a build identity.

**Required fix:** Add `project(urpg VERSION x.y.z LANGUAGES C CXX)`, generate a version header, and wire `--version` support into shipped executables.

### C-2: No Release Tags

**Evidence:** `git tag -l` returned no tags. On the clean `development` branch at commit `f9d74b2dca0423dafcce2cd9e009cab4ce8555e3`, `git describe --tags --always --dirty` returned:

```text
f9d74b2dc
```

**Impact:** There is no immutable release marker or version history anchor in the audited repository.

**Required fix:** Create an initial release or prerelease tag only after validation passes, and document release contents in `CHANGELOG.md`.

### C-3: Runtime Boots Directly Into a Hardcoded Map Scene

**Evidence:** [apps/runtime/main.cpp](../../apps/runtime/main.cpp) clears the scene stack and calls:

```cpp
urpg::scene::SceneManager::getInstance().gotoScene(
    std::make_shared<urpg::scene::MapScene>("RuntimeBoot", 16, 12));
```

The app search found no `TitleScreen` or `MenuScene` reference in `apps/runtime/main.cpp`.

**Impact:** The runtime does not expose a release-style boot flow such as title screen, new game, continue, options, or exit.

**Required fix:** Replace the direct boot map with a real startup flow and coverage proving the flow is reachable from `urpg_runtime`.

### C-4: Runtime Entry Point Has No Save/Load Flow

**Evidence:** [apps/runtime/main.cpp](../../apps/runtime/main.cpp) parses only `--headless`, `--frames`, `--width`, `--height`, and `--project-root`. The app search found no `SaveRuntime` reference in `apps/runtime/main.cpp`.

**Impact:** A released runtime built from this entry point cannot continue a game, select a save slot, recover corrupted saves, or autosave through the main boot path.

**Required fix:** Wire save metadata discovery, continue/new-game selection, load failure handling, and save write coverage into the runtime entry point or a boot scene reached from it.

### C-5: Runtime Entry Point Does Not Initialize Audio, Asset Bundle Validation, Localization, Profiler, or Input Manager

**Evidence:** Searching `apps/runtime/main.cpp` found no references to `AudioCore`, `AssetLoader`, `RuntimeBundleLoader`, `LocaleCatalog`, `PerfProfiler`, or `InputManager`.

**Impact:** Major engine subsystems may exist elsewhere, but they are not proven reachable from the shipped runtime entry point.

**Required fix:** Add explicit runtime startup integration or document why each subsystem is intentionally initialized through another verified path, then add smoke tests for that path.

### C-6: Editor Links Many Panel Sources but Registers Only Six Top-Level Panels

**Evidence:** [apps/editor/main.cpp](../../apps/editor/main.cpp) registers exactly these panels through `EditorShell::addPanel`:

| ID | Title | Category |
|---|---|---|
| `diagnostics` | Diagnostics | System |
| `assets` | Assets | Content |
| `ability` | Ability Inspector | Gameplay |
| `patterns` | Pattern Field Editor | Gameplay |
| `mod` | Mod Manager | Runtime |
| `analytics` | Analytics | Runtime |

The editor tree contains 67 `*panel.h` files and 64 `*panel.cpp` files. `CMakeLists.txt` includes many editor panel sources, including battle, save, message, UI, spatial, accessibility, audio, export, character, achievement, input, platform, performance, and other panels.

**Impact:** Many compiled editor surfaces are not exposed as top-level editor panels from `apps/editor/main.cpp`. Some may be indirectly reachable from `DiagnosticsWorkspace`, but top-level navigation coverage is incomplete.

**Required fix:** Define the intended navigation model, register all user-facing panels through a discoverable menu/workspace, and add smoke coverage for every intended top-level surface.

### C-7: Editor Smoke Test Covers Only Five Panels

**Evidence:** [apps/editor/main.cpp](../../apps/editor/main.cpp) declares the smoke-required panel IDs as:

```cpp
"diagnostics", "assets", "ability", "mod", "analytics"
```

It omits the registered `patterns` panel and does not cover the many other compiled panels.

**Impact:** Current smoke coverage can pass while large portions of the editor UI are unreachable, broken, empty, or unrendered.

**Required fix:** Expand smoke coverage to all registered top-level panels and add an explicit allowlist for intentionally hidden/internal panels.

### C-8: Release Validation Uses `assert()`

**Evidence:** [engine/core/presentation/release_validation.cpp](../../engine/core/presentation/release_validation.cpp) includes `<cassert>` and uses `assert(...)` for validation checks.

**Impact:** In Release/NDEBUG builds, these validations can be compiled out, turning the release validation executable into a weaker signal than its name implies.

**Required fix:** Replace `assert()` checks with explicit runtime checks that print diagnostics and return non-zero on failure.

### C-9: Install Rules Are Minimal

**Evidence:** [CMakeLists.txt](../../CMakeLists.txt) contains:

```cmake
install(TARGETS urpg_runtime urpg_editor urpg_audio_smoke
    RUNTIME DESTINATION bin
)
```

No verified install rule was found for data assets, config files, licenses, third-party notices, shaders, plugins, runtime bundles, or platform metadata.

**Impact:** A `cmake --install` output is not a complete distributable application.

**Required fix:** Define install components for binaries, runtime data, project templates, licenses/notices, plugin/runtime files, and platform-specific support files.

### C-10: Required Legal/Release Documents Are Missing

**Evidence:** Direct file checks found no root-level or docs-level:

- `THIRD_PARTY_NOTICES.md`
- `EULA.md`
- `PRIVACY_POLICY.md`
- `CREDITS.md`
- `CHANGELOG.md`

The root `LICENSE` exists, and `third_party/` contains `aseprite`, `external-repos`, `github_assets`, `huggingface`, `itch-assets`, and `rpgmaker-mz`.

**Impact:** The repository lacks the legal and release documentation needed for a credible external release review.

**Required fix:** Create and maintain third-party notices, credits, release notes/changelog, and any product legal documents required by the chosen distribution channel.

## High Findings

### H-1: Runtime and Editor Do Not Support `--help` or `--version`

**Evidence:** App-level search found `--help` only in `apps/audio_smoke/main.cpp`. No `--help` or `--version` handling was found in `apps/runtime/main.cpp` or `apps/editor/main.cpp`.

**Impact:** Shipped executables lack basic operational metadata and user-facing CLI discovery.

**Required fix:** Add `--help` and `--version` to all shipped CLI/app entry points.

### H-2: Runtime Main Loop Has No Verified Frame Cap in Non-Headless Mode

**Evidence:** [apps/runtime/main.cpp](../../apps/runtime/main.cpp) sleeps for 1 ms only when `options.headless` is true. No VSync or frame limiter reference was found in the runtime entry point.

**Impact:** Non-headless runtime execution can run as fast as `shell.tick()` and presentation allow unless throttled deeper in the engine. That deeper throttling was not verified by this report.

**Required fix:** Add explicit frame pacing or verify and document where frame pacing is enforced.

### H-3: Runtime Main Loop Has No Verified Pause/Focus Handling

**Evidence:** [apps/runtime/main.cpp](../../apps/runtime/main.cpp) loops on `shell.isRunning()` and calls `shell.tick()` each frame. No pause/focus/suspend handling is visible in the entry point.

**Impact:** Focus loss, minimize, suspend/resume, and pause menu behavior are not release-proven from the runtime entry point.

**Required fix:** Wire platform events into a pause/suspend state or verify that `EngineShell`/surface implementations handle this and add tests.

### H-4: Startup Failures Are Console-Only

**Evidence:** Runtime/editor startup failures print to `std::cerr` and return non-zero. No user-visible error surface, persistent log path, or crash/error report workflow was verified in the entry points.

**Impact:** A release build can fail at startup without an end-user-visible recovery path.

**Required fix:** Add startup error presentation, persistent logs, and documented diagnostics collection.

### H-5: Editor ImGui Settings Persistence Is Disabled

**Evidence:** [apps/editor/main.cpp](../../apps/editor/main.cpp) sets:

```cpp
ImGui::GetIO().IniFilename = nullptr;
ImGui::GetIO().LogFilename = nullptr;
```

**Impact:** Layout and ImGui settings are not persisted through the default ImGui mechanism.

**Required fix:** Either configure a project-local settings path or document and test the replacement persistence mechanism.

### H-6: Catch2 Is Fetched by Default Independent of Release Intent

**Evidence:** [CMakeLists.txt](../../CMakeLists.txt) defines:

```cmake
option(URPG_FETCH_CATCH2 "Fetch Catch2 via CMake" ON)
```

and later calls `FetchContent_MakeAvailable(Catch2)` when enabled. Test executables are defined in the same top-level file.

**Impact:** Release configuration is not cleanly separated from test dependency acquisition by an obvious `URPG_BUILD_TESTS` option.

**Required fix:** Add a test-build option and gate Catch2 acquisition and test executable definitions behind it.

### H-7: No CPack Configuration Was Verified

**Evidence:** Searching `CMakeLists.txt` found no `include(CPack)` or `CPACK_` configuration.

**Impact:** The repository does not define a verified native packaging path through CPack.

**Required fix:** Add explicit packaging configuration or document the supported packaging system if it is not CPack.

### H-8: Spatial Panels Include Empty `render()` Implementations

**Evidence:** `editor/spatial/procedural_map_panel.cpp` and `editor/spatial/region_rules_panel.cpp` contain empty `render()` implementations.

**Impact:** Some editor surfaces compile but have no visible UI through their lowercase `render()` path.

**Required fix:** Implement real UI or explicit disabled/empty states for these panels, and add smoke coverage.

### H-9: Release Validation Depends on Git LFS Availability

**Evidence:** The repository had 156,845 tracked deletions under asset/import paths. They were restored with `git restore`, after which `git ls-files --deleted` returned zero. During restore, Git LFS attempted to download at least one missing object and reported:

```text
This repository exceeded its LFS budget. The account responsible for the budget should increase it to restore access.
```

`git lfs checkout` then succeeded locally:

```text
Checking out LFS objects: 100% (156847/156847), 19 GB | 392 MB/s, done.
```

As a mitigation after this audit, 39 duplicate source ZIP archives under `more assets/` were removed from Git tracking and `more assets/**/*.zip` was added to `.gitignore`. The extracted/imported asset directories under `imports/raw/more_assets/` remain tracked. This removes about 5.68 GB of ZIP archive payload from future tracked revisions, but it does not rewrite existing Git LFS history.

A second duplicate-asset mitigation removed the byte-for-byte duplicate `third_party/itch-assets/packs-by-category/ui/fantasy-platformer-game-ui/` tree from Git tracking and added that path to `.gitignore`. The canonical copy remains tracked at `third_party/itch-assets/packs/fantasy-platformer-game-ui/`. This removes another 592 duplicate tracked files, about 444.67 MB of LFS payload from future tracked revisions.

Two additional narrow archive removals were completed after confirming retained canonical copies:

- `imports/root-drop/archives/rpgmaker/visustella/VisuMZ_Sample_Game_Project.zip`, because the sample project is already unpacked under `third_party/rpgmaker-mz/visumz-sample-project/`.
- `third_party/itch-assets/packs-by-category/bundles-mega/2017patronbundle/2017/children/children_frames.zip`, because the canonical tracked copy remains under `third_party/itch-assets/packs/2017patronbundle/2017/children/children_frames.zip`.

**Impact:** The local machine can restore the asset payload from its LFS cache, but a fresh clone or CI job may fail to hydrate assets while the remote LFS budget is exhausted.

**Required fix:** Confirm CI and release build machines can access required LFS objects. Either fix the remote LFS budget, mirror the required release assets to an accessible artifact store, or reduce the tracked LFS footprint.

## Medium Findings

### M-1: No Verified Window Icon or Platform App Metadata

**Evidence:** This audit did not find app-level platform metadata wiring in the runtime/editor entry points, and install rules do not include platform metadata files.

**Impact:** Platform polish and OS integration are incomplete or at least not release-proven.

**Required fix:** Add Windows/macOS/Linux application metadata appropriate to supported release targets.

### M-2: No Verified Privacy/Consent Flow for Analytics

**Evidence:** [apps/editor/main.cpp](../../apps/editor/main.cpp) constructs analytics dispatcher/uploader/privacy controller objects and binds them to `AnalyticsPanel`. This audit did not verify a first-run consent gate.

**Impact:** Analytics behavior is not release-cleared from the editor entry point.

**Required fix:** Document analytics data flow and add a consent/disable path with tests.

### M-3: No Verified Runtime Settings Persistence

**Evidence:** No runtime/editor entry point settings file load/save was found during this audit.

**Impact:** Window, audio, input, and accessibility preferences are not proven persistent.

**Required fix:** Add a settings store or verify an existing one is reached from app startup/shutdown.

### M-4: No Verified Runtime Asset Directory Validation

**Evidence:** `RuntimeOptions.project_root` defaults to `std::filesystem::current_path()`, but the runtime entry point does not visibly validate required project/runtime directories before startup.

**Impact:** Missing data can degrade into generic startup failure instead of targeted diagnostics.

**Required fix:** Validate required directories/manifests early and emit actionable errors.

### M-5: CLI Parsing Ignores Unknown Options

**Evidence:** Runtime/editor parsing loops handle known flags but do not reject unknown arguments.

**Impact:** User mistakes or CI misconfiguration can be silently ignored.

**Required fix:** Return an error for unknown flags and missing values, with `--help` output.

### M-6: Release Readiness Governance Is Split Across Documents

**Evidence:** Existing docs include `PROGRAM_COMPLETION_STATUS.md`, `RELEASE_READINESS_MATRIX.md`, and `KNOWN_BREAK_WAIVERS.md`, but no app-level completion readiness matrix was found.

**Impact:** Engine subsystem readiness and app/product release readiness can diverge.

**Required fix:** Add an app-level release matrix covering boot flow, save/load, settings, audio, input, packaging, legal docs, and editor navigation.

## Audit Gaps

The prior report contained many claims that may be true but were not fully verified in this pass. They should be audited before being promoted to findings:

- Button handlers and disabled states across every editor panel.
- Empty/loading/error states across every editor panel.
- Full asset reference integrity and unused asset inventory.
- Orphaned event listeners and repeated timers in the QuickJS compatibility runtime.
- Unhandled JavaScript promise rejection behavior.
- Native input handling inside lower-level surface or engine shell implementations.
- Crash handling beyond `std::exception` catches in app entry points.
- Storefront-specific certification requirements for any chosen store.
- Runtime behavior of save, audio, localization, analytics, event, and profiling subsystems outside `apps/*`.

## Minimum Release-Candidate Exit Criteria

A release candidate should not be cut from this repository until all critical findings are closed and verified. Minimum objective gates:

1. Clean git status and a reproducible commit SHA.
2. CMake project version, generated version header, `--version`, and changelog.
3. Runtime boot flow with title/new/continue/options/exit or a documented non-game runtime mode.
4. Runtime save/load path with failure handling.
5. Runtime startup validation for project data and packaged bundles.
6. Editor navigation coverage for every intended top-level panel.
7. Smoke tests for all registered editor panels.
8. Release validation executable that fails reliably in Release builds.
9. Complete install/package rules including legal notices and runtime data.
10. Third-party notices and release/legal documentation.

## Final Assessment

URPG contains a substantial engine and editor codebase, but the audited entry points and packaging metadata are not yet at release-candidate quality. The main blocker is not the absence of subsystem code; it is that essential product flows are either not wired through the shipped applications, not covered by smoke tests, or not represented in release/package metadata.

