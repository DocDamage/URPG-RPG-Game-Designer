# UI / Menu Core Native-First Spec

Date: 2026-04-14  
Status: active implementation baseline (runtime core slices landed; editor/schema/migration/release closure pending)  
Scope: runtime ownership, editor ownership, schema, migration, diagnostics, and test anchors for native UI/Menu Core absorption

## Last landed progress (2026-04-15)

- Native runtime interaction slices are active in:
  - `engine/core/ui/menu_command_registry.h` (state-aware visibility/enabled evaluation)
  - `engine/core/ui/menu_route_resolver.h` (primary->fallback route behavior)
  - `engine/core/ui/menu_scene_graph.h` (confirm/cancel/focus traversal/recovery/blocked-command metadata)
- Registry helper integration path is active (`setCommandStateFromRegistry`) for switch/variable-driven command state.
- Unit coverage expanded in `tests/unit/test_menu_core.cpp` for command activation, focus traversal, blocked command reasons, and recovery behavior.
- Focused validation lane is active:
  - `ctest --test-dir build/dev-ninja-debug -R "MenuSceneGraph|MenuRouteResolver|MenuCommandRegistry" --output-on-failure`

## Next steps

- Ship menu authoring/editor surfaces with production workflows:
  - menu structure inspector
  - layout composer
  - command inspector
  - live preview panel
- Finalize schema + import mapping for fallback-route and state-rule migration from compat evidence.
- Add integration anchors beyond unit checks for runtime+editor behavior parity.

## Purpose

UI / Menu Core becomes the native owner for menu composition, command routing, dashboard chrome, codex entry points, and presentation layout that currently appear across compat fixtures and plugin ecosystems.

The subsystem should absorb the high-value behavior proven by the routed compat anchors while avoiding a plugin-parameter-driven product model.

## Source evidence

Primary routed evidence currently comes from:

- `menu-stack reload`
- `library-dashboard reload`
- `menu-presentation reload`
- `codex reload`
- dependency recovery after core reload

These anchors show that the product needs first-class ownership of:

- deterministic command composition
- dashboard entry routing
- reusable command-window chrome
- menu layout regions
- codex/library entry surfaces
- lifecycle-safe reload semantics

## Ownership boundary

UI / Menu Core owns:

- scene-level menu composition
- command registry for menu-visible actions
- command visibility/enabled-state rules
- layout regions, panes, dashboard blocks, and visual chrome
- menu-facing content routes such as codex, quest log, encyclopedia, and book/library entry points
- preview-safe reload of menu layouts and content routing state

UI / Menu Core does not own:

- battle simulation rules
- save serialization internals
- localization storage
- event runtime sequencing

It consumes those systems through typed interfaces.

## Runtime model

### Core runtime objects

- `MenuSceneGraph`
  - authoritative tree of scenes, panes, regions, and command hosts
- `MenuCommandRegistry`
  - declarative command definitions with IDs, labels, route targets, enabled rules, and visibility rules
- `MenuRouteResolver`
  - resolves command activation into native route targets such as codex views, quest logs, options panels, and save panels
- `MenuLayoutModel`
  - data-only layout description for regions, columns, stacks, tabs, and dashboard cards
- `MenuPresentationState`
  - ephemeral runtime state for selection, focus, active tab, preview overlays, and animation-safe transitions

### Runtime contracts

- command enumeration order is deterministic
- route resolution is schema-owned, not plugin-command-string-owned
- menu layout changes are hot-reload-safe when they do not change authoritative save schema or runtime ownership boundaries
- UI-only presentation changes never own gameplay state
- codex/library surfaces resolve through native route IDs, not imported plugin function names

## Editor surfaces

UI / Menu Core should ship with these editor owners:

- `Menu Structure Inspector`
  - scene tree, route targets, command ordering, and visibility rules
- `Menu Layout Composer`
  - region-based editor for panes, columns, cards, list blocks, and dock zones
- `Command Window Inspector`
  - visible rows, command grouping, command categories, and enable-state logic
- `Menu Preview Panel`
  - live preview for desktop resolutions, template skins, and focus flow
- `Codex / Library Route Inspector`
  - configures encyclopedia, quest log, book, glossary, and similar entry views as native content routes

## Schema and data contracts

### Required schemas

- `menu_scenes.json`
  - scene definitions, route graph, default scene entry points
- `menu_layouts.json`
  - layout regions, pane placements, template variants, breakpoints
- `menu_commands.json`
  - command IDs, labels, icons, visibility rules, enable rules, route bindings
- `menu_content_routes.json`
  - codex/library/quest/save/options route descriptors and backing data source references

### Schema rules

- menu command IDs are stable and migration-safe
- route targets are explicit typed records, not freeform strings
- layout schema is data-only and previewable without executing gameplay code
- imported compat metadata must be preserved as mapping notes during upgrade, not as the native source of truth

## Migration and import rules

### Import goals

- translate menu plugins into native scene/layout/command records where possible
- preserve route intent even when specific plugin commands are not retained
- separate visual layout concerns from command behavior concerns during import

### Mapping strategy

- `VisuStella_MainMenuCore_MZ` maps primarily into command groups, command ordering, and scene entry structure
- `VisuStella_OptionsCore_MZ` maps into native settings panels and option-route descriptors
- `CGMZ_MenuCommandWindow` maps into command-window presentation and command visibility metadata
- `AltMenuScreen_MZ` maps into layout regions and pane composition
- `CGMZ_Encyclopedia`, `EliMZ_Book`, and `Galv_QuestLog_MZ` map into native content-route definitions under UI/Menu ownership

### Failure handling

- unsupported plugin behavior is recorded as import diagnostics with retained compat notes
- imported routes can remain bridged through compat only when no native route descriptor exists yet
- conflicts between multiple menu plugins are resolved into explicit upgrade diagnostics instead of silent precedence rules

## Diagnostics and safety

UI / Menu Core diagnostics should include:

- duplicate command IDs
- unreachable route targets
- invalid layout region references
- layout overflow or missing required panes
- command visibility rules referencing missing state
- imported plugin mappings that could not be normalized cleanly

Safe-mode expectations:

- disable nonessential presentation overlays
- preserve core command navigation and save/options escape routes
- surface import/migration warnings without blocking basic menu access

## Extension points

Allowed extension points:

- custom command providers
- custom content-route providers
- layout skin providers
- menu preview decorators

Disallowed extension patterns:

- direct ownership of authoritative save state from menu plugins
- ad hoc mutation of native route graph without schema updates
- hidden command ordering side effects that bypass the registry

## Test anchors

The subsystem should inherit and later replace these evidence paths:

- `tests/compat/test_compat_plugin_fixtures.cpp`
  - menu-stack reload
  - library-dashboard reload
  - menu-presentation reload
  - codex reload
  - dependency recovery after core reload
- future native tests should add:
  - menu schema migration tests
  - layout preview snapshot tests
  - route-resolution unit tests
  - reload-safe state preservation tests
  - import upgrade tests from compat mappings into native menu schema

## First implementation slice

Phase 1 of UI / Menu Core absorption should deliver:

- native menu command registry
- native menu scene graph
- command-window presentation model
- options and codex route descriptors
- editor structure inspector and preview panel
- import mapping for the current strongest compat menu anchors

<!-- WAVE1_CHECKLIST_START -->
## Wave 1 Closure Checklist (Canonical)

_Managed by `tools/docs/sync-wave1-spec-checklist.ps1`. Do not edit manually._
_Canonical source: [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md](WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)_

### Universal closure gates

- [ ] Runtime ownership is authoritative and compat behavior for this subsystem is bridge-only.
- [ ] Editor productization is complete (inspect/edit/preview/validate) with diagnostics surfaced.
- [ ] Schema contracts and migration/import paths are explicit, versioned, and test-backed.
- [ ] Deterministic validation exists (unit + integration + snapshot where layout/presentation applies).
- [ ] Failure-path diagnostics and safe-mode/bounded fallback behavior are explicitly documented and tested.
- [ ] Release evidence is published in status docs and gate snapshots are recorded.

### UI / Menu Core specific closure gates

- [ ] Menu runtime route/command ownership is authoritative and no plugin command string owns route resolution.
- [ ] Menu authoring surface (structure/layout/command/preview) supports inspect/edit/validate workflows.
- [ ] Fallback routes and command-state migration paths are schema-defined and test-backed.

### Closure sign-off artifact checklist

- [ ] Runtime owner files listed (header + source).
- [ ] Editor owner files listed.
- [ ] Schema and migration files listed.
- [ ] Latest deterministic test outputs recorded.
- [ ] README.md, docs/PROGRAM_COMPLETION_STATUS.md, and URPG_Blueprint_v3_1_Integrated.md updated.
<!-- WAVE1_CHECKLIST_END -->

## Non-goals for this slice

- recreating every MZ menu plugin as a built-in one-off feature
- absorbing battle simulation ownership into menu code
- making plugin parameter sheets the primary authoring UI
- shipping theme-only polish before the route/model layer is authoritative
