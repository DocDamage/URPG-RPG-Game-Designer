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
   - Canonical JSON serializer contract and deterministic migration runner (`rename` + `set`) with CLI.
   - Source-of-truth authority policy (`Compat` raw authoritative, `Native` AST authoritative, `Mixed` per-block authority tags) with guardrail validation.
- Unit test baseline added and passing.
- Migration and schema anchors added:
  - `tools/migrate/migration_op.json`
  - `tools/migrate/fuzz_migrate.cpp`
  - `content/schemas/project.schema.json`

## Immediate next implementation lane

1. Save recovery tiers
   - Add level 1/2/3 recovery entry points (autosave restore, metadata-only, skeleton load).

2. CI gate skeleton
   - Add CTest labels/groups for PR vs nightly vs weekly suites.
   - Add baseline workflow config + known-break waiver mechanism.

3. Event integration hook-up
   - Thread authority validation into event editor mutation paths.
   - Emit structured diagnostics when edits target derived views.

## Build/test

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
