# URPG Maker

URPG Maker is a native-first RPG engine and editor for building deterministic, data-driven RPGs with a serious authoring workflow. It combines a C++20 runtime, an ImGui editor, RPG Maker MZ compatibility and migration tooling, OpenGL and headless rendering paths, governed asset intake, export validation, AI-assisted project editing, and a large validation suite.

The project goal is not just "an engine that can run RPG logic." URPG Maker is being built as a WYSIWYG game maker: features are expected to have saved project data, visible editor controls, live preview, runtime execution, diagnostics, tests, and truthful readiness documentation.

## Current State

Status date: 2026-04-29

The `development` branch is suitable for internal/private release-candidate validation. It is not yet public-release-ready. Public release remains blocked by legal/privacy/distribution approval or waiver, final release decision recording, and release tagging.

Recent progress includes:

- Review-gated AI project edits with visible approve, reject, approve-all, apply, preview, result-diff, and revert state in editor-facing models.
- AI-applied project changes now persist `_ai_change_history` records with forward/reverse JSON patch data, before/after project data, and reverted state so applied edits can be undone from project state.
- AI knowledge/tool indexing now covers project metadata, canonical docs, asset reports, schemas, readiness reports, template specs, validation reports, and subsystem-specific tool candidates.
- Concrete AI tool surfaces are tracked for event graph authoring, VFX timeline edits, lighting/weather preview updates, ability sandbox composition, export preview configuration, and asset import/promotion.
- `SRC-007` local URPG assets are catalog-normalized, deduped, and split into editor-loadable category shards.
- Generator candidates such as sprite, planet, background, and Tiled/TMX tooling are cataloged for both editor integration and generated-game runtime use.
- Third-party and itch assets have been moved out of root folders and into intake roots under `imports/raw/`.
- The third-party/itch asset roots are indexed through the local asset DB with a tracked audit summary.
- Documentation now distinguishes live product claims from catalog-only, private-use, and release-blocked asset intake.

## Why Use URPG Maker

Choose URPG Maker when you want RPG Maker-style productivity without giving up deterministic native runtime ownership, validation, and deeper systems control.

Compared with RPG Maker:

- Native C++ runtime ownership instead of a primarily JavaScript game runtime.
- Compatibility and migration tooling for RPG Maker MZ concepts without claiming full JS runtime parity.
- Stronger validation culture: schemas, tests, release gates, truth-alignment checks, and editor snapshots.
- Broader WYSIWYG ambitions: event graphs, VFX timelines, save labs, export previews, 3D/spatial authoring, template certification, and AI-reviewed project editing.

Compared with Unity or Godot:

- RPG-first data models and editor workflows instead of a general-purpose engine where the RPG toolchain must be built from scratch.
- Deterministic native systems and headless validation designed for CI, migration, and repeatable content checks.
- Built-in governance around release readiness, asset provenance, source manifests, bundle manifests, and attribution.
- A curated maker workflow where templates, diagnostics, export validation, and runtime profiles are first-class project concepts.

Compared with browser-only RPG tools:

- Native runtime and editor architecture.
- Offline/local-first authoring and validation.
- Structured import paths for RPG Maker, itch, third-party packs, and raw local asset drops.
- Explicit separation between raw intake, catalog browsing, normalized promoted assets, and release-export eligibility.

## Implemented Surface

### Runtime Core

- C++20 deterministic runtime kernels.
- Title, menu, map, battle, and startup-adjacent scene flows.
- Save/load, slot metadata, recovery diagnostics, migration paths, and save-preview data.
- Settings, input remapping, localization bundles, performance profiling, analytics consent, and startup diagnostics.
- Battle, ability, event, dialogue, quest, relationship, crafting, encounter, loot, NPC schedule, character creation, achievement, mod-state, audio-mix, accessibility, and template runtime models.

### Editor

- ImGui editor shell with governed panel registration.
- Diagnostics workspace for project audit, migration, compatibility, audio, abilities, events, menu/message/battle/save, project health, and export surfaces.
- WYSIWYG-oriented panels for ability sandboxing, battle VFX timelines, event command graphs, dialogue preview, export preview, save/load lab, map environment preview, 3D dungeon/world authoring, visual novel pacing, and template workflows.
- AI assistant and creator-command models for reviewable AI-generated project changes.
- Snapshot coverage for disabled, empty, error, and populated editor states.

### Compatibility And Migration

- QuickJS-based RPG Maker MZ compatibility layer under `runtimes/compat_js/`.
- Bounded DataManager, BattleManager, Window, Sprite, Input, Audio, plugin-fixture, and migration surfaces.
- RPG Maker compatibility is an import, validation, and migration harness. It is not advertised as full live RPG Maker JavaScript runtime parity.

### AI

- `IChatService` abstraction and deterministic in-tree `MockChatService`.
- Creator-command transport profiles for OpenAI-compatible hosted and local providers such as ChatGPT/OpenAI, Kimi, OpenRouter, Ollama, and LM Studio, with editor-visible provider selection, dry-run/live test state, and failure reasons.
- Review-gated AI tool plans before mutation.
- Patch-backed AI apply/revert flow for project JSON changes.
- Current live chatbot provider implementation is still a productization priority; deterministic local behavior remains the shipped in-tree baseline.

### Templates

URPG Maker has starter/template governance for JRPG, visual novel, turn-based RPG, tactics RPG, ARPG, monster collector, cozy/life, metroidvania-lite, 2.5D RPG, roguelite dungeon, survival horror, farming adventure, card battler, platformer, gacha hero, mystery detective, world exploration, space opera, soulslike-lite, school life, rhythm RPG, racing adventure, post-apocalyptic RPG, pirate RPG, cooking/restaurant RPG, city builder RPG, strategy kingdom RPG, sports team RPG, tactical mecha RPG, tower defense RPG, faction politics RPG, and related variants.

Template support includes runtime profiles, starter manifests, specs, certification loops, readiness rows, and WYSIWYG showcase bindings. A `READY` row means the bounded claimed template scope has evidence; it does not mean every possible game in that genre is complete.

### Assets

Asset handling is intentionally governed:

- Raw intake lives under `imports/raw/`.
- Normalized promoted assets live under `imports/normalized/`.
- Source manifests live under `imports/manifests/asset_sources/`.
- Bundle manifests live under `imports/manifests/asset_bundles/`.
- Intake and audit reports live under `imports/reports/asset_intake/`.
- The local asset DB is ignored under `.urpg/asset-index/`.

Current cataloged intake includes:

- `SRC-007` local URPG asset drop, deduped to 56,096 non-audio/tool catalog records with zero remaining exact duplicate groups in the promotion catalog.
- Existing OGG audio context preserved under raw intake; tracked MP3/WAV files are intentionally excluded from Git.
- Third-party and itch roots indexed from `imports/raw/third_party_assets/` and `imports/raw/itch_assets/`.
- Generator/tool candidates for sprite generation, pixel planets, space backgrounds, and Tiled/TMX workflows.

Cataloged raw assets are not release-export eligible until curated subsets receive attribution, bundle manifests, and promotion approval.

## Repository Layout

- `apps/`: runtime/editor entry points.
- `engine/core/`: native runtime systems.
- `engine/api/`: public runtime API entry points.
- `engine/runtimes/bridge/`: native/script bridge support.
- `editor/`: editor panels, models, diagnostics, and workspaces.
- `runtimes/compat_js/`: QuickJS and RPG Maker MZ compatibility surfaces.
- `content/`: schemas, fixtures, templates, abilities, readiness data, and release-required content.
- `resources/`: release-required app resources.
- `imports/raw/`: quarantined raw local, third-party, itch, RPG Maker, and generator/tool source intake.
- `imports/normalized/`: promoted or metadata-normalized assets.
- `imports/manifests/`: source and bundle manifests.
- `imports/reports/`: tracked intake, attribution, validation, and audit reports.
- `tests/`: unit, integration, snapshot, compat, and engine coverage.
- `tools/`: CI gates, docs checks, asset tooling, packaging, migration, RPG Maker utilities, and workflow scripts.
- `docs/`: architecture, status, release, governance, signoff, ADRs, asset intake, integrations, and template docs.
- `.urpg/`: ignored local cache/archive/state, including the local asset index.

The old root-level `third_party/` and `itch/` folders have been retired. Their ingested content now belongs under `imports/raw/third_party_assets/` and `imports/raw/itch_assets/`.

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

Asset and docs checks:

```powershell
python .\tools\assets\asset_db.py index
python .\tools\assets\report_third_party_itch_ingest.py
.\tools\ci\check_asset_library_governance.ps1
.\tools\ci\check_phase4_intake_governance.ps1
.\tools\docs\check_truth_alignment.ps1
```

## Documentation Map

- [Agent Knowledge Index](./docs/agent/INDEX.md)
- [Architecture Map](./docs/agent/ARCHITECTURE_MAP.md)
- [Quality Gates](./docs/agent/QUALITY_GATES.md)
- [Execution Workflow](./docs/agent/EXECUTION_WORKFLOW.md)
- [Known Debt](./docs/agent/KNOWN_DEBT.md)
- [Program Completion Status](./docs/status/PROGRAM_COMPLETION_STATUS.md)
- [Release Readiness Matrix](./docs/release/RELEASE_READINESS_MATRIX.md)
- [Editor Control Inventory](./docs/release/EDITOR_CONTROL_INVENTORY.md)
- [AI Copilot Guide](./docs/integrations/AI_COPILOT_GUIDE.md)
- [Asset Intake Registry](./docs/asset_intake/ASSET_SOURCE_REGISTRY.md)
- [Asset Promotion Guide](./docs/asset_intake/ASSET_PROMOTION_GUIDE.md)
- [Template Specs](./docs/templates/)

## Current Boundaries

- Public release is blocked by legal/privacy/distribution approval or waiver and release tagging.
- Raw/vendor/source packs are catalog and private-use intake until promotion is approved.
- Full platform signing, notarization, and public artifact policy remain backlog beyond the current internal validation checkpoint.
- Live cloud sync, marketplace publishing, payments, reviews, and production analytics upload are not shipped as public product surfaces.
- Heavy research/ML integrations remain offline tooling lanes under `tools/`; generated artifacts and metadata may be imported, but heavyweight runtime dependencies are not part of the shipped engine.

## Development Rules

- Keep claims truthful and evidence-backed.
- Prefer existing subsystem patterns over new abstractions.
- Update tests and docs when behavior changes.
- Keep raw intake separate from promoted release assets.
- Treat `imports/raw/` as quarantine/catalog input, not automatic game-shipping content.
- Use focused verification for narrow changes and broader gates for release-facing changes.
- Do not mark public release readiness until legal/public distribution exits are closed or formally waived.
