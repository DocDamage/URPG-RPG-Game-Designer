# URPG (Universal RPG Engine)

![URPG Header](https://raw.githubusercontent.com/URPG-Project/assets/main/header.png)

URPG is a native-first C++20 RPG engine and editor for deterministic 2D RPG development. It combines a native runtime core, an ImGui editor, RPG Maker MZ compatibility and migration tooling, OpenGL and headless rendering paths, release packaging, and evidence-backed validation.

The product goal is a WYSIWYG authoring experience where creators edit the same runtime behavior that ships. A feature is treated as complete only when it is implemented, visually authorable where appropriate, live-previewable, test-covered, and represented truthfully in the project docs.

## Current Status

Status date: 2026-04-27

The current `development` branch has passing local and remote release-candidate gates for private/internal release-candidate validation. The remote manual GitHub Actions release-candidate workflow passed in run `25025111713` at commit `7439132f4fa2638730498781f617d78af7b16514`.

URPG is not yet public-release-ready. Remaining public release exits are:

- qualified legal/privacy/distribution approval or an explicit public-release waiver
- a recorded final release decision
- an annotated prerelease or release tag

Repository-wide source/vendor LFS hydration is still externally constrained by GitHub LFS budget/access, but release-required package assets are normal Git blobs and pass fresh-clone release verification.

## What URPG Can Do Today

### Native Runtime

- Deterministic native runtime core in C++20.
- Scene shell with title, menu, map, battle, options/startup-adjacent, and headless test paths.
- Runtime title flow with New Game, Continue, Options-style startup integration surfaces, disabled/error states, and save discovery.
- Runtime startup diagnostics for audio, input, localization, profiler, project data, and preflight failures.
- Save/load flow with slot metadata, recovery/failure handling, migration diagnostics, and native runtime load paths.
- Settings persistence for runtime and editor configuration.
- Input remapping with MZ-compatible defaults, JSON persistence, version validation, unsaved-change tracking, and runtime startup loading.
- Localization bundles with merge support, completeness checking, missing-key reporting, schema validation, and CI consistency checks.
- Performance profiling with frame-time buffers, budget violations, named section timings, JSON export, and diagnostics integration.
- Analytics consent handling with opt-in gating, persisted consent, local buffering, and no default upload behavior.

### Rendering And Presentation

- SDL2/OpenGL rendering path with headless CI fallback.
- Value-owned frame render commands consumed by OpenGL/headless backends.
- Text and rectangle command submission through the presentation/render bridge.
- Renderer-backed visual capture tests with committed goldens.
- Visual regression harness with golden load/save, snapshot comparison, diff heatmap generation, JSON reports, and approval tooling.
- Presentation bridge for active scene frame construction and battle scene state translation.
- Runtime VFX cue baseline for battle effects, including semantic cue kinds and resolver/translator coverage.
- Spatial presentation/editor support for terrain elevation, props, interaction overlays, and composed workspace snapshots.
- Baseline raycast/2.5D presentation lane support in the native render stack.

### RPG Systems

- Native Wave 1 ownership for UI/Menu Core, Message/Text Core, Battle Core, and Save/Data Core.
- Menu scene graph, command registry, route resolver, editor preview, diagnostics export, and schema/migration coverage.
- Message flow runner, rich text layout, choice prompt state, portrait binding, text rendering handoff, editor workflows, and diagnostics round-trips.
- Battle runtime with participant state, bounded formula evaluation, battle event migration, reward/event cadence coverage, BGM/ME routing through compat audio state, battleback diagnostics, HUD quads, gauges, popups, guard markers, and state icon presentation.
- Save/data runtime with slot descriptors, save policy governance, compat save import, native payload generation, JSONL diagnostics, and metadata preservation.
- Gameplay Ability Framework with tags, abilities, effects, cooldown/cost checks, ability system components, battle activation, map-side ability consumption, authored ability assets, diagnostics preview, save/load, and runtime apply flows.
- Map interaction ability binding for tile, prop, and region triggers.
- Character identity and create-a-character runtime/editor slice with name, portrait, body, class, attributes, appearance tokens, JSON round-trip, schema, editor model/panel, runtime screen, and tests.
- Achievement registry with definitions, progress, unlock-condition tokens, save/load JSON, vendor-neutral trophy export payload, editor panel, and tests.
- Audio mix presets with default, battle, and cinematic profiles, category volumes, ducking rules, schema, editor panel, and tests.
- Accessibility auditor with missing-label, focus-order, contrast, and navigation rules plus editor reporting.
- Mod registry with manifest registration, dependency ordering, activation state, save/load JSON, schema, editor panel, and tests.
- Export validator and packager with platform requirement checks, package validation, config schema, editor diagnostics panel, install smoke, package smoke, and release-candidate gates.

### Editor And Authoring

- ImGui editor shell with governed production panel registration and headless panel listing.
- Diagnostics workspace covering audit, migration, audio, abilities, event authority, menu, message, battle, project health, export, and other subsystem projections.
- Project Audit CLI and diagnostics tab with readiness-derived blockers, fix-next queues, stale-state detection, and JSON output.
- Project Health dashboard slice with grouped health cards and deterministic remediation ordering.
- Asset Library runtime/editor model with asset records, provenance packets, duplicate grouping, case-collision flags, report loading, safe cleanup previews, and raw asset intake indexing.
- Ability Inspector with draft authoring, preview/test activation, runtime application, canonical `content/abilities/` discovery, load/apply/save workflows, and map-scene handoff.
- Spatial Authoring Workspace that composes elevation painting, prop placement, map ability binding, and direct canvas overlays.
- Direct canvas interaction authoring with click-to-place tile bindings, prop handles, region painting/resizing, hover previews, trigger switching, conflict warnings, and suggested conflict resolution.
- Character Creator, Achievement, Accessibility, Audio Mix, Analytics, Export Diagnostics, Mod Manager, Input Remap, Localization, Performance Diagnostics, and Project/New Project surfaces with explicit disabled/empty/error states where dependencies are absent.
- Migration wizard diagnostics with rendered workflow actions, selected-result details, issue navigation, report save/load, and bound-runtime rerun flows.

### RPG Maker MZ Compatibility And Migration

- QuickJS-based bounded compatibility layer under `runtimes/compat_js/`.
- RPG Maker MZ-shaped DataManager, BattleManager, Window, Sprite, Input, and Audio surfaces for import, validation, and migration confidence.
- DataManager seeded-container behavior, database loading, enemy battler name preservation, and export paths.
- BattleManager support for bounded formulas, troop page conditions, variable conditional branches, switches, battle audio routing, reward/event cadence, and fallback preservation for unsupported effects.
- Window compatibility for text drawing, `drawTextEx`, selectable pointer/mouse-wheel semantics, message alignment, contents lifecycle, and bitmap handle dimensions.
- Sprite compatibility for character/actor bitmap ownership, reload, destructor cleanup, source rect mapping, and actor motion lifecycle.
- Save migration from `.rpgsave`-style compat documents into native runtime save state while retaining unsupported plugin blobs and unmapped payload fields.
- Compatibility corpus health checks with executable fixtures separated from health-only corpus fixtures.

### Tools, Governance, And Release Engineering

- CMake/Ninja and Visual Studio build presets.
- Catch2 unit, integration, snapshot, compat, and engine test suites.
- Local gates, presentation gate, release-candidate gate, install smoke, package smoke, CMake completeness, truth reconciliation, schema-breaking-change, release-readiness, localization, save-policy, and asset/governance checks.
- GitHub Actions CI with PR, release-candidate, nightly, and weekly lanes.
- Release-required asset verification from a fresh GitHub clone.
- Component install and CPack package layout with runtime data, docs, metadata, icon, and desktop metadata entries.
- Legal/release docs, app readiness matrix, release readiness report, legal signoff record, EULA, privacy policy, third-party notices, credits, and changelog.
- Documentation health checks for agent knowledge, canonical status, and readiness references.

## Current Limitations

URPG is a strong native/editor foundation, not a finished public product. Important limitations are intentionally documented:

- Public release is blocked until legal/privacy/distribution approval or waiver is recorded.
- Some editor surfaces are release-registered but still have partial graphical/manual behavior verification.
- Export hardening still has backlog items such as runtime-side bundle signature enforcement, atomic publication, native signing, notarization, and broader platform validation.
- Achievement platform backends are out of tree; the current implementation is a vendor-neutral registry/export payload.
- The QuickJS compatibility layer is a bounded import/validation/migration harness, not a promise of complete live RPG Maker JavaScript runtime parity.
- The in-tree AI/cloud surfaces are local/simulated boundaries only; live providers are out of tree.
- The production asset library is still conservative. Raw/vendor/source asset packs are not treated as release-package dependencies.

## Planned Product Work

The roadmap remains WYSIWYG-first. Planned or partially landed productization lanes include:

- richer Project Health guided remediation workflows
- asset promotion, license review, and cleanup automation
- visual event authoring and event debugging
- plugin compatibility inspection
- battle presentation authoring
- tilemap, terrain, layer, collision, navigation, overpass, lighting, and weather upgrades
- starter project templates
- save/load debugger and corruption lab
- one-click dev room test harness
- quest/objective system
- dialogue graph editor
- common event library
- database editor parity
- cutscene/timeline sequencer
- in-editor playtest with time travel
- localization workspace
- accessibility authoring assistant
- formula/rule debugger
- economy balancer
- encounter table editor
- world map and travel systems
- sprite/animation import pipeline
- controller and keyboard remap UX
- crash/diagnostics bundle export
- mod SDK docs and sample mod
- cloud-free backup/project snapshotting
- tutorial project and interactive lessons
- procedural map generation, dependency graphs, narrative continuity checking, replay galleries, UI skinning, crafting, codex/bestiary, patch/DLC packaging, and marketplace-ready packaging

## Architecture

Core stack:

- Language: C++20
- Build system: CMake 3.23+
- Primary local generator: Ninja
- Platforms: Windows first, with GCC/Clang support paths
- Rendering/windowing: SDL2 + OpenGL, with headless CI paths
- JSON: nlohmann/json
- Testing: Catch2 v3
- Compatibility runtime: QuickJS
- Editor UI: ImGui

Repository layout:

- `apps/`: runtime/editor entry points
- `engine/core/`: native runtime systems
- `engine/api/`: public runtime API entry points
- `engine/runtimes/bridge/`: native/script bridge support
- `editor/`: editor panels, models, diagnostics, and workspaces
- `runtimes/compat_js/`: QuickJS and RPG Maker MZ compatibility surfaces
- `content/`: schemas, fixtures, abilities, readiness data, and release-required content
- `resources/`: release-required app resources
- `tests/`: unit, integration, snapshot, compat, and engine coverage
- `tools/`: CI gates, docs checks, asset tooling, packaging, migration, and workflow scripts
- `docs/`: architecture, roadmap, status, readiness, signoff, ADRs, and validation guides

## Build

Recommended local build:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
```

Run the default local test suite:

```powershell
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

## Validation

Focused gates:

```powershell
ctest -L pr
ctest -L nightly
ctest -L weekly
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_release_candidate_gate.ps1
```

Useful focused test examples:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[editor][spatial]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][integration][abilities]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[scene][map]" --reporter compact
.\build\dev-ninja-debug\urpg_snapshot_tests.exe "[snapshot][renderer][visual_capture]" --reporter compact
```

Documentation and readiness checks:

```powershell
.\tools\docs\check-agent-knowledge.ps1 -BuildRoot build/dev-ninja-debug
.\tools\ci\check_release_required_assets.ps1
```

## Documentation Map

Start with:

- [Agent Knowledge Index](./docs/agent/INDEX.md)
- [Architecture Map](./docs/agent/ARCHITECTURE_MAP.md)
- [Quality Gates](./docs/agent/QUALITY_GATES.md)
- [Execution Workflow](./docs/agent/EXECUTION_WORKFLOW.md)
- [Known Debt](./docs/agent/KNOWN_DEBT.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [App Release Readiness Matrix](./docs/APP_RELEASE_READINESS_MATRIX.md)
- [AAA Release Readiness Report](./docs/release/AAA_RELEASE_READINESS_REPORT.md)
- [AAA Release Execution Plan](./docs/release/AAA_RELEASE_EXECUTION_PLAN.md)
- [Native Feature Absorption Plan](./docs/NATIVE_FEATURE_ABSORPTION_PLAN.md)
- [Release Readiness Matrix](./docs/RELEASE_READINESS_MATRIX.md)
- [Future Feature Actionable Sprint Plan](./docs/archive/planning/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md)

## Development Rules

- Keep feature claims truthful and evidence-backed.
- Prefer existing subsystem patterns over new abstractions.
- Update tests and docs when behavior changes.
- Keep release-required assets independent of unavailable source/vendor LFS packs.
- Use focused verification for narrow changes and broader gates for release-facing changes.
- Do not mark public release readiness until legal/public distribution exits are closed or formally waived.

## Summary

URPG is building a native, deterministic RPG runtime and a WYSIWYG editor that previews real shipped behavior. The current tree has broad runtime, editor, compatibility, packaging, and validation foundations in place, with private/internal release-candidate gates passing. The next development cycle can add features on top of this checkpoint while preserving the project rule: implemented, authorable, previewable, tested, and documented.
