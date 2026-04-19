# URPG Development Status (v3.1)

Snapshot Date: 2026-04-15

This repository is bootstrapped to the blueprint's canonical structure and now includes core + compat kernels with active CI gate wiring.

This document is a kickoff snapshot, not the canonical current-status file.
For current completion, remediation, and validation status, use:
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`

Cross-cutting debt closure, documentation-truth alignment, and intake-governance tracking: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

Historical note: Phase 2 runtime closure completed later on 2026-04-19. Any remaining compat work referenced here should be read as post-closure hardening unless the canonical status docs say otherwise.

## Progress tracker

Historical kickoff snapshot values below are preserved for context. For current counts and active validation commands, use `docs/PROGRAM_COMPLETION_STATUS.md`.

- Phase 0 Foundation: 100% complete.
- Phase 1 Native Core: 100% complete for v3.1 kickoff/continuation/integration contracts.
- Phase 2 Compat Layer: runtime closure completed on 2026-04-19; remaining work is post-closure compat exit hardening (conformance depth, diagnostics exit criteria, and status-discipline maintenance).
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
  - Event-id, severity, and mode filtering plus selected-row one-click navigation target resolution.
  - Visible-row body projection in event-authority panel snapshots and diagnostics workspace export.
- Phase 1 Native Core kickoff implemented:
   - Event runtime kernels for deterministic priority ordering and cancellation/default behavior controls.
   - Debugger contract kernels for breakpoint storage and watch variable tables.
- Phase 1 Native Core continuation implemented:
   - Event execution timeline + reentrancy depth tracking contract kernel (`EventExecutionTimeline`).
   - Debugger call-stack frame + step-control contract kernels (`CallStack`, `StepController`).
- Phase 1 Native Core integration follow-up implemented:
   - Runtime dispatch-session wiring over event timeline/reentrancy contracts (`EventDispatchSession`).
   - Runtime debug-session wiring over breakpoints/watches/call-stack/step controls (`DebugRuntimeSession`).
- Test baseline added and passing (Debug snapshot, 2026-04-15):
  - historical kickoff snapshot: `urpg_tests` => 3907 assertions / 287 test cases
  - Latest local gate snapshot:
    - recorded under the `dev-ninja-debug` preset: `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure`: 289/289 passed
    - recorded under the `dev-ninja-debug` preset: `ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure`: 42/42 passed
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
   - AudioManager compat closure advanced without overstating parity: deterministic frame-based crossfade, playback-position progression, BGM save/restore metadata, duck/unduck ramps, active master/bus volume scaling, and live QuickJS `AudioManager` bindings are now covered, but the surface remains honestly `PARTIAL` because it is still a deterministic compat harness rather than a live mixer/backend.
   - Burned down BattleManager compat status: `processEscape` from `PARTIAL` to `FULL` with deterministic MZ-style escape ratio + fail-ramp handling.
   - `executeCommandAsync` now has deterministic FIFO task queue execution and callback ordering, but the surface still remains `PARTIAL` because callbacks run on the worker thread and the JS bridge is fixture-backed.
   - Input/Touch QuickJS API registration now returns live runtime state and `TouchInput` movement telemetry now tracks `moveSpeed` and `tapCount`.
- Migration and schema anchors added:
  - `tools/migrate/migration_op.json`
  - `tools/migrate/fuzz_migrate.cpp`
  - `content/schemas/project.schema.json`

## Immediate next implementation lanes

1. Compat exit completion
   - Finish remaining routed conformance/failure depth and lock every new failure mode to JSONL/report/panel assertions until explicit exit criteria are satisfied.
2. Message/Text renderer closure
   - Complete backend text-command consumption and native MessageScene handoff from compat `Window_Message` bridge behavior.
3. Wave 1 native runtime ownership
   - Complete native subsystem ownership for UI/Menu, Message/Text, Battle, and Save/Data beyond currently landed slices.
4. Wave 1 editor + schema productization
   - Ship subsystem inspectors/previews/diagnostics and finalize migration-safe schemas/import upgrade paths.
5. Wave 2 advanced capability delivery
   - Start implementation tracks for ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, and editor utilities.
6. Completion tracking
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
