# URPG Maker

URPG Maker is a deterministic C++20 RPG engine and editor for native-first RPG creation. It combines a native runtime core, an ImGui editor, RPG Maker MZ compatibility and migration tooling, OpenGL/headless rendering paths, release packaging, Catch2 validation, and governance docs that keep feature claims tied to evidence.

The product direction is WYSIWYG-first: a feature is not considered complete just because its backend exists. The expected bar is visual authoring, live preview of the behavior that will ship, saved project data, runtime execution, diagnostics, tests, and truthful readiness docs.

## Current State

Status date: 2026-04-28

Current pushed checkpoint:

- Branch: `development`
- Latest pushed commit at the time of this README update: `cfdae41e9` (`Advance AI editor and template readiness`)
- Remote: `origin/development` at [DocDamage/URPG-RPG-Game-Designer](https://github.com/DocDamage/URPG-RPG-Game-Designer)

The tree is suitable for internal/private release-candidate validation. It is not yet public-release-ready. Public release remains blocked on qualified legal/privacy/distribution approval or waiver, a recorded final release decision, and an annotated prerelease or release tag.

Recent current-state additions include:

- AI assistant project-edit workflow with visible approve, reject, approve-all, apply, revert, preview, and result-diff state exposed through deterministic editor snapshots.
- AI tool applications now record forward JSON patches and reverse patches so an AI-applied project change can be reverted.
- A deterministic AI knowledge and tool registry covering map authoring, event graphs, dialogue, localization, abilities, battle VFX, save preview, export preview, asset import, template instancing, and creator commands.
- Creator-command planning with deterministic local plans plus provider profiles for hosted and OpenAI-compatible local transports.
- Expanded template coverage with starter manifests, specs, certification loops, runtime profiles, readiness rows, and WYSIWYG showcase bindings for the advanced template set.
- Visual novel pacing runtime/editor/schema coverage and additional panel snapshot evidence across ability, battle VFX, dialogue, events, export preview, save preview, spatial environment, and 3D dungeon/world authoring surfaces.

## What Is Implemented

### Runtime Core

- C++20 deterministic runtime kernels and app shell.
- Scene flow for title/menu/map/battle/startup-adjacent paths.
- Save/load, slot metadata, recovery diagnostics, migration paths, and save preview lab coverage.
- Settings, input remapping, localization bundles, performance profiling, analytics consent, and startup diagnostics.
- Gameplay systems for battles, abilities, events, dialogue, quests, relationships, crafting, encounters, loot, NPC schedules, character creation, achievements, mod state, audio mix presets, accessibility auditing, and template runtime profiles.

### Editor

- ImGui editor shell with release-governed panel registration.
- Diagnostics workspace for project audit, migration, compatibility, audio, abilities, events, menu/message/battle/save, project health, and export surfaces.
- WYSIWYG panels for ability sandboxing, battle VFX timelines, event command graphs, dialogue preview, export preview, save/load lab, map environment preview, 3D dungeon/world authoring, visual novel pacing, template workflows, and broad maker-style feature families.
- AI assistant and creator command panels for reviewable AI-generated project edits.
- Disabled, empty, and error state snapshots for headless verification of many editor controls.

### Compatibility And Migration

- QuickJS-based RPG Maker MZ compatibility layer under `runtimes/compat_js/`.
- Bounded DataManager, BattleManager, Window, Sprite, Input, Audio, plugin-fixture, and migration surfaces.
- Compat is intentionally scoped as an import, validation, and migration harness. It is not claimed as full live RPG Maker JavaScript runtime parity.

### AI

- `IChatService` abstraction and deterministic `MockChatService` remain the in-tree runtime provider path.
- Live chatbot providers are still not shipped as runtime services in tree.
- Creator-command transport profiles exist for ChatGPT/OpenAI, Kimi, OpenRouter, Ollama, LM Studio, and other hosted/local OpenAI-compatible endpoints.
- AI tool plans require explicit review before mutation. The editor exposes approval controls and diff/revert data; `ChatbotComponent` exposes equivalent tool commands for scripted review flows.

### Templates And Governance

- Template runtime profiles, starter project generation, certification, readiness rows, and specs cover JRPG, visual novel, turn-based RPG, tactics RPG, ARPG, monster collector, cozy/life, metroidvania-lite, 2.5D RPG, roguelite dungeon, survival horror, farming adventure, card battler, platformer, gacha hero, mystery detective, world exploration, and the additional genre-template set under `content/templates/` and `docs/templates/`.
- Readiness docs are conservative. `READY` means the bounded claimed scope has evidence; `PARTIAL` means either the code is intentionally scoped or productization/release evidence is still incomplete.

### Release Engineering

- CMake/Ninja and Visual Studio build presets.
- Catch2 unit, integration, snapshot, compat, and engine test lanes.
- Local, PR, nightly, weekly, presentation, package, install, release-readiness, schema, truth-reconciliation, asset, and docs checks.
- Export packager/validator and release-required asset verification.

## Boundaries

- Public release is blocked until legal/privacy/distribution and tagging exits are closed.
- Full native signing, notarization, broader platform packaging, and public artifact policy are still backlog beyond the current internal validation checkpoint.
- Achievement platform SDKs, live cloud sync, external marketplaces, payments, reviews, publishing, and production analytics upload remain project-configured, simulated, out-of-tree, or future work.
- Raw/vendor/source asset packs are not release-package dependencies until license review, attribution review, promotion, and package constraints are cleared.
- Offline FAISS/SAM/SAM2/Demucs/Encodec-style tooling remains an artifact-generation lane under `tools/`, not a runtime dependency.

## Repository Layout

- `apps/`: runtime and editor entry points.
- `engine/core/`: native runtime systems.
- `engine/api/`: public runtime API entry points.
- `engine/runtimes/bridge/`: native/script bridge support.
- `editor/`: editor panels, models, diagnostics, and workspaces.
- `runtimes/compat_js/`: QuickJS and RPG Maker MZ compatibility surfaces.
- `content/`: schemas, fixtures, templates, abilities, readiness data, and release-required content.
- `resources/`: release-required app resources.
- `tests/`: unit, integration, snapshot, compat, and engine coverage.
- `tools/`: CI gates, docs checks, asset tooling, packaging, migration, and workflow scripts.
- `docs/`: architecture, roadmap, status, readiness, signoff, ADRs, governance, and validation guides.

## Build

Recommended Windows/Ninja build:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
```

Other supported local presets:

```powershell
cmake --preset dev-vs2022
cmake --build --preset dev-vs2022-debug

cmake --preset dev-mingw-debug
cmake --build --preset dev-mingw-debug-build
```

## Test And Validate

Run all registered tests for the active debug preset:

```powershell
ctest --preset dev-all --output-on-failure
```

Common focused gates:

```powershell
ctest --preset dev-all -L pr --output-on-failure
ctest --preset dev-all -L nightly --output-on-failure
ctest --preset dev-all -L weekly --output-on-failure
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_local_gates.ps1
```

AI/editor lanes used for the latest AI assistant checkpoint:

```powershell
ctest --preset dev-all -R "AI (knowledge|task|tool|assistant)|Chatbot component" --output-on-failure
ctest --preset dev-all -R "FFS-16|AI suggestions require approval" --output-on-failure
```

Docs and governance checks:

```powershell
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
.\tools\ci\check_release_required_assets.ps1
```

## Documentation Map

Start here:

- [Agent Knowledge Index](./docs/agent/INDEX.md)
- [Architecture Map](./docs/agent/ARCHITECTURE_MAP.md)
- [Quality Gates](./docs/agent/QUALITY_GATES.md)
- [Execution Workflow](./docs/agent/EXECUTION_WORKFLOW.md)
- [Known Debt](./docs/agent/KNOWN_DEBT.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Status Snapshot](./docs/status/PROGRAM_COMPLETION_STATUS.md)
- [Release Readiness Matrix](./docs/release/RELEASE_READINESS_MATRIX.md)
- [Editor Control Inventory](./docs/release/EDITOR_CONTROL_INVENTORY.md)
- [AI Copilot Guide](./docs/integrations/AI_COPILOT_GUIDE.md)
- [Template Specs](./docs/templates/)

## Development Rules

- Keep claims truthful and evidence-backed.
- Prefer existing subsystem patterns over new abstractions.
- Update tests and docs when behavior changes.
- Keep release-required assets independent of unavailable source/vendor LFS packs.
- Use focused verification for narrow changes and broader gates for release-facing changes.
- Do not mark public release readiness until legal/public distribution exits are closed or formally waived.

## Summary

URPG Maker is an ambitious native RPG engine/editor with broad deterministic runtime coverage, many WYSIWYG authoring surfaces, a bounded RPG Maker MZ compatibility harness, internal release validation infrastructure, and conservative readiness governance. The current `development` branch is a strong internal validation checkpoint, with public release still gated by distribution/legal exits and remaining platform-productization work.
