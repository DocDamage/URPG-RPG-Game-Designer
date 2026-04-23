# URPG Master Blueprint v3.1

Upgrade Date: 2026-03-03  
v3.1 Focus: architecture-core determinism + lifecycle/hot-reload + save metadata + test anchors

_Full Engineering Specification - No Fluff_

## Progress Tracker (Live)

Status Date: 2026-04-17

Cross-cutting debt closure, documentation-truth alignment, and intake-governance tracking: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

| Track | Status | Completion | Notes |
| --- | --- | --- | --- |
| Phase 0 Foundation | Complete | 100% | Core kernels, authority guards, migration/save lanes, diagnostics indexing/panel wiring, and CI lane scaffolding are in place. |
| Phase 1 Native Core | Complete | 100% | Event dispatch, debug runtime, EngineShell lifecycle, and deterministic ECS iteration are fully implemented. |
| Phase 2 Compat Layer | In Progress | ~85% | Large compat surface is wired and test-heavy, but significant areas remain fixture-, stub-, or placeholder-backed; status labels and docs now reflect this truth. |
| Native Feature Absorption Wave 1 | In progress | ~99% | UI/Menu, Message/Text, Save/Data, and Battle closure evidence are recorded. Remaining work is the broader Wave 1/program release close-out. |
| Native Capability Expansion Wave 2 | Active | ~60% | Developed core kernels for Gameplay Ability Framework, Pattern Fields, Modular Level Assembly, Sprite Pipeline, Raycast Renderer, and Animation/Timeline orchestration. |
| Native Capability Expansion Wave 3 | Complete | 100% | Technical delivery of All Template Expansion systems (ARPG, VN, Tactics), Subsystem Debugging (Profiler/History), and Phase 4 Ecosystem (Cloud, Security, Plugins) completed and cross-validated. |
| Native Feature Polish Wave 4 | Complete | 100% | Full technical delivery of Engine Polish tracks (4.1-4.6). Includes Scene Transition Manager, Priority Asset Loader, Spatial Audio Mixer, Hot-Reload, Export Packager, and Resource Protector. |
| Editor Workspace Wave 5 | Complete | 100% | Full technical delivery of Editor Workspace tracks (5.1-5.4). Includes Workspace Kernel, Undo/Redo tracking, Viewport Overlays, and Property Inspector Auto-Generation. |
| ImGui UI Panels Wave 6 | Complete | 100% | Full implementation of Editor Shell, Scene Hierarchy Panel, Asset Browser, Property Inspector, and Theme Styling. |
| Plugin & Scripting Wave 7 | Complete | 100% | Full technical delivery of Extensibility tracks (7.1-7.4). Includes Plugin Host, C++ API Exports, JavaScript Bridge, and Scripting Console UI. |
| Phase 15 Final Integration | Complete | 100% | Assembly of all Wave tracks (1-7) into the unified **URPG v3.1 Gold distribution**. Historical planning language referenced global `EngineAssembly` / `MainAssembly` entry points; the active runtime entry point in tree is `EngineShell`. |
| CI Gate Lanes | Active | 100% | PR/nightly/weekly labels are active, with nightly renderer-tier matrix + artifact uploads and snapshot validation. |
| Validation Baseline | Passing | 5098 assertions / 400 cases | `urpg_tests` (Debug, `ctest -C Debug -L pr`, 2026-04-17). |

### Current Weekly Focus

1. Close compat exit criteria: expand remaining routed conformance depth across the curated 10-profile corpus and lock each new failure mode to JSONL/report/panel assertions.
2. Close Message/Text renderer handoff: consume backend text commands end-to-end and align compat `Window_Message` parity behavior with native message-scene ownership.
3. Execute the remaining Wave 1 closure proof: publish the broader Wave 1/program release evidence and final reconciliation.
4. Ship Wave 1 editor + schema productization: inspectors, previews, diagnostics, and migration-aware import upgrade paths.
5. Begin Wave 2 advanced capability implementation tracks from `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`.
6. Track completion against `docs/PROGRAM_COMPLETION_STATUS.md` until all checklist items are complete.
7. Keep Wave 1 subsystem closure checklists synchronized from `docs/WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md` via `tools/docs/sync-wave1-spec-checklist.ps1`.
8. Use `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` as the canonical tracker for debt closure, truthfulness corrections, and external/private intake governance.

## v3.1 Upgrade Notes


- Added Security + Permissions contract for native + plugin lanes (least privilege by default).

- Added Observability / Crash Recovery / Safe Mode contracts for editor and exported builds.

- Expanded File Format rules: canonical JSON, schema validation, corruption detection, and deterministic migrations.

- Expanded Compat Layer: budgets, polyfill strategy, and an automated plugin surface conformance suite.

- Expanded Testing Infrastructure into a concrete CI gating plan plus reference implementations + sample tests.

- Added Architecture Core contracts: ECS World, deterministic iteration rules, and system registration boundaries.

- Added Fixed32 (Q16.16) fixed-point math contract to make Deterministic Mode meaningful across CPUs/compilers.

- Added FrameBudget allocator + enforcement contract for native vs plugin lanes (budget registry + per-plugin slices).

- Added Plugin lifecycle hooks, conflict graph ownership model, and hot-reload safety boundaries.

- Added Event priority, cancellation, bubbling, and coroutine-based async timeline execution contract.

- Added Save slot metadata schema + tiered corruption recovery flow (including Safe Mode skeleton load).

- Added Localization fallback chain + hot-reload contract, plus Copilot locale output requirements.

- Added Producer Copilot guardrails: canon constraint validator, atomic undo/rollback, and reproducible difficulty knobs.

- Expanded test anchors: combat formula baselines, bridge marshalling round-trip tests, and migration fuzzing.

- Expanded Security UX: permission prompt contract + audit log schema for permission use.

## Implementation Status (Live)

Status Date: 2026-04-15

This section tracks what has been implemented in code so this blueprint doubles as an execution ledger.

### Completed Foundation Work

- Canonical repository layout + CMake test/build wiring.
- Deterministic core kernels and tests:
  - SemVer, Fixed32 (Q16.16), FrameBudget, ECS deterministic iteration, CombatCalc baseline, bridge `Value` model.
- Threading contract:
  - Render/Logic/Audio/AssetStreaming role mapping + script access rules.
- Renderer capability tiers:
  - Basic/Standard/Advanced feature gate primitives.
- File format + migration lane:
  - Canonical JSON serializer, deterministic migration runner (`rename`, `set`), and `urpg_migrate` CLI.
- Save lane:
  - Save metadata envelopes, journaled writes, and recovery tiers (L1 autosave, L2 metadata+variables, L3 safe skeleton).
  - Runtime save load integration via `RuntimeSaveLoader`, including recovery fallback and force-safe-mode startup flow.
- Source-of-truth lane:
  - Compat/native/mixed authority policy with guardrails.
  - Event edit integration guard with structured `event_authority` rejection diagnostics.
  - `event_authority` diagnostics now emit `block_id` for navigation.
  - Editor diagnostics index parses JSONL diagnostics and resolves event/block navigation targets.
  - Editor diagnostics panel/view model now supports row rendering projections, event-id filtering, and one-click event/block navigation target selection.
- Phase 1 Native Core kickoff:
  - Event runtime kernel now enforces deterministic execution ordering by priority then registration order.
  - Event runtime kernel includes cancellation and prevent-default contract behavior.
  - Debugger kernels now include breakpoint storage contracts and watch table contracts.
- Phase 1 Native Core continuation:
  - Event runtime now includes execution timeline + reentrancy depth tracking contracts via `EventExecutionTimeline`.
  - Debugger kernels now include call-stack frame contracts (`CallStack`) and stepping controls (`StepController`).
- Phase 1 Native Core integration follow-up:
  - Event runtime now includes dispatch-session wiring for timeline/reentrancy gating via `EventDispatchSession`.
  - Debugger runtime now includes session wiring for breakpoints, watches, stack frames, and stepping via `DebugRuntimeSession`.
- CI gating suites:
  - PR/nightly/weekly labels with real gate coverage.
  - Nightly now runs integration + snapshot suites (`ctest -L nightly`) with renderer-tier matrix (`basic/standard/advanced`).
  - Weekly now runs compat suite (`ctest -L weekly`).
  - Nightly/weekly workflows upload gate log artifacts.
  - Known-break waiver schema + validator (`tools/ci/check_waivers.ps1`).
- Phase 4 Optimization & Asset Management:
  - `AssetCache<T>`: LRU-based template container with thread-safety and capacity-based eviction.
  - `TextureRegistry`: Metadata storage for textures/sprites, bridged to MapScene rendering.
  - `SaveSerializationHub`: Binary JSON packer (`URSV` format) with XOR checksums and versioning.
- Phase 5 Native Engine Shell:
  - `EngineShell`: Central loop coordinator for Tick -> Update -> Render lifecycle.
  - Scene Authority: Refactored `SceneManager` to a Singleton; added `handleInput` to `GameScene`.
  - Fixed `EngineShell::startup()` signature to require `IPlatformSurface` and `RendererBackend`.
  - Input/Logic Sync: Verified that `EngineShell::tick()` correctly routes input events to active Map/Menu scenes.
  - Render Layer: Headless command-based batching for sprite and tile submissions.
  - Updated `HeadlessSurface` for CI testing environments.
  - Resolved C++ standard 20 namespace and forward declaration issues in UI headers.
  - Fixed OpenGL linking issues with fallback path for missing `glActiveTexture`.
- Technical debt remediation (2026-04-17):
  - Compat status truth pass: downgraded inflated `FULL` claims to `PARTIAL`/`STUB` in `audio_manager.cpp`, `battle_manager.cpp`, `data_manager.cpp`, `window_compat.cpp`, and `input_manager.cpp`.
  - QuickJS scope explicitly documented as fixture-backed compat harness, not a live production runtime.
  - Test/build registration drift resolved: removed duplicate `test_engine_shell.cpp`, registered missing `test_menu_orchestration.cpp`, added SE channel-growth regression test.
  - External repository intake governance established (`docs/external-intake/`).
  - Private-use asset intake governance established (`docs/asset_intake/`).
  - Migration wizard productization: added `rerunSubsystem(id, project_data)` action, `bindMigrationWizardRuntime()` wiring, and panel snapshot exposure.
  - Data manager runtime closure: real `loadDatabase()` orchestration with seeded records; wired JS bindings; implemented real `get*AsValue()` serializers.

### Phase 14 Native Battle & UI (Complete)

- `BattleScene` Native Logic:
  - Turn-based state machine (START -> INPUT -> ACTION -> VICTORY/DEFEAT).
  - Automated turn progression and victory condition checks in C++.
- Native UI Components:
  - `UIWindow`: Base native C++ UI component with position and visibility state.
  - `UICommandList`: Command selection UI for battle menus (Attack, Skill, Item, Guard).
- Battle Handoff & Sync:
  - `BattleManager::instance()` singleton for JS-to-Native synchronization and state tracking.
- Battle Validation:
  - Automated battle tests in `tests/unit/test_battle_scene_native.cpp`.
  - Verified compilation and execution on Windows/MSVC.

### Phase 2 Compat Layer (Complete)

- QuickJS runtime integration contract kernel:
  - `QuickJSContext` with eval, module loading, function registration, and object binding.
  - `QuickJSRuntime` managing multiple isolated contexts per-plugin.
  - Virtual filesystem mounts for sandboxed file access.
  - Memory/CPU budget tracking with `CompatBudget` struct.
  - API status registry tracking FULL/PARTIAL/STUB/UNSUPPORTED surface coverage.
  - Stub conformance lane now supports explicit eval-failure directives (`@urpg-fail-eval`) and evalModule failure propagation coverage for deterministic failure-path tests.
- WindowCompat core surface stubs:
  - `Window_Base` with `drawText`, `drawIcon`, `drawActorFace`, `drawActorName`, `drawActorLevel`, `drawActorHp/Mp/Tp` gauges.
  - `Window_Selectable` and `Window_Command` stubs for menu plugin compatibility.
  - `Sprite_Character` and `Sprite_Actor` stubs for battle visual plugins.
  - All surfaces tagged with `CompatStatus` (FULL/PARTIAL/STUB/UNSUPPORTED).
  - Expanded API registry exposure for extended `Window_Base` + `Window_Selectable` + `Window_Command` methods used by menu plugins.
  - Added WindowCompat method call-count tracking in compat reports for runtime surface usage visibility.
  - Added command-window compatibility helpers (`isCommandEnabled`, `isCurrentItemEnabled`, `findSymbol`, `findExt`, `callOkHandler`).
  - Window text parity burn-down: `drawTextEx`, `textWidth`, and `textSize` now run through escape-token-aware compat text layout/measurement and are marked `FULL`.
  - Window text renderer bridge: `Window_Base.drawText` now emits backend-facing `RenderLayer::TextCommand` payloads using resolved alignment offsets, font metadata, and RGBA color state.
  - Added compat dialogue surface: `Window_Message` now supports deterministic message-body alignment (`left`/`center`/`right`) through native rich-text layout behavior.
  - Wrapped placement regression lock: snapshot-style centered/right `drawTextEx` history assertions now gate deterministic multiline alignment output.
  - Additional WindowCompat burn-down: `drawItemName` now resolves DataManager-backed icon/name semantics (`FULL`), and `Sprite_Actor.startEffect` now runs deterministic effect-duration lifecycle handling (`FULL`).
  - Additional Sprite_Actor burn-down: `startAnimation` now runs deterministic frame-duration playback lifecycle handling (`FULL`).
  - Additional Window_Base face burn-down: `drawActorFace` now runs MZ-canonical face cell clipping/centering semantics with deterministic draw metadata (`FULL`).
  - Compat status burn-down checkpoint: runtime compat API registry now reports no remaining `PARTIAL`/`STUB` surfaces in active modules.
- BattleManager MZ battle pipeline hooks:
  - Battle phase tracking (NONE, INIT, START, INPUT, TURN, ACTION, END, ABORT).
  - Hook points at each battle phase for plugin interception.
  - Turn management, action queue, damage/healing methods.
  - Battle result handling (WIN, ESCAPE, DEFEAT, ABORT).
  - Escape processing now follows deterministic MZ-style ratio + failure-ramp semantics (`processEscape` now `FULL`).
  - All methods tagged with `CompatStatus` registry.
- DataManager MZ data/save/load semantics:
  - Database loading (actors, skills, items, weapons, armors, enemies, troops, states, animations).
  - Global state management ($gameXxx variables: party, gold, steps, playtime).
  - Switch/variable/self-switch access with MZ-compatible indexing.
  - Item inventory management with gain/lose operations.
  - Save/load operations with header extensions.
  - Added `ActorData::level` and `ItemData::occasion` to `data_manager.h` for MZ compatibility.
  - Added `BattleManager::instance()` singleton for JS-to-Native synchronization.
  - Save-slot runtime now persists `GlobalState` + `SaveHeader` in-memory with slot-bounds enforcement and autosave slot (`0`) semantics.
  - Save header extension key/value metadata now round-trips per slot (`setSaveHeaderExtension`/`getSaveHeaderExtension`) and is cleared on slot deletion.
  - All methods tagged with `CompatStatus` registry.
- AudioManager MZ audio middleware compatibility:
  - Audio buses (BGM, BGS, ME, SE) with channel management.
  - Volume control per-bus and master volume.
  - Ducking support for BGM during ME playback.
  - Crossfade transitions for BGM.
  - BGM/BGS crossfades now run as deterministic frame-based fade-out/switch/fade-in sequences (`FULL` status).
  - BGM save/restore metadata now preserves real track filenames/positions (not channel ids) for correct round-trip behavior.
  - Save/restore BGM settings.
  - All methods tagged with `CompatStatus` registry.
- InputManager/TouchInput MZ input compatibility:
  - Keyboard state queries (isPressed, isTriggered, isRepeated, isReleased).
  - Direction input (dir4, dir8) with MZ-compatible values.
  - Mouse state tracking (position, buttons, wheel).
  - Touch state tracking (position, pressed, count).
  - `worldX/worldY` now apply camera-transform-aware mapping in compat lane (`FULL`).
  - QuickJS API registration now routes through live Input/Touch runtime state; touch movement telemetry now tracks `moveSpeed` and `tapCount`.
  - Gamepad support (buttons, axes, connection state).
  - Action mapping system for customizable controls.
  - All methods tagged with `CompatStatus` registry.
- PluginManager MZ plugin command registry:
  - Plugin lifecycle management (load, unload, reload).
  - Command registration and execution.
  - Async command execution now routes through deterministic FIFO task-queue processing (`executeCommandAsync` now `FULL`).
  - Parameter management per-plugin.
  - JSON fixture plugin loading, script-driven fixture command execution, and directory discovery for executable conformance runs.
  - PluginManager directory/plugin/command/dependent enumeration paths now enforce deterministic lexical ordering (`loadPluginsFromDirectory`, `getLoadedPlugins`, `getPluginCommands`, `getDependents`).
  - Fixture script commands are now executed through per-plugin `QuickJSRuntime` contexts (`QuickJSContext::call` bridge path).
  - Fixture JSON commands now support lightweight JS source + explicit entrypoint dispatch through `QuickJSContext::eval` + `call`.
  - Diagnostics system now includes a lightweight `DiagnosticsFacade` for on-demand JSON snapshot emission, enabling integration with external editor shells and UI transports.
  - Curated fixtures now exercise JS directive `arg` and `const` modes across all 10 plugin profiles.
  - Fixture script DSL expanded with conditional flow + richer resolvers (`if`, `args`, `paramKeys`, `hasParam`, `hasArg`, `equals`, `coalesce`, `length`, `contains`, `greaterThan`, `lessThan`, `not`, `all`, `any`) and executable `append`/`local`/`concat` chain coverage.
  - Fixture script DSL now supports nested command-chain dispatch via `invoke` and `invokeByName`, including deterministic `store` capture and `expect: non_nil` assertions.
    - Fixture script invoke expectations now cover `nil`, `truthy`, `falsey`, and `equals`, with explicit resolver metadata validation for `arg`/`hasArg`/`param`/`hasParam`/`local`/`argCount`/`args`/`paramKeys` enforced through runtime failures and unit coverage.
  - Executable compat fixtures now include deterministic cross-plugin invoke fuzz conformance (32 generated chain cases across curated profiles with mixed `invoke` + `invokeByName` branch routing).
  - Reload flow now tracks plugin source paths so JSON-backed fixture plugins rehydrate commands on `reloadPlugin`.
    - Curated compat fixture scenarios now cover richer menu-stack, codex/content, library-dashboard, save-data, menu-presentation, and presentation-family routing behavior, menu-stack/codex/content/library-dashboard/save-data/menu-presentation/presentation survival across plugin reload, and dependent command recovery after unloading and reloading shared core fixtures.
    - Native-first planning handoff is now seeded with an ownership-matrix input table derived from the strongest routed compat anchors, expanded with first-pass Message/Text and Save/Data rows, so UI/Menu, Message/Text, and Save/Data ownership work can start from verified lifecycle-backed or API-surface-backed evidence.
    - Curated message-text lifecycle coverage now proves routed speaker, narration, and system dialogue modes survive plugin reload while still exercising `Window_Base` escape-text, sizing, and face-rendering contracts.
    - Curated battle-flow lifecycle coverage now proves battle HUD and motion plugin routes can stay aligned with deterministic `BattleManager` turn, action, damage/heal, and escape behavior across plugin reload.
    - Curated save-data failure coverage now proves routed save-surface failures project through raw JSONL diagnostics, report-model ingestion/export, and panel refresh while slot metadata and autosave state remain recoverable.
    - Save/Data migration-path coverage now proves imported save metadata can be normalized into runtime-facing shape and then hydrated by the runtime save loader.
    - The first native-first subsystem spec drafts now exist for UI/Menu Core (`docs/UI_MENU_CORE_NATIVE_SPEC.md`), Message/Text Core (`docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`), Save/Data Core (`docs/SAVE_DATA_CORE_NATIVE_SPEC.md`), and Battle Core (`docs/BATTLE_CORE_NATIVE_SPEC.md`) and are ready to drive the next subsystem-spec pass.
  - Command failure diagnostics now route missing-command/full-name parse failures through `PluginManager::setErrorHandler` for deterministic capture.
  - Plugin failure-path diagnostics are now exportable as structured JSONL artifacts (`exportFailureDiagnosticsJsonl` / `clearFailureDiagnostics`) with deterministic sequence IDs and operation tags.
  - Plugin failure diagnostics JSONL now include explicit compat severity tags (`WARN`, `SOFT_FAIL`, `HARD_FAIL`, `CRASH_PREVENTED`) for downstream report classification.
  - Failure diagnostics export now enforces bounded retention (last 2048 events) while preserving monotonic sequence IDs across trims.
  - `executeCommandByName` now routes through exact registered full keys (supporting underscore-heavy command names) and rejects missing plugin/command segments deterministically through `execute_command_by_name_parse`.
  - QuickJS fixture lane now supports explicit runtime call-failure directives (`@urpg-fail-call`) for deterministic `execute_command_quickjs_call` diagnostics coverage.
  - Fixture command validation now rejects malformed payload shapes deterministically (`js` string required, `script` array required, `dropContextBeforeCall` boolean required when present, optional `entry`/`description`/`mode` metadata string required when present, `mode` values restricted to `const` or `arg_count`, and no mixed `js`+`script` declarations) with structured diagnostics operation tags.
  - Fixture metadata shape validation now rejects malformed `dependencies`/`parameters`/`commands` containers and non-string dependency entries deterministically with structured diagnostics operation tags.
  - Fixture script runtime now supports explicit `error` op and unknown-op hard-fail behavior, surfaced through deterministic `execute_command_quickjs_call` diagnostics artifacts.
  - Fixture script validation conformance now covers malformed `set`/`append` key requirements, malformed `invoke`/`invokeByName` target/store/expect shapes, and default `error`-op message behavior.
  - Fixture script malformed nested-branch conformance now covers invalid nested `if` branch shape paths and nested branch-step validation failures.
  - Unknown (non-`std::exception`) command/runtime throws are now classified as `CRASH_PREVENTED` diagnostics in compat lanes.
  - Dependent plugin command execution is now gated when required dependencies are missing, surfaced through deterministic `execute_command_dependency_missing` diagnostics artifacts.
  - Weekly combined conformance regression now executes dependency-gating checks across all 10 curated fixtures plus mixed malformed payload/eval/runtime/full-name-parse failure chains (including fixture-open/fixture-name/duplicate-load failures, `load_plugin_name` + `load_plugin_register_command` + `load_plugin_register_script_fn` + `load_plugin_quickjs_context` failures, parameter-parse failures, deterministic directory-scan iterator/entry-status failures (`load_plugins_directory_scan` / `load_plugins_directory_scan_entry`), malformed command metadata failures (`dropContextBeforeCall`/`entry`/`description` type validation), malformed fixture metadata shape failures (`dependencies`/`parameters`/`commands` type validation plus dependency-entry string enforcement), nested `all`/`any` runtime branch failures, `invoke`/`invokeByName` command-chain runtime failures, deterministic post-load context-drop coverage for `execute_command_quickjs_context_missing`, strict script-shape runtime failures, and malformed command-shape/name load failures) in one diagnostics pass, including compat report model/panel ingestion + export projection checks.
  - Compat report model now ingests PluginManager JSONL failure artifacts for event timeline/error summary wiring (`ingestPluginFailureDiagnosticsJsonl`).
  - Compat report ingestion now maps PluginManager compat severity tags into timeline severity (`WARNING`/`ERROR`/`CRITICAL`) for more accurate diagnostics projection.
    - Compat report model unit coverage now directly verifies severity mapping for `WARN`, `SOFT_FAIL`, `HARD_FAIL`, and `CRASH_PREVENTED` diagnostics tags, and compat failure suites now assert routed lifecycle unload failures through JSONL, report-model ingestion, export, and panel refresh, including in the combined weekly regression.
  - Compat report panel runtime refresh now consumes and clears PluginManager diagnostics artifacts each update cycle (`CompatReportPanel::refresh`/`update`).
  - Compat report diagnostics model hardening: per-method warning/error flags now update correctly when compat status transitions over time, and call-count sorting now uses total aggregated calls (including unsupported operations).
  - Compat report panel now records bounded per-plugin session score history plus first-seen/last-updated timestamps, and `LAST_UPDATED` sorting projects human-readable recency labels instead of placeholders.
  - Dependency checking between plugins.
  - Event hooks for plugin lifecycle events.
  - Execution context tracking for nested command calls.
  - All methods tagged with `CompatStatus` registry.
- Compat tests:
  - Active in CMake/CI:
    - `test_quickjs_runtime.cpp` for QuickJS context and runtime tests.
    - `test_window_compat.cpp` for WindowCompat surface tests.
    - `test_compat_report_panel.cpp` for compat report model/view coverage.
    - `test_battlemgr.cpp` for BattleManager hook and phase tests.
    - `test_data_manager.cpp` for DataManager save/load tests.
    - `test_audio_manager.cpp` for AudioManager bus/volume tests.
    - `test_input_manager.cpp` for InputManager/TouchInput tests.
    - `test_plugin_manager.cpp` for PluginManager command registry tests.
    - `test_compat_window_plugin_profiles.cpp` for curated 10-plugin profile conformance checks.
    - `test_compat_plugin_fixtures.cpp` for executable 10-plugin fixture loading/execution checks, including deterministic cross-plugin invoke fuzz matrix coverage.
    - `test_compat_plugin_failure_diagnostics.cpp` for curated 10-plugin failure-path diagnostics (missing-command/full-name parse, malformed fixture/load artifacts, malformed command payload shapes and metadata types, fixture script runtime op failures, fixture script validation-shape failures (`set`/`append` key requirements plus malformed `invoke`/`invokeByName` target/store/expect shapes and default `error`-op message path), malformed nested-branch fixture failures, directory-scan failures including deterministic iterator/entry-status branches, dependency execution gating, combined weekly mixed malformed payload/eval/runtime/full-name-parse chain regression with fixture-open/fixture-name/duplicate-load failures, `load_plugin_name` + `load_plugin_register_command` + `load_plugin_register_script_fn` + `load_plugin_quickjs_context` failures, parameter-parse failures, nested `all`/`any` runtime branch failures, strict script-shape runtime failures, and malformed command-shape/name load failures).

### Current Validation Baseline

- Unit tests: 27 files in active `urpg_tests` target:
  - `test_semver.cpp`, `test_fixed32.cpp`, `test_frame_budget.cpp`, `test_ecs_world.cpp`
  - `test_combat_calc.cpp`, `test_bridge_roundtrip.cpp`, `test_thread_roles.cpp`, `test_render_tier.cpp`
  - `test_save_journal.cpp`, `test_canonical_json.cpp`, `test_migration_runner.cpp`, `test_source_authority.cpp`
  - `test_save_recovery.cpp`, `test_save_runtime.cpp`, `test_event_edit_guard.cpp`, `test_event_runtime.cpp`
  - `test_debug_session.cpp`, `test_quickjs_runtime.cpp`, `test_window_compat.cpp`
  - `test_event_authority_diagnostics.cpp`, `test_event_authority_panel.cpp`, `test_compat_report_panel.cpp`
  - `test_battlemgr.cpp`, `test_data_manager.cpp`, `test_audio_manager.cpp`, `test_input_manager.cpp`, `test_plugin_manager.cpp`
- Integration tests: 1 file (`test_integration_runtime_recovery.cpp`).
- Snapshot tests: 1 file (`test_snapshot_canonical_outputs.cpp`).
- Compat tests: 4 files (`test_compat_authority_suite.cpp`, `test_compat_window_plugin_profiles.cpp`, `test_compat_plugin_fixtures.cpp`, `test_compat_plugin_failure_diagnostics.cpp`).
- Debug validation snapshot (2026-04-15):
  - `urpg_tests`: 3907 assertions / 287 test cases

- CLI tools:
  - `urpg_migrate` - migration runner CLI (`tools/migrate/migrate_cli.cpp`)
### Implemented Source Files Map

The following table maps documentation contracts to their actual source implementations:

| Component | Header | Implementation | Test File |
|-----------|--------|----------------|-----------|
| SemVer | `engine/core/semver.h` | `engine/core/semver.cpp` | `tests/unit/test_semver.cpp` |
| Fixed32 | `engine/core/math/fixed32.h` | (header-only) | `tests/unit/test_fixed32.cpp` |
| FrameBudget | `engine/core/perf/frame_budget.h` | (header-only) | `tests/unit/test_frame_budget.cpp` |
| ECS World | `engine/core/ecs/world.h` | (header-only) | `tests/unit/test_ecs_world.cpp` |
| CombatCalc | `engine/gameplay/combat/combat_calc.h` | `engine/gameplay/combat/combat_calc.cpp` | `tests/unit/test_combat_calc.cpp` |
| Bridge Value | `engine/runtimes/bridge/value.h` | (header-only) | `tests/unit/test_bridge_roundtrip.cpp` |
| ThreadRoles | `engine/core/threading/thread_roles.h` | `engine/core/threading/thread_roles.cpp` | `tests/unit/test_thread_roles.cpp` |
| RenderTier | `engine/core/render/render_tier.h` | (header-only) | `tests/unit/test_render_tier.cpp` |
| CanonicalJson | `engine/core/format/canonical_json.h` | `engine/core/format/canonical_json.cpp` | `tests/unit/test_canonical_json.cpp` |
| MigrationRunner | `engine/core/migrate/migration_runner.h` | `engine/core/migrate/migration_runner.cpp` | `tests/unit/test_migration_runner.cpp` |
| SourceAuthority | `engine/core/sync/source_authority.h` | `engine/core/sync/source_authority.cpp` | `tests/unit/test_source_authority.cpp` |
| SaveJournal | `engine/core/save/save_journal.h` | `engine/core/save/save_journal.cpp` | `tests/unit/test_save_journal.cpp` |
| SaveRecovery | `engine/core/save/save_recovery.h` | `engine/core/save/save_recovery.cpp` | `tests/unit/test_save_recovery.cpp` |
| SaveRuntime | `engine/core/save/save_runtime.h` | `engine/core/save/save_runtime.cpp` | `tests/unit/test_save_runtime.cpp` |
| SaveTypes | `engine/core/save/save_types.h` | (header-only) | (via save tests) |
| EventRuntime | `engine/core/events/event_runtime.h` | `engine/core/events/event_runtime.cpp` | `tests/unit/test_event_runtime.cpp` |
| EventEditGuard | `engine/core/events/event_edit_guard.h` | `engine/core/events/event_edit_guard.cpp` | `tests/unit/test_event_edit_guard.cpp` |
| DebugSession | `engine/core/debug/debug_session.h` | `engine/core/debug/debug_session.cpp` | `tests/unit/test_debug_session.cpp` |
| QuickJSRuntime | `runtimes/compat_js/quickjs_runtime.h` | `runtimes/compat_js/quickjs_runtime.cpp` | `tests/unit/test_quickjs_runtime.cpp` |
| WindowCompat | `runtimes/compat_js/window_compat.h` | `runtimes/compat_js/window_compat.cpp` | `tests/unit/test_window_compat.cpp` |
| BattleManager | `runtimes/compat_js/battle_manager.h` | `runtimes/compat_js/battle_manager.cpp` | `tests/unit/test_battlemgr.cpp` |
| DataManager | `runtimes/compat_js/data_manager.h` | `runtimes/compat_js/data_manager.cpp` | `tests/unit/test_data_manager.cpp` |
| AudioManager | `runtimes/compat_js/audio_manager.h` | `runtimes/compat_js/audio_manager.cpp` | `tests/unit/test_audio_manager.cpp` |
| InputManager | `runtimes/compat_js/input_manager.h` | `runtimes/compat_js/input_manager.cpp` | `tests/unit/test_input_manager.cpp` |
| PluginManager | `runtimes/compat_js/plugin_manager.h` | `runtimes/compat_js/plugin_manager.cpp` | `tests/unit/test_plugin_manager.cpp` |
| EventAuthorityDiagnostics | `editor/diagnostics/event_authority_diagnostics.h` | `editor/diagnostics/event_authority_diagnostics.cpp` | `tests/unit/test_event_authority_diagnostics.cpp` |
| EventAuthorityPanel | `editor/diagnostics/event_authority_panel.h` | `editor/diagnostics/event_authority_panel.cpp` | `tests/unit/test_event_authority_panel.cpp` |
| GameplayTags | `engine/core/ability/gameplay_tags.h` | (header-only) | `tests/test_gameplay_tags.cpp` |
| GameplayAbility | `engine/core/ability/gameplay_ability.h` | `engine/core/ability/gameplay_ability.cpp` | `tests/test_gameplay_ability.cpp` |
| AbilitySystemComponent | `engine/core/ability/ability_system_component.h` | `engine/core/ability/ability_system_component.cpp` | `tests/test_ability_system.cpp` |
| AbilityStateMachine | `engine/core/ability/ability_state_machine.h` | `engine/core/ability/ability_state_machine.cpp` | `tests/test_ability_state_machine.cpp` |
| AbilityTask | `engine/core/ability/ability_task.h` | `engine/core/ability/ability_task.cpp` | `tests/test_ability_tasks.cpp` |
| GameplayEffect | `engine/core/ability/gameplay_effect.h` | `engine/core/ability/gameplay_effect.cpp` | `tests/test_gameplay_effect.cpp` |
| PatternField | `engine/core/ability/pattern_field.h` | (header-only) | `tests/test_pattern_field.cpp` |
| PatternFieldSerializer | `engine/core/ability/pattern_field_serializer.h` | `engine/core/ability/pattern_field_serializer.cpp` | `tests/test_pattern_field_serialization.cpp` |
| AbilityInspectorPanel | `editor/ability/ability_inspector_panel.h` | `editor/ability/ability_inspector_panel.cpp` | (manual/panel) |
| AbilityInspectorModel | `editor/ability/ability_inspector_model.h` | `editor/ability/ability_inspector_model.cpp` | (manual/model) |
| LevelAssembly | `engine/core/level/level_assembly.h` | (header-only) | `tests/test_level_assembly.cpp` |
| LevelBlockImporter | `engine/core/level/level_block_importer.h` | (header-only) | `tests/test_level_assembly.cpp` |
| ProceduralToolkit | `engine/core/level/procedural_toolkit.h` | (header-only) | (seeded kernel) |
| FOVSystem | `engine/core/level/fov_system.h` | (header-only) | (seeded kernel) |
| SpritePipeline | `tools/sprite_pipeline/sprite_pipeline_defs.h` | `tools/sprite_pipeline/sprite_pack_main.cpp` | (CLI tool) |
| RaycastRenderer | `engine/core/render/raycast_renderer.h` | (header-only) | (seeded kernel) |
| AnimationClip | `engine/core/animation/animation_clip.h` | (header-only) | (seeded kernel) |
| EditorUtilityTask | `editor/productivity/editor_utility_task.h` | (header-only) | (seeded kernel) |
| PatternAbility | `engine/core/ability/pattern_ability.h` | (header-only) | `tests/test_ability_pattern_integration.cpp` |

### Next Execution Lanes

1. Compat exit hardening: complete remaining curated routed conformance and diagnostics closure until explicit exit criteria are met.
2. Native Feature Absorption Wave 1 runtime delivery: implement native ownership for UI/Menu, Message/Text, Battle, and Save/Data.
3. Wave 1 editor and schema delivery: ship subsystem inspectors, previews, diagnostics, schema finalization, and migration upgrade paths.
4. Native Capability Expansion Wave 2 delivery: implement the advanced capability baseline (ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, editor utilities).
5. Phase 3 Copilot + Polish: Producer Copilot canon-aware generation, cutscene timeline editor, full debugger profiler.

## 0 — Core Philosophy

> *These are the invariants the whole system is built around. Everything downstream derives from them.*

- **Invariant 1:** Compat never degrades Native.

    - MZ compat runs in an isolated lane. A broken plugin cannot corrupt URPG-native state.

- **Invariant 2:** Round-trip is sacred in Compat Mode.

    - If you imported it, you can export it byte-equivalent unless the user explicitly opts into upgrade.

- **Invariant 3:** The editor is the debugger.

    - Play mode, profiler, and breakpoints share the same runtime as shipped builds — not afterthoughts.

- **Invariant 4:** Templates are not skins.

    - Templates define default system wiring. Switching after project start is a destructive operation: warn, checkpoint, migrate.

- Invariant 5: Determinism is a product feature.

  - Deterministic mode exists for CI, replays, and debugging. If it cannot be replayed, it cannot be trusted.

  - Determinism levels are explicit: Level A is required in V1 (authoritative combat + events + RNG); Level B (deterministic physics) is template-gated and reserved for later.

  - Authoritative math is fixed-point (Fixed32 Q16.16) for combat formulas, timers, and replay-critical state. Float is display-only.

  - Deterministic ordering is mandatory: ECS queries iterate EntityID ascending; event handlers execute by priority then registration order.

- Invariant 6: Data is the authority; runtime is a view.

  - All authoritative state is serializable and diffable. Caches exist, but they are never the only copy of truth.

- Invariant 7: Every system has an escape hatch.

  - Safe Mode can bypass plugins, custom scripts, advanced renderer paths, and corrupted saves to recover projects.

- Invariant 8: Least privilege by default.

  - Plugins and scripting sandboxes request explicit permissions; the engine denies by default and logs violations.

## 1 — Product Modes

| **Mode** | **JS Runtime**      | **Lua Runtime** | **WindowCompat** | **Renderer**             |
|----------|---------------------|-----------------|------------------|--------------------------|
| Native   | ✗                   | ✓               | ✗                | URPG GPU                 |
| Compat   | ✓ (QuickJS/Browser) | Optional        | ✓                | URPG GPU + Compat canvas |
| Mixed    | Per-system          | Per-system      | Per-system       | Routed                   |

### Switch Granularity Contract

Each system (UI, message, battle, save, input, audio, map) has exactly one active implementation at a time.

- Mid-session switches allowed: UI, message only.

- Require scene reload: battle, save, input, audio, map.

- Rationale: prevents state corruption bugs before they are written.

## 2 — Tech Stack

### Runtime

- C++ engine with SDL2 (platform, input, audio device).

- Renderer: GPU-backed 2D batching via OpenGL / Metal / Vulkan abstraction.

- Web export: WASM build + WebGL (WebGPU reserved for V2).

- Physics: Box2D — template defaults control behavior.

### Minimum Target Specs — Define Now

> *Everything downstream (batch sizes, WASM budget, texture atlas limits) derives from this. Lock it before writing a line of renderer code.*

| **Platform** | **Minimum**                             |
|--------------|-----------------------------------------|
| GPU          | 2015-era integrated GPU                 |
| RAM          | 4 GB                                    |
| Windows      | Windows 10                              |
| macOS        | macOS 12                                |
| Linux        | Ubuntu 22                               |
| Web          | Chrome 110+ / Firefox 115+ / Safari 16+ |

### Renderer Capability Tiers

- TIER_BASIC — OpenGL 3.3 / WebGL 1

- TIER_STANDARD — OpenGL 4.5 / WebGL 2 / Metal

- TIER_ADVANCED — Vulkan / WebGPU

Features advertise their minimum tier at declaration. Prevents 'works on my machine' shipping bugs.

### Threading Model — Must Be Explicit

| **Thread**      | **Responsibility**           | **Script Access**  |
|-----------------|------------------------------|--------------------|
| Render          | Draw calls, buffer swaps     | None               |
| Logic           | QuickJS + Lua VM, game state | Full               |
| Audio           | Mix, stream, decode          | None               |
| Asset Streaming | Async load from disk         | Via callbacks only |

No cross-thread script calls. Asset loads are always async from scripts.

### Engine Core Object Model (ECS) — Contract

ECS World is the internal object model. Entities are stable IDs; components are pure data; systems register/deregister on World. Templates decide which systems are active.

Deterministic iteration is required: any forEachWith(...) iteration must be EntityID-ascending, and component storage order must never change outcomes.

### Determinism Mode Levels — Contract

Projects declare their determinism mode up-front. V1 requires Level A; templates may opt into higher levels later.

```json
// project.json (determinism declaration)
{
"determinism": {
"level": "A",
"authoritative_math": "fixed32",
"physics": { "authoritative": false, "note": "float physics excluded from replay determinism in V1" }
}
}
```

### Authoritative Math — Fixed32

All replay-critical math uses Fixed32 (Q16.16). Float is permitted only for rendering, interpolation, and UI display.

### Frame Budget Enforcement — Contract

Budgets are enforced at runtime, not advisory. The engine reserves a non-preemptible slice; the plugin pool is time-sliced and can be preempted per severity rules.

```cpp
// engine/core/perf/frame_budget.h
struct FrameBudget {
uint32_t total_us = 16'667; // 60fps
uint32_t native_reserve = 8'000; // guaranteed for engine
uint32_t plugin_pool = 6'000; // shared across plugins
uint32_t headroom = 2'667; // buffer
};
```

Each plugin declares a priority tier; slices are allocated proportionally. Overages trigger WARN → SOFT_FAIL → HARD_FAIL (plugin isolated) depending on sandbox policy.

### Scripting

- Lua VM — URPG-native runtime.

- QuickJS on desktop — MZ plugins.

- Browser JS on web — MZ plugins via WASM bridge.

- Lua overrides JS at all hook points. Bridge is bidirectional via URPGCompat APIs.

### Editor

- Native app (C++ + ImGui or equivalent).

- Dockable pro panels, command palette, global search.

- Embedded play mode + debugger/profiler sharing shipped runtime.

- Project versioning is a first-class editor feature — not a user responsibility.

    - Auto-checkpoint on play, on export, and on destructive operations.

    - Git integration is a layer on top, not a replacement.

- Live asset hot-reload: map, script, and image changes reflect in running play session without restart.

- Compat Report panel: per-plugin status, warnings trending over time, one-click jump to offending line.

### Command Palette Schema

All editor commands are registered via schema (id/label/shortcut/category/contexts/handler). This makes commands discoverable, keybind-remappable, scriptable, and testable.

```json
CommandPalette::register({
.id = "editor.play_mode.start",
.label = "Start Play Mode",
.shortcut = "F5",
.category = "Play",
.contexts = { "editor", "project_open" },
.handler = []{ EditorApp::startPlayMode(); }
});
```

### Storage

- JSON canonical content — Git-friendly, human-readable.

- SQLite index / cache / search layer on top.

### Input Remapping — Action Map Schema (Required)

Input is action-based (not key-based). Templates and plugins bind to action IDs; users remap per-context without breaking content.

```json
// input_action_map.json (schema sketch)
{
"actions": [
{ "id": "move_up", "type": "axis", "default": ["kb:W", "gp:left_stick_y-"] },
{ "id": "confirm", "type": "button", "default": ["kb:Enter", "gp:A"] }
],
"contexts": [
{ "id": "gameplay", "active_when": "Scene_Map" },
{ "id": "menu", "active_when": "Scene_Menu" }
]
}
```

### Audio Middleware Contract (Above SDL2)

SDL2 is the device layer only. URPG defines an audio middleware layer with buses (BGM/SFX/UI/VO), group volumes, ducking rules, and crossfade contracts.

Crossfades are deterministic (time-based in frames in deterministic mode). Ducking rules are explicit per bus and per cue.

### Map Chunk Streaming Contract (V1)

Large maps are split into fixed-size chunks (default 32x32 tiles). Runtime keeps chunks within radius R loaded, prefetches one ring ahead, and evicts via LRU. Chunk load/unload must not change authoritative state ordering.

### Network / Multiplayer Stub (Reserved)

Even if multiplayer is V3, the runtime reserves a NetworkManager interface now. Default is null/disabled; templates can opt in later without rewriting core systems.

```cpp
// engine/net/network_manager.h (stub)
#pragma once
namespace urpg {
class NetworkManager {
public:
virtual ~NetworkManager() = default;
virtual bool IsEnabled() const = 0;
};
}
```

## 3 — File Formats & Strict MZ Round-Trip

### Versioning Contract

> *Every URPG file must carry format version and minimum engine version. Without this, 'strict round-trip' is aspirational.*

Required fields on every file:

- \_urpg_format_version: "1.0"

- \_engine_version_min: "0.9.0"

A migration runner takes any file from version N to N+1 and is executed in CI on every build.

### Source-of-Truth Contract

The biggest ambiguity in v1 — resolved here:

| **Mode**    | **Source of Truth**                         | **Derived View**               |
|-------------|---------------------------------------------|--------------------------------|
| Compat Mode | Raw MZ command list                         | URPG AST (read-only)           |
| Native Mode | URPG AST                                    | No raw list exists             |
| Mixed Mode  | Per-event block, tagged with authority mode | Whichever is not authoritative |

Edits in the URPG graph editor in Compat Mode write back to the raw list. The AST is never the authority in Compat.

### MZ Compat Store

- URPG stores verbatim copies of data/\*.json including Map###.json.

- Plugin load configs stored opaque.

- Mapping metadata kept in sync with URPG AST.

### Import Modes

- Strict Compat — no changes unless user triggers upgrade. Round-trip guaranteed.

- Upgrade — normalize fully into URPG-native. May break round-trip. Requires explicit user confirmation.

## 4 — MZ Plugin Compatibility Layer

### Compatibility Surface Registry

Every MZ API surface URPG implements carries a published status tag. This registry is a versioned doc page updated with each release.

| **Tag**     | **Meaning**                                |
|-------------|--------------------------------------------|
| FULL        | Behaves identically to MZ                  |
| PARTIAL     | Documented deviations listed               |
| STUB        | Present but no-ops (optional warnings)     |
| UNSUPPORTED | Throws gracefully; logged in Compat Report |

### WindowCompat — Explicit Surface

'Enough for common plugins' is not shippable. V1 must implement at minimum:

- Window_Base: drawText, drawIcon, drawActorFace, drawActorName, drawActorLevel, drawActorHp/Mp/Tp gauges.

- Window_Selectable + Window_Command — required for any menu plugin.

- Sprite_Character, Sprite_Actor — required for most battle visual plugins.

- BattleManager pipeline hooks.

- DataManager save/load semantics + header extensions.

- Input / TouchInput behavior matching.

- AudioManager semantics.

- Plugin command registry + execution.

### Compat Sandbox — Severity Model

| **Severity**    | **Behavior**                                              |
|-----------------|-----------------------------------------------------------|
| WARN            | API missing but game continues normally                   |
| SOFT_FAIL       | System fallback activated (e.g., MZ battle → URPG battle) |
| HARD_FAIL       | Plugin isolated, game continues without it                |
| CRASH_PREVENTED | Would have crashed; logged with full stack trace          |

## 5 — Plugin Ecosystem

### URPG Plugin API (Lua) — V1

- Runtime plugins only in V1.

- Versioned APIs with capability permissions.

- Hook points: events, combat, UI, map, save, audio.

### Plugin Distribution — V1 Minimum

Even without a marketplace, these decisions must be made now:

- Installation: folder drop only in V1.

- Each plugin ships a plugin.json manifest declaring id, version, urpg_api compatibility, conflicts, and permissions.

- Dependency resolution: manual in V1; automatic in V2.

Plugin Lifecycle Hooks (Required)

Plugins have an explicit lifecycle: onInstall, onEnable, onDisable, onUninstall. Lifecycle hooks run before gameplay hooks. Order is deterministic (plugin load order).

Plugin Conflict Graph (Ownership + Patches)

Conflict detection is data-first. Plugins declare what they own/patch and any known conflicts so the editor can render a dependency/conflict graph and block incompatible enable states.

Plugin Hot-Reload Boundaries (Safety Contract)

Hot-reload is scoped by what state is safe to preserve:

- SAFE: UI overlays, editor panels, cosmetic shaders.

- REQUIRES_SCENE_RELOAD: battle, save, map, input, audio implementation changes.

- FORBIDDEN_MID_SESSION: anything that changes ECS component layouts or authoritative save schema.

```json
// plugin.json lifecycle + ownership example
{
"id": "my_plugin",
"version": "1.0.0",
"urpg_api": "\>=0.9.0",
"permissions": ["save.read", "ui.overlay"],
"hooks": {
"onInstall": "scripts/install.lua",
"onEnable": "scripts/main.lua",
"onDisable": "scripts/teardown.lua",
"onUninstall": "scripts/cleanup.lua",
"onSaveWrite": "scripts/save_hook.lua",
"onMapLoad": "scripts/map_hook.lua"
},
"owns": ["BattleManager.prototype.startBattle", "Window_BattleLog"],
"patches": ["Scene_Battle"],
"conflicts_with": ["other_battle_plugin"]
}
```

### Lua ↔ JS Bridge Contract

Define the marshalling rules explicitly — every bridge call is an adventure without this:

- JS objects → Lua tables.

- JS arrays → Lua sequences.

- JS primitives → Lua primitives.

- Circular references → error, not undefined behavior.

- Functions → wrapped as callable userdata, never cloned.

### URPGCompat.js

- Always included in MZ plugin runtime.

- Provides missing APIs, standardizes hook points across QuickJS and browser JS, allows aggressive patches.

- Logging and inspection tools for cross-runtime debugging.

## 6 — Templates

Templates define default system wiring, not just assets. Each ships a capability manifest:

| **Template**      | **Battle**               | **Save**                | **UI Skin**           | **Upgrade Path**       |
|-------------------|--------------------------|-------------------------|-----------------------|------------------------|
| JRPG (Turn-Based) | URPG turn-based (locked) | Slot-based (switchable) | Fantasy (switchable)  | → Action RPG (partial) |
| Action RPG        | Real-time (locked)       | Checkpoint (switchable) | Action (switchable)   | → JRPG (partial)       |
| Visual Novel      | None                     | Scene-based             | VN Frame (switchable) | → JRPG (full)          |
| Tactics / SRPG    | Grid-turn (locked)       | Slot-based              | Tactical (switchable) | → JRPG (partial)       |

> *Tactics/SRPG is a new addition. Grid movement, team management, and turn order queue are underserved by existing tools and fit naturally into URPG's grid-based map model. Even a minimal V1 implementation differentiates significantly.*

Template Switching Contract (Non-Destructive Migration)

Template switching is treated as a migration, not a style change. The editor must show a diff preview and create a checkpoint before applying any destructive rewiring.

Diff preview separates:

- Preserved: assets, localization, save format, project DB tables (IDs stable).

- Regenerated: default system wiring, battle defaults, UI layouts/skins, template-provided events/macros.

- Conditional migrations: maps/events that are template-specific (must declare upgrade path or be blocked).

After switching, rollback is one-click to the pre-switch checkpoint (no manual cleanup).

## 7 — Event System + Debugger

### Event Authoring

- List view + Graph view — both first-class.

- Macros/functions as first-class event constructs.

- Cutscene timeline editor with audio sync preview.

- Timeline can be baked to a flat event list (for performance or shipping).

- Timeline files are standalone and reusable across projects.

### Execution Safety Guarantees

- Infinite loop detection: events looping \> N iterations without yielding trigger a soft breakpoint, not a hang. N is configurable.

- Reentrancy: events are not reentrant by default. A per-event flag enables it with a configurable depth limit.

- Concurrent variable writes: two events modifying the same variable in the same frame logs a conflict warning.

Event Priority & Ordering

Event execution order is explicit and deterministic: CRITICAL \> HIGH \> NORMAL \> LOW \> LAST. Equal priorities execute in registration order.

Event Cancellation and Default Behavior

Handlers can stop propagation (cancel) and can skip engine default behavior (preventDefault). Both actions are logged for debuggability.

Async Event Coroutines (Cutscenes / Timelines)

Async event scripts run as coroutines/fibers to avoid callback chains. Yield points preserve call stacks for the debugger and remain deterministic under replay.

```lua
-- Lua event registration with priority + cancellation
EventManager.on("item_use", function(ctx)
```

if ctx.item.id == "forbidden_scroll" then

```
ctx.cancel()
ctx.preventDefault()
```

end

end, { priority = EventPriority.HIGH, reentrancy = false })

```lua
-- Coroutine-based cutscene/timeline execution
EventScript.run(function(e)
e.showMessage("The door creaks open...")
e.await(e.playSound("door_creak"))
e.waitFrames(60)
e.fadeOut(30)
e.transferPlayer(MAP_ID, 5, 8)
e.fadeIn(30)
```

end)

### Debugger — V1 Feature Set

> *This is a flagship feature. It should feel like debugging code, not guessing.*

- Breakpoints at command level.

- Step over / step into macros.

- Watch variables and switches.

- Full call stack display.

- Execution timeline (frame-by-frame replay).

- Variable change history: 'who changed this variable and when?'

- Profiler: slowest events, execution counts, cumulative time.

- Available in exported builds behind a launch flag — not editor-only.

## 8 — Producer Copilot

> *'2-minute quest' is a marketing promise. This section defines the actual behavioral contract.*

### Session Model

- Copilot operates in scoped sessions: one quest, one dungeon, or one NPC chain at a time.

- Each session produces a reviewable diff before any write. Nothing is applied silently.

- Canon awareness: Copilot reads the existing project DB before generating. It will not reference skills, items, or maps that do not exist yet.

- All Copilot outputs are tagged copilot_generated: true in the database for filtering, auditing, and bulk-editing.

- Revision flow: user can say 'make the dungeon harder' and receive a targeted diff, not a full regeneration.

Canon Constraint Validator (Preflight)

Before generating anything, Copilot runs a constraint check against the project DB and surfaces blockers instead of emitting broken content.

❌ Cannot generate quest: references Skill ID 42 ("Thunder Blade") — not in project DB

⚠️ References map "Ancient Ruins" — exists but has no spawn points defined

✅ Party members, items, switches all resolve

Copilot Undo / Rollback (Atomic Sessions)

Each Copilot session is applied as a single atomic edit group tagged with a session UUID, so the user can undo the entire session in one operation.

Difficulty Knobs as First-Class Parameters

Difficulty revisions map to explicit levers so changes are reproducible and auditable (no hidden heuristics).

```json
// Difficulty knobs (example)
{
"difficulty_delta": +1,
"enemy_hp_scale": 1.15,
"enemy_count_scale": 1.20,
"trap_density_delta_per_room": 1,
"healing_item_density_delta_per_room": -1,
"boss_phase_delta": 1
}
```

### Output Contract

| **Output Type**  | **Contents**                                                                |
|------------------|-----------------------------------------------------------------------------|
| Maps Blueprint   | 2 maps: town + dungeon with descriptor files                                |
| NPCs             | 6 NPCs with roles, dialogue trees, event scripts                            |
| Quest Steps      | 3 steps with triggers, conditions, and completion flags                     |
| Events / Macros  | All event logic generated as URPG-native or MZ-compat based on project mode |
| Asset Selections | References to existing library assets or prompts for missing ones           |
| Diff Preview     | Patchable by section — user accepts/rejects per block                       |

### Missing Asset Flow (3 options)

- Stop and ask — Copilot pauses session until resolved.

- Insert placeholder — named slot in asset manifest, flagged for later.

- Generate via AI — calls external image gen API to produce a candidate (V1 optional; V2 standard).

## 9 — Asset System

### Asset Pipeline

- All imported images go through a normalization step: power-of-2 atlas packing, format conversion to engine-native (BC7 / ETC2 compressed textures).

- Raw source files are always preserved; engine files are derived and rebuilable.

- Asset fingerprinting: every asset gets a content hash. Duplicate detection is automatic. Renaming never breaks references.

### License Tagging Schema

| **Tag**    | **Meaning**                      |
|------------|----------------------------------|
| CC0        | Public domain — no restrictions  |
| CC-BY      | Attribution required             |
| commercial | Licensed for commercial use      |
| private    | Not cleared for distribution     |
| unknown    | No license information available |

Projects can opt into a pre-export license audit listing every asset tagged unknown or private. One day to implement; prevents serious user headaches.

- URPG indexes and tags assets without reorganizing source files.

- External libraries (e.g., private asset folders) can be attached as read-only mounts.

## 10 — Save System (New)

> *This section was entirely absent from v1. Save format decisions cannot be deferred — plugins, exports, and cloud sync all depend on them.*

- Save format: versioned JSON envelope + binary blob for large state (e.g., map state snapshots).

- Save modes: slot-based, named saves, and autosave — all three, configured per template.

- Save data is observable by plugins via a typed API. No direct file access from scripts.

- Cloud save: interface reserved now (SaveManager.syncProvider = null by default). Implemented in V2.

- Migration runner handles save format upgrades, same as asset/map format versioning.

Save Slot Metadata Schema (UI-Facing)

Save UI must never parse the binary blob to list saves. Slot metadata is stored in a separate JSON envelope/index used for UI and quick integrity checks.

```json
// save_slot_meta.json (per-slot envelope example)
{
"_slot_id": 3,
"_save_version": "1.1",
"_timestamp": "2026-03-03T14:22:00Z",
"_playtime_seconds": 7342,
"_thumbnail_hash": "a1b2c3d4",
"_map_display_name": "Crimson Keep - B2",
"_party_snapshot": [
{ "name": "Aria", "level": 18, "hp": 340, "max_hp": 340 }
],
"_flags": { "autosave": true, "copilot_generated": false, "corrupted": false },
"data": "slot_003.bin"
}
```

Journaling + Integrity Writes

All saves are journaled: write temp → fsync → atomic rename. The previous good save is kept as slot.backup until the next successful write.

Save Corruption Recovery Tiers

- Level 1: reload from last autosave checkpoint.

- Level 2: load metadata + variables only (discard map snapshots/blobs).

- Level 3: Safe Mode skeleton load (party preserved, variables reset, spawn at last known map origin).

## 11 — Localization (New)

> *Also absent from v1. The 'deep database schema' mentioned localization without specifying it.*

- All user-visible strings live in locale files keyed by ID. Zero hardcoded strings anywhere in the engine or editor.

- Locale files use JSON with ICU message format for pluralization rules.

- Right-to-left support: reserved in V1. Layout engine must not assume LTR.

- Font fallback chains defined per locale.

- Copilot generates localization keys alongside content, not raw strings.

Locale Fallback Chain

Missing keys follow an explicit fallback chain: active locale → language → en-US → display key ID + warning. This prevents silent blank strings in partial translations.

Locale Hot-Reload (Editor + Play Mode)

Locale files hot-reload live so translators and writers can validate changes without restarting the editor or play session.

Copilot Locale Output Contract

Copilot emits locale keys alongside default strings and flags review requirements.

```json
{
"key": "npc.inn_keeper.greeting",
"default_locale": "en-US",
"value": "Welcome, weary traveler. A room for the night?",
"copilot_generated": true,
"needs_review": true
}
```

## 12 — Build & Export Pipeline (New)

### Export Targets

| **Target**         | **V1** | **V2**  |
|--------------------|--------|---------|
| Windows            | Yes    | --      |
| macOS              | Yes    | --      |
| Linux              | Yes    | --      |
| Web (WASM + WebGL) | Yes    | --      |
| Android            | No     | Planned |
| iOS                | No     | Planned |

### Export Guarantees

- Every export produces an auditable manifest of all included assets, scripts, and plugins.

- Compat Mode exports bundle the QuickJS runtime (desktop) or browser JS runtime (web). Budget: \< 5 MB overhead.

- Optional JS obfuscation for plugins. Engine code is not obfuscated.

- Pre-export license audit runs automatically if any asset is tagged unknown or private.

## 13 — Testing Infrastructure (New)

Testing is a first-class feature. CI enforces it. No optional testing culture.

Test Pyramid (Required)

- Unit tests (fast): core libraries (formatting, serialization, rules, bridge marshalling, math, pathing).

- Integration tests: headless play mode running scripted scenarios and asserting state + logs.

- Snapshot tests: renderer output diffs against golden images on each renderer tier target.

- Compat tests: curated MZ plugins + URPGCompat surfaces verified by automated conformance probes.

- Performance tests: microbenchmarks + budget regression checks (frame time, memory, startup).

### Required for Maintaining Quality Over Time

- Headless play mode: run a project without a display window, driven by a test script. Required for automated regression testing of event logic and combat balance.

- Snapshot testing for scenes: render a scene, compare against a reference image. Catches renderer regressions automatically.

- Plugin compat CI: a curated suite of popular MZ plugins runs against every URPG build. New breakage is flagged before release. This is how 'MZ-enough' stays true over time without silently rotting.

- Format migration CI: every file format version migration is tested on a set of reference projects.

## 14 — Priority Build Sequencing (New)

> *The v1 spec was a flat list. Build in this order to minimize rework. Nothing in Phase N+1 should require redesigning Phase N.*

### Phase 0 — Foundation

Do not build anything else until these are locked:

- Threading model.

- Renderer capability tiers.

- File format versioning contract.

- Save format.

- Source-of-truth contract for event syncing.

### Phase 1 — Native Core

- URPG event system.

- Debugger: breakpoints + watch only.

- JRPG template.

- Basic asset pipeline (import + normalize).

- Editor with play mode.

### Phase 2 — Compat Layer

- QuickJS runtime integration.

- WindowCompat core surface (FULL and PARTIAL surfaces only).

- BattleManager hooks.

- Compat Sandbox + Report panel.

- Validate against 10 real-world popular MZ plugins. Fix before proceeding.

### Phase 3 — Copilot + Polish

- [x] Producer Copilot kernel (canon-aware validation primitives in `engine/core/copilot/copilot_kernel.h`).

- [x] Cutscene timeline editor kernel (`engine/core/animation/timeline_kernel.h`).

- [x] Full debugger: profiler + variable history (`engine/core/debug/full_debugger.h`).

- [x] Action RPG Template core (`arpg_core.h`) + Editor UI (`template_ui.h`).

- [x] Visual Novel Template core (`vn_core.h`) + Editor UI (`template_ui.h`).

- [x] Tactics / SRPG template core (`engine/core/templates/tactics_core.h`) + Editor UI (`template_ui.h`).

- [x] Plugin manifest system (`plugin.json` schema and `engine/core/plugin/plugin_manifest.h` parser).

- [x] Asset license audit + pre-export report (implemented in `engine/core/asset/asset_license_audit.h`).

- [x] Headless play mode baseline (implemented in `engine/core/testing/headless_play_mode.h`).

- [x] Snapshot testing for scenes (implemented in `engine/core/testing/snapshot_validator.h`).

- [x] Cloud save interface (stubbed in `engine/core/social/cloud_service.h`).

*URPG Master Blueprint v2 — Confidential Engineering Spec*

15 — Security, Permissions, and Trust (New)

This section formalizes the safety contract for scripts, plugins, and imported content. It is required for 'Compat never degrades Native' to be enforceable.

Permission Model

- Permission is declared in plugin.json and reviewed in-editor on install/update.

- Denied by default: if permission is missing, the capability is unavailable and a WARN is logged.

- Permissions are scoped: project-wide, per-save, per-map, or per-scene (declared).

| Permission         | Scope   | Allows                                                                               |
|--------------------|---------|--------------------------------------------------------------------------------------|
| fs.read_project    | Project | Read-only access to project files (native lane only; compat lane gets a virtual FS). |
| fs.write_project   | Project | Write access to whitelisted project paths (never raw engine binaries).               |
| net.http           | Project | Outbound HTTP(S) for analytics, mod downloads, or cloud services.                    |
| input.global_hooks | Runtime | Non-standard input hooks (e.g., raw mouse/IME).                                      |
| debug.attach       | Runtime | Debugger attach in exported builds (requires user launch flag).                      |

Sandbox Boundaries

- Compat JS runs in a virtual filesystem with explicit mounts; no direct disk access.

- Native Lua plugins can read/write only through typed engine APIs (SaveManager, AssetManager, etc.).

- CPU budgets per frame per plugin: configurable hard limit with SOFT_FAIL fallback to preserve gameplay.

- Memory budgets per plugin runtime lane: enforceable ceiling; exceeding triggers HARD_FAIL for that plugin only.

Permission Prompt UX Contract

When installing/updating a plugin, the editor shows requested permissions, blocked permissions, and per-permission explanations with explicit Allow/Deny actions.

Plugin "VisuMZ_BattleCore" is requesting:

ui.overlay - draw over game canvas

save.read - read save data

save.write - modify save data (explain why)

net.http - BLOCKED (not declared in plugin.json)

\[Allow All\] \[Allow Selected\] \[Deny\]

Permission Audit Log

Every permission use emits a structured audit event (jsonl). The editor provides filtering (by plugin, permission, save slot, time) and export for debugging and trust.

```cpp
// audit_event.jsonl (one event)
{ "ts": "2026-03-03T14:22:12Z", "plugin": "battle_core", "permission": "save.write", "call": "SaveManager.setFlag", "key": "boss_defeated_3" }
```

Enforcement Notes

CPU/memory limits are enforced via the frame budget + sandbox policy (see Tech Stack). Violations are denied-by-default and logged with callsite + plugin id.

16 — Observability, Diagnostics, and Crash Recovery (New)

If the editor is the debugger, the shipped game must also be diagnosable. This section defines the contract.

Telemetry (Local-First)

- All diagnostics are local-first: logs, traces, and crash dumps are stored locally and shareable by the user.

- No network telemetry is enabled by default. Any outbound telemetry requires explicit opt-in + plugin permission net.http.

Structured Logging

- Every subsystem logs structured events (JSON lines): {ts, level, subsystem, event, fields...}.

- Export builds include a rolling log buffer; when a crash occurs, last N seconds are flushed to disk.

- Compat lane logs include plugin id + callsite; native lane logs include system id + event id.

Crash Recovery and Safe Mode

- Safe Mode is launchable from the editor and from exported builds via a flag.

- Safe Mode disables: all third-party plugins, all custom scripts (optional), advanced renderer paths, and uses conservative defaults.

- If project fails to boot normally, editor auto-offers Safe Mode and opens the Compat Report / Crash Report panels.

- Autosave is paused if corruption is detected to avoid overwriting the last known-good save.

17 — Repository Layout, Tooling, and CI (New)

This is the minimum repo layout required to make the testing strategy in Section 13 enforceable.

Canonical Repo Layout

Suggested tree:

urpg/

engine/ \# C++ runtime (SDL2 + renderer abstraction + core systems)

editor/ \# native editor (ImGui) + tooling panels

runtimes/

lua/ \# URPG-native scripting runtime + stdlib

compat_js/ \# QuickJS build + polyfills + URPGCompat.js

web_bridge/ \# WASM bridge for browser JS compat

content/

schemas/ \# JSON schemas for all URPG files

reference_projects/ \# golden projects for migrations + compat CI

tools/

migrate/ \# migration runner CLI

pack/ \# export packager

snapshot/ \# headless snapshot runner

tests/

unit/ \# pure unit tests (fast)

integration/ \# headless play mode + event scripts

snapshot/ \# renderer snapshots (image diffs)

compat/ \# plugin compat suite (MZ plugins + assertions)

docs/

third_party/

CI Gates

- Gate 1 (PR): unit + format validation + migration tests (fast, deterministic).

- Gate 2 (nightly): headless play suites + snapshot tests on each renderer tier target.

- Gate 3 (weekly): curated plugin compat suite across OS targets; regressions block release branch.

- No bypass: failures require either a fix or an explicit 'known break' waiver with an issue link and expiry date.

18 — Test Anchors: Reference Code + Sample Tests (New)

This section includes small reference implementations so the test examples are anchored to real code contracts (not guesses). These are not the full engine - they are the minimum 'contract kernels' every subsystem must match.

### 18.1 SemVer (C++)

```cpp
// engine/core/semver.h
#pragma once
#include <cstdint>
#include <string>

namespace urpg {
struct SemVer {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;

    static bool TryParse(const std::string& s, SemVer& out);
    std::string ToString() const;
};

inline bool operator==(const SemVer& a, const SemVer& b) {
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}

inline bool operator<(const SemVer& a, const SemVer& b) {
    if (a.major != b.major) return a.major < b.major;
    if (a.minor != b.minor) return a.minor < b.minor;
    return a.patch < b.patch;
}
} // namespace urpg
```

### 18.2 Unit Test Example (Catch2)

```cpp
// tests/unit/test_semver.cpp
#include "engine/core/semver.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("SemVer parses valid versions", "[semver]") {
    urpg::SemVer v;
    REQUIRE(urpg::SemVer::TryParse("1.2.3", v));
    REQUIRE(v.major == 1);
    REQUIRE(v.minor == 2);
    REQUIRE(v.patch == 3);
    REQUIRE(v.ToString() == "1.2.3");
}

TEST_CASE("SemVer rejects invalid versions", "[semver]") {
    urpg::SemVer v;
    REQUIRE_FALSE(urpg::SemVer::TryParse("1.2", v));
    REQUIRE_FALSE(urpg::SemVer::TryParse("a.b.c", v));
}
```

18.3 Migration Runner Contract (JSON Patch)

All format migrations are pure functions: input JSON -\> output JSON. No side effects. This makes migrations testable and reversible.

```json
// tools/migrate/migration_op.json (example)
{
"from": "1.0",
"to": "1.1",
"ops": [
{ "op": "rename", "fromPath": "/player/atk", "toPath": "/player/attack" },
{ "op": "set", "path": "/_urpg_format_version", "value": "1.1" }
]
}
```

### 18.4 Fixed32 Determinism Kernel (Q16.16)

Deterministic Mode must avoid float math for authoritative gameplay state. Fixed32 is Q16.16: enough range for RPG formulas while staying fast and predictable.

```cpp
// engine/core/math/fixed32.h
#pragma once
#include <cstdint>

namespace urpg {
struct Fixed32 {
    int32_t raw = 0; // Q16.16

    static constexpr Fixed32 FromInt(int32_t v) {
        return Fixed32{v << 16};
    }

    static constexpr Fixed32 FromRaw(int32_t r) {
        return Fixed32{r};
    }

    // For display/debug only. Never use for authoritative state.
    float ToFloat() const {
        return static_cast<float>(raw) / 65536.0f;
    }

    friend constexpr Fixed32 operator+(Fixed32 a, Fixed32 b) {
        return Fixed32{a.raw + b.raw};
    }

    friend constexpr Fixed32 operator-(Fixed32 a, Fixed32 b) {
        return Fixed32{a.raw - b.raw};
    }

    friend constexpr Fixed32 operator*(Fixed32 a, Fixed32 b) {
        return Fixed32{static_cast<int32_t>(
            (static_cast<int64_t>(a.raw) * static_cast<int64_t>(b.raw)) >> 16
        )};
    }

    friend constexpr bool operator<(Fixed32 a, Fixed32 b) {
        return a.raw < b.raw;
    }

    friend constexpr bool operator==(Fixed32 a, Fixed32 b) {
        return a.raw == b.raw;
    }
};
} // namespace urpg
```

```cpp
// tests/unit/test_fixed32.cpp
#include "engine/core/math/fixed32.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Fixed32 multiplies deterministically", "[math]") {
    using urpg::Fixed32;
    auto a = Fixed32::FromInt(3);
    auto b = Fixed32::FromInt(2);
    auto c = a * b;
    REQUIRE(c.raw == Fixed32::FromInt(6).raw);
}

TEST_CASE("Fixed32 handles negatives", "[math]") {
    using urpg::Fixed32;
    auto a = Fixed32::FromInt(-3);
    auto b = Fixed32::FromInt(2);
    REQUIRE((a * b).raw == Fixed32::FromInt(-6).raw);
}
```

### 18.5 FrameBudget Allocation Kernel

Frame budgets must be enforceable and testable. Time accounting uses injected clocks so tests stay deterministic.

```cpp
// engine/core/perf/frame_budget.h
#pragma once
#include <cstdint>

namespace urpg {

enum class BudgetTier : uint8_t {
    CRITICAL = 0,
    HIGH = 1,
    NORMAL = 2,
    LOW = 3
};

struct FrameBudget {
    uint32_t total_us = 16667;      // 60fps
    uint32_t native_reserve = 8000;
    uint32_t plugin_pool = 6000;
    uint32_t headroom = 2667;
};

struct PluginBudgetDecl {
    BudgetTier tier = BudgetTier::NORMAL;
    uint32_t weight = 1;  // proportional slice within tier
};

} // namespace urpg
```

```cpp
// tests/unit/test_frame_budget.cpp
#include "engine/core/perf/frame_budget.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("FrameBudget totals sum correctly", "[perf]") {
    urpg::FrameBudget b;
    REQUIRE(b.native_reserve + b.plugin_pool + b.headroom == b.total_us);
}
```

### 18.6 ECS World Kernel (Deterministic Iteration)

ECS is the internal object model. Determinism requires stable iteration order: ForEachWith must iterate entities in ascending EntityID order.

```cpp
// engine/core/ecs/world.h
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace urpg {

using EntityID = uint32_t;

class World {
public:
    EntityID CreateEntity() {
        const EntityID id = next_id_++;
        alive_.push_back(id);
        return id;
    }

    void DestroyEntity(EntityID id) {
        const auto iter = std::lower_bound(alive_.begin(), alive_.end(), id);
        if (iter != alive_.end() && *iter == id) {
            alive_.erase(iter);
        }
    }

    template <typename T>
    T& AddComponent(EntityID id, const T& component) {
        auto& storage = TypedStorage<T>();
        storage[id] = component;
        return storage[id];
    }

    template <typename T>
    T* GetComponent(EntityID id) {
        auto& storage = TypedStorage<T>();
        auto iter = storage.find(id);
        if (iter == storage.end()) {
            return nullptr;
        }
        return &iter->second;
    }

    template <typename... Ts, typename Fn>
    void ForEachWith(Fn&& fn) {
        for (EntityID id : alive_) {
            if constexpr (sizeof...(Ts) == 0) {
                fn(id);
            } else {
                if ((GetComponent<Ts>(id) && ...)) {
                    fn(id, *GetComponent<Ts>(id)...);
                }
            }
        }
    }

private:
    struct IStorage {
        virtual ~IStorage() = default;
    };

    template <typename T>
    struct Storage final : IStorage {
        std::unordered_map<EntityID, T> data;
    };

    EntityID next_id_ = 1;
    std::vector<EntityID> alive_;
    std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages_;

    template <typename T>
    std::unordered_map<EntityID, T>& TypedStorage();
};

} // namespace urpg
```

```cpp
// tests/unit/test_ecs_world.cpp
#include "engine/core/ecs/world.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("World creates monotonic EntityIDs", "[ecs]") {
    urpg::World w;
    auto a = w.CreateEntity();
    auto b = w.CreateEntity();
    REQUIRE(a < b);
}

TEST_CASE("ForEachWith iteration is deterministic by EntityID", "[ecs]") {
    urpg::World w;
    auto e1 = w.CreateEntity();
    auto e2 = w.CreateEntity();
    REQUIRE(e1 < e2);
}
```

### 18.7 CombatCalc Kernel + Baseline Unit Tests

Combat formulas must be anchored by deterministic tests. The kernel below is intentionally small but defines the baseline contract for physical damage.

```cpp
// engine/gameplay/combat/combat_calc.h
#pragma once
#include "engine/core/math/fixed32.h"
#include <cstdint>

namespace urpg {

struct ActorStats {
    int32_t level = 1;
    Fixed32 atk = Fixed32::FromInt(0);
    Fixed32 def = Fixed32::FromInt(0);
};

struct DamageResult {
    int32_t damage = 0;
    bool critical = false;
};

class CombatCalc {
public:
    // variance_seed is authoritative input for determinism.
    DamageResult PhysicalDamage(const ActorStats& a, const ActorStats& d, uint32_t variance_seed) const;
};

} // namespace urpg
```

```cpp
// tests/unit/test_combat_calc.cpp
#include "engine/gameplay/combat/combat_calc.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Physical damage formula baseline", "[combat]") {
    urpg::CombatCalc calc;
    urpg::ActorStats attacker{ .level=10, .atk=urpg::Fixed32::FromInt(100), .def=urpg::Fixed32::FromInt(0) };
    urpg::ActorStats defender{ .level=10, .atk=urpg::Fixed32::FromInt(0), .def=urpg::Fixed32::FromInt(50) };
    auto result = calc.PhysicalDamage(attacker, defender, /*variance_seed=*/0);
    REQUIRE(result.damage == 50); // variance_seed=0 locks variance to 0% in the kernel
    REQUIRE_FALSE(result.critical);
}
```

### 18.8 Bridge Marshalling Value Model + Round-Trip Tests

Instead of unit-testing QuickJS and Lua directly, define a runtime-agnostic Value model. Each runtime marshals to/from Value. Round-trip tests run purely in C++ and cannot drift.

```cpp
// engine/runtimes/bridge/value.h
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace urpg {

struct Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value>; // ordered for determinism

struct Value {
    using V = std::variant<std::monostate, bool, int64_t, double, std::string, Array, Object>;
    V v;

    static Value Nil() {
        Value out;
        out.v = std::monostate{};
        return out;
    }

    static Value Int(int64_t x) {
        Value out;
        out.v = x;
        return out;
    }

    static Value Obj(Object o) {
        Value out;
        out.v = std::move(o);
        return out;
    }

    static Value Arr(Array a) {
        Value out;
        out.v = std::move(a);
        return out;
    }
};

} // namespace urpg
```

```cpp
// tests/unit/test_bridge_roundtrip.cpp
#include "engine/runtimes/bridge/value.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Bridge Value round-trip is stable", "[bridge]") {
    using urpg::Value;
    urpg::Object nested;
    nested["y"] = Value::Int(2);
    urpg::Object root;
    root["x"] = Value::Int(1);
    root["nested"] = Value::Obj(nested);
    root["arr"] = Value::Arr({ Value::Int(10), Value::Int(20), Value::Int(30) });
    Value v = Value::Obj(root);
    auto* obj = std::get_if<urpg::Object>(&v.v);
    REQUIRE(obj != nullptr);
    REQUIRE(obj->at("x").v.index() == 2); // int64_t
}
```

### 18.9 Migration Fuzz Harness Contract

Migration runners must never crash on malformed input. Fuzz tests mutate valid JSON and assert: (a) no crash, (b) output is valid JSON or explicit MigrationError.

```cpp
// tools/migrate/fuzz_migrate.cpp
#include <cstdint>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Seed corpus: valid URPG project JSON and save envelopes.
    // Mutate via libFuzzer; feed to migration runner.
    // Assert: never throws uncaught; returns Ok(json) or MigrationError.
    return 0;
}
```

Appendix A — Test Coverage Matrix (v3.1)

Minimum required tests per component. Each row must map to concrete files in the repo layout.

| Component          | Unit                                                     | Integration                                       | Snapshot/Golden                 |
|--------------------|----------------------------------------------------------|---------------------------------------------------|---------------------------------|
| Format + Migration | SemVer, canonical JSON, schema validate                  | migrate reference projects                        | golden hash of migrated outputs |
| Save System        | encode/decode envelope, integrity checks                 | load/save in headless session                     | golden save round-trip          |
| Event VM           | opcode/macro evaluation                                  | scripted quest scenarios                          | N/A                             |
| Renderer           | math + batching rules                                    | headless scene renders                            | image diffs per tier            |
| Compat JS          | URPGCompat polyfills                                     | run plugin suite                                  | N/A                             |
| Lua Plugins        | permission gate checks                                   | load plugin project                               | N/A                             |
| Asset Pipeline     | hashing + dedupe                                         | import+pack pipeline                              | atlas golden output             |
| Math (Fixed32)     | Fixed32 ops, overflow policy, formatting                 | combat calc uses Fixed32 in headless scenarios    | golden combat outputs           |
| ECS Core           | entity lifecycle, deterministic iteration                | headless scenario: spawn/despawn stress           | N/A                             |
| Perf Budgets       | budget math + enforcement decisions                      | headless: plugin budget exceed triggers SOFT_FAIL | golden perf budget report       |
| Plugin Manifests   | schema validate, lifecycle hook presence, conflict graph | install/enable/disable in test project            | N/A                             |
| Input Mapping      | action map parse + resolve                               | headless input script drives actions              | N/A                             |
| Audio Middleware   | ducking + crossfade rules                                | headless audio graph sim asserts bus levels       | N/A                             |
| Map Streaming      | chunk index math + cache eviction                        | headless large-map walk asserts load/unload       | golden chunk trace              |
| Security / Audit   | permission checks + audit event schema                   | plugin permission use recorded in audit log       | golden audit log output         |
| Copilot Validator  | DB reference resolution + blocker reporting              | copilot session generates diff with undo          | N/A                             |
