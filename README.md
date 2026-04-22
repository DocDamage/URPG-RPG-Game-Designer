# URPG (Universal RPG Engine)

![URPG Header](https://raw.githubusercontent.com/URPG-Project/assets/main/header.png)

URPG is a native-first C++20 RPG engine and editor focused on deterministic runtime ownership, migration-safe data contracts, and truthful compatibility with RPG Maker MZ content.

It is not a thin skin over a plugin host. The long-term product direction is:

- native runtime ownership for game-critical systems
- editor and diagnostics surfaces for shipped systems
- schema and migration contracts for authored and imported data
- a bounded QuickJS compatibility harness for import, verification, and migration confidence
- evidence-gated documentation, readiness, and release governance

## Why URPG

URPG is for teams that want stronger runtime control, stronger tooling, and less hidden engine behavior than typical plugin-heavy RPG stacks.

Compared with similar products, URPG is differentiated by:

- Native ownership instead of plugin accretion: core systems are implemented in C++ and tested as first-class engine features rather than delegated to third-party plugin stacks.
- Deterministic architecture: ECS-style world iteration, explicit scene/runtime ownership, deterministic input/state flows, and stable validation lanes make behavior easier to test and reason about.
- Migration and compatibility discipline: the RPG Maker MZ lane is treated as an import and verification bridge, not marketing theater. Compatibility claims stay intentionally conservative.
- Diagnostics as product surface: ProjectAudit, diagnostics panels, validation scripts, and readiness records are part of the shipped repo, not ad hoc one-off scripts.
- Governance that matches the code: release-readiness, schema changelog coverage, truth reconciliation, template readiness, and signoff workflow checks are machine-validated.
- Multi-genre roadmap with landed baseline systems: beyond JRPG basics, the repo already includes ability systems, pattern authoring, modular level assembly, procedural toolkit foundations, 2.5D/raycast groundwork, animation timeline pieces, accessibility, analytics, export validation, mod registry support, and more.

If you want a pure drag-and-drop, low-code editor with a mature asset marketplace, URPG is not trying to be that. If you want a code-owning engine/editor stack that can grow into a serious native RPG toolchain while still helping ingest RPG Maker content, that is exactly the niche URPG is targeting.

## Current State

Status date: 2026-04-21

What is true in the current tree:

- Wave 1 native ownership for UI/Menu, Message/Text, Battle, and Save/Data is landed.
- Phase 2 compat runtime closure is complete; the remaining compat work is post-closure hardening, confidence depth, and truthful residual scope.
- ProjectAudit, release-readiness, template-readiness, schema-changelog, and truth-reconciliation governance are active and enforced.
- Advanced capability lanes are meaningfully implemented, but many remain honestly `PARTIAL` at the product/readiness level.

What is not true:

- The repo is not claiming full release readiness across every subsystem or template.
- The QuickJS layer is not presented as a finished live scripting runtime for all use cases.
- Several advanced/editor surfaces are compiled and tested but still have honest residual gaps in workflow depth or workspace integration.

For canonical current status, use:

- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Technical Debt Remediation Plan](./docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- [Release Readiness Matrix](./docs/RELEASE_READINESS_MATRIX.md)
- [Project Audit](./docs/PROJECT_AUDIT.md)

## Architecture

Core stack:

- Language: C++20
- Build system: CMake 3.23+
- Platforms: Windows first, with GCC/Clang support paths
- Rendering/windowing: SDL2 + OpenGL, with headless CI paths
- JSON: nlohmann/json
- Testing: Catch2 v3
- Compatibility runtime: QuickJS under `runtimes/compat_js/`

Repository structure:

- `engine/`: native runtime systems
- `editor/`: editor panels, models, and diagnostics surfaces
- `runtimes/compat_js/`: bounded RPG Maker MZ compatibility bridge
- `tests/`: unit, integration, snapshot, compat, and low-level engine coverage
- `tools/`: CI gates, migration tools, audit tooling, asset hygiene, workflow/bootstrap tooling
- `content/`: schemas, readiness records, fixtures
- `docs/`: canonical roadmap, status, readiness, signoff, ADRs, and validation guides

## Shipped Engine Capabilities

### Wave 1 native runtime ownership

- UI/Menu Core:
  - native command registry, route resolution, scene graph orchestration
  - menu inspector and preview surfaces
  - schema and migration support
- Message/Text Core:
  - native message flow runner, text layout, prompt state
  - renderer-facing text and rect command emission
  - inspector workflows, schema, and migration support
- Battle Core:
  - native battle flow, diagnostics, preview integration, migration coverage
  - presentation bridge/runtime handoff for battle-facing frame intent
  - signoff artifact and readiness governance
- Save/Data Core:
  - save runtime, migration, recovery, policy governance, metadata, slot descriptors
  - RPG Maker MV/MZ binary save ingestion path
  - save inspector/editor workflows and diagnostics

### Advanced capability systems

- Gameplay Ability Framework:
  - gameplay tags, abilities, effects, state machines, ability tasks
  - ability inspector and end-to-end validation coverage
- Pattern Field Editor:
  - painted pattern resources, presets, validation, preview flows
- Modular Level Assembly:
  - connector-based block workflows, deterministic placement validation, thumbnails/import helpers
- Sprite Pipeline Toolkit:
  - atlas packing, metadata generation, preview/tuning flows
- Procedural Content Toolkit:
  - dungeon/layout primitives, deterministic scenario generation, FOV foundations
- 2.5D presentation lane:
  - raycast renderer groundwork with explicit project-mode gating
- Timeline and animation orchestration:
  - animation clips, timeline kernel, event-triggered transient effect support
- Editor productivity utilities:
  - selected helper/task infrastructure for authoring support

### Product and governance subsystems landed as bounded lanes

- Character Identity:
  - identity runtime, editor creator panel, ECS/system sync, deterministic spawner
  - bounded validation runtime and governance script
- Achievement Registry:
  - definitions, progress tracking, trigger parsing, event bus integration, editor panel
  - bounded validation runtime and governance script
- Accessibility Auditor:
  - missing-label, focus-order, contrast, and navigation audits
  - live menu-ingest adapter and governance checks
- Audio Mix Presets:
  - preset bank, category volumes, ducking rules, editor panel
  - validator and governance coverage
- Export Validator:
  - target-specific export validation, synthetic pack-and-validate path, diagnostics panel
- Mod Registry:
  - manifest registration, dependency resolution, load-order topology, editor diagnostics
  - validator and governance coverage
- Analytics Dispatcher:
  - opt-in event buffer, deterministic counters, bounded validator, editor diagnostics
  - governance and ProjectAudit coverage
- Input controller governance:
  - `InputRemapStore`
  - bounded `ControllerBindingRuntime`
  - bounded `ControllerBindingPanel`
  - governance script and audit coverage

### Compatibility and migration lane

- QuickJS compatibility harness for RPG Maker MZ-facing verification
- `Window_Base`, `Window_Selectable`, `Window_Command`, `Window_Message`, `BattleManager`, `DataManager`, `AudioManager`, `InputManager`, and `PluginManager` compat surfaces
- curated compat fixture suites and failure-path diagnostics
- JSONL diagnostics export and report/panel ingestion
- plugin failure containment and deterministic reporting
- migration-safe handling of unsupported or lossy paths

This lane is intentionally described as a compat bridge and verification harness, not as blanket live parity with all RPG Maker runtime behavior.

## Editor And Tooling

The repo ships substantial editor and tooling coverage already:

- diagnostics workspace and project audit panel
- battle, save, message, ability, audio, analytics, accessibility, achievement, character, export, mod, and spatial panels/models
- migration CLI: `urpg_migrate`
- release-readiness and truth-reconciliation gates
- schema changelog governance
- waiver checking
- plugin drop-in validation
- asset hygiene and intake governance
- LLM workflow/bootstrap tooling (`codemunch`, `contextlattice`, `memorybridge`, workflow installers)

## Why Teams Would Choose URPG Over Similar Tools

Choose URPG if your priorities look like this:

- You need native engine ownership and testability, not just event/plugin scripting.
- You want to import or reason about RPG Maker content without making plugin compatibility your permanent architecture.
- You care about evidence-backed status, release gating, and migration contracts.
- You want a repo where diagnostics, auditability, and signoff discipline are part of the product.
- You are comfortable with a serious codebase and want a foundation that can support custom runtime work, genre expansion, and stricter CI/governance practices.

Choose something else if your priorities are:

- minimal-code onboarding
- a broad ready-made marketplace/community of drop-in assets and plugins
- immediate WYSIWYG content production over runtime ownership and deterministic behavior

## Honest Current Gaps

URPG still has important residual gaps. Some of the main ones currently called out by the canonical docs include:

- broader product-readiness and template-readiness proof
- real renderer/backend polish beyond the current bounded presentation/runtime contract
- deeper controller-governance and workspace-level panel wiring
- real device polling and richer per-device remap workflows
- export pipeline depth beyond current synthetic validation paths
- live mod loading/sandboxed execution/distribution workflows
- analytics upload, aggregation, and privacy/compliance workflows
- additional accessibility adapters beyond the current bounded menu slice

The project intentionally documents these as residual gaps instead of flattening them into vague “coming soon” language.

## Build

Recommended local configuration:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest --preset dev-all
```

Other useful presets:

```powershell
cmake --preset dev-vs2022
cmake --build --preset dev-vs2022-debug

cmake --preset dev-mingw-debug
cmake --build --preset dev-mingw-debug-build

cmake --preset ci
cmake --build --preset ci-release
```

## Focused Validation

Presentation gate:

```powershell
.\tools\ci\run_presentation_gate.ps1
```

Full local gates:

```powershell
.\tools\ci\run_local_gates.ps1
```

Examples of focused test lanes:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[analytics]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[input]" --reporter compact
ctest -L pr
ctest -L nightly
ctest -L weekly
```

## CI And Governance

Active governance/tooling lanes include:

- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `tools/ci/check_schema_changelog.ps1`
- `tools/ci/check_accessibility_governance.ps1`
- `tools/ci/check_audio_governance.ps1`
- `tools/ci/check_achievement_governance.ps1`
- `tools/ci/check_character_governance.ps1`
- `tools/ci/check_mod_governance.ps1`
- `tools/ci/check_analytics_governance.ps1`
- `tools/ci/check_input_governance.ps1`

Canonical audit and readiness documents:

- [Project Audit](./docs/PROJECT_AUDIT.md)
- [Release Readiness Matrix](./docs/RELEASE_READINESS_MATRIX.md)
- [Template Readiness Matrix](./docs/TEMPLATE_READINESS_MATRIX.md)
- [Truth Alignment Rules](./docs/TRUTH_ALIGNMENT_RULES.md)
- [Subsystem Status Rules](./docs/SUBSYSTEM_STATUS_RULES.md)
- [Release Signoff Workflow](./docs/RELEASE_SIGNOFF_WORKFLOW.md)

## Documentation Map

Start here for truth:

- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Technical Debt Remediation Plan](./docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- [Native Feature Absorption Plan](./docs/NATIVE_FEATURE_ABSORPTION_PLAN.md)

Subsystem, validation, and governance references:

- [Project Audit](./docs/PROJECT_AUDIT.md)
- [Schema Changelog](./docs/SCHEMA_CHANGELOG.md)
- [Wave 1 Closure Checklist](./docs/WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)
- [AI Copilot Guide](./docs/AI_COPILOT_GUIDE.md)
- [Presentation Docs Hub](./docs/presentation/README.md)
- [Presentation Validation Guide](./docs/presentation/VALIDATION.md)
- [Archive Index](./docs/archive/README.md)

Current sprint pointer:

- [Active Sprint](./docs/superpowers/plans/ACTIVE_SPRINT.md)

## Contributor Workflow

Useful commands:

```powershell
python -m pip install pre-commit
pre-commit install
pre-commit run --all-files

python .\tools\assets\asset_hygiene.py --write-reports
.\tools\rpgmaker\validate-plugin-dropins.ps1
```

See also:

- [CONTRIBUTING.md](./CONTRIBUTING.md)
- [.github/PULL_REQUEST_TEMPLATE.md](./.github/PULL_REQUEST_TEMPLATE.md)

## Migration CLI

```powershell
.\build\dev-ninja-debug\urpg_migrate.exe --input <project.json> --migration tools\migrate\migration_op.json --output <out.json>
```

## Summary

URPG is already more than a prototype: it has native runtime ownership for the main RPG lanes, meaningful advanced systems, a compatibility bridge, editor surfaces, diagnostics, migration tooling, and real governance.

Its value is not “everything is done.” Its value is that the repo makes it unusually clear what is done, what is partial, how it is validated, and where teams can build next without inheriting a hidden plugin-shaped architecture.
