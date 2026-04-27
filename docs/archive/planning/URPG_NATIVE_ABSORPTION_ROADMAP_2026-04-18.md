# URPG Native Absorption Roadmap

Date: 2026-04-18
Status: superseded planning input retained for traceability
Scope: convert the plugin-ecosystem research brief into a repo-ready, native-first execution plan with phases, file targets, issue breakdowns, acceptance criteria, and release gates.

Canonical governance and current execution authority live in:
- `../../PROGRAM_COMPLETION_STATUS.md`
- `../../NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `../../PROGRAM_COMPLETION_STATUS.md`
- `URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`

This file is retained as an earlier native-absorption planning input. It is no longer a standalone execution authority; planning, status, and release-truth updates must route through the documents above.

Related historical inputs:
- `URPG_PLUGIN_ABSORPTION_RESEARCH_2026-04-18.md`

---

## Executive position

URPG should not chase plugin parity by building a giant plugin clone catalog.

URPG should absorb the **highest-value plugin demand centers** as **native engine, editor, schema, migration, and validation lanes**. The target is not “support more plugins.” The target is:

1. replace the most common plugin dependencies with first-class native ownership,
2. import and migrate projects that currently depend on those behaviors,
3. expose modern authoring workflows that are better than plugin parameter sheets,
4. make HD-2D / 2.5D / action-hybrid projects a deliberate product capability,
5. preserve truthfulness about what is fully native, approximated, imported, or unsupported.

This roadmap captured how the research findings were being turned into a staged product plan at the time.

---

## Strategic outcome

When this roadmap is complete, URPG should be able to do all of the following without core dependence on third-party plugin stacks:

- ship modern battle systems and battle UI natively,
- ship modern HUD/menu/dialogue/cutscene systems natively,
- ship quests, codex/bestiary, crafting, professions, and achievements natively,
- support advanced inventory/equipment patterns such as independent items and socket/materia-like systems natively,
- support action, platformer, and hybrid map-interaction models natively,
- support lighting, shadows, camera, depth, and layered presentation natively,
- support HD-2D / 2.5D scene profiles natively,
- import and migrate plugin-heavy RPG Maker projects into native URPG schemas with explicit diagnostics.

---

## Non-goals

This roadmap does **not** aim to:

- recreate every historical plugin one-to-one,
- preserve every plugin’s exact parameter surface forever,
- embed plugin-shaped core behavior as long-term engine architecture,
- promise exact parity with every legacy ecosystem quirk,
- block progress until all import edge cases are solved.

---

## Core principles

### 1. Native ownership first
All major capability lands in runtime, editor, schema, migration, and tests as URPG-native systems.

### 2. Compat is an intake bridge, not the product
Compat/import exists to discover behavior, validate migration, and surface safe fallbacks.

### 3. Category absorption beats plugin cloning
Absorb **battle UI** as a product lane, not 12 separate battle UI plugins.

### 4. Truthful status labels
Every lane must explicitly distinguish:
- `NATIVE_FULL`
- `NATIVE_PARTIAL`
- `IMPORTED_APPROXIMATED`
- `IMPORT_BLOCKED`
- `UNSUPPORTED`

### 5. Editor ownership is mandatory
A feature does not count as shipped if it only exists as runtime code and hidden JSON.

### 6. HD-2D / 2.5D is a first-class design constraint
All major visual systems added by this roadmap must be evaluated for compatibility with:
- standard 2D RPG mode,
- action-hybrid mode,
- HD-2D / 2.5D presentation mode.

---

## Program structure

This roadmap is split into 8 execution phases:

- Phase 0: Intake, classification, and governance
- Phase 1: Native substrate and shared authoring infrastructure
- Phase 2: Battle Core Suite
- Phase 3: UI / HUD / Menu Composition Suite
- Phase 4: Dialogue / Cutscene / Narrative Authoring Suite
- Phase 5: Systems Expansion Suite
- Phase 6: Action / Interaction / Presentation Suite
- Phase 7: HD-2D / 2.5D Native Presentation Lane
- Phase 8: Migration, stabilization, and release proof

---

# Phase 0 — Intake, classification, and governance

## Goal
Build the intake system that decides **which plugin demand clusters become native product lanes**, how they are prioritized, and how imported plugin evidence maps into URPG-native contracts.

## Deliverables

- canonical plugin-category taxonomy,
- plugin-demand absorption scorecard,
- native ownership decision matrix,
- migration evidence schema for plugin-heavy projects,
- issue labels and execution board conventions.

## File targets

### New docs
- `docs/PLUGIN_NATIVE_ABSORPTION_ROADMAP.md`
- `docs/PLUGIN_CATEGORY_TAXONOMY.md`
- `docs/PLUGIN_ABSORPTION_SCORECARD.md`
- `docs/IMPORT_EVIDENCE_AND_NATIVE_MAPPING_POLICY.md`
- `docs/HD2D_25D_COMPATIBILITY_RULES.md`

### New schemas
- `content/schemas/import/plugin_category_evidence.schema.json`
- `content/schemas/import/native_absorption_status.schema.json`
- `content/schemas/import/project_plugin_dependency_report.schema.json`

### New tooling
- `tools/docs/generate_plugin_absorption_matrix.py`
- `tools/ci/check_plugin_absorption_docs.ps1`

## Issue breakdown

### Issue 0.1 — Create canonical plugin-category taxonomy
Define the native categories URPG will absorb.

Acceptance criteria:
- every researched plugin demand cluster maps to exactly one primary category and optional secondary tags,
- categories are stable enough for issue planning and migration diagnostics,
- the taxonomy explicitly includes HD-2D / 2.5D cross-cutting relevance.

### Issue 0.2 — Create native absorption scorecard
Rank each category by:
- user demand,
- strategic value,
- import frequency,
- implementation cost,
- editor burden,
- 2.5D/HD-2D leverage.

Acceptance criteria:
- each category has a weighted score,
- the scorecard produces phase ordering without hand-wavy prioritization,
- the scorecard is documented and reproducible.

### Issue 0.3 — Create import evidence schema
Define how URPG records plugin-derived project features during intake.

Acceptance criteria:
- supports plugin family name, feature tags, confidence, mapped native owner, and fallback status,
- supports multiple source engines or plugin ecosystems,
- produces deterministic JSON export.

### Issue 0.4 — Create governance and issue labels
Suggested labels:
- `absorption:battle`
- `absorption:ui`
- `absorption:dialogue`
- `absorption:systems`
- `absorption:action`
- `absorption:presentation`
- `absorption:hd2d`
- `import:migration`
- `import:diagnostics`
- `editor:authoring`
- `schema:native`
- `truthfulness:status`

Acceptance criteria:
- issue templates and labels exist,
- new work items clearly declare owner lane, editor impact, migration impact, and HD-2D/2.5D compatibility impact.

---

# Phase 1 — Native substrate and shared authoring infrastructure

## Goal
Lay down the shared systems that all major absorption lanes will reuse.

## Deliverables

- unified UI composition schema,
- unified timeline/event orchestration schema,
- shared diagnostics export lane,
- shared migration report panel patterns,
- shared feature-flag/project-mode framework.

## File targets

### Engine core
- `engine/core/project/project_modes.h`
- `engine/core/project/project_modes.cpp`
- `engine/core/diagnostics/native_absorption_diagnostics.h`
- `engine/core/diagnostics/native_absorption_diagnostics.cpp`
- `engine/core/timeline/native_event_timeline.h`
- `engine/core/timeline/native_event_timeline.cpp`
- `engine/core/ui/ui_layout_schema.h`
- `engine/core/ui/ui_layout_schema.cpp`

### Editor
- `editor/shared/import_migration_report_model.h`
- `editor/shared/import_migration_report_model.cpp`
- `editor/shared/import_migration_report_panel.h`
- `editor/shared/import_migration_report_panel.cpp`
- `editor/shared/project_mode_panel.h`
- `editor/shared/project_mode_panel.cpp`

### Schemas
- `content/schemas/project_modes.schema.json`
- `content/schemas/ui_layouts.schema.json`
- `content/schemas/native_event_timelines.schema.json`
- `content/schemas/native_absorption_diagnostics.schema.json`

## Issue breakdown

### Issue 1.1 — Add project mode framework
Project modes should include:
- `jrpg_2d`
- `action_2d`
- `tactics_2d`
- `platformer_2d`
- `hd2d_25d`
- `hybrid_custom`

Acceptance criteria:
- mode selection exists at project level,
- downstream systems can branch behavior by mode without forked engine code,
- migration reports can declare mode recommendations.

### Issue 1.2 — Add shared absorption diagnostics lane
Acceptance criteria:
- all later phases can emit native mapping status in one common format,
- diagnostics support grouped summaries, per-feature status, and severity,
- editor report panels can consume the format directly.

### Issue 1.3 — Add shared timeline/event substrate
Acceptance criteria:
- supports cutscenes, battle action sequences, transient effects, camera rails, and scripted UI transitions,
- deterministic serialization exists,
- unit anchors verify order, timing, branching, and replay safety.

### Issue 1.4 — Add unified UI layout substrate
Acceptance criteria:
- reusable by menus, HUDs, battle HUDs, codex screens, crafting UIs, and dialogue staging,
- supports anchors, image widgets, gauges, lists, panes, and animated transitions,
- authorable without code.

---

# Phase 2 — Battle Core Suite

## Goal
Eliminate the biggest historical dependency center: battle overhauls, battle UI replacements, action sequences, projectiles, camera control, and enemy AI authoring.

## Deliverables

- alternative turn-model families,
- battle action sequencing,
- projectile lane,
- battle camera lane,
- battle UI composition system,
- enemy/ally tactical behavior authoring.

## File targets

### Engine core
- `engine/core/battle/turn_models.h`
- `engine/core/battle/turn_models.cpp`
- `engine/core/battle/battle_sequence_runtime.h`
- `engine/core/battle/battle_sequence_runtime.cpp`
- `engine/core/battle/battle_projectile_runtime.h`
- `engine/core/battle/battle_projectile_runtime.cpp`
- `engine/core/battle/battle_camera_controller.h`
- `engine/core/battle/battle_camera_controller.cpp`
- `engine/core/battle/battle_ui_layout_runtime.h`
- `engine/core/battle/battle_ui_layout_runtime.cpp`
- `engine/core/battle/battle_ai_graph.h`
- `engine/core/battle/battle_ai_graph.cpp`

### Editor
- `editor/battle/turn_model_panel.h`
- `editor/battle/turn_model_panel.cpp`
- `editor/battle/battle_sequence_editor_panel.h`
- `editor/battle/battle_sequence_editor_panel.cpp`
- `editor/battle/battle_projectile_editor_panel.h`
- `editor/battle/battle_projectile_editor_panel.cpp`
- `editor/battle/battle_camera_editor_panel.h`
- `editor/battle/battle_camera_editor_panel.cpp`
- `editor/battle/battle_ui_layout_panel.h`
- `editor/battle/battle_ui_layout_panel.cpp`
- `editor/battle/battle_ai_graph_panel.h`
- `editor/battle/battle_ai_graph_panel.cpp`

### Schemas
- `content/schemas/battle_turn_models.schema.json`
- `content/schemas/battle_sequences.schema.json`
- `content/schemas/battle_projectiles.schema.json`
- `content/schemas/battle_cameras.schema.json`
- `content/schemas/battle_ui_layouts.schema.json`
- `content/schemas/battle_ai_graphs.schema.json`

### Import/migration
- `engine/import/mapping/battle_plugin_mapping.h`
- `engine/import/mapping/battle_plugin_mapping.cpp`

### Tests
- `tests/unit/test_battle_turn_models.cpp`
- `tests/unit/test_battle_sequence_runtime.cpp`
- `tests/unit/test_battle_projectile_runtime.cpp`
- `tests/unit/test_battle_camera_controller.cpp`
- `tests/unit/test_battle_ui_layout_runtime.cpp`
- `tests/unit/test_battle_ai_graph.cpp`
- `tests/integration/test_battle_native_absorption.cpp`

## Issue breakdown

### Issue 2.1 — Implement alternative turn-model family
Target support:
- DTB-like
- CTB-like
- STB-like
- FTB-like
- PTB/ETB-style continuous variants where appropriate

Acceptance criteria:
- model selection is data-driven,
- battle previews can simulate order/timing deterministically,
- migration can recommend best-fit native mapping.

### Issue 2.2 — Implement battle sequence runtime
Acceptance criteria:
- sequences can orchestrate animation, movement, projectiles, camera, timing windows, hit timing, and UI callouts,
- sequence preview works in editor,
- deterministic replay exists.

### Issue 2.3 — Implement projectile lane
Acceptance criteria:
- projectiles support travel rules, hit rules, visuals, and impact triggers,
- usable by turn-based and action-hybrid projects,
- integrates with later HD-2D/2.5D presentation.

### Issue 2.4 — Implement battle camera lane
Acceptance criteria:
- supports pan, zoom, shake, focus target, rail, and transition easing,
- safe fallbacks exist for standard 2D projects,
- camera preview exists in editor.

### Issue 2.5 — Implement battle UI composition
Acceptance criteria:
- battle HUD can be fully rearranged without code,
- portraits/busts/gauges/status panes/log panes are modular,
- image-based custom widgets are supported.

### Issue 2.6 — Implement battle AI authoring
Acceptance criteria:
- graph or rule-based AI can be authored visually,
- supports priorities, conditions, cooldowns, and target policies,
- imported AI evidence can map to rules or safe fallbacks.

---

# Phase 3 — UI / HUD / Menu Composition Suite

## Goal
Replace stock-style UI plugin dependence with a single native composition framework across menus, HUDs, overlays, codex screens, and battle UIs.

## Deliverables

- menu scene composition,
- HUD composition,
- drag-and-drop layout tools,
- portrait/bust staging,
- theme/style system,
- image-based widgets.

## File targets

### Engine core
- `engine/core/ui/hud_layout_runtime.h`
- `engine/core/ui/hud_layout_runtime.cpp`
- `engine/core/ui/ui_theme_registry.h`
- `engine/core/ui/ui_theme_registry.cpp`
- `engine/core/ui/portrait_bust_layout.h`
- `engine/core/ui/portrait_bust_layout.cpp`
- `engine/core/ui/image_widget_runtime.h`
- `engine/core/ui/image_widget_runtime.cpp`

### Editor
- `editor/ui/ui_layout_canvas_panel.h`
- `editor/ui/ui_layout_canvas_panel.cpp`
- `editor/ui/ui_theme_panel.h`
- `editor/ui/ui_theme_panel.cpp`
- `editor/ui/portrait_bust_layout_panel.h`
- `editor/ui/portrait_bust_layout_panel.cpp`
- `editor/ui/image_widget_panel.h`
- `editor/ui/image_widget_panel.cpp`

### Schemas
- `content/schemas/hud_layouts.schema.json`
- `content/schemas/ui_themes.schema.json`
- `content/schemas/portrait_bust_layouts.schema.json`
- `content/schemas/image_widgets.schema.json`

### Tests
- `tests/unit/test_hud_layout_runtime.cpp`
- `tests/unit/test_ui_theme_registry.cpp`
- `tests/unit/test_portrait_bust_layout.cpp`
- `tests/unit/test_image_widget_runtime.cpp`
- `tests/integration/test_ui_native_absorption.cpp`

## Issue breakdown

### Issue 3.1 — Ship drag-and-drop UI layout canvas
Acceptance criteria:
- menus and HUDs can be composed visually,
- snapping/anchors/z-order are supported,
- layout previews match runtime.

### Issue 3.2 — Ship theme/style system
Acceptance criteria:
- fonts, panel styles, selection effects, transitions, and color profiles are reusable,
- themes can switch by project mode or scene profile,
- HD-2D presentation mode can bind presentation-aware themes.

### Issue 3.3 — Ship portrait/bust composition
Acceptance criteria:
- dialogue, menu, battle, and codex scenes can use staged portrait assets consistently,
- supports layered bust placement and animations,
- import mapping can approximate common portrait plugin behavior.

### Issue 3.4 — Ship advanced image/widget runtime
Acceptance criteria:
- custom gauges, icons, frames, and animated image widgets work natively,
- layout data is schema-driven,
- battle and map overlays can reuse the same widget system.

---

# Phase 4 — Dialogue / Cutscene / Narrative Authoring Suite

## Goal
Replace event-spam and visual-dialogue plugin dependence with a proper native dialogue and cutscene authoring environment.

## Deliverables

- node-based dialogue editor,
- cutscene graph/timeline editor,
- contextual dialogue rules,
- text SFX and speaker styling,
- choice/result/state integration,
- portrait/bust/camera/timeline integration.

## File targets

### Engine core
- `engine/core/message/dialogue_graph.h`
- `engine/core/message/dialogue_graph.cpp`
- `engine/core/message/contextual_dialogue_rules.h`
- `engine/core/message/contextual_dialogue_rules.cpp`
- `engine/core/cutscene/cutscene_graph.h`
- `engine/core/cutscene/cutscene_graph.cpp`
- `engine/core/cutscene/cutscene_runtime.h`
- `engine/core/cutscene/cutscene_runtime.cpp`
- `engine/core/message/text_sfx_runtime.h`
- `engine/core/message/text_sfx_runtime.cpp`

### Editor
- `editor/message/dialogue_graph_panel.h`
- `editor/message/dialogue_graph_panel.cpp`
- `editor/cutscene/cutscene_graph_panel.h`
- `editor/cutscene/cutscene_graph_panel.cpp`
- `editor/cutscene/cutscene_timeline_panel.h`
- `editor/cutscene/cutscene_timeline_panel.cpp`
- `editor/message/contextual_dialogue_panel.h`
- `editor/message/contextual_dialogue_panel.cpp`

### Schemas
- `content/schemas/dialogue_graphs.schema.json`
- `content/schemas/contextual_dialogue_rules.schema.json`
- `content/schemas/cutscene_graphs.schema.json`
- `content/schemas/cutscene_timelines.schema.json`
- `content/schemas/text_sfx_profiles.schema.json`

### Import/migration
- `engine/import/mapping/dialogue_plugin_mapping.h`
- `engine/import/mapping/dialogue_plugin_mapping.cpp`

### Tests
- `tests/unit/test_dialogue_graph.cpp`
- `tests/unit/test_contextual_dialogue_rules.cpp`
- `tests/unit/test_cutscene_graph.cpp`
- `tests/unit/test_cutscene_runtime.cpp`
- `tests/unit/test_text_sfx_runtime.cpp`
- `tests/integration/test_dialogue_native_absorption.cpp`

## Issue breakdown

### Issue 4.1 — Ship node-based dialogue editor
Acceptance criteria:
- branches, conditions, state changes, and outcomes are authorable visually,
- preview exists in editor,
- supports imported dialogue evidence with safe loss reporting.

### Issue 4.2 — Ship cutscene graph + timeline runtime
Acceptance criteria:
- cutscenes can drive camera, movement, dialogue, effects, and waits on a unified timeline,
- deterministic play/skip/fast-forward semantics exist,
- reusable in standard 2D and HD-2D/2.5D projects.

### Issue 4.3 — Ship contextual dialogue rules
Acceptance criteria:
- dialogue can react to quest state, flags, party state, map context, and time/state tags,
- rule conflicts are diagnosed clearly,
- migration can map simple contextual plugins cleanly.

### Issue 4.4 — Ship text SFX and speaker styling
Acceptance criteria:
- speaker profiles control text effects, SFX, pacing, and presentation,
- integrates with bust/portrait staging,
- compatible with low-spec fallback mode.

---

# Phase 5 — Systems Expansion Suite

## Goal
Absorb the common “RPG extension” plugin categories as native systems: quests, codex/bestiary, crafting, professions, achievements, and advanced inventory/equipment.

## Deliverables

- quest/journal system,
- codex/encyclopedia/bestiary,
- crafting/professions/achievements,
- advanced inventory/equipment,
- sockets/shards/materia-like systems,
- independent items.

## File targets

### Engine core
- `engine/core/quest/quest_system.h`
- `engine/core/quest/quest_system.cpp`
- `engine/core/codex/codex_system.h`
- `engine/core/codex/codex_system.cpp`
- `engine/core/crafting/crafting_system.h`
- `engine/core/crafting/crafting_system.cpp`
- `engine/core/progression/profession_system.h`
- `engine/core/progression/profession_system.cpp`
- `engine/core/progression/achievement_system.h`
- `engine/core/progression/achievement_system.cpp`
- `engine/core/inventory/independent_item_runtime.h`
- `engine/core/inventory/independent_item_runtime.cpp`
- `engine/core/inventory/socket_system.h`
- `engine/core/inventory/socket_system.cpp`
- `engine/core/inventory/item_affix_runtime.h`
- `engine/core/inventory/item_affix_runtime.cpp`

### Editor
- `editor/quest/quest_editor_panel.h`
- `editor/quest/quest_editor_panel.cpp`
- `editor/codex/codex_editor_panel.h`
- `editor/codex/codex_editor_panel.cpp`
- `editor/crafting/crafting_editor_panel.h`
- `editor/crafting/crafting_editor_panel.cpp`
- `editor/progression/profession_editor_panel.h`
- `editor/progression/profession_editor_panel.cpp`
- `editor/progression/achievement_editor_panel.h`
- `editor/progression/achievement_editor_panel.cpp`
- `editor/inventory/independent_item_panel.h`
- `editor/inventory/independent_item_panel.cpp`
- `editor/inventory/socket_system_panel.h`
- `editor/inventory/socket_system_panel.cpp`

### Schemas
- `content/schemas/quests.schema.json`
- `content/schemas/codex_entries.schema.json`
- `content/schemas/crafting_recipes.schema.json`
- `content/schemas/professions.schema.json`
- `content/schemas/achievements.schema.json`
- `content/schemas/independent_items.schema.json`
- `content/schemas/socket_systems.schema.json`
- `content/schemas/item_affixes.schema.json`

### Import/migration
- `engine/import/mapping/system_plugin_mapping.h`
- `engine/import/mapping/system_plugin_mapping.cpp`

### Tests
- `tests/unit/test_quest_system.cpp`
- `tests/unit/test_codex_system.cpp`
- `tests/unit/test_crafting_system.cpp`
- `tests/unit/test_profession_system.cpp`
- `tests/unit/test_achievement_system.cpp`
- `tests/unit/test_independent_item_runtime.cpp`
- `tests/unit/test_socket_system.cpp`
- `tests/unit/test_item_affix_runtime.cpp`
- `tests/integration/test_systems_native_absorption.cpp`

## Issue breakdown

### Issue 5.1 — Ship quest/journal system
Acceptance criteria:
- objective trees, pins, statuses, rewards, and hooks exist,
- board/auto-accept/manual-accept models are supported,
- editor previews objective flows.

### Issue 5.2 — Ship codex/bestiary/encyclopedia
Acceptance criteria:
- discovery state is native and data-driven,
- battle/map hooks can unlock entries automatically,
- UI supports multiple presentation layouts.

### Issue 5.3 — Ship crafting/profession/achievement lane
Acceptance criteria:
- crafting and professions are interoperable,
- achievements can observe cross-system events,
- supports custom currencies and meta progression.

### Issue 5.4 — Ship independent items and affix runtime
Acceptance criteria:
- per-instance identity is preserved,
- random/generated stat or affix support exists,
- save/load/migration is deterministic.

### Issue 5.5 — Ship socket/shard/materia-like lane
Acceptance criteria:
- items can contain socket rules and inserted payloads,
- UI and battle/stat systems respect inserted effects,
- import/migration can map common equipment-modifier patterns.

---

# Phase 6 — Action / Interaction / Presentation Suite

## Goal
Open URPG to action RPGs, platformers, pseudo-3D interaction, and modern presentation-heavy projects.

## Deliverables

- pixel/sub-tile movement,
- arbitrary collision geometry,
- click/touch interaction,
- platformer mode,
- action battle hooks,
- dynamic lights/shadows/fog/layers,
- advanced camera profiles.

## File targets

### Engine core
- `engine/core/map/movement_model.h`
- `engine/core/map/movement_model.cpp`
- `engine/core/map/collision_geometry.h`
- `engine/core/map/collision_geometry.cpp`
- `engine/core/map/interaction_runtime.h`
- `engine/core/map/interaction_runtime.cpp`
- `engine/core/map/platformer_runtime.h`
- `engine/core/map/platformer_runtime.cpp`
- `engine/core/map/action_combat_hooks.h`
- `engine/core/map/action_combat_hooks.cpp`
- `engine/core/render/light_runtime.h`
- `engine/core/render/light_runtime.cpp`
- `engine/core/render/shadow_runtime.h`
- `engine/core/render/shadow_runtime.cpp`
- `engine/core/render/fog_layer_runtime.h`
- `engine/core/render/fog_layer_runtime.cpp`
- `engine/core/camera/map_camera_profiles.h`
- `engine/core/camera/map_camera_profiles.cpp`

### Editor
- `editor/map/collision_geometry_panel.h`
- `editor/map/collision_geometry_panel.cpp`
- `editor/map/movement_model_panel.h`
- `editor/map/movement_model_panel.cpp`
- `editor/map/platformer_mode_panel.h`
- `editor/map/platformer_mode_panel.cpp`
- `editor/render/light_shadow_panel.h`
- `editor/render/light_shadow_panel.cpp`
- `editor/camera/map_camera_profile_panel.h`
- `editor/camera/map_camera_profile_panel.cpp`

### Schemas
- `content/schemas/movement_models.schema.json`
- `content/schemas/collision_geometry.schema.json`
- `content/schemas/platformer_maps.schema.json`
- `content/schemas/action_combat_hooks.schema.json`
- `content/schemas/lights.schema.json`
- `content/schemas/shadows.schema.json`
- `content/schemas/fog_layers.schema.json`
- `content/schemas/map_camera_profiles.schema.json`

### Import/migration
- `engine/import/mapping/action_plugin_mapping.h`
- `engine/import/mapping/action_plugin_mapping.cpp`
- `engine/import/mapping/presentation_plugin_mapping.h`
- `engine/import/mapping/presentation_plugin_mapping.cpp`

### Tests
- `tests/unit/test_movement_model.cpp`
- `tests/unit/test_collision_geometry.cpp`
- `tests/unit/test_interaction_runtime.cpp`
- `tests/unit/test_platformer_runtime.cpp`
- `tests/unit/test_action_combat_hooks.cpp`
- `tests/unit/test_light_runtime.cpp`
- `tests/unit/test_shadow_runtime.cpp`
- `tests/unit/test_map_camera_profiles.cpp`
- `tests/integration/test_action_presentation_native_absorption.cpp`

## Issue breakdown

### Issue 6.1 — Ship movement and collision lane
Acceptance criteria:
- grid and sub-tile/pixel movement can coexist under project mode/config,
- arbitrary collision geometry is supported,
- pathfinding respects geometry and movement model.

### Issue 6.2 — Ship platformer mode
Acceptance criteria:
- gravity, slopes, one-way platforms, hazards, and jump rules are supported,
- maps can opt in independently,
- platformer preview exists in editor.

### Issue 6.3 — Ship action battle hooks
Acceptance criteria:
- hitboxes/hurtboxes/projectiles/cooldowns can work on map runtime,
- battle and map action logic share core effect systems where appropriate,
- deterministic low-level rules are test-covered.

### Issue 6.4 — Ship lighting/shadows/fog/layers lane
Acceptance criteria:
- dynamic lights and shadows are authorable in editor,
- fog/layer graphics support deterministic presentation order,
- safe low-tier rendering fallbacks exist.

### Issue 6.5 — Ship advanced camera profiles
Acceptance criteria:
- pans, zooms, tilt, offsets, locks, rails, and focus rules are supported,
- profiles can be reused by map, cutscene, and battle systems,
- HD-2D mode can use depth-aware camera behavior.

---

# Phase 7 — HD-2D / 2.5D Native Presentation Lane

## Goal
Turn the most requested forward-looking visual direction into a native product capability rather than an afterthought.

## Deliverables

- hybrid scene profiles,
- sprite-in-3D-space rendering,
- depth-aware occlusion,
- elevation-aware map schema,
- normal/emissive/specular support,
- cinematic camera rails,
- battle/map presentation parity.

## File targets

### Engine core
- `engine/core/render/hd2d_scene_profile.h`
- `engine/core/render/hd2d_scene_profile.cpp`
- `engine/core/render/depth_sprite_renderer.h`
- `engine/core/render/depth_sprite_renderer.cpp`
- `engine/core/render/elevation_map_runtime.h`
- `engine/core/render/elevation_map_runtime.cpp`
- `engine/core/render/material_sprite_runtime.h`
- `engine/core/render/material_sprite_runtime.cpp`
- `engine/core/camera/cinematic_rail_runtime.h`
- `engine/core/camera/cinematic_rail_runtime.cpp`
- `engine/core/render/hd2d_battle_presentation.h`
- `engine/core/render/hd2d_battle_presentation.cpp`

### Editor
- `editor/render/hd2d_scene_profile_panel.h`
- `editor/render/hd2d_scene_profile_panel.cpp`
- `editor/render/elevation_map_panel.h`
- `editor/render/elevation_map_panel.cpp`
- `editor/render/material_sprite_panel.h`
- `editor/render/material_sprite_panel.cpp`
- `editor/camera/cinematic_rail_panel.h`
- `editor/camera/cinematic_rail_panel.cpp`
- `editor/render/hd2d_preview_panel.h`
- `editor/render/hd2d_preview_panel.cpp`

### Schemas
- `content/schemas/hd2d_scene_profiles.schema.json`
- `content/schemas/elevation_maps.schema.json`
- `content/schemas/material_sprites.schema.json`
- `content/schemas/cinematic_rails.schema.json`
- `content/schemas/hd2d_battle_presentations.schema.json`

### Import/migration
- `engine/import/mapping/hd2d_recommendation_mapping.h`
- `engine/import/mapping/hd2d_recommendation_mapping.cpp`

### Tests
- `tests/unit/test_hd2d_scene_profile.cpp`
- `tests/unit/test_depth_sprite_renderer.cpp`
- `tests/unit/test_elevation_map_runtime.cpp`
- `tests/unit/test_material_sprite_runtime.cpp`
- `tests/unit/test_cinematic_rail_runtime.cpp`
- `tests/unit/test_hd2d_battle_presentation.cpp`
- `tests/integration/test_hd2d_native_absorption.cpp`

## Issue breakdown

### Issue 7.1 — Ship HD-2D scene profile runtime
Acceptance criteria:
- scenes can opt into hybrid presentation cleanly,
- standard 2D projects are not forced into expensive rendering paths,
- rendering mode boundaries are explicit and testable.

### Issue 7.2 — Ship elevation-aware schema and depth renderer
Acceptance criteria:
- map tiles, sprites, props, and effects can participate in depth ordering,
- occlusion rules are deterministic,
- editor preview accurately reflects runtime depth behavior.

### Issue 7.3 — Ship material-aware sprites
Acceptance criteria:
- normal/emissive/specular channels are supported where project mode allows,
- light/shadow integration works with sprite materials,
- fallback rendering path exists for unsupported tiers.

### Issue 7.4 — Ship cinematic rails and battle parity
Acceptance criteria:
- camera rails can drive map and battle scenes,
- battle presentation can match map depth/perspective profile where desired,
- cutscenes can reuse the same rails.

---

# Phase 8 — Migration, stabilization, and release proof

## Goal
Prove that the new native lanes are coherent, importable, diagnosable, and shippable.

## Deliverables

- migration wizard updates,
- plugin-heavy project intake reports,
- native absorption status dashboards,
- release-readiness matrices,
- published stability gates.

## File targets

### Engine/import
- `engine/import/native_absorption_pipeline.h`
- `engine/import/native_absorption_pipeline.cpp`
- `engine/import/native_absorption_report.h`
- `engine/import/native_absorption_report.cpp`

### Editor
- `editor/import/native_absorption_wizard_panel.h`
- `editor/import/native_absorption_wizard_panel.cpp`
- `editor/import/native_absorption_dashboard_panel.h`
- `editor/import/native_absorption_dashboard_panel.cpp`

### Docs
- `docs/NATIVE_ABSORPTION_RELEASE_MATRIX.md`
- `docs/IMPORT_CONFIDENCE_CHECKLIST.md`
- `docs/HD2D_25D_RELEASE_GATES.md`

### Tooling
- `tools/ci/run_native_absorption_gate.ps1`
- `tools/ci/export_native_absorption_report.py`

### Tests
- `tests/integration/test_native_absorption_pipeline.cpp`
- `tests/integration/test_native_absorption_dashboard.cpp`
- `tests/snapshot/test_native_absorption_reports.cpp`

## Issue breakdown

### Issue 8.1 — Extend migration wizard for native absorption reporting
Acceptance criteria:
- wizard can summarize per-category native mapping results,
- blocked or approximated features are visible and exportable,
- mode recommendations are surfaced for imported projects.

### Issue 8.2 — Add release-readiness matrix
Acceptance criteria:
- every lane has runtime/editor/schema/migration/test/gate status,
- public claims are bounded by actual evidence,
- matrix supports “safe to ship,” “preview,” and “experimental” markers.

### Issue 8.3 — Add native absorption CI gate
Acceptance criteria:
- gate validates schema contracts, unit/integration anchors, and report generation,
- failures are categorized by lane,
- dashboards can consume the artifacts.

### Issue 8.4 — Publish import-confidence checklist
Acceptance criteria:
- import status is never claimed without a measurable checklist,
- loss reporting exists for approximated or blocked features,
- HD-2D recommendation logic is tested and documented.

---

# Cross-phase dependency map

## Critical prerequisites

- The draft dependency order expected Phase 1 to start before Phases 2–7 could scale cleanly.
- Phases 2, 3, and 4 should overlap because battle/UI/dialogue share major infrastructure.
- Phase 6 should begin before Phase 7 is considered “real,” because HD-2D/2.5D without action/presentation groundwork becomes a renderer-only gimmick.
- Phase 8 should begin in parallel once Phase 2 starts landing, not at the very end.

## Recommended execution order

1. Phase 0
2. Phase 1
3. Phase 2
4. Phase 3
5. Phase 4
6. Phase 5
7. Phase 6
8. Phase 7
9. Phase 8

---

# Milestones

## Milestone A — Governance + substrate
Includes:
- Phase 0 complete
- Phase 1 substantially complete

Exit criteria:
- canonical taxonomy exists,
- shared diagnostics, timeline, UI layout, and project modes exist,
- issues can now be planned by lane without ambiguity.

## Milestone B — Major dependency replacement
Includes:
- Phase 2 complete to production baseline
- Phase 3 complete to production baseline
- Phase 4 complete to production baseline

Exit criteria:
- battle, UI, and dialogue no longer depend on plugin-shaped core delivery,
- editor ownership exists,
- migration can classify common plugin-derived behavior in those lanes.

## Milestone C — Systems + interaction expansion
Includes:
- Phase 5 complete to production baseline
- Phase 6 complete to production baseline

Exit criteria:
- URPG now supports modern systemic RPG features plus action/platformer/presentation-heavy maps natively.

## Milestone D — Strategic differentiation
Includes:
- Phase 7 complete to production baseline
- Phase 8 complete

Exit criteria:
- HD-2D / 2.5D is a native, documented, gated product lane,
- migration and release proof are publishable,
- URPG can honestly position itself beyond plugin-stacked RPG Maker workflows.

---

# Suggested GitHub epic structure

## Epic 1 — Plugin absorption governance
Child issues:
- taxonomy
- scorecard
- import evidence schema
- doc and CI governance

## Epic 2 — Shared native authoring substrate
Child issues:
- project modes
- diagnostics lane
- timeline substrate
- UI layout substrate

## Epic 3 — Battle Core Suite
Child issues:
- turn models
- sequences
- projectiles
- cameras
- battle UI
- AI authoring

## Epic 4 — UI / HUD / Menu Suite
Child issues:
- layout canvas
- themes
- portraits/busts
- image widgets

## Epic 5 — Dialogue / Cutscene Suite
Child issues:
- dialogue graph
- cutscene graph
- contextual dialogue
- text SFX / speaker styling

## Epic 6 — Systems Expansion Suite
Child issues:
- quests
- codex/bestiary
- crafting/professions/achievements
- independent items
- sockets/shards/materia-like system

## Epic 7 — Action / Interaction / Presentation Suite
Child issues:
- movement/collision
- platformer mode
- action battle hooks
- lighting/shadows
- cameras

## Epic 8 — HD-2D / 2.5D Lane
Child issues:
- scene profiles
- elevation/depth renderer
- material sprites
- cinematic rails
- battle parity

## Epic 9 — Migration + release proof
Child issues:
- wizard extension
- dashboards
- gates
- release matrix
- import-confidence checklist

---

# Acceptance rules for any lane to count as “absorbed”

A category is not considered fully absorbed unless all of the following exist:

1. native runtime owner,
2. native schema contracts,
3. native editor authoring surface,
4. migration/import mapping or explicit unsupported declaration,
5. diagnostics and export/report visibility,
6. unit and integration anchors,
7. release gate coverage,
8. truthfully bounded public status.

---

# Definition of done

This roadmap is complete when:

1. the major plugin demand centers identified in the research brief have native owners,
2. the most common historical plugin dependencies are replaced by native systems,
3. imported projects can be analyzed and migrated into those native systems with explicit diagnostics,
4. HD-2D / 2.5D support is a first-class gated lane, not a side experiment,
5. release matrices and CI gates prove what URPG can and cannot currently absorb.

---

# Draft next actions recorded at the time

## Draft doc actions

1. Copy this file into `docs/PLUGIN_NATIVE_ABSORPTION_ROADMAP.md`
2. Add it to the README documentation index
3. Link it from `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
4. Link it from `docs/PROGRAM_COMPLETION_STATUS.md` under next-lane planning

## Draft implementation actions

1. Execute Phase 0 Issue 0.1 and 0.2 immediately
2. Create the new import evidence schemas
3. Stand up the shared diagnostics lane
4. Start Battle Core Suite and UI layout substrate in parallel

## Next repo-management actions

1. Create 9 epics matching the epic structure above
2. Add issue templates requiring lane, editor impact, schema impact, migration impact, and HD-2D relevance
3. Add CI stub for native absorption gate reporting

---

# Recommended first vertical slice

If only one end-to-end native absorption slice is started immediately, it should be:

## “Battle UI + Battle Sequence + Camera + Migration Report”

Why this slice first:
- directly attacks one of the largest plugin dependency centers,
- produces visible payoff fast,
- forces the shared UI, timeline, diagnostics, and migration infrastructure to become real,
- is naturally compatible with later HD-2D / 2.5D work.

### Vertical-slice file targets
- `engine/core/battle/battle_sequence_runtime.*`
- `engine/core/battle/battle_camera_controller.*`
- `engine/core/battle/battle_ui_layout_runtime.*`
- `editor/battle/battle_sequence_editor_panel.*`
- `editor/battle/battle_camera_editor_panel.*`
- `editor/battle/battle_ui_layout_panel.*`
- `content/schemas/battle_sequences.schema.json`
- `content/schemas/battle_cameras.schema.json`
- `content/schemas/battle_ui_layouts.schema.json`
- `engine/import/mapping/battle_plugin_mapping.*`
- `tests/unit/test_battle_sequence_runtime.cpp`
- `tests/unit/test_battle_camera_controller.cpp`
- `tests/unit/test_battle_ui_layout_runtime.cpp`
- `tests/integration/test_battle_native_absorption.cpp`

### Vertical-slice exit criteria
- battle UI can be authored in-editor,
- sequence preview works,
- camera preview works,
- import report can classify common battle-plugin evidence into native mapping status,
- a focused CI gate passes for the slice.

---

# Final note

The value of this roadmap is not that it names many systems.

The value is that it gives URPG a clean rule for turning chaotic plugin demand into a **native product roadmap** with:
- explicit owners,
- explicit file targets,
- explicit issue boundaries,
- explicit migration logic,
- explicit proof of readiness.

That is how URPG avoids becoming “RPG Maker plus more plugins” and instead becomes a native-first creation engine.
