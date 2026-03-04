# URPG Development Kickoff (v3.1)

This repository is now bootstrapped to the blueprint's canonical structure and includes contract kernels + baseline tests.

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
- Test baseline added and passing (47 tests).
- Migration and schema anchors added:
  - `tools/migrate/migration_op.json`
  - `tools/migrate/fuzz_migrate.cpp`
  - `content/schemas/project.schema.json`

## Immediate next implementation lanes

1. Phase 1 Native Core continuation
   - Event runtime execution timeline + reentrancy depth contract kernels.
   - Debugger call-stack frame + step-control contract kernels.

## Build/test

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
