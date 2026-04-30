# URPG Maker

URPG Maker is a native-first RPG engine and editor for building deterministic,
data-driven RPGs with a serious authoring workflow. It combines a C++20 runtime,
an ImGui editor, a bounded RPG Maker MZ compatibility layer, OpenGL and headless
rendering paths, governed asset intake, export validation, package governance,
AI-assisted project editing, and a growing grid-part level builder.

The product goal is a real WYSIWYG game maker, not only a runtime library.
Features are expected to have saved project data, visible editor controls, live
preview or headless verification, runtime execution, diagnostics, tests, and
truthful readiness documentation.

## Status

Status date: 2026-04-30

The `development` branch is suitable for internal and private release-candidate
validation. It is not yet public-release-ready. Public release remains blocked by
legal, privacy, distribution approval or waiver, final release decision recording,
and release tagging.

Current headline work:

- Grid-part level authoring from catalog to document, editor controls, validation,
  runtime compile, map-scene application, serialization, package governance, and
  CTest-registered lanes.
- Export and package robustness for creator manifests, patch manifests, export
  validation reports, payload discovery, dependency manifests, and package
  readiness checks.
- Compatibility/plugin robustness for manifest normalization, dependency
  de-duplication, permission filtering, compatibility scoring, and plugin manager
  dependency gates.
- Scene/runtime robustness for map construction, tilemap rendering, partial
  compile rejection, tile ID clamping, and invalid texture/layer handling.
- Presentation, schema, WYSIWYG, AI, asset-intake, and release governance
  surfaces continue to be tracked through canonical docs and tests.

## What URPG Maker Is Optimized For

URPG Maker is built for teams or solo creators who want RPG Maker-style
productivity with stronger runtime ownership, validation, and content governance.

Use it when you want:

- RPG-first data models instead of a general-purpose engine where the RPG toolset
  must be built from scratch.
- Native C++ runtime systems with deterministic test and headless execution.
- Editor workflows that expose diagnostics, previews, schemas, package readiness,
  and export validation as first-class product surfaces.
- Bounded RPG Maker MZ migration and compatibility tooling without pretending to
  be a full live RPG Maker JavaScript runtime clone.
- Local-first authoring, AI-assisted edits with review/revert state, and governed
  raw asset intake before anything is eligible for release export.

## Major Feature Areas

### Native Runtime Core

- C++20 runtime kernels for deterministic game systems.
- Title, menu, map, battle, startup-adjacent, save/load, and scene-stack flows.
- Ability, battle, event, dialogue, quest, relationship, crafting, encounter,
  loot, NPC schedule, character creation, achievement, mod-state, audio-mix,
  accessibility, analytics-consent, input, settings, and template runtime models.
- Save metadata, recovery diagnostics, save migration paths, save/load preview
  lab data, and explicit policy schemas.
- Presentation runtime and render-frame intent paths with OpenGL and headless
  validation coverage.
- Runtime diagnostics for project health, asset placeholders, plugin failures,
  map rendering, battle state, ability state, and export/package surfaces.

### Grid-Part Level Builder

The grid-part level builder is the newest large authoring surface. It provides a
part-based level construction workflow on top of the native map/runtime stack.

Implemented pieces include:

- `GridPartCatalog` and `GridPartDefinition` for curated reusable part catalogs.
- Base JRPG part catalog at `content/part_catalogs/base_jrpg_parts.json`.
- JSON schemas for catalog, authoring, and runtime state:
  - `content/schemas/grid_part_catalog.schema.json`
  - `content/schemas/grid_part_authoring.schema.json`
  - `content/schemas/grid_part_runtime_state.schema.json`
- `GridPartDocument` for authored map-level part placement, dimensions, chunks,
  dirty tracking, part locking, and footprint validation.
- Commands and undo/redo support for place, remove, move, resize, replace,
  property edits, and bulk operations.
- Stamp placement for reusable part layouts.
- Ruleset profiles for RPG map styles such as JRPG, dungeon room, and
  side-scroller-inspired constraints.
- Document validation for bounds, catalog mismatches, collision overlaps,
  unsupported rulesets, and category/layer errors.
- Objective validation for map goals and completion signals.
- Reachability analysis for spawn-to-goal paths, locked doors, keys, obstacles,
  and blocked objectives.
- Dependency graph extraction for assets, tilesets, enemies, NPCs, dialogue,
  shop tables, loot tables, quests, quest items, cutscenes, timelines, abilities,
  audio, animations, scripts, and prefabs.
- Runtime compiler output for:
  - `TileLayerDocument`
  - spatial props
  - spawn tables
  - region rules
  - compiled chunks
  - compiled instance IDs
- Full compile, filtered chunk compile, and dirty chunk compile.
- Partial compile rejection when applying a chunk result as a full `MapScene`
  replacement.
- MapScene application for terrain and collision, including tile ID clamping and
  scene-size checks.
- Deterministic JSON serializer and loader with schema versioning.
- Package governance with dependency manifests, license/provenance checks,
  readiness evidence, publishable/exportable/certified levels, duplicate
  detection, unreferenced dependency diagnostics, and package ID mismatch checks.
- Editor-facing panels:
  - grid part palette
  - placement panel
  - inspector panel
  - playtest panel
  - spatial workspace integration
- Focused unit and integration test lanes registered in CTest:
  - `urpg_grid_part_unit_lane`
  - `urpg_grid_part_integration_lane`

### Editor And WYSIWYG Workflows

- ImGui editor shell with governed panel registration.
- Diagnostics workspace for project audit, migration, compatibility, audio,
  abilities, events, menu/message/battle/save, project health, and export
  surfaces.
- Spatial authoring workspace with prop placement, map ability bindings, grid
  part mode, grid part palette, placement, inspector, and playtest flows.
- WYSIWYG-oriented panels and models for:
  - ability sandboxing
  - battle VFX timelines
  - event command graphs
  - dialogue preview
  - export preview
  - save/load lab
  - map environment preview
  - 3D dungeon/world authoring
  - visual novel pacing
  - template workflows
  - package/readiness diagnostics
- Snapshot coverage for disabled, empty, error, and populated editor states.

### RPG Maker MZ Compatibility And Migration

- QuickJS-based compatibility layer under `runtimes/compat_js/`.
- Bounded DataManager, BattleManager, Window, Sprite, Input, Audio, plugin
  fixture, and migration surfaces.
- Plugin manager support for fixture-backed and live JavaScript plugin command
  registration, execution, reload, failure diagnostics, and dependency gates.
- Plugin manifest parsing with normalized dependencies and permissions.
- Plugin compatibility scoring for missing dependencies, permissions,
  unsupported APIs, fixture-only behavior, fallback paths, failure diagnostics,
  cycles, override conflicts, and native shim hints.
- Compatibility reporting is non-authoritative by default and intended for
  migration triage.

URPG Maker compatibility is an import, validation, migration, and diagnostic
harness. It should not be advertised as full live RPG Maker JavaScript runtime
parity.

### Export, Packaging, And Governance

- Creator package manifests with required metadata, dependency validation,
  duplicate detection, normalized JSON output, and source governance.
- Patch manifests with normalized changed data, changed assets, dependencies,
  malformed-array rejection, blank dependency rejection, and duplicate detection.
- Export validation report generation with deterministic normalized errors.
- Export packager payload discovery with repository-bound relative roots and
  explicit errors for escaping discovery roots.
- Bundle writer and payload builder paths for packaging workflows.
- Grid-part package governance for dependency manifests, readiness levels,
  provenance/license evidence, redistribution flags, private-use checks, package
  ID mismatches, unreferenced dependencies, and duplicate manifest rows.
- Release packaging docs under `docs/release/`.

### AI-Assisted Project Editing

- `IChatService` abstraction and deterministic in-tree `MockChatService`.
- Creator-command transport profiles for OpenAI-compatible hosted and local
  providers, including OpenAI/ChatGPT-style endpoints, Kimi, OpenRouter, Ollama,
  and LM Studio.
- Editor-visible provider selection, dry-run/live test state, and failure
  reasons.
- Review-gated AI tool plans before mutation.
- AI-applied project changes persist `_ai_change_history` records with forward
  and reverse JSON patch data, before/after project data, and reverted state.
- Tool and knowledge indexing for project metadata, canonical docs, reports,
  schemas, readiness data, template specs, and subsystem-specific tool
  candidates.

The deterministic local behavior is the shipped in-tree baseline. Live provider
productization remains a release-governed surface.

### Templates

URPG Maker tracks starter/template governance for JRPG, visual novel, turn-based
RPG, tactics RPG, ARPG, monster collector, cozy/life, metroidvania-lite,
2.5D RPG, roguelite dungeon, survival horror, farming adventure, card battler,
platformer, gacha hero, mystery detective, world exploration, space opera,
soulslike-lite, school life, rhythm RPG, racing adventure, post-apocalyptic RPG,
pirate RPG, cooking/restaurant RPG, city builder RPG, strategy kingdom RPG,
sports team RPG, tactical mecha RPG, tower defense RPG, faction politics RPG,
and related variants.

Template support includes runtime profiles, starter manifests, specs,
certification loops, readiness rows, and WYSIWYG showcase bindings. A `READY`
row means the bounded claimed template scope has evidence; it does not mean every
possible game in that genre is complete.

### Asset Intake

Asset handling is intentionally governed:

- Raw intake lives under `imports/raw/`.
- Normalized promoted assets live under `imports/normalized/`.
- Source manifests live under `imports/manifests/asset_sources/`.
- Bundle manifests live under `imports/manifests/asset_bundles/`.
- Intake and audit reports live under `imports/reports/asset_intake/`.
- The local asset DB is ignored under `.urpg/asset-index/`.

Current cataloged intake includes:

- `SRC-007` local URPG asset drop, deduped to 56,096 non-audio/tool catalog
  records with zero remaining exact duplicate groups in the promotion catalog.
- Existing OGG audio context preserved under raw intake; tracked MP3/WAV files
  are intentionally excluded from Git.
- Third-party and itch roots indexed from `imports/raw/third_party_assets/` and
  `imports/raw/itch_assets/`.
- Generator/tool candidates for sprite generation, pixel planets, space
  backgrounds, and Tiled/TMX workflows.

Cataloged raw assets are not release-export eligible until curated subsets
receive attribution, bundle manifests, and promotion approval.

### Robustness And Validation Culture

Recent robustness work hardened:

- Grid-part chunk filters, dirty chunk pruning, serializer rejection, locked-part
  mutation rules, partial compile application, and tile ID bounds.
- Export/package dependency normalization, duplicate reporting, unreferenced
  manifest diagnostics, private-use redistribution checks, and escaping path
  rejection.
- Plugin manifest and compatibility manifest dependency/permission
  normalization.
- Plugin manager dependency gates for blank and duplicate dependency metadata.
- MapScene invalid dimensions, empty collision-safe maps, tilemap renderer layer
  bounds, invalid texture dimensions, and zero tile-column math.

Validation is treated as a product feature. New behavior should come with
focused tests, CTest registration where appropriate, and documentation updates
when claims or schemas change.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `apps/` | Runtime and editor entry points. |
| `engine/core/` | Native runtime systems and data models. |
| `engine/api/` | Public runtime API entry points. |
| `engine/runtimes/bridge/` | Native/script value bridge support. |
| `editor/` | ImGui panels, editor models, diagnostics, and workspaces. |
| `runtimes/compat_js/` | QuickJS and RPG Maker MZ compatibility surfaces. |
| `content/` | Schemas, fixtures, templates, abilities, part catalogs, readiness data, and release-required content. |
| `resources/` | Release-required app resources. |
| `imports/raw/` | Quarantined raw local, third-party, itch, RPG Maker, and generator/tool source intake. |
| `imports/normalized/` | Promoted or metadata-normalized assets. |
| `imports/manifests/` | Source and bundle manifests. |
| `imports/reports/` | Tracked intake, attribution, validation, and audit reports. |
| `tests/` | Unit, integration, snapshot, compat, and engine coverage. |
| `tools/` | CI gates, docs checks, asset tooling, packaging, migration, RPG Maker utilities, and workflow scripts. |
| `docs/` | Architecture, status, release, governance, signoff, ADRs, asset intake, integrations, and template docs. |
| `.urpg/` | Ignored local cache/archive/state, including the local asset index. |

The old root-level `third_party/` and `itch/` folders have been retired. Their
ingested content belongs under `imports/raw/third_party_assets/` and
`imports/raw/itch_assets/`.

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
```

Grid-part lanes:

```powershell
ctest --test-dir build\dev-ninja-debug -R "grid_part" --output-on-failure
ctest --test-dir build-local -C Debug -R "grid_part" --output-on-failure
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
```

## Documentation Map

- [Agent Knowledge Index](./docs/agent/INDEX.md)
- [Architecture Map](./docs/agent/ARCHITECTURE_MAP.md)
- [Quality Gates](./docs/agent/QUALITY_GATES.md)
- [Execution Workflow](./docs/agent/EXECUTION_WORKFLOW.md)
- [Known Debt](./docs/agent/KNOWN_DEBT.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [Status Program Completion Mirror](./docs/status/PROGRAM_COMPLETION_STATUS.md)
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
- Raw/vendor/source packs are catalog and private-use intake until promotion is
  approved.
- Full platform signing, notarization, and public artifact policy remain backlog
  beyond the current internal validation checkpoint.
- Live cloud sync, marketplace publishing, payments, reviews, and production
  analytics upload are not shipped as public product surfaces.
- Heavy research and ML integrations remain offline tooling lanes under `tools/`.
  Generated artifacts and metadata may be imported, but heavyweight runtime
  dependencies are not part of the shipped engine.
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
