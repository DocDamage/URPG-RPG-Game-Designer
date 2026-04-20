# Runtime VFX Cue Pipeline Design

**Date:** 2026-04-20
**Scope:** Shared runtime visual-effects cue pipeline with battle-first validation and hybrid world/overlay output

## Goal

Add a native runtime VFX foundation that can drive hit effects, spell bursts, and emphasis cues through the existing presentation architecture without making battle logic own render behavior.

The first shipped slice should establish:

- a shared, deterministic `EffectCue` contract
- a shared resolver that expands cues into reusable runtime effect instances
- hybrid output support for both world-anchored and overlay-style effects
- battle as the first fully wired emitter and validation path

This slice is intended to create a durable native foundation for runtime polish, not to deliver a full cinematic or editor-authored VFX stack in one pass.

## Problem

The roadmap now needs a new native feature slice beyond the documented Wave 1 and Wave 2 baseline closure work. The chosen area is player-facing runtime visuals, specifically moment-to-moment feedback such as:

- hits
- spell bursts
- guard emphasis
- heal emphasis
- critical-hit punctuation
- defeat/phase emphasis

Today, the repo has presentation runtime infrastructure, environment command handling, and battle/map/menu translators, but it does not yet have a dedicated shared runtime effect pipeline that:

- accepts gameplay-resolved effect facts as deterministic presentation cues
- expands those cues into reusable runtime effect instances
- supports both world-space and overlay-space presentation outputs
- keeps the visual interpretation layer separate from battle-state ownership

If this work is done as battle-local scene logic, the engine will likely duplicate the same effect concepts when map gameplay later needs them. If it is done as a full data-driven effect-graph/editor lane now, the initial slice will grow too large and delay usable runtime integration.

## Chosen Approach

Implement a dedicated runtime VFX cue pipeline with three layers:

1. `EffectCue` emission from authoritative gameplay code
2. shared cue resolution into one or more `ResolvedEffectInstance`s
3. presentation translation from resolved instances into renderable commands

Battle will be the first wired runtime client, but the cue schema, resolver, and presentation translation layers should be shared from day one.

This approach intentionally prioritizes runtime triggering and integration over deep authoring tooling. V1 should prove deterministic triggering, hybrid output, and bounded fallback behavior before expanding into richer editor-facing composition tools.

## Architecture

### Layer 1: EffectCue emission

Gameplay systems emit compact, deterministic `EffectCue` records at the moment gameplay truth is finalized.

For v1, the initial cue family should cover:

- cast/start emphasis
- hit confirm
- critical hit
- guard/reduced impact
- miss
- heal
- defeat
- battle phase/banner emphasis

These cues are facts, not effect scripts. They should carry only the information needed to support downstream visual resolution, such as:

- source and target anchors
- cue kind and emphasis class
- optional element/theme tag
- magnitude or intensity tier
- deterministic ordering/timing metadata

They must not carry backend-specific render details, shader identifiers, or scene-specific ad hoc state.

### Layer 2: Shared effect resolution

A shared resolver transforms `EffectCue` records into one or more `ResolvedEffectInstance`s using a preset catalog plus explicit fallback rules.

This layer is where hybrid output is decided. For example:

- a heavy hit may become a world impact burst plus a short overlay flash
- a critical hit may become a world burst plus a stronger overlay punctuation accent
- a miss may resolve to overlay emphasis only

This is the key architectural seam for the feature. It keeps:

- battle logic from embedding visual-composition rules
- presentation translation from reinventing gameplay-driven mapping logic
- future map-scene adoption from requiring a system redesign

### Layer 3: Presentation translation

The presentation layer consumes resolved instances and translates them into renderable commands that fit the current presentation frame build path.

This layer owns visual interpretation only:

- choosing command families
- applying layer placement
- preserving anchor information
- degrading gracefully on lower tiers

Gameplay remains the only owner of combat truth, and the presentation layer remains the only owner of visual meaning.

## Responsibilities and Boundaries

### New shared responsibilities

The new runtime VFX subsystem should own:

- effect cue type definitions
- cue ordering semantics
- preset-backed cue expansion logic
- effect fallback policy
- resolved effect instance representation
- presentation translation from resolved effects to frame commands

### Battle responsibilities after this change

Battle should own:

- deciding when a cast, hit, miss, crit, heal, guard, defeat, or phase emphasis actually occurred
- emitting cues in deterministic order as part of resolved action flow
- passing cues through a narrow enqueue/report interface instead of constructing render commands directly

Battle should not own:

- render command construction
- hybrid world/overlay composition rules
- preset selection logic beyond cue-kind emission

### Presentation responsibilities after this change

Presentation should own:

- translating resolved effect instances into runtime frame commands
- anchoring effects into world or overlay space
- preserving ordering and layering
- applying tier-aware simplification or collapse rules

Presentation should not own:

- deciding whether gameplay events happened
- inventing battle-state outcomes from visual state

## Proposed Components

### Shared effect contract and runtime types

- `engine/core/presentation/effects/effect_cue.h`
  - deterministic cue kinds and compact payload structure
- `engine/core/presentation/effects/effect_instance.h`
  - resolved runtime-ready effect instances with placement mode, lifetime, layer, and emphasis strength

### Shared resolution layer

- `engine/core/presentation/effects/effect_catalog.h`
- `engine/core/presentation/effects/effect_catalog.cpp`
  - built-in preset catalog for v1
- `engine/core/presentation/effects/effect_resolver.h`
- `engine/core/presentation/effects/effect_resolver.cpp`
  - shared cue-to-instance expansion and fallback behavior

### Shared presentation translation layer

- `engine/core/presentation/effects/effect_translator.h`
- `engine/core/presentation/effects/effect_translator.cpp`
  - resolved-instance translation into presentation frame commands

### Battle integration

- battle runtime files under `engine/core/battle/` and/or `engine/core/scene/battle_scene.*`
  - emit `EffectCue` records at deterministic action/phase milestones through a narrow interface

### Test surfaces

- `tests/unit/test_effect_cue.cpp`
- `tests/unit/test_effect_resolver.cpp`
- `tests/unit/test_battle_effect_cues.cpp`
- `tests/unit/test_presentation_effect_translation.cpp`
- `tests/unit/test_presentation_runtime.cpp`

The exact file split can vary during implementation, but the subsystem boundary should stay shared rather than battle-local.

## Initial V1 Preset Catalog

V1 should use a code-defined preset catalog rather than a full editor-authored graph pipeline.

Recommended initial presets:

- `cast_small`
- `cast_large`
- `impact_light`
- `impact_heavy`
- `heal_pulse`
- `crit_burst`
- `guard_spark`
- `miss_sweep`
- `defeat_fade`
- `phase_banner`

This keeps the first slice small while still allowing reusable mapping behavior in the resolver.

## Data Flow

1. Battle resolves authoritative gameplay state.
2. Battle emits ordered `EffectCue` records when gameplay facts are finalized.
3. The shared resolver expands each cue into one or more `ResolvedEffectInstance`s.
4. The presentation layer translates resolved instances into `PresentationFrameIntent` commands.
5. The render backend consumes those commands as world-anchored and/or overlay-style runtime effects.

Key rules for this path:

- gameplay never emits low-level render commands
- presentation never decides whether a battle event happened
- effect ordering is deterministic and testable
- hybrid output is created only by the shared resolver/translation path
- lower tiers must preserve readability through explicit downgrade behavior

## Hybrid Output Model

The first slice should support both:

- `world-anchored effects`
  - impact bursts
  - ground flashes
  - rings
  - projectile-like travel accents
- `overlay emphasis effects`
  - hit punctuation
  - short flashes
  - critical-hit emphasis
  - phase/banner accents

Not every cue must resolve to both output families, but the system must make hybrid composition a first-class supported path.

## Error Handling and Fallbacks

V1 should degrade gracefully instead of silently dropping visual feedback.

### Missing preset

If a cue resolves to a missing preset, the resolver should:

- select a documented fallback preset
- keep deterministic output ordering
- record a diagnostics warning where the engine already exposes effect/presentation diagnostics

### Missing world anchor

If world anchoring data is absent or unusable:

- downgrade to overlay emphasis where possible
- avoid dropping the cue entirely unless no meaningful fallback exists

### Overlay-unavailable path

If overlay emphasis is unsupported in a given path:

- preserve a world-space effect when one exists
- otherwise use the simplest still-readable fallback behavior available to the active tier

### Lower render tiers

When the presentation tier cannot support richer multi-part output:

- collapse multi-part effects into a smaller resolved set
- preserve the most informative effect component
- keep collapse rules deterministic and test-backed

### Burst-heavy moments

If multiple cues arrive in a short burst:

- preserve cue ordering
- cap expansion by policy
- use deterministic truncation/collapse rather than unbounded command growth

## In Scope

- shared runtime VFX cue contract
- shared cue resolver and preset catalog
- shared presentation translation for effect instances
- battle-first cue emission and integration
- hybrid world/overlay runtime effect output
- deterministic fallback policy
- focused unit/integration validation and presentation-lane verification
- canonical docs for the new design and later implementation status alignment

## Out of Scope

- a full editor-authored effect composer
- full schema-driven effect graphs
- map-scene runtime integration in the first implementation slice
- backend-specific cinematic rendering work
- replacing or redefining existing battle authority boundaries
- unrelated presentation refactors

Map-scene adoption should be enabled by the shared contract but intentionally deferred until after the battle-first slice is stable.

## Testing Strategy

Testing should be organized around the real subsystem risks rather than only checking effect existence.

### 1. Cue contract tests

Lock the `EffectCue` schema and deterministic behavior:

- stable default values
- explicit optional-field behavior
- deterministic ordering keys
- no render-backend-specific data leaking into the cue layer

### 2. Battle emission timeline tests

Verify emitted cue timelines around authoritative action resolution:

- cast/start emphasis before impact
- impact after real hit resolution
- crit emphasis only when a crit actually occurs
- miss replaces impact rather than supplementing it
- defeat/finish cues only appear after final target outcome

These tests are the primary guardrail for the gameplay/presentation boundary.

### 3. Resolver mapping tests

Prove that the shared resolver:

- expands one cue into one or more resolved instances
- supports deterministic hybrid expansion
- routes missing presets into documented fallbacks
- collapses meaningfully on lower tiers
- stays within bounded expansion policy

### 4. Translation tests

Verify that translation preserves intent rather than merely producing commands:

- world-anchored instances preserve anchor and world layer placement
- overlay instances preserve overlay routing and emphasis priority
- mixed outputs preserve relative order from resolved instance input
- translation does not mutate gameplay-facing data

### 5. Frame composition tests

Extend the presentation runtime lane to prove:

- effect commands compose with existing environment commands
- effect output does not clobber dialogue/readability overrides or frame-building rules
- identical cue input produces identical `PresentationFrameIntent` output
- total command growth remains bounded and observable

### 6. Stress and envelope tests

Add focused validation for burst-heavy battle moments:

- repeated multi-target hits
- repeated crit/heal emphasis
- bounded command-count growth
- deterministic truncation/collapse when limits are reached

## Validation Plan

Implementation of this design should extend the presentation-focused verification story rather than invent a separate ad hoc lane.

Expected validation includes:

- focused unit tests for cue contracts, resolver behavior, battle emission, and translation
- `tests/unit/test_presentation_runtime.cpp` coverage expansion
- presentation gate re-run through:
  - `ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure`
- one release-validation expansion proving battle-driven effect output stays within a bounded frame envelope

## Risks and Controls

### Risk: battle logic starts encoding visual-composition rules

Control: keep battle emission limited to compact cue records and prohibit direct render-command construction from battle runtime code.

### Risk: resolver becomes battle-specific despite shared intent

Control: design cue kinds and resolved-instance types around generic runtime events and anchors, not battle-scene-only classes.

### Risk: hybrid output grows command counts too quickly

Control: enforce explicit expansion caps and test burst-heavy truncation/collapse behavior.

### Risk: lower-tier rendering silently drops readability

Control: make fallback presets and collapse rules explicit, deterministic, and directly tested.

### Risk: future map-scene adoption requires a redesign

Control: make the cue contract, resolver, and translator shared from the first slice even though only battle is wired initially.

## Expected Outcome

After this slice lands, URPG should have a truthful native runtime VFX foundation that:

- gives battle readable, more satisfying hit/spell/emphasis feedback
- preserves the project rule that gameplay owns truth and presentation owns interpretation
- supports both world and overlay runtime effects through one shared path
- remains deterministic, bounded, and testable
- provides a real contract that map gameplay can adopt next without re-architecting the system

## Completion Definition

This design is satisfied when the first implementation slice achieves all of the following:

- battle emits a deterministic, reviewable effect cue timeline
- the shared resolver is the only place hybrid cue expansion logic lives
- the presentation runtime can emit both world and overlay effect commands from the same cue stream
- lower-tier fallback behavior remains readable and test-backed
- burst-heavy moments remain bounded under explicit envelope rules
- canonical docs and validation evidence describe the resulting scope truthfully
