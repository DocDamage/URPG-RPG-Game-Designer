# Future Feature Actionable Sprint Plan

Status Date: 2026-04-24

This document converts `docs/FUTURE_FEATURE_UPGRADE_PLANS.md` into an implementation-grade backlog. It is designed for humans or LLM coding agents to execute without rediscovering the project shape each time.

This plan is future scope. It does not promote any subsystem or template to `READY`, does not change `content/readiness/readiness_status.json`, and does not replace the current release-signoff workflow.

## Ground Rules

- Do not start these sprints until the current release gates are intentionally closed or deliberately deferred.
- Every feature must land as a narrow vertical slice: runtime model, editor projection where applicable, schema/fixture when data-driven, diagnostics, tests, docs, and CMake wiring.
- Keep deterministic behavior first. Any generator, replay, timeline, AI-assist, balance simulation, or snapshot feature must be seedable and reproducible.
- Keep live providers out of runtime by default. AI, cloud, marketplace, and remote analytics integrations must stay optional, opt-in, or out-of-tree unless product scope changes.
- Do not mark any new feature `READY` without evidence in code, tests, docs, and project audit/readiness governance.
- When a feature cannot support full RPG Maker parity, preserve unsupported payloads and emit explicit diagnostics instead of silently dropping data.

## Canonical Implementation Pattern

Use this structure unless an existing subsystem already has a stronger local pattern.

| Layer | Preferred Location | Naming Pattern |
|---|---|---|
| Runtime | `engine/core/<feature>/` | `<feature>.h`, `<feature>.cpp`, `<feature>_validator.*`, `<feature>_migration.*` |
| Editor model/panel | `editor/<feature>/` | `<feature>_model.*`, `<feature>_panel.*` |
| Tests | `tests/unit/`, `tests/integration/` | `test_<feature>.cpp`, `test_<feature>_panel.cpp`, `test_<feature>_integration.cpp` |
| Schemas | `content/schemas/` | `<feature>.schema.json` |
| Fixtures | `content/fixtures/` | `<feature>_fixture.json` |
| Governance scripts | `tools/ci/` | `check_<feature>_governance.ps1` |
| Docs | `docs/` or `docs/<feature>/` | `<FEATURE>_*.md`, validation docs |

Add source files to `urpg_core` and tests to `urpg_tests` in `CMakeLists.txt`. Add focused CTest tags to every `TEST_CASE`.

### Shared Definition Of Done

Each sprint is complete only when:

- [ ] Runtime code exists and is wired into `CMakeLists.txt`.
- [ ] Editor-facing model/panel exists when the feature is authorable or inspectable.
- [ ] Data schema and canonical fixture exist when the feature persists authorable content.
- [ ] Unit tests cover happy path, invalid input, empty state, duplicate IDs, unknown references, and deterministic ordering.
- [ ] Integration tests cover at least one real cross-subsystem path when the feature touches save/load, battle, event, message, export, or presentation.
- [ ] Governance script exists for any schema/fixture/artifact lane that must stay in sync.
- [ ] Project audit/readiness docs are updated conservatively, never overclaiming.
- [ ] `powershell -ExecutionPolicy Bypass -File .\tools\ci\check_release_readiness.ps1` passes.
- [ ] `powershell -ExecutionPolicy Bypass -File .\tools\ci\truth_reconciler.ps1` passes.
- [ ] `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` passes or any failure is explicitly unrelated and documented.

## Sprint Overview

| Sprint | Lane | Depends On | Primary Output |
|---|---|---|---|
| FFS-00 | Current Gate Closure Prerequisites | Current release work | Human-review and export-hardening boundary clear |
| FFS-01 | Project Health Dashboard | FFS-00 | Editor-ready audit summary and fix queue |
| FFS-02 | Asset Library And Intake UX | FFS-00 | Asset browser with provenance and cleanup previews |
| FFS-03 | Visual Event Authoring | FFS-01 | Event page editor, dependency graph, breakpoints |
| FFS-04 | Plugin Compatibility Inspector | FFS-01 | Plugin scoring, dependency graph, shim hints |
| FFS-05 | Battle Authoring Suite | FFS-03 | Battle presentation, boss designer, formula debugger |
| FFS-06 | Map And Worldbuilding Suite | FFS-03 | Tilemap, regions, lighting/weather, procedural maps |
| FFS-07 | Export, Patch, And Packaging Hardening | FFS-01 | Runtime bundle validation, release comparison, DLC packaging |
| FFS-08 | Project Onboarding Suite | FFS-01, FFS-07 | New project wizard, templates, dev room, tutorials |
| FFS-09 | Save/Load Debugging Suite | FFS-01 | Save debugger, corruption lab, migration preview, snapshots |
| FFS-10 | Narrative And Quest Suite | FFS-03 | Quests, dialogue graph, continuity, choices, endings |
| FFS-11 | Timeline, Macro, Replay Suite | FFS-03, FFS-10 | Cutscenes, macro recorder, time travel, replay gallery |
| FFS-12 | RPG Database And Balance Suite | FFS-05, FFS-10 | DB parity, economy, encounters, shops, loot, jobs |
| FFS-13 | Simulation And World Systems | FFS-10, FFS-12 | Relationships, calendar, NPC schedules, crafting, codex |
| FFS-14 | Player Experience And Platform Suite | FFS-08 | Localization, accessibility, input remap, device profiles |
| FFS-15 | Capture, Theme, And Presentation Polish | FFS-06, FFS-14 | Trailer capture, photo mode, UI skin builder |
| FFS-16 | Collaboration, SDK, And Optional AI | FFS-01 | Diagnostics bundles, mod SDK, co-author workflow, AI assist |
| FFS-17 | Certification And Governance Integration | All prior slices | Template certification, project completeness scoring, CI guards |

---

## FFS-00 - Current Gate Closure Prerequisites

### Objective
Prevent future feature work from hiding current release blockers.

### Checklist
- [ ] Close `battle_core` human review with explicit human accept/reject decision.
- [ ] Close `save_data_core` human review with explicit human accept/reject decision.
- [ ] Keep `promotionRequiresHumanReview: true` in signoff-gated readiness records unless governance rules are changed deliberately.
- [ ] Finish or deliberately defer runtime-side `data.pck` signature enforcement before broad export claims.
- [ ] Re-run `urpg_project_audit --json` and record release/export blocker counts.
- [ ] Confirm all new future-plan docs are excluded from `READY` status claims.

### Edge Cases
- Human reviewer accepts one lane and blocks the other.
- A reviewer accepts residual gaps but requires follow-up issues before promotion.
- Audit returns `releaseBlockerCount: 0` but template bars remain `PARTIAL`.
- Export validation passes offline but runtime load still accepts tampered bundles.

### Required Commands

```powershell
.\build\dev-ninja-debug\urpg_project_audit.exe --json
powershell -ExecutionPolicy Bypass -File .\tools\ci\check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File .\tools\ci\truth_reconciler.ps1
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
```

---

## FFS-01 - Project Health Dashboard

### 2026-04-24 Slice Status
First vertical slice landed. It builds on the already-existing `ProjectAuditPanel` and adds a separate project-health projection with grouped cards, stale-state detection, a deterministic `fix_next` queue, diagnostics workspace export, and focused tests. This is not a readiness promotion.

### Objective
Make release blockers, export blockers, asset issues, schema drift, and fix guidance visible inside the editor.

### Files To Add Or Modify
- [x] `editor/diagnostics/project_health_model.h`
- [x] `editor/diagnostics/project_health_model.cpp`
- [x] `editor/diagnostics/project_health_panel.h`
- [x] `editor/diagnostics/project_health_panel.cpp`
- [x] `tests/unit/test_project_health_model.cpp`
- [x] `tests/unit/test_project_health_panel.cpp`
- [x] Extend `editor/diagnostics/diagnostics_workspace.*`
- [x] Extend `editor/diagnostics/diagnostics_workspace_export.cpp`
- [x] Extend `CMakeLists.txt`

### Checklist
- [x] Ingest `urpg_project_audit` JSON already accepted by `ProjectAuditPanel`.
- [x] Produce grouped cards: release blockers, export blockers, warnings, governance issues, asset issues, schema issues.
- [x] Add "Fix next" queue ordered by severity, blocker type, and owning subsystem.
- [x] Each fix card includes code, title, detail, affected paths, validation commands, and acceptance criteria.
- [x] Add empty state for no report.
- [x] Add stale-state indicator when report `statusDate` differs from readiness date.
- [x] Add machine-readable snapshot export through diagnostics workspace.

### Edge Cases
- Report is missing optional governance sections.
- Report has unknown severity string.
- Report has counts but no issue array.
- Report has issue array but no counts.
- File paths in issues are absent, relative, or outside repo root.
- Report is huge and contains thousands of asset issues.

### Tests
- [x] `ProjectHealthModel` groups issues deterministically by severity then code.
- [x] Missing counts are derived from issue flags.
- [x] Unknown severity maps to info, not crash.
- [x] Empty report renders no-data snapshot.
- [x] Fix queue preserves exact validation command strings.
- [x] Diagnostics workspace export includes health cards.

### Acceptance Commands

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[editor][diagnostics][project_health]" --reporter compact
.\build\dev-ninja-debug\urpg_project_audit.exe --json
```

---

## FFS-02 - Asset Library And Intake UX

### 2026-04-24 Slice Status
First vertical slice landed. It provides a runtime asset library, editor-facing model/panel snapshot, provenance packet export, safe cleanup preview, report-directory loading, governance script, focused tests, and raw `more assets/` archive intake/indexing. This is not a content readiness or license-clearance promotion.

### Objective
Turn asset hygiene and intake reports into a creator-facing asset browser with cleanup previews and provenance packets.

### Files To Add Or Modify
- [x] `engine/core/assets/asset_library.h`
- [x] `engine/core/assets/asset_library.cpp`
- [x] `engine/core/assets/asset_provenance.h`
- [x] `engine/core/assets/asset_cleanup_planner.h`
- [x] `engine/core/assets/asset_cleanup_planner.cpp`
- [x] `editor/assets/asset_library_model.*`
- [x] `editor/assets/asset_library_panel.*`
- [x] `tests/unit/test_asset_library.cpp`
- [x] `tests/unit/test_asset_cleanup_planner.cpp`
- [x] `tests/unit/test_asset_library_panel.cpp`
- [x] `tools/ci/check_asset_library_governance.ps1`

### Checklist
- [x] Load existing hygiene reports from `imports/reports/`.
- [x] Surface statuses: usable, risky, duplicate, oversized, missing license, missing file, unsupported format.
- [x] Do not delete files directly. Generate cleanup plans only.
- [x] Add safe-delete preview that proves no referenced asset uses the candidate file.
- [x] Export provenance packet with original source, license, normalized path, and export eligibility.
- [x] Add case-sensitivity warnings for paths that differ only by case.

### Edge Cases
- Duplicate files where both are referenced by different fixtures.
- Asset path exists on Windows but would fail on case-sensitive systems.
- Oversized archive is intentionally retained under `imports/`.
- Missing license for a file not included in export.
- Broken symlink or inaccessible file.
- Unsupported extension with valid metadata.

### Tests
- [x] Duplicate groups sort deterministically by canonical path.
- [x] Cleanup plan refuses to delete referenced files.
- [x] Missing license blocks export eligibility but not editor preview.
- [x] Path case collision is detected.
- [x] Provenance packet round-trips JSON.
- [x] Governance script fails when fixture/report is missing.

---

## FFS-03 - Visual Event Authoring

### 2026-04-25 Status
FFS-03 implementation slice is complete. It adds a runtime event authoring document, deterministic active-page resolution, unsupported-command preservation, switch/variable/quest/save-field dependency graph edges, debugger stepping/breakpoints/watch variables, editor model/panel snapshots, semantic validation, a canonical schema/fixture pair, governance artifact check, CMake wiring, focused tests, and a save/load integration test. This is not a readiness promotion and does not claim full RPG Maker event-command parity beyond the supported command categories listed below.

### Objective
Build the authoring spine for RPG logic: visual event pages, command editing, dependency graph, and debugger.

### Files To Add Or Modify
- [x] `engine/core/events/event_document.h`
- [x] `engine/core/events/event_document.cpp`
- [x] `engine/core/events/event_dependency_graph.h`
- [x] `engine/core/events/event_dependency_graph.cpp`
- [x] `engine/core/events/event_debugger.h`
- [x] `engine/core/events/event_debugger.cpp`
- [x] `editor/events/event_authoring_model.*`
- [x] `editor/events/event_authoring_panel.*`
- [x] `content/schemas/events.schema.json`
- [x] `content/fixtures/events_fixture.json`
- [x] `tests/unit/test_event_document.cpp`
- [x] `tests/unit/test_event_dependency_graph.cpp`
- [x] `tests/unit/test_event_debugger.cpp`
- [x] `tests/unit/test_event_authoring_panel.cpp`
- [x] `tests/integration/test_event_authoring_integration.cpp`
- [x] `tools/ci/check_events_governance.ps1`

### Checklist
- [x] Model event pages with conditions, priority, trigger, and commands.
- [x] Support commands for message, switch, variable, transfer, common event, battle, item, gold, wait, fade, sound, and plugin fallback.
- [x] Preserve unsupported commands as `_compat_command_fallbacks`.
- [x] Build dependency graph of reads/writes for switches, variables, quest flags, save fields, and common events.
- [x] Add debugger with breakpoints, current page, command pointer, watch variables, and event stack.
- [x] Add semantic validation: duplicate event IDs, missing target map, missing common event, unreachable page, page shadowing, cyclic common-event calls.

### Edge Cases
- Multiple event pages match; highest-priority page must win deterministically.
- Event command references deleted switch/variable.
- Common event recursion.
- Plugin command is present but plugin is disabled.
- Transfer target map exists but coordinates are out of bounds.
- Event page has no commands but valid conditions.
- Event graph contains thousands of nodes.

### Tests
- [x] Event page resolution is deterministic when conditions overlap.
- [x] Unsupported commands are preserved in fallback payloads.
- [x] Dependency graph reports read/write edges.
- [x] Cyclic common-event calls emit diagnostics and do not recurse forever.
- [x] Debugger can step, break, resume, and expose watch values.
- [x] Empty event document returns empty snapshot without crash.

---

## FFS-04 - Plugin Compatibility Inspector

### 2026-04-25 Status
FFS-04 implementation slice is complete. It adds a runtime compatibility scorer for plugin manifests and plugin-manager failure JSONL, deterministic dependency graph/cycle detection, profile-aware dependency drift handling, conservative compatible/partial/risky/unsupported tiers, denied-permission and unsupported-API issue surfacing, native shim hints only for mapped in-tree features, a headless editor inspector snapshot, machine-readable JSON export, governance artifact checks, CMake wiring, and focused tests. This is not release-authoritative and does not change compatibility promotion policy.

### Objective
Expose plugin compatibility, sandbox permissions, unsupported APIs, load order, and suggested native replacements.

### Files To Add Or Modify
- [x] `engine/core/plugin/plugin_compatibility_score.h`
- [x] `engine/core/plugin/plugin_compatibility_score.cpp`
- [x] `editor/plugin/plugin_inspector_model.*`
- [x] `editor/plugin/plugin_inspector_panel.*`
- [x] `tests/unit/test_plugin_compatibility_score.cpp`
- [x] `tests/unit/test_plugin_inspector_panel.cpp`
- [x] Extend `tools/rpgmaker/validate-plugin-dropins.ps1` if needed. Not needed for this inspector/governance slice.
- [x] `tools/ci/check_plugin_inspector_governance.ps1`

### Checklist
- [x] Ingest existing compat plugin manifests and failure reports.
- [x] Compute conservative score: compatible, partial, risky, unsupported.
- [x] List unsupported JS APIs, missing dependencies, permission denials, fixture-only behavior, and fallback paths.
- [x] Visualize plugin dependency order and cycles through a deterministic inspector projection.
- [x] Provide shim hints only when an in-tree native feature exists.
- [x] Keep score non-release-authoritative until governance integrates it.
- [x] Export machine-readable inspector snapshots for diagnostics and audit handoff.
- [x] Preserve fixture/profile-allowed dependency drift without silently treating it as full compatibility.

### Edge Cases
- Plugin declares dependency that is missing but unused by the active profile.
- Plugin uses dynamic eval or obfuscated command names.
- Plugin load order cycle.
- Plugin requests permission not granted by sandbox.
- Manifest is malformed but plugin file exists.
- Multiple plugins override the same RPG Maker method.

### Tests
- [x] Missing dependency lowers score and surfaces exact dependency ID.
- [x] Permission denial produces blocking issue.
- [x] Cycle detection is deterministic.
- [x] Unknown API appears in unsupported list.
- [x] Suggested native replacement appears only for mapped APIs.
- [x] Existing compat fixture directory loads deterministically.
- [x] Profile-allowed missing dependency is preserved as a graph edge without lowering the dependency score as a hard miss.

---

## FFS-05 - Battle Authoring Suite

### 2026-04-25 Status
FFS-05 implementation slice is complete. It adds battle presentation profiles with battleback/HUD/cue validation, boss profiles with deterministic phase threshold validation, a formula debugger backed by the existing bounded `CombatFormula` contract, deterministic weighted enemy AI profiles, party tactics/auto-battle decisions, editor-facing presentation/boss/formula panel snapshots, canonical schemas and fixture data, governance artifact checks, CMake wiring, and focused tests. This is not a readiness promotion and does not claim a full visual battle editor or live renderer integration beyond these authoring contracts.

### Objective
Make battle authoring visual, inspectable, and designer-friendly.

### Feature Bundle
- Battle presentation authoring.
- Boss fight designer.
- Formula/rule debugger.
- Enemy AI behavior designer.
- Party tactics/auto-battle planner.

### Files To Add Or Modify
- [x] `engine/core/battle/battle_presentation_profile.*`
- [x] `engine/core/battle/boss_profile.*`
- [x] `engine/core/battle/battle_formula_probe.*`
- [x] `engine/core/battle/enemy_ai_profile.*`
- [x] `engine/core/battle/party_tactics_profile.*`
- [x] `editor/battle/battle_presentation_panel.*`
- [x] `editor/battle/boss_designer_panel.*`
- [x] `editor/battle/formula_debugger_panel.*`
- [x] `content/schemas/battle_presentation.schema.json`
- [x] `content/schemas/boss_profiles.schema.json`
- [x] Focused tests under `tests/unit/`.
- [x] `content/fixtures/battle_authoring_fixture.json`
- [x] `tools/ci/check_battle_authoring_governance.ps1`

### Checklist
- [x] Battleback assignment validates missing assets and case-sensitive paths.
- [x] HUD layout supports gauges, state icons, turn order, damage popups, and guard markers.
- [x] Cue timeline supports cast, hit, miss, critical, death, phase transition, victory, defeat, BGM, ME, and camera shake.
- [x] Boss profiles support phases, thresholds, summons, enrage, dialogue barks, rewards, and music transitions.
- [x] Formula debugger uses existing `CombatFormula` bounded contract and explains fallback reasons.
- [x] Enemy AI profiles choose actions deterministically from weighted rules.
- [x] Party tactics produce deterministic auto-battle decisions.

### Edge Cases
- Missing battleback asset.
- Zero or negative HP threshold.
- Overlapping boss phases.
- Formula references unsupported symbol.
- Enemy AI has no legal action.
- Weighted action rules sum to zero.
- Party tactic tries to heal when no healing skill exists.
- Battle reward references missing item.

### Tests
- [x] Missing battleback emits named diagnostic, not silent fallback.
- [x] Boss phase threshold ordering is validated.
- [x] Formula batch probe reports unsupported symbols and malformed expressions.
- [x] Enemy AI selects same action for same state and seed.
- [x] Party tactic heals below threshold and defends when no heal is possible.
- [x] Cue timeline serializes and replays deterministically.

---

## FFS-06 - Map And Worldbuilding Suite

### 2026-04-25 Status
FFS-06 implementation slice is complete. It adds editable tile layer documents, deterministic terrain brush previews, map region rule validation, deterministic procedural map profiles that return normal editable map documents, tactical move range previews, spawn table JSON persistence, a weather profile contract, editor-facing terrain/region/procedural panel snapshots, canonical schemas and fixture data, governance artifact checks, CMake wiring, and focused tests. This is not a readiness promotion and does not claim full tile renderer/editor integration beyond these authoring contracts.

### Objective
Upgrade map creation from basic spatial authoring to full RPG worldbuilding.

### Feature Bundle
- Tilemap/terrain/layer upgrade.
- 2D lighting/weather authoring.
- Map region rules editor.
- Procedural dungeon/map generator.
- Tactical grid/range preview toolkit.
- Spawn/respawn system.

### Files To Add Or Modify
- [x] `engine/core/map/tile_layer_document.*`
- [x] `engine/core/map/terrain_brush.*`
- [x] `engine/core/map/map_region_rules.*`
- [x] `engine/core/map/procedural_map_generator.*`
- [x] `engine/core/map/spawn_table.*`
- [x] `engine/core/map/tactical_grid_preview.*`
- [x] `engine/core/presentation/weather_profile.*`
- [x] `editor/spatial/terrain_brush_panel.*`
- [x] `editor/spatial/region_rules_panel.*`
- [x] `editor/spatial/procedural_map_panel.*`
- [x] `content/schemas/map_regions.schema.json`
- [x] `content/schemas/procedural_map_profiles.schema.json`
- [x] `content/fixtures/map_worldbuilding_fixture.json`
- [x] `tools/ci/check_map_worldbuilding_governance.ps1`

### Checklist
- [x] Add layers with visibility, lock state, collision, navigation, and draw order.
- [x] Add terrain brushes: single, rectangle, line, random weighted, stamp, autotile.
- [x] Add region metadata for encounters, ambient audio, weather, hazards, movement rules, and events.
- [x] Add deterministic map generation profiles: dungeon, cave, town, forest, overworld.
- [x] Generated maps must be normal editable map documents, not special runtime-only data.
- [x] Add tactical overlay for move range, attack range, blocked cells, and line of sight.
- [x] Add spawn/respawn tables with cooldowns, persistence, and save/load rules.

### Edge Cases
- Layer locked but command tries to edit.
- Autotile references missing neighbor tiles.
- Region overlaps with conflicting movement rules.
- Procedural generator cannot satisfy required boss/key/shop constraints.
- Spawn table references missing enemy.
- Navigation generated for impassable cells.
- Large maps should avoid O(width * height * layers) work every frame.

### Tests
- [x] Terrain brush output is deterministic for the same seed.
- [x] Locked layer rejects edits.
- [x] Region conflict validator emits exact cell/rule conflict.
- [x] Procedural generator reports unsatisfied constraints.
- [x] Tactical range preview respects blocked cells.
- [x] Spawn persistence round-trips through save JSON.

---

## FFS-07 - Export, Patch, And Packaging Hardening

### Objective
Make shipping artifacts safer, comparable, and suitable for updates.

### Feature Bundle
- Runtime-side `data.pck` signature enforcement.
- Release candidate comparison.
- Patch/DLC builder.
- Creator marketplace-ready packaging.

### 2026-04-25 Status
- Landed runtime signed bundle loading and temp-file publication helpers.
- Added release artifact diffing, patch manifests, creator package manifests, schemas, and fixtures.
- Extended `urpg_pack_cli` with an export-hardening explanation report.
- Added FFS-07 unit coverage plus a governance check for registration drift.

### Files To Add Or Modify
- [x] `engine/core/export/runtime_bundle_loader.*`
- [x] `engine/core/export/export_artifact_compare.*`
- [x] `engine/core/export/patch_manifest.*`
- [x] `engine/core/export/creator_package_manifest.*`
- [x] `tools/pack/` pack CLI extensions.
- [x] `tests/unit/test_runtime_bundle_loader.cpp`
- [x] `tests/unit/test_export_artifact_compare.cpp`
- [x] `tests/unit/test_patch_manifest.cpp`
- [x] `tests/unit/test_creator_package_manifest.cpp`
- [x] `content/schemas/patch_manifest.schema.json`
- [x] `content/schemas/creator_package_manifest.schema.json`
- [x] `content/fixtures/export_packaging_fixture.json`
- [x] `tools/ci/check_export_packaging_governance.ps1`

### Checklist
- [x] Runtime loader rejects tampered bundles before loading content.
- [x] Bundle publication uses temp file plus atomic rename where platform supports it.
- [x] Comparison reports changed assets, changed schemas, missing files, signature status, and manifest differences.
- [x] Patch builder detects changed data/assets and validates compatibility.
- [x] Creator package manifest includes type, license evidence, compatibility target, dependencies, and validation summary.

### Edge Cases
- Signature missing.
- Signature present but wrong key.
- Bundle truncated after manifest.
- Temp file exists from interrupted export.
- Patch removes a schema required by old saves.
- DLC package depends on missing base content.

### Tests
- [x] Tampered bundle fails runtime load.
- [x] Valid signed bundle loads.
- [x] Interrupted atomic publish leaves previous bundle intact.
- [x] Artifact compare detects changed schema version.
- [x] Patch manifest rejects missing dependency.
- [x] Creator package rejects absent license evidence.

---

## FFS-08 - Project Onboarding Suite

### Objective
Help creators get from blank project to working, validated RPG slice quickly.

### Feature Bundle
- Guided new project wizard.
- Starter project templates.
- One-click dev room test harness.
- Tutorial project and interactive lessons.

### 2026-04-25 Status
- Landed deterministic starter project generation for JRPG, visual novel, and turn-based RPG templates.
- Added new project wizard model/panel snapshots, dev room generation, tutorial lesson progress persistence, starter template content, and dev room fixture coverage.
- Added FFS-08 unit coverage plus a governance check for build/doc registration drift.

### Files To Add Or Modify
- [x] `engine/core/project/project_template_generator.*`
- [x] `engine/core/project/dev_room_generator.*`
- [x] `engine/core/tutorial/tutorial_lesson.*`
- [x] `editor/project/new_project_wizard_model.*`
- [x] `editor/project/new_project_wizard_panel.*`
- [x] `content/templates/`
- [x] `content/fixtures/dev_room_fixture.json`
- [x] `tests/unit/test_project_template_generator.cpp`
- [x] `tests/unit/test_dev_room_generator.cpp`
- [x] `tests/unit/test_tutorial_lesson.cpp`
- [x] `tools/ci/check_project_onboarding_governance.ps1`

### Checklist
- [x] Generate project from template with maps, menu, message, battle, save, localization, input, and export profile.
- [x] Initial audit report is generated after project creation.
- [x] Dev room includes stations for message, menu, battle, save/load, plugin report, audio, input, asset warning, export preflight.
- [x] Interactive lessons track completion and can be reset.
- [x] Generated content has stable IDs and deterministic ordering.

### Edge Cases
- Template references missing subsystem or asset.
- User cancels halfway through project creation.
- Generated project path already exists.
- Invalid project name/path characters.
- Dev room station fails but report still completes.

### Tests
- [x] Template generator emits valid project JSON for JRPG/VN/TBR.
- [x] Duplicate project IDs are rejected.
- [x] Dev room scripted route visits all stations.
- [x] Tutorial completion persists through save/load.
- [x] Generated project passes project schema validation.

---

## FFS-09 - Save/Load Debugging Suite

### Objective
Make persistence visible, testable, and safe across project updates.

### Feature Bundle
- Save/load debugger.
- Corruption lab and recovery simulation.
- Save compatibility/migration previewer.
- Cloud-free backup/project snapshots.

### 2026-04-25 Status
- Landed save debugger inspection for slot metadata, recovery tiers, migration notes, subsystem state, and diagnostics.
- Added a corruption lab that writes mutated test copies, recovery simulation reporting, compatibility migration previews, and local project snapshot/restore support.
- Added editor panel snapshots, a save debugging fixture, FFS-09 unit coverage, and a governance check for registration drift.

### Files To Add Or Modify
- [x] `engine/core/save/save_debugger.*`
- [x] `engine/core/save/save_corruption_lab.*`
- [x] `engine/core/save/save_compatibility_preview.*`
- [x] `engine/core/project/project_snapshot_store.*`
- [x] `editor/save/save_debugger_panel.*`
- [x] `editor/save/save_migration_preview_panel.*`
- [x] `tests/unit/test_save_debugger.cpp`
- [x] `tests/unit/test_save_corruption_lab.cpp`
- [x] `tests/unit/test_save_compatibility_preview.cpp`
- [x] `tests/unit/test_project_snapshot_store.cpp`
- [x] `content/fixtures/save_debugging_fixture.json`
- [x] `tools/ci/check_save_debugging_governance.ps1`

### Checklist
- [x] Inspect slots, metadata, recovery tier, migration notes, and subsystem state.
- [x] Intentionally corrupt test saves without touching real user saves.
- [x] Run recovery tiers and export diagnostics.
- [x] Preview old save fixtures through current migration stack.
- [x] Create local project snapshots before migrations and risky edits.

### Edge Cases
- Corrupted JSON.
- Valid JSON with wrong schema version.
- Missing primary but valid autosave.
- Unknown subsystem blob in save payload.
- Snapshot restore target path exists.
- Snapshot references deleted assets.

### Tests
- [x] Corruption lab never mutates original fixture.
- [x] Recovery simulation reports tier chosen.
- [x] Migration preview preserves unrelated blobs.
- [x] Snapshot restore round-trips project files.
- [x] Unknown save fields remain preserved or explicitly diagnosed.

---

## FFS-10 - Narrative And Quest Suite

### Objective
Build first-class RPG narrative systems with validation and authoring support.

### Feature Bundle
- Quest/objective system.
- Dialogue graph editor.
- Narrative continuity checker.
- Player choice consequence tracker.
- Ending/route manager.
- Relationship/reputation system.
- Reputation-gated content browser.

### 2026-04-25 Status
- Landed deterministic quest state, quest validation, dialogue graph, narrative continuity, choice consequence, ending route, and relationship/reputation helpers.
- Added editor panel snapshots for quest, dialogue, narrative continuity, and relationship views.
- Added narrative schemas, fixture content, FFS-10 unit coverage, and a governance check for build/doc registration drift.

### Files To Add Or Modify
- [x] `engine/core/quest/quest_registry.*`
- [x] `engine/core/quest/quest_validator.*`
- [x] `engine/core/dialogue/dialogue_graph.*`
- [x] `engine/core/narrative/narrative_continuity_checker.*`
- [x] `engine/core/narrative/choice_consequence_tracker.*`
- [x] `engine/core/narrative/ending_route_manager.*`
- [x] `engine/core/relationship/relationship_registry.*`
- [x] Editor panels under `editor/quest/`, `editor/dialogue/`, `editor/narrative/`, `editor/relationship/`.
- [x] Schemas and fixtures for each data-driven feature.
- [x] `tools/ci/check_narrative_quest_governance.ps1`

### Checklist
- [x] Quest registry supports objective states: locked, active, completed, failed, hidden.
- [x] Quest objectives can depend on switches, variables, items, battles, dialogue choices, and reputation.
- [x] Dialogue graph supports nodes, choices, conditions, effects, speaker metadata, localization keys, and preview.
- [x] Continuity checker reports unreachable dialogue, orphaned nodes, impossible conditions, unresolved choices, and missing endings.
- [x] Choice consequence tracker links choices to state changes.
- [x] Ending manager evaluates endings deterministically by priority.
- [x] Relationship registry persists faction/NPC affinity and reputation.

### Edge Cases
- Quest objective references deleted item.
- Quest has no reachable start.
- Choice loops forever without terminal condition.
- Multiple endings match; priority resolves deterministically.
- Relationship value overflows configured bounds.
- Dialogue localization key missing.
- Condition references unknown switch.

### Tests
- [x] Quest objective advancement is deterministic.
- [x] Quest save/load preserves objective state and timestamps.
- [x] Dialogue graph detects orphaned nodes.
- [x] Continuity checker flags contradictory conditions.
- [x] Ending manager chooses highest-priority eligible ending.
- [x] Reputation-gated browser lists content by state.

---

## FFS-11 - Timeline, Macro, Replay Suite

### 2026-04-25 Status
FFS-11 implementation slice is complete. It adds deterministic timeline documents/playback, event macro draft conversion, replay artifacts with seed/input/state-hash metadata, divergent replay comparison, bounded checkpoint restore, a replay gallery query, headless editor panel snapshots, canonical timeline/replay schemas and fixture data, a golden replay CI lane that compares state hashes and fails on divergence, governance artifact checks, CMake wiring, focused tests, and a save/load integration path. This is not a full visual cutscene editor or live in-editor playtest loop beyond the deterministic authoring/replay contracts listed here.

### Objective
Make scripted scenes, repros, and deterministic replay authorable and testable.

### Feature Bundle
- Cutscene/timeline sequencer.
- Event macro recorder.
- In-editor playtest with time travel.
- Deterministic replay gallery.
- Golden replay CI lane.

### Files To Add Or Modify
- [x] `engine/core/timeline/timeline_document.*`
- [x] `engine/core/timeline/timeline_player.*`
- [x] `engine/core/events/event_macro_recorder.*`
- [x] `engine/core/replay/replay_recorder.*`
- [x] `engine/core/replay/replay_player.*`
- [x] `engine/core/replay/replay_gallery.*`
- [x] Editor panels under `editor/timeline/` and `editor/replay/`.
- [x] `tests/unit/test_timeline_player.cpp`
- [x] `tests/unit/test_event_macro_recorder.cpp`
- [x] `tests/unit/test_replay_recorder.cpp`
- [x] `tests/integration/test_replay_integration.cpp`
- [x] `tools/ci/check_golden_replays.ps1`

### Checklist
- [x] Timeline tracks support message, movement, audio, fade, tint, camera, wait, event call, and battle cue.
- [x] Macro recorder converts playtest actions into draft event/timeline commands.
- [x] Replay artifacts include seed, input log, state hashes, project version, and labels.
- [x] Time travel uses bounded checkpoints and deterministic replay from checkpoint.
- [x] Golden replay CI lane compares state hashes and fails on divergence.

### Edge Cases
- Timeline references missing actor.
- Wait duration is zero or negative.
- Replay generated on old project version.
- Checkpoint memory limit reached.
- Input log contains unknown action.
- Event macro includes unsupported command.

### Tests
- [x] Timeline playback produces identical command stream for same seed.
- [x] Macro recorder emits editable draft commands.
- [x] Replay state hash is stable.
- [x] Divergent replay reports first mismatched tick.
- [x] Time travel restores previous state without leaking later mutations.

---

## FFS-12 - RPG Database And Balance Suite

**Status (2026-04-25): Complete.**
FFS-12 implementation slice is complete. It adds deterministic RPG database records with CSV import/export, duplicate/malformed-row validation, seeded autofill from explicit balance profiles, economy route simulation, encounter weights and preview metrics, vendor stock gating, rest point recovery costs, seeded loot affixes, class progression graph validation, skill combo matching, headless editor panel snapshots, canonical database/balance schemas and fixture data, governance artifact checks, CMake wiring, and focused tests. This is not a full visual database editor surface beyond the deterministic runtime/editor snapshot contracts listed here.

### Objective
Make core RPG databases fast to author, validate, simulate, and balance.

### Feature Bundle
- Database editor parity pass.
- Smart autofill database tools.
- Economy balancer.
- Encounter table editor.
- Shop/vendor designer.
- Inn/rest/recovery system.
- Loot affix/rarity generator.
- Job/class progression designer.
- Skill combo/synergy system.

### Files To Add Or Modify
- [x] `engine/core/database/rpg_database.*`
- [x] `engine/core/balance/economy_simulator.*`
- [x] `engine/core/balance/encounter_table.*`
- [x] `engine/core/shop/vendor_catalog.*`
- [x] `engine/core/rest/rest_point.*`
- [x] `engine/core/items/loot_affix_generator.*`
- [x] `engine/core/progression/class_progression.*`
- [x] `engine/core/ability/skill_combo_rules.*`
- [x] Editor panels under `editor/database/`, `editor/balance/`, `editor/shop/`.
- [x] Schemas and fixtures under `content/schemas/` and `content/fixtures/`.
- [x] Governance check under `tools/ci/`.
- [x] Focused tests under `tests/unit/`.

### Checklist
- [x] Bulk edit actors, classes, skills, items, enemies, troops, states, equipment, animations, formulas.
- [x] CSV import/export is deterministic and schema-validated.
- [x] Autofill generates draft stats/prices/XP/skills from explicit design profile.
- [x] Economy simulator runs scripted routes and reports gold, XP, inventory, affordability, and resource drain.
- [x] Encounter editor supports weights, regions, conditions, preview simulation, and difficulty charts.
- [x] Vendors support stock, refresh conditions, buy/sell modifiers, and progression gates.
- [x] Loot affix generator uses seeded rolls and validates economy impact.
- [x] Job/class graph validates prerequisites, learned skills, and stat curves.

### Edge Cases
- Duplicate database IDs.
- CSV missing required column.
- Generated values exceed configured caps.
- Encounter weights sum to zero.
- Vendor item missing from database.
- Affix pool empty for rarity tier.
- Class graph cycle.
- Combo rule references deleted skill.

### Tests
- [x] CSV round-trip preserves IDs and values.
- [x] Autofill is deterministic for same profile and seed.
- [x] Economy route flags unaffordable required item.
- [x] Encounter table rejects zero-weight pool.
- [x] Vendor stock refresh respects conditions.
- [x] Loot affix roll stays within bounds.
- [x] Class graph cycle is detected.
- [x] Skill combo triggers only under matching tags.

---

## FFS-13 - Simulation And World Systems

**Status (2026-04-25): Complete.**
FFS-13 implementation slice is complete. It adds deterministic world-map routing with unlock flags and vehicles, crafting recipes with preview and validation, bestiary state persistence, calendar event windows including day-boundary handling, NPC schedule fallback resolution, ordered puzzle triggers with reset behavior, runtime hint dismissal persistence, headless editor panel snapshots, canonical schemas and fixture data, a simulation governance check, CMake wiring, focused tests, and a cross-system integration path. This is not a full visual authoring workspace or broad spawn/respawn expansion beyond the runtime/editor snapshot contracts listed here.

### Objective
Add durable RPG systems that make worlds richer and support more genres.

### Feature Bundle
- World map/travel system.
- Crafting/recipe system.
- Bestiary/codex system.
- Calendar/time-of-day system.
- NPC schedule/routine designer.
- Puzzle/lock-key system.
- Runtime tutorial/hint system.
- Spawn/respawn system if not completed in FFS-06.

### Files To Add Or Modify
- [x] `engine/core/world/world_map_graph.*`
- [x] `engine/core/crafting/crafting_registry.*`
- [x] `engine/core/codex/bestiary_registry.*`
- [x] `engine/core/time/calendar_runtime.*`
- [x] `engine/core/npc/npc_schedule.*`
- [x] `engine/core/puzzle/puzzle_registry.*`
- [x] `engine/core/tutorial/runtime_hint_system.*`
- [x] Editor panels under `editor/world/`, `editor/crafting/`, `editor/codex/`, `editor/time/`, `editor/npc/`, and `editor/puzzle/`.
- [x] Schemas and fixtures for each persisted registry.
- [x] Focused tests under `tests/unit/` and save/load integration coverage under `tests/integration/`.

### Checklist
- [x] World map graph supports nodes, routes, unlocks, fast travel, vehicles, portals, and save persistence.
- [x] Crafting supports recipes, ingredients, unlock conditions, result preview, and economy validation.
- [x] Bestiary supports seen/scanned/defeated states, drops, weaknesses, lore unlocks, and completion tracking.
- [x] Calendar supports day/time blocks, schedules, events, lighting hooks, and save persistence.
- [x] NPC routines support map, position, animation, dialogue state, and fallback behavior.
- [x] Puzzle assets support locks, keys, ordered triggers, reset rules, and rewards.
- [x] Runtime hints support once-only flags, dismissal state, localization keys, and accessibility settings.

### Edge Cases
- Travel route unlocks itself.
- Recipe consumes item that is also result.
- Bestiary entry references missing enemy.
- Calendar event crosses day boundary.
- NPC schedule map missing.
- Puzzle reset conflicts with save state.
- Hint loops every frame because dismissal flag fails.

### Tests
- [x] World route availability changes with flags.
- [x] Crafting validates missing ingredients.
- [x] Bestiary state persists.
- [x] Calendar event fires exactly once per scheduled window.
- [x] NPC routine fallback activates when map is missing.
- [x] Puzzle ordered trigger rejects wrong sequence and can reset.
- [x] Hint dismissal persists.

### Acceptance Commands

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[world],[crafting],[codex],[calendar],[npc],[puzzle],[tutorial]" --reporter compact
.\build\dev-ninja-debug\urpg_integration_tests.exe "[integration][simulation]" --reporter compact
```

---

## FFS-14 - Player Experience And Platform Suite

**Status (2026-04-25): Complete.**
FFS-14 implementation slice is complete. It adds a localization workspace model/panel for source-target key sync, fallback resolution, glossary checks, and layout-limit snapshots; accessibility fix advising and an assistant panel; input remap profiles with conflict detection and explicit accessibility duplicates; device profile budget evaluation and panel snapshots; canonical device profile schema and fixtures; a player-experience governance check; CMake wiring; and focused tests. This is not a full visual localization, accessibility, input, or device-preview authoring UI beyond the runtime/editor snapshot contracts listed here.

### Objective
Make projects more accessible, localizable, input-friendly, and platform-aware.

### Feature Bundle
- Localization workspace.
- Accessibility authoring assistant.
- Controller + keyboard remap UX.
- Device/platform preview profiles.

### Files To Add Or Modify
- [x] `editor/localization/localization_workspace_model.*`
- [x] `editor/localization/localization_workspace_panel.*`
- [x] `engine/core/accessibility/accessibility_fix_advisor.*`
- [x] `editor/accessibility/accessibility_assistant_panel.*`
- [x] `engine/core/input/input_remap_profile.*`
- [x] `editor/input/input_remap_panel.*`
- [x] `engine/core/platform/device_profile.*`
- [x] `editor/platform/device_profile_panel.*`
- [x] `content/schemas/device_profile.schema.json`
- [x] `content/fixtures/device_profile_fixture.json`
- [x] Focused tests under `tests/unit/`.
- [x] Governance scripts for localization/accessibility/input/device fixtures when new artifacts become canonical.

### Checklist
- [x] Localization panel syncs string IDs, missing keys, glossary terms, translation memory, and layout snapshots.
- [x] Accessibility assistant suggests fixes for labels, contrast, focus order, text speed, input alternatives, and controller-only navigation.
- [x] Input remap screen supports conflicts, defaults, profiles, keyboard, controller, and accessibility alternatives.
- [x] Device profiles report performance, memory, input, resolution, storage, and export constraints.

### Edge Cases
- Missing translation but fallback locale exists.
- Glossary term translated inconsistently.
- Text overflows in one locale only.
- Controller disconnected mid-remap.
- Duplicate binding is intentional for accessibility.
- Low-end profile cannot support selected resolution.

### Tests
- [x] Localization workspace flags missing key and respects fallback.
- [x] Glossary enforcement reports inconsistent term.
- [x] Accessibility preview emits focus-order issue.
- [x] Remap rejects accidental conflict unless explicitly allowed.
- [x] Device profile flags over-budget frame time/memory.

### Acceptance Commands

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[localization],[accessibility],[input],[platform]" --reporter compact
powershell -ExecutionPolicy Bypass -File .\tools\ci\check_localization_consistency.ps1
```

---

## FFS-15 - Capture, Theme, And Presentation Polish

**Status (2026-04-25): Complete.** Landed bounded runtime/editor contracts for deterministic capture naming, trailer frame manifests, photo-mode state isolation, theme validation, and stable UI preview snapshots. This slice intentionally covers headless-safe contracts and editor snapshot surfaces; full renderer-backed visual authoring remains governed by the existing presentation gate.

### Objective
Help creators present, brand, and market their games.

### Feature Bundle
- In-editor screenshot/trailer capture.
- Photo/diorama mode.
- Theme/UI skin builder.

### Files To Add Or Modify
- [x] `engine/core/capture/capture_session.*`
- [x] `engine/core/capture/trailer_capture_manifest.*`
- [x] `engine/core/presentation/photo_mode_state.*`
- [x] `engine/core/ui/theme_registry.*`
- [x] `engine/core/ui/theme_validator.*`
- [x] `editor/capture/capture_panel.*`
- [x] `editor/presentation/photo_mode_panel.*`
- [x] `editor/ui/theme_builder_panel.*`
- [x] `content/schemas/ui_theme.schema.json`
- [x] `content/fixtures/ui_theme_fixture.json`
- [x] Focused tests under `tests/unit/` plus renderer-backed snapshot coverage where visuals are claimed.

### Checklist
- [x] Screenshot capture uses deterministic scene state and stable output naming.
- [x] Trailer capture supports short clips, GIF-like frame sequence, thumbnails, and store-page media presets.
- [x] Photo mode supports pause, camera movement, UI hide, time/weather override, character pose presets, and screenshot export.
- [x] UI skin builder supports window frames, fonts, cursors, button states, menu sounds, and cross-screen previews.

### Edge Cases
- Capture requested in headless CI.
- Output path unwritable.
- Missing font in selected theme.
- Skin references missing sound.
- Photo mode tries to pose actor not present.

### Tests
- [x] Capture returns explicit unsupported status in headless mode.
- [x] Theme validator catches missing font and missing sound.
- [x] UI skin preview renders all core screens into a snapshot.
- [x] Photo mode state does not leak back into gameplay state after exit.

### Acceptance Commands

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[capture],[photo_mode],[theme]" --reporter compact
powershell -ExecutionPolicy Bypass -File .\tools\ci\run_presentation_gate.ps1 -SkipBuild
```

---

## FFS-16 - Collaboration, SDK, And Optional AI

### Objective
Support teams and extension authors without adding hidden live-service dependencies.

### Feature Bundle
- Crash/diagnostics bundle export.
- Mod SDK documentation + sample mod.
- Local co-author review workflow.
- Local AI design assistant.

### Files To Add Or Modify
- [ ] `engine/core/diagnostics/diagnostics_bundle_exporter.*`
- [ ] `engine/core/mod/mod_sdk_sample_validator.*`
- [ ] `engine/core/collaboration/local_review_bundle.*`
- [ ] `engine/core/ai/ai_assistant_config.*`
- [ ] `engine/core/ai/ai_suggestion_record.*`
- [ ] `editor/diagnostics/diagnostics_bundle_panel.*`
- [ ] `editor/mod/mod_sdk_panel.*`
- [ ] `editor/collaboration/local_review_panel.*`
- [ ] `editor/ai/ai_assistant_panel.*`
- [ ] `docs/modding/`
- [ ] `content/fixtures/mod_sdk_sample/`
- [ ] Focused tests under `tests/unit/`.

### Checklist
- [ ] Diagnostics bundle includes logs, project audit, asset report, config, save metadata, system info, and recent diagnostics snapshots.
- [ ] Mod SDK sample includes manifest, permissions, validation docs, and expected diagnostics.
- [ ] Co-author review produces local change summaries, comments, review checklists, and handoff bundles.
- [ ] AI assistant is opt-in, provider-independent, and never required for runtime.
- [ ] AI suggestions have review state, provenance/source notes, and generated-content flags.

### Edge Cases
- Diagnostics bundle contains secrets or ignored `.env` files.
- Mod sample requests forbidden permission.
- Git unavailable for local co-author workflow.
- AI provider unavailable.
- AI suggestion tries to modify runtime status docs.

### Tests
- [ ] Diagnostics bundle excludes ignored secret files.
- [ ] Mod sample passes validation and forbidden-permission sample fails.
- [ ] Co-author summary works without Git by falling back to file manifest.
- [ ] AI assistant disabled state is explicit and non-error.
- [ ] Generated suggestions require approval before applying.

### Acceptance Commands

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[diagnostics_bundle],[mod_sdk],[collaboration],[ai_assistant]" --reporter compact
powershell -ExecutionPolicy Bypass -File .\tools\rpgmaker\validate-plugin-dropins.ps1
```

---

## FFS-17 - Certification And Governance Integration

### Objective
Make the future feature set governable and shippable rather than just large.

### Files To Add Or Modify
- [ ] `tools/ci/check_template_certification.ps1`
- [ ] `tools/ci/check_feature_governance.ps1`
- [ ] `tools/docs/check_future_feature_docs.ps1`
- [ ] `tools/audit/project_completeness_score.*`
- [ ] `engine/core/project/template_certification.*`
- [ ] `content/fixtures/template_certification/`
- [ ] `docs/certification/`
- [ ] Extend `tools/audit/project_audit.*` only after advisory scoring is stable.
- [ ] Focused tests under `tests/unit/` and governance-script negative fixtures.

### Checklist
- [ ] Add template certification suites for JRPG, VN, TBR, tactics, ARPG, monster collector, cozy/life, metroidvania-lite, and 2.5D RPG where appropriate.
- [ ] Add content completeness score to project audit as a non-authoritative advisory first.
- [ ] Promote advisory score to release gate only after false positives are understood.
- [ ] Add per-feature governance scripts only when artifacts are stable.
- [ ] Add docs explaining supported scope, unsupported scope, and residual gaps.
- [ ] Add schema changelog entries for every new schema.
- [ ] Keep readiness matrix and template matrix conservative.

### Edge Cases
- Completeness score penalizes intentionally minimalist projects.
- Template certification assumes assets that are not license-cleared.
- Optional features accidentally become hard requirements.
- Governance script fails on generated local files.

### Tests
- [ ] Completeness scoring ignores explicitly disabled optional features.
- [ ] Template certification fails when required template loop is missing.
- [ ] Governance script positive and negative fixtures are both covered.
- [ ] Truth reconciler passes after docs are updated.

### Acceptance Commands

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[certification],[project_audit]" --reporter compact
powershell -ExecutionPolicy Bypass -File .\tools\ci\check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File .\tools\ci\truth_reconciler.ps1
```

---

## Cross-Cutting Edge Case Checklist

Run this list for every new runtime/editor feature.

- [ ] Empty input document.
- [ ] Malformed JSON.
- [ ] Missing required field.
- [ ] Unknown enum value.
- [ ] Duplicate ID.
- [ ] Unknown referenced ID.
- [ ] Orphaned record.
- [ ] Cycle in graph data.
- [ ] Unsupported compat payload.
- [ ] Large input set.
- [ ] Deterministic ordering under equal priority.
- [ ] Save/load round-trip.
- [ ] Schema version mismatch.
- [ ] Missing asset path.
- [ ] Case-sensitive path mismatch.
- [ ] Headless CI behavior.
- [ ] Permission denied for file write.
- [ ] Interrupted write/export.
- [ ] Disabled optional feature.
- [ ] Local-only/offline mode.
- [ ] Stale generated report.

## Test Strategy

Use focused tests first, then broaden.

| Test Type | Purpose | Example Command |
|---|---|---|
| Unit | Runtime model, validators, panels, edge cases | `.\build\dev-ninja-debug\urpg_tests.exe "[quest]" --reporter compact` |
| Integration | Cross-system flows like quest + save + event | `.\build\dev-ninja-debug\urpg_integration_tests.exe "[quest][save]" --reporter compact` |
| Snapshot | Visual/capture/theme regressions | `ctest --test-dir build/dev-ninja-debug -L nightly --output-on-failure` |
| Governance | Schema/docs/artifact parity | `powershell -ExecutionPolicy Bypass -File .\tools\ci\check_<feature>_governance.ps1` |
| PR Lane | Broad sanity | `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` |

## Copy-Ready Code Patterns

These are intentionally small, concrete examples that match the repo's current C++20, Catch2, and nlohmann/json style.

### CMake Wiring Pattern

```cmake
# Add runtime/editor sources to urpg_core.
engine/core/quest/quest_registry.cpp
engine/core/quest/quest_validator.cpp
editor/quest/quest_panel.cpp

# Add focused tests to urpg_tests.
tests/unit/test_quest_registry.cpp
tests/unit/test_quest_panel.cpp
```

### Runtime Header Pattern

```cpp
#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::quest {

enum class QuestObjectiveState {
    Locked,
    Active,
    Completed,
    Failed,
    Hidden
};

struct QuestObjective {
    std::string id;
    std::string title;
    QuestObjectiveState state = QuestObjectiveState::Locked;
    std::vector<std::string> requiredFlags;
};

struct QuestDef {
    std::string id;
    std::string title;
    std::vector<QuestObjective> objectives;
};

struct QuestDiagnostic {
    std::string code;
    std::string detail;
    bool blocksRelease = false;
};

class QuestRegistry {
public:
    bool registerQuest(const QuestDef& quest);
    bool activateObjective(const std::string& questId, const std::string& objectiveId);
    bool completeObjective(const std::string& questId, const std::string& objectiveId);
    std::optional<QuestDef> getQuest(const std::string& questId) const;
    std::vector<QuestDiagnostic> validate() const;
    nlohmann::json saveToJson() const;
    void loadFromJson(const nlohmann::json& value);

private:
    std::unordered_map<std::string, QuestDef> quests_;
};

} // namespace urpg::quest
```

### Runtime Unit Test Pattern

```cpp
#include "engine/core/quest/quest_registry.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::quest;

TEST_CASE("QuestRegistry advances objectives deterministically", "[quest][runtime]") {
    QuestRegistry registry;

    QuestDef quest;
    quest.id = "quest_intro";
    quest.title = "Find the Gate";
    quest.objectives.push_back({"obj_find_key", "Find the key", QuestObjectiveState::Locked, {}});

    REQUIRE(registry.registerQuest(quest));
    REQUIRE(registry.activateObjective("quest_intro", "obj_find_key"));
    REQUIRE(registry.completeObjective("quest_intro", "obj_find_key"));

    const auto loaded = registry.getQuest("quest_intro");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->objectives.size() == 1);
    REQUIRE(loaded->objectives[0].state == QuestObjectiveState::Completed);
}

TEST_CASE("QuestRegistry reports duplicate quest ids", "[quest][validation]") {
    QuestRegistry registry;

    QuestDef first;
    first.id = "quest_intro";
    first.title = "Intro";

    QuestDef duplicate = first;
    duplicate.title = "Duplicate";

    REQUIRE(registry.registerQuest(first));
    REQUIRE_FALSE(registry.registerQuest(duplicate));

    const auto diagnostics = registry.validate();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics[0].code == "quest.duplicate_id");
}
```

### Editor Panel Snapshot Test Pattern

```cpp
#include "editor/quest/quest_panel.h"
#include "engine/core/quest/quest_registry.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("QuestPanel snapshot reflects active quest state", "[quest][editor][panel]") {
    urpg::quest::QuestRegistry registry;
    registry.registerQuest({"quest_intro", "Find the Gate", {}});

    urpg::editor::QuestPanel panel;
    panel.bindRegistry(&registry);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.quest_count == 1);
    REQUIRE(snapshot.rows[0].id == "quest_intro");
}
```

### Dependency Graph Test Pattern

```cpp
#include "engine/core/events/event_dependency_graph.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("EventDependencyGraph reports switch reads and writes", "[event][graph]") {
    urpg::events::EventDocument doc;
    doc.events.push_back(urpg::events::EventDef{
        .id = "ev_gate",
        .pages = {
            urpg::events::EventPage{
                .id = "page_locked",
                .conditions = {urpg::events::EventCondition::SwitchOn("sw_gate_open")},
                .commands = {urpg::events::EventCommand::SetSwitch("sw_gate_open", true)}
            }
        }
    });

    const auto graph = urpg::events::EventDependencyGraph::Build(doc);
    REQUIRE(graph.readersOf("sw_gate_open").size() == 1);
    REQUIRE(graph.writersOf("sw_gate_open").size() == 1);
    REQUIRE(graph.diagnostics().empty());
}
```

### Save/Load Round-Trip Test Pattern

```cpp
TEST_CASE("QuestRegistry save/load preserves objective state", "[quest][save]") {
    urpg::quest::QuestRegistry registry;
    registry.registerQuest({"quest_intro", "Intro", {{"obj_1", "Start", urpg::quest::QuestObjectiveState::Active, {}}}});
    REQUIRE(registry.completeObjective("quest_intro", "obj_1"));

    const auto saved = registry.saveToJson();

    urpg::quest::QuestRegistry loaded;
    loaded.loadFromJson(saved);

    const auto quest = loaded.getQuest("quest_intro");
    REQUIRE(quest.has_value());
    REQUIRE(quest->objectives[0].state == urpg::quest::QuestObjectiveState::Completed);
}
```

### Governance Script Pattern

```powershell
param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$schema = Join-Path $RepoRoot "content\schemas\quest.schema.json"
$fixture = Join-Path $RepoRoot "content\fixtures\quest_fixture.json"

if (-not (Test-Path $schema)) {
  throw "Missing quest schema: $schema"
}

if (-not (Test-Path $fixture)) {
  throw "Missing quest fixture: $fixture"
}

$fixtureJson = Get-Content $fixture -Raw | ConvertFrom-Json
if (-not $fixtureJson.quests) {
  throw "Quest fixture must contain a top-level 'quests' array."
}

Write-Host "Quest governance validation passed."
```

### Schema Fixture Pattern

```json
{
  "schemaVersion": "1.0.0",
  "quests": [
    {
      "id": "quest_intro",
      "title": "Find the Gate",
      "objectives": [
        {
          "id": "obj_find_key",
          "title": "Find the key",
          "initialState": "active",
          "requiredFlags": []
        }
      ]
    }
  ]
}
```

## LLM Execution Prompt Template

Use this prompt shape when handing one sprint to a future agent:

```text
Implement FFS-XX from docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md.

Scope:
- Own only the files listed under FFS-XX unless a build/test registration file must be updated.
- Follow existing URPG patterns: C++20, nlohmann/json, Catch2 v3, editor panel snapshot tests, CMake registration.
- Add runtime, editor, schema/fixture/governance only where the sprint requires them.
- Preserve unsupported data with explicit diagnostics.
- Do not update readiness status to READY.

Required verification:
- cmake --build --preset dev-debug
- focused urpg_tests tag for this sprint
- any new governance script
- check_release_readiness.ps1
- truth_reconciler.ps1

Final response must list changed files, commands run, and residual gaps.
```

## Suggested Parallelization

- FFS-01 and FFS-02 can run in parallel after FFS-00.
- FFS-03 should start before most authoring features because many later systems depend on event/dependency infrastructure.
- FFS-05 and FFS-06 can run in parallel after FFS-03 if file ownership is split between `engine/core/battle` and `engine/core/map`/`editor/spatial`.
- FFS-10 and FFS-12 can run in parallel after FFS-03, but coordinate IDs and save/load contracts.
- FFS-14 can run alongside almost everything if it only consumes existing snapshots and adds validation.
- FFS-17 should run continuously as features land, not only at the end.

## Final Product Gate For These Future Plans

The future feature program is complete only when:

- [ ] All selected feature sprints have runtime/editor/test/doc evidence.
- [ ] Every new schema has a fixture, changelog entry, and governance check.
- [ ] Every persisted feature has save/load and migration coverage.
- [ ] Every authoring feature has empty/loading/error/success/disabled state coverage in editor snapshots.
- [ ] Every generator/replay/simulation is deterministic under fixed seed.
- [ ] `urpg_project_audit --json` reports no unexpected blocker growth.
- [ ] `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` passes.
- [ ] Release docs distinguish current shipped scope from future optional scope.
