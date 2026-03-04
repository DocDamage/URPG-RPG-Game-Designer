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
- CI gate suites:
  - Gate 1 (PR): `ctest -L pr`
  - Gate 2 (nightly): `ctest -L nightly` (integration + snapshot suites)
  - Gate 3 (weekly): `ctest -L weekly` (compat suite)
  - Nightly renderer-tier matrix (`basic`, `standard`, `advanced`) + test log artifacts in CI
  - Known-break waiver validation via `tools/ci/check_waivers.ps1`
- Migration CLI: `urpg_migrate`
- Catch2/CTest baseline (51 passing tests).

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

1. Phase 2 Compat Layer kickoff:
  - Add QuickJS runtime integration contract kernel scaffolding.
  - Start WindowCompat core surface stubs + status tagging primitives.
