# URPG Planning Addendum — Native Feature Absorption Strategy

Date: 2026-04-13  
Status: approved direction / pending execution  
Scope: engine, editor, runtime, data model, and compat planning

This addendum updates the working plan for URPG.

## Strategic decision

URPG will absorb high-value feature sets commonly delivered by large RPG Maker plugin ecosystems into the core product as first-class engine/editor/runtime systems.

The goal is **not** to recreate a giant plugin stack as the main user-facing solution.
The goal is to make URPG itself natively capable enough that advanced menu, message, battle, save, HUD, event, and data features feel built in.

Compat and plugin work still matter, but they are now a supporting lane rather than the primary product destination.

## What this changes

- Compat remains important for import, validation, behavior study, migration, and regression detection.
- Compat is no longer the intended end-state delivery model for core gameplay and editor capability.
- Native subsystem ownership becomes the main roadmap target after the current compat hardening lane is stable enough.
- Core UX and gameplay power should not require a third-party plugin layer to feel complete.

## Core rule change

From this point forward, "plugin parity" is measured by one of these outcomes:

1. Native URPG subsystem ownership with editor/runtime support.
2. Compat/import support used as a bridge for migrated projects.
3. Extension hooks for optional or project-specific behavior.

It is **not** measured by whether URPG ships a one-for-one plugin clone set.

## Native-first capability absorption targets

These are the feature families URPG should absorb into the product itself.

### Priority 1 — foundation systems

- UI / Menu Core
  - advanced command windows
  - dynamic menu composition
  - actor panes, categories, command injection, state-aware visibility
  - reusable layout regions and skinning hooks
- Message / Text Core
  - rich text, escapes, portraits/faces, name boxes, timing controls
  - message flow control, choice UX, formatting parity where needed
  - editor preview and localization-aware text inspection
- Battle Core
  - action sequencing hooks
  - targeting rules
  - damage/formula extension points
  - turn/phase pipelines
  - state/buff/debuff lifecycle ownership
  - battle UI integration points
- Save / Data Core
  - extensible save metadata
  - slot presentation and integrity layers
  - native schema-owned extension data
  - migration-safe serialization paths

### Priority 2 — gameplay authoring systems

- Event Command Expansion Framework
- HUD / Overlay Framework
- Skills / States / Items / Equipment rule framework
- Map / Movement / interaction framework
- Animation / effects orchestration framework
- Audio behavior and UX hooks exposed as native authoring controls

### Priority 3 — editor-facing productization

- subsystem inspectors instead of plugin parameter walls
- live previews for UI/message/battle presentation
- validation and diagnostics built into editor workflows
- conflict detection expressed at the system/data-contract level
- import upgrade wizards for migrated MZ content

### Priority 4 — extension surface

- thin scripting API for edge cases
- project-specific extensions
- community add-ons
- optional experimental systems
- compatibility shims and translators

## Revised role of compat and plugins

### Compat lane responsibilities

Compat should now exist to:

- import MZ projects with trustworthy behavior capture
- validate expected behavior against curated fixtures
- identify missing native subsystem coverage
- provide migration confidence during project upgrade
- isolate foreign behavior without degrading native URPG systems

### Plugin lane responsibilities

Plugins/extensions should now exist for:

- optional mods
- experimental or niche systems
- project-local scripting
- third-party integrations
- non-core feature packs

Plugins should **not** be the main answer for features that URPG needs in order to feel complete as a product.

## Revised execution lanes

### Lane 1 — finish current compat hardening enough to be trustworthy

Complete the current Phase 2 work to the point where compat is reliable for:

- import confidence
- fixture-based behavior verification
- diagnostics and regression tracking
- migration support for real-world MZ projects

This lane is about trustworthiness and containment, not about making compat the permanent product center.

### Lane 2 — start the Native Feature Absorption Program

Begin formal native ownership work for these subsystems:

1. UI / Menu Core  
2. Message / Text Core  
3. Battle Core  
4. Save / Data Core

Each subsystem must define:

- runtime owner
- editor surface
- schema/data contract
- migration/import rules
- deterministic test coverage
- extension points

### Lane 3 — connect native systems to editor workflows

For each absorbed feature family, the editor must expose:

- inspectors
- previews
- validation
- diagnostics
- authoring affordances
- migration-aware upgrade tools

### Lane 4 — reduce dependence on plugin-shaped thinking

Do not frame major roadmap progress as "what plugin do we need next?"
Frame it as:

- what subsystem should own this behavior?
- what editor workflow should expose it?
- what schema should define it?
- what compat evidence is needed to verify it?

## Immediate planning consequences

- Native subsystem design must take priority over building a huge URPG-side plugin catalog.
- High-value behaviors inspired by advanced MZ plugin ecosystems should be redesigned as first-class URPG capabilities.
- Compat fixtures should be used as behavior references and regression anchors, not as proof that plugin-shaped delivery is the target architecture.
- Any proposed "core plugin" should be challenged first: should this actually be a built-in system?

## Acceptance criteria for absorbed capabilities

A capability is not considered successfully absorbed unless it has all of the following:

- native runtime implementation
- explicit editor surface
- data/schema representation
- tests covering expected behavior
- diagnostics when misconfigured
- extension points for non-core customization
- migration story from compat/imported content when relevant

## Near-term deliverables

### Deliverable A — native-first ownership matrix

Create a matrix mapping:

- capability family
- current compat/plugin source of truth
- future URPG subsystem owner
- editor panel/inspector owner
- runtime module owner
- migration/import notes
- test anchor location

Initial seeded inputs from current routed compat anchors:

| Capability slice | Routed compat evidence | Current compat source of truth | Future URPG subsystem owner | Editor surface owner | Runtime owner | Migration/import notes | Test anchor location |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Menu composition and command routing | Menu-stack reload plus menu-presentation reload verify `openMenu`, `openOptions`, command-window refresh, and routed layout/HUD branches survive reload | `VisuStella_MainMenuCore_MZ`, `VisuStella_OptionsCore_MZ`, `CGMZ_MenuCommandWindow`, `AltMenuScreen_MZ`, `MOG_BattleHud_MZ` fixture commands | UI / Menu Core | Scene/menu inspector plus command-window layout inspector | Native menu scene graph plus command registry | Imported menu plugins should map command injection, command visibility rules, and default-route settings into native menu schema instead of opaque plugin params | `tests/compat/test_compat_plugin_fixtures.cpp` routed reload anchors for menu-stack and menu-presentation |
| Menu presentation layout and dashboard chrome | Menu-presentation reload and library-dashboard reload show menu command chrome coexisting with alt-menu layout and content-entry routes | `CGMZ_MenuCommandWindow`, `AltMenuScreen_MZ` and routed dashboard fixture wiring | UI / Menu Core | Menu layout composer, panel docking rules, style/skin preview | Native menu presenter layer with deterministic region/layout contracts | Compat import should split visual layout concerns from command wiring so imported projects can upgrade presentation without preserving plugin-specific parameter walls | `tests/compat/test_compat_plugin_fixtures.cpp` library-dashboard and menu-presentation reload anchors |
| Library / codex navigation surfaces | Codex reload plus library-dashboard reload verify encyclopedia, book, and quest routes remain addressable after reload and route through shared dashboard entry points | `CGMZ_Encyclopedia`, `EliMZ_Book`, `Galv_QuestLog_MZ` routed fixture commands | UI / Menu Core first, with later content/codex extension hooks | Codex/library inspector, quest-log inspector, route preview surface | Native codex/query runtime under menu scene ownership | Imported encyclopedia/book/quest plugins should become native content-view definitions with preserved route IDs and fallback compat shims only for unsupported custom commands | `tests/compat/test_compat_plugin_fixtures.cpp` codex and library-dashboard reload anchors |
| Presentation overlays tied to routed state | Presentation-family reload plus menu-presentation reload keep layout, HUD, and motion routes callable across reload boundaries | `AltMenuScreen_MZ`, `MOG_BattleHud_MZ`, `MOG_CharacterMotion_MZ` fixture commands | UI / Menu Core for layout and HUD, Battle Core for battle HUD state, later presentation FX layer for motion hooks | Menu preview, HUD preview, battle overlay inspector | Native HUD/layout presenter plus battle presentation adapter | Import should preserve overlay intent and routing while translating display primitives into native presenter contracts rather than plugin command names | `tests/compat/test_compat_plugin_fixtures.cpp` presentation and menu-presentation reload anchors |
| Message flow, text layout, and dialogue chrome | Message-text reload anchor now verifies routed speaker, narration, and system dialogue modes survive plugin reload while still driving `drawTextEx`, `textWidth`, `textSize`, and face rendering parity through `Window_Base` | `Window_Base` text/face compat surfaces plus routed message-text fixture wiring | Message / Text Core | Dialogue inspector, rich-text preview, face/namebox layout preview, localization-aware validation | Native message runner plus text layout engine | Import should translate escape codes, timing rules, portraits/faces, and message-box variants into native dialogue schema with a compat fallback only for unsupported control flows | `tests/compat/test_compat_plugin_fixtures.cpp` message-text reload anchor, `tests/unit/test_window_compat.cpp`, and blueprint message/text contracts |
| Save slot metadata, autosave routing, and recovery UX | Save-data reload anchor verifies slot metadata and autosave flows stay aligned with routed menu/dashboard entry points across plugin reload, save-data lifecycle failure coverage projects routed dashboard failures through JSONL/report/panel while authoritative slot data remains intact, and migration-path evidence now proves imported save metadata can be normalized into runtime-facing shape | `DataManager` compat save/header APIs plus routed save-data fixture wiring | Save / Data Core | Save-slot inspector, save metadata panel, recovery diagnostics view, template save-policy settings | Native save catalog, metadata index, and recovery orchestrator | Imported save plugins should map slot presentation, header-extension metadata, autosave affordances, and recovery flags into native save schema rather than opaque plugin config | `tests/compat/test_compat_plugin_fixtures.cpp` save-data reload anchor, `tests/compat/test_compat_plugin_failure_diagnostics.cpp` save-data failure projection, `tests/unit/test_data_manager.cpp`, `tests/unit/test_migration_runner.cpp`, and save runtime/recovery tests |
| Lifecycle dependency recovery and reload safety | Core reload recovery and weekly routed unload diagnostics prove command availability, dependency gating, and unload warnings are first-class behavior references | `VisuStella_CoreEngine_MZ` dependency contract plus routed unload diagnostics in failure fixtures | Shared runtime lifecycle manager supporting UI / Menu Core, Battle Core, and Save / Data Core | Diagnostics panels and subsystem health views | Plugin/runtime lifecycle coordinator with deterministic reload policy | Native absorption work should preserve explicit dependency health signals and routed failure reporting so imports can fail closed during upgrade | `tests/compat/test_compat_plugin_fixtures.cpp` dependency recovery anchor and `tests/compat/test_compat_plugin_failure_diagnostics.cpp` routed unload regressions |

Near-term interpretation:

- The strongest routed anchors already justify starting UI / Menu Core ownership design immediately.
- Battle Core ownership should treat HUD state as a routed presentation client, not the owner of menu/dashboard composition.
- Message / Text Core now has a first routed anchor, but it still needs deeper choice-flow, timing-control, and failure-path evidence before the subsystem is fully specified.
- Save / Data Core now has routed reload, routed diagnostics projection, and migration-path evidence, so it is ready to move from initial draft into an implementation slice.
- Battle Core ownership now has a first routed battle-flow anchor in addition to deterministic battle-flow tests and HUD/presentation client evidence, but it still needs broader routed combat coverage over time.

### Deliverable B — first absorption wave specs

Write subsystem design docs for:

- UI / Menu Core
  - Initial draft: `docs/UI_MENU_CORE_NATIVE_SPEC.md`
- Message / Text Core
  - Initial draft: `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
- Battle Core
  - Initial draft: `docs/BATTLE_CORE_NATIVE_SPEC.md`
- Save / Data Core
  - Initial draft: `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`

### Deliverable C — roadmap rewiring

Update execution planning so that post-compat work is tracked as native subsystem delivery, not as plugin-count growth.

## Non-goals

These are specifically **not** the goal of this direction update:

- shipping a one-for-one clone catalog of commercial plugin ecosystems
- depending on external plugin stacks for core URPG value
- treating plugin parameter sheets as the primary UX model for advanced engine features
- letting compat become the permanent architecture center of the product

## Summary

URPG should not become "RPG Maker plus a bigger plugin list."

URPG should become a stronger native RPG creation product where the most valuable advanced capabilities are part of the engine, part of the editor, and part of the runtime by default.

Compat remains essential, but as an adapter, validation, and migration lane.
Native subsystem ownership is now the main path.
