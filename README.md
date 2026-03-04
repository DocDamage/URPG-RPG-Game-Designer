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
  - Canonical JSON serializer + deterministic migration runner (`rename`, `set`)
  - Source-of-truth authority policy (Compat/Native/Mixed)
  - Event edit guard with structured `event_authority` diagnostics
- CI gate skeleton:
  - Gate 1 (PR): `ctest -L pr`
  - Gate 2 (nightly): `ctest -L nightly` (placeholder suite)
  - Gate 3 (weekly): `ctest -L weekly` (placeholder suite)
  - Known-break waiver validation via `tools/ci/check_waivers.ps1`
- Migration CLI: `urpg_migrate`
- Catch2 unit test baseline (29 passing tests).

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

1. Recovery integration hook-up into runtime save load flow and crash-safe mode startup.
2. CI suite expansion (replace nightly/weekly placeholders with real integration/snapshot/compat suites).
3. Editor diagnostics UI hook-up for `event_authority` rejection events.
