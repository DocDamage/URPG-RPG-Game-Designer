# Schema Changelog

Status Date: 2026-04-25

This changelog records versioned schema-governance entries for schemas that are explicitly covered by the readiness-governance lane.

## Entry Format

Each governed schema entry should include:

- schema id
- version
- date
- change summary
- migration impact

## Entries

### `asset_promotion_manifest` (`asset_promotion_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-30`
- Summary: initial governed manifest for assets promoted from raw/source intake into runtime/package-ready URPG content, including source and promoted paths, license id, promotion status, preview metadata, package inclusion flags, and diagnostics
- Migration Impact: none; additive schema for curated asset promotion records

### `grid_part_authoring` (`grid_part_authoring.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-29`
- Summary: initial schema for persisted grid-part authoring documents, including stable placed-part IDs, grid coordinates, lock/hidden flags, properties, and chunk size
- Migration Impact: none; additive schema for the new grid-part authoring document serializer

### `grid_part_catalog` (`grid_part_catalog.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-29`
- Summary: initial schema for grid-part catalog definitions used by maker-style map authoring palettes
- Migration Impact: none; additive schema for the new grid-part authoring layer

### `grid_part_runtime_state` (`grid_part_runtime_state.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-29`
- Summary: initial schema for instance-id keyed grid-part runtime state such as opened chests, collected quest items, unlocked doors, defeated enemies, and fired triggers
- Migration Impact: none; additive schema for the new grid-part save/runtime state layer

### `presentation_schema` (`presentation_schema.json`)

- Version: `1.1.0`
- Date: `2026-04-29`
- Summary: adds optional `instanceId` to spatial prop instances so prop bindings and runtime state can target a stable placed prop instead of only an asset id
- Migration Impact: additive optional field; legacy prop records without `instanceId` are upgraded deterministically from map id, asset id, and per-asset index

### `visual_novel_pacing` (`visual_novel_pacing.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for visual-novel backlog, auto-advance, skip-read, and text-speed pacing controls
- Migration Impact: none; additive schema for visual-novel template authoring

### `character_appearance_composition` (`character_appearance_composition.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for layered portrait, field, and battle character appearance composition data
- Migration Impact: none; additive schema for character identity authoring

### `community_wysiwyg_feature` (`community_wysiwyg_feature.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for community-requested WYSIWYG maker feature documents
- Migration Impact: none; additive schema for maker feature authoring

### `crafting_economy_loop` (`crafting_economy_loop.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for crafting, gathering, economy, shop, and recipe-loop authoring data
- Migration Impact: none; additive schema for gameplay loop authoring

### `created_protagonist_save` (`created_protagonist_save.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for runtime-created protagonist save persistence data
- Migration Impact: none; additive schema for character creation persistence

### `cutscene_timeline` (`cutscene_timeline.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for WYSIWYG cutscene timeline tracks, beats, and runtime preview data
- Migration Impact: none; additive schema for cutscene authoring

### `dungeon3d_world` (`dungeon3d_world.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for RPG-style 3D dungeon/world floors, interactions, audio zones, automap, and template bindings
- Migration Impact: none; additive schema for 3D dungeon/world authoring

### `encounter_designer` (`encounter_designer.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for encounter table, zone, weight, and preview authoring data
- Migration Impact: none; additive schema for encounter design

### `gameplay_wysiwyg_system` (`gameplay_wysiwyg_system.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for deterministic WYSIWYG gameplay maker system documents
- Migration Impact: none; additive schema for gameplay system authoring

### `loot_generator` (`loot_generator.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for loot tables, rolls, weights, and diagnostics preview data
- Migration Impact: none; additive schema for loot authoring

### `maker_wysiwyg_feature` (`maker_wysiwyg_feature.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for expanded maker WYSIWYG feature documents and editor/runtime proof data
- Migration Impact: none; additive schema for maker feature authoring

### `map_environment_preview` (`map_environment_preview.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for map lighting, weather, region, tactical overlay, and spawn-table preview data
- Migration Impact: none; additive schema for map environment authoring

### `metroidvania_ability_gates` (`metroidvania_ability_gates.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for ability-gated progression, region unlock, and traversal preview data
- Migration Impact: none; additive schema for metroidvania-lite authoring

### `mod_store_catalog` (`mod_store_catalog.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for local mod-store catalog entries, install metadata, and package descriptors
- Migration Impact: none; additive schema for local mod catalog tooling

### `monster_collection` (`monster_collection.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for monster species, capture, party, storage, and evolution authoring data
- Migration Impact: none; additive schema for monster collector templates

### `quest_objective_graph` (`quest_objective_graph.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for quest objective graphs, dependencies, rewards, and runtime preview diagnostics
- Migration Impact: none; additive schema for quest authoring

### `relationship_affinity` (`relationship_affinity.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for relationship affinity rules, schedules, gifts, and preview state
- Migration Impact: none; additive schema for relationship authoring

### `skill_tree` (`skill_tree.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: initial schema for class progression, skill tree nodes, unlock rules, and preview diagnostics
- Migration Impact: none; additive schema for progression authoring

### `stat_allocation` (`stat_allocation.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-30`
- Summary: initial schema for actor stat-allocation pools, unspent points, per-stat point costs, gains, and caps
- Migration Impact: none; additive schema for progression/stat-allocation authoring and save preview validation

### `picture_tasks` (`picture_tasks.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-30`
- Summary: initial schema for picture-to-task/common-event bindings with trigger and enabled-state metadata
- Migration Impact: none; additive schema for picture interaction authoring

### `scoped_state_banks` (`scoped_state_banks.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-30`
- Summary: initial schema for scoped switch and variable banks across global, map, self, scoped, and JavaScript-derived state
- Migration Impact: none; additive schema for scoped state import and governance validation

### `export_preview` (`export_preview.schema.json`)

- Version: `1.1.0`
- Date: `2026-04-28`
- Summary: adds expected shipping artifact declarations so export preview can block exact-ship claims when required artifacts are absent
- Migration Impact: additive optional field; existing export preview documents default to no expected artifact checks

### `save_load_preview_lab` (`save_load_preview_lab.schema.json`)

- Version: `1.1.0`
- Date: `2026-04-28`
- Summary: runtime/editor behavior now records loaded-vs-expected payload diffs, variables payload matching, and save/load traces without changing required saved fields
- Migration Impact: none; schema remains compatible and existing documents continue to load

### `ability_sandbox` (`ability_sandbox.schema.json`)

- Version: `1.1.0`
- Date: `2026-04-28`
- Summary: adds repeated activation attempt count and interval timing so the WYSIWYG ability sandbox can preview cooldown blocking and cooldown recovery through the runtime component
- Migration Impact: additive optional fields; existing ability sandbox documents default to one activation attempt with no interval

### `event_command_graph` (`event_command_graph.schema.json`)

- Version: `1.1.0`
- Date: `2026-04-28`
- Summary: adds conditional edge metadata for switch and variable conditions so visual event graphs can preview branch traversal and runtime state changes
- Migration Impact: additive optional fields; existing event command graph edges continue to load as sequence edges with no conditions

### `dialogue_preview` (`dialogue_preview.schema.json`)

- Version: `1.1.0`
- Date: `2026-04-28`
- Summary: adds choice target pages, choice command hooks, and choice variable writes for WYSIWYG dialogue preview/runtime trace parity
- Migration Impact: additive optional fields; existing dialogue preview documents continue to load with empty choice targets, commands, and variable writes

### `battle_vfx_timeline` (`battle_vfx_timeline.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: project-data schema for WYSIWYG battle animation/VFX timeline events, including frame timing, cue kind, anchor, participants, intensity, overlay emphasis, labels, and payload metadata
- Migration Impact: none; initial schema for the battle VFX timeline authoring surface

### `template_certification` (`template_certification` report payload)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: Advisory template certification report for minimum loop evidence across JRPG, VN, TBR, tactics, ARPG, monster collector, cozy/life, metroidvania-lite, and 2.5D RPG templates
- Migration Impact: none; additive FFS-17 governance payload and not a release gate

### `project_completeness_score` (`project_completeness_score` advisory payload)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: Non-authoritative project audit completeness score that ignores disabled optional features and keeps missing required evidence advisory-only
- Migration Impact: none; additive FFS-17 project audit advisory

### `feature_governance_manifest` (`governance_manifest.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: Local feature governance manifest with owner, docs, schema/data, and tests pointers for stable feature artifacts
- Migration Impact: none; additive FFS-17 governance fixture contract

### `ui_theme` (`ui_theme.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: UI theme schema for window frames, fonts, cursors, button states, menu sounds, and cross-screen preview contracts
- Migration Impact: none; initial introduction for the FFS-15 capture, theme, and presentation polish slice

### `readiness_status` (`readiness_status.schema.json`)

- Version: `1.0.1`
- Date: `2026-04-28`
- Summary: adds explicit WYSIWYG done-rule evidence fields for runtime execution, visual authoring surface, live preview, and saved project data
- Migration Impact: additive evidence fields; existing readiness entries must populate the new booleans before claiming `READY`

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: initial readiness-governance schema for subsystem and template status records
- Migration Impact: none; initial introduction

### `input_bindings` (`input_bindings.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: input remapping store schema with action-to-key mappings and version validation
- Migration Impact: none; initial introduction

### `controller_bindings` (`controller_bindings.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-21`
- Summary: canonical controller-binding runtime schema for the bounded controller/editor governance contract layered on top of input remapping
- Migration Impact: none; additive schema alias for the Sprint 22 controller-binding governance slice

### `localization_bundle` (`localization_bundle.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: locale catalog bundle schema with language, region, and key-value entries
- Migration Impact: none; initial introduction

### `character_identity` (`character_identity.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: create-a-character identity schema with name, portrait, class, attributes, and appearance tokens
- Migration Impact: none; initial introduction

### `achievements` (`achievements.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: achievement/trophy registry schema with definitions, progress, and unlock conditions
- Migration Impact: none; initial introduction

### `achievement_trophy_export` (`achievement_trophy_export.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-24`
- Summary: vendor-neutral achievement trophy export payload with summary counts, per-trophy progress, and configured/not-configured platform backend integration state
- Migration Impact: none; additive export contract for the existing achievement registry

### `achievement_platform_profile` (`achievement_platform_profile.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: packaged achievement platform profile schema for memory/command backend configuration, package id, and editor-applied platform sync evidence.
- Migration Impact: none; additive platform-profile contract for achievement backend synchronization

### `accessibility_report` (`accessibility_report.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: accessibility audit report schema with issue severity, category, and element snapshots
- Migration Impact: none; initial introduction

### `audio_mix_presets` (`audio_mix_presets.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: audio mix preset bank schema with category volumes, ducking rules, and preset list
- Migration Impact: none; initial introduction

### `export_validation_report` (`export_validation_report.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: platform export validation report schema with target, pass/fail state, and error list
- Migration Impact: none; initial introduction

### `mod_manifest` (`mod_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: mod manifest schema with id, name, version, dependencies, and entry point
- Migration Impact: none; initial introduction

### `analytics_config` (`analytics_config.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: analytics configuration schema with opt-in flag, session id, buffer size, allowed categories, and governed upload endpoint profile/privacy-review fields
- Migration Impact: none; initial introduction

### `analytics_endpoint_profile` (`analytics_endpoint_profile_fixture.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: fixture-backed telemetry endpoint profile contract with local/HTTP modes, headers, curl executable, redacted token surface, and privacy review evidence used by `AnalyticsEndpointProfile`.
- Migration Impact: none; extends analytics endpoint configuration without enabling uploads by default

### `export_config` (`export_config.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: export configuration schema with target platform, output directory, obfuscation, compression, and debug symbols
- Migration Impact: none; initial introduction

### `battle_actions` (`battle_actions.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: battle action contract schema for skill/item execution, scope, cost, and effects
- Migration Impact: none; initial introduction

### `battle_troops` (`battle_troops.schema.json`)

- Version: `1.3.0`
- Date: `2026-04-21`
- Summary: battle troop contract schema for enemy groups, member placement, phase triggers, and grouped condition trees
- Changes: added recursive grouped condition support via `op` + `children`; documented `_compat_condition_fallbacks` for unrepresentable source condition nodes; added `change_switches`, `change_variables`, and `unsupported_command` effect types; retained prior additive condition/effect expansions (`actor_id`, `change_gold`, `change_items`, `change_weapons`, `change_armors`, `transfer_player`, `game_over`)
- Migration Impact: additive only; existing 1.2.0 documents remain valid

### `choice_prompts` (`choice_prompts.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: choice prompt schema with options, default index, and cancel behavior
- Migration Impact: none; initial introduction

### `dialogue_sequences` (`dialogue_sequences.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: dialogue sequence schema with speaker, route, pages, and linked choice prompts
- Migration Impact: none; initial introduction

### `level_block` (`level_block.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: modular level assembly block schema with connectors, prefab references, and thumbnail metadata
- Migration Impact: none; initial introduction

### `menu_commands` (`menu_commands.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: menu command schema with route targets, fallback routes, visibility rules, and enable rules
- Migration Impact: none; initial introduction

### `menu_scene_graph` (`menu_scene_graph.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: menu scene graph schema with registered scenes, pane definitions, and command lists
- Migration Impact: none; initial introduction

### `message_styles` (`message_styles.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: message window style schema with font, color, sound, and positioning rules
- Migration Impact: none; initial introduction

### `pattern_field` (`pattern_field.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: pattern field schema with point arrays, origin validation, and preset references
- Migration Impact: none; initial introduction

### `plugin_manifest` (`plugin_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: RPG Maker MZ plugin manifest schema with name, version, parameters, and dependency list
- Migration Impact: none; initial introduction

### `project` (`project.schema.json`)

- Version: `1.1.0`
- Date: `2026-04-23`
- Summary: top-level project schema with deterministic base contract plus bounded governance sections for localization bundles, input/controller fixtures, and export profiles
- Migration Impact: additive; existing base project documents remain valid because the governance sections are optional at the schema root and enforced conservatively by ProjectAudit only when selected template bars require them

### `rich_text_tokens` (`rich_text_tokens.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: rich text token schema with escape sequences, color codes, and inline effect markers
- Migration Impact: none; initial introduction

### `save_metadata` (`save_metadata.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: save metadata schema with slot summaries, flags, party snapshots, and custom fields
- Migration Impact: none; initial introduction

### `save_migrations` (`save_migrations.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: save migration schema with from/to version ranges and operation arrays
- Migration Impact: none; initial introduction

### `save_policies` (`save_policies.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: save policy schema with autosave settings, retention limits, and pruning behavior
- Migration Impact: none; initial introduction

### `save_slots` (`save_slots.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: save slot descriptor schema with categories, reservation flags, and stable identifiers
- Migration Impact: none; initial introduction

### `sprite_atlas` (`sprite_atlas.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-20`
- Summary: sprite atlas schema with frame metadata, animation clips, and preview loop settings
- Migration Impact: none; initial introduction

### `gameplay_ability` (`gameplay_ability.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: JSON Schema draft-07 for AuthoredAbilityAsset — covers ability_id, cooldown_seconds, mp_cost, effect_id/attribute/operation/value/duration, pattern (embedded patternField), and unsupported_fields fallback container for compat-to-native mapping. S24-T01 schema migration proof.
- Migration Impact: none; initial introduction. unsupported_fields preserves MZ source fields verbatim for round-trip fidelity.

### `events` (`events.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: visual event authoring schema for maps, event pages, triggers, conditions, common events, supported command kinds, and compatibility fallback payloads
- Migration Impact: none; initial introduction for the FFS-03 future-feature event authoring slice

### `battle_presentation` (`battle_presentation.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: battle presentation profile schema for battleback assets, HUD elements, and deterministic cue timeline records
- Migration Impact: none; initial introduction for the FFS-05 battle authoring slice

### `boss_profiles` (`boss_profiles.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: boss profile schema for phase thresholds, summons, enrage flags, dialogue barks, rewards, and music transitions
- Migration Impact: none; initial introduction for the FFS-05 battle authoring slice

### `map_regions` (`map_regions.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: map region rules schema for rectangular regions, encounter tables, ambient audio, weather, hazards, movement rules, and linked events
- Migration Impact: none; initial introduction for the FFS-06 map and worldbuilding slice

### `procedural_map_profiles` (`procedural_map_profiles.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: procedural map profile schema for deterministic seeded dungeon, cave, town, forest, and overworld generation inputs
- Migration Impact: none; initial introduction for the FFS-06 map and worldbuilding slice

### `patch_manifest` (`patch_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: patch manifest schema for base/target versions, changed data, changed assets, and package dependencies
- Migration Impact: none; initial introduction for the FFS-07 export and packaging slice

### `creator_package_manifest` (`creator_package_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: creator package manifest schema for package identity, type, license evidence, compatibility target, dependencies, and validation summary
- Migration Impact: none; initial introduction for the FFS-07 export and packaging slice

### `quest_registry` (`quest_registry.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: quest registry schema for versioned quest registry payloads and quest arrays
- Migration Impact: none; initial introduction for the FFS-10 narrative and quest slice

### `dialogue_graph` (`dialogue_graph.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: dialogue graph schema for versioned dialogue nodes and a deterministic start node
- Migration Impact: none; initial introduction for the FFS-10 narrative and quest slice

### `relationship_registry` (`relationship_registry.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: relationship registry schema for versioned affinity values and relationship gates
- Migration Impact: none; initial introduction for the FFS-10 narrative and quest slice

### `presentation_spatial_map_overlay` (`presentation_schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: JSON Schema draft-07 for SpatialMapOverlay and embedded profile types (elevationGrid, fogProfile, postFXProfile, propInstance, lightProfile, vec3). S23-T03 schema migration proof.
- Migration Impact: none; initial introduction.

### `asset_pipeline_manifest` (`asset_pipeline_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: offline asset-pipeline manifest schema for source path, emitted outputs, tool identity, status, and bounded processing notes.
- Migration Impact: none; initial introduction for tooling-bound asset processing records.

### `audio_pipeline_manifest` (`audio_pipeline_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: offline audio-pipeline manifest schema for source path, emitted outputs, tool identity, status, and prototype-only processing flags.
- Migration Impact: none; initial introduction for tooling-bound audio preprocessing records.

### `retrieval_chunk_manifest` (`retrieval_chunk_manifest.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: retrieval chunk-manifest schema covering source roots, chunk sizing, overlap policy, and deterministic chunk payload records for offline indexing pipelines.
- Migration Impact: none; initial introduction for retrieval artifact governance.

### `retrieval_index_bundle` (`retrieval_index_bundle.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: retrieval index-bundle schema covering embedding engine metadata, adapter contract, source manifest linkage, entry counts, and stored chunk embeddings.
- Migration Impact: none; initial introduction for offline retrieval index artifacts.

### `timeline` (`timeline.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: deterministic timeline authoring schema for actors and ordered message, movement, audio, fade, tint, camera, wait, event-call, battle-cue, and unsupported command records
- Migration Impact: none; initial introduction for the FFS-11 future-feature timeline slice

### `replay` (`replay.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: deterministic replay artifact schema for seed, project version, labels, input log, and per-tick state hashes
- Migration Impact: none; initial introduction for the FFS-11 future-feature replay slice

### `rpg_database` (`rpg_database.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: RPG database schema for actor and item records, class links, prices, tags, and balance-relevant stats
- Migration Impact: none; initial introduction for the FFS-12 future-feature database and balance slice

### `balance_suite` (`balance_suite.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: balance suite schema for economy routes, encounter pools, vendor stock, rest points, loot affixes, class progression, and skill combo records
- Migration Impact: none; initial introduction for the FFS-12 future-feature database and balance slice

### `world_map_graph` (`world_map_graph.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: world-map graph schema for nodes, routes, unlock flags, vehicles, fast travel, and portal-capable travel records
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `crafting_registry` (`crafting_registry.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: crafting registry schema for recipes, ingredient counts, result previews, and unlock conditions
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `bestiary_registry` (`bestiary_registry.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: bestiary registry schema for enemy entries, weaknesses, drops, lore keys, and persisted seen/scanned/defeated state
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `calendar_runtime` (`calendar_runtime.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: calendar runtime schema for day/time windows, event flags, and day-boundary scheduling contracts
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `npc_schedule` (`npc_schedule.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: NPC schedule schema for map, position, animation, dialogue state, hour windows, and fallback behavior
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `puzzle_registry` (`puzzle_registry.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: puzzle registry schema for ordered triggers, reset rules, state, and reward flags
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `runtime_hint_system` (`runtime_hint_system.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: runtime hint schema for once-only hint definitions, localization keys, trigger flags, accessibility settings, and dismissed state
- Migration Impact: none; initial introduction for the FFS-13 simulation and world systems slice

### `ability_orchestration` (`ability_orchestration.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-28`
- Summary: ability orchestration schema for authored battle/map activation documents with source/target actors, pattern-aware target diagnostics, battle queue metadata, and saved editor preview data.
- Migration Impact: none; initial introduction for the gameplay ability authored-orchestration slice.

### `device_profile` (`device_profile.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-25`
- Summary: device profile schema for performance, memory, input, resolution, storage, and export constraint preview records
- Migration Impact: none; initial introduction for the FFS-14 player experience and platform slice

### `tooling_job_run` (`tooling_job_run.schema.json`)

- Version: `1.0.0`
- Date: `2026-04-23`
- Summary: offline tooling job-run schema for job type, status, timestamps, inputs, and outputs across bounded `tools/` pipeline executions.
- Migration Impact: none; initial introduction for restartable tooling-run records.
