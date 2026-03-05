# URPG Development Status (v3.1)

Snapshot Date: 2026-03-05

This repository is bootstrapped to the blueprint's canonical structure and now includes core + compat kernels with active CI gate wiring.

## Progress tracker

- Phase 0 Foundation: complete.
- Phase 1 Native Core: complete for v3.1 kickoff/continuation/integration contracts.
- Phase 2 Compat Layer: in progress (active burn-down on remaining `PARTIAL`/`STUB` surface gaps).

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
  - `urpg_tests`: 847 assertions / 207 test cases
  - `urpg_integration_tests`: 10 assertions / 2 test cases
  - `urpg_snapshot_tests`: 4 assertions / 2 test cases
  - `urpg_compat_tests`: 125 assertions / 3 test cases
- Phase 2 compat expansion implemented in active targets:
   - WindowCompat surface expansion for `Window_Base`, `Window_Selectable`, and `Window_Command` API registration.
   - WindowCompat method call-count tracking surfaced in compat reports.
   - Curated plugin profile conformance suite (`test_compat_window_plugin_profiles.cpp`) for 10 real-world MZ plugin profiles.
   - Executable plugin fixture suite (`test_compat_plugin_fixtures.cpp`) for the same 10 plugin profiles in the weekly lane.
- Phase 2 compat module wiring completed in active targets:
   - `battle_manager`, `data_manager`, `audio_manager`, `input_manager`, `plugin_manager` are now part of active CMake builds.
   - Corresponding unit tests are active in `urpg_tests`: `test_battlemgr`, `test_data_manager`, `test_audio_manager`, `test_input_manager`, `test_plugin_manager`.
   - `PluginManager` fixture loading now supports JSON plugin fixtures, directory scanning, JSON parameter parsing, and script-driven fixture command execution.
   - Fixture script commands now execute through per-plugin `QuickJSRuntime` contexts (`QuickJSContext::call`) rather than direct in-manager dispatch.
   - Fixture JSON commands now support lightweight JS source + entrypoint execution via `QuickJSContext::eval` + `call`.
   - Curated fixture profiles now validate JS directive `arg` and `const` modes across all 10 real-world plugin fixtures.
   - Fixture script DSL expanded with control flow + richer value sources (`if`, `args`, `paramKeys`, `hasParam`, `equals`, `coalesce`).
   - Plugin reload now rehydrates from tracked source paths and restores fixture commands for JSON-backed plugin profiles.
   - DataManager save/load compat lane now persists per-slot `GlobalState` + `SaveHeader` in-memory (including autosave slot `0`) with slot bounds validation.
   - DataManager header-extension compat APIs now round-trip per-slot plugin metadata and clear extension state on save-slot deletion.
   - DataManager transfer semantics now track reserved transfers and apply them via `processTransfer`.
   - Burned down selected compat statuses: `Window_Base.drawItemName`, `Window_Base.textWidth`, `Window_Base.textSize` from `STUB` to `PARTIAL`; `TouchInput.worldX/worldY` from `STUB` to `FULL`; `Window_Base.drawActorHp/drawActorMp/drawActorTp` from `PARTIAL` to `FULL`.
   - Input/Touch QuickJS API registration now returns live runtime state and `TouchInput` movement telemetry now tracks `moveSpeed` and `tapCount`.
- Migration and schema anchors added:
  - `tools/migrate/migration_op.json`
  - `tools/migrate/fuzz_migrate.cpp`
  - `content/schemas/project.schema.json`

## Immediate next implementation lanes

1. Complete Phase 2 validation depth
   - Validate wired compat modules/tests against 10 real-world MZ plugins and close remaining `PARTIAL`/`STUB` deviations.
2. Deepen compat validation
   - Expand fixture script DSL coverage and bridge script-backed fixtures into real JS plugin execution paths.

## Build/test

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
