# URPG Native Capability Expansion Plan

Date: 2026-04-20  
Status: active roadmap  
Scope: native runtime, editor workflows, schema/migration, compat bridge exit, advanced capability expansion, and template/governance productization

This is the roadmap document, not the canonical latest-status snapshot. For current completion, remediation, and validation state, use:
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`

Cross-cutting debt closure, documentation-truth alignment, and intake-governance companion: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

Phase/remediation vocabulary note: Phase 2 runtime closure completed on 2026-04-19 in the canonical remediation/status docs. This roadmap remains the product roadmap; compat follow-up work belongs to later hardening and exit lanes unless the canonical docs explicitly re-open a Phase 2 slice.

Planning-input annexes for this roadmap:
- `./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md` - detailed PGMMV/native-absorption expansion input retained for traceability
- `./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md` - detailed PGMMV intake/migration input retained for traceability
- `./archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md` - earlier native-absorption planning input retained as superseded reference
- `../URPG_MISSING_FEATURES_GOVERNANCE_AND_TEMPLATE_EXPANSION_PLAN_v2.md` - governance/template expansion addendum retained as planning input while its approved deltas are absorbed into this roadmap and the canonical status stack
- `./URPG_facebookresearch_tooling_integration_plan.md` - offline ML/research tooling boundary input retained as planning input while its approved scope is absorbed into this roadmap and the canonical status stack

These annexes are not parallel execution authorities. New phase, status, and release-gate claims become canonical only after they are absorbed into this roadmap, `docs/PROGRAM_COMPLETION_STATUS.md`, and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

This plan replaces the previous addendum and becomes the single roadmap for turning URPG into a high-power engine for multiple game styles, while preserving import and migration trust for RPG Maker content.

## Strategic position

URPG's core value is native ownership, not plugin accumulation.

URPG's product goal is also explicitly WYSIWYG and easy to use. Native ownership is not the alternative to that goal; it is the foundation that lets the editor preview and ship the real thing without hidden plugin behavior or editor-only fakes.

- Compat exists to import, verify, migrate, and contain risk.
- Native systems own long-term product capability.
- Extension and customization infrastructure are first-class product lanes that must be governed explicitly, even when they are not the core delivery vehicle for a feature.
- Features are only considered complete when they are implemented, visually authorable, live-previewable, and low-friction for creators to use.

## Planning goals

1. Keep current native-first Wave 1 (UI/Menu, Message/Text, Battle, Save/Data) as the execution baseline.
2. Expand the roadmap to support broader game styles through additional high-leverage systems.
3. Integrate external-repo-inspired capabilities as first-class native tracks, not side appendices.
4. Add explicit governance, readiness, and cross-cutting minimum-bar lanes so subsystem and template claims stay evidence-gated.
5. Close each remaining lane as both runtime capability and WYSIWYG editor workflow, instead of deferring ease-of-use until after backend completion.

## Integrated capability model

### Tier A: Core ownership (must ship as native)

- UI/Menu Core
- Message/Text Core
- Battle Core
- Save/Data Core

### Tier B: Advanced power systems (planned native lanes)

- Gameplay Ability Framework (tags, abilities, explicit unsupported scripted-condition fields, state machines)
- AI Copilot Core (knowledge bridges, streaming services, tool-calling, orchestration) [Landed 2026-04-16]
- Visual Pattern Authoring (painted coordinate patterns for AoE, inventory-like footprints, rule masks)
- Modular Level Assembly (snap/connect block authoring with import helpers)
- Sprite Pipeline Toolkit (atlas packing, crop/trim, animation preview/tuning)
- Procedural Content Toolkit (dungeon generation, FOV, encounter scaffolding)
- 2.5D Presentation Lane (raycast camera/runtime mode for supported project styles)
- Timeline/Animation Orchestration (keyframes, temporary animated effects, event-driven animation control)
- Editor Productivity and Debug Utilities (selected high-value authoring helpers)
- Character Identity / Create-a-Character Runtime Lane (authoring assets plus exported-game runtime support)

### Tier C: Extension and customization lane

- thin scripting API
- project-specific add-ons
- staged customization modules

### Tier D: Offline tooling and content-prep lane

- semantic retrieval and indexing tooling
- image segmentation and cutout tooling
- audio separation, analysis, and compression experimentation
- importer-specific offline adapters and manifest builders

## Repository-informed capability seeds

These projects informed what to absorb. We treat them as design references first, and only reuse code when licensing/fit is explicitly approved.

| Source repo | Capability seed | URPG native target |
| --- | --- | --- |
| `Leshy/GAS4Godot` | Gameplay ability/tag/state architecture | `Gameplay Ability Framework` runtime + editor inspectors |
| `theNullinator/pattern_field_2d` | Painted pattern authoring UX | `Pattern Field Editor` resource + inspector widgets |
| `patatabravajocs/hippodamos` | Snap connector level authoring | `Modular Level Assembly` tools and import contracts |
| `slundi/spriters` | Sprite atlas/crop pipeline | `Sprite Pipeline Toolkit` CLI + editor hooks |
| `MajorMcDoom/cozy-cube-godot-addons` | Practical editor utility patterns | `Editor Productivity` module set |
| `fleton/dungeon-of-doom` | Dungeon/FOV/roguelike loop patterns | `Procedural Content Toolkit` |
| `juanfgs/raycaster` | Raycast runtime and map tooling concepts | `2.5D Presentation Lane` |
| `Dahie/sprite-spicifier` | Animation preview/tuning workflow | `Sprite Animation Preview` panel |
| `matoymush/ExtendedTAS` | Temporary animated sprite spawning | `Transient FX/Animation Events` in timeline lane |
| `loopier/animatron` | Live animation/timeline orchestration ideas | `Timeline/Animation Orchestration` layer |

## Ownership boundaries

### Native systems own

- authoritative runtime behavior
- editor authoring workflows
- schema/data contracts
- diagnostics and validation
- migration mapping from compat/import

### Compat lane owns

- import behavior capture
- regression verification
- migration confidence evidence
- controlled fallback when native mapping is incomplete

### Extension lane owns

- governed customization points and extension contracts
- no hidden authority over core save/state/combat/menu/message contracts

### Offline tooling lane owns

- heavy ML/research dependencies and helper environments
- restartable offline jobs under `tools/`
- stable manifests, cutouts, indexes, and processed asset outputs
- no direct authority over shipped runtime behavior beyond exported artifacts

## Execution roadmap

## Lane 1: Compat bridge exit and post-closure hardening

Goal: continue trustworthy import/verification/migration bridge hardening after the 2026-04-19 Phase 2 runtime closure, and define explicit exit criteria for the remaining compat lane.

Current scope note:

- Phase 2 runtime closure is already complete; this lane is the post-closure compat hardening and exit track.
- The QuickJS compat lane is still treated as a fixture-backed contract harness for import/verification work, not as a finished live JS runtime.
- Compat surface labels must stay conservative until placeholder-backed runtime paths are actually closed.

Deliverables:

- [x] Troop phase condition mapping and event command effect mapping: turn-based, enemy HP threshold, switch-based, **actor-present** conditions and common battle event commands (Show Text, Common Event, Change State, Force Action, Change Enemy HP/MP, **Change Gold/Items/Weapons/Armors, Transfer Player, Game Over**) are now migrated with honest fallback for unmapped commands.
- [x] Action scope expansion: all 12 RPG Maker MV/MZ scope codes map to native scope strings.
- expand routed conformance depth over curated profiles
- keep JSONL/report/panel diagnostics in lockstep for all new failures
- publish signed compat exit checklist

## Lane 2: Wave 1 native ownership (in progress)

Goal: convert existing seeded slices into full native delivery.

Latest landed progress (2026-04-19):

- **UI/Menu Core** — CLOSED. Runtime ownership (`MenuSceneGraph`, `MenuCommandRegistry`, `MenuRouteResolver`), editor productization (`MenuInspectorModel` edit workflows, `MenuPreviewPanel` fidelity), schema/migration (full route enums, visibility/enable rules, fallback preservation), and diagnostics workspace integration (edit/export/save/load round-trips) are all landed and validated.
- **Message/Text Core** — CLOSED. Runtime ownership (`MessageFlowRunner`, `RichTextLayoutEngine`, `ChoicePromptState`, `PortraitBindingRegistry`), renderer handoff (`MapScene` submits `TextCommand`/`RectCommand`, `OpenGLRenderer` consumes both), editor productization (`MessageInspectorModel` edit workflows, `MessageInspectorPanel` delegation), schema/migration (`default_choice_index`, `command`, window/audio style field mapping), and diagnostics workspace integration (mutation/export/save/load round-trips) are all landed and validated.
- WindowCompat bridge now emits backend-facing text draw commands (`RenderLayer::TextCommand`) from `Window_Base::drawText`.
- Compat `Window_Message` parity surface and deterministic centered/right wrapped snapshot tests are active.

Subsystem targets:

1. UI/Menu Core — CLOSED
2. Message/Text Core — CLOSED
3. Battle Core — CLOSED 2026-04-20 (closure signoff at `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`; promotion to `READY` requires human review of residual gaps)
4. Save/Data Core — CLOSED 2026-04-20 (closure signoff at `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`; promotion to `READY` requires human review of residual gaps)
   - recovery diagnostics and serialization schema export are landed; save policy editing/validation/apply is also landed

- [x] Wave 2 Audio State Sync: Implemented `GlobalStateHub` with "Diff-First" notifications and `StateDrivenAudioResolver` for automated BGM transitions. [Landed 2026-04-16]
- [x] Spatial Presentation: `SpatialMapOverlay` authoring coverage is backed by compiled `editor/spatial/*` panel sources in the build graph alongside the registered presentation runtime/release-validation lane.
- [x] Runtime VFX cue pipeline baseline: battle emits shared deterministic effect cues, and the presentation runtime now resolves newly-emitted cues through the shared resolver/translator path into world/overlay effect commands under focused battle bridge/runtime coverage.
- [x] Runtime VFX cue pipeline enrichment: battle emits semantic cue kinds; resolver uses kind-based preset routing; translator produces world/overlay commands.
- [x] Editor Tooling: `ElevationBrushPanel` and `PropPlacementPanel` now have compiled `.cpp` implementations plus render-snapshot coverage in `test_spatial_editor.cpp`.

Required outcomes per subsystem:

- runtime owner implemented
- editor surface implemented
- schema contracts finalized
- migration mapping implemented
- tests + diagnostics complete

## Lane 3: Wave 2 advanced power systems (new integrated lane)

Goal: expand engine power for multi-genre projects while preserving deterministic core behavior.

### 3.1 Gameplay Ability Framework

- [x] Gameplay tags and tag-gated activation queries (hierarchical tag support implemented in `engine/core/ability/gameplay_tags.h`; string-scripted `activeCondition` / `passiveCondition` remain deliberately unsupported in-tree).
- [x] Ability definitions, activation/cooldown/cost pipelines (implemented in `engine/core/ability/gameplay_ability.h` and `ability_system_component.h`).
- [x] State machines and transition guards (implemented in `engine/core/ability/ability_state_machine.h`).
- [x] Ability Inspector UI (implemented in `editor/ability/ability_inspector_panel.h`).
- [x] Ability Tasks for async execution (loops/delays implemented in `engine/core/ability/ability_task.h`).
- [x] Effect Modifiers and attribute math (implemented in `engine/core/ability/gameplay_effect.h`).
- [x] Ability/effect diagnostics and replay-safe execution.
- [x] Live battle-scene runtime integration for actor skill activation, cost checks, and execution history (implemented through participant-owned `AbilitySystemComponent` state in `engine/core/scene/battle_scene.h` / `.cpp`).
- [x] Target-aware battle ability execution context for live skill resolution (implemented through `GameplayAbility::AbilityExecutionContext` plus battle-scene target-side heal/state effect routing).
- [x] Live compat buff/debuff effect-stage support in battle (participant modifier stages now affect resolved battle params and decay on turn end in the native battle-scene path).
- [x] Diagnostics-workspace live ability preview workflow (implemented through selectable ability rows plus preview/test activation in `editor/ability/ability_inspector_panel.*` and `editor/diagnostics/diagnostics_workspace.*`).
- [x] Workspace-owned draft ability authoring preview for id/cost/cooldown/effect/pattern edits without pre-granted fixtures (implemented through draft-preview runtime synthesis in `editor/ability/ability_inspector_panel.*` and `editor/diagnostics/diagnostics_workspace.*`).
- [x] Draft ability round-trip persistence and battle-runtime application workflow (implemented through draft save/load plus mutable-runtime apply in `editor/diagnostics/diagnostics_workspace_serialization.cpp` and `editor/diagnostics/diagnostics_workspace.*`).
- [x] Shared authored-ability asset contract plus map-scene runtime consumption (implemented through `engine/core/ability/authored_ability_asset.h`, `engine/core/scene/map_scene.*`, and diagnostics-workspace application into map-owned ASC state).
- [x] Canonical `content/abilities/` project-content discovery plus explicit picker/load/apply/save workflow in the diagnostics workspace (implemented through authored-ability asset discovery helpers and project-content binding state in `engine/core/ability/authored_ability_asset.h` and `editor/diagnostics/diagnostics_workspace.*`).
- [x] Real map-authoring ability binding workflow for canonical project content (implemented through `editor/spatial/map_ability_binding_panel.*` and map interaction bindings in `engine/core/scene/map_scene.*`).
- [x] Multi-target map interaction authoring for tile- and prop-specific bindings on top of the same canonical authored-ability content lane (implemented through scoped interaction bindings in `engine/core/scene/map_scene.*` and placement-oriented state in `editor/spatial/map_ability_binding_panel.*`).
- [x] First visual map-canvas interaction authoring pass: click-to-place tile bindings, prop selection from placed props, and rectangular area painting on the shared interaction-binding runtime contract (implemented through screen-projected placement helpers in `editor/spatial/map_ability_binding_panel.*` and region-capable bindings in `engine/core/scene/map_scene.*`).
- [x] Richer map-canvas overlay authoring for abilities: committed multi-region painting, per-prop visual handle snapshots, and binding-overlay export so the editor can show selected, pending, and already-bound interaction targets directly on the canvas (implemented through overlay-ready snapshot state in `editor/spatial/map_ability_binding_panel.*` plus normalized multi-region runtime coverage in `tests/unit/test_scene_manager.cpp`).
- [x] Direct-on-canvas ability editing workflow for map interactions: an always-visible spatial ability canvas layer now consumes those overlay snapshots so existing tile, prop, and region bindings can be clicked, selected, and rebound from the map surface itself instead of only from the side panel (implemented through `editor/spatial/spatial_ability_canvas_panel.*` on top of `editor/spatial/map_ability_binding_panel.*`).
- [x] Bi-directional canvas editing for map interaction abilities: dragged tile/prop handles now move live bindings, selected regions can be resized in place, and inline asset/trigger badges are exported for on-canvas affordances instead of keeping that context hidden in side panels (implemented through explicit unbind/rebind helpers in `engine/core/scene/map_scene.*`, move/resize helpers in `editor/spatial/map_ability_binding_panel.*`, and drag/badge state in `editor/spatial/spatial_ability_canvas_panel.*`).
- [x] WYSIWYG map-canvas affordances for interaction authoring: hover previews now show prospective drop/resize state before commit, selected bindings can switch triggers directly from the canvas, and visible conflict warnings surface overlapping or competing tile/prop/region bindings instead of hiding them behind runtime ambiguity (implemented through trigger-switch helpers in `editor/spatial/map_ability_binding_panel.*` and preview/conflict snapshot state in `editor/spatial/spatial_ability_canvas_panel.*`).
- [x] Actionable conflict handling plus first composed spatial workspace: the canvas now exposes removable secondary-conflict targets and available trigger menus, while a dedicated `SpatialAuthoringWorkspace` composes elevation, prop placement, map-ability binding, and the canvas layer into one intentional editor surface instead of leaving them as isolated panels (implemented through removal helpers in `editor/spatial/map_ability_binding_panel.*`, actionable conflict metadata in `editor/spatial/spatial_ability_canvas_panel.*`, and `editor/spatial/spatial_authoring_workspace.*`).
- [x] Smarter conflict resolution and workspace-level spatial tooling: the canvas now distinguishes blocking same-trigger conflicts from cross-trigger advisory overlaps, supports keep-primary/keep-secondary, trigger-swap, and replace-secondary flows, and the composed spatial workspace now exposes a shared authoring mode toolbar so elevation, props, and ability binding behave like one continuous WYSIWYG surface instead of disconnected subpanels (implemented through replacement/swap helpers in `editor/spatial/map_ability_binding_panel.*`, policy-aware warnings in `editor/spatial/spatial_ability_canvas_panel.*`, and toolbar/mode state in `editor/spatial/spatial_authoring_workspace.*`).
- [x] Workflow-driving spatial toolbar and shared canvas routing: the spatial workspace toolbar now drives the editing flow instead of only describing it, with shared placement state, routed canvas actions for elevation painting / prop placement / ability authoring, and one-click suggested conflict resolution hooks that operate on the live canvas/runtime seam rather than panel-local state (implemented through routing and toolbar actions in `editor/spatial/spatial_authoring_workspace.*` plus suggested-resolution helpers in `editor/spatial/spatial_ability_canvas_panel.*`).

### 3.2 Pattern Field Editor

- [x] Painted 2D/3D pattern resources with JSON serialization.
- [x] Reusable pattern presets (implemented in `engine/core/ability/pattern_field.h`).
- [x] Reusable pattern presets for skills, items, placement, and interaction masks
- [x] pattern validation and preview in inspectors

### 3.3 Modular Level Assembly

- [x] Snap-connector block placement system (kernel implemented in `engine/core/level/level_assembly.h`).
- [x] block libraries and thumbnail generation
- [x] importer conventions for connector metadata
- [x] deterministic placement validation

### 3.4 Sprite Pipeline Toolkit

- [x] Atlas generation/packing and trim/crop tooling (CLI utility implemented in `tools/sprite_pipeline/`).
- [x] Schema definition for atlas metadata (implemented in `content/schemas/sprite_atlas.schema.json`).
- [x] animation-sheet preview/tuning panel
- [x] build-time artifact generation for runtime consumption

### 3.5 Procedural Content Toolkit

- [x] Dungeon/room/corridor generation primitives (baseline implemented in `engine/core/level/procedural_toolkit.h`).
- [x] Deterministic encounter/scenario generation from seeds
- [x] FOV and visibility systems (baseline implemented in `engine/core/level/fov_system.h`).
- [x] encounter/scenario generators tied to deterministic seeds

### 3.6 2.5D Presentation Lane

- [x] Raycast camera/runtime mode (baseline implemented in `engine/core/render/raycast_renderer.h`).
- [x] map authoring adapters for 2.5D projects
- [x] strict isolation from core 2D native contracts

### 3.7 Timeline/Animation Orchestration

- [x] Keyframe/timeline data model (implemented in `engine/core/animation/animation_clip.h`).
- [x] keyframe/timeline authoring for scene and UI animation
- [x] transient animated effect spawning hooks
- [x] deterministic event-to-animation bindings
  - VFX cue pipeline baseline is enriched with semantic kinds; transient animated effect spawning can consume resolved effect instances directly.

### 3.8 Editor Productivity Utilities

- [x] Selective adoption of high-value editor utilities (implemented `editor/productivity/editor_utility_task.h`).
- [x] focus on maintainability and ownership fit, not bulk addon import

## Lane 4: Offline tooling boundary and pipeline expansion

Goal: add high-value retrieval, vision, and audio tooling without turning URPG into a research-lab runtime or contaminating shipped builds with heavyweight ML dependencies.

Boundary rules:

- heavy research dependencies stay in `tools/` or a separate helper environment
- runtime consumes static outputs only: JSON, manifests, indexes, PNG/WebP cutouts, WAV/OGG, and metadata
- no PyTorch-heavy dependency enters the player/runtime build unless there is a separately proven product need
- every offline stage must be restartable, inspectable, and safe to rerun

### 4.1 Retrieval and search tooling

- [ ] Add an offline FAISS-based retrieval lane under `tools/retrieval`.
- [ ] Build searchable indexes for lore, dialogue, quests, items, and imported project metadata.
- [ ] Export chunk manifests with source paths and stable chunk IDs.
- [ ] Add a local query/debug CLI for authoring and import workflows.

### 4.2 Vision and segmentation tooling

- [ ] Add a SAM / SAM2-compatible segmentation lane under `tools/vision`.
- [ ] Support batch mask generation, cutout export, and per-asset manifest export.
- [ ] Preserve manual overrides so reruns do not wipe reviewed work.
- [ ] Keep the runtime consuming exported cutouts and manifests rather than segmentation code directly.

### 4.3 Audio tooling pipeline

- [ ] Add Demucs-based separation tooling under `tools/audio`.
- [ ] Add Encodec-based compression experiment tooling and output manifests.
- [ ] Add AudioCraft-based prototype-generation workflow only as temp-asset ideation, not as a shipped runtime dependency.
- [ ] Keep all generated/prototype outputs clearly marked and manifest-backed.

### 4.4 Optional later tooling

- [ ] Evaluate Detectron2 only if asset-scale tagging or QA pressure justifies maintenance.
- [ ] Evaluate PyTorch3D only if the 2.5D / 3D asset pipeline grows into a real maintained content lane.

## Lane 5: Productization and release hardening

Goal: convert delivered systems into stable production paths.

Deliverables:

- [x] Migration wizard flow for imported projects.
  Implemented as `MigrationWizardPanel` plus `MigrationWizardModel` with:
  - message, menu, and battle migration execution reporting
  - typed per-subsystem results
  - rendered summary text and headline snapshots
  - clear/reset support
  - default subsystem selection after a run
  - selected-subsystem detail, status, and summary snapshot fields
- [x] Release-readiness matrix by subsystem.
  Canonical matrix exists with evidence-gated `READY` rules and CI enforcement via `check_release_readiness.ps1`.
- [x] Template readiness matrix and template claim guardrails.
  Canonical matrix exists with template-claim validation in CI.
- [x] Project audit command and diagnostics panel.
  Current state: landed as a conservative readiness-derived scanner plus diagnostics tab with grouped governance detail for asset-intake, readiness schema/changelog, project-schema presence, and missing canonical input/localization/export artifact paths.
- [x] Canonical truth reconciler for subsystem/template/public readiness drift.
  Implemented as `tools/ci/truth_reconciler.ps1` with cross-doc alignment checks for readiness records, matrices, schemas, and docs.
- [x] Schema versioning and changelog governance.
  Implemented via `check_breaking_changes.ps1` and `check_schema_changelog.ps1`, both now enforced in CI.
- [x] Breaking change detection in CI.
  `check_breaking_changes.ps1` runs in all three gates (gate1-pr, gate2-nightly, gate3-weekly).
- [x] Gate reports proving stability for native + compat lanes.
- [x] Save policy governance CI gate with canonical fixture validation and schema alignment checks.
- [x] RPG Maker MV/MZ save file binary format loader with LZString decompression, XOR decryption, and JSON extraction.

## Lane 6: Governance foundation, template readiness, and cross-cutting product bars

Goal: make URPG safe to expose as a multi-template engine product by giving subsystem readiness, template readiness, and cross-cutting release bars explicit owners and verification paths.

### 5.1 Governance foundation

- [x] Release-readiness matrix, subsystem badge rules, and evidence-backed first-slice `READY` promotion flow
- [x] Template readiness matrix, template label rules, and safe-scope/export-confidence reporting
- [x] Canonical truth reconciliation checks for docs, diagnostics, and public-facing readiness language
- [x] Project audit command and diagnostics panel with template/subsystem/export blocker reporting
- [x] Schema versioning registry, schema changelog governance, and CI guardrails for version drift
- [x] Breaking-change detection for schemas, export contracts, and template minimum bars

### 5.2 Cross-cutting minimum bars

- [x] Accessibility governance and minimum exported-UI accessibility bar
- [x] Audio subsystem governance beyond current harness-backed compat truth
- [x] Input remapping and controller-governance lane
- [x] Localization completeness checker and missing-key validation
- [x] Performance budget profiling subsystem and budget-backed readiness gating
- [x] Visual regression testing suite for template-facing and presentation-facing proof

### 5.3 Template and product expansion

- [x] Create-a-Character runtime lane with authoring assets plus exported-game runtime support
- [x] Monster Collector RPG template spec
- [x] Cozy / Life RPG template spec
- [x] Metroidvania-lite template spec
- [x] 2.5D RPG template spec
- [x] Achievement/trophy system (registry, progress tracking, save/load, trigger parsing, event-bus auto-unlock, and editor panel)
- [x] Platform-specific export validation
- [x] Mod and player extension layer
- [x] Analytics and opt-in telemetry layer

## Milestone framing

### Milestone M1 (current roadmap baseline)

- Lane 1 post-closure near-exit hardening
- Lane 2 Wave 1 runtime ownership delivery start

### Milestone M2

- Wave 1 editor/schema/migration closure:
  - UI/Menu Core — CLOSED 2026-04-19
  - Message/Text Core — CLOSED 2026-04-19
  - Battle Core — CLOSED 2026-04-20
  - Save/Data Core — CLOSED 2026-04-20
- Lane 3 tracks 3.1 to 3.4 seeded and partially implemented

### Milestone M3

- Lane 3 tracks 3.5 to 3.8 implemented to production baseline
- cross-lane integration validation complete

### Milestone M4

- Lane 5 release hardening complete
- Lane 6 governance foundation started

### Milestone M5

- Lane 4 offline tooling boundary and first pipeline slices landed
- Lane 6 governance foundation and cross-cutting minimum bars complete
- template-readiness contracts published for supported game types
- full program completion report published

## Definition of complete (100% for this plan)

This plan is complete when:

1. Compat bridge exit criteria are signed off.
2. Wave 1 subsystems are fully native across runtime/editor/schema/migration/test/diagnostics.
3. Wave 2 advanced power systems are delivered at production baseline.
4. Governance foundation artifacts exist for subsystem readiness, template readiness, schema/version change control, and truth reconciliation.
5. Cross-cutting release bars for accessibility, localization, input, audio governance, and performance are defined and enforced where claimed.
6. Release gates validate native and compat stability with published evidence.
7. The remaining supported feature lanes are usable through live, low-friction WYSIWYG editor workflows rather than backend-only seams.
8. Any adopted ML/research tooling remains isolated behind an offline tooling boundary and contributes only stable exported artifacts to the shipped runtime.

## Acceptance rules for external-source-inspired work

1. Default rule: absorb architecture patterns and behavior ideas first.
2. Code reuse only after explicit license and compatibility review.
3. No direct import of third-party code that conflicts with URPG licensing strategy.
4. Every adopted capability must map to a named native owner and test anchor.

## Non-goals

- shipping a giant one-for-one plugin clone catalog
- making extension stacks mandatory for core capability
- treating imported plugin parameter sheets as long-term authoring UX
- bypassing native schema ownership for convenience

## Execution references

- Cross-cutting truthfulness, intake governance, and roadmap-alignment requirements: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- Program status and remaining checklist: `docs/PROGRAM_COMPLETION_STATUS.md`
- Governance foundation references:
  - `docs/RELEASE_READINESS_MATRIX.md`
  - `docs/TEMPLATE_READINESS_MATRIX.md`
  - `docs/TRUTH_ALIGNMENT_RULES.md`
  - `docs/TEMPLATE_LABEL_RULES.md`
  - `docs/SUBSYSTEM_STATUS_RULES.md`
- Archive index: `docs/archive/README.md`
- Detailed planning annexes:
  - `docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`
  - `docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md`
  - `docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md`
  - `URPG_MISSING_FEATURES_GOVERNANCE_AND_TEMPLATE_EXPANSION_PLAN_v2.md`
- Wave 1 specs:
  - `docs/UI_MENU_CORE_NATIVE_SPEC.md`
  - `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
  - `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
  - `docs/BATTLE_CORE_NATIVE_SPEC.md`
