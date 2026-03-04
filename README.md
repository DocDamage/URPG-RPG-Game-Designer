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
  - Canonical JSON serializer + deterministic migration runner (`rename`, `set`)
- Migration CLI: `urpg_migrate`
- Catch2 unit test baseline (17 passing tests).

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

1. Source-of-truth runtime scaffolding (Compat raw list vs Native AST authority tagging).
2. Save corruption recovery tiers (L1/L2/L3 entry points).
3. CI gate skeleton (PR/nightly/weekly stratification + waiver workflow).
