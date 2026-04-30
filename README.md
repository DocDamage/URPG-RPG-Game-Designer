# URPG Maker

URPG Maker is a native-first RPG engine and editor for building deterministic,
data-driven RPGs with production-minded authoring workflows. It combines a
C++20 runtime, an ImGui editor, a first-class grid-part Level Builder, OpenGL
and headless validation paths, bounded RPG Maker MZ compatibility tooling,
governed asset intake, package/export validation, and review-gated AI-assisted
project editing.

The product is aimed at creators who want RPG Maker-style speed without giving
up native runtime ownership, validation, diagnostics, packaging governance, and
source-control-friendly project data.

## Current Status

Status date: 2026-04-30

The `development` branch is suitable for internal and private release-candidate
validation. It is not yet public-release-ready. Public release remains blocked
by legal/privacy review, public distribution approval or waiver, and release
tagging.

Current headline state:

- Native Level Builder is now the top-level shippable map editor surface.
- `GridPartDocument` is the canonical editable map document for grid-part level
  authoring.
- Legacy spatial authoring is now supporting tooling under Level Builder for
  elevation, props, and ability-binding workflows.
- Grid-part unit and integration lanes are registered in CTest and passing in
  the active `build/dev-ninja-debug` tree.
- Export/package robustness, plugin compatibility robustness, runtime scene
  hardening, AI review workflows, template governance, asset intake, and release
  validation remain tracked through canonical docs and tests.

## Why Use URPG Maker

Use URPG Maker when you want an RPG-specific editor and runtime where validation
and shipping constraints are part of authoring, not an afterthought.

URPG Maker is a better fit than RPG Maker when you need:

- Native C++ runtime ownership instead of a primarily JavaScript/plugin-driven
  runtime.
- Deterministic tests, headless validation, CTest lanes, and CI-friendly project
  data.
- Explicit package governance, dependency manifests, provenance checks, and
  export readiness gates.
- A bounded RPG Maker MZ compatibility and migration harness that diagnoses what
  migrated and what did not, instead of pretending every plugin behavior is live
  runtime parity.

URPG Maker is a better fit than Unity, Unreal, or Godot when you want:

- RPG-first workflows out of the box rather than building editor tools,
  databases, event workflows, map validation, save inspection, and export
  governance from scratch.
- Smaller, deterministic native subsystems with repository-visible tests and
  docs rather than a general-purpose engine stack.
- Local-first authoring with structured JSON data and focused validation gates.

URPG Maker is a better fit than browser-only or no-code RPG tools when you need:

- Native runtime code, native packaging paths, and repository-controlled
  validation.
- Explicit editor snapshots and diagnostics for disabled/error/ready states.
- Governed raw asset intake and promotion workflows.
- AI-assisted editing that is review-gated and reversible instead of silent
  mutation.

URPG Maker is not the right choice if you want a finished public marketplace,
turnkey console certification, live cloud collaboration, or full RPG Maker
JavaScript plugin parity today. Those are either external release gates or
explicitly bounded compatibility/migration scopes.

## Product Pillars

### Native Level Builder

The Level Builder is the end-shippable map authoring surface. It is registered
as the top-level `level_builder` editor panel and owns the normal map-building
workflow.

Implemented Level Builder capabilities:

- Build, Validate, Playtest, Package, and Supporting Spatial workflow modes.
- `GridPartDocument` source-of-truth editing.
- Palette-driven part selection and deterministic grid placement.
- Inspector selection, property editing, command history, and diagnostics focus.
- Top-level undo/redo across placement and inspector histories.
- Save draft, load draft, and export-current-level commands.
- Deterministic JSON serialization through the grid-part authoring serializer.
- Load safeguards for malformed JSON, invalid documents, and map-id mismatch.
- Native commands for common level intent:
  - mark selected instance as player spawn
  - set selected instance as reach-exit objective
  - mark target export checks passed
  - mark accessibility checks passed
  - mark performance budget passed
  - mark human review passed
- Playtest from start, return to editor, runtime compile validation, and
  successful playtest promotion into readiness evidence.
- Package readiness summary for draft, playable, validated, publishable,
  exportable, and certified states.
- Actionable diagnostic rows with source, severity, code, message, target,
  instance id, part id, grid coordinate, and blocking flag.
- Diagnostic focus: instance-backed diagnostics select the offending part in
  the inspector.
- Supporting spatial mode for elevation, prop, ability, and composite spatial
  tools without making legacy spatial authoring the primary editor.

Core files:

- `editor/spatial/level_builder_workspace.*`
- `editor/spatial/grid_part_palette_panel.*`
- `editor/spatial/grid_part_placement_panel.*`
- `editor/spatial/grid_part_inspector_panel.*`
- `editor/spatial/grid_part_playtest_panel.*`
- `engine/core/map/grid_part_*`
- `tests/unit/test_grid_part_editor.cpp`

### Grid-Part Runtime And Data Model

The grid-part stack provides the authored map format and native runtime bridge
behind the Level Builder.

Implemented pieces:

- `GridPartCatalog` and `GridPartDefinition` for reusable part catalogs.
- Starter JRPG catalog: `content/part_catalogs/base_jrpg_parts.json`.
- Schemas:
  - `content/schemas/grid_part_catalog.schema.json`
  - `content/schemas/grid_part_authoring.schema.json`
  - `content/schemas/grid_part_runtime_state.schema.json`
- `GridPartDocument` for map id, dimensions, chunks, dirty tracking, placed
  parts, lock/hidden flags, properties, and footprint validation.
- Commands for place, remove, move, resize, replace, property edit, and bulk
  operations.
- Stamp placement for reusable part layouts.
- Ruleset profiles for top-down JRPG, dungeon room, side-scroller, tactical,
  world map, town hub, battle arena, and cutscene-stage constraints.
- Document, ruleset, objective, reachability, dependency, package-governance,
  serializer, runtime-compiler, and runtime-state validation.
- Dependency graph extraction for assets, tilesets, enemies, NPCs, dialogue,
  shops, loot, quests, quest items, cutscenes, timelines, abilities, audio,
  animations, scripts, and prefabs.
- Runtime compiler output for tile layers, spatial props, spawn tables, region
  rules, chunks, and stable compiled instance ids.
- Dirty/filtered chunk compile support and rejection of partial compile results
  when a full scene replacement is required.
- `MapScene` application for terrain/collision with size checks and tile-id
  clamping.
- Package governance for dependency manifests, license evidence, private asset
  checks, redistribution flags, duplicate rows, missing dependencies,
  unreferenced dependencies, package-id mismatch, and readiness levels.

CTest lanes:

```powershell
ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure
```

Registered tests:

- `urpg_grid_part_unit_lane`
- `urpg_grid_part_integration_lane`

### Native Runtime Core

URPG Maker includes deterministic native runtime systems for:

- Title/startup, scene stack, map, battle, menu, save/load, and settings flows.
- Ability, event, dialogue, quest, relationship, crafting, encounter, loot,
  NPC, character creation, achievement, mod-state, audio, accessibility,
  analytics consent, input, localization, and template data models.
- Save metadata, recovery diagnostics, migration paths, save/load preview lab,
  and policy schemas.
- Presentation runtime with OpenGL and headless validation paths.
- Runtime diagnostics for project health, assets, plugins, map rendering,
  battle state, ability state, package readiness, and export validation.

### Editor And WYSIWYG Surfaces

Editor behavior is expected to be visible, testable, and diagnostic-rich.

Current editor surfaces include:

- Top-level release panels governed by `engine/core/editor/editor_panel_registry.*`.
- Native Level Builder for map authoring.
- Diagnostics workspace for project audit, migration, compatibility, audio,
  abilities, events, menu/message/battle/save, project health, and export data.
- Asset library model/panel with filtering, source/status metadata,
  promote/archive action state, preview data, and intake reports.
- Ability inspector/sandbox/orchestration panels.
- Spatial supporting tools for elevation, props, ability bindings, and canvas
  conflict resolution.
- Battle, VFX, map environment, save/load lab, dialogue preview, visual novel
  pacing, templates, mod, analytics, and WYSIWYG coverage panels.
- Deterministic render snapshots for disabled, empty, error, and populated
  states.

### RPG Maker MZ Compatibility And Migration

The compatibility layer is bounded and diagnostic-first:

- QuickJS harness under `runtimes/compat_js/`.
- DataManager, BattleManager, Window, Sprite, Input, Audio, plugin fixture, and
  migration surfaces.
- Plugin manager support for command registration, execution, reload, failure
  diagnostics, manifest parsing, permissions, dependency gates, compatibility
  scoring, and native shim hints.

Compatibility is an import, validation, migration, and diagnostic harness. It is
not advertised as full live RPG Maker JavaScript runtime parity.

### Export, Packaging, And Governance

Export and package code is designed to make release blockers explicit:

- Creator package manifests and dependency validation.
- Patch manifest normalization and duplicate/malformed dependency diagnostics.
- Export validation reports with normalized errors.
- Export packager payload discovery with repository-root containment checks.
- Bundle writer and payload builder paths.
- Runtime bundle signature/integrity validation.
- Grid-part package governance and readiness levels.
- Package smoke and install smoke scripts under `tools/ci/`.

Public release remains blocked until legal/privacy and distribution exits are
closed or formally waived.

### AI-Assisted Editing

AI assistance is review-gated:

- `IChatService` abstraction and deterministic local `MockChatService`.
- OpenAI-compatible transport profiles for hosted and local providers.
- Provider selection, dry-run/live test state, streaming request metadata, and
  visible failure reasons.
- Reviewable plans before mutation.
- Persisted `_ai_change_history` with forward and reverse JSON patch data,
  before/after project data, and reverted state.
- Shared apply/revert controls across AI assistant and chatbot snapshots.
- Knowledge indexing for project files, docs, schemas, readiness reports,
  validation reports, asset catalogs, and template specs.

### Templates

Template governance exists for JRPG, visual novel, turn-based RPG, tactics RPG,
ARPG, monster collector, cozy/life, metroidvania-lite, 2.5D RPG, roguelite
dungeon, survival horror, farming adventure, card battler, platformer, gacha
hero, mystery detective, world exploration, space opera, soulslike-lite, school
life, rhythm RPG, racing adventure, post-apocalyptic RPG, pirate RPG,
cooking/restaurant RPG, city builder RPG, strategy kingdom RPG, sports team
RPG, tactical mecha RPG, tower defense RPG, faction politics RPG, and related
variants.

Template `READY` rows describe bounded starter-template evidence, not a claim
that every possible game in that genre is complete.

### Asset Intake

Asset handling is intentionally governed:

- Raw intake: `imports/raw/`
- Normalized assets: `imports/normalized/`
- Source manifests: `imports/manifests/asset_sources/`
- Bundle manifests: `imports/manifests/asset_bundles/`
- Reports: `imports/reports/asset_intake/`
- Local ignored asset DB: `.urpg/asset-index/`

Raw assets are quarantine/catalog inputs. They are not release-export eligible
until curated subsets receive attribution, bundle manifests, and promotion
approval.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `apps/` | Runtime and editor entry points. |
| `engine/core/` | Native runtime systems and data models. |
| `engine/api/` | Public runtime API entry points. |
| `engine/runtimes/bridge/` | Native/script value bridge support. |
| `editor/` | ImGui panels, editor models, diagnostics, and workspaces. |
| `runtimes/compat_js/` | QuickJS and RPG Maker MZ compatibility surfaces. |
| `content/` | Schemas, fixtures, templates, abilities, catalogs, readiness data. |
| `resources/` | Release-required app resources. |
| `imports/raw/` | Quarantined raw local, third-party, itch, RPG Maker, and tool intake. |
| `imports/normalized/` | Promoted or metadata-normalized assets. |
| `imports/manifests/` | Source and bundle manifests. |
| `imports/reports/` | Intake, attribution, validation, and audit reports. |
| `tests/` | Unit, integration, snapshot, compat, and engine tests. |
| `tools/` | CI, docs, assets, packaging, migration, and workflow scripts. |
| `docs/` | Architecture, release, governance, signoff, ADRs, status, and templates. |
| `.urpg/` | Ignored local cache/archive/state. |

The old root-level `third_party/` and `itch/` folders are retired. Ingested
content belongs under `imports/raw/third_party_assets/` and
`imports/raw/itch_assets/`.

## Build

Recommended Windows/Ninja build:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
```

Other local presets:

```powershell
cmake --preset dev-vs2022
cmake --build --preset dev-vs2022-debug

cmake --preset dev-mingw-debug
cmake --build --preset dev-mingw-debug-build
```

## Test And Validate

Run all registered tests:

```powershell
ctest --preset dev-all --output-on-failure
```

Common focused gates:

```powershell
ctest --preset dev-all -L pr --output-on-failure
ctest --preset dev-all -L nightly --output-on-failure
ctest --preset dev-all -L weekly --output-on-failure
```

Level Builder / grid-part lane:

```powershell
ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure
```

Focused executable coverage used during Level Builder work:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[grid_part][editor]"
.\build\dev-ninja-debug\urpg_tests.exe "[editor][panel][registry]"
.\build\dev-ninja-debug\urpg_tests.exe "[dungeon3d][wysiwyg]"
```

Presentation and local release gates:

```powershell
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
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build\dev-ninja-debug
```

## Documentation Map

- [Agent Knowledge Index](./docs/agent/INDEX.md)
- [Architecture Map](./docs/agent/ARCHITECTURE_MAP.md)
- [Quality Gates](./docs/agent/QUALITY_GATES.md)
- [Execution Workflow](./docs/agent/EXECUTION_WORKFLOW.md)
- [Known Debt](./docs/agent/KNOWN_DEBT.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Status Mirror](./docs/status/PROGRAM_COMPLETION_STATUS.md)
- [App Release Readiness Matrix](./docs/APP_RELEASE_READINESS_MATRIX.md)
- [Release Readiness Matrix](./docs/release/RELEASE_READINESS_MATRIX.md)
- [Editor Control Inventory](./docs/release/EDITOR_CONTROL_INVENTORY.md)
- [Release Packaging](./docs/release/RELEASE_PACKAGING.md)
- [AI Copilot Guide](./docs/integrations/AI_COPILOT_GUIDE.md)
- [Asset Intake Registry](./docs/asset_intake/ASSET_SOURCE_REGISTRY.md)
- [Asset Promotion Guide](./docs/asset_intake/ASSET_PROMOTION_GUIDE.md)
- [Schema Changelog](./docs/SCHEMA_CHANGELOG.md)
- [Governance Schema Changelog](./docs/governance/SCHEMA_CHANGELOG.md)
- [Template Specs](./docs/templates/)

## Current Boundaries

- Public release is blocked by legal/privacy/distribution approval or waiver and
  release tagging.
- Raw/vendor/source packs are catalog/private-use intake until promotion is
  approved.
- Full platform signing, notarization, public artifact policy, live cloud sync,
  marketplace publishing, payments, reviews, and production analytics upload are
  not shipped public product surfaces.
- Heavy research and ML integrations remain offline tooling lanes under
  `tools/`; heavyweight runtime ML dependencies are not part of the shipped
  engine.
- RPG Maker compatibility remains bounded. Unsupported source data should be
  preserved and diagnosed rather than silently treated as fully migrated.

## Development Rules

- Keep claims truthful and evidence-backed.
- Prefer existing subsystem patterns over new abstractions.
- Update tests and docs when behavior changes.
- Update schema changelogs when schemas change.
- Keep raw intake separate from promoted release assets.
- Treat `imports/raw/` as quarantine/catalog input, not automatic game-shipping
  content.
- Use focused verification for narrow changes and broader gates for
  release-facing changes.
- Do not mark public release readiness until legal/public distribution exits are
  closed or formally waived.
