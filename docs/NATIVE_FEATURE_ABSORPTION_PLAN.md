# URPG Native Capability Expansion Plan

Date: 2026-04-15  
Status: active execution  
Scope: native runtime, editor workflows, schema/migration, compat bridge exit, and advanced capability expansion

Cross-cutting debt closure, documentation-truth alignment, and intake-governance companion: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

This plan replaces the previous addendum and becomes the single roadmap for turning URPG into a high-power engine for multiple game styles, while preserving import and migration trust for RPG Maker content.

## Strategic position

URPG's core value is native ownership, not plugin accumulation.

- Compat exists to import, verify, migrate, and contain risk.
- Native systems own long-term product capability.
- Extensions remain optional for edge behavior, not required for core power.

## Planning goals

1. Keep current native-first Wave 1 (UI/Menu, Message/Text, Battle, Save/Data) as the execution baseline.
2. Expand the roadmap to support broader game styles through additional high-leverage systems.
3. Integrate external-repo-inspired capabilities as first-class native tracks, not side appendices.

## Integrated capability model

### Tier A: Core ownership (must ship as native)

- UI/Menu Core
- Message/Text Core
- Battle Core
- Save/Data Core

### Tier B: Advanced power systems (planned native lanes)

- Gameplay Ability Framework (tags, abilities, conditions, state machines)
- AI Copilot Core (knowledge bridges, streaming services, tool-calling, orchestration) [Landed 2026-04-16]
- Visual Pattern Authoring (painted coordinate patterns for AoE, inventory-like footprints, rule masks)
- Modular Level Assembly (snap/connect block authoring with import helpers)
- Sprite Pipeline Toolkit (atlas packing, crop/trim, animation preview/tuning)
- Procedural Content Toolkit (dungeon generation, FOV, encounter scaffolding)
- Optional 2.5D Presentation Lane (raycast camera/runtime mode for specific project styles)
- Timeline/Animation Orchestration (keyframes, temporary animated effects, event-driven animation control)
- Editor Productivity and Debug Utilities (selected high-value authoring helpers)

### Tier C: Optional extension lane

- thin scripting API
- project-specific add-ons
- experiment-only modules

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
| `juanfgs/raycaster` | Raycast runtime and map tooling concepts | `Optional 2.5D Presentation Lane` |
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

- optional customization points only
- no hidden authority over core save/state/combat/menu/message contracts

## Execution roadmap

## Lane 1: Compat bridge exit (in progress)

Goal: complete trustworthy import/verification/migration bridge and define explicit exit criteria.

Current scope note:

- The QuickJS compat lane is still treated as a fixture-backed contract harness for import/verification work, not as a finished live JS runtime.
- Compat surface labels must stay conservative until placeholder-backed runtime paths are actually closed.

Deliverables:

- expand routed conformance depth over curated profiles
- keep JSONL/report/panel diagnostics in lockstep for all new failures
- publish signed compat exit checklist

## Lane 2: Wave 1 native ownership (in progress)

Goal: convert existing seeded slices into full native delivery.

Latest landed progress (2026-04-15):

- Message/Text runtime ownership slice is active with native rich-text layout and message flow core.
- WindowCompat bridge now emits backend-facing text draw commands (`RenderLayer::TextCommand`) from `Window_Base::drawText`.
- Compat `Window_Message` parity surface and deterministic centered/right wrapped snapshot tests are active.
- Remaining closure is MessageScene-native renderer ownership handoff and full backend text-command consumption across renderer tiers.

Subsystem targets:

1. UI/Menu Core
2. Message/Text Core
3. Battle Core
4. Save/Data Core

- [x] Wave 2 Audio State Sync: Implemented `GlobalStateHub` with "Diff-First" notifications and `StateDrivenAudioResolver` for automated BGM transitions. [Landed 2026-04-16]
- [x] Spatial Presentation: Implemented `SpatialMapOverlay` with elevation sculpting, 3D prop placement, and cinematic camera profiles for battles. [Landed 2026-04-16]
- [x] Editor Tooling: Launched `ElevationBrush` and `PropPlacement` native panels. [Landed 2026-04-16]

Required outcomes per subsystem:

- runtime owner implemented
- editor surface implemented
- schema contracts finalized
- migration mapping implemented
- tests + diagnostics complete

## Lane 3: Wave 2 advanced power systems (new integrated lane)

Goal: expand engine power for multi-genre projects while preserving deterministic core behavior.

### 3.1 Gameplay Ability Framework

- [x] Gameplay tags and condition queries (hierarchical tag support implemented in `engine/core/ability/gameplay_tags.h`).
- [x] Ability definitions, activation/cooldown/cost pipelines (implemented in `engine/core/ability/gameplay_ability.h` and `ability_system_component.h`).
- [x] State machines and transition guards (implemented in `engine/core/ability/ability_state_machine.h`).
- [x] Ability Inspector UI (implemented in `editor/ability/ability_inspector_panel.h`).
- [x] Ability Tasks for async execution (loops/delays implemented in `engine/core/ability/ability_task.h`).
- [x] Effect Modifiers and attribute math (implemented in `engine/core/ability/gameplay_effect.h`).
- [ ] Ability/effect diagnostics and replay-safe execution.

### 3.2 Pattern Field Editor

- [x] Painted 2D/3D pattern resources with JSON serialization.
- [x] Reusable pattern presets (implemented in `engine/core/ability/pattern_field.h`).
- [ ] Reusable pattern presets for skills, items, placement, and interaction masks
- [ ] pattern validation and preview in inspectors

### 3.3 Modular Level Assembly

- [x] Snap-connector block placement system (kernel implemented in `engine/core/level/level_assembly.h`).
- [ ] block libraries and thumbnail generation
- [ ] importer conventions for connector metadata
- [ ] deterministic placement validation

### 3.4 Sprite Pipeline Toolkit

- [x] Atlas generation/packing and trim/crop tooling (CLI utility implemented in `tools/sprite_pipeline/`).
- [x] Schema definition for atlas metadata (implemented in `content/schemas/sprite_atlas.schema.json`).
- [ ] animation-sheet preview/tuning panel
- [ ] build-time artifact generation for runtime consumption

### 3.5 Procedural Content Toolkit

- [x] Dungeon/room/corridor generation primitives (baseline implemented in `engine/core/level/procedural_toolkit.h`).
- [x] FOV and visibility systems (baseline implemented in `engine/core/level/fov_system.h`).
- [ ] encounter/scenario generators tied to deterministic seeds

### 3.6 Optional 2.5D Presentation Lane

- [x] Raycast camera/runtime mode (baseline implemented in `engine/core/render/raycast_renderer.h`).
- [ ] map authoring adapters for 2.5D projects
- [ ] strict isolation from core 2D native contracts

### 3.7 Timeline/Animation Orchestration

- [x] Keyframe/timeline data model (implemented in `engine/core/animation/animation_clip.h`).
- [ ] keyframe/timeline authoring for scene and UI animation
- [ ] transient animated effect spawning hooks
- [ ] deterministic event-to-animation bindings

### 3.8 Editor Productivity Utilities

- [x] Selective adoption of high-value editor utilities (implemented `editor/productivity/editor_utility_task.h`).
- [ ] focus on maintainability and ownership fit, not bulk addon import

## Lane 4: Productization and release hardening

Goal: convert delivered systems into stable production paths.

Deliverables:

- [x] Migration wizard flow for imported projects (implemented as `MigrationWizardPanel` with subsystem-agnostic orchestration).
- [ ] Release-readiness matrix by subsystem.
- [ ] Gate reports proving stability for native + compat lanes.

## Milestone framing

### Milestone M1 (current)

- Lane 1 near-exit hardening
- Lane 2 Wave 1 runtime ownership delivery start

### Milestone M2

- Wave 1 editor/schema/migration closure
- Lane 3 tracks 3.1 to 3.4 seeded and partially implemented

### Milestone M3

- Lane 3 tracks 3.5 to 3.8 implemented to production baseline
- cross-lane integration validation complete

### Milestone M4

- Lane 4 release hardening complete
- full program completion report published

## Definition of complete (100% for this plan)

This plan is complete when:

1. Compat bridge exit criteria are signed off.
2. Wave 1 subsystems are fully native across runtime/editor/schema/migration/test/diagnostics.
3. Wave 2 advanced power systems are delivered at production baseline.
4. Release gates validate native and compat stability with published evidence.

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

- Program status and remaining checklist: `docs/PROGRAM_COMPLETION_STATUS.md`
- Wave 1 specs:
  - `docs/UI_MENU_CORE_NATIVE_SPEC.md`
  - `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
  - `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
  - `docs/BATTLE_CORE_NATIVE_SPEC.md`
