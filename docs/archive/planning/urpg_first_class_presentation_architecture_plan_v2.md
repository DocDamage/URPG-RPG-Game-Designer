# URPG Presentation Core — First-Class Integration Program
## Architecture, execution, risk control, and release plan

**Date:** 2026-04-16  
**Version:** 2.0  
**Status:** Historical architecture/delivery planning input retained for traceability  
**Supersedes:** v1.0 (2026-04-16)

This archived plan preserves the architecture program as it was being drafted on 2026-04-16. It is not a current implementation authority for this branch; current execution truth must route through the canonical program/remediation docs and any later ADR corrections.

---

## Table of Contents

1. [Executive Mandate](#1-executive-mandate)
2. [Product Thesis](#2-product-thesis)
3. [Non-Negotiable Outcomes](#3-non-negotiable-outcomes)
4. [Explicit Non-Goals](#4-explicit-non-goals)
5. [First Principles](#5-first-principles)
6. [Decision Record Requirement](#6-decision-record-requirement)
7. [Domain Ownership Model](#7-domain-ownership-model)
8. [Required Repo Structure](#8-required-repo-structure)
9. [Contract-First Runtime Model](#9-contract-first-runtime-model)
10. [Scene Family Contracts](#10-scene-family-contracts)
11. [Capability Tiers and Fallback Policy](#11-capability-tiers-and-fallback-policy)
12. [Schema and Data Ownership](#12-schema-and-data-ownership)
13. [Editor Parity Requirements](#13-editor-parity-requirements)
14. [Migration Program](#14-migration-program)
15. [Diagnostics and Observability](#15-diagnostics-and-observability)
16. [Asset Pipeline Requirements](#16-asset-pipeline-requirements)
17. [Render Integration Requirements](#17-render-integration-requirements)
18. [Threading and Execution Model](#18-threading-and-execution-model)
19. [Memory and Allocation Model](#19-memory-and-allocation-model)
20. [Streaming and Level of Detail](#20-streaming-and-level-of-detail)
21. [Hot-Reload and Live Iteration](#21-hot-reload-and-live-iteration)
22. [Performance Program](#22-performance-program)
23. [Test Matrix](#23-test-matrix)
24. [CI and Merge Policy](#24-ci-and-merge-policy)
25. [Program Governance](#25-program-governance)
26. [Risk Register](#26-risk-register)
27. [Hard Stop Conditions](#27-hard-stop-conditions)
28. [Bounded-Step Execution Model](#28-bounded-step-execution-model)
29. [Phase Plan](#29-phase-plan)
30. [Definition of Done for Any Task](#30-definition-of-done-for-any-task)
31. [First 30 Bounded Tasks](#31-first-30-bounded-tasks)
32. [Release Gate](#32-release-gate)
33. [Rollback Policy](#33-rollback-policy)
34. [Out-of-Scope Declarations](#34-out-of-scope-declarations)
35. [Content Team Onboarding](#35-content-team-onboarding)
36. [Final Directive](#36-final-directive)

---

## 1. Executive Mandate

Presentation in URPG must stop being thought of as "the renderer" and must stop being framed as "HD-2D support."

That framing is too weak and guarantees drift.

The real deliverable is a **Presentation Core**: a native engine domain that owns how game state becomes visual intent across all supported presentation styles.

That core must treat these as first-class engine paths:

- `Classic2DPresentation`
- `SpatialPresentation`
- `ProjectionImplementations`
  - orthographic 2D
  - staged spatial / HD-2D
  - raycast 2.5D

This is not an optional visual lane.  
This is not a premium mode.  
This is not renderer-special behavior.  
This is a product-level engine subsystem.

If it does not own runtime, authoring, schema, migration, diagnostics, testing, and release criteria, then it is not integrated. It is just a prettier detour.

---

## 2. Product Thesis

URPG needs one authoritative system that answers this question:

**Given project settings, scene state, assets, authoring data, device capability, and active scene family, what is the correct visual interpretation of the world, and how should that interpretation degrade when constraints are hit?**

That system is Presentation Core.

The renderer does not answer that question.  
The backend does not answer that question.  
Map code does not answer that question.  
Editor hacks do not answer that question.

Presentation Core answers it once, consistently, and emits renderable intent.

---

## 3. Non-Negotiable Outcomes

This subsystem is only successful if all of the following are true:

- Classic 2D and Spatial are both native project modes
- both modes are authored through normal editor workflows
- both modes serialize through stable, versioned schema
- both modes pass through the same runtime ownership layers
- both modes can be migrated, validated, and downgraded with diagnostics
- both modes are test-gated in CI
- both modes have explicit performance budgets and fallback behavior
- scene families beyond maps are covered by contract
- the renderer executes presentation output instead of inventing rules ad hoc
- no scene family is left without editor, migration, and diagnostic coverage

Anything less is incomplete.

---

## 4. Explicit Non-Goals

These are the things the program must **not** pretend to be doing:

- replacing the entire rendering backend in one shot
- adding fully general 3D engine support
- supporting arbitrary mesh-heavy action game workflows
- making every legacy asset "automatically spatial" with zero authoring review
- allowing hand-authored renderer-only behavior to become production truth
- shipping "temporary" editor gaps that never get closed
- using screenshot quality as proof of integration
- owning audio environment composition (see Section 34)
- owning physics simulation or collision response
- providing a general-purpose material editor

Spatial presentation can be stylized and limited.  
It does **not** need to be a general-purpose 3D engine.  
It **does** need to be a first-class URPG engine path.

---

## 5. First Principles

### Principle A — Presentation is a top-level engine domain
Presentation must live under a dedicated domain, not be buried as renderer glue.

### Principle B — Renderer is execution, not authorship
The backend draws. Presentation decides what should be drawn and why.

### Principle C — Equal ownership across modes
If Classic2D gets schema, editor, validation, migration, diagnostics, and tests, Spatial must get the same classes of ownership.

### Principle D — Scene-family coverage is designed up front
MapScene ships first, but BattleScene, MenuScene, and overlay/UI composition cannot be afterthoughts. Their contracts are written before MapScene implementation begins.

### Principle E — Migration is part of integration
A feature without project upgrade rules is not native.

### Principle F — Fallback is a feature, not a failure
Capability downgrades must be authored, deterministic, visible, and testable.

### Principle G — Bounded-step execution only
No sprawling epic branches. No prestige refactors with no exit. One bounded slice at a time, with validation.

### Principle H — Threading and ownership are declared, not discovered
The execution thread of every Presentation Core call must be explicitly documented. Race conditions are not debugged after the fact; they are prevented by design.

### Principle I — Memory behavior is a first-class output constraint
Frame allocation patterns must be defined before implementation, not discovered at scale.

### Principle J — Hot-reload is not a luxury
Authors must be able to modify authoring data and see results without restarting the engine. This is a productivity contract, not a stretch goal.

---

## 6. Decision Record Requirement

At draft time, the program expected the following ADRs to be created before major implementation began. Each ADR was expected to include: context, decision, rationale, alternatives considered, and consequences.

### ADR-001 — Presentation Core ownership model
**Must answer:** What are the authoritative domain boundaries? What constitutes a boundary violation? How is leakage detected in code review?

**Skeleton:**
- Context: Presentation logic is currently scattered across renderer, map code, and editor hacks.
- Historical draft state: ownership table decision had not yet been finalized in this plan.
- Consequences: Any presentation semantic in renderer code is a review blocker.

---

### ADR-002 — Project presentation mode model
**Must answer:** Is mode project-wide, scene-overridable, or both? What is the resolution order? Can a scene family opt out of a mode?

**Skeleton:**
- Context: Some scenes (e.g., battle intros) may need to override the project-level mode temporarily.
- Historical draft state: mode scope hierarchy decision had not yet been finalized in this plan.
- Consequences: Affects schema design, migration logic, and editor mode-switching UI.

---

### ADR-003 — Spatial data representation
**Must answer:** What is the overlay schema format? What is the elevation model (grid-based, heightmap, discrete steps)? How are props referenced — by asset ID or by embedded definition? What is the coordinate system?

**Skeleton:**
- Context: Spatial data needs to be authorable, diff-friendly, and round-trip stable.
- Historical draft state: representation format decision had not yet been finalized in this plan.
- Consequences: Schema choice determines asset pipeline requirements and editor tool design.

---

### ADR-004 — Capability and fallback matrix
**Must answer:** What features are required, optional, degradable, or unsupported per tier? Who is responsible for declaring a feature's tier assignment? Can tier assignments change at runtime?

**Skeleton:**
- Context: Devices vary widely; the same project must degrade gracefully.
- Historical draft state: capability matrix structure decision had not yet been finalized in this plan.
- Consequences: Every new feature must specify its tier assignment before merge.

---

### ADR-005 — Scene-family minimum contracts
**Must answer:** What must MapScene, BattleScene, MenuScene, and UI layers support at minimum? What is the enforcement mechanism?

**Skeleton:**
- Context: MapScene will ship first; without contracts, other scenes will be left behind.
- Historical draft state: per-family minimum feature table had not yet been finalized in this plan.
- Consequences: Contracts must be met before any scene adapter is considered shippable.

---

### ADR-006 — Migration policy
**Must answer:** What is the upgrade behavior? What is downgrade behavior? Is migration reversible? How are unsupported constructs handled — error, warning, or skip? What is the rollback surface?

**Skeleton:**
- Context: Legacy projects must have a real upgrade path without faking completeness.
- Historical draft state: migration stages and reversibility rules had not yet been finalized in this plan.
- Consequences: Migration tooling must generate reviewable output, never silently complete.

---

### ADR-007 — Render pass architecture
**Must answer:** What is the pass order? Who owns world/UI separation? Who owns post-processing? How are shadow passes handled? How does transparency sorting integrate with pass order? Is the pass list static or dynamic?

**Skeleton:**
- Context: A spatial presentation path requires structured pass ordering that the backend cannot improvise.
- Historical draft state: initial pass architecture had not yet been finalized in this plan.
- Consequences: Render command families are tied to pass assignment; adding a new pass family is an architectural change.

---

### ADR-008 — Asset pipeline expectations
**Must answer:** What is required asset metadata vs. recommended? What happens at import when required metadata is missing? What is the build-time failure mode? Who owns metadata validation — import, editor, or runtime?

**Skeleton:**
- Context: Spatial presentation makes asset metadata load-bearing in a way Classic2D did not.
- Historical draft state: metadata contract and enforcement points had not yet been finalized in this plan.
- Consequences: Classic2D-safe assets must continue to work in Classic mode without spatial claims.

---

### ADR-009 — Threading model
**Must answer:** On what thread does `BuildPresentationFrame` execute? Is the result consumed on the same thread? What data can be accessed from what threads? What is the synchronization surface between Presentation Core and the renderer?

**Skeleton:**
- Context: Presentation Core must not introduce new race conditions or prohibit future parallelism.
- Historical draft state: thread ownership per subsystem had not yet been finalized in this plan.
- Consequences: All public API calls must document their thread requirement. See Section 18.

---

### ADR-010 — Memory and allocation model
**Must answer:** How is `PresentationFrameIntent` allocated? Is it per-frame arena, pool, or heap? Who owns lifetime? What is the maximum expected frame allocation cost?

**Skeleton:**
- Context: Frame-by-frame allocation strategy must not become a regression source at scale.
- Historical draft state: allocation strategy had not yet been finalized in this plan.
- Consequences: Allocation approach must be profiled as part of Phase 1 exit criteria. See Section 19.

---

If these decisions stay implicit, the implementation will fork itself in practice.

---

## 7. Domain Ownership Model

### 7.1 Presentation Core owns

- active presentation mode resolution
- presentation capability resolution
- scene-to-visual translation
- camera profile resolution
- environment composition intent
- elevation interpretation
- prop/overlay semantics
- actor visual semantics
- fallback routing
- diagnostics and warnings
- migration conversion rules
- presentation-specific validation
- presentation performance policy hooks
- streaming viewport hints

### 7.2 Renderer backend owns

- pass execution
- batching
- sorting primitives within pass rules
- draw submission
- GPU resource allocation
- shader/material binding
- render target orchestration
- shadow map generation
- depth prepass execution
- platform-specific backend behavior

### 7.3 Editor owns

- authoring UI
- visualization
- previewing
- validation surfaces
- migration review workflows
- compatibility display
- authoring-time warnings and fixups
- hot-reload triggering on authoring data change

### 7.4 Asset/content pipeline owns

- import-time metadata validation
- derived data generation
- asset dependency reporting
- presentation compatibility tagging
- build-time integrity enforcement

### 7.5 Boundary enforcement

This separation must be enforced in code review. The following are immediate review blockers:

- presentation rules appearing in renderer backend execution code
- scene-specific visual semantics in generic renderer passes
- fallback decisions made in the asset pipeline rather than in Presentation Core
- editor tools writing directly to renderer state without going through authoring data

---

## 8. Required Repo Structure

```text
engine/
  core/
    presentation/
      presentation_mode.h
      presentation_runtime.h/.cpp
      presentation_scene_translator.h/.cpp
      presentation_context.h/.cpp
      presentation_capability_matrix.h/.cpp
      presentation_validation.h/.cpp
      presentation_fallback_router.h/.cpp
      presentation_diagnostics.h/.cpp
      presentation_authoring_data.h/.cpp   ← new: see Section 9.3

      camera/
        camera_profile.h/.cpp
        camera_runtime.h/.cpp
        camera_constraints.h/.cpp

      environment/
        spatial_map_overlay.h/.cpp
        elevation_grid.h/.cpp
        prop_instance.h/.cpp
        light_profile.h/.cpp
        fog_profile.h/.cpp
        post_fx_profile.h/.cpp
        particle_emitter_profile.h/.cpp
        material_response_profile.h/.cpp
        occlusion_profile.h/.cpp
        occlusion_query_profile.h/.cpp      ← new: GPU occlusion integration

      actors/
        actor_presentation_profile.h/.cpp
        billboard_rules.h/.cpp
        anchor_rules.h/.cpp
        shadow_proxy_rules.h/.cpp
        depth_policy.h/.cpp

      projection/
        orthographic_projection.h/.cpp
        staged_spatial_projection.h/.cpp
        raycast_projection.h/.cpp

      scenes/
        map_presentation_adapter.h/.cpp
        battle_presentation_adapter.h/.cpp
        menu_presentation_adapter.h/.cpp
        overlay_presentation_adapter.h/.cpp

      streaming/                            ← new: see Section 20
        presentation_stream_hints.h/.cpp
        presentation_lod_policy.h/.cpp

  render/
    commands/
    passes/
    backends/
    materials/
    postfx/
    shadow/                                 ← new: shadow pass ownership

editor/
  presentation/
    inspectors/
    tools/
    preview/
    validators/
    migration/
    hot_reload/                             ← new: see Section 21

content/
  schemas/
    presentation/
  defaults/
    presentation/
  templates/
    presentation/

tests/
  unit/presentation/
  integration/presentation/
  snapshot/presentation/
  perf/presentation/
  hot_reload/presentation/                  ← new

docs/
  adr/
  presentation/
  onboarding/                               ← new: see Section 35
```

---

## 9. Contract-First Runtime Model

### 9.1 Core runtime contract

Presentation Core must expose something functionally equivalent to:

```cpp
PresentationFrameIntent BuildPresentationFrame(
    const SceneState& scene,
    const ProjectPresentationSettings& project_settings,
    const PresentationCapabilityProfile& capabilities,
    const PresentationAuthoringData& authored_data,
    const FrameContext& frame_context);
```

That function is the spine.

It must be:
- pure enough to test and stable enough to snapshot
- callable from the game thread only (see ADR-009 and Section 18)
- non-allocating in the steady state hot path (see ADR-010 and Section 19)
- side-effect free with respect to persistent engine state

### 9.2 PresentationFrameIntent must contain

- resolved presentation mode
- resolved projection implementation
- camera intent
- world composition intent
- actor presentation intent
- environment volumes
- light intent
- fog intent
- post-FX intent
- particle intent
- overlay/UI composition intent
- shadow pass parameters
- depth prepass parameters
- diagnostics emitted during resolution
- fallback decisions applied
- performance budget tags
- streaming viewport hints for upcoming frames

The renderer consumes this. It does not derive it.

### 9.3 PresentationAuthoringData definition

`PresentationAuthoringData` is the deserialized, validated, runtime-ready form of all authored presentation schema. It is the bridge between the content pipeline and the runtime contract.

It must contain:

```cpp
struct PresentationAuthoringData {
    // Per-project
    ProjectPresentationSettings          project_settings;

    // Per-scene
    MapPresentationOverlay               map_overlay;        // empty if not a map scene
    BattlePresentationConfig             battle_config;      // empty if not a battle scene
    MenuPresentationConfig               menu_config;        // empty if not a menu scene

    // Asset-bound
    CameraProfileLibrary                 camera_profiles;
    LightProfileLibrary                  light_profiles;
    FogProfileLibrary                    fog_profiles;
    PostFxProfileLibrary                 post_fx_profiles;
    ActorPresentationProfileLibrary      actor_profiles;
    ParticleEmitterProfileLibrary        particle_profiles;
    MaterialResponseProfileLibrary       material_profiles;
    PropLibrary                          prop_library;

    // Capability and fallback
    TierFallbackDeclaration              tier_fallback;
    PresentationMigrationMetadata        migration_metadata;

    // Validation status at load time
    PresentationValidationReport         load_validation;
};
```

`PresentationAuthoringData` is loaded once at scene load and updated via hot-reload. It is never partially mutable during frame execution.

### 9.4 Contract stability requirement

The signature of `BuildPresentationFrame` must not change without an ADR update and a migration review. Breaking changes to this interface are a phase-level event, not a task-level event.

---

## 10. Scene Family Contracts

### 10.1 MapScene
Must support:

- terrain/world composition
- elevation-aware traversal display
- props and occlusion metadata
- actor anchoring and depth rules
- local fog/light/post-FX zones
- camera anchors and constraints
- deterministic fallback to Classic2D-compatible output
- streaming-aware viewport hints for large maps

### 10.2 BattleScene
Must support:

- staged spatial composition with authoring-controlled depth
- battle formation anchoring for all supported formation sizes
- target readability guarantees — no effect or environment layer may obscure target indicators
- effect layering guarantees — hit effects, status effects, and skill effects have defined layer priority
- camera framing profiles per battle type (standard encounter, boss, ambush, arena)
- camera follow and transition profiles for sequence-driven battles
- UI-safe composition zones — no spatial element bleeds into UI reserved regions
- fallback to flatter staging without breaking formation logic or effect targeting
- per-formation occlusion and depth sorting rules
- explicit pre-battle and post-battle scene transition behavior

### 10.3 MenuScene
Must support:

- presentation-aware backgrounds with authoring-controlled visual depth
- low-cost environmental layering that does not spike GPU cost
- safe post-FX without UI degradation
- deterministic downgrade to static or lightly animated backgrounds
- explicit cost ceiling so menu scene cannot exceed defined budget

### 10.4 Message/UI overlays
Must support:

- absolute readability priority — no visual element may compromise text legibility
- stable composition independent of background presentation mode
- predictable z-order and contrast behavior across all tiers
- no presentation feature allowed to obscure UI correctness
- contrast guarantees independent of post-FX stack

**If UI readability loses to visual flair, the subsystem fails the product test.**

### 10.5 Contract enforcement

Scene-family contracts are not aspirational. They are:

- documented in `TEST_MATRIX.md` with specific coverage entries
- validated in integration tests
- required for phase exit (see Section 29)
- reviewed in every code review touching a scene adapter

---

## 11. Capability Tiers and Fallback Policy

A real subsystem needs explicit tiering.

### Tier classes

#### Tier 0 — Compatibility Baseline
- Classic2D required
- spatial features unavailable or explicitly minimal
- static or lightly animated backgrounds only
- no dynamic lighting requirement
- no expensive post-FX requirement
- guaranteed on all supported platforms

#### Tier 1 — Standard Spatial
- billboard actors
- elevation
- props
- limited dynamic lights (define count in ADR-004)
- fog volumes
- restrained post-FX

#### Tier 2 — Enhanced Spatial
- higher fog/light counts
- enhanced particles
- more material response complexity
- stronger post-FX budget
- improved shadow proxies
- soft-body occlusion masking

#### Tier 3 — Full Spatial
- full supported feature budget within engine-defined limits
- maximum density within documented constraints
- all optional features available

### Fallback rules

Fallback must be:

- deterministic — same inputs always produce same fallback
- diagnosable — every applied fallback emits a diagnostic
- author-visible — fallback preview is available in editor
- previewable by tier — authors can preview what their scene looks like at each tier
- snapshot-tested — fallback decisions are part of snapshot output
- safe for save/load and content packaging

**No silent degradation. No backend-only magic.**

### Fallback authoring requirement

Authors must be able to declare explicit fallback behavior per feature, not rely on engine-chosen defaults. Engine defaults exist for safety, but authored fallback is preferred.

---

## 12. Schema and Data Ownership

### 12.1 Required schema families

- `project_presentation_settings`
- `map_presentation_overlay`
- `camera_profile`
- `light_profile`
- `fog_profile`
- `post_fx_profile`
- `actor_presentation_profile`
- `particle_emitter_profile`
- `prop_library`
- `prop_placement_manifest`
- `material_response_profile`
- `occlusion_query_profile`          ← new
- `tier_fallback_declaration`
- `presentation_migration_metadata`
- `battle_presentation_config`        ← new: formally separate from map overlay
- `menu_presentation_config`          ← new: formally separate

### 12.2 Schema rules

Every presentation schema must be:

- versioned with an integer schema version field
- validated both at load time and at import time
- editor-visible with an appropriate inspector surface
- diff-friendly — arrays of named objects, not positional arrays
- deterministically loadable — identical bytes produce identical runtime state
- migration-aware — each version step has an explicit upgrade rule
- safely ignorable only when the schema family itself is explicitly marked optional
- round-trip stable through editor save/load

### 12.3 Serialization rule

No renderer-owned hidden defaults may serve as the real source of truth for production content.  
If a value matters, it must live in schema or a documented default profile.  
Undocumented defaults in renderer code are a review blocker.

### 12.4 Schema versioning strategy

- Schema versions are monotonic integers starting at 1
- A breaking change increments the integer
- Backward compatibility between adjacent versions is required
- Compatibility across more than two versions must be explicit, not assumed
- Schema version history is tracked in `docs/presentation/schema_changelog.md`

---

## 13. Editor Parity Requirements

If creators have to hand-edit core presentation data, the subsystem is unfinished.

### Required editor modules

- Presentation Mode Inspector
- Spatial Map Inspector
- Elevation Brush
- Stair / cliff continuity tool
- Prop Placement Tool
- Prop Library Browser
- Light Authoring Tool
- Fog Authoring Tool
- Camera Composition Panel
- Post-FX Profile Panel
- Actor Presentation Inspector
- Material Response Inspector
- Capability/Fallback Preview Panel (per tier)
- Tier Compatibility Validator
- Spatial Preview Viewport
- Migration Review Surface
- Presentation Diagnostics Panel
- Battle Presentation Inspector       ← new
- Menu Presentation Inspector         ← new
- Occlusion Debug Overlay             ← new
- Streaming Hint Visualizer           ← new

### Editor guarantees

- every authored construct must be previewable without leaving the editor
- every fallback must be previewable, per tier
- every validation failure must be visible in the diagnostics panel without console digging
- downgrade warnings must be visible before save/export
- migration changes must be reviewable before commit
- hot-reload must propagate authoring changes within one second (see Section 21)
- no editor tool may write directly to renderer state as a shortcut

---

## 14. Migration Program

Migration cannot be "we will deal with old projects later."

### 14.1 Required migration capabilities

- classic project detection
- presentation upgrade eligibility scan
- overlay bootstrap generation from existing map data
- unsupported construct reporting with explicit author action items
- reversible staging — migration is applied, not committed, until author approves
- partial upgrade mode — scenes can be upgraded independently
- downgrade back to Classic2D where possible
- explicit unsupported-loss warnings where downgrade is not possible

### 14.2 Migration outputs

Migration must generate:

- migrated schema changes in reviewable diff format
- reviewable diagnostics report
- unresolved item list with per-item context
- author action items with priority classifications
- downgrade risk report per scene
- compatibility summary by scene family

### 14.3 Migration principle

Automatic conversion may accelerate adoption, but it must never fake completeness.

If spatial data is missing, the system must say so directly and block the upgrade from being marked complete. A migration is not complete if it produces unresolved items without surfacing them.

### 14.4 Migration test requirement

Every migration rule must have a corresponding test that covers:
- successful upgrade of a known-good input
- detection and reporting of an unsupported construct
- downgrade from upgraded state
- round-trip stability (upgrade then downgrade produces a valid but marked-partial result)

---

## 15. Diagnostics and Observability

A first-class subsystem must explain itself.

### Required diagnostic classes

- invalid presentation mode resolution
- unsupported tier usage
- missing overlay data
- invalid elevation transitions
- anchor mismatch
- prop collision / overlap conflict
- light budget overflow
- fog/post-FX incompatibility
- missing material dependency
- unsupported asset metadata
- migration downgrade warning
- orphaned spatial data
- projection incompatibility
- scene-family contract violation
- performance budget breach
- fallback applied notice
- streaming hint generation failure     ← new
- occlusion query timeout or failure    ← new
- hot-reload schema validation failure  ← new

### Runtime debug overlays

Required debug overlay modes:

- active presentation mode
- active projection
- current capability tier
- fallback decisions in flight
- current pass timings
- shadow pass cost
- light/fog/particle counts vs. budget
- actor anchor/debug markers
- occlusion/debug masks
- streaming region boundaries
- LOD state per visible prop
- diagnostics stream

Without these, regression triage will be slow and debugging will be miserable.

### Diagnostic severity classification

Every diagnostic must declare a severity:

- `Error` — prevents correct frame generation; must be surfaced to author immediately
- `Warning` — generates degraded but valid output; must be logged and visible
- `Notice` — informational; visible on demand in diagnostics panel
- `Debug` — available only in debug builds or when overlay is enabled

---

## 16. Asset Pipeline Requirements

Spatial presentation without asset policy becomes a perpetual compatibility mess.

### Asset metadata expectations

#### Minimum required (import fails without these for spatial-claimed assets)
- pivot / anchor data
- footprint or occupancy hint
- depth category
- optional height class
- optional material response tag

#### Recommended enhanced
- emissive hint
- normal/specular support tags
- shadow proxy hint
- occlusion mask hint
- LOD/variant hint
- fallback sprite or simplified presentation tag

### Pipeline guarantees

- validate required metadata at import
- generate warnings for weak-but-usable assets
- fail build for invalid required metadata when used in spatial content
- permit Classic2D-safe assets to continue working in Classic mode without spatial claims
- generate a presentation compatibility report per asset as part of derived data

### Asset compatibility tagging

Every asset in the content pipeline must carry a `presentation_compatibility` tag set:

- `classic2d_safe` — works in Classic2D with no spatial metadata
- `spatial_eligible` — has minimum required spatial metadata
- `spatial_enhanced` — has full recommended metadata
- `spatial_incompatible` — missing required metadata for spatial use

This tag set is generated at import and is never hand-authored.

---

## 17. Render Integration Requirements

The current renderer remains useful, but it must be promoted from sprite executor to presentation executor.

### Required command families

- `Sprite`
- `Tile`
- `Text`
- `Rect`
- `BillboardSprite`
- `Mesh`
- `ShadowProxy`
- `Light`
- `FogVolume`
- `ParticleEmitter`
- `PostProcessVolume`
- `Decal`
- `OcclusionQuery`           ← new
- `DepthPrepassEntry`        ← new
- `DebugGizmo`

### Required pass model

The following is the initial pass order. This is a starting model — ADR-007 must validate and potentially expand it before Phase 4 implementation begins. Pass order may evolve, but changes require ADR update.

1. depth prepass (optional, tier-dependent)
2. shadow map generation (all shadow-casting objects)
3. world base / terrain
4. props and opaque scene composition
5. actor billboards / transparency-managed objects
6. shadow proxy and lighting accumulation
7. fog / atmosphere
8. particles
9. transparent / additive overlays
10. UI and text (always final before post-processing)
11. post-processing / presentation finishing

**Notes on the pass model:**

- Transparency sorting within pass 5 must be explicitly defined (camera-distance sort, layer-based, or hybrid). This is an ADR-007 decision.
- Pass 1 and pass 2 are optional at Tier 0 and must be absent by default at that tier.
- UI (pass 10) must be isolated from all post-processing effects. Post-processing must not bleed into UI composition.
- Any pass added beyond this list is an architectural change requiring ADR review.

### Rule

Renderer backend does not improvise scene semantics.  
It executes a resolved presentation plan.

---

## 18. Threading and Execution Model

This section defines the execution thread requirements for Presentation Core. All public API calls must document their thread constraint. Violations are review blockers.

### Thread ownership

| Component | Allowed Thread | Notes |
|---|---|---|
| `BuildPresentationFrame` | Game thread only | Must not be called from render thread or async jobs |
| Schema load / `PresentationAuthoringData` construction | Game thread or asset loading thread | Must be complete before first frame call |
| Hot-reload dispatch | Game thread | Hot-reload applies at safe points only; see Section 21 |
| Renderer command consumption | Render thread | `PresentationFrameIntent` is read-only after production |
| Diagnostics write | Game thread | Diagnostic collection during `BuildPresentationFrame` |
| Debug overlay read | Render thread | Read-only snapshot of last frame's diagnostics |

### Concurrency rules

- `PresentationFrameIntent` is produced by the game thread and consumed by the render thread. It must be treated as immutable after `BuildPresentationFrame` returns.
- No Presentation Core state is mutated during render thread execution.
- Streaming hints may be produced on the game thread and consumed asynchronously by the streaming system. The hint structure must be copyable and independent of frame lifetime.
- If parallelism is introduced in a future phase, it requires an ADR update. No ad hoc threading.

### Future parallelism note

The current model is deliberately single-threaded on the game thread side to minimize coupling. Parallelism within `BuildPresentationFrame` is a future option but must not be assumed in Phase 1 through 5.

---

## 19. Memory and Allocation Model

Frame-by-frame allocation behavior must be defined before implementation, not discovered at scale.

### Allocation strategy

- `PresentationFrameIntent` is allocated from a per-frame linear arena. The arena is reset each frame.
- No heap allocation occurs in the steady-state `BuildPresentationFrame` hot path.
- Schema data (`PresentationAuthoringData`) is heap-allocated once at scene load and is stable until hot-reload.
- Diagnostic objects are allocated from a per-frame diagnostic pool. Pool size is defined per tier.
- Streaming hints are copied to a small, fixed-size ring buffer.

### Arena sizing requirement

- Before Phase 1 exit: profile the arena cost for a representative MapScene at Tier 1.
- Before Phase 3 exit: establish an arena size ceiling per tier per scene family.
- Arena overflows are fatal errors in debug builds, degraded-with-diagnostic in release builds.

### Prohibited patterns

- No `new` / `malloc` calls in the `BuildPresentationFrame` call path.
- No `std::vector` push_back with growth inside frame execution.
- No deferred destruction of frame-produced objects on the render thread.

### Memory budget requirement

Perf targets (Section 22) must include a memory footprint target per scene family per tier. Memory cost is a first-class output metric.

---

## 20. Streaming and Level of Detail

Large maps and complex scenes require a streaming-aware Presentation Core. This section defines the minimum contract; a full streaming architecture is a Phase 7+ concern.

### Streaming responsibility split

- Presentation Core owns: viewport hints, LOD policy per prop and actor, streaming boundary visibility contributions.
- Engine streaming system owns: actual asset load/unload, sector management, memory budget enforcement.
- Presentation Core does not directly load or unload assets.

### Required streaming outputs in PresentationFrameIntent

- `StreamingViewportHint`: camera position, facing, projected movement delta for next N frames
- `PropLODState`: per-visible-prop LOD selection for this frame
- `ActorLODState`: per-visible-actor billboard complexity selection

### LOD policy requirements

- LOD thresholds are authored in `prop_library` entries, not hardcoded in renderer
- LOD selection must be deterministic given the same camera and capability tier
- LOD transitions must not produce visible pop without authored cross-fade behavior
- LOD state must be available in the debug overlay

### Streaming integration test requirement

Before Phase 8 exit: a stress map at maximum supported prop density must demonstrate stable streaming behavior with no hitching above the defined frame time target.

---

## 21. Hot-Reload and Live Iteration

Authors must be able to modify presentation authoring data and see results in editor preview without restarting the engine. This is a productivity contract.

### Hot-reload scope

The following must be hot-reloadable in editor:

- light profiles
- fog profiles
- post-FX profiles
- camera profiles
- actor presentation profiles
- material response profiles
- prop placement manifests
- elevation grid data
- tier fallback declarations

The following are **not** required to be hot-reloadable in Phase 5 (may be addressed later):

- schema family structure (adding/removing fields requires reload)
- scene adapter code
- projection implementation code

### Hot-reload contract

- A hot-reload event is dispatched on the game thread at a safe point (between frames).
- `PresentationAuthoringData` is rebuilt from updated schema in place.
- The next `BuildPresentationFrame` call uses the updated data.
- Hot-reload must not corrupt frame intent that is already in flight on the render thread.
- Hot-reload failures must emit a diagnostic with the reason and leave the previous valid state in place.

### Hot-reload latency requirement

The time from author saving a profile to seeing the result in editor preview must not exceed one second for any hot-reloadable asset type at the minimum hardware spec.

---

## 22. Performance Program

A first-class path without budgets is a future regression farm.

### Required performance targets

Before Phase 1 exit, define concrete targets for each of the following per capability tier. These must be documented in `docs/presentation/performance_budgets.md`.

- frame time target (ms) by capability tier on minimum spec and target spec hardware
- maximum supported actor count per scene family per tier
- maximum billboard count
- maximum light count
- maximum fog volume count
- maximum particle emitter count
- maximum post-FX stack cost
- arena allocation budget per frame per tier (see Section 19)
- scene-switch cost (time from scene unload begin to first valid frame of next scene)
- editor preview responsiveness (see Section 21)
- memory footprint per scene family per tier

### Required performance tooling

- automated performance scenarios run in CI on every merge to main
- pass timing capture with per-pass breakdown
- arena allocation tracking per frame
- perf snapshot baselines stored in repo alongside test fixtures
- regression alerts in CI when any scenario exceeds baseline by more than 5%
- fallback activation verification when budgets are breached

### Performance regression policy

A PR that causes a measurable perf regression without an explicit justification and budget amendment is a review blocker. "It's just a little slower" is not an acceptable review comment.

---

## 23. Test Matrix

Graphics theater without tests is not product engineering.

### 23.1 Unit tests

- presentation mode resolution
- capability matrix resolution
- fallback router behavior
- camera profile calculations
- elevation adjacency rules
- actor anchoring math
- depth policy resolution
- schema validation
- schema version migration
- migration conversion rules
- diagnostics emission (severity and content)
- asset compatibility checks
- streaming hint generation
- LOD policy selection
- arena allocation boundaries

### 23.2 Integration tests

- MapScene through PresentationRuntime end-to-end
- BattleScene staged spatial composition
- BattleScene target readability guarantee (effect layers must not obscure targets)
- MenuScene background composition within budget ceiling
- overlay/UI coexistence — readability guarantees hold across all tiers
- project mode switching
- migration wizard execution
- downgrade behavior on lower tier capabilities
- save/load/editor round-trip parity
- hot-reload stability (schema reload does not corrupt in-flight frame)

### 23.3 Snapshot tests

- `PresentationFrameIntent` snapshots per scene family per tier
- render command stream snapshots
- schema round-trip snapshots
- diagnostics output snapshots
- migration report snapshots
- fallback decision snapshots

Snapshot drift is a signal that intent generation behavior changed. Intentional changes require a snapshot update committed alongside the code change. Unintentional drift is a blocker.

### 23.4 Performance tests

- stress map: maximum prop density, Tier 1 and Tier 2
- stress battle: maximum formation size, all camera profiles
- stress menu: maximum layered background
- tier downgrade activation: budget breach triggers correct fallback
- editor preview stress: hot-reload under authoring load
- arena allocation ceiling: verify no frame exceeds defined budget

---

## 24. CI and Merge Policy

This is where most large features rot.

### Required CI gates

No Presentation Core PR merges unless all applicable gates pass:

- compile (all supported platforms)
- unit tests
- integration tests
- snapshot tests
- schema compatibility tests (new schema version does not break load of previous version fixtures)
- migration tests
- performance smoke tests (no regression beyond 5% from baseline)
- editor serialization tests
- docs/ADR presence checks for any PR touching domain boundaries or schema families

### Required repo rules

- no direct renderer-only feature additions that bypass Presentation Core
- no new presentation schema without validator, version metadata, and round-trip test
- no fallback-affecting change without snapshot update
- no scene adapter change without integration coverage
- no migration-affecting change without migration test coverage
- no new thread-of-execution requirement without ADR update
- no new allocation in the `BuildPresentationFrame` hot path without memory budget review

### Platform gate requirement

CI must run on at minimum the minimum-spec platform definition for Tier 0 and on the target-spec definition for Tier 2. Tier 3 performance gates may run on a dedicated hardware pool.

---

## 25. Program Governance

This subsystem is large enough to require actual control.

### Required artifacts

- `PLAN.md` — living task state
- `WORKLOG.md` — exact outcome of every completed slice
- `RISKS.md` — risk register with current severity and mitigation status
- `DECISIONS/ADR-*.md` — one file per ADR, updated when decisions change
- `TEST_MATRIX.md` — mapping of coverage requirements to test implementations
- `RELEASE_CHECKLIST.md` — full release gate checklist
- `docs/presentation/performance_budgets.md` — per-tier, per-scene-family performance targets
- `docs/presentation/schema_changelog.md` — schema version history

### Required review cadence

Every bounded slice must answer before merge:

- what changed
- what was validated
- what remained blocked
- what risk changed
- whether any ADR must be updated
- whether performance budget was impacted
- whether snapshot baseline was updated intentionally

### Governance failure modes

The following are governance failures that require escalation, not just a comment:

- a slice merges without all CI gates passing
- a schema change ships without version metadata
- a new thread-of-execution is introduced without ADR update
- `WORKLOG.md` falls more than one slice behind actual progress
- a risk changes severity without `RISKS.md` being updated

---

## 26. Risk Register

Risks are rated by **severity** (impact if realized) and **likelihood** (probability within this program). Rating scale: Low / Medium / High / Critical.

---

### Risk 1 — Backend becomes the real source of truth again
**Severity:** Critical | **Likelihood:** High

Presentation logic migrates back into renderer execution because it is "easier" or "faster to ship."

**Mitigation:** Contract-first `PresentationFrameIntent`; explicit review rule against semantic leakage; CI gate blocking renderer-only presentation behavior; boundary violation is a review blocker, not a comment.

---

### Risk 2 — Maps get all the love and everything else rots
**Severity:** High | **Likelihood:** High

BattleScene, MenuScene, and overlays are deferred until after MapScene ships and never get proper coverage.

**Mitigation:** Scene-family contracts written and reviewed before MapScene implementation begins. BattleScene and MenuScene adapter interfaces are created in Phase 1 (see task list). Phase 6 has explicit exit criteria per scene family.

---

### Risk 3 — Editor support gets deferred indefinitely
**Severity:** High | **Likelihood:** High

Editor tools are treated as polish and never actually ship, leaving authors hand-editing schema files.

**Mitigation:** Editor parity is a phase gate (Phase 5), not a polish task. Phase 5 exit criteria require authors to be able to author and validate real spatial maps in editor without hand-editing.

---

### Risk 4 — Migration becomes fake-complete
**Severity:** High | **Likelihood:** Medium

Migration tooling appears to work but silently drops unsupported constructs, leaving projects in a broken state.

**Mitigation:** Force unresolved-item reporting; reversible staging; migration tests cover unsupported construct detection. A migration that silently discards data is a test failure.

---

### Risk 5 — Performance collapses after content scales up
**Severity:** High | **Likelihood:** Medium

Stress testing is delayed until late in the program, at which point architectural changes are expensive.

**Mitigation:** Perf tests start at Phase 1 exit. Arena allocation targets are defined before code is written. Performance budget documents are written before Phase 3.

---

### Risk 6 — Asset requirements are unclear and content teams invent workarounds
**Severity:** Medium | **Likelihood:** High

Asset metadata contract is vague, content teams make assumptions, and spatial content has inconsistent behavior.

**Mitigation:** Explicit metadata contract in ADR-008 before content production begins. Import-time validation blocks underdefined assets from being used in spatial content. Asset compatibility tagging makes status visible.

---

### Risk 7 — Tasks stay too large and churn without closure
**Severity:** Medium | **Likelihood:** Medium

Slices are defined too broadly, become multi-week efforts, and never formally close.

**Mitigation:** Bounded-step slicing rules (Section 28) enforced at planning time. A task that spans multiple schema/runtime/editor categories at once is split before work begins.

---

### Risk 8 — Spatial mode becomes visually impressive but functionally unsupported
**Severity:** Critical | **Likelihood:** Medium

Spatial rendering looks good in demos but has no diagnostics, no migration path, no editor support, and no documented fallback — making it unusable in production projects.

**Mitigation:** Release gate (Section 32) requires all of: diagnostics, tests, migration, editor authoring, and documented fallback. None can be waived.

---

### Risk 9 — Team lacks domain knowledge for spatial/projection concepts
**Severity:** High | **Likelihood:** Medium

Implementors are unfamiliar with billboard sorting, elevation grid design, or staged spatial composition, leading to incorrect implementations that pass visual inspection but fail edge cases.

**Mitigation:** ADRs for spatial data representation (ADR-003) and render pass architecture (ADR-007) must be reviewed by someone with prior spatial rendering experience before implementation begins. Snapshot tests catch behavioral drift even when visual output looks acceptable.

---

### Risk 10 — Platform GPU behavior diverges between development and target hardware
**Severity:** High | **Likelihood:** Low

Spatial presentation works correctly on developer workstations but exhibits different behavior (sorting, blending, occlusion) on target platform hardware.

**Mitigation:** CI platform gate runs on minimum-spec and target-spec hardware (Section 24). Platform-specific behavior is owned by the renderer backend, not by Presentation Core, limiting the surface area of divergence.

---

### Risk 11 — Shader complexity becomes a maintenance problem
**Severity:** Medium | **Likelihood:** Medium

Spatial presentation requires new shaders that are not covered by existing review practices, leading to GPU-side bugs that are difficult to diagnose.

**Mitigation:** Shader authoring and review are explicitly in scope for Phase 4. ADR-007 defines shader ownership boundaries. Debug gizmo pass provides GPU-side diagnostic visibility.

---

### Risk 12 — Hot-reload introduces frame corruption
**Severity:** High | **Likelihood:** Low

Hot-reload applies schema changes at an unsafe point in the frame, corrupting a `PresentationFrameIntent` that is already in flight on the render thread.

**Mitigation:** Hot-reload applies only at explicitly defined safe points (between frames). `PresentationAuthoringData` is rebuilt atomically. Hot-reload tests in CI cover concurrent frame production (Section 23.2).

---

## 27. Hard Stop Conditions

Stop and surface a blocker when:

- the same task fails repeatedly without narrowing
- a task cannot complete inside one bounded slice
- an ADR question blocks correct implementation
- fallback behavior is undefined for a required capability
- editor ownership is unclear for new schema
- migration behavior is unknown for a changed construct
- snapshot churn suggests unstable intent generation
- performance cost cannot be bounded within the active tier
- a threading assumption is introduced that is not covered by ADR-009
- an allocation pattern in the hot path is not covered by ADR-010

When this happens, do not push through. Break the task down or resolve the missing decision. Pushing through a hard stop produces debt that compounds.

---

## 28. Bounded-Step Execution Model

The execution model must remain brutally simple.

### Loop

1. choose the first unchecked bounded task
2. confirm the task fits one slice (if not, split it now)
3. implement only that slice
4. update `PLAN.md` truthfully
5. append exact outcome to `WORKLOG.md`
6. run relevant tests and perf smoke checks
7. keep only validated changes
8. stop on stall, blocker, or red state

### Slice rule

A slice is too big if it requires more than one of these at once:

- new schema family
- new runtime contract
- new editor module
- migration behavior
- new pass family
- new scene adapter
- new fallback policy
- new allocation pattern
- new thread boundary

If it spans multiple categories, split it. The split happens before work begins, not after the task has been running for a week.

### Slice output requirement

A slice is not done until `WORKLOG.md` records:
- what was built
- what tests were added or updated
- what snapshot baselines were updated
- what performance impact was measured
- what was blocked or deferred with explicit rationale

---

## 29. Phase Plan

### Phase 0 — Program Setup
Deliver:

- Presentation Core naming locked and documented
- repo structure scaffolded (empty directories + placeholder headers)
- all 10 ADRs opened with skeleton content (see Section 6)
- ownership matrix defined (Section 7)
- capability tier model defined and documented
- performance budget document initialized (targets TBD at Phase 1 exit)
- release checklist initialized
- test matrix initialized
- risk register initialized
- content team onboarding doc started (see Section 35)
- all scene-family contracts written (Section 10) and reviewed

#### Exit criteria
No unresolved ambiguity on ownership or success criteria. All ADR skeletons reviewed. All scene-family contracts written.

---

### Phase 1 — Runtime Spine
Deliver:

- `PresentationMode` enum and resolver
- project presentation settings schema (versioned)
- `PresentationContext` core type
- `PresentationAuthoringData` structure (Section 9.3)
- `PresentationRuntime` entry point
- `PresentationDiagnostics` channel (with severity classification)
- capability tier enum/model
- `PresentationCapabilityMatrix`
- `PresentationFallbackRouter`
- `PresentationSceneTranslator` base contract
- camera runtime skeleton
- all scene adapter interfaces (map, battle, menu, overlay)
- arena allocator skeleton with budget tracking
- initial performance budget document (targets established from profiling)

#### Exit criteria
Scenes can resolve presentation intent through a native entry point. Arena allocation is measured and within budget. All CI gates pass.

---

### Phase 2 — Spatial Data Foundation
Deliver:

- `SpatialMapOverlay` schema (versioned)
- `ElevationGrid` schema and validator
- prop instance schema
- light profile schema
- fog profile schema
- post-FX profile schema
- material response profile schema
- tier fallback declaration schema
- validation for all new schema families
- import/load round-trip coverage
- schema changelog initialized

#### Exit criteria
Spatial presentation data exists as first-class content, not ad hoc fields. All schema families have validators and round-trip tests.

---

### Phase 3 — MapScene Integration
Deliver:

- MapScene adapter (full, not skeleton)
- actor anchoring rules
- depth policy
- first spatial command emission
- first deterministic Classic2D fallback with diagnostic
- diagnostic surfaces for all defined map spatial issues
- streaming viewport hint generation
- initial LOD policy for props

#### Exit criteria
MapScene routes cleanly through Presentation Core. Fallback is deterministic and tested. Streaming hints are generated and validated.

---

### Phase 3.5 — Early Proof of Concept Milestone *(new)*
Before Phase 4 begins, demonstrate end-to-end:

- a real map scene rendered through Presentation Core
- fallback to Classic2D rendered through the same path
- at least one diagnostic visible in the diagnostics panel
- performance measured against Phase 1 budget targets

This milestone is not a phase gate. It is a sanity check. If the end-to-end path does not work at this point, Phase 4 must not begin.

---

### Phase 4 — Render Execution Expansion
Deliver:

- expanded command model (all command families in Section 17)
- pass structure per ADR-007
- depth prepass
- shadow map pass
- billboard path
- light/fog/post-FX execution
- world/UI separation enforced in pass structure
- transparency sorting implementation
- pass timing instrumentation

#### Exit criteria
Backend executes `PresentationFrameIntent` without semantic hacks. Pass timing is captured and within budget. UI isolation is verified in snapshot tests.

---

### Phase 5 — Editor Parity Foundation
Deliver:

- Presentation Mode Inspector
- Spatial Map Inspector
- Elevation Brush
- Prop Placement Tool
- Presentation Diagnostics Panel
- preview viewport (all tiers)
- tier/fallback preview
- hot-reload for all hot-reloadable asset types (Section 21)
- Battle Presentation Inspector (initial)
- Menu Presentation Inspector (initial)

#### Exit criteria
Designers can author and validate real spatial maps in editor without hand-editing schema files. Hot-reload latency is within the one-second target. All editor tools have validation integration.

---

### Phase 6 — Battle and Menu Coverage
Deliver:

- BattleScene adapter (full contract coverage per Section 10.2)
- MenuScene adapter (full contract coverage per Section 10.3)
- overlay/UI composition rules
- safe composition presets
- battle/menu fallback behavior
- target readability guarantee tests
- UI safe zone enforcement tests

#### Exit criteria
Presentation Core is not "the fancy map path." BattleScene and MenuScene adapters meet their full scene-family contracts. All integration tests for non-map scenes pass.

---

### Phase 7 — Migration
Deliver:

- classic project detection
- presentation upgrade eligibility scan
- overlay bootstrap generation
- unsupported construct reporting
- reversible staging workflow
- partial upgrade mode
- downgrade diagnostics
- migration tests (all four required test types per Section 14.4)
- migration review UI in editor

#### Exit criteria
Legacy projects have a real, reviewable path forward. Migration does not fake completeness. Unresolved items are surfaced to authors, not silently dropped.

---

### Phase 8 — Hardening
Deliver:

- unit test coverage ≥ 90% for all Presentation Core modules
- integration test coverage: all scene families × all tier levels × all fallback paths
- stress scenarios at defined maximum scale (Section 23.4)
- perf baselines finalized and committed
- all perf targets met on minimum-spec and target-spec platforms
- memory footprint within defined ceilings per tier per scene family
- docs completed: all ADRs finalized, all schema documented, onboarding guide complete
- outstanding risks in RISKS.md are either closed or explicitly accepted
- release checklist (Section 32) reviewed and all items confirmed

#### Exit criteria
Subsystem is supportable, testable, and shippable. No release checklist item is marked incomplete. All CI gates pass on all platform targets.

---

## 30. Definition of Done for Any Task

A Presentation Core task is done only when all applicable items are true:

- code exists and compiles on all supported platforms
- schema exists if needed (versioned, validated)
- validation exists if needed
- editor exposure exists, or a documented deferral is approved with a specific re-engagement date
- migration impact is addressed
- diagnostics are emitted where failure is possible
- tests cover the slice (unit, integration, or snapshot as appropriate)
- performance impact is at minimum smoke-checked against the current baseline
- `PLAN.md` was updated
- `WORKLOG.md` was updated with exact outcome
- no hidden backend-only behavior became authoritative
- a peer review with at least one non-author reviewer was completed
- no new thread or allocation pattern was introduced without ADR coverage

Anything weaker is partial progress, not done.

---

## 31. First 30 Bounded Tasks

Tasks are grouped by phase. Dependencies are noted where a task must not begin until a prior task is complete.

**Phase 0:**
1. Write and review all 10 ADR skeletons
2. Write and review all 4 scene-family contracts (Section 10)
3. Scaffold repo structure (empty directories + placeholder headers)
4. Initialize PLAN / WORKLOG / RISKS / ADR repo scaffolding
5. Initialize performance budget document with placeholder targets

**Phase 1:**
6. Define `PresentationMode` enum and resolver *(depends on: ADR-001, ADR-002)*
7. Add `ProjectPresentationSettings` schema with version metadata *(depends on: ADR-002)*
8. Add `PresentationContext` core type *(depends on: 6)*
9. Add `PresentationAuthoringData` structure *(depends on: 6, 7)*
10. Add `PresentationRuntime` entry point *(depends on: 8, 9)*
11. Add `PresentationDiagnostics` channel with severity classification *(depends on: 10)*
12. Add capability tier enum and model *(depends on: ADR-004)*
13. Add `PresentationCapabilityMatrix` *(depends on: 12)*
14. Add `PresentationFallbackRouter` *(depends on: 13)*
15. Add `PresentationSceneTranslator` base contract *(depends on: 10)*
16. Add MapScene presentation adapter interface *(depends on: 15)*
17. Add BattleScene presentation adapter interface *(depends on: 15)*
18. Add MenuScene presentation adapter interface *(depends on: 15)*
19. Add overlay/UI composition adapter interface *(depends on: 15)*
20. Add camera profile schema and camera runtime skeleton *(depends on: ADR-003)*
21. Add arena allocator skeleton with budget tracking *(depends on: ADR-010)*
22. Profile arena cost on representative MapScene; commit baseline to performance budget doc *(depends on: 21)*

**Phase 2:**
23. Add `SpatialMapOverlay` schema (versioned) *(depends on: ADR-003)*
24. Add `ElevationGrid` schema and validator *(depends on: ADR-003)*
25. Add prop instance schema *(depends on: ADR-003, ADR-008)*
26. Add light / fog / post-FX / material response profile schemas *(depends on: ADR-003)*
27. Add `TierFallbackDeclaration` schema *(depends on: 13)*
28. Add `ActorPresentationProfile` schema *(depends on: ADR-003)*
29. Add schema round-trip tests for all Phase 2 schema families *(depends on: 23–28)*
30. Initialize schema changelog and commit first version entries *(depends on: 29)*

That is the correct starting backlog: small, concrete, checkable, and dependency-ordered.

---

## 32. Release Gate

Presentation Core is not release-ready until all of the following are confirmed and documented in `RELEASE_CHECKLIST.md`:

- runtime path is stable and has been exercised in all scene families
- all scene families are contract-covered (per Section 10)
- schema is versioned, validated, and documented
- editor authoring exists for all required modules (per Section 13)
- migration exists and is not fake-complete
- downgrade path exists for all supported constructs
- diagnostics exist for all defined diagnostic classes
- unit, integration, and snapshot test coverage meets Phase 8 exit criteria
- performance budgets are defined, measured, and met on all platform tiers
- CI enforces all required gates on all platform targets
- ADRs are finalized (not just opened)
- schema changelog is complete
- onboarding documentation is complete and reviewed by a new team member
- unsupported cases are explicitly documented

If any one item is missing or marked incomplete, the subsystem is still in development.

---

## 33. Rollback Policy

If a phase or a bounded task produces a result that cannot be brought to a passing state within a defined window, a rollback is required. Pushing through a broken phase is not acceptable.

### Rollback triggers

- a phase exit criterion cannot be met after two additional bounded slices of focused remediation
- a CI gate is broken and cannot be restored within 48 hours
- a performance regression exceeds 20% above baseline and cannot be traced to a bounded cause
- a hard stop condition (Section 27) is reached and no resolution is possible within the current slice

### Rollback procedure

1. stop all work in the affected phase under this draft program
2. revert to the last known-good state (last passing CI + last confirmed `WORKLOG.md` entry)
3. document the failure in `WORKLOG.md` with the exact failure mode and what was learned
4. update `RISKS.md` with the risk that was realized
5. if an ADR decision contributed to the failure, update the ADR before re-attempting
6. re-scope the phase or the failing slice before resuming

A rollback is not a failure of the program. It is the program working correctly. The failure is pushing through a broken state rather than rolling back.

---

## 34. Out-of-Scope Declarations

The following are explicitly outside the scope of this program. They may interact with Presentation Core but are not owned by it.

### Audio environment composition
Reverb zones, music transitions, and ambient sound profiles are not owned by Presentation Core. They may share scene-family context but must be driven by a separate audio subsystem. If future integration is desired, it requires a separate ADR.

### Physics and collision
Spatial presentation uses elevation data for visual display, not for physics or collision detection. The elevation grid is a presentation construct, not a physics construct. Physics systems that need elevation data must source it from the authoritative physics/world representation.

### General-purpose material editor
Presentation Core owns material response profiles as a presentation semantic (how does this material look under spatial lighting?). It does not own a general-purpose material authoring system.

### Fully general 3D engine support
Mesh-heavy workflows, skeletal animation, and arbitrary 3D scene composition are out of scope.

### Automated asset upgrades with no author review
Automatic migration may bootstrap spatial data, but it must not produce a "complete" result that skips author review. Fully automated, review-free spatial upgrades are explicitly out of scope.

---

## 35. Content Team Onboarding

Authors must be able to work with Presentation Core without reading the source code.

### Required onboarding deliverables

The following must exist before Phase 8 exit:

- **Presentation Mode Guide**: what modes exist, how to select them, what each mode implies
- **Spatial Authoring Guide**: how to use the Elevation Brush, Prop Placement Tool, and Spatial Map Inspector
- **Capability Tier Guide**: what each tier supports, how to preview tiers, how to author fallback behavior
- **Asset Preparation Guide**: what metadata is required, how to check asset compatibility, what the pipeline tags mean
- **Migration Guide**: how to migrate a classic project, what the staging workflow looks like, how to handle unresolved items
- **Diagnostics Reference**: what each diagnostic class means, what action to take for each

### Onboarding validation

Before Phase 8 exit, at least one team member who was not involved in Presentation Core implementation must complete the authoring guide and successfully produce a valid spatial map scene. Their feedback must be incorporated before the release gate is signed off.

---

## 36. Final Directive

Do not build "HD-2D support."

Do not "upgrade the renderer."

Do not "experiment with a spatial mode."

Build a **Presentation Core** that owns visual interpretation as a first-class engine responsibility, and deliver it through a strict bounded-step program with:

- hard validation at every phase gate
- hard ownership enforced at every code review
- hard migration rules with no silent completions
- hard release gates that cannot be partially waived
- hard threading and allocation contracts established before implementation
- hard performance budgets established before content scales up
- hard scene-family contracts written before MapScene ships

That is the only version of this plan that has a real chance of surviving contact with a large repo, a growing content team, and the accumulated pressure to ship "just the visual part" without the infrastructure that makes it supportable.

Every section of this document exists because the alternative has been tried before and failed in a predictable way. The structure is not bureaucracy. It is the minimum viable skeleton of a subsystem that can be maintained by people who were not in the room when it was built.

---

*End of document. All sections are required. No section is optional polish.*
