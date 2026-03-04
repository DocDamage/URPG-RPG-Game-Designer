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
   - Canonical JSON serializer contract and deterministic migration runner (`rename` + `set`) with CLI.
   - Source-of-truth authority policy (`Compat` raw authoritative, `Native` AST authoritative, `Mixed` per-block authority tags) with guardrail validation.
   - Event integration hook-up for authority validation and structured rejection diagnostics (`event_authority` JSONL).
- Unit test baseline added and passing.
- Migration and schema anchors added:
  - `tools/migrate/migration_op.json`
  - `tools/migrate/fuzz_migrate.cpp`
  - `content/schemas/project.schema.json`

## Immediate next implementation lane

1. Recovery integration hook-up
   - Wire SaveRecoveryManager into runtime save load flow and crash-safe mode startup path.

2. CI suite expansion
   - Replace nightly/weekly placeholders with real integration/snapshot/compat suites.
   - Add test artifact upload and renderer-tier matrix for nightly gates.

3. Editor diagnostics UI hook-up
   - Surface `event_authority` diagnostics in editor panels with one-click navigation to offending event blocks.

## Build/test

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
