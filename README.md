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

Status date: 2026-04-22

What is true in the current tree:

- Wave 1 native ownership for UI/Menu, Message/Text, Battle, and Save/Data is landed.
- Phase 2 compat runtime closure is complete; remaining compat work is post-closure hardening and truth maintenance.
- Canonical governance, readiness, audit, and reconciliation lanes are active.
- The Gameplay Ability Framework has moved beyond backend-only groundwork into live editor/runtime workflows.
- The spatial editor lane now includes a composed WYSIWYG workspace for terrain, props, and map ability interactions.

What is also true:

- URPG is not yet feature-complete.
- URPG is not yet fully WYSIWYG across the whole product.
- Several advanced systems are partially implemented, or implemented at the runtime layer before their easiest visual workflows are fully closed.

So the current repo should be read as:

- strong native/runtime/editor foundations are landed
- the remaining mission is to close missing features and finish their WYSIWYG workflows, not to argue against WYSIWYG

For canonical status, use:

- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Technical Debt Remediation Plan](./docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- [Native Feature Absorption Plan](./docs/NATIVE_FEATURE_ABSORPTION_PLAN.md)
- [Release Readiness Matrix](./docs/RELEASE_READINESS_MATRIX.md)

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
- [Technical Debt Remediation Plan](./docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md)
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
