# URPG Development Status (v3.1)

Snapshot Date: 2026-04-18 (Swarm 5 complete) / 2026-04-15

This repository is bootstrapped to the blueprint's canonical structure and now includes core + compat kernels with active CI gate wiring.

Cross-cutting debt closure, documentation-truth alignment, and intake-governance tracking: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

## Progress tracker

- Phase 0 Foundation: 100% complete.
- Phase 1 Native Core: 100% complete for v3.1 kickoff/continuation/integration contracts.
- Phase 2 Compat Layer: in progress (~98%; runtime module wiring + status burn-down complete, DataManager JSON loading and BattleManager pipeline now wired, audio ducking/position tracking implemented, Window_Selectable input + touch tap wired, bitmap lifecycle + sprite updates implemented, QuickJS stub GC/heap simulation hardened, manager JS APIs wired to live singletons, DataManager-backed actor drawing implemented, CombatCalc integrated into battle actions, $gameActors runtime state implemented, Window_Command drawing + color system implemented, sprite motion animation implemented, battle state effects + animation recording implemented, state system end-to-end wired with slip damage/auto-removal/remove-by-damage, TP ceiling no longer hardcoded, GameParty/GameTroop runtime classes created, integration test coverage expanded).
- Phase 2 Compat Layer: hardening exit (~96%; runtime module wiring + status burn-down are complete, with remaining work on executable conformance depth and diagnostics exit criteria).
- Native Feature Absorption Wave 1: in progress (~63%; UI/Menu and Message/Text runtime slices are substantially landed, with remaining closure in renderer handoff, editor/schema/migration productization, and integration depth).
- Native Capability Expansion Wave 2: planning baseline (~5%; integrated tracks are defined in `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`, implementation pending).

## Where we are now (2026-04-15)

- PR `#5` native-first direction is merged into `main`.
- Branch model is now intentionally minimal: `main` plus one work branch (`plan/native-feature-absorption-20260413`), aligned to the same tip commit.
- `main` is protected with PR-required flow and required checks (`gate1-pr`, `gitleaks`).
- Dependabot version-update branch churn is disabled.
- Active roadmap has been rewritten as one integrated execution plan (`docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`) including Wave 2 advanced capability tracks.
- Canonical completion checklist (done + remaining to 100%): `docs/PROGRAM_COMPLETION_STATUS.md`.
- Canonical debt/truthfulness/intake-governance tracker: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.
- Canonical Wave 1 subsystem closure checklist source: `docs/WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md`.
- Subsystem checklist sync is automated via `tools/docs/sync-wave1-spec-checklist.ps1` and verified by `tools/ci/check_wave1_spec_checklists.ps1`.

## Completed now

- Canonical layout roots created (`engine`, `editor`, `runtimes`, `content`, `tools`, `tests`, `docs`, `third_party`).
- Section 18 kernels implemented:
  - `SemVer`
  - `Fixed32`
  - `FrameBudget`
  - `World` deterministic iteration (EntityID order)
  - `CombatCalc` baseline
  - Bridge `Value` model
- Phase 0 foundations implemented:
   - Thread role + script access contracts (`Render/Logic/Audio/AssetStreaming`).
   - Renderer capability tier model (`Basic/Standard/Advanced`) with feature gating helpers.
   - Save metadata envelopes + journaled atomic save writes with backup retention.
   - Save corruption recovery tiers: Level 1 autosave restore, Level 2 metadata+variables restore, Level 3 safe-mode skeleton load.
   - Runtime save load integration with recovery fallback and force-safe-mode startup (`RuntimeSaveLoader`).
   - Canonical JSON serializer contract and deterministic migration runner (`rename` + `set`) with CLI.
   - Source-of-truth authority policy (`Compat` raw authoritative, `Native` AST authoritative, `Mixed` per-block authority tags) with guardrail validation.
   - Event integration hook-up for authority validation and structured rejection diagnostics (`event_authority` JSONL).
- CI suite expansion implemented:
   - Nightly gate now runs real integration + snapshot suites (`ctest -L nightly`).
   - Weekly gate now runs compat suite (`ctest -L weekly`).
   - Nightly renderer-tier matrix (`basic/standard/advanced`) and test log artifact upload in workflow.
- Editor diagnostics hook-up implemented:
   - `event_authority` diagnostics now include `block_id` for navigation.
   - Added editor diagnostics index to parse JSONL streams and resolve event/block navigation targets.
- Editor diagnostics panel/view model implemented:
   - Row projection + summary strings for `event_authority` diagnostics.
   - Event-id filtering and selected-row one-click navigation target resolution.
- Phase 1 Native Core kickoff implemented:
   - Event runtime kernels for deterministic priority ordering and cancellation/default behavior controls.
   - Debugger contract kernels for breakpoint storage and watch variable tables.
- Phase 1 Native Core continuation implemented:
   - Event execution timeline + reentrancy depth tracking contract kernel (`EventExecutionTimeline`).
   - Debugger call-stack frame + step-control contract kernels (`CallStack`, `StepController`).
- Phase 1 Native Core integration follow-up implemented:
   - Runtime dispatch-session wiring over event timeline/reentrancy contracts (`EventDispatchSession`).
   - Runtime debug-session wiring over breakpoints/watches/call-stack/step controls (`DebugRuntimeSession`).
- Test baseline added and passing (Release snapshot):
  - `urpg_tests`: 3911 assertions / 301 test cases
  - `urpg_integration_tests`: 33 assertions / 6 test cases
  - `urpg_snapshot_tests`: 4 assertions / 2 test cases
  - `urpg_compat_tests`: 1985 assertions / 24 test cases
  - **Total: 5933 assertions / 333 test cases**
- Test baseline added and passing (Debug snapshot, 2026-04-15):
  - `urpg_tests`: 3907 assertions / 287 test cases
  - Latest local gate snapshot:
    - `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure`: 289/289 passed
    - `ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure`: 42/42 passed
- Phase 2 compat expansion implemented in active targets:
   - WindowCompat surface expansion for `Window_Base`, `Window_Selectable`, and `Window_Command` API registration.
   - WindowCompat method call-count tracking surfaced in compat reports.
   - Curated plugin profile conformance suite (`test_compat_window_plugin_profiles.cpp`) for 10 real-world MZ plugin profiles.
   - Executable plugin fixture suite (`test_compat_plugin_fixtures.cpp`) for the same 10 plugin profiles in the weekly lane, now including deterministic cross-plugin invoke chain fuzz matrix coverage (32 generated chain cases across curated fixtures with mixed `invoke` + `invokeByName` branch routing).
   - Curated failure-path diagnostics suite (`test_compat_plugin_failure_diagnostics.cpp`) now gates missing-command/full-name parse diagnostics, malformed fixture/load artifacts, malformed fixture command payload shapes, fixture script runtime op failures, fixture script validation-shape failures (`set`/`append` key requirements plus malformed `invoke`/`invokeByName` target/store/expect shapes), malformed nested-branch fixture failures (`if` branch-shape and nested branch-step validation paths), directory-scan failures (including deterministic iterator/entry-status branches), and dependency-failure behavior across the same 10 plugin profiles.
   - Added a combined weekly regression that executes dependency-gating conformance across all 10 curated fixtures plus mixed malformed payload/eval/runtime/full-name-parse chain scenarios (including fixture-open/fixture-name/duplicate-load failures, `load_plugin_name` + `load_plugin_register_command` + `load_plugin_register_script_fn` + `load_plugin_quickjs_context` failures, parameter-parse failures, deterministic directory-scan iterator/entry-status failures (`load_plugins_directory_scan` / `load_plugins_directory_scan_entry`), malformed command metadata failures (`dropContextBeforeCall`/`entry`/`description` type validation), malformed fixture metadata shape failures (`dependencies`/`parameters`/`commands` type validation plus dependency-entry string enforcement), nested `all`/`any` runtime branch failures, `invoke`/`invokeByName` command-chain runtime failures, deterministic post-load context-drop coverage for `execute_command_quickjs_context_missing`, strict script-shape runtime failures, and malformed command-shape/name load failures) in one diagnostics pass, including compat report model/panel ingestion and export projection checks.
- Phase 2 compat module wiring completed in active targets:
   - `battle_manager`, `data_manager`, `audio_manager`, `input_manager`, `plugin_manager` are now part of active CMake builds.
   - Corresponding unit tests are active in `urpg_tests`: `test_battlemgr`, `test_data_manager`, `test_audio_manager`, `test_input_manager`, `test_plugin_manager`.
   - `PluginManager` fixture loading now supports JSON plugin fixtures, directory scanning, JSON parameter parsing, and script-driven fixture command execution.
   - PluginManager directory/plugin/command/dependent enumeration paths now enforce deterministic lexical ordering (`loadPluginsFromDirectory`, `getLoadedPlugins`, `getPluginCommands`, `getDependents`).
   - Fixture script commands now execute through per-plugin `QuickJSRuntime` contexts (`QuickJSContext::call`) rather than direct in-manager dispatch.
   - Fixture JSON commands now support lightweight JS source + entrypoint execution via `QuickJSContext::eval` + `call`.
   - Fixture commands now support deterministic `dropContextBeforeCall` hooks for `execute_command_quickjs_context_missing` conformance coverage.
   - Curated fixture profiles now validate JS directive `arg` and `const` modes across all 10 real-world plugin fixtures.
   - Fixture script DSL expanded with control flow + richer value sources (`if`, `args`, `paramKeys`, `hasParam`, `hasArg`, `equals`, `coalesce`, `length`, `contains`, `greaterThan`, `lessThan`, `not`, `all`, `any`) and executable chain coverage for `append`/`local`/`concat`.
   - Fixture script DSL now supports command-chain dispatch ops (`invoke`, `invokeByName`) with deterministic `store` capture and `expect: non_nil` assertions for nested execution conformance.
   - Compat diagnostics hardening now includes invoke-chain malformed/runtime failure-path coverage (`execute_command`, `execute_command_by_name_parse`, `execute_command_quickjs_call`) plus malformed fixture script validation branch coverage in weekly fixtures.
   - Unknown (non-`std::exception`) command/runtime throws are now emitted as `CRASH_PREVENTED` diagnostics for deterministic crash-containment classification.
   - Plugin reload now rehydrates from tracked source paths and restores fixture commands for JSON-backed plugin profiles.
   - PluginManager command/load failure diagnostics now propagate through `setErrorHandler` and structured JSONL artifact export (`exportFailureDiagnosticsJsonl` / `clearFailureDiagnostics`) for deterministic conformance capture.
   - PluginManager diagnostics JSONL now include compat severity tags (`WARN`, `SOFT_FAIL`, `HARD_FAIL`, `CRASH_PREVENTED`) for downstream report classification.
   - Failure diagnostics export now enforces bounded retention (last 2048 events) while preserving monotonic sequence IDs across trims (unit-gated).
   - `executeCommandByName` now resolves exact registered full keys (supporting underscore-heavy command names) and rejects missing plugin/command segments deterministically via `execute_command_by_name_parse`.
   - Compat report model now ingests PluginManager JSONL failure artifacts (`ingestPluginFailureDiagnosticsJsonl`) and projects them into event timeline + per-plugin unsupported/error summaries.
   - Compat report ingestion now maps PluginManager compat severity tags into timeline severity levels (`WARNING`/`ERROR`/`CRITICAL`).
   - QuickJS stub runtime now supports explicit eval-failure directives (`@urpg-fail-eval`), explicit runtime call-failure directives (`@urpg-fail-call`), deterministic context-init failure marker support (`__urpg_fail_context_init__`), and evalModule failure propagation tests for deterministic failure-path conformance.
   - Fixture command validation now rejects malformed command payload shapes deterministically (`js` string required, `script` array required, `dropContextBeforeCall` boolean required when present, optional `entry`/`description`/`mode` metadata string required when present, `mode` values restricted to `const` or `arg_count`, and no mixed `js`+`script` declarations) with structured diagnostics operations.
   - Fixture metadata shape validation now rejects malformed `dependencies`/`parameters`/`commands` containers and non-string dependency entries deterministically with structured diagnostics operations.
   - Fixture script runtime now supports explicit `error` op and unknown-op hard-fail behavior, exported via deterministic `execute_command_quickjs_call` diagnostics artifacts.
   - Dependent plugin command execution is now gated when required dependencies are missing, exported via deterministic `execute_command_dependency_missing` diagnostics artifacts.
   - Compat report panel refresh/update flow now consumes and clears PluginManager diagnostics JSONL artifacts in live editor update cycles.
   - Compat report diagnostics model hardening now updates warning/error flags when method statuses transition and sorts call-volume views by total calls (including unsupported operations).
   - Compat report panel now records bounded per-plugin session score history plus first-seen/last-updated timestamps, and `LAST_UPDATED` sorting now projects human-readable recency labels.
   - DataManager save/load compat lane now persists per-slot `GlobalState` + `SaveHeader` in-memory (including autosave slot `0`) with slot bounds validation.
   - DataManager header-extension compat APIs now round-trip per-slot plugin metadata and clear extension state on save-slot deletion.
   - DataManager transfer semantics now track reserved transfers and apply them via `processTransfer`.
   - `SaveSessionCoordinator` now loads `save_slots.json` descriptors and projects slot labels into save-inspector rows when map names are absent.
   - Compat routed battle-flow evidence now includes a tactical reload-survival fixture scenario.
   - Burned down selected compat statuses: `Window_Base.drawItemName`, `Window_Base.textWidth`, `Window_Base.textSize` from `STUB` to `PARTIAL`; `TouchInput.worldX/worldY` from `STUB` to `FULL`; `Window_Base.drawActorHp/drawActorMp/drawActorTp` from `PARTIAL` to `FULL`.
   - Burned down WindowCompat text-surface statuses: `drawTextEx`, `textWidth`, `textSize` from `PARTIAL` to `FULL` using escape-token parity and compat renderer-backed measurement/layout flow.
   - `Window_Base.drawText` now submits backend-facing `RenderLayer::TextCommand` payloads using resolved alignment offsets and active font/color state.
   - Added compat `Window_Message` surface (`drawMessageBody`) with deterministic dialogue-body alignment behavior (`left`/`center`/`right`).
   - Added snapshot-style wrapped centered/right `drawTextEx` history coverage to lock deterministic placement semantics.
   - Burned down additional WindowCompat statuses: `drawItemName` from `PARTIAL` to `FULL` (DataManager-backed icon+label semantics) and `Sprite_Actor.startEffect` from `PARTIAL` to `FULL` (deterministic effect-duration lifecycle).
   - Burned down additional Sprite_Actor animation status: `startAnimation` from `PARTIAL` to `FULL` with deterministic frame-duration playback lifecycle.
   - Burned down remaining Window_Base face status: `drawActorFace` from `PARTIAL` to `FULL` with MZ-canonical face cell clipping/centering semantics and deterministic face draw metadata.
   - Earlier compat burn-down work materially improved several surfaces, but a follow-up audit is still required because some runtime registries continue to overclaim `FULL` on stub-, fixture-, or placeholder-backed paths.
   - Burned down AudioManager compat statuses: `crossfadeBgm`/`crossfadeBgs` from `PARTIAL` to `FULL` via deterministic frame-based crossfade sequencing; BGM save/restore metadata now retains true track filename/position.
   - Burned down BattleManager compat status: `processEscape` from `PARTIAL` to `FULL` with deterministic MZ-style escape ratio + fail-ramp handling.
   - `executeCommandAsync` now has deterministic FIFO task queue execution and callback ordering, but the surface still remains `PARTIAL` because callbacks run on the worker thread and the JS bridge is fixture-backed.
   - Input/Touch QuickJS API registration now returns live runtime state and `TouchInput` movement telemetry now tracks `moveSpeed` and `tapCount`.
- **Technical debt burn-down (Agent Swarm 2026-04-18 — Swarm 1):**
  - `DataManager` JSON loading implemented: all database files (Actors, Classes, Skills, Items, Weapons, Armors, Enemies, Troops, States, Animations, Tilesets, CommonEvents, System, MapInfos, MapXXX) now parse from MZ-format JSON via `nlohmann/json`. Includes `setDataPath()` API and file-existence guards.
  - `DataManager` `getXxxAsValue()` methods now return actual parsed data as `Value` arrays/objects.
  - `BattleManager` pipeline wired to `DataManager`: `setup()` loads troop data and populates enemies from `EnemyData`; copies party actors from `GlobalState`; `calculateExp()`/`calculateGold()`/`calculateDrops()` use real enemy stats; `applyGold()`/`applyDrops()` route through `DataManager`; `checkSwitchCondition()` reads live switches.
  - `BattleManager` action processing: `SKILL` and `ITEM` actions now resolve targets and call `applySkill()`/`applyItem()` with `DataManager` lookups.
  - `AudioManager` playback position tracking: `AudioChannel::update()` increments `pos_` deterministically per frame.
  - `AudioManager` smooth ducking/unducking: frame-based volume interpolation over configurable duration; instant fallback for `duration <= 0`.
  - Test hygiene: deleted orphan `tests/unit/test_compat_reportPanel.cpp` (only underscore-named file is referenced in CMake).
  - Integration tests added: `tests/integration/test_integration_battle_data.cpp` validates DataManager + BattleManager end-to-end interaction (troop setup, victory rewards, switch conditions, skill lookup).
  - Test data assets: `tests/data/mz_data/` sample JSON database files created for deterministic test loading.
- **Technical debt burn-down (Agent Swarm 2026-04-18 — Swarm 2):**
  - `Window_Selectable` input wiring: `processCursorMove()` now reads `InputManager::getDir4()`/`getDir8()` and drives `cursorDown/Up/Left/Right()`; `processHandling()` reads `InputManager::isTriggered(InputKey::DECISION/CANCEL)` and dispatches `onOk()`/`onCancel()`. `Window_Command::onOk()` delegates to `callOkHandler()`.
  - WindowCompat bitmap lifecycle: added deterministic `BitmapHandle` allocator (`allocateBitmap`/`releaseBitmap`/`isValidBitmap`); `Window_Base::createContents()`/`destroyContents()` now manage real handles; destructors release bitmaps for `Window_Base`, `Sprite_Character`, `Sprite_Actor`.
  - WindowCompat drawing stubs: replaced raw TODOs with compat-layer recording stubs (`drawText`, `drawIcon`, `drawGauge`, `drawCharacter`) that document native-renderer tier delegation.
  - Sprite animation: `Sprite_Character::update()` advances `pattern_` deterministically (0→1→2→3 cycle); `Sprite_Actor::update()` decrements animation/effect frame counters to completion.
  - `QuickJSRuntime` stub hardening: added `URPG_HAS_QUICKJS` compile-time gate; `runGC()` on stub path collects unreachable globals and recalculates simulated heap; `eval()`/`call()`/`registerFunction()`/`setGlobal()` increment simulated heap size deterministically; `isMemoryExceeded()` works against the simulated heap.
  - Added 11 new test cases across window, sprite, and QuickJS test suites.
- Migration and schema anchors added:
  - `tools/migrate/migration_op.json`
  - `tools/migrate/fuzz_migrate.cpp`
  - `content/schemas/project.schema.json`

## Immediate next implementation lanes

1. Complete Phase 2 validation depth
   - Expand executable conformance/failure-path scenarios across curated 10-plugin fixtures (additional malformed/eval/runtime command chains).
2. Deepen compat validation
   - Expand fixture script DSL + report artifact coverage for real JS plugin execution paths and compat report exports.
3. Remaining technical debt (next swarm)
   - `WindowCompat` full software rasterization: actual text layout, icon atlas lookup, gauge gradient fill, character sheet slicing (currently recording stubs).
   - `QuickJSRuntime` real QuickJS wiring: fetch QuickJS via CMake, link against C API, implement `URPG_HAS_QUICKJS` path.
   - `$gameParty` / `$gameTroop` QuickJS global exposure: wire `GameParty`/`GameTroop` into `DataManager::registerAPI` as `$gameParty`/`$gameTroop`.
4. Compat exit completion
   - Finish remaining routed conformance/failure depth and lock every new failure mode to JSONL/report/panel assertions until explicit exit criteria are satisfied.
5. Message/Text renderer closure
   - Complete backend text-command consumption and native MessageScene handoff from compat `Window_Message` bridge behavior.
6. Wave 1 native runtime ownership
   - Complete native subsystem ownership for UI/Menu, Message/Text, Battle, and Save/Data beyond currently landed slices.
7. Wave 1 editor + schema productization
   - Ship subsystem inspectors/previews/diagnostics and finalize migration-safe schemas/import upgrade paths.
8. Wave 2 advanced capability delivery
   - Start implementation tracks for ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, and editor utilities.
9. Completion tracking
   - Execute and maintain `docs/PROGRAM_COMPLETION_STATUS.md` as the source of truth for remaining work to 100% completion in this program scope.

## Build/test

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

Focused presentation validation is also available through:

```powershell
pwsh -File .\tools\ci\run_presentation_gate.ps1
```

The broader local gate runner now invokes that focused presentation gate before the repo-wide labeled suites:

```powershell
pwsh -File .\tools\ci\run_local_gates.ps1
```
