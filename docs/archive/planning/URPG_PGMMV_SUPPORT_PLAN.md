# URPG Pixel Game Maker MV Support Plan

Date: 2026-04-18
Status: detailed planning annex / historical execution-detail input
Scope: add Pixel Game Maker MV (PGMMV) project support to URPG as a native import, migration, diagnostics, and editor workflow lane

Canonical governance and current execution authority live in:
- `../../PROGRAM_COMPLETION_STATUS.md`
- `../../NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `../../PROGRAM_COMPLETION_STATUS.md`
- `URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`

Use this file as a detailed historical PGMMV planning annex, not as a parallel status or release-readiness authority. Its deltas only mattered once they were absorbed into the documents above.

## 1. Executive summary

This plan proposed adding **Pixel Game Maker MV** support to URPG without compromising URPG’s native-first direction.

The support model is **not** full PGMMV runtime emulation and **not** a forced extension of the existing RPG Maker MZ JavaScript compatibility lane. Instead, PGMMV becomes a **separate source-engine intake path** that:

1. detects and reads PGMMV projects,
2. normalizes source data into a URPG-controlled import model,
3. migrates supported content into native URPG schemas and runtime owners,
4. emits structured diagnostics for approximated or unsupported constructs,
5. surfaces the results in the editor’s migration/import workflow.

This approach preserves the repo’s stated strategy:

- native systems own long-term product capability,
- compat/import lanes exist to verify, migrate, and contain risk,
- extension/plugin behavior must not become the authority for core systems,
- truthfulness matters more than inflated compatibility claims.

The end goal is:

> A PGMMV game can be brought into URPG through a trustworthy migration path, with clear reporting about what was preserved, approximated, or rejected, and the imported project can continue life as a native URPG project.

---

## 2. Why this belongs in URPG

PGMMV support is strategically aligned with URPG for five reasons:

### 2.1 URPG already positions itself as a multi-genre native engine
PGMMV projects commonly lean toward action/platforming and hybrid 2D gameplay structures. URPG already has roadmap direction for advanced native capability lanes such as:

- gameplay abilities,
- sprite pipeline tooling,
- modular level assembly,
- procedural toolkit,
- timeline/animation orchestration,
- 2.5D presentation,
- editor productivity tooling.

Those systems are a better long-term home for PGMMV project behavior than any source-engine-emulation layer.

### 2.2 PGMMV import broadens URPG’s intake funnel
URPG should not be locked to RPG Maker MZ only. Supporting PGMMV turns URPG into a broader “universal intake” engine instead of a single-source migration tool.

### 2.3 Native ownership remains intact
The imported result becomes a URPG-native project, not a brittle PGMMV compatibility shell.

### 2.4 It creates pressure to harden action-oriented runtime lanes
Adding PGMMV support forces URPG to clarify and strengthen:

- action-state runtime ownership,
- stage/level representation,
- collision and trigger schemas,
- sprite and animation import tooling,
- migration diagnostics.

### 2.5 It fits the repo’s migration wizard and diagnostics direction
URPG already leans on structured migration, diagnostics artifacts, and editor-facing reporting. PGMMV should plug into that same pattern rather than inventing a special-case flow.

---

## 3. Product goal

Add a new source-engine path:

- `SourceEngine::PixelGameMakerMV`

URPG should be able to:

1. recognize a PGMMV project,
2. inspect the project and summarize importability,
3. import supported assets and gameplay structures,
4. convert them into URPG-native project content,
5. present a migration report with explicit categories:
   - **Preserved**
   - **Approximated**
   - **Unsupported**
6. let the user continue editing and shipping the project as a URPG game.

---

## 4. Non-goals

These are explicitly out of scope for the first delivery waves.

### 4.1 Full runtime emulation of PGMMV
URPG should not attempt to become a binary/runtime clone of PGMMV.

### 4.2 Perfect behavioral parity for every edge case
Exact parity for obscure timing, editor quirks, or source-engine-specific bugs is not required.

### 4.3 Blind “100% compatible” marketing claims
Support must remain evidence-based.

### 4.4 Permanent dependence on source-engine concepts
Imported projects should graduate into native URPG structures rather than remain permanently coupled to PGMMV vocabulary or storage layouts.

### 4.5 Importing unlicensed third-party sample corpora indiscriminately
All fixture corpora must be license-safe and intentionally curated.

---

## 5. Operating principles

### 5.1 Native-first, import-second
Import lanes exist to help users enter URPG. They do not define the engine’s authority.

### 5.2 Normalize first, convert second
Never map raw PGMMV data directly into many downstream URPG systems in one step. Create a normalized import model first.

### 5.3 Truthful migration reporting
Every imported element must land in one of:

- preserved,
- approximated,
- unsupported.

### 5.4 Deterministic import behavior
The same source project should produce the same normalized model, migration report, and native output.

### 5.5 Safe fallback over silent corruption
When a source feature cannot be preserved, URPG should reject or downgrade it with diagnostics rather than silently invent broken behavior.

### 5.6 Editor visibility is mandatory
A headless importer alone is not enough. Users need UI surfaces to inspect the migration result.

---

## 6. High-level architecture

PGMMV support should be added as a **parallel import lane** with distinct phases.

```text
PGMMV Project
  -> project detector
  -> source reader
  -> normalized import model
  -> migration analyzer
  -> native URPG schema emitters
  -> diagnostics artifacts
  -> editor migration wizard/report panels
```

### 6.1 Major layers

#### Layer A — Detection and source reading
Responsible for:

- identifying project roots,
- validating expected file/folder structure,
- reading source metadata,
- enumerating levels, assets, events, actors, and project settings.

#### Layer B — Normalized import model
Responsible for:

- holding engine-agnostic project semantics,
- representing importable gameplay in a stable intermediate structure,
- decoupling source format specifics from URPG runtime/schema emitters.

#### Layer C — Migration analyzers and mappers
Responsible for:

- determining what can be preserved,
- converting source semantics into URPG-native equivalents,
- assigning approximation paths,
- generating structured diagnostics.

#### Layer D — Native schema emitters
Responsible for:

- producing URPG project content,
- materializing assets and references,
- generating editor-usable native data.

#### Layer E — Editor workflow integration
Responsible for:

- previewing the import,
- presenting migration diagnostics,
- allowing selective import decisions,
- launching post-import repair workflows.

---

## 7. The correct support model

There are three possible ways to add PGMMV support:

### 7.1 Wrong model: extend MZ compat lane to pretend PGMMV is similar
This is structurally wrong. The MZ lane is JS/plugin compatibility-oriented; PGMMV is a different engine model and needs a distinct intake path.

### 7.2 Wrong model: implement a PGMMV runtime clone inside URPG
This is too expensive, too brittle, and misaligned with URPG’s native-first direction.

### 7.3 Correct model: import + migrate + native ownership
This is the recommended path.

The imported game should become a URPG-native project, with explicit migration evidence and editor visibility.

---

## 8. Proposed subsystem additions

## 8.1 Source engine registry

Add a new source-engine identifier and intake registration path.

### Proposed additions

- `SourceEngine::PixelGameMakerMV`
- import source registration entry in migration/import orchestration
- PGMMV capability profile descriptor

### Responsibilities

- project detection
- source engine labeling
- import capability reporting
- migration wizard routing

---

## 8.2 PGMMV project detector

### Goal
Recognize whether a folder is a valid or likely PGMMV project root.

### Responsibilities

- validate expected PGMMV folder/file patterns,
- detect project manifest/settings files,
- identify stage/map data roots,
- identify asset directories,
- produce a confidence score:
  - confirmed
  - likely
  - invalid

### Deliverables

- `pgmmv_project_detector.h/.cpp`
- unit tests for valid/invalid/partial project structures

### Acceptance criteria

- correctly distinguishes PGMMV roots from random folders,
- produces deterministic results,
- reports missing critical files clearly.

---

## 8.3 PGMMV source reader

### Goal
Read raw project content from disk into source-shaped structures.

### Responsibilities

- read project metadata,
- enumerate levels/stages/scenes,
- read object placements,
- read collision or tile metadata,
- read actor/enemy/player definitions where available,
- read animations and sprite references,
- read audio assignments,
- read transition or scene flow metadata.

### Deliverables

- `pgmmv_project_reader.h/.cpp`
- source data structs
- file format parsers
- fixture-backed unit tests

### Acceptance criteria

- can ingest a minimal sample project end to end,
- fails clearly on malformed data,
- never silently discards structural failures.

---

## 8.4 Normalized import model

This is the most important structural piece.

### Goal
Represent imported PGMMV project semantics in a stable URPG-owned intermediate form.

### Proposed core entities

- `ImportedProject`
- `ImportedStage`
- `ImportedLayer`
- `ImportedTileLayer`
- `ImportedObjectLayer`
- `ImportedActor`
- `ImportedPlayerControllerConfig`
- `ImportedEnemyControllerConfig`
- `ImportedAnimationClip`
- `ImportedSpriteSheet`
- `ImportedCollisionRegion`
- `ImportedHazard`
- `ImportedPickup`
- `ImportedTrigger`
- `ImportedSpawnPoint`
- `ImportedCheckpoint`
- `ImportedSceneTransition`
- `ImportedAudioCue`
- `ImportedUiHint`
- `ImportIssue`
- `ImportEvidence`

### Required properties

The model should capture:

- source IDs and names,
- stable imported IDs,
- source location references,
- asset references,
- semantic tags,
- import confidence,
- migration classification,
- diagnostics provenance.

### Acceptance criteria

- model is source-engine-neutral enough to survive future expansion,
- model is rich enough to preserve PGMMV semantics without forcing early loss,
- model is deterministic and serializable for debug snapshots.

---

## 8.5 Migration analyzer

### Goal
Decide what the importer can preserve, approximate, or reject before emitting native content.

### Required output categories

- **Preserved**: maps cleanly to native URPG behavior.
- **Approximated**: maps to a close but not exact URPG behavior.
- **Unsupported**: no safe native mapping yet.

### Responsibilities

- evaluate stage structures,
- evaluate actor/controller behaviors,
- evaluate triggers and interactions,
- evaluate physics/collision assumptions,
- evaluate animation and sprite compatibility,
- assign a migration severity and recommendation,
- emit structured issues.

### Deliverables

- `pgmmv_migration_analyzer.h/.cpp`
- migration rules registry
- JSONL export support for issues and decisions

### Acceptance criteria

- every imported feature receives an explicit disposition,
- no unsupported construct is silently treated as preserved,
- diagnostics include source location and reason.

---

## 8.6 Native emitters

### Goal
Convert the normalized model into native URPG schemas and project structures.

### Target lanes

#### A. Level / stage lane
Map PGMMV stage content into native URPG level and scene ownership.

#### B. Collision / trigger lane
Map collision regions, damage zones, and interaction triggers.

#### C. Sprite / animation lane
Map sprite sheets, frame data, animation timelines, and animation state relationships.

#### D. Entity / controller lane
Map players, enemies, pickups, hazards, checkpoints, and scene objects.

#### E. Audio lane
Map BGM, ambient, and SFX references.

#### F. UI/HUD lane
Map only simple importable HUD metadata in early versions; more complex HUD behavior should be approximated or flagged.

### Deliverables

- `pgmmv_native_emitter.h/.cpp`
- per-domain mappers
- native schema output tests

### Acceptance criteria

- imported project can open as a URPG-native project,
- key references are intact,
- unsupported constructs are reported, not hidden.

---

## 8.7 Editor integration

### Goal
Make PGMMV import usable by normal URPG users.

### Required surfaces

#### A. Import source selector
Add PGMMV as an import/migration option.

#### B. Preflight summary panel
Show:

- project identity,
- source-engine detection result,
- asset counts,
- stage counts,
- high-risk areas,
- import readiness score.

#### C. Migration report panel
Show:

- preserved items,
- approximated items,
- unsupported items,
- per-category totals,
- filters by severity and subsystem.

#### D. Selective import controls
Allow selective acceptance where safe, such as:

- import assets now, gameplay later,
- skip unsupported HUD conversion,
- import only selected stages.

#### E. Post-import repair workflow hooks
Offer follow-up repair views for:

- missing animation bindings,
- trigger rewiring,
- collision review,
- controller tuning.

### Deliverables

- `PGMMVImportWizardPanel`
- `PGMMVImportReportPanel`
- integration into existing migration wizard orchestration

### Acceptance criteria

- a user can import a sample PGMMV project without using CLI-only tooling,
- import decisions are visible and reviewable,
- editor surfaces do not overstate parity.

---

## 9. First-version import scope

The first useful version should be intentionally constrained.

## 9.1 Must-import in v1

### Project metadata
- project name
- author/publisher if present
- default resolution or stage metrics if present
- core source-engine identity metadata

### Stage structure
- stages/scenes/maps
- stage dimensions
- stage backgrounds/parallax layers
- tile/layer references
- spawn points
- transition points

### Asset references
- sprite sheets
- tile sets / tile atlases
- music
- sound effects
- static UI references where trivial

### Basic gameplay structures
- player start positions
- enemy placements
- pickups/collectibles
- hazards/damage zones
- simple triggers
- checkpoints
- simple moving-object definitions where the mapping is safe

### Collision / interaction
- collision regions
- one-way surfaces if confidently detectable
- noninteractive decoration layers
- basic trigger boxes

### Animation
- frame sequences
- directional/variant relationships where available
- loop / non-loop flags where available

---

## 9.2 Should-import in v2

- moving platforms,
- scripted stage events,
- ladders,
- one-way platform nuances,
- enemy patrol/attack state mapping,
- camera regions,
- boss encounter setup,
- checkpoint/save semantics,
- more advanced HUD reconstruction.

---

## 9.3 Defer until later

- obscure source-engine-specific editor quirks,
- exact source timing bug compatibility,
- complex custom scripting equivalents,
- anything that requires URPG to become a runtime clone rather than a migration target.

---

## 10. Required native runtime pressure points

PGMMV support will expose weak spots. This is good, because it makes the next layers of URPG more honest and usable.

## 10.1 Action runtime ownership
URPG needs a clean native action-oriented entity/controller lane for platformer and action game semantics.

### Required capabilities
- deterministic update order,
- player movement state ownership,
- enemy controller state ownership,
- hazard/pickup interaction processing,
- trigger dispatch,
- stage transition execution.

## 10.2 Level representation
URPG needs stage/scene data structures that can express action-platform layouts cleanly.

### Required capabilities
- layered stage content,
- tile and object layers,
- collision overlays,
- stage metadata,
- camera/viewport rules,
- transition anchors.

## 10.3 Collision and trigger schemas
URPG needs native schemas for:

- collision volumes,
- trigger regions,
- hazards,
- pickups,
- checkpoints,
- movement constraints.

## 10.4 Sprite pipeline maturity
URPG needs stronger import handling for:

- atlas unpack/repack,
- frame extraction,
- trim/crop metadata,
- animation preview and tuning,
- runtime-facing animation metadata generation.

## 10.5 Diagnostics maturity
Import support will fail badly without:

- structured diagnostics,
- reproducible migration reports,
- clear unsupported-feature surfacing.

---

## 11. Proposed repository layout

This is a suggested file/folder shape, not a rigid mandate.

```text
engine/
  import/
    common/
      import_issue.h
      import_evidence.h
      imported_project.h
    pgmmv/
      pgmmv_project_detector.h
      pgmmv_project_detector.cpp
      pgmmv_project_reader.h
      pgmmv_project_reader.cpp
      pgmmv_import_model.h
      pgmmv_import_model.cpp
      pgmmv_migration_analyzer.h
      pgmmv_migration_analyzer.cpp
      pgmmv_native_emitter.h
      pgmmv_native_emitter.cpp
      pgmmv_asset_mapper.h
      pgmmv_asset_mapper.cpp
      pgmmv_stage_mapper.h
      pgmmv_stage_mapper.cpp
      pgmmv_entity_mapper.h
      pgmmv_entity_mapper.cpp
      pgmmv_animation_mapper.h
      pgmmv_animation_mapper.cpp
      pgmmv_diagnostics_export.h
      pgmmv_diagnostics_export.cpp

editor/
  import/
    pgmmv_import_wizard_panel.h
    pgmmv_import_wizard_panel.cpp
    pgmmv_import_report_panel.h
    pgmmv_import_report_panel.cpp
    pgmmv_import_preflight_model.h
    pgmmv_import_preflight_model.cpp

content/
  schemas/
    import/
      pgmmv_import_report.schema.json
      pgmmv_import_evidence.schema.json

tools/
  import/
    pgmmv/
      inspect_pgmmv_project.cpp or .py
      export_pgmmv_import_report.cpp or .py

tests/
  import/
    test_pgmmv_project_detector.cpp
    test_pgmmv_project_reader.cpp
    test_pgmmv_import_model.cpp
    test_pgmmv_migration_analyzer.cpp
    test_pgmmv_stage_mapping.cpp
    test_pgmmv_animation_mapping.cpp
    test_pgmmv_native_emitter.cpp
    test_pgmmv_diagnostics.cpp

third_party/
  fixtures/
    pgmmv/
      minimal_platformer/
      minimal_action/
      unsupported_edge_cases/
```

---

## 12. Execution roadmap

## Milestone 0 — Discovery and intake rules

### Goal
Define what a PGMMV project looks like and what support means.

### Tasks
1. Confirm PGMMV project root signatures and required files.
2. Define a license-safe fixture acquisition strategy.
3. Build a source format inventory:
   - project settings
   - stage files
   - asset folders
   - object/event data
   - animation metadata
4. Define v1 support boundaries.
5. Define migration classification rules.

### Deliverables
- format notes doc,
- fixture strategy doc,
- v1 support matrix,
- risk register.

### Exit criteria
- team has a stable, documented interpretation of PGMMV intake.

---

## Milestone 1 — Detection and preflight

### Goal
Determine whether a folder is a PGMMV project and summarize what is importable.

### Tasks
1. Implement `pgmmv_project_detector`.
2. Implement source root validation.
3. Build a preflight summary object.
4. Add CLI and editor inspection hooks.
5. Add tests for valid, partial, and invalid roots.

### Deliverables
- detector,
- preflight model,
- unit tests,
- basic editor summary.

### Exit criteria
- URPG can identify a PGMMV project and provide a useful readiness summary.

---

## Milestone 2 — Source reader and normalized import model

### Goal
Read PGMMV content into a stable intermediate model.

### Tasks
1. Implement source parsers.
2. Build normalized import entities.
3. Add deterministic IDs and source provenance.
4. Add serialization/debug dump support for normalized snapshots.
5. Add fixture-backed tests.

### Deliverables
- reader,
- normalized model,
- snapshot tests,
- import debug dump support.

### Exit criteria
- a minimal PGMMV sample can be read end to end into the normalized model.

---

## Milestone 3 — Native stage and asset conversion

### Goal
Import the first useful class of PGMMV projects.

### Tasks
1. Map stages into native URPG scene/level data.
2. Map layers and backgrounds.
3. Map sprite sheets and frame metadata.
4. Map collision regions and trigger boxes.
5. Map spawn points, pickups, hazards, and transitions.
6. Emit native project content.

### Deliverables
- stage mapper,
- asset mapper,
- native emitter,
- v1 imported sample project.

### Exit criteria
- a minimal PGMMV sample opens in URPG as a native project with level geometry, assets, and core entities intact.

---

## Milestone 4 — Migration analysis and diagnostics

### Goal
Make the feature trustworthy instead of merely functional.

### Tasks
1. Implement migration analyzer.
2. Emit structured issues.
3. Export JSONL diagnostics and import reports.
4. Add issue classification and severity.
5. Build report aggregation and summary stats.

### Deliverables
- analyzer,
- diagnostics exporter,
- report schema,
- tests for preserved/approximated/unsupported classification.

### Exit criteria
- every import run produces a reproducible migration report.

---

## Milestone 5 — Editor workflow integration

### Goal
Make PGMMV support usable from the standard editor UX.

### Tasks
1. Add PGMMV source option to import/migration entry points.
2. Build preflight summary panel.
3. Build migration report panel.
4. Add selective import toggles.
5. Add post-import repair links.

### Deliverables
- import wizard integration,
- report panel,
- selective import workflow,
- editor tests.

### Exit criteria
- a user can perform the full PGMMV import flow from the editor without needing dev-only internal steps.

---

## Milestone 6 — Action runtime enrichment

### Goal
Expand native runtime support so more PGMMV gameplay imports cleanly.

### Tasks
1. Harden action-state ownership.
2. Add more entity/controller mappings.
3. Add moving platform support.
4. Add richer trigger semantics.
5. Add controller tuning and preview tools.

### Deliverables
- action-runtime improvements,
- richer import coverage,
- expanded fixtures.

### Exit criteria
- medium-complexity PGMMV samples import with manageable repair work.

---

## 13. Detailed work breakdown structure

## Track A — Source format understanding

### A1. Project root inventory
- identify required files,
- identify optional files,
- classify hard-fail vs soft-fail missing elements.

### A2. Data-file parser design
- determine parsing approach,
- define validation rules,
- define tolerant parsing rules.

### A3. Asset inventory rules
- sprite folders,
- tile folders,
- audio folders,
- UI folders,
- animation metadata locations.

### A4. Source-semantic dictionary
Document what source concepts mean in URPG terms.

---

## Track B — Normalized model

### B1. Core entities
Define the intermediate types.

### B2. ID and reference policy
- stable IDs,
- source reference strings,
- dependency graph references.

### B3. Import evidence model
Capture provenance for migration reporting and debugging.

### B4. Snapshotability
Provide deterministic debug serialization.

---

## Track C — Mapping and conversion

### C1. Stage mapping
- coordinate systems,
- dimensions,
- layers,
- transitions,
- camera bounds.

### C2. Asset mapping
- sprite sheets,
- tiles,
- backgrounds,
- audio.

### C3. Entity mapping
- player,
- enemies,
- pickups,
- hazards,
- checkpoints.

### C4. Animation mapping
- frames,
- clip definitions,
- looping,
- directional states.

### C5. Interaction mapping
- trigger regions,
- action zones,
- interaction prompts,
- scene transitions.

---

## Track D — Diagnostics and reports

### D1. Issue taxonomy
Examples:
- `missing_asset`
- `unknown_collision_semantic`
- `unsupported_controller_behavior`
- `approximated_trigger_dispatch`
- `unsupported_hud_construct`
- `stage_dimension_conflict`

### D2. Severity model
- info
- warning
- error
- critical import-blocking

### D3. Migration disposition model
- preserved
- approximated
- unsupported

### D4. Report generation
- per-stage summaries,
- subsystem totals,
- import confidence score,
- repair recommendations.

---

## Track E — Editor and workflow

### E1. Preflight summary UX
### E2. Migration report UX
### E3. Selective import UX
### E4. Repair workflow UX
### E5. Re-import and diff behavior

---

## 14. Acceptance matrix

## 14.1 v1 acceptance

URPG may claim **initial PGMMV support** only when all are true:

1. A PGMMV project root can be detected reliably.
2. A minimal PGMMV sample imports into a native URPG project.
3. The import preserves:
   - stages,
   - basic layers,
   - sprite references,
   - collision regions,
   - simple entities,
   - simple triggers,
   - audio references.
4. Unsupported constructs are surfaced explicitly.
5. Migration diagnostics are exported deterministically.
6. The editor exposes a preflight and report view.

## 14.2 v2 acceptance

URPG may claim **practical PGMMV migration support** when all are true:

1. Medium-complexity projects import with limited manual cleanup.
2. Moving platforms and richer trigger behavior are supported.
3. Animation mapping is materially stronger.
4. Post-import repair workflows are present.
5. Sample corpus coverage includes both easy and adversarial fixtures.

## 14.3 production-baseline acceptance

URPG may claim **strong PGMMV migration support** when all are true:

1. Multi-stage action projects import consistently.
2. Migration reports are trusted and reproducible.
3. Editor tooling supports repair and verification.
4. Runtime ownership is clearly native, not source-engine-dependent.
5. Release gates prove stability for this import lane.

---

## 15. Test strategy

## 15.1 Unit tests
Cover:
- project root detection,
- parser correctness,
- normalized model construction,
- per-domain mapping,
- issue generation,
- report export.

## 15.2 Fixture tests
Use small real or synthetic PGMMV projects to validate end-to-end behavior.

### Required fixture classes
- minimal valid project,
- action sample,
- platformer sample,
- malformed sample,
- unsupported-edge-case sample.

## 15.3 Snapshot tests
Snapshot:
- normalized import model,
- migration reports,
- selected native schema outputs.

## 15.4 Integration tests
Validate:
- editor import flow,
- native project open-after-import,
- re-import stability,
- diagnostics panel projection.

## 15.5 Regression gates
Add a dedicated import lane to CI once the first vertical slice is real.

---

## 16. Diagnostics contract

Every PGMMV import should produce machine-readable output.

### Required fields
- source engine,
- project identifier,
- timestamp or deterministic run identifier,
- source file/location,
- subsystem,
- severity,
- migration disposition,
- issue code,
- human-readable message,
- recommendation,
- affected native object ID if created.

### Example categories
- source read failure,
- stage mapping approximation,
- missing asset fallback,
- unsupported trigger semantic,
- animation conversion downgrade,
- controller behavior omission.

### Contract rules
- no silent hard failures,
- no hidden downgrades,
- no “success” result with unreported critical losses.

---

## 17. Licensing and fixture governance

This lane will be useless if fixture strategy is sloppy.

### Rules
1. Only ingest fixture projects with clear redistribution rights.
2. Prefer synthetic internal fixtures for baseline tests.
3. Keep a manifest for provenance and permissions.
4. Never build the lane around legally dubious corpora.
5. Separate license-safe public fixtures from internal local-only validation fixtures.

### Deliverables
- fixture manifest,
- license notes,
- import corpus inventory.

---

## 18. Risk register

## Risk 1 — Source format ambiguity
PGMMV project data may contain undocumented or version-sensitive structures.

### Mitigation
- tolerant parsing,
- explicit version tagging,
- fixture diversity,
- structured unknown-field diagnostics.

## Risk 2 — Overpromising compatibility
The feature could drift into fake “supports PGMMV” messaging while only handling trivial demos.

### Mitigation
- disposition-based reporting,
- published support matrix,
- conservative language.

## Risk 3 — Direct mapper sprawl
Skipping the normalized import model would create fragile conversion code.

### Mitigation
- make the intermediate model mandatory.

## Risk 4 — Runtime gaps exposed late
Import support may get ahead of native action-runtime capabilities.

### Mitigation
- tie import milestones to runtime-owner readiness,
- do not mark mappings as preserved until runtime ownership exists.

## Risk 5 — Asset pipeline mismatch
Sprite and animation import may be lossy without better pipeline tooling.

### Mitigation
- prioritize sprite pipeline improvements alongside the importer.

## Risk 6 — Editor UX becomes an afterthought
A functional CLI importer with weak editor support will feel unfinished.

### Mitigation
- reserve explicit milestone capacity for editor workflow integration.

---

## 19. Recommended first implementation slice

This is the smallest slice worth doing.

### Slice goal
Import one simple PGMMV sample project into a URPG-native project with a truthful report.

### Scope
- project detection,
- source reader for core metadata,
- normalized import model,
- one stage importer,
- basic layer/background mapping,
- sprite sheet import,
- collision region import,
- spawn point import,
- simple trigger import,
- diagnostics export,
- editor preflight/report view.

### Why this slice is correct
It proves the architecture without pretending the whole lane is solved.

### Exit criteria
- imported project opens in editor,
- user sees what preserved vs approximated vs failed,
- import is repeatable and tested.

---

## 20. Recommended issue breakdown

## Epic 1 — PGMMV intake foundation
- add source engine registration
- add project detector
- add preflight summary model

## Epic 2 — PGMMV reader and normalized model
- parse project metadata
- parse stage data
- parse asset references
- implement normalized entities

## Epic 3 — First native import vertical slice
- stage mapping
- sprite import
- collision mapping
- trigger mapping
- native emitter

## Epic 4 — Diagnostics and reports
- issue taxonomy
- JSONL export
- report aggregation
- editor report surface

## Epic 5 — Editor workflow
- wizard integration
- selective import controls
- post-import repair hooks

## Epic 6 — Runtime expansion for stronger PGMMV coverage
- action runtime hardening
- richer entity/controller mapping
- moving platforms and advanced triggers

---

## 21. Suggested success language

Use language like this internally and externally:

### Good
- “PGMMV import is available for supported project structures.”
- “Unsupported constructs are reported explicitly.”
- “Imported projects continue as native URPG projects.”
- “Compatibility claims are based on tested fixture coverage.”

### Bad
- “URPG fully runs PGMMV games.”
- “PGMMV compatibility is complete.”
- “Everything imports automatically.”

---

## 22. Definition of done

This plan is complete when:

1. URPG can detect and preflight a PGMMV project.
2. URPG can ingest supported PGMMV source data into a normalized import model.
3. URPG can emit native project content for a defined v1 support subset.
4. Every import run produces structured diagnostics and a migration report.
5. Editor import/report workflows are present and truthful.
6. Test fixtures and CI coverage prove the lane is stable.
7. The imported project is owned by native URPG systems rather than a PGMMV emulation layer.

---

## 23. Draft next steps recorded at the time

1. Create `docs/PIXEL_GAME_MAKER_MV_SUPPORT_PLAN.md` in the repo.
2. Open an epic for “PGMMV intake foundation.”
3. Define the exact PGMMV project-root signature and fixture acquisition rules.
4. Implement the detector + preflight summary first.
5. Freeze a minimal support matrix before any broad compatibility claims are made.
