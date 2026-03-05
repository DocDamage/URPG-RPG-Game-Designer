# URPG

Bootstrap workspace aligned to URPG Master Blueprint v3.1.

## Current scope

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
  - PluginManager now supports JSON fixture loading, directory discovery, JSON parameter parsing, and script-driven fixture command execution.
  - Fixture script commands now route through per-plugin `QuickJSRuntime` contexts (`QuickJSContext::call`) for real compat-lane execution plumbing.
  - Fixture JSON commands can now provide lightweight JS source (`js`) + entrypoint (`entry`) and execute via `QuickJSContext::eval` + `call`.
  - Curated fixture profiles now actively exercise JS directive `arg` and `const` modes across all 10 real-world plugin fixtures.
  - Fixture script DSL now supports conditional flow and richer value resolvers (`if`, `args`, `paramKeys`, `hasParam`, `equals`, `coalesce`).
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
  - QuickJS stub lane now supports explicit eval failure directives (`@urpg-fail-eval`) and evalModule failure propagation tests for deterministic conformance.
  - Compat report model now ingests PluginManager diagnostics JSONL (`ingestPluginFailureDiagnosticsJsonl`) into timeline events and per-plugin error summaries.
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
  - `urpg_tests`: 1183 assertions / 231 test cases
  - `urpg_integration_tests`: 10 assertions / 2 test cases
  - `urpg_snapshot_tests`: 4 assertions / 2 test cases
  - `urpg_compat_tests`: 534 assertions / 10 test cases
  - Total: 1731 assertions / 245 test cases

## Build

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Migration CLI

```powershell
.\build\urpg_migrate.exe --input <project.json> --migration tools\migrate\migration_op.json --output <out.json>
```

## Next lane

Implement the next blueprint-critical contracts:

1. Complete Phase 2 Compat Layer validation:
  - Wire runtime/session-level compat report refresh to consume exported PluginManager diagnostics artifacts in editor update flow.
2. Expand plugin conformance depth:
  - Expand fixture script DSL coverage and report artifact depth over real JS plugin execution paths.
