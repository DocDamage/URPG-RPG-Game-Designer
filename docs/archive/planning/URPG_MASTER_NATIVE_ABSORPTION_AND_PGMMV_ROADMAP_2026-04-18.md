# URPG Master Native Absorption and PGMMV Roadmap v2

Date: 2026-04-18  
Status: detailed planning annex / execution-detail input (v2 planning-closure update)  
Scope: unify **PGMMV support**, **native absorption of plugin-driven demand**, and **HD-2D / 2.5D productization** into one repo-ready planning input with phases, file targets, issue breakdowns, acceptance criteria, and release gates.

Canonical governance and current execution authority live in:
- `../../TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `../../NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `../../PROGRAM_COMPLETION_STATUS.md`

Use this file as a detailed planning annex, not as a parallel status or release-readiness authority. Its deltas become canonical only after they are absorbed into the documents above.

Derived from and superseding earlier raw planning inputs:
- `URPG_PGMMV_SUPPORT_PLAN.md`
- `URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md`
- `../../WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md`

---

# 1. Executive position

URPG should not become:

- a plugin clone catalog,
- a PGMMV runtime emulator,
- or a pile of legacy compatibility surfaces pretending to be a product.

URPG should become a **native-first creation engine** that can:

1. absorb the most important historically plugin-solved capability categories as native runtime/editor/schema systems,
2. intake and migrate PGMMV projects as a separate source-engine lane,
3. classify imported and inferred legacy behavior truthfully,
4. support standard 2D RPG, action-hybrid, platformer, and HD-2D / 2.5D presentation as deliberate project modes,
5. publish release claims bounded by evidence rather than wishful compatibility language.

The product target is not “support every plugin.” The product target is:

> The major reasons users depend on plugin stacks or alternate makers become first-class URPG systems, and imported projects can continue life as native URPG projects with explicit reporting about what was preserved, approximated, blocked, or unsupported.

---

# 2. Strategic outcome

When this roadmap is complete, URPG should be able to do all of the following without core dependence on third-party plugin stacks or source-engine emulation:

- import supported Pixel Game Maker MV projects through a dedicated intake lane,
- classify imported PGMMV constructs into preserved / approximated / unsupported,
- ship modern battle systems and battle UI natively,
- ship modern HUD/menu/dialogue/cutscene systems natively,
- ship quests, codex/bestiary, crafting, professions, achievements, and advanced inventory/equipment systems natively,
- ship action/platformer/collision/interaction systems natively,
- ship lighting, shadows, camera, depth, layering, and scene profile systems natively,
- ship HD-2D / 2.5D scene presentation and battle presentation natively,
- migrate plugin-heavy or source-engine-derived project evidence into native URPG schemas with explicit diagnostics,
- prove all of the above through editor surfaces, schemas, tests, migration reports, and CI gates.

---

# 3. Non-goals

This roadmap does **not** aim to:

- recreate every historical plugin one-to-one,
- preserve every plugin parameter surface forever,
- fully emulate PGMMV runtime behavior,
- promise perfect parity with every source-engine or plugin quirk,
- mark a lane `FULL` unless runtime, editor, schema, migration, diagnostics, and tests all exist,
- let compatibility vocabulary become long-term product architecture.

---

# 4. Core principles

## 4.1 Native ownership first
Every major lane must land as:

- native runtime owner,
- native schema contract,
- native editor authoring surface,
- native diagnostics/reporting path,
- migration/import mapping or explicit unsupported declaration.

## 4.2 Intake bridges are not the product
Compat/import/source-engine lanes exist to:

- discover behavior,
- classify risk,
- prove migration confidence,
- and route users into native URPG ownership.

## 4.3 Normalize first, convert second
No direct spaghetti mapping from raw source-engine data or plugin evidence into many downstream URPG systems.

Everything routes through:
- normalized evidence models,
- source-engine-neutral migration analysis,
- then native schema emitters.

## 4.4 Category absorption beats plugin cloning
Absorb:
- battle UI,
- quest systems,
- dialogue staging,
- lighting/camera,
- action movement/collision,
- equipment individuality,
- HD-2D / 2.5D presentation,

not 200 isolated plugin brands.

## 4.5 Truthful status labels
Every lane and imported feature must clearly distinguish:

- `NATIVE_FULL`
- `NATIVE_PARTIAL`
- `IMPORTED_PRESERVED`
- `IMPORTED_APPROXIMATED`
- `IMPORT_BLOCKED`
- `UNSUPPORTED`
- `EXPERIMENTAL`

## 4.6 Editor ownership is mandatory
A feature does not count as delivered if it only exists in hidden data or runtime code.

## 4.7 HD-2D / 2.5D is a first-class cross-cutting constraint
All major visual and gameplay systems added by this roadmap must be evaluated for compatibility with:

- `jrpg_2d`
- `action_2d`
- `platformer_2d`
- `tactics_2d`
- `hd2d_25d`
- `hybrid_custom`

---

# 5. Program structure

This roadmap is split into 10 execution phases:

- Phase 0: Governance, taxonomy, and authoritative planning alignment
- Phase 1: Shared substrate and project-mode foundation
- Phase 2: PGMMV intake and normalized import foundation
- Phase 3: Core native absorption suites I — Battle / UI / Dialogue
- Phase 4: Core native absorption suites II — Systems expansion
- Phase 5: Action / interaction / map runtime and PGMMV coverage expansion
- Phase 6: Lighting / camera / depth / presentation substrate
- Phase 7: HD-2D / 2.5D native presentation lane
- Phase 8: Migration wizard, diagnostics dashboards, and confidence proofs
- Phase 9: Release hardening, matrices, and truthfulness gates

Program tracks that run across phases:

- **Track A — Source-engine intake expansion**
- **Track B — Native absorption of plugin-solved demand centers**
- **Track C — HD-2D / 2.5D productization**
- **Track D — Diagnostics, migration truthfulness, and release proof**

---

# 6. Canonical lane taxonomy

These are the native product lanes URPG will absorb and/or map imported evidence into.

## 6.1 Intake lanes
- source engine registry
- PGMMV intake
- plugin evidence intake
- migration evidence and project recommendation logic

## 6.2 Runtime-authority lanes
- battle core suite
- UI / HUD / menu composition suite
- dialogue / cutscene / narrative authoring suite
- systems expansion suite
- action / interaction / platformer runtime suite
- lighting / shadows / camera / depth suite
- HD-2D / 2.5D scene and battle presentation suite

## 6.3 Editor-authority lanes
- migration wizard and report surfaces
- battle sequence and battle UI authoring
- layout canvas and theme authoring
- dialogue graph and cutscene staging
- quest/codex/crafting/equipment editors
- collision / trigger / platform / action behavior editors
- scene profile, lighting, camera, depth, and cinematic rail editors

## 6.4 Cross-cutting truthfulness lanes
- diagnostics export
- import-confidence checklists
- release-readiness matrices
- CI proof and dashboard artifacts

---

# 7. Phase 0 — Governance, taxonomy, and authoritative planning alignment

## Goal
Replace split planning with one canonical execution structure and define how source-engine intake and plugin-demand absorption map into native URPG ownership.

## Deliverables
- canonical roadmap doc in repo,
- plugin-category taxonomy,
- source-engine intake taxonomy,
- native absorption scorecard,
- status vocabulary and truthfulness policy,
- issue labels and epic conventions.

## File targets

### New docs
- `docs/MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP.md`
- `docs/PLUGIN_CATEGORY_TAXONOMY.md`
- `docs/SOURCE_ENGINE_INTAKE_TAXONOMY.md`
- `docs/NATIVE_ABSORPTION_SCORECARD.md`
- `docs/IMPORT_EVIDENCE_AND_NATIVE_MAPPING_POLICY.md`
- `docs/HD2D_25D_COMPATIBILITY_RULES.md`
- `docs/IMPORT_STATUS_VOCABULARY.md`

### New schemas
- `content/schemas/import/plugin_category_evidence.schema.json`
- `content/schemas/import/source_engine_evidence.schema.json`
- `content/schemas/import/project_plugin_dependency_report.schema.json`
- `content/schemas/import/native_absorption_status.schema.json`

### New tooling
- `tools/docs/generate_native_absorption_matrix.py`
- `tools/ci/check_master_absorption_docs.ps1`

## Issue breakdown

### Issue 0.1 — Create canonical plugin-category taxonomy
Acceptance criteria:
- every researched demand center maps to one primary lane and optional secondary tags,
- taxonomy includes HD-2D / 2.5D relevance,
- taxonomy is stable enough for issue planning and migration diagnostics.

### Issue 0.2 — Create source-engine intake taxonomy
Acceptance criteria:
- explicitly distinguishes RPG Maker MZ intake, PGMMV intake, and future source engines,
- identifies what evidence is structural, behavioral, or content-oriented,
- documents what belongs in normalized import models.

### Issue 0.3 — Create native absorption scorecard
Acceptance criteria:
- ranks categories by demand, migration frequency, strategic value, editor burden, implementation cost, and HD-2D leverage,
- produces phase ordering without hand-waving,
- is reproducible and documented.

### Issue 0.4 — Create authoritative status and truthfulness policy
Acceptance criteria:
- status labels are defined once and used everywhere,
- public docs cannot claim `FULL` without all required ownership pillars,
- imported approximations and blocked features are visible in reports.

### Issue 0.5 — Create repo labels and issue templates
Suggested labels:
- `track:intake`
- `track:absorption`
- `track:hd2d`
- `track:diagnostics`
- `absorption:battle`
- `absorption:ui`
- `absorption:dialogue`
- `absorption:systems`
- `absorption:action`
- `absorption:presentation`
- `import:pgmmv`
- `import:migration`
- `truthfulness:status`

Acceptance criteria:
- issue templates require lane, editor impact, schema impact, migration impact, diagnostics impact, and HD-2D / 2.5D relevance.

---

# 8. Phase 1 — Shared substrate and project-mode foundation

## Goal
Lay down shared systems that every later lane reuses.

## Deliverables
- project mode framework,
- shared diagnostics/report format,
- shared timeline/event substrate,
- shared UI layout substrate,
- shared migration report models and panels,
- mode-aware capability flags.

## File targets

### Engine core
- `engine/core/project/project_modes.h`
- `engine/core/project/project_modes.cpp`
- `engine/core/diagnostics/native_absorption_diagnostics.h`
- `engine/core/diagnostics/native_absorption_diagnostics.cpp`
- `engine/core/diagnostics/import_status_registry.h`
- `engine/core/diagnostics/import_status_registry.cpp`
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
- `editor/shared/capability_profile_panel.h`
- `editor/shared/capability_profile_panel.cpp`

### Schemas
- `content/schemas/project_modes.schema.json`
- `content/schemas/ui_layouts.schema.json`
- `content/schemas/native_event_timelines.schema.json`
- `content/schemas/native_absorption_diagnostics.schema.json`
- `content/schemas/import_status_registry.schema.json`

## Issue breakdown

### Issue 1.1 — Add project mode framework
Modes:
- `jrpg_2d`
- `action_2d`
- `tactics_2d`
- `platformer_2d`
- `hd2d_25d`
- `hybrid_custom`

Acceptance criteria:
- project-level mode selection exists,
- downstream systems can branch by mode without engine forks,
- migration lanes can recommend a target mode.

### Issue 1.2 — Add shared diagnostics lane
Acceptance criteria:
- all later phases emit one common format,
- supports grouped summaries, severity, disposition, owner lane, and recommendation fields,
- editor panels consume the format directly.

### Issue 1.3 — Add shared timeline/event substrate
Acceptance criteria:
- supports cutscenes, battle action sequences, camera rails, transient effects, and scripted UI transitions,
- deterministic serialization exists,
- unit anchors verify order, timing, branching, and replay safety.

### Issue 1.4 — Add unified UI layout substrate
Acceptance criteria:
- reusable by menus, HUDs, battle HUDs, codex screens, crafting UIs, and dialogue staging,
- supports panes, anchors, portraits, gauges, lists, animated transitions, and image widgets,
- authorable without code.

---

# 9. Phase 2 — PGMMV intake and normalized import foundation

## Goal
Add PGMMV as a separate source-engine intake lane that routes into normalized import models and native URPG ownership.

## Deliverables
- source-engine registration for PGMMV,
- project detector,
- project reader,
- normalized import model,
- preflight summary,
- first migration report path,
- native emitter hooks for v1 support subset.

## File targets

### Engine intake and import
- `engine/import/source/source_engine_registry.h`
- `engine/import/source/source_engine_registry.cpp`
- `engine/import/source/source_engine_capability_profile.h`
- `engine/import/source/source_engine_capability_profile.cpp`
- `engine/import/pgmmv/pgmmv_project_detector.h`
- `engine/import/pgmmv/pgmmv_project_detector.cpp`
- `engine/import/pgmmv/pgmmv_project_reader.h`
- `engine/import/pgmmv/pgmmv_project_reader.cpp`
- `engine/import/pgmmv/pgmmv_preflight_summary.h`
- `engine/import/pgmmv/pgmmv_preflight_summary.cpp`
- `engine/import/pgmmv/pgmmv_import_model.h`
- `engine/import/pgmmv/pgmmv_import_model.cpp`
- `engine/import/pgmmv/pgmmv_migration_analyzer.h`
- `engine/import/pgmmv/pgmmv_migration_analyzer.cpp`
- `engine/import/pgmmv/pgmmv_native_emitters.h`
- `engine/import/pgmmv/pgmmv_native_emitters.cpp`

### Editor
- `editor/import/pgmmv_import_wizard_panel.h`
- `editor/import/pgmmv_import_wizard_panel.cpp`
- `editor/import/pgmmv_import_report_panel.h`
- `editor/import/pgmmv_import_report_panel.cpp`
- `editor/import/pgmmv_preflight_panel.h`
- `editor/import/pgmmv_preflight_panel.cpp`

### Schemas
- `content/schemas/import/pgmmv_preflight_summary.schema.json`
- `content/schemas/import/pgmmv_import_report.schema.json`
- `content/schemas/import/pgmmv_normalized_entities.schema.json`

### Tests
- `tests/unit/test_pgmmv_project_detector.cpp`
- `tests/unit/test_pgmmv_project_reader.cpp`
- `tests/unit/test_pgmmv_import_model.cpp`
- `tests/unit/test_pgmmv_migration_analyzer.cpp`
- `tests/integration/test_pgmmv_import_vertical_slice.cpp`

### Fixtures
- `tests/fixtures/pgmmv/minimal_platformer/`
- `tests/fixtures/pgmmv/minimal_action_topdown/`
- `tests/fixtures/pgmmv/import_edge_cases/`
- `tests/fixtures/pgmmv/fixture_manifest.json`

## Issue breakdown

### Issue 2.1 — Add `SourceEngine::PixelGameMakerMV`
Acceptance criteria:
- PGMMV appears as a distinct intake source,
- migration wizard can route to PGMMV-specific preflight logic,
- source-engine capability profile exists.

### Issue 2.2 — Add project detector and preflight summary
Acceptance criteria:
- detects valid, likely, and invalid PGMMV roots,
- reports missing critical files clearly,
- produces deterministic confidence results.

### Issue 2.3 — Add project reader and normalized import model
Acceptance criteria:
- extracts metadata, stage structures, asset references, collision-relevant structures, spawn points, and simple trigger information,
- routes into source-engine-neutral normalized entities,
- handles unknown fields with structured diagnostics.

### Issue 2.4 — Add first migration analyzer
Acceptance criteria:
- classifies imported items into preserved / approximated / unsupported,
- no silent downgrades,
- emits machine-readable diagnostics and summary counts.

### Issue 2.5 — Add first native import vertical slice
Initial v1 subset:
- one simple stage,
- background/layer mapping,
- sprite sheet import,
- collision region import,
- spawn point import,
- simple trigger import,
- report surface in editor.

Acceptance criteria:
- imported project opens in editor,
- report shows preserved / approximated / failed,
- import is repeatable and tested.

---

# 10. Phase 3 — Core native absorption suites I: Battle / UI / Dialogue

## Goal
Eliminate the largest historical dependency centers first while also giving PGMMV and plugin-derived projects native targets to map into.

## Deliverables
- battle turn model families,
- battle sequence authoring,
- battle projectiles,
- battle camera control,
- battle UI composition,
- dialogue graph authoring,
- cutscene staging,
- UI / HUD / menu composition and theming.

## File targets

### Battle engine core
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

### UI engine core
- `engine/core/ui/ui_theme_runtime.h`
- `engine/core/ui/ui_theme_runtime.cpp`
- `engine/core/ui/portrait_widget_runtime.h`
- `engine/core/ui/portrait_widget_runtime.cpp`
- `engine/core/ui/hud_overlay_runtime.h`
- `engine/core/ui/hud_overlay_runtime.cpp`
- `engine/core/ui/menu_canvas_runtime.h`
- `engine/core/ui/menu_canvas_runtime.cpp`

### Dialogue and cutscene core
- `engine/core/dialogue/dialogue_graph.h`
- `engine/core/dialogue/dialogue_graph.cpp`
- `engine/core/dialogue/dialogue_staging_runtime.h`
- `engine/core/dialogue/dialogue_staging_runtime.cpp`
- `engine/core/cutscene/cutscene_graph.h`
- `engine/core/cutscene/cutscene_graph.cpp`
- `engine/core/cutscene/cutscene_director.h`
- `engine/core/cutscene/cutscene_director.cpp`

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
- `editor/ui/ui_theme_panel.h`
- `editor/ui/ui_theme_panel.cpp`
- `editor/ui/menu_canvas_panel.h`
- `editor/ui/menu_canvas_panel.cpp`
- `editor/ui/hud_overlay_panel.h`
- `editor/ui/hud_overlay_panel.cpp`
- `editor/dialogue/dialogue_graph_panel.h`
- `editor/dialogue/dialogue_graph_panel.cpp`
- `editor/dialogue/dialogue_staging_panel.h`
- `editor/dialogue/dialogue_staging_panel.cpp`
- `editor/cutscene/cutscene_graph_panel.h`
- `editor/cutscene/cutscene_graph_panel.cpp`

### Schemas
- `content/schemas/battle_turn_models.schema.json`
- `content/schemas/battle_sequences.schema.json`
- `content/schemas/battle_projectiles.schema.json`
- `content/schemas/battle_cameras.schema.json`
- `content/schemas/battle_ui_layouts.schema.json`
- `content/schemas/ui_themes.schema.json`
- `content/schemas/hud_overlays.schema.json`
- `content/schemas/menu_canvases.schema.json`
- `content/schemas/dialogue_graphs.schema.json`
- `content/schemas/dialogue_staging.schema.json`
- `content/schemas/cutscene_graphs.schema.json`

### Import mapping
- `engine/import/mapping/battle_plugin_mapping.h`
- `engine/import/mapping/battle_plugin_mapping.cpp`
- `engine/import/mapping/ui_plugin_mapping.h`
- `engine/import/mapping/ui_plugin_mapping.cpp`
- `engine/import/mapping/dialogue_plugin_mapping.h`
- `engine/import/mapping/dialogue_plugin_mapping.cpp`

### Tests
- `tests/unit/test_battle_turn_models.cpp`
- `tests/unit/test_battle_sequence_runtime.cpp`
- `tests/unit/test_battle_camera_controller.cpp`
- `tests/unit/test_battle_ui_layout_runtime.cpp`
- `tests/unit/test_ui_theme_runtime.cpp`
- `tests/unit/test_menu_canvas_runtime.cpp`
- `tests/unit/test_dialogue_graph.cpp`
- `tests/unit/test_cutscene_graph.cpp`
- `tests/integration/test_battle_native_absorption.cpp`
- `tests/integration/test_ui_dialogue_absorption.cpp`

## Issue breakdown

### Issue 3.1 — Battle Core Suite
Acceptance criteria:
- battle UI, sequences, projectiles, and camera behaviors can be authored natively,
- import evidence can classify common battle-plugin behaviors into native mapping statuses,
- focused battle absorption CI gate passes.

### Issue 3.2 — UI / HUD / Menu composition suite
Acceptance criteria:
- HUD and menu authoring is layout-canvas-driven rather than plugin-parameter-driven,
- themes, portraits, and overlay widgets are native,
- import evidence can map common HUD/menu behaviors.

### Issue 3.3 — Dialogue and cutscene suite
Acceptance criteria:
- branching dialogue, speaker staging, portrait/bust staging, text SFX hooks, and basic cutscene flow are native,
- import evidence can map dialogue/cutscene behaviors or explicitly mark unsupported constructs,
- preview tooling exists in editor.

---

# 11. Phase 4 — Core native absorption suites II: Systems expansion

## Goal
Absorb the next major plugin-solved dependency centers: progression systems, codex data, crafting, advanced items/equipment, achievements, and profession-like systems.

## Deliverables
- quest and journal systems,
- codex / bestiary / glossary systems,
- crafting / recipe / profession systems,
- achievements / milestones,
- independent items and unique-instance equipment,
- sockets / shards / materia-like systems.

## File targets

### Engine core
- `engine/core/quest/quest_runtime.h`
- `engine/core/quest/quest_runtime.cpp`
- `engine/core/codex/codex_runtime.h`
- `engine/core/codex/codex_runtime.cpp`
- `engine/core/crafting/crafting_runtime.h`
- `engine/core/crafting/crafting_runtime.cpp`
- `engine/core/profession/profession_runtime.h`
- `engine/core/profession/profession_runtime.cpp`
- `engine/core/achievement/achievement_runtime.h`
- `engine/core/achievement/achievement_runtime.cpp`
- `engine/core/inventory/independent_item_runtime.h`
- `engine/core/inventory/independent_item_runtime.cpp`
- `engine/core/inventory/socketed_equipment_runtime.h`
- `engine/core/inventory/socketed_equipment_runtime.cpp`

### Editor
- `editor/quest/quest_editor_panel.h`
- `editor/quest/quest_editor_panel.cpp`
- `editor/codex/codex_editor_panel.h`
- `editor/codex/codex_editor_panel.cpp`
- `editor/crafting/crafting_editor_panel.h`
- `editor/crafting/crafting_editor_panel.cpp`
- `editor/profession/profession_editor_panel.h`
- `editor/profession/profession_editor_panel.cpp`
- `editor/achievement/achievement_editor_panel.h`
- `editor/achievement/achievement_editor_panel.cpp`
- `editor/inventory/independent_item_panel.h`
- `editor/inventory/independent_item_panel.cpp`
- `editor/inventory/socketed_equipment_panel.h`
- `editor/inventory/socketed_equipment_panel.cpp`

### Schemas
- `content/schemas/quests.schema.json`
- `content/schemas/codex_entries.schema.json`
- `content/schemas/crafting_recipes.schema.json`
- `content/schemas/professions.schema.json`
- `content/schemas/achievements.schema.json`
- `content/schemas/independent_items.schema.json`
- `content/schemas/socketed_equipment.schema.json`

### Import mapping
- `engine/import/mapping/systems_plugin_mapping.h`
- `engine/import/mapping/systems_plugin_mapping.cpp`

### Tests
- `tests/unit/test_quest_runtime.cpp`
- `tests/unit/test_codex_runtime.cpp`
- `tests/unit/test_crafting_runtime.cpp`
- `tests/unit/test_independent_item_runtime.cpp`
- `tests/unit/test_socketed_equipment_runtime.cpp`
- `tests/integration/test_systems_native_absorption.cpp`

## Issue breakdown

### Issue 4.1 — Quest / codex / achievement suite
Acceptance criteria:
- quests, journals, bestiary/codex, glossary, and achievements are native systems with editor surfaces,
- imported evidence can map common plugin-derived data structures,
- diagnostics flag unsupported advanced behavior clearly.

### Issue 4.2 — Crafting / professions suite
Acceptance criteria:
- recipes, resource requirements, profession progression, and outputs are native,
- authoring and preview exist in editor,
- import evidence can classify common crafting/profession behaviors.

### Issue 4.3 — Independent items and socketed equipment suite
Acceptance criteria:
- unique-instance gear and socket/shard/materia-like systems exist natively,
- item identity and serialization are stable,
- migration can map common historical equipment-enhancement behavior into native status categories.

---

# 12. Phase 5 — Action / interaction / map runtime and PGMMV coverage expansion

## Goal
Build the action-oriented runtime owners needed both for plugin absorption and for deeper PGMMV support.

## Deliverables
- collision and movement runtime,
- triggers and interactions,
- platformer-specific movement model,
- top-down action interaction model,
- hazard and projectile map runtime,
- moving platforms and advanced stage mechanics,
- broader PGMMV stage and controller mapping.

## File targets

### Engine core
- `engine/core/action/movement_runtime.h`
- `engine/core/action/movement_runtime.cpp`
- `engine/core/action/collision_runtime.h`
- `engine/core/action/collision_runtime.cpp`
- `engine/core/action/trigger_runtime.h`
- `engine/core/action/trigger_runtime.cpp`
- `engine/core/action/platformer_runtime.h`
- `engine/core/action/platformer_runtime.cpp`
- `engine/core/action/topdown_action_runtime.h`
- `engine/core/action/topdown_action_runtime.cpp`
- `engine/core/action/map_projectile_runtime.h`
- `engine/core/action/map_projectile_runtime.cpp`
- `engine/core/action/moving_platform_runtime.h`
- `engine/core/action/moving_platform_runtime.cpp`
- `engine/core/action/checkpoint_runtime.h`
- `engine/core/action/checkpoint_runtime.cpp`

### PGMMV expansion mapping
- `engine/import/pgmmv/pgmmv_stage_mapper.h`
- `engine/import/pgmmv/pgmmv_stage_mapper.cpp`
- `engine/import/pgmmv/pgmmv_controller_mapping.h`
- `engine/import/pgmmv/pgmmv_controller_mapping.cpp`
- `engine/import/pgmmv/pgmmv_trigger_mapping.h`
- `engine/import/pgmmv/pgmmv_trigger_mapping.cpp`

### Editor
- `editor/action/collision_editor_panel.h`
- `editor/action/collision_editor_panel.cpp`
- `editor/action/trigger_editor_panel.h`
- `editor/action/trigger_editor_panel.cpp`
- `editor/action/platformer_profile_panel.h`
- `editor/action/platformer_profile_panel.cpp`
- `editor/action/topdown_action_panel.h`
- `editor/action/topdown_action_panel.cpp`
- `editor/action/moving_platform_panel.h`
- `editor/action/moving_platform_panel.cpp`

### Schemas
- `content/schemas/action_movement_profiles.schema.json`
- `content/schemas/collision_regions.schema.json`
- `content/schemas/triggers.schema.json`
- `content/schemas/platformer_profiles.schema.json`
- `content/schemas/map_projectiles.schema.json`
- `content/schemas/moving_platforms.schema.json`
- `content/schemas/checkpoints.schema.json`

### Tests
- `tests/unit/test_movement_runtime.cpp`
- `tests/unit/test_collision_runtime.cpp`
- `tests/unit/test_trigger_runtime.cpp`
- `tests/unit/test_platformer_runtime.cpp`
- `tests/unit/test_moving_platform_runtime.cpp`
- `tests/integration/test_pgmmv_action_mapping.cpp`
- `tests/integration/test_action_native_absorption.cpp`

## Issue breakdown

### Issue 5.1 — Action and collision runtime suite
Acceptance criteria:
- map interactions, hazards, collision, and triggers are native,
- action-hybrid and platformer project modes are real runtime lanes,
- editor authoring exists for action and collision constructs.

### Issue 5.2 — PGMMV coverage expansion
Acceptance criteria:
- broader stage/controller/trigger mapping exists for PGMMV imports,
- moving platforms and advanced trigger constructs are classified honestly,
- diagnostics show what is preserved vs approximated vs blocked.

### Issue 5.3 — Native import targets for action-like plugin ecosystems
Acceptance criteria:
- common action/movement plugin evidence can map into native action runtime statuses,
- migration reports recommend project modes accurately,
- integration tests verify mode-aware behavior.

---

# 13. Phase 6 — Lighting / camera / depth / presentation substrate

## Goal
Deliver the native visual substrate required by both plugin-demand absorption and later HD-2D / 2.5D presentation.

## Deliverables
- dynamic lights,
- shadow proxies and shadow casting rules,
- scene camera profiles,
- depth and elevation layers,
- presentation profiles,
- parallax and layered scene composition rules,
- cinematic rails and transitions.

## File targets

### Engine core
- `engine/core/render/light_runtime.h`
- `engine/core/render/light_runtime.cpp`
- `engine/core/render/shadow_proxy_runtime.h`
- `engine/core/render/shadow_proxy_runtime.cpp`
- `engine/core/render/scene_camera_profiles.h`
- `engine/core/render/scene_camera_profiles.cpp`
- `engine/core/render/depth_layer_runtime.h`
- `engine/core/render/depth_layer_runtime.cpp`
- `engine/core/render/presentation_profile_runtime.h`
- `engine/core/render/presentation_profile_runtime.cpp`
- `engine/core/render/cinematic_rail_runtime.h`
- `engine/core/render/cinematic_rail_runtime.cpp`

### Editor
- `editor/render/light_editor_panel.h`
- `editor/render/light_editor_panel.cpp`
- `editor/render/shadow_proxy_panel.h`
- `editor/render/shadow_proxy_panel.cpp`
- `editor/render/scene_camera_profile_panel.h`
- `editor/render/scene_camera_profile_panel.cpp`
- `editor/render/depth_layer_panel.h`
- `editor/render/depth_layer_panel.cpp`
- `editor/render/presentation_profile_panel.h`
- `editor/render/presentation_profile_panel.cpp`
- `editor/render/cinematic_rail_panel.h`
- `editor/render/cinematic_rail_panel.cpp`

### Schemas
- `content/schemas/lights.schema.json`
- `content/schemas/shadow_proxies.schema.json`
- `content/schemas/scene_camera_profiles.schema.json`
- `content/schemas/depth_layers.schema.json`
- `content/schemas/presentation_profiles.schema.json`
- `content/schemas/cinematic_rails.schema.json`

### Import mapping
- `engine/import/mapping/presentation_plugin_mapping.h`
- `engine/import/mapping/presentation_plugin_mapping.cpp`

### Tests
- `tests/unit/test_light_runtime.cpp`
- `tests/unit/test_shadow_proxy_runtime.cpp`
- `tests/unit/test_scene_camera_profiles.cpp`
- `tests/unit/test_depth_layer_runtime.cpp`
- `tests/integration/test_presentation_native_absorption.cpp`

## Issue breakdown

### Issue 6.1 — Lighting and shadow suite
Acceptance criteria:
- dynamic lights and shadows are native and authorable,
- project modes can opt into or bound their usage,
- import evidence can map common lighting/shadow plugin behaviors.

### Issue 6.2 — Camera and cinematic suite
Acceptance criteria:
- camera profiles and cinematic rails are native and previewable,
- battle, map, dialogue, and cutscene systems can consume them,
- migration can map common camera-behavior evidence.

### Issue 6.3 — Depth and layered presentation suite
Acceptance criteria:
- elevation, parallax, depth layers, and presentation profiles are native,
- these systems remain compatible with later HD-2D / 2.5D lane needs,
- diagnostics can explain unsupported visual mappings.

---

# 14. Phase 7 — HD-2D / 2.5D native presentation lane

## Goal
Turn HD-2D / 2.5D from an idea into a gated native product lane with runtime, editor, schema, migration logic, and battle parity considerations.

## Deliverables
- HD-2D / 2.5D project mode rules,
- scene profile templates,
- elevation/depth renderer behavior,
- materialized sprite / billboard rules,
- battle presentation parity rules,
- cinematic staging integration,
- import recommendation logic for projects that should land in `hd2d_25d` mode.

## File targets

### Engine core
- `engine/core/render/hd2d_scene_profile.h`
- `engine/core/render/hd2d_scene_profile.cpp`
- `engine/core/render/hd2d_depth_resolver.h`
- `engine/core/render/hd2d_depth_resolver.cpp`
- `engine/core/render/material_sprite_runtime.h`
- `engine/core/render/material_sprite_runtime.cpp`
- `engine/core/render/hd2d_battle_presentation.h`
- `engine/core/render/hd2d_battle_presentation.cpp`
- `engine/core/render/scene_profile_recommender.h`
- `engine/core/render/scene_profile_recommender.cpp`

### Editor
- `editor/render/hd2d_scene_profile_panel.h`
- `editor/render/hd2d_scene_profile_panel.cpp`
- `editor/render/material_sprite_panel.h`
- `editor/render/material_sprite_panel.cpp`
- `editor/render/hd2d_battle_presentation_panel.h`
- `editor/render/hd2d_battle_presentation_panel.cpp`

### Schemas
- `content/schemas/hd2d_scene_profiles.schema.json`
- `content/schemas/material_sprites.schema.json`
- `content/schemas/hd2d_battle_presentation.schema.json`

### Import/migration
- `engine/import/mapping/hd2d_recommendation_mapping.h`
- `engine/import/mapping/hd2d_recommendation_mapping.cpp`

### Tests
- `tests/unit/test_hd2d_scene_profile.cpp`
- `tests/unit/test_hd2d_depth_resolver.cpp`
- `tests/unit/test_hd2d_battle_presentation.cpp`
- `tests/integration/test_hd2d_project_mode_recommendation.cpp`

## Issue breakdown

### Issue 7.1 — HD-2D / 2.5D scene profile system
Acceptance criteria:
- `hd2d_25d` is a real project mode with documented rules,
- scene profile authoring exists in editor,
- depth/elevation/material sprite behavior is deterministic and schema-backed.

### Issue 7.2 — HD-2D / 2.5D battle parity
Acceptance criteria:
- battle presentation can consume scene depth/camera rules,
- battle camera, sequence, and effects remain compatible,
- a focused battle parity test suite exists.

### Issue 7.3 — Mode recommendation and migration logic
Acceptance criteria:
- imported projects can be recommended into `hd2d_25d` when evidence warrants it,
- migration reports explain why a recommendation was made,
- unsupported visual expectations are flagged explicitly.

---

# 15. Phase 8 — Migration wizard, diagnostics dashboards, and confidence proofs

## Goal
Make migration and absorption visible, inspectable, and measurable.

## Deliverables
- extended migration wizard,
- per-lane dashboards,
- import confidence checklists,
- absorption status summaries,
- selective import/repair workflows,
- JSONL/export artifacts for CI and dashboards.

## File targets

### Engine and tooling
- `engine/import/migration/import_confidence_model.h`
- `engine/import/migration/import_confidence_model.cpp`
- `engine/import/migration/lane_status_rollup.h`
- `engine/import/migration/lane_status_rollup.cpp`
- `tools/ci/run_native_absorption_gate.ps1`
- `tools/reports/render_import_confidence_report.py`

### Editor
- `editor/import/migration_dashboard_panel.h`
- `editor/import/migration_dashboard_panel.cpp`
- `editor/import/import_confidence_panel.h`
- `editor/import/import_confidence_panel.cpp`
- `editor/import/post_import_repair_panel.h`
- `editor/import/post_import_repair_panel.cpp`

### Schemas
- `content/schemas/import/import_confidence_report.schema.json`
- `content/schemas/import/lane_status_rollup.schema.json`

### Tests
- `tests/unit/test_import_confidence_model.cpp`
- `tests/unit/test_lane_status_rollup.cpp`
- `tests/integration/test_migration_dashboard_projection.cpp`

## Issue breakdown

### Issue 8.1 — Extend migration wizard
Acceptance criteria:
- one wizard can route MZ-derived, PGMMV-derived, and plugin-evidence-derived imports through lane-aware steps,
- selective import and repair hooks exist,
- preflight and post-import summaries are visible.

### Issue 8.2 — Add dashboards and confidence panels
Acceptance criteria:
- dashboards show runtime/editor/schema/migration/test/gate status by lane,
- import confidence and feature loss are visible,
- per-project summaries are exportable.

### Issue 8.3 — Add post-import repair workflows
Acceptance criteria:
- blocked or approximated features can launch into repair panels or authoring surfaces,
- recommendations are actionable,
- diagnostics connect directly to repair entry points.

---

# 16. Phase 9 — Release hardening, matrices, and truthfulness gates

## Goal
Prove what URPG can and cannot currently absorb, import, and ship.

## Deliverables
- release-readiness matrix,
- native absorption CI gate,
- PGMMV intake CI gate,
- per-lane focused test gates,
- published support matrix language,
- truthful README/docs updates.

## File targets

### Docs
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PGMMV_SUPPORT_MATRIX.md`
- `docs/NATIVE_ABSORPTION_SUPPORT_MATRIX.md`
- `docs/IMPORT_CONFIDENCE_CHECKLIST.md`

### CI tooling
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/check_pgmmv_support_matrix.ps1`
- `tools/ci/check_native_absorption_matrix.ps1`

### Tests / dashboards
- `tests/integration/test_release_matrix_claims.cpp`
- `tests/integration/test_pgmmv_support_claims.cpp`
- `tests/integration/test_native_absorption_claims.cpp`

## Issue breakdown

### Issue 9.1 — Add release-readiness matrix
Acceptance criteria:
- every lane has runtime/editor/schema/migration/test/gate status,
- public claims are bounded by evidence,
- matrix supports `safe_to_ship`, `preview`, and `experimental` markers.

### Issue 9.2 — Add CI gates by lane
Acceptance criteria:
- native absorption gate validates schemas, tests, and report generation,
- PGMMV gate validates detector, preflight, import vertical slice, and support matrix consistency,
- failures are categorized by lane.

### Issue 9.3 — Publish support matrices and confidence checklist
Acceptance criteria:
- import status is never claimed without a measurable checklist,
- approximated and blocked features are visible,
- HD-2D / 2.5D recommendation logic is documented and tested.

---

# 17. Cross-phase dependency map

## Critical prerequisites
- Phase 1 must land before Phases 3–8 can scale cleanly.
- Phase 2 should begin immediately after Phase 1 starts because PGMMV intake needs the shared diagnostics/report substrate.
- Phase 3 should begin before Phase 7 is treated as strategic, because HD-2D / 2.5D without battle/UI/dialogue parity becomes renderer theater.
- Phase 5 must begin before broader PGMMV compatibility claims are made.
- Phase 6 must substantially land before Phase 7 can be called real.
- Phase 8 should begin once Phase 2 and Phase 3 have real vertical slices, not at the very end.
- Phase 9 must run continuously once the first real import and first real absorption slice exist.

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
10. Phase 9

---

# 18. Milestones

## Milestone A — Governance + substrate + PGMMV intake foundation
Includes:
- Phase 0 complete
- Phase 1 substantially complete
- Phase 2 intake foundation complete

Exit criteria:
- canonical taxonomy exists,
- project modes, diagnostics, timeline, and UI substrate exist,
- PGMMV is a registered source engine,
- detector and preflight summary are real,
- one PGMMV import vertical slice exists.

## Milestone B — Major dependency replacement I
Includes:
- Phase 3 complete to production baseline

Exit criteria:
- battle, UI, and dialogue/cutscene no longer rely on plugin-shaped core delivery,
- editor ownership exists,
- migration can classify common evidence in those lanes,
- battle/UI/dialogue focused CI gates pass.

## Milestone C — Major dependency replacement II + action coverage
Includes:
- Phase 4 complete to production baseline
- Phase 5 complete to production baseline

Exit criteria:
- systems expansion lanes are native,
- action/platformer/top-down interaction lanes are real,
- PGMMV support moves beyond trivial demos,
- mode-aware migration recommendations are real.

## Milestone D — Strategic differentiation
Includes:
- Phase 6 complete to production baseline
- Phase 7 complete to production baseline

Exit criteria:
- lighting, shadows, camera, depth, and presentation are native,
- HD-2D / 2.5D is a documented, schema-backed, editor-backed, tested product lane,
- battle and map presentation parity exists.

## Milestone E — Truthful product proof
Includes:
- Phase 8 complete
- Phase 9 complete

Exit criteria:
- migration dashboards and confidence reports are real,
- support matrices and release-readiness matrices are publishable,
- URPG can make honest claims about PGMMV intake and native absorption coverage.

---

# 19. Suggested GitHub epic structure

## Epic 1 — Governance and taxonomy
Child issues:
- plugin-category taxonomy
- source-engine taxonomy
- scorecard
- status vocabulary
- labels and templates

## Epic 2 — Shared substrate and project modes
Child issues:
- project modes
- diagnostics lane
- timeline substrate
- UI layout substrate
- shared migration report model

## Epic 3 — PGMMV intake foundation
Child issues:
- source engine registration
- detector
- preflight summary
- project reader
- normalized model
- first migration analyzer

## Epic 4 — PGMMV first import vertical slice
Child issues:
- stage mapping
- sprite import
- collision mapping
- spawn and trigger mapping
- editor report surface

## Epic 5 — Battle / UI / Dialogue absorption suite
Child issues:
- battle turn models
- battle sequences
- battle projectiles
- battle cameras
- battle UI
- UI theme and menu canvas
- dialogue graph and cutscene graph

## Epic 6 — Systems expansion suite
Child issues:
- quests
- codex / bestiary
- crafting / professions
- achievements
- independent items
- socketed equipment

## Epic 7 — Action / interaction / PGMMV runtime expansion
Child issues:
- movement and collision
- triggers
- platformer mode
- top-down action mode
- moving platforms
- PGMMV controller mapping
- PGMMV advanced trigger mapping

## Epic 8 — Presentation substrate
Child issues:
- lights
- shadows
- scene camera profiles
- depth layers
- cinematic rails
- presentation profiles

## Epic 9 — HD-2D / 2.5D lane
Child issues:
- scene profiles
- material sprites
- depth resolver
- battle parity
- mode recommendation logic

## Epic 10 — Migration dashboards and release proof
Child issues:
- wizard extension
- dashboards
- confidence model
- support matrices
- CI gates
- release matrix

---

# 20. Acceptance rules for any lane to count as shipped

A lane is not considered truly delivered unless all of the following exist:

1. native runtime owner,
2. native schema contracts,
3. native editor authoring surface,
4. migration/import mapping or explicit unsupported declaration,
5. diagnostics and export/report visibility,
6. unit and integration anchors,
7. focused CI gate coverage,
8. truthfully bounded public status.

Additional PGMMV-specific rule:

9. source-engine support claims must be backed by fixture coverage and a support matrix, not by anecdotal success on one project.

---

# 21. Definition of done

This roadmap is complete when:

1. PGMMV is a real intake lane with detector, reader, normalized model, migration analyzer, editor workflow, tests, and support matrix.
2. The major plugin demand centers identified by research have native owners.
3. The most common historical plugin dependencies are replaced by native systems.
4. Imported projects can be analyzed and migrated into those native systems with explicit diagnostics.
5. Action, platformer, lighting, camera, depth, and presentation systems are native product lanes rather than ad hoc additions.
6. HD-2D / 2.5D is a first-class gated lane, not a side experiment.
7. Release matrices, confidence checklists, and CI gates prove what URPG can and cannot currently absorb or import.

---

# 22. Immediate next actions

## Doc actions
1. Copy this file into `docs/MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP.md`
2. Link it from the README documentation index
3. Link it from `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
4. Link it from `docs/PROGRAM_COMPLETION_STATUS.md`
5. Mark the earlier standalone PGMMV plan and standalone plugin-absorption roadmap as superseded planning sources

## Implementation actions
1. Execute Phase 0 Issue 0.1–0.5 immediately
2. Execute Phase 1 Issue 1.1–1.4 immediately after Phase 0 starts landing
3. Start Phase 2 Issue 2.1 and 2.2 as the first source-engine intake slice
4. Start Phase 3 Issue 3.1 in parallel once shared substrate exists

## Repo-management actions
1. Create 10 epics matching the structure above
2. Add issue templates requiring lane, editor impact, schema impact, migration impact, diagnostics impact, and HD-2D relevance
3. Add CI stubs for:
   - native absorption gate
   - PGMMV intake gate
   - support-matrix consistency checks

---

# 23. Recommended first vertical slices

## Slice A — PGMMV intake foundation
Why first:
- turns the earlier PGMMV support plan into real code,
- proves source-engine intake architecture,
- forces preflight/reporting to become real early.

### File targets
- `engine/import/source/source_engine_registry.*`
- `engine/import/pgmmv/pgmmv_project_detector.*`
- `engine/import/pgmmv/pgmmv_preflight_summary.*`
- `editor/import/pgmmv_preflight_panel.*`
- `content/schemas/import/pgmmv_preflight_summary.schema.json`
- `tests/unit/test_pgmmv_project_detector.cpp`

### Exit criteria
- PGMMV source engine registered,
- detector works,
- preflight summary visible in editor,
- CI can validate detection on fixtures.

## Slice B — Battle UI + Battle Sequence + Camera + Migration Report
Why first:
- directly attacks one of the largest plugin dependency centers,
- produces visible payoff fast,
- proves shared UI, timeline, diagnostics, and migration infrastructure,
- naturally aligns with later HD-2D / 2.5D work.

### File targets
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

### Exit criteria
- battle UI authored in editor,
- sequence preview works,
- camera preview works,
- import report classifies common battle-plugin evidence,
- focused CI gate passes.

---

# 24. Final note

This roadmap matters because it replaces two failure modes at the same time:

1. “Support more plugins” chaos.
2. “Import another engine” chaos.

The fix is the same for both:

- explicit native owners,
- explicit source-engine intake lanes,
- explicit schemas,
- explicit editor surfaces,
- explicit migration logic,
- explicit diagnostics,
- explicit proof of readiness.

That is how URPG stops being “RPG Maker plus more plugins” or “PGMMV but imported somehow,” and becomes a native-first creation engine with a believable intake and migration story.


---

# 25. Planning-closure additions before roadmap freeze (v2)

This section captures the final high-value additions identified during the last gap pass and promotes them from “good ideas” to **in-scope roadmap requirements**.

These additions exist because native runtime ownership alone is not enough. URPG also needs:

- native **authoring UX** that replaces plugin parameter-sheet pain,
- native **instance-based equipment and inventory individuality**,
- a fuller **action-runtime product lane** beyond “ARPG support exists,”
- a broader **HD-2D / 2.5D presentation stack** than a single render mode,
- explicit **plugin-evidence intake and replacement mapping**,
- recurring **RPG glue systems** that users historically buy as plugin packs,
- and explicit **accessibility / localization** foundations so late-stage productization does not become a rewrite.

These additions are now part of the authoritative plan.

## 25.1 New mandatory cross-cutting track

Add a fifth always-on program track:

- **Track E — Authoring UX, accessibility, localization, and product glue**

Track E applies to all phases after Phase 1 and cannot be deferred to “polish only.”

It must ensure that every major runtime lane is evaluated for:

- authoring surface completeness,
- template-driven no-code / low-code workflows where appropriate,
- localization readiness,
- accessibility hooks,
- and integration with shared progression, schedule, codex, and state-routing systems.

## 25.2 Lane additions and expansions

### A. Native authoring UX suite
Add or expand the following as first-class editor-authority lanes:

- dialogue graph editor,
- cutscene/state-sequence editor,
- HUD/layout composer,
- conditional logic graph,
- event-flow visualizer,
- battle sequence canvas,
- cinematic rail/timeline authoring,
- shared property-inspector patterns for reusable authoring ergonomics.

This is the editor layer that replaces event spaghetti and plugin parameter sheets with native workflows.

### B. Instance-based items and visual equipment suite
Expand the inventory/equipment lane to explicitly include:

- unique per-instance items,
- randomized or rolled properties,
- affixes / rarity / provenance,
- durability and repair metadata,
- sockets / gems / shards / materia-like behaviors,
- paper-doll / layered equipment presentation,
- import and save semantics centered on item instances rather than database-only references.

### C. Full action-runtime product lane
Expand the action/platformer lane to explicitly include:

- hitbox / hurtbox authoring,
- melee arc profiles,
- projectile authoring and state rules,
- controller remapping,
- input buffering,
- dash / cancel / interrupt / invulnerability windows,
- enemy aggro / patrol / reaction state authoring,
- action camera presets,
- top-down / side-action / beat-em-up presets,
- follower and companion action behaviors.

### D. Full HD-2D / 2.5D presentation stack
Expand the presentation lane to explicitly include:

- elevation and occlusion rules,
- sprite sorting against height and geometry,
- material/billboard rules,
- reflections,
- fog and atmosphere profiles,
- post-process profiles,
- camera rigs, rails, and shot presets,
- parallax depth layers,
- safe style templates so users can target a coherent HD-2D look without shader expertise.

### E. Plugin evidence intake and native replacement matrix
Strengthen intake and migration planning by requiring:

- plugin-stack scanning,
- plugin-category classification,
- mapping from plugin evidence to native URPG target lanes,
- confidence scoring,
- unsupported / blocked construct diagnostics,
- repair recommendations and launch points into the right editor surfaces.

### F. RPG glue systems suite
Promote the following from optional nice-to-have systems to named product lanes:

- clock / calendar / time-of-day runtime,
- day-night routing hooks,
- dynamic audio state router,
- reputation / faction systems,
- NPC schedules and routine planners,
- codex / glossary / lorebook unification,
- achievements / milestones / challenge tracking,
- professions / gathering / crafting integration.

### G. Accessibility and localization foundation
Add explicit productization lanes for:

- text style abstraction,
- subtitle / backlog / dialogue history,
- input-icon abstraction across keyboard / controller modes,
- font fallback and locale-aware text rendering,
- localization asset pipeline,
- colorblind-safe UI theme hooks,
- reduced shake / reduced flash / reduced effect intensity settings,
- scalable UI text and layout breakpoints.

## 25.3 Phase deltas

These additions modify the roadmap phases as follows.

### Phase 1 delta — shared substrate and project-mode foundation
Add:

- shared input-icon abstraction,
- locale and text-style abstraction,
- common timeline/editor shell ergonomics,
- project-mode accessibility defaults,
- item-instance identity primitives,
- shared schedule/calendar/time router.

### Phase 3 delta — battle / UI / dialogue absorption
Expand deliverables to explicitly include:

- dialogue graph editor,
- cutscene/state-sequence editor,
- HUD composer,
- battle UI composer,
- battle sequence canvas,
- battle hitbox / hurtbox and projectile authoring,
- subtitle/backlog/message-history support.

### Phase 4 delta — systems expansion
Expand deliverables to include:

- calendar/day-night,
- reputation/factions,
- NPC schedules,
- codex/glossary unification,
- paper-doll equipment presentation,
- item-instance metadata and repair/upgrade hooks.

### Phase 5 delta — action / interaction runtime
Expand deliverables to include:

- controller remapping,
- input buffering,
- action camera presets,
- aggro/patrol/reaction authoring,
- companion action behaviors,
- top-down / side-action / beat-em-up templates.

### Phase 6 delta — lighting / camera / depth
Expand deliverables to include:

- fog/atmosphere profiles,
- post-process bundles,
- reflection hooks,
- cinematic shot presets,
- editor-safe style templates.

### Phase 7 delta — HD-2D / 2.5D lane
Expand deliverables to include:

- occlusion/elevation sorting rules,
- parallax depth authoring,
- material-sprite style presets,
- battle-scene parity for camera, fog, and post-fx,
- style-guided scene profiles for coherent output.

### Phase 8 delta — migration wizard and confidence proofs
Expand deliverables to include:

- plugin-stack scan report,
- native replacement matrix view,
- editor repair-route links,
- accessibility/localization readiness summary.

### Phase 9 delta — release hardening
Add:

- accessibility readiness matrix,
- localization readiness matrix,
- authoring UX readiness matrix,
- “native replacement coverage by plugin category” report.

## 25.4 New file targets

## Engine runtime and shared substrate
- `engine/core/ui/accessibility_runtime.h`
- `engine/core/ui/accessibility_runtime.cpp`
- `engine/core/ui/input_icon_registry.h`
- `engine/core/ui/input_icon_registry.cpp`
- `engine/core/text/text_style_registry.h`
- `engine/core/text/text_style_registry.cpp`
- `engine/core/text/localization_runtime.h`
- `engine/core/text/localization_runtime.cpp`
- `engine/core/time/calendar_runtime.h`
- `engine/core/time/calendar_runtime.cpp`
- `engine/core/time/day_night_router.h`
- `engine/core/time/day_night_router.cpp`
- `engine/core/social/faction_runtime.h`
- `engine/core/social/faction_runtime.cpp`
- `engine/core/social/reputation_runtime.h`
- `engine/core/social/reputation_runtime.cpp`
- `engine/core/npc/npc_schedule_runtime.h`
- `engine/core/npc/npc_schedule_runtime.cpp`
- `engine/core/inventory/item_instance_runtime.h`
- `engine/core/inventory/item_instance_runtime.cpp`
- `engine/core/inventory/paper_doll_runtime.h`
- `engine/core/inventory/paper_doll_runtime.cpp`
- `engine/core/action/hitbox_runtime.h`
- `engine/core/action/hitbox_runtime.cpp`
- `engine/core/action/hurtbox_runtime.h`
- `engine/core/action/hurtbox_runtime.cpp`
- `engine/core/action/input_buffer_runtime.h`
- `engine/core/action/input_buffer_runtime.cpp`
- `engine/core/action/controller_binding_runtime.h`
- `engine/core/action/controller_binding_runtime.cpp`
- `engine/core/render/fog_profile_runtime.h`
- `engine/core/render/fog_profile_runtime.cpp`
- `engine/core/render/postfx_profile_runtime.h`
- `engine/core/render/postfx_profile_runtime.cpp`
- `engine/core/render/reflection_runtime.h`
- `engine/core/render/reflection_runtime.cpp`
- `engine/core/render/occlusion_sort_runtime.h`
- `engine/core/render/occlusion_sort_runtime.cpp`

## Import and migration
- `engine/import/plugin/plugin_stack_scanner.h`
- `engine/import/plugin/plugin_stack_scanner.cpp`
- `engine/import/plugin/plugin_category_classifier.h`
- `engine/import/plugin/plugin_category_classifier.cpp`
- `engine/import/plugin/native_replacement_matrix.h`
- `engine/import/plugin/native_replacement_matrix.cpp`
- `engine/import/report/accessibility_readiness_report.h`
- `engine/import/report/accessibility_readiness_report.cpp`
- `engine/import/report/localization_readiness_report.h`
- `engine/import/report/localization_readiness_report.cpp`

## Editor and authoring surfaces
- `editor/dialogue/dialogue_graph_editor_panel.h`
- `editor/dialogue/dialogue_graph_editor_panel.cpp`
- `editor/cutscene/cutscene_sequence_editor_panel.h`
- `editor/cutscene/cutscene_sequence_editor_panel.cpp`
- `editor/ui/hud_composer_panel.h`
- `editor/ui/hud_composer_panel.cpp`
- `editor/logic/conditional_logic_graph_panel.h`
- `editor/logic/conditional_logic_graph_panel.cpp`
- `editor/action/hitbox_editor_panel.h`
- `editor/action/hitbox_editor_panel.cpp`
- `editor/action/hurtbox_editor_panel.h`
- `editor/action/hurtbox_editor_panel.cpp`
- `editor/action/controller_binding_panel.h`
- `editor/action/controller_binding_panel.cpp`
- `editor/inventory/item_instance_panel.h`
- `editor/inventory/item_instance_panel.cpp`
- `editor/inventory/paper_doll_panel.h`
- `editor/inventory/paper_doll_panel.cpp`
- `editor/time/calendar_editor_panel.h`
- `editor/time/calendar_editor_panel.cpp`
- `editor/time/day_night_router_panel.h`
- `editor/time/day_night_router_panel.cpp`
- `editor/social/faction_editor_panel.h`
- `editor/social/faction_editor_panel.cpp`
- `editor/social/reputation_editor_panel.h`
- `editor/social/reputation_editor_panel.cpp`
- `editor/npc/npc_schedule_panel.h`
- `editor/npc/npc_schedule_panel.cpp`
- `editor/render/fog_profile_panel.h`
- `editor/render/fog_profile_panel.cpp`
- `editor/render/postfx_profile_panel.h`
- `editor/render/postfx_profile_panel.cpp`
- `editor/render/reflection_panel.h`
- `editor/render/reflection_panel.cpp`
- `editor/import/plugin_stack_report_panel.h`
- `editor/import/plugin_stack_report_panel.cpp`
- `editor/import/native_replacement_matrix_panel.h`
- `editor/import/native_replacement_matrix_panel.cpp`
- `editor/import/accessibility_readiness_panel.h`
- `editor/import/accessibility_readiness_panel.cpp`
- `editor/import/localization_readiness_panel.h`
- `editor/import/localization_readiness_panel.cpp`

## Schemas
- `content/schemas/text_styles.schema.json`
- `content/schemas/localization.schema.json`
- `content/schemas/calendar.schema.json`
- `content/schemas/day_night_routes.schema.json`
- `content/schemas/factions.schema.json`
- `content/schemas/reputation.schema.json`
- `content/schemas/npc_schedules.schema.json`
- `content/schemas/item_instances.schema.json`
- `content/schemas/paper_doll.schema.json`
- `content/schemas/hitboxes.schema.json`
- `content/schemas/hurtboxes.schema.json`
- `content/schemas/controller_bindings.schema.json`
- `content/schemas/fog_profiles.schema.json`
- `content/schemas/postfx_profiles.schema.json`
- `content/schemas/reflections.schema.json`
- `content/schemas/import/plugin_stack_report.schema.json`
- `content/schemas/import/native_replacement_matrix.schema.json`
- `content/schemas/import/accessibility_readiness.schema.json`
- `content/schemas/import/localization_readiness.schema.json`

## Tests
- `tests/unit/test_accessibility_runtime.cpp`
- `tests/unit/test_localization_runtime.cpp`
- `tests/unit/test_calendar_runtime.cpp`
- `tests/unit/test_day_night_router.cpp`
- `tests/unit/test_faction_runtime.cpp`
- `tests/unit/test_npc_schedule_runtime.cpp`
- `tests/unit/test_item_instance_runtime.cpp`
- `tests/unit/test_paper_doll_runtime.cpp`
- `tests/unit/test_hitbox_runtime.cpp`
- `tests/unit/test_input_buffer_runtime.cpp`
- `tests/unit/test_fog_profile_runtime.cpp`
- `tests/unit/test_postfx_profile_runtime.cpp`
- `tests/unit/test_plugin_stack_scanner.cpp`
- `tests/unit/test_native_replacement_matrix.cpp`
- `tests/integration/test_authoring_workflow_roundtrip.cpp`
- `tests/integration/test_accessibility_and_localization_readiness.cpp`

## 25.5 New epics and issue packs

### Epic 10 — Authoring UX and visual authoring parity
Issues:
1. dialogue graph editor
2. cutscene/state-sequence editor
3. HUD composer
4. conditional logic graph
5. shared visual timeline/canvas conventions
6. authoring workflow roundtrip tests

### Epic 11 — Item individuality and equipment visualization
Issues:
1. item-instance runtime
2. affix/rarity/durability/provenance metadata
3. socket/shard/materia-like expansion
4. paper-doll and layered equipment
5. save/import/update semantics for item instances

### Epic 12 — Full action-runtime expansion
Issues:
1. hitbox/hurtbox runtime and editor
2. controller remapping and input buffering
3. melee/profile and projectile authoring completion
4. aggro/patrol/reaction state authoring
5. top-down / side-action / beat-em-up project templates
6. companion/follower action behaviors

### Epic 13 — RPG glue systems
Issues:
1. calendar and day-night routing
2. faction/reputation systems
3. NPC schedules and routines
4. codex/glossary/lorebook unification
5. dynamic audio state router integration
6. achievements/crafting/professions integration hardening

### Epic 14 — Accessibility and localization foundation
Issues:
1. text-style abstraction
2. subtitle / backlog / dialogue history
3. input-icon abstraction
4. font fallback and locale-aware rendering
5. localization pipeline and schema tooling
6. accessibility settings and validation

### Epic 15 — Plugin evidence replacement matrix and migration readiness
Issues:
1. plugin-stack scanner
2. plugin-category classifier
3. native replacement matrix
4. repair-route links to editor surfaces
5. accessibility/localization readiness reporting
6. replacement coverage dashboard for release proofs

### Epic 16 — HD-2D / 2.5D presentation completion
Issues:
1. fog/post-fx/reflection substrate
2. occlusion/elevation sort rules
3. cinematic shot/rail presets
4. scene style templates
5. battle-scene parity for advanced presentation
6. safe-style authoring presets

## 25.6 Additional acceptance criteria

Planning is **not** allowed to freeze until the roadmap explicitly covers all of the following:

1. Native authoring UX exists as a named product lane, not just runtime follow-up work.
2. Item-instance and socketed / materia-like equipment systems are explicitly in scope.
3. Action-runtime expansion includes hitbox/hurtbox, controller remapping, and input buffering.
4. HD-2D / 2.5D includes fog, post-fx, occlusion, and safe-style authoring, not just billboard rules.
5. Plugin-stack evidence can be scanned and mapped to native replacement lanes.
6. Calendar/day-night, factions/reputation, and NPC schedules are explicitly represented.
7. Accessibility and localization have runtime, editor, schema, report, and release-readiness coverage.
8. Release matrices include native replacement coverage, accessibility readiness, and localization readiness.

## 25.7 Immediate next actions after this v2 update

1. Replace the in-repo master roadmap with this v2 text.
2. Add Epic 10 through Epic 16 to the GitHub planning structure.
3. Promote Track E into all issue templates and roadmap status reporting.
4. Re-check all prior phase deliverables so no lane now marked “complete” ignores Track E obligations.
5. Keep the first implementation slices the same unless issue sizing proves that Track E substrate pieces must land first:
   - PGMMV intake foundation,
   - battle UI / sequence / camera / migration report slice.

## 25.8 Revised planning-close statement

With these additions, the roadmap is now much closer to an actual planning endpoint.

Without them, URPG risked finishing planning with strong runtime ideas but weak authoring/productization coverage.

With them, the plan now explicitly covers:

- source-engine intake,
- plugin-demand absorption,
- authoring UX,
- item individuality,
- full action-runtime expansion,
- HD-2D / 2.5D presentation completion,
- migration truthfulness,
- RPG glue systems,
- accessibility,
- localization,
- and release-proof governance.
