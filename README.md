# URPG

Bootstrap workspace aligned to URPG Master Blueprint v3.1.

## Current scope

- Documentation sync:
  - `URPG_Blueprint_v3_1_Integrated.md` now maintains a top-of-file live progress tracker (phase status, completion, baseline counts, and weekly focus).
- Canonical repo layout skeleton (engine/editor/runtimes/content/tools/tests/docs).
- Contract kernels from section 18:
  - SemVer
  - Fixed32 (Q16.16)
  - FrameBudget types
  - ECS World deterministic iteration behavior
  - CombatCalc baseline
  - Bridge Value model
- Phase 0 foundations:
  - Thread role + script access model (Render/Logic/Audio/AssetStreaming)
  - Renderer capability tiers + feature-gate helpers
  - Save metadata envelopes + journaled atomic write path
  - Save corruption recovery tiers (Level 1/2/3)
  - Runtime save load integration with recovery fallback + force-safe-mode startup (`RuntimeSaveLoader`)
  - Canonical JSON serializer + deterministic migration runner (`rename`, `set`)
  - Source-of-truth authority policy (Compat/Native/Mixed)
  - Event edit guard with structured `event_authority` diagnostics (with `block_id` emission)
  - Editor diagnostics index for `event_authority` JSONL parsing + event/block navigation targets
  - Editor diagnostics panel/view model for rows, filtering, and one-click navigation target selection
- Phase 1 kickoff kernels:
  - Event runtime priority ordering + cancellation + prevent-default contracts
  - Debugger breakpoint store + watch table contract kernels
- Phase 1 continuation kernels:
  - Event execution timeline + reentrancy depth tracking contracts (`EventExecutionTimeline`)
  - Debugger call-stack frame + step-control contracts (`CallStack`, `StepController`)
- Phase 1 integration follow-up:
  - Event runtime dispatch-session facade now wires timeline/reentrancy control (`EventDispatchSession`)
  - Debug runtime session facade now wires breakpoints/watches/call-stack/step flow (`DebugRuntimeSession`)
- Phase 2 compat progression:
  - QuickJS runtime kernel, WindowCompat surfaces, and Battle/Data/Audio/Input/Plugin compat modules are wired in active build targets.
  - WindowCompat now includes expanded API registration coverage + method call-count telemetry for compat reports.
  - Added curated compat profile conformance suite for 10 popular MZ plugin profiles.
  - Added executable plugin fixture suite for 10 real-world MZ plugin profiles in weekly compat CI coverage.
  - Curated failure-path diagnostics suite now gates missing-command/full-name parse errors, malformed fixture command payloads, fixture script runtime op failures, fixture script validation-shape failures (`set`/`append` key requirements plus malformed `invoke`/`invokeByName` target/store/expect shapes), malformed nested-branch fixture failures (`if` branch shape and nested branch-step validation), directory-scan failures (including deterministic iterator and entry-status branches), and dependency-failure behavior across the 10-profile corpus.
  - Added a combined weekly regression that runs dependency-gating conformance across all 10 curated fixtures plus mixed malformed payload/eval/runtime/full-name-parse failure chains (including fixture-open/fixture-name/duplicate-load failures, `load_plugin_name` + `load_plugin_register_command` + `load_plugin_register_script_fn` + `load_plugin_quickjs_context` failures, parameter-parse failures, deterministic directory-scan iterator/entry-status failures (`load_plugins_directory_scan` / `load_plugins_directory_scan_entry`), malformed command metadata failures (`dropContextBeforeCall`/`entry`/`description` type validation), malformed fixture metadata shape failures (`dependencies`/`parameters`/`commands` type validation plus dependency-entry string enforcement), nested `all`/`any` runtime branch failures, `invoke`/`invokeByName` command-chain runtime failures, deterministic post-load context-drop coverage for `execute_command_quickjs_context_missing`, strict script-shape runtime failures, and malformed command-shape/name load failures) in one end-to-end diagnostics pass, including compat report model/panel ingestion and export projection checks.
  - PluginManager now supports JSON fixture loading, directory discovery, JSON parameter parsing, and script-driven fixture command execution.
  - PluginManager directory/plugin/command/dependent enumeration paths now enforce deterministic lexical ordering (`loadPluginsFromDirectory`, `getLoadedPlugins`, `getPluginCommands`, `getDependents`).
  - Fixture script commands now route through per-plugin `QuickJSRuntime` contexts (`QuickJSContext::call`) for real compat-lane execution plumbing.
  - Fixture JSON commands can now provide lightweight JS source (`js`) + entrypoint (`entry`) and execute via `QuickJSContext::eval` + `call`.
  - Fixture commands now support a deterministic `dropContextBeforeCall` hook for conformance coverage of `execute_command_quickjs_context_missing`.
  - Curated fixture profiles now actively exercise JS directive `arg` and `const` modes across all 10 real-world plugin fixtures.
  - Fixture script DSL now supports conditional flow and richer value resolvers (`if`, `args`, `paramKeys`, `hasParam`, `hasArg`, `equals`, `coalesce`, `length`, `contains`, `greaterThan`, `lessThan`, `not`, `all`, `any`) with executable coverage expanded for `append`/`local`/`concat` chain behavior.
  - Fixture script DSL now supports deterministic command-chain dispatch via `invoke` and `invokeByName`, including `store` capture and `expect: non_nil` assertions for nested execution validation.
  - Compat conformance now includes invoke-chain success and failure-path diagnostics coverage (`execute_command`, `execute_command_by_name_parse`, and `execute_command_quickjs_call`) for nested fixture command flows, including malformed `invoke`/`invokeByName` validation branches.
  - Added deterministic cross-plugin invoke chain fuzz matrix coverage (32 generated chain cases across curated fixtures, mixed `invoke` + `invokeByName`, nested branch routing).
  - Unknown (non-`std::exception`) command/runtime throws are now classified as `CRASH_PREVENTED` in diagnostics to preserve deterministic crash-containment semantics.
  - Plugin reload now reuses tracked source paths and rehydrates fixture commands for JSON-backed compat plugins.
  - DataManager compat save lane now persists per-slot `GlobalState` + `SaveHeader` in-memory (including autosave slot `0`) with slot bound checks, plus `setSaveHeaderExtension`/`getSaveHeaderExtension` round-trip semantics and cleanup on slot deletion.
  - DataManager map transfer flow now tracks reserved transfers and applies them deterministically through `processTransfer`.
  - Burned down selected compat statuses: `Window_Base.drawItemName/textWidth/textSize` advanced from `STUB` to `PARTIAL`; `TouchInput.worldX/worldY` advanced from `STUB` to `FULL`; `Window_Base.drawActorHp/drawActorMp/drawActorTp` advanced from `PARTIAL` to `FULL`.
  - Burned down WindowCompat text-surface statuses: `Window_Base.drawTextEx`, `Window_Base.textWidth`, `Window_Base.textSize` advanced from `PARTIAL` to `FULL` with escape-token parity and compat renderer-backed text measurement/layout.
  - Burned down additional WindowCompat statuses: `Window_Base.drawItemName` advanced from `PARTIAL` to `FULL` with DataManager-backed icon+label semantics, and `Sprite_Actor.startEffect` advanced from `PARTIAL` to `FULL` with deterministic effect-duration lifecycle behavior.
  - Burned down additional Sprite_Actor animation status: `startAnimation` advanced from `PARTIAL` to `FULL` with deterministic frame-duration playback lifecycle.
  - Burned down remaining Window_Base face status: `drawActorFace` advanced from `PARTIAL` to `FULL` with MZ-canonical face cell clipping/centering semantics and deterministic face draw metadata.
  - Compat status burn-down checkpoint: active runtime compat API registry now reports no `PARTIAL`/`STUB` surfaces; remaining Phase 2 work is deep executable validation + diagnostics hardening.
  - Burned down AudioManager compat statuses: `crossfadeBgm`/`crossfadeBgs` advanced from `PARTIAL` to `FULL` with deterministic frame-based crossfade sequencing; BGM save metadata now preserves track filename/position for correct restore semantics.
  - Burned down BattleManager compat status: `processEscape` advanced from `PARTIAL` to `FULL` with deterministic MZ-style escape ratio/failure ramp semantics.
  - Burned down PluginManager compat status: `executeCommandAsync` advanced from `PARTIAL` to `FULL` with deterministic FIFO task-queue execution + callback ordering.
  - Input/Touch QuickJS API registration now routes to live runtime state (no placeholder zeros) and `TouchInput` movement/tap tracking now computes `moveSpeed` + `tapCount`.
  - PluginManager failure-path diagnostics now emit deterministic JSONL artifacts (`exportFailureDiagnosticsJsonl` / `clearFailureDiagnostics`) with operation tags + sequence IDs.
  - PluginManager diagnostics JSONL now include compat severity tags (`WARN`, `SOFT_FAIL`, `HARD_FAIL`, `CRASH_PREVENTED`) for downstream report classification.
  - PluginManager failure diagnostics export now enforces bounded retention (last 2048 events) while preserving monotonic sequence IDs across trims; coverage is gated in unit tests.
  - `executeCommandByName` now routes through exact registered full keys (supports underscore-heavy command names) and rejects missing plugin/command segments via deterministic `execute_command_by_name_parse` diagnostics.
  - QuickJS stub lane now supports explicit eval failure directives (`@urpg-fail-eval`), explicit runtime call-failure directives (`@urpg-fail-call`), deterministic context-init failure marker support (`__urpg_fail_context_init__`), and evalModule failure propagation tests for deterministic conformance.
  - Fixture command validation now fails malformed payloads deterministically (`js` must be string, `script` must be array, `dropContextBeforeCall` must be boolean, optional `entry`/`description`/`mode` metadata must be strings, `mode` values are restricted to `const` or `arg_count`, and commands cannot declare both `js` and `script`) with exported diagnostics operation tags.
  - Fixture metadata shape validation now fails malformed `dependencies`/`parameters`/`commands` containers and non-string dependency entries deterministically with exported diagnostics operation tags.
  - Fixture script runtime now supports explicit `error` op and unknown-op hard-fail behavior, surfaced through deterministic `execute_command_quickjs_call` diagnostics artifacts.
  - Dependent plugin command execution is now gated when required dependencies are missing, surfaced through deterministic `execute_command_dependency_missing` diagnostics artifacts.
  - Compat report model now ingests PluginManager diagnostics JSONL (`ingestPluginFailureDiagnosticsJsonl`) into timeline events and per-plugin error summaries.
  - Compat report ingestion now maps PluginManager compat severity tags into timeline severity levels (`WARNING`/`ERROR`/`CRITICAL`).
  - Compat report panel runtime refresh now consumes and clears PluginManager diagnostics artifacts each update cycle (`CompatReportPanel::refresh`/`update`).
  - Compat report diagnostics model hardening now updates warning/error flags when method statuses transition and sorts call-volume views by total calls (including unsupported operations).
  - Compat report panel now records bounded per-plugin session score history plus first-seen/last-updated timestamps, and `LAST_UPDATED` sorting now projects human-readable recency labels.
  - Compat module unit suites (`test_battlemgr`, `test_data_manager`, `test_audio_manager`, `test_input_manager`, `test_plugin_manager`) are active in `urpg_tests`.
- Active build wiring snapshot:
  - `urpg_core` currently builds core kernels + editor diagnostics/panel + compat report panel + QuickJS + WindowCompat + Battle/Data/Audio/Input/Plugin compat modules.
  - `urpg_tests` includes the full compat unit slice in active CMake targets.
- CI gate suites:
  - Gate 1 (PR): `ctest -L pr`
  - Gate 2 (nightly): `ctest -L nightly` (integration + snapshot suites)
  - Gate 3 (weekly): `ctest -L weekly` (compat suite)
  - Nightly renderer-tier matrix (`basic`, `standard`, `advanced`) + test log artifacts in CI
  - Known-break waiver validation via `tools/ci/check_waivers.ps1`
- Migration CLI: `urpg_migrate`
- Catch2/CTest baseline (Release snapshot, 2026-03-05):
  - `urpg_tests`: 3447 assertions / 237 test cases
  - `urpg_integration_tests`: 10 assertions / 2 test cases
  - `urpg_snapshot_tests`: 4 assertions / 2 test cases
  - `urpg_compat_tests`: 1985 assertions / 24 test cases
  - Total: 5446 assertions / 265 test cases

## Build

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest --preset dev-all
```

## Build cache (sccache)

```powershell
$env:SCCACHE_DIR = "$PWD/.cache/sccache"
sccache --zero-stats
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
sccache --show-stats
```

`URPG_USE_SCCACHE` is enabled by default in `CMakePresets.json`.

## Git LFS

```powershell
git lfs install
git lfs track
```

## Local gates

```powershell
.\tools\ci\run_local_gates.ps1
```

## CodeMunch Pro Workflow

```powershell
.\tools\codemunch\bootstrap-project.ps1
.\tools\codemunch\index-project.ps1 -ProjectRoot . -Embed -OutFile ".codemunch\last-index.json"
```

See `tools/codemunch/README.md` for full usage and cross-project setup.

## Contributor guide

See `CONTRIBUTING.md` for workflow, LFS policy, and asset hygiene checks.

```powershell
python .\tools\assets\asset_hygiene.py --write-reports --prune-junk
.\tools\rpgmaker\validate-plugin-dropins.ps1
```

## Pre-commit

```powershell
python -m pip install pre-commit
pre-commit install
pre-commit run --all-files
```

## Migration CLI

```powershell
.\build\urpg_migrate.exe --input <project.json> --migration tools\migrate\migration_op.json --output <out.json>
```

## Next lane

Implement the next blueprint-critical contracts:

1. Complete Phase 2 Compat Layer validation:
  - Expand executable conformance/failure-path depth across curated 10-plugin fixtures (additional malformed/eval/runtime command chains).
2. Expand plugin conformance depth:
  - Expand fixture script DSL coverage and report artifact depth over real JS plugin execution paths + compat report exports.
