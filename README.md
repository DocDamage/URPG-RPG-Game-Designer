# URPG (Universal RPG Engine)

![URPG Header](https://raw.githubusercontent.com/URPG-Project/assets/main/header.png)

URPG is a native-first C++20 RPG engine and editor targeting a fully WYSIWYG, extremely easy-to-use authoring experience on top of a deterministic runtime core.

The project is not trying to make creators think like engine programmers. The native/runtime work exists so the editor can preview and ship the real game behavior truthfully instead of relying on hidden plugin stacks, fake editor-only approximations, or fragile export-time glue.

## Product Direction

URPG is aiming for:

- real WYSIWYG authoring backed by the same runtime that ships
- low-friction editor workflows for non-technical creators
- native ownership of game-critical systems instead of plugin accretion
- deterministic runtime behavior, migration-safe schemas, and evidence-backed validation
- bounded RPG Maker MZ compatibility for import, verification, and migration confidence

The core product rule is simple:

- a feature is not considered complete unless it is implemented, visually authorable, live-previewable, and usable through a low-friction editor workflow

## Why URPG

URPG is for teams that want both:

- a serious native engine/runtime they can trust
- an editor that feels immediate, visual, and approachable

Compared with similar tools, URPG is differentiated by:

- Native ownership instead of plugin accretion: core systems are implemented in C++ and tested as first-class engine features.
- Deterministic architecture: ECS-style ownership, explicit scene/runtime boundaries, and stable validation lanes make behavior easier to reason about.
- WYSIWYG backed by real runtime seams: the editor is being built to drive the same runtime paths that ship, not a disconnected mock layer.
- Migration and compatibility discipline: RPG Maker MZ support is treated as an import and verification bridge, not vague parity marketing.
- Diagnostics and governance as product surface: audit panels, readiness records, validation scripts, and signoff workflows are part of the repo.

## Current State

Status date: 2026-04-24

What is true in the current tree:

- Wave 1 native ownership for UI/Menu, Message/Text, Battle, and Save/Data is landed.
- Phase 2 compat runtime closure is complete; remaining compat work is post-closure hardening and truth maintenance.
- Canonical governance, readiness, audit, and reconciliation lanes are active.
- The Gameplay Ability Framework has moved beyond backend-only groundwork into live editor/runtime workflows.
- The spatial editor lane now includes a composed WYSIWYG workspace for terrain, props, and map ability interactions.
- The first future-feature vertical slices for Project Health and Asset Library intake are now landed as conservative editor/runtime projections over existing audit and hygiene data.
- A large local `more assets/` intake has been unpacked into `imports/raw/more_assets/`, cleaned of OS/archive junk, indexed, and prepared for Git LFS tracking.

What is also true:

- URPG is not yet feature-complete.
- URPG is not yet fully WYSIWYG across the whole product.
- Several advanced systems are partially implemented, or implemented at the runtime layer before their easiest visual workflows are fully closed.

So the current repo should be read as:

- strong native/runtime/editor foundations are landed
- the remaining mission is to close missing features and finish their WYSIWYG workflows, not to argue against WYSIWYG

For canonical status, use:

- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Native Feature Absorption Plan](./docs/NATIVE_FEATURE_ABSORPTION_PLAN.md)
- [Release Readiness Matrix](./docs/RELEASE_READINESS_MATRIX.md)
- [Future Feature Upgrade Plans](./docs/FUTURE_FEATURE_UPGRADE_PLANS.md)
- [Future Feature Actionable Sprint Plan](./docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md)

## What Is Landed

### Wave 1 native runtime ownership

- UI/Menu Core
- Message/Text Core
- Battle Core
- Save/Data Core

Each of those lanes now has native runtime ownership plus editor, schema, migration, diagnostics, and validation coverage in the current tree.

### Gameplay Ability and WYSIWYG spatial authoring

- gameplay tags, abilities, effects, and state-machine-integrated execution
- battle-scene ability activation through participant-owned ability systems
- authored ability assets under `content/abilities/`
- diagnostics workspace ability authoring, preview, save/load, and runtime apply flows
- map-scene ability consumption and interaction binding
- click-to-place tile bindings, prop bindings, and region painting on the map
- always-visible canvas overlays, direct on-canvas selection and editing, hover previews, trigger switching, and conflict warnings
- a composed spatial authoring workspace that keeps terrain, props, and interaction authoring in one editor surface

### Product and governance systems already in tree

- ProjectAudit and diagnostics workspace
- ProjectHealth diagnostics projection with grouped health cards and a deterministic fix-next queue
- AssetLibrary runtime/editor model with provenance packets, duplicate status, case-collision warnings, and cleanup previews
- release-readiness and template-readiness artifacts
- truth reconciliation and schema changelog checks
- accessibility, audio, analytics, achievement, character, export, mod, and input governance lanes
- migration CLI and compatibility validation tooling

## What Still Needs To Be Finished

The canonical docs still call out mandatory remaining work, including:

- wider Gameplay Ability Framework closure beyond the current landed slices
- Character Identity / full Create-a-Character closure
- Achievement Registry export/platform follow-through
- Accessibility Auditor expansion
- Audio Mix Presets expansion
- Mod Registry live loading and sandboxed execution
- Analytics Dispatcher completion
- Export hardening, broader visual validation, and stronger release-signoff enforcement

Those are not separate from the WYSIWYG goal. Each one still needs both:

- runtime capability
- easy visual authoring and live preview

## Future Work Roadmap

The future roadmap is intentionally separated from current readiness claims. These items are planned product direction, not automatic `READY` commitments. Each one should land as a narrow, evidence-backed vertical slice with runtime code, editor workflow, schema or fixtures where needed, diagnostics, tests, docs, and conservative readiness updates.

Detailed planning lives in:

- [Future Feature Upgrade Plans](./docs/FUTURE_FEATURE_UPGRADE_PLANS.md)
- [Future Feature Actionable Sprint Plan](./docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md)
- [Asset Library And More Assets Intake](./docs/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md)

### First product upgrades

- Project Health / Readiness Dashboard: first editor-facing health/fix-queue projection is landed; remaining work is richer remediation workflow wiring.
- Real Asset Library + Asset Intake UX: first runtime/editor asset-library slice is landed with provenance, duplicate detection, report loading, safe cleanup previews, and raw `more assets/` intake indexing; remaining work is promotion workflow and richer license review UX.
- Visual Event Authoring: add a no-code event-page workflow for conditions, commands, switches, variables, movement, messages, battles, transfers, and common events.
- Plugin Compatibility Inspector: surface plugin manifests, dependencies, permissions, unsupported calls, load order, fallback behavior, and shim hints.
- Battle Presentation Authoring: give creators control over battlebacks, HUD layout, cues, popups, icons, music, and deterministic preview playback.
- Tilemap / Terrain / Layer Upgrade: improve spatial authoring with terrain sets, layer locking, collision/navigation metadata, overpasses, and runtime-preview parity.
- Export Runtime Hardening: add runtime-side `data.pck` signature enforcement, atomic bundle publication, release comparison, patching, and clearer export artifact reporting.
- Starter Project Templates: ship polished JRPG, Visual Novel, and Turn-Based RPG starters with maps, dialogue, battle, save/load, menus, export profiles, and readiness evidence.
- Save/Load Debugger: inspect slots, recovery tiers, migration notes, corrupted payloads, subsystem state, and save compatibility.
- One-Click Dev Room Test Harness: generate a test scene that exercises events, battle, save/load, plugins, audio, input, and export warnings.

### Second-wave authoring systems

- Guided New Project Wizard.
- Quest / Objective System.
- Dialogue Graph Editor.
- Common Event Library.
- Database Editor Parity Pass.
- Cutscene / Timeline Sequencer.
- In-Editor Playtest With Time Travel.
- Localization Workspace.
- Accessibility Authoring Assistant.
- Formula / Rule Debugger.
- Economy Balancer.
- Encounter Table Editor.
- World Map / Travel System.
- 2D Lighting / Weather Authoring.
- Sprite / Animation Import Pipeline.
- Controller + Keyboard Remap UX.
- Crash / Diagnostics Bundle Export.
- Mod SDK Documentation + Sample Mod.
- Cloud-Free Backup / Project Snapshotting.
- Tutorial Project + Interactive Lessons.

### Differentiating "magic layer" upgrades

- Procedural Dungeon / Map Generator.
- Flag / Switch / Variable Dependency Graph.
- Narrative Continuity Checker.
- Event Macro Recorder.
- Local AI Design Assistant with opt-in providers and no runtime dependency.
- Smart Autofill Database Tools.
- Visual Diff For Game Data.
- Local-only Authoring Heatmaps.
- Deterministic Replay Gallery.
- Boss Fight Designer.
- State Machine Visualizer.
- Content Completeness Score.
- In-Editor Screenshot / Trailer Capture.
- Theme / UI Skin Builder.
- Relationship / Reputation System.
- Crafting / Recipe System.
- Bestiary / Codex System.
- Photo Mode / Diorama Mode.
- Patch / DLC Builder.
- Creator Marketplace-Ready Packaging.

### Fourth-wave genre and depth systems

- Enemy AI Behavior Designer.
- Party Tactics / Auto-Battle Planner.
- Job / Class Progression Designer.
- Skill Combo / Synergy System.
- Loot Affix / Rarity Generator.
- Tactical Grid / Range Preview Toolkit.
- Puzzle / Lock-Key System.
- Shop / Vendor Designer.
- Inn / Rest / Recovery System.
- Calendar / Time-of-Day System.
- NPC Schedule / Routine Designer.
- Reputation-Gated Content Browser.
- Map Region Rules Editor.
- Spawn / Respawn System.
- Runtime Tutorial / Hint System.
- Player Choice Consequence Tracker.
- Ending / Route Manager.
- Save Compatibility / Migration Previewer.
- Device / Platform Preview Profiles.
- Local Co-Author Review Workflow.

### Planned follow-on upgrades

- Guided remediation workflows for the Project Health / Readiness Dashboard.
- Auto-cleanup and provenance packets for the asset library.
- Event debugger with breakpoints for visual event authoring.
- Compatibility scoring and shim hints for plugin inspection.
- Phase timelines and cue choreography for battle presentation.
- Advanced brushes and chunked validation for tilemaps.
- Release candidate comparison for export hardening.
- Template certification suites for starter projects.
- Corruption lab and recovery simulation for save/load debugging.
- Auto-generated regression routes for dev rooms.
- Quest graph and route coverage.
- Dialogue text pipeline integration.
- Parametric common-event recipes.
- Bulk edit and validation lanes for database editing.
- Runtime capture and replay for cutscenes.
- Bug repro export for playtest time travel.
- Translation memory and glossary support for localization.
- Accessibility preview modes.
- Batch balance probes for formulas and rules.
- Playthrough economy simulation.
- Difficulty-curve visualization for encounters.
- Generator profiles and constraints for procedural maps.
- Auto-fix and impact preview for dependency graphs.
- Route proof reports for narrative continuity.
- Golden replay CI lanes.
- Guardrailed source attribution and review state for AI suggestions.

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
- `editor/`: editor panels, models, and workspaces
- `runtimes/compat_js/`: bounded RPG Maker MZ compatibility bridge
- `tests/`: unit, integration, snapshot, compat, and engine coverage
- `tools/`: CI gates, migration tools, audit tooling, asset hygiene, workflow/bootstrap tooling
- `docs/`: roadmap, status, readiness, signoff, ADRs, and validation guides

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

Examples of focused lanes:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[editor][spatial]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][integration][abilities]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[scene][map]" --reporter compact
ctest -L pr
ctest -L nightly
ctest -L weekly
```

## Documentation Map

Start here for truth:

- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Native Feature Absorption Plan](./docs/NATIVE_FEATURE_ABSORPTION_PLAN.md)

Supporting references:

- [Project Audit](./docs/PROJECT_AUDIT.md)
- [Release Readiness Matrix](./docs/RELEASE_READINESS_MATRIX.md)
- [Template Readiness Matrix](./docs/TEMPLATE_READINESS_MATRIX.md)
- [Presentation Docs Hub](./docs/presentation/README.md)
- [AI Copilot Guide](./docs/AI_COPILOT_GUIDE.md)
- [Archive Index](./docs/archive/README.md)
- [Active Sprint](./docs/superpowers/plans/ACTIVE_SPRINT.md)

## Summary

URPG is building toward a product that is both native-first and genuinely WYSIWYG.

The infrastructure work is not a detour away from that goal. It is the foundation that lets the editor preview the real runtime truthfully. The remaining work is to finish the missing systems and close their visual authoring workflows until the whole product meets that standard.
