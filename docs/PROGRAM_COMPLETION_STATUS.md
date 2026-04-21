# URPG Program Completion Status

Status Date: 2026-04-21  
Program Scope: native-first roadmap rewire plus Wave 1 absorption, Wave 2 advanced capability expansion, post-Phase-2 compat exit hardening, and governance/template-readiness consolidation

Cross-cutting debt, truthfulness, and intake-governance source of truth: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

Canonical planning chain:
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` governs cross-cutting truthfulness, reconciliation, and Definition-of-Done requirements.
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` is the canonical product roadmap.
- `docs/PROGRAM_COMPLETION_STATUS.md` is the canonical latest-status snapshot.
- `docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`, `docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md`, and `docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md` are detailed planning inputs retained for traceability until their remaining deltas are absorbed into the canonical docs above.

2026-04-20 planning absorption note:
- `../URPG_MISSING_FEATURES_GOVERNANCE_AND_TEMPLATE_EXPANSION_PLAN_v2.md` is now treated as a planning-input addendum, not a parallel authority.
- Its approved deltas are being absorbed into the canonical roadmap/status/remediation stack as planned work.
- The absorbed scope from that addendum is now reflected here as explicit program-level remaining work, including:
  - schema versioning and changelog governance
  - breaking-change detection in CI
  - localization completeness checking
  - performance budget profiling and readiness gating
  - visual regression testing
  - accessibility governance
  - audio governance beyond the current compat-truth slice
  - input remapping and controller-governance
  - platform-specific export validation
  - achievement/trophy, mod-extension, and opt-in analytics layers
  - Create-a-Character runtime expansion
  - expanded template-readiness coverage for Monster Collector RPG, Cozy/Life RPG, Metroidvania-lite, and 2.5D RPG
- Its presence does **not** mean the proposed governance foundation, template matrices, or cross-cutting minimum bars are already landed.
- The first canonical governance artifacts for this lane now exist under:
  - [`docs/RELEASE_READINESS_MATRIX.md`](./RELEASE_READINESS_MATRIX.md)
  - [`docs/TEMPLATE_READINESS_MATRIX.md`](./TEMPLATE_READINESS_MATRIX.md)
  - [`docs/TRUTH_ALIGNMENT_RULES.md`](./TRUTH_ALIGNMENT_RULES.md)
  - [`docs/TEMPLATE_LABEL_RULES.md`](./TEMPLATE_LABEL_RULES.md)
  - [`docs/SUBSYSTEM_STATUS_RULES.md`](./SUBSYSTEM_STATUS_RULES.md)

Phase 2 runtime closure is complete as of 2026-04-19. Remaining compat work in this document is post-closure exit hardening, corpus depth, and ongoing truth-maintenance work, not unfinished baseline closure of the audited Phase 2 runtime slice.

Phase 3 diagnostics productization is complete as of 2026-04-19, Phase 4 governance/reconciliation closure is complete as of 2026-04-19, and Phase 5 hardening closure is also complete as of 2026-04-19. The canonical remediation findings table is now fully green, and focused Wave 1/program validation proof is recorded as of 2026-04-20. Remaining work in this document is post-closure compat hardening depth, Wave 2 roadmap execution, governance-foundation buildout, and ongoing truth-maintenance work rather than an unfinished remediation phase.

## Where we are now

- Phase 2 runtime closure is complete:
  - battle reward/event cadence and switch coverage are closed in the compat lane
  - `DataManager::loadDatabase()` seeded-container behavior is explicitly exercised
  - `Window_Base::contents()` lifecycle truthfulness is explicitly exercised
  - AudioManager remains honestly `PARTIAL` as a deterministic harness-backed surface with live compat-state observability
  - compat exit checklist evidence is refreshed through regression anchors for `PARTIAL` battle/window/data/audio truth surfaces plus diagnostics/export parity
- Phase 3 diagnostics surfaces are complete:
  - audio diagnostics project live `AudioCore` state and now expose selected-row navigation through the workspace export surface
  - migration wizard diagnostics expose rendered workflow actions, selected-result detail, issue-focused navigation, report save/load, and bound-runtime rerun flows
  - event-authority, menu, message, battle, and ability diagnostics remain snapshot/export truthful alongside their current workflow surfaces
- Presentation planning is now aligned around **Phase 5 â€” Environment & Presentation Polish**:
  - `editor/spatial/*` panels (`ElevationBrushPanel`, `PropPlacementPanel`) now have compiled `.cpp` sources registered in the build graph and expose lightweight render snapshots through the spatial editor lane.
  - `engine/core/presentation/*` still remains mostly header-oriented abstraction, but the registered runtime/release-validation lane is stable and the spatial authoring surfaces are no longer just header-only scaffolding.
  - `test_spatial_editor.cpp` and `test_presentation_runtime.cpp` are registered and passing, anchoring the now-compiled spatial authoring path.
- Supporting enablement landed for curated Hugging Face fixture ingestion:
  - permissive TMX, Visual Novel Maker, and Godot samples are vendored under `third_party/huggingface/`
  - restrictive RPG Maker MV / XP corpora remain manifest-only due to license constraints
  - asset tooling now indexes and validates the Hugging Face fixture roots
- Asset reality remains intentionally conservative:
  - the repo has strong importer/test/reference corpora
  - the repo does **not** yet have a serious license-cleared production asset library across tiles, portraits, UI, VFX, and audio
- Phase 4 governance/reconciliation closure is complete:
  - external repository intake governance now has recorded repo dispositions, fixture/attribution schemas, and a dedicated validation gate under [`docs/external-intake/`](./external-intake/) and [`tools/ci/check_phase4_intake_governance.ps1`](../tools/ci/check_phase4_intake_governance.ps1)
  - private-use asset intake governance now has schema-backed source manifests, truthful capture-state reporting, and reconciled asset-gap records under [`docs/asset_intake/`](./asset_intake/), [`imports/manifests/`](../imports/manifests/), and [`imports/reports/asset_intake/`](../imports/reports/asset_intake/)
  - release-readiness and truth-reconciliation gates now enforce bidirectional matrix/readiness coverage plus canonical status-date alignment across the core readiness docs
- Phase 5 hardening closure is complete:
  - async plugin callbacks are now deferred, FIFO, owning-thread-only, and covered by focused queue-integrity and stale-error-clearing regressions
  - `MapScene` audio ownership is now explicit and observable instead of relying on a constructor-created service instance
  - retained tile render commands are now explicitly documented and verified as pointer-stable across unchanged frames
  - the Phase 4 governance gate now enforces wrapper/facade-only production-candidate adoption plus provenance-preserving asset-promotion records
  - the native `PluginAPI` bridge now routes globals into `GlobalStateHub`, input queries into compat `InputManager` state, and entity lifecycle into a caller-bound ECS world instead of scratch-state placeholders
- **Planning Governance:** standalone PGMMV/native-absorption roadmap files are now treated as reference annexes under [`docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`](./TECHNICAL_DEBT_REMEDIATION_PLAN.md) and indexed in [`docs/archive/README.md`](./archive/README.md), not parallel execution authorities.
- The current program-level risk has shifted from baseline closure to governance drift:
  - subsystem-wide release-readiness matrix is still not landed as a full product-readiness signoff system; the repo currently has a first-slice canonical matrix plus machine-readable readiness records
  - template readiness matrix and template-claim guardrails are still not landed as full template signoff governance; the repo currently has a first-slice canonical matrix and conservative claim checks
  - subsystem-wide release-readiness and template-readiness governance now have canonical docs plus a machine-readable readiness record, but they are still a first-slice governance baseline rather than a complete product-readiness proof
  - template-claim guardrails, subsystem badge checks, truth-alignment checks, and a canonical release-signoff workflow artifact now exist, but they still stop short of full release-signoff enforcement
  - the project audit command and diagnostics tab now exist as a conservative readiness-derived scanner/scaffold, and the reported blockers can now reflect asset-intake, schema/changelog, project-schema, missing canonical input/localization/export artifact governance, and first-slice accessibility/audio/performance artifact governance where those concerns are represented by the current readiness records or roadmap-defined canonical paths; the audit engine is not yet a full project scanner
  - schema versioning/changelog governance and focused CI/doc enforcement now exist; a first-slice breaking-change detection script (`tools/ci/check_breaking_changes.ps1`) is now landed and validates schema `$id`/`title` presence, changelog freshness, and schema/changelog alignment
  - cross-cutting minimum bars for accessibility, localization completeness, input remapping, audio governance, and performance budgets are now canonical governance bars first, and first-slice implementations are landed for input remapping (`InputRemapStore`), localization completeness (`LocaleCatalog` + `CompletenessChecker`), and performance budget profiling (`PerfProfiler` + diagnostics panel)
- The 2026-04-20 validation snapshot should not be read as full product readiness:
  - it proves the currently exercised Wave 1/program lanes are green
  - it does not by itself satisfy the proposed subsystem/template readiness matrices or cross-cutting governance bars

- `main` is up to date and protected:
  - pull request required
  - 1 approval required
  - stale-review dismissal enabled
  - conversation resolution required
  - required checks: `gate1-pr`, `gitleaks`
- Branch layout is intentionally minimal:
  - `main`
  - `plan/native-feature-absorption-20260413`
- PR `#5` (native-first absorption plan) is merged into `main`.
- Recent execution slices landed:
  - Save/Data importer/upgrader ownership follow-through now lives under `engine/core/save/save_migration.*`, with typed diagnostics/JSONL export and preserved `_compat_mapping_notes` for unmapped compat metadata fields
  - native shell loop and scene scaffolding
  - compat global-state bridges for battle and data-manager flows
  - save slot descriptor loading and save-inspector slot-label projection
  - tactical routed battle fixture coverage across plugin reload
  - WindowCompat text bridge now submits renderer-facing `RenderLayer::TextCommand` payloads from `Window_Base::drawText`
  - compat `Window_Message` surface landed for dialogue-body alignment parity (`left`/`center`/`right`)
  - snapshot-style wrapped centered/right `drawTextEx` draw-history coverage landed
  - `Window_Selectable` pointer semantics now cover press/drag/release hit-testing, drag retargeting, drag-scroll, and mouse-wheel scrolling through `InputManager`
  - `Window_Base::contents()` now allocates compat bitmap records with tracked dimensions that stay synchronized with rect/padding changes instead of exposing handle-only state
  - runtime VFX cue pipeline baseline landed: active `BattleScene` cues now flow through the shared resolver/translator path into one-shot world/overlay presentation effect commands, with focused bridge/runtime coverage plus a sample release-validation effect-command envelope check
  - governance/template expansion addendum is now absorbed as planning input, and the canonical docs now treat subsystem readiness, template readiness, schema/version governance, and cross-cutting minimum bars as explicit remaining roadmap work rather than implied closure
  - governance foundation baseline landed: canonical release/template readiness matrices, truth/label rules, readiness schema data, focused CI/doc checks, and a first `ProjectAudit` CLI plus diagnostics tab scaffold are now part of the active tree
  - the current `ProjectAudit` slice remains readiness-derived first; asset-intake and schema/changelog governance affect the scan only where they have already been folded into canonical readiness/template blocker records, and the scanner now also surfaces missing canonical input/localization/export artifact paths where the roadmap already defines them
  - compat status truth pass is now reflected in the canonical docs; remaining work is to keep those labels and docs aligned as post-closure hardening continues
  - AudioManager compat closure advanced: deterministic playback position now advances during `update()`, duck/unduck ramps are frame-based, master/bus volume changes affect active playback, and the QuickJS `AudioManager` bridge now routes live compat state for BGM/BGS/ME/SE plus volume/ducking helpers
  - 2026-04-19 Phase 2 runtime closure reconciled the battle reward/event and switch coverage, window `contents()` lifecycle truthfulness, `DataManager::loadDatabase()` seeded-container behavior, and audio semantics documentation against the focused dev-mingw-debug verification lane
  - 2026-04-19 Phase 4 governance closure reconciled external-repo dispositions, private-asset source manifests, asset-gap truthfulness, and canonical status docs, and added a local validation gate for the intake-governance lane
- Agent swarm execution slice (2026-04-20): governance foundation and cross-cutting minimum bars expanded
  - `InputRemapStore` landed with MZ-compatible defaults, JSON persistence, version validation, and unsaved-changes tracking; schema `input_bindings.schema.json` and focused `[input][remap]` tests are registered and passing
  - `LocaleCatalog` and `CompletenessChecker` landed with bundle loading, merge, missing-key detection, coverage-percent reporting, and bundle JSON schema; focused `[localization]` tests are registered and passing
  - `PerfProfiler` landed with frame-time circular buffer, budget-violation detection, named section timings, and JSON summary export; `PerfDiagnosticsModel`/`PerfDiagnosticsPanel` provide diagnostics-workspace-ready snapshot integration; focused `[perf]` tests are registered and passing
  - `tools/ci/check_release_readiness.ps1` hardened with evidence-gated `READY` validation and template required-subsystem cross-checks
  - `tools/ci/check_breaking_changes.ps1` created for schema-breaking-change governance (validates schema identity, changelog freshness, and schema/changelog alignment)
  - Template spec docs created for `monster_collector_rpg`, `cozy_life_rpg`, `metroidvania_lite`, and `2_5d_rpg` under `docs/templates/`
  - `content/readiness/readiness_status.json` updated with new template entries; `docs/RELEASE_READINESS_MATRIX.md` and `docs/TEMPLATE_READINESS_MATRIX.md` updated to reflect `governance_foundation` promotion to `PARTIAL` and new template rows
- Agent swarm execution slice (2026-04-20 continued): Wave 2 advanced capabilities and remaining cross-cutting bars
  - `CharacterIdentity` runtime landed with name/portrait/body/class/attributes/appearance tokens, JSON round-trip, schema `character_identity.schema.json`, editor `CharacterCreatorModel`/`CharacterCreatorPanel`, and focused `[character]` tests
  - `AchievementRegistry` landed with `AchievementDef`/`AchievementProgress`, unlock-condition tokens, progress tracking, save/load JSON, schema `achievements.schema.json`, editor `AchievementPanel`, and focused `[achievement]` tests
  - `AccessibilityAuditor` landed with missing-label, focus-order, contrast, and navigation audit rules, schema `accessibility_report.schema.json`, editor `AccessibilityPanel`, and focused `[accessibility]` tests
  - `VisualRegressionHarness` landed with golden file load/save, `SnapshotValidator`-backed comparison, diff heatmap generation, JSON report export, PowerShell approval script `tools/visual_regression/approve_golden.ps1`, and focused `[testing][visual_regression]` tests
  - Fixed pre-existing test bug in `tests/unit/test_ability_activation.cpp` (`MockAbility::activate` now calls `commitAbility` so cooldown blocking is exercised)
  - Fixed pre-existing script bug in `tools/ci/run_presentation_gate.ps1` (`cmake --build --preset` was incorrectly run from inside the build directory)
- Third agent swarm execution slice (2026-04-20): platform export validation, mod extension, and analytics telemetry
  - `AudioMixPresetBank` landed with default presets (Default, Battle, Cinematic), category volume mapping, ducking rules, JSON serialization, schema `audio_mix_presets.schema.json`, editor `AudioMixPanel`, and focused `[audio][mix]` tests
  - `ExportValidator` landed with per-target requirement definitions (Windows, Linux, macOS, Web), `<filesystem>` existence checks, JSON report export, schema `export_validation_report.schema.json`, PowerShell CI script `tools/ci/check_platform_exports.ps1`, and focused `[export][validation]` tests
  - `ModRegistry` landed with manifest registration, Kahn's-algorithm dependency resolution, load-order topology, activate/deactivate state tracking, JSON save/load, schema `mod_manifest.schema.json`, editor `ModManagerPanel`, and focused `[mod]` tests
  - `AnalyticsDispatcher` landed with opt-in gating, deterministic tick counters, 1000-event circular buffer, JSON buffer snapshot, schema `analytics_config.schema.json`, editor `AnalyticsPanel`, and focused `[analytics]` tests
  - Fixed pre-existing header bug in `engine/core/tools/export_packager.h` (removed non-existent `#include` paths that were never exercised because the header was header-only and uncompiled)
  - `content/readiness/readiness_status.json` updated with new subsystem entries: `audio_mix_presets`, `export_validator`, `mod_registry`, `analytics_dispatcher`
  - `docs/SCHEMA_CHANGELOG.md` backfilled with entries for the 10 schema artifacts introduced across the second and third agent swarms
- Fourth agent swarm execution slice (2026-04-20): Lane 4 release hardening and cross-subsystem integration
  - `ExportPackager` converted from header-only to compiled `.cpp` with `validateBeforeExport` integration against `ExportValidator`, editor `ExportDiagnosticsPanel`, schema `export_config.schema.json`, and focused `[export][packager]` tests
  - `test_battle_save_integration.cpp` added to `urpg_integration_tests` with 4 cross-subsystem tests covering battle state serialization, save metadata preservation, post-battle map transition, and combined migration wizard reporting
  - `tools/ci/truth_reconciler.ps1` created for canonical truth reconciliation across readiness records, release/template matrices, schemas, and docs; added to `gate1-pr` in `.github/workflows/ci-gates.yml`
  - `tools/ci/check_cmake_completeness.ps1` created to validate no orphaned `.cpp` or test files exist outside `CMakeLists.txt`; added to `gate1-pr` and `gate2-nightly` in `.github/workflows/ci-gates.yml`
  - `tools/ci/check_breaking_changes.ps1` registered in all three CI gates and fixed for cross-platform path separators
  - `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` Lane 4 and Lane 5.1 checkboxes marked complete: breaking-change detection in CI, canonical truth reconciler, schema versioning governance, release/template readiness matrix enforcement
- Fifth agent swarm execution slice (2026-04-20): Wave 1 closure signoffs, presentation bridge, and incubating test hardening
  - `test_wave1_closure_integration.cpp` added to `urpg_integration_tests` with 4 cross-subsystem tests and 39 assertions covering battle-save-menu-message boundaries, migration wizard batch orchestration, and diagnostics workspace export parity
  - `BATTLE_CORE_CLOSURE_SIGNOFF.md` and `SAVE_DATA_CORE_CLOSURE_SIGNOFF.md` created as evidence-gathering artifacts for Wave 1 closure review
  - `PresentationBridge` extracted from header-only to compiled `engine/core/presentation/presentation_bridge.cpp` with `BuildFrameForActiveScene` and `BuildBattleSceneState` implementations
  - `test_presentation_bridge.cpp` added to `urpg_tests` with frame-building and battle-scene-state translation coverage
  - Incubating standalone tests (`test_ability_inspector.cpp`, `test_ability_pattern_integration.cpp`, `test_ability_state_machine.cpp`, `test_ability_tasks.cpp`, `test_effect_modifiers.cpp`, `test_pattern_field_editor.cpp`, `test_pattern_serialization.cpp`) converted from `int main()` to Catch2 and registered in `CMakeLists.txt`
  - Fixed `check_truth_alignment.ps1` stale check that was incorrectly throwing on roadmap release-readiness matrix claims
  - Fixed engine editor source compilation issues: `security_manager.h` missing includes, `cloud_service.h` broken include, 6 orphaned `.cpp` files registered in `CMakeLists.txt`
  - VFX cue pipeline enriched: `BattleScene` now emits semantic `EffectCueKind` values (`CastStart`, `HitConfirm`, `CriticalHit`, `GuardClash`, `MissSweep`, `HealPulse`, `DefeatFade`, `PhaseBanner`); `EffectResolver` routes kinds to dedicated presets (`healGlow`, `missSweep`, `defeatFade`, `phaseBanner`, `castSmall`, `critBurst`, `impactHeavy`); focused integration tests cover kind-based resolution and battle-scene emission.
- Sixth agent swarm execution slice (2026-04-20): BattleMigration troop phase condition mapping
  - BattleMigration troop phase condition mapping: `migrateTroop()` now maps turn-based, enemy HP threshold, and switch-based conditions from RPG Maker MV/MZ troop pages into native `phases[].condition` objects. Focused tests cover turn_count, hp_below_percent, switch_id, enemy_index, actor-condition warnings, and multi-page mapping.
  - BattleMigration action scope expansion: all 12 RPG Maker MV/MZ scope codes now map to native scope strings (single/random/all enemy/ally/dead/user combinations).
  - BattleMigration troop page event command mapping: common battle event commands (Show Text, Common Event, Change State, Force Action, Change Enemy HP/MP) now map to native phase effects. Unmapped commands are collected into a fallback effect with honest warnings.
  - Save/Data policy governance CI gate: `tools/ci/check_save_policy_governance.ps1` created to validate canonical save policy fixture against schema, enforce non-negative retention limits and autosave slot invariants, and verify changelog coverage. Wired into `gate1-pr` in `.github/workflows/ci-gates.yml`. Focused tests prove `SaveSessionCoordinator` can load the canonical fixture and produce expected runtime state.
- Seventh agent swarm execution slice (2026-04-20): BattleMigration residual gap closure
  - BattleMigration actor-based condition mapping: `migrateTroop()` now maps `actorValid` + `actorId` RPG Maker MV/MZ troop page conditions to native `phases[].condition.actor_id` strings. Previously emitted a warning; now produces mapped output.
  - BattleMigration non-battle event command mapping: `migrateTroop()` now maps Change Gold (code 125), Change Items (126), Change Weapons (127), Change Armors (128), Transfer Player (201), and Game Over (353) to native phase effects with operation/value parameters where applicable.
  - `battle_troops.schema.json` updated to accept `actor_id` in condition properties and new effect type enum values (`change_gold`, `change_items`, `change_weapons`, `change_armors`, `transfer_player`, `game_over`).
  - 7 new focused tests cover actor condition mapping, each new event command type, and mixed mapped/unmapped command lists.
- Sprint 01 execution slice (2026-04-21): BattleMigration boolean condition tree preservation
  - `BattleMigration::migrateTroop()` now preserves grouped troop-page condition logic as explicit recursive `and` / `or` trees under `phases[].condition.children` instead of flattening multi-leaf logic.
  - Legacy RPG Maker pages with multiple enabled condition flags now migrate to explicit `and` groups, while single-leaf pages keep the existing flat condition shape for compatibility.
  - Unsupported or malformed condition-tree nodes now emit typed warnings (`battle_condition_tree_unsupported_shape`) and preserve `_compat_condition_fallbacks` records on the migrated phase so no source branch is silently dropped.
  - `battle_troops.schema.json` now models recursive grouped conditions plus `_compat_condition_fallbacks`, and focused tests cover single-leaf, `and`, `or`, nested, and unsupported-tree scenarios.
- Sprint 01 execution slice (2026-04-21): BattleMigration remaining event-command coverage
  - `BattleMigration::migrateTroop()` now maps RPG Maker troop-page `Change Switches` (121) and `Change Variables` (122) commands into native `change_switches` and `change_variables` effects.
  - Unsupported troop-page commands now emit per-command structured `unsupported_command` effects carrying command code, reason, and preserved parameters instead of a single generic unmapped bucket.
  - Conditional branches (`111`) are now preserved as explicit unsupported artifacts with captured nested source commands so branch-scoped bodies are not misrepresented as unconditional native effects.
  - `battle_troops.schema.json` now accepts `change_switches`, `change_variables`, and `unsupported_command` effect types, and focused tests cover new mappings plus branch/unknown-command fallback behavior.
- Eighth agent swarm execution slice (2026-04-20): Achievement trigger integration
  - `AchievementTrigger` struct with structured unlock condition parsing supporting legacy underscore format (`kill_count_10`) and parameterized format (`item_collect:item_id=5:count=3`).
  - `AchievementEventBus` lightweight pub/sub for gameplay events.
  - `AchievementTriggerResolver` binds registry + event bus and auto-reports progress when emitted events match registered achievement triggers.
  - `AchievementDef` now supports explicit `target` override field for conditions where the target cannot be parsed from the condition string.
  - 11 focused tests cover trigger parsing, event matching, bus emission, resolver auto-reporting, parameterized events, and explicit target override.
- Ninth agent swarm execution slice (2026-04-20): Character Identity ECS integration
  - `CharacterIdentityComponent` ECS component wraps `CharacterIdentity` and attaches to entities with a dirty flag for sync tracking.
  - `CharacterIdentitySystem` propagates identity fields (name, classId, bodySpriteId) into `ActorComponent` and `VisualComponent` on update, respecting dirty flags and skipping empty fields.
  - `CharacterSpawner` deterministic helper creates actor entities from `CharacterSpawnRequest` with position, enemy flag, and immediate identity sync.
  - `ActorManager::CreateActorFromIdentity()` convenience method for spawning actors with identity pre-attached.
  - 8 focused tests cover component attachment, system sync propagation, empty-field preservation, dirty-flag gating, spawner positioning, enemy flag, and identity round-trip through ECS.
- Tenth agent swarm execution slice (2026-04-20): Save binary format loader
  - `LZString` decompressor implemented in C++ with `decompressFromBase64()` supporting the Base64 variant used by RPG Maker MV/MZ save files.
  - `RPGMakerSaveFileReader` class handles format detection (MV vs MZ heuristic), optional XOR decryption, LZString decompression, and JSON parsing.
  - 9 focused tests cover LZString decompression against known vectors (empty, single char, words, sentences, JSON payloads), format detection, XOR round-trip, file-not-found handling, invalid format handling, and end-to-end JSON payload reading.
- Sprint 01 execution slice (2026-04-21): Save/Data compat import closure follow-through
  - `SaveMigration` now includes `ImportCompatSaveDocument()` for end-to-end compat save import, producing native runtime payload JSON plus migrated metadata and severity-graded diagnostics from a single compat save document.
  - Common compat payload fields (`gold`, `mapId`, player position/direction, party, switches, variables) now map into native runtime-owned save state, while unsupported plugin blobs and unmapped payload fields are retained explicitly under `_compat_payload_retained`.
  - Focused save migration diagnostics now cover lossy field mapping, missing actor references, invalid-but-recoverable payload fields, and unsupported plugin-owned payload blobs.
  - Runtime validation now includes a real `.rpgsave` read/import/write/load round-trip proving imported RPG Maker save data can flow through the normal native runtime load path without structural corruption.
- Sprint 02 execution slice (2026-04-21): ProjectAudit cross-cutting governance artifact checks
  - `urpg_project_audit` now emits explicit `accessibilityArtifacts`, `audioArtifacts`, and `performanceArtifacts` governance sections plus matching top-level issue counts in addition to the earlier localization/input/export sections.
  - `ProjectAuditPanel::RenderSnapshot` now captures those new governance sections and counts so diagnostics consumers can render the richer project-audit contract without inferring it from prose.
  - Focused CLI and panel tests now assert the expanded audit JSON shape and summary strings for accessibility/audio/performance governance evidence.
- Sprint 02 execution slice (2026-04-21): ProjectAudit diagnostics/export parity
  - `DiagnosticsWorkspace::exportAsJson()` now carries the richer project-audit governance contract through active-tab export, including top-level accessibility/audio/performance artifact issue counts and the corresponding governance sections beside the earlier localization/input/export data.
  - The workspace integration regression now binds a report containing all seven governance sections and verifies both the rendered panel snapshot and the exported diagnostics JSON stay in parity for those fields.
  - Focused verification passed via `.\build\dev-ninja-debug\urpg_tests.exe "[project_audit],[editor][diagnostics][integration][project_audit]" --reporter compact` plus the CLI audit lane.
- Sprint 02 execution slice (2026-04-21): Governance-depth readiness enforcement
  - `check_release_readiness.ps1` now validates the emitted `urpg_project_audit` JSON contract directly, including the richer `accessibilityArtifacts`, `audioArtifacts`, and `performanceArtifacts` governance sections plus their top-level issue counts, instead of only checking doc/date/matrix drift.
  - `truth_reconciler.ps1` now includes `docs/PROJECT_AUDIT.md` in the status-date alignment chain and verifies the project-audit doc still describes the shipped richer governance contract.
  - `docs/PROJECT_AUDIT.md`, `docs/RELEASE_READINESS_MATRIX.md`, and `docs/TRUTH_ALIGNMENT_RULES.md` were reconciled so the governance-foundation wording matches the now-enforced audit depth.
- Sprint 02 closeout snapshot (2026-04-21)
  - `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` => 835/835 passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` => passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` => passed
- Sprint 03 execution slice (2026-04-21): Compat bridge exit signoff artifact
  - Added `docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md` so `compat_bridge_exit` now has the same explicit evidence-gathering signoff pattern already used by `battle_core` and `save_data_core`.
  - `check_release_readiness.ps1` now requires signoff artifacts for human-review-gated subsystems (`battle_core`, `save_data_core`, `compat_bridge_exit`) instead of relying only on matrix prose.
  - `truth_reconciler.ps1`, `RELEASE_READINESS_MATRIX.md`, `COMPAT_EXIT_CHECKLIST.md`, and `readiness_status.json` were reconciled so compat signoff language and residual gaps stay aligned with that governed artifact.
- Sprint 03 execution slice (2026-04-21): Generalized signoff-governance enforcement
  - `check_release_readiness.ps1` now verifies that every currently signoff-governed subsystem row carries signoff/human-review wording in both `readiness_status.json` and the release matrix row text, instead of only checking for artifact existence.
  - `truth_reconciler.ps1` now validates the conservative content pattern for all current signoff artifacts (`battle_core`, `save_data_core`, `compat_bridge_exit`) rather than only the compat signoff doc.
  - `SUBSYSTEM_STATUS_RULES.md` and `TRUTH_ALIGNMENT_RULES.md` now state the canonical rule directly: human-review-gated readiness language requires an explicit non-promoting signoff artifact.
- Sprint 03 closeout snapshot (2026-04-21)
  - `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` => 835/835 passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` => passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` => passed
- Sprint 04 execution slice (2026-04-21): Canonical release-signoff workflow artifact
  - Added `docs/RELEASE_SIGNOFF_WORKFLOW.md` as the canonical non-promoting workflow artifact for governed release-signoff discipline across readiness/signoff lanes.
  - `check_release_readiness.ps1` and `truth_reconciler.ps1` now require that workflow doc, align its status date with the readiness stack, and verify its expected non-promoting workflow language.
  - `RELEASE_READINESS_MATRIX.md`, `TRUTH_ALIGNMENT_RULES.md`, and `readiness_status.json` now record that the governance foundation includes a canonical release-signoff workflow artifact, while keeping full release-signoff enforcement honestly below `READY`.
- Sprint 04 execution slice (2026-04-21): Project-audit release-signoff workflow parity
  - `urpg_project_audit` now emits a dedicated `releaseSignoffWorkflow` governance section plus `releaseSignoffWorkflowIssueCount`, so the audit contract can surface the canonical release-signoff workflow artifact instead of leaving it implicit in the readiness/truth gates.
  - `ProjectAuditPanel` and `DiagnosticsWorkspace` now preserve and export that workflow-governance section through the diagnostics path, with focused panel/workspace/CLI coverage keeping the contract aligned end to end.
- Sprint 04 closeout snapshot (2026-04-21)
  - `.\build\dev-ninja-debug\urpg_tests.exe "[project_audit],[project_audit_cli],[editor][diagnostics][integration][project_audit]" --reporter compact` => 368 assertions / 10 test cases passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` => passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` => passed
  - `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` => 835/835 passed
- Sprint 05 execution slice (2026-04-21): Compat corpus directory re-import depth
  - `test_compat_plugin_fixtures.cpp` now includes a weekly-lane anchor proving the curated all-profile orchestration fixture survives directory-based corpus import, full unload, directory re-import, and by-name rerun without failure diagnostics.
  - `COMPAT_EXIT_CHECKLIST.md` now records that the weekly compat lane covers directory import plus orchestration rerun, narrowing the remaining compat maintenance gap to future corpus growth and new failure-path parity rather than a missing import-style depth anchor.
- Sprint 05 execution slice (2026-04-21): Compat by-name dependency-gating parity
  - `test_compat_plugin_failure_diagnostics.cpp` now proves real curated by-name dispatch emits the same deterministic dependency-missing diagnostics as direct command dispatch when a dependent profile loses `VisuStella_CoreEngine_MZ`.
  - `COMPAT_EXIT_CHECKLIST.md` now records that the failure-parity lane covers both direct and by-name invocation surfaces for this dependency-gating path, narrowing the remaining compat maintenance work to future failure-operation growth and routine truth upkeep.
- Sprint 05 closeout snapshot (2026-04-21)
  - `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "Compat fixtures: curated all-profile orchestration scenario survives directory re-import|Compat fixtures: dependent command execution is gated with diagnostics when core dependency is missing"` => 2/2 passed
  - `ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure` => 44/44 passed
  - `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` => 835/835 passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` => passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` => passed
- Latest recorded local validation snapshot:
  - recorded under the `dev-ninja-debug` preset: `ctest --preset dev-all --output-on-failure` => 869/869 passed (includes all previous lanes plus Wave 1 closure integration, presentation bridge, incubating test conversion, cmake-completeness-governance, save-policy-governance, battle-migration-residual-gaps, achievement-trigger-integration, character-identity-ecs, and save-binary-format-loader lanes)
- Latest focused Phase 5 hardening validation snapshot:
  - `ctest --test-dir build -C Debug --output-on-failure -R "PluginManager: Command execution|MapScene:|SceneManager:"` => 9/9 passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1` => passed
- Latest focused presentation validation snapshot:
  - `ctest --test-dir build -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure` => 3/3 passed
  - includes the dedicated `[presentation]` unit lane, the standalone release-validation harness, and the spatial editor authoring lane
  - CI `gate1-pr` now invokes the focused presentation gate explicitly via `tools/ci/run_presentation_gate.ps1` after the shared build step
- Latest broader Wave 1/program release-readiness snapshot (2026-04-20):
  - `ctest --test-dir build -C Debug --output-on-failure -R "PluginManager: Command execution|MapScene:|SceneManager:"` => 9/9 passed
  - `ctest --test-dir build -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure` => 3/3 passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1` => passed
  - `ctest --preset dev-all --output-on-failure` => 790/790 passed
- Latest focused compat validation snapshot:
  - `.\build\Debug\urpg_compat_tests.exe --reporter compact` => 3375 assertions / 43 test cases passed
- Latest focused audio validation snapshot:
  - `.\build\dev-mingw-debug\urpg_tests.exe "[audio_manager]"` => 147 assertions / 12 test cases passed
- Latest focused Phase 2 validation snapshot:
  - re-run after the 2026-04-19 doc edits: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleManager:|Window_Base contents lifecycle allocates and rotates deterministic handles|DataManager loadDatabase populates seeded database containers|AudioManager:"` => 42/42 passed on the exact focused subset (`BattleManager:` and `AudioManager:` suites plus one `DataManager` case and one `Window_Base` case)

## Progress made in this cycle

- Merged native-first direction and follow-up execution commits into `main`.
- 2026-04-19 closure note: battle reward distribution, switch checks, and battle-event cadence coverage are now closed in the compat lane; `Window_Base::contents()` lifecycle truthfulness and seeded `loadDatabase()` behavior are explicitly tested; AudioManager remains honestly `PARTIAL` while deterministic harness semantics stay documented and verified. The focused Phase 2 lane was re-run after the doc edits and stayed green.
- Finished repo governance cleanup:
  - removed extra remote branches
  - aligned work branch and main tip
  - disabled Dependabot PR branch churn in `.github/dependabot.yml`
  - enforced `main` branch protection policy
- Established external repository intake governance artifacts:
  - [docs/external-intake/repo-watchlist.md](./external-intake/repo-watchlist.md)
  - [docs/external-intake/license-matrix.md](./external-intake/license-matrix.md)
  - [docs/external-intake/repo-audit-template.md](./external-intake/repo-audit-template.md)
  - [docs/external-intake/urpg_feature_adoption_matrix.md](./external-intake/urpg_feature_adoption_matrix.md)
- Established private-use asset intake governance artifacts:
  - [docs/asset_intake/ASSET_SOURCE_REGISTRY.md](./asset_intake/ASSET_SOURCE_REGISTRY.md)
  - [docs/asset_intake/ASSET_PROMOTION_GUIDE.md](./asset_intake/ASSET_PROMOTION_GUIDE.md)
  - [docs/asset_intake/ASSET_CATEGORY_GAPS.md](./asset_intake/ASSET_CATEGORY_GAPS.md)
  - Scaffolded [`imports/staging/asset_intake/`](../imports/staging/asset_intake/), [`imports/normalized/`](../imports/normalized/), [`imports/manifests/`](../imports/manifests/), [`imports/reports/`](../imports/reports/), and [`third_party/github_assets/`](../third_party/github_assets/)
- Updated subsystem planning set:
  - `docs/UI_MENU_CORE_NATIVE_SPEC.md`
  - `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
  - `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
  - `docs/BATTLE_CORE_NATIVE_SPEC.md`
- Rewrote the primary roadmap into an integrated plan:
  - `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
  - includes Wave 2 advanced capability tracks (ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, 2.5D presentation lane, timeline orchestration, editor utilities)
- Normalized planning authority for PGMMV/native-absorption scope:
  - `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` now governs roadmap-alignment and truthfulness requirements for newly added planning scope
  - `docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`, `docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md`, and `docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md` are now linked as detailed planning inputs rather than canonical status/roadmap authorities
  - the remediation hub now also carries a finding-status dashboard, expanded documentation tree, and additional planning-governance risks so execution/state review can start from one document
- Added canonical Wave 1 closure checklist governance:
  - canonical source: `docs/WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md`
  - subsystem spec sync tool: `tools/docs/sync-wave1-spec-checklist.ps1`
  - CI/local gate drift check: `tools/ci/check_wave1_spec_checklists.ps1`
- Added Message/Text Core runtime ownership slice:
  - `engine/core/message/message_core.h`
  - `engine/core/message/message_core.cpp`
  - route-mode mapping for `speaker`/`narration`/`system`
  - native rich-text escape tokenization/layout and deterministic metrics
  - message flow runner with page advance + choice prompt state snapshot/restore
  - unit anchor coverage in `tests/unit/test_message_text_core.cpp`
  - compat-bridge integration updates:
    - `Window_Base::drawText` now emits backend-facing text draw commands through `RenderLayer`
    - `Window_Base::drawTextEx` now honors configurable text alignment state (`left`/`center`/`right`)
    - `Window_Message` compat surface provides deterministic message-body alignment behavior for dialogue windows
  - unit/snapshot coverage updates in `tests/unit/test_window_compat.cpp`:
    - renderer command emission checks (`RenderCmdType::Text`)
    - `Window_Message` centered/right dialogue alignment checks
    - wrapped centered/right `drawTextEx` deterministic snapshot checks
- Added Message/Text editor inspector + preview diagnostics slice:
  - `editor/message/message_inspector_model.h`
  - `editor/message/message_inspector_model.cpp`
  - `editor/message/message_inspector_panel.h`
  - `editor/message/message_inspector_panel.cpp`
  - diagnostics workspace `message_text` tab wiring
  - unit coverage in `tests/unit/test_message_inspector_model.cpp` and `tests/unit/test_message_inspector_panel.cpp`
- Added Message/Text schema + migration completion slice:
  - schema contracts:
    - `content/schemas/message_styles.schema.json`
    - `content/schemas/dialogue_sequences.schema.json`
    - `content/schemas/rich_text_tokens.schema.json`
    - `content/schemas/choice_prompts.schema.json`
  - compat-to-native upgrader mapping:
    - `engine/core/message/message_migration.h`
    - `engine/core/message/message_migration.cpp`
    - safe fallback mapping for unsupported route/escape/body/choice constructs
    - structured migration diagnostics JSONL export
  - unit coverage in `tests/unit/test_message_migration.cpp` and `tests/unit/test_message_schema_contracts.cpp`
- Added Battle Core native runtime ownership slice:
  - `engine/core/battle/battle_core.h`
  - `engine/core/battle/battle_core.cpp`
  - deterministic `BattleFlowController` phase/turn/escape state owner
  - deterministic `BattleActionQueue` ordering owner (speed + priority + stable tie-break)
  - deterministic `BattleRuleResolver` owner (damage + escape ratio baselines)
  - live `BattleScene` routing now sends phase progression, ordered action draining, and damage resolution through Battle Core rather than a parallel scene-local authority
  - `PresentationBridge` now derives battle presentation state from an active `BattleScene` so battle translator/HUD clients can consume native battle participants without a manually populated `PresentationContext::battleState`
  - unit coverage in `tests/unit/test_battle_core.cpp` and `tests/unit/test_battle_scene_native.cpp`
- Added Battle Core editor inspector + preview diagnostics slice:
  - `editor/battle/battle_inspector_model.h`
  - `editor/battle/battle_inspector_model.cpp`
  - `editor/battle/battle_preview_panel.h`
  - `editor/battle/battle_preview_panel.cpp`
  - `editor/battle/battle_inspector_panel.h`
  - `editor/battle/battle_inspector_panel.cpp`
  - diagnostics workspace `battle` tab wiring
  - battle diagnostics live-scene parity path now accepts a runtime exposing `flowController()`, `nativeActionQueue()`, and optional `buildDiagnosticsPreview()` preview data so preview/export state can mirror the live scene snapshot instead of only default preview inputs
  - unit coverage in:
    - `tests/unit/test_battle_inspector_model.cpp`
    - `tests/unit/test_battle_preview_panel.cpp`
    - `tests/unit/test_battle_inspector_panel.cpp`
    - `tests/unit/test_diagnostics_workspace.cpp`
    - `tests/unit/test_presentation_runtime.cpp`
- Continued diagnostics workspace productization and truthfulness closure:
  - `event_authority` tab now renders through a panel-owned snapshot instead of refreshing silently, and that snapshot/export now carries the visible row body plus event-id/severity/mode filtering, selection, row-navigation, and selected-row detail state
  - audio diagnostics now project real `AudioCore` active-source rows rather than count-only placeholder state, and the panel snapshot now carries those projected rows directly
  - migration wizard diagnostics now support:
    - message, menu, and battle migration execution reporting
    - rendered summary text in panel snapshots
    - clear/reset through both the panel and `DiagnosticsWorkspace`
    - typed per-subsystem results
    - selectable subsystem state with default selection after a run
    - next/previous subsystem-result navigation with explicit can-navigate snapshot state
    - selected-subsystem detail, status, and summary fields in the render snapshot
  - focused coverage expanded in:
    - `tests/unit/test_event_authority_panel.cpp`
    - `tests/unit/test_audio_inspector.cpp`
    - `tests/unit/test_migration_wizard.cpp`
    - `tests/unit/test_diagnostics_workspace.cpp`
- **Added AI Copilot Core Native Ownership Slice (Wave 2 Advanced):**
  - `engine/core/message/chatbot_component.h`: Primary AI hub supporting streaming, tool-calling, and history management.
  - `engine/core/message/ai_sync_coordinator.h`: Local-memory sync plumbing only; operational cloud-ready history synchronization requires a non-stub `ICloudService` backend provided out of tree.
  - `engine/core/ai/ai_connectivity.h`: Service interface scaffolding for OpenAI, Anthropic, and Local Llama.cpp; live connectivity requires out-of-tree backend integration.
  - `engine/core/ai/personality_registry.h`: Prompt templates for archetypal NPC behaviors (Elder, Warrior, Rogue, etc.).
  - `engine/core/audio/audio_ai_bridge.h`: Dynamic audio orchestration via AI commands.
  - `engine/core/animation/animation_ai_bridge.h`: NLP-to-keyframe translation for actor movement.
  - `engine/core/debug/debug_ai_bridge.h`: Runtime state and call-stack serialization for AI-driven debugging.
  - `engine/core/ui/chat_window.h`: Native UI supporting multi-line wrapping and real-time streaming text.
  - `tests/unit/test_ai_bridge_regex.cpp`: Unit-level validation for cross-component AI command parsing.
- Added Battle Core schema + migration completion slice:
  - schema contracts:
    - [content/schemas/battle_troops.schema.json](../content/schemas/battle_troops.schema.json)
    - [content/schemas/battle_actions.schema.json](../content/schemas/battle_actions.schema.json)
  - compat-to-native upgrader mapping:
    - [engine/core/battle/battle_migration.h](../engine/core/battle/battle_migration.h)
    - [tests/unit/test_battle_migration.cpp](../tests/unit/test_battle_migration.cpp)
  - unsupported troop page/phase scripts and unsupported action scope/effect payloads now emit migration warnings while preserving schema-shaped fallback output
  - migration wizard battle summaries now count those warnings through `MigrationWizardModel`
  - unit coverage in `tests/unit/test_battle_migration.cpp`, `tests/unit/test_battle_core.cpp`, and `tests/unit/test_migration_wizard.cpp`
- Hardened compat directory-load failure diagnostics coverage:
  - upgraded `load_plugins_directory`, `load_plugins_directory_scan`, and `load_plugins_directory_scan_entry` tests to assert JSONL row shape plus report-model and panel projection parity
  - added explicit severity and operation mapping checks in `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- Expanded routed conformance depth across the curated 10-profile corpus:
  - added a single orchestration fixture scenario that invokes all 10 profile commands (mixed `invoke` + `invokeByName`) and validates profile routing before and after plugin reload
  - added coverage in `tests/compat/test_compat_plugin_fixtures.cpp` (`curated all-profile orchestration scenario survives plugin reload`)
- Added native UI/Menu interaction ownership slices:
  - state-aware command visibility and enabled evaluation in `engine/core/ui/menu_command_registry.h`
  - route fallback support (`primary -> fallback`) for both native and custom routes in `engine/core/ui/menu_route_resolver.h`
  - menu scene interaction flow in `engine/core/ui/menu_scene_graph.h`:
    - confirm command activation through `MenuRouteResolver`
    - root-guarded cancel/back navigation with optional root-pop mode
    - multi-pane left/right focus traversal with wrap behavior
    - pane focus gating to skip panes without visible+enabled commands
    - active-pane auto-recovery when state changes invalidate current focus
    - blocked-command metadata (`lastBlockedCommandId`/`lastBlockedReason`) plus blocked callback hook
  - one-call registry integration helper (`setCommandStateFromRegistry`) to bind switch/variable state into scene evaluators
  - expanded unit coverage in `tests/unit/test_menu_core.cpp` for confirm/cancel/pane-focus/recovery/state-helper/blocked-reason flows
- Added UI/Menu editor/runtime integration hardening:
  - `DiagnosticsWorkspace::bindMenuRuntime` / `clearMenuRuntime` now have integration coverage proving menu diagnostics bind, export, tab-switch, and clear without stale state
  - `MenuInspectorModel::Clear()` now resets transient filters, selection, issues, and summary state for clean runtime rebinding
  - `MenuPreviewPanel` is now surfaced through `DiagnosticsWorkspace` so the menu tab exposes both inspector and preview workflow surfaces instead of leaving preview tooling orphaned
  - `DiagnosticsWorkspace` now exposes menu workflow actions directly at the workspace layer (`setMenuCommandIdFilter`, `clearMenuCommandIdFilter`, `setMenuShowIssuesOnly`, `selectMenuRow`) instead of requiring callers to reach through the menu model
  - `MenuInspectorModel` now preserves selected command state across filter rebuilds when the selected command remains visible
  - menu active-tab export now includes `command_id_filter`, `show_issues_only`, and a structured `selected_row` payload so the workspace snapshot carries real menu workflow context instead of only row lists and issue arrays
- Added richer UI/Menu legacy import mapping:
  - `MenuSceneSerializer::ImportLegacy()` now preserves explicit `mainMenu.commands` route targets, fallback routes, custom route IDs, and visibility/enable rules when compat evidence provides them
  - route parsing now accepts lower-case native route identifiers during menu legacy import
  - unit coverage expanded in `tests/unit/test_menu_legacy_import.cpp`
- Added UI/Menu round-trip serialization:
  - `MenuSceneGraph` now exposes registered-scene enumeration needed for truthful export
  - `MenuSceneSerializer::Serialize()` now emits a non-empty native scene definition for registered menu graphs
  - round-trip coverage now serializes a native menu graph, deserializes it, and checks structural equivalence
- Latest focused validation snapshot for native UI/Menu lane:
  - `.\build\dev-mingw-debug\urpg_tests.exe "[ui][menu]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
- Latest focused validation snapshot for migration wizard/editor diagnostics productization:
  - `.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][wizard]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[events][panel]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[events][panel][render]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[editor][audio][inspector]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
- Latest focused validation snapshot for Battle Core native runtime/editor/migration closure:
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[battle][scene][diagnostics],[battle][editor][panel],[editor][diagnostics][integration][battle_preview]" --reporter compact` => 67 assertions / 5 test cases passed
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[presentation][bridge],[presentation][runtime]" --reporter compact` => 40 assertions / 5 test cases passed
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[editor][diagnostics][wizard],[battle][migration]" --reporter compact` => 521 assertions / 40 test cases passed
  - `ctest --test-dir C:\dev\urpg-battle-build --output-on-failure -R "PresentationBridge derives battle frame from active BattleScene|PresentationBridge builds frame for active scene using runtime|BattleScene builds diagnostics preview from the next ordered queued action|Battle inspector panel binds live scene diagnostics preview payload|DiagnosticsWorkspace - Battle tab exports live scene diagnostics preview payload|BattleMigration:|MigrationWizardModel: battle migration warnings propagate from unsupported troop phase/page data|MigrationWizardModel: Batch Orchestration"` => 11/11 passed
- Latest focused validation snapshot for Save/Data schema and runtime/catalog lanes:
  - `.\build\Debug\urpg_tests.exe "[save][schema],[save][catalog],[save][runtime],[save][editor],[save][panel][integration],[save][metadata],[editor][diagnostics][integration][save_actions]" --reporter compact` => 378 assertions / 25 test cases passed
  - `.\build\Debug\urpg_integration_tests.exe "[integration][save]" --reporter compact` => 10 assertions / 2 test cases passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "Save migration|Runtime save loader hydrates metadata after imported save migration|Migration runner upgrades imported save metadata into URPG runtime shape|MigrationWizard"` => 42/42 passed
  - Save/Data diagnostics export now includes runtime-owned `recovery_diagnostics` and `serialization_schema` payloads from the bound save inspector seam, plus live policy draft/validation/apply workflow coverage through the save inspector panel and diagnostics workspace.
- Second agent swarm pass (2026-04-17):
  - Input manager status honesty: downgraded all 79 inflated `FULL` labels to `PARTIAL` in `runtimes/compat_js/input_manager.cpp`; aligned `tests/unit/test_input_manager.cpp`.
  - Migration wizard productization: added `rerunSubsystem(id, project_data)` to `MigrationWizardModel` and `MigrationWizardPanel`; exposed `can_rerun_selected_subsystem` in render snapshot; implemented `bindMigrationWizardRuntime()` in `DiagnosticsWorkspace`; added 3 new workflow tests.
  - Data manager runtime closure: implemented real `loadDatabase()` orchestration with seeded actor/class/skill/item records; wired up all stubbed JS bindings in `registerAPI` (loadDatabase, saveGame, loadGame, getGold, setGold, getSwitch, setSwitch, getVariable, setVariable, getItemCount, gainItem); implemented real `getActorsAsValue()`, `getItemsAsValue()`, `getSkillsAsValue()`, `getWeaponsAsValue()`, `getArmorsAsValue()`, `getClassesAsValue()` serializers.
  - Doc sync: linked new intake governance artifacts into `URPG_repo_intake_plan.md`, `URPG_private_asset_intake_plan.md`, `TECHNICAL_DEBT_REMEDIATION_PLAN.md`, and `PROGRAM_COMPLETION_STATUS.md`; marked P3-02 and P3-03 as partially remediated.
  - Latest recorded validation snapshot: `urpg_tests` => 400/400 passed (5,098 assertions).
- 2026-04-18 audio compat closure:
  - `AudioManager` now advances deterministic playback position, applies deterministic duck/unduck ramps, applies master/bus volume scaling to active playback, and exposes live compat-state bindings through `AudioManager::registerAPI()`.
  - Documentation truth was reconciled across `WORKLOG.md`, `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`, `docs/DEVELOPMENT_KICKOFF.md`, and `docs/COMPAT_EXIT_CHECKLIST.md` so the audio lane is consistently described as deterministic harness-backed `PARTIAL` behavior rather than a `FULL` live-audio surface.
- 2026-04-18 window compat closure follow-through:
  - `Window_Selectable` now supports keyboard/gamepad navigation, pointer press/drag/release hit-testing, drag retargeting, drag-scroll, and mouse-wheel scrolling through `InputManager`.
  - `Window_Base::contents()` now tracks compat bitmap metadata (handle, width, height) and keeps that metadata synchronized with rect/padding changes.
  - Focused validation now passes through the active local profile with `urpg_tests.exe "[input_manager],[compat][window]"` and repo-wide validation passes with `ctest --preset dev-all --output-on-failure`.
- Latest focused validation snapshot for Message/Text renderer integration lane:
  - `.\build\dev-mingw-debug\urpg_tests.exe "[compat]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[compat][window]"` => active local lane now includes richer pointer/release semantics and contents-bitmap metadata coverage; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[compat][window][message]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot
  - `.\build\dev-mingw-debug\urpg_tests.exe "[compat][window][snapshot]"` => lane previously validated; rerun in the active local profile before treating as a fresh release snapshot

## Definition of 100% complete (for this program scope)

The scope in this document is considered 100% complete when all items below are done:

1. Compat lane is trustworthy as an import/validation bridge with explicit exit criteria satisfied.
   This includes keeping the QuickJS lane explicitly scoped as a fixture-backed compat harness until a real runtime exists.
2. Wave 1 subsystems have native runtime ownership and no longer rely on plugin-shaped core delivery.
3. Wave 1 subsystems have first-class editor workflows (inspect, preview, diagnose, migrate).
4. Native schemas and migration paths are production-ready for upgraded MZ projects.
5. Regression and release gates prove stability for both native and compat lanes.
6. Wave 2 advanced capability baseline is delivered at production quality.

## Next steps (current sprint)

1. ~~Complete UI/Menu editor productization~~ (DONE 2026-04-19):
   - MenuInspectorModel now supports `UpdateCommandLabel`, `UpdateCommandRoute`, `RemoveCommand`, `AddCommand`, and `ApplyToRuntime` for true authoring workflows.
   - MenuPreviewPanel snapshot now carries `command_labels` and `command_enabled` for richer preview fidelity.
   - Added 6 new tests covering edit, route change, remove/add, selection preservation, preview refresh, and clear behavior.
   - All menu lanes pass (35/35 in focused suite, 581/581 repo-wide).
2. ~~Complete Message/Text renderer bridge closure~~ (DONE 2026-04-19):
   - `OpenGLRenderer::processCommands()` now explicitly handles `RectCommand` (TIER_BASIC placeholder logging).
   - `MapScene::onUpdate()` now submits `TextCommand` and `RectCommand` to `RenderLayer` when `m_messageRunner.isActive()`.
   - Added 5 focused tests: MessageFlowRunner advance-to-Completed, cancel-to-Idle, all-disabled choice rejection, MapScene render command submission during dialogue, and Window_Message RenderLayer emission verification.
   - All message lanes pass (`[message]` 22 cases, `[compat][window][message]` 2 cases, `MapScene:` 7 cases).
3. ~~Finalize UI/Menu schema + migration mapping~~ (DONE 2026-04-19):
   - Updated `menu_scene_graph.schema.json` and `menu_commands.schema.json` to match runtime routes and include `visibility_rules`/`enable_rules` definitions.
   - `MenuSceneSerializer::Serialize` now emits rules arrays; `Deserialize` now restores them.
   - `MenuMigration::MigrateCommandPanel` now maps fallback routes and preserves rich visibility/enable rules from plugin evidence.
   - Added focused tests for rule round-tripping and migration fallback/rule coverage.
4. Continue post-Phase-2 compat exit hardening through Sprint 05:
   - Sprint 05 opens the explicit compat maintenance lane for curated corpus depth, new failure-path parity, and truth-maintenance follow-through.
   - keep new routed failure operations locked to JSONL/report/panel parity and maintain weekly conformance depth growth.
5. Keep canonical status/docs aligned with current validation evidence:
   - update README/remediation/kickoff/checklist language whenever closure state or residual compat scope changes.
6. Wave 2 roadmap execution:
   - The initial Wave 2 baseline slices tracked on this branch are landed, but the documented Wave 2 roadmap remains mandatory first-class scope.
   - Any documented Wave 2 lane that is not yet product-complete must be treated as required execution work, not optional follow-on scope.

## Remaining work to reach 100%

### 1. Compat exit hardening (post-Phase-2 remaining work)

Phase 2 runtime closure is already complete. The remaining compat work below is about confidence depth, ongoing evidence, and truthful residual scoping.

- [x] Expand routed conformance coverage beyond the current anchor scenarios across the curated 10-profile corpus.
- [x] Keep every new failure operation locked to JSONL artifacts, report ingestion/export, and panel projection assertions.
- [x] Complete explicit compat exit checklist with signed pass criteria for import confidence and migration confidence.
- [x] Keep runtime status labels and public docs aligned with actual implementation scope; do not relabel fixture-backed or placeholder-backed paths as `FULL` without closing the underlying TODOs.

### 2. Wave 1 native runtime ownership (remaining)

- [x] UI/Menu Core: complete production closure for command registry/scene graph/route resolver ownership (runtime, editor, schema, and migration are now closed).
- [x] Message/Text Core: runtime renderer handoff AND editor productization are closed. Schema and migration field coverage is complete. Remaining work is release-readiness proof (Task 10).
- [x] Battle Core: runtime, editor, schema, migration, diagnostics, preview parity, and active local release evidence are now reconciled and recorded. Wave 1 closure signoff exists at `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`.
- [x] Save/Data Core: runtime, editor, schema, migration, diagnostics, recovery, importer/upgrader ownership, and active local release evidence are now reconciled and recorded. Wave 1 closure signoff exists at `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`.

### 3. Wave 1 editor productization (remaining)

- [x] UI/Menu Core: inspector and preview surfaces are shipped; remaining work is deeper authoring/productization beyond the landed diagnostics workflow.
- [x] Message/Text Core: Ship inspector and preview surfaces.
- [x] Battle Core: Ship preview surfaces.
- [x] Ship diagnostics and validation wiring directly in editor panels for Wave 1 schemas.

### 4. Schema and migration completion (remaining)

- [x] Finalize Wave 1 schema contracts for runtime and editor ownership boundaries.
- [x] Implement importer/upgrader mapping from compat/plugin evidence into native schemas.
- [x] Add migration diagnostics and safe fallback paths for unsupported constructs.

### 5. Validation and release readiness (remaining)

- [x] Add native-first test suites for each Wave 1 subsystem (unit plus integration anchors).
- [x] Maintain weekly compat regression while Wave 1 native ownership replaces plugin-shaped behavior.
- [x] Publish a release-readiness pass report proving gate stability and migration safety.

### 6. Wave 2 advanced capability baseline (remaining)

- [x] Gameplay Ability Framework delivered with tags, conditions, and state-machine-integrated execution.
- [x] Pattern Field Editor delivered for visual coordinate/pattern authoring.
- [x] Modular Level Assembly lane delivered for connector-based block workflows.
- [x] Sprite Pipeline Toolkit delivered (atlas/crop/preview baseline with runtime artifacts).
- [x] Procedural Content Toolkit delivered (generation + FOV baseline).
- [x] 2.5D presentation lane delivered behind explicit project-mode boundaries.
- [x] Timeline/Animation orchestration and transient effect events delivered.
- [x] Selected editor productivity utilities delivered and stabilized.

## Work complete, not remaining

- [x] Main/work branch hygiene reset and branch alignment
- [x] PR merge for native-first direction
- [x] Main branch protection policy
- [x] Dependabot branch churn disabled
- [x] Initial Wave 1 spec set and first implementation slices seeded
- [x] AI Copilot Native Infrastructure (Wave 2 Advanced):
  - [x] Knowledge Bridges (World, Battle, Audio, Animation, Debug).
  - [x] `ChatbotComponent` with Streaming and Tool-Calling support.
  - [x] Service interface scaffolding for OpenAI and Local Llama (live connectivity requires out-of-tree backend integration).
  - [x] NPC Personality Registry and prompt templating.
  - [x] `ChatWindow` UI with word-wrap and streaming logic.
  - [x] Unit/Regex test coverage for AI orchestration logic.
- [x] Wave 1 Schema & Migration Completion:
  - [x] Message/Text (schema, migration, unit coverage)
  - [x] Battle Core (schema, migration, unit coverage)
  - [x] Save/Data (schema, serialization/migration, differential saving)
  - [x] UI/Menu (schema, migration logic, unit coverage)
- [x] UI/Menu Runtime Ownership delivery complete:
  - [x] `MenuCommandRegistry` with native command storage and sorting.
  - [x] `MenuRouteResolver` for abstract command-to-action resolution.
  - [x] `MenuSceneGraph` command orchestration (Confirm/Cancel/Navigation) and audio sync.
  - [x] Cross-component unit coverage for menu orchestration.
- [x] Local build-environment hardening baseline:
  - [x] MinGW SDL resolution constrained to the active compiler root, with a safe vendored fallback when no compatible package is installed.
  - [x] Visual Studio SDL discovery no longer imports MSYS2 MinGW headers into MSVC projects.
  - [x] Focused presentation gate helper now reconfigures stale local build trees before running.
  - [x] `urpg_core` and `urpg_tests` build in both `dev-vs2022` and `dev-mingw-debug`, with the focused presentation gate passing locally on the Visual Studio lane.
- [x] Wave 2 opening slice: gameplay ability replay-safe diagnostics
- [x] Wave 2 opening slice: pattern validation and inspector preview
- [x] Wave 2 opening slice: modular level assembly validation
  - `PatternField` now normalizes point ordering, validates origin-presence/radius bounds, exposes reusable preset application, and surfaces preview snapshots through the model/panel path with focused `[pattern]` coverage.
  - `PatternFieldPresets` now exposes reusable categorized presets for skills, items, placement, and interaction masks, and `PatternFieldModel` can filter/apply them through a deterministic preset catalog.
  - `LevelAssemblyWorkspace` and `SnapLogic` now treat connector offsets as authored metadata, require registered blocks to attach through connector-backed neighbors after seeding, and reject offset-mismatched snaps with focused `[level][assembly]` coverage.
  - `LevelBlockLibrary` now provides deterministic block-library registration and ASCII thumbnail generation, while `LevelBlockImporter` preserves library names plus prefab paths for thumbnail-ready imported block catalogs.
- [x] Wave 2 opening slice: sprite pipeline runtime artifacts
  - `SpriteAnimator` now supports authored atlas clips in addition to legacy grid sheets, exposing normalized frame views from metadata-backed animations for focused runtime consumption tests.
  - `tools/sprite_pipeline` now emits animation and preview metadata (`preview_loop`, ordered frame IDs, frame count) so packed atlas JSON includes a deterministic runtime/preview contract, with focused `[sprite]` coverage.
  - `SpriteAnimationPreviewPanel` now exposes deterministic clip selection, playback stepping, and frame-duration/loop tuning from packed atlas metadata, with focused `[sprite][editor][panel]` coverage.
- [x] Wave 2 opening slice: procedural toolkit scenario generation
  - `ProceduralToolkit` now exposes a dedicated `GeneratedBlock` scenario output, selects deterministic seed openers, and expands connector-backed layouts while preferring continuation-capable blocks so seeded generation stays reproducible and budget-bounded.
  - `ProceduralToolkit::generateScenario()` now emits deterministic scenario/encounter bundles anchored to the seeded layout, and focused `[procedural][level]` coverage locks same-seed reproduction, different-seed divergence, encounter roles, and max-block budget behavior.
- [x] Wave 2 opening slice: timeline and animation orchestration
  - `AnimationSystem` now binds authored `AnimationClip` tracks into `AnimationComponent` playback and performs deterministic interpolation instead of stopping at the previous keyframe.
  - `TimelineKernel` now supports scene/UI track authoring (`ensureTrack`, `addEvent`, `updateEvent`, `removeEvent`), sorts track events on ingest, and records triggered transient events in playback order, while `AnimationKnowledgeBridge` normalizes parsed keyframe order so clip/timeline playback remains deterministic under test.
- [x] Wave 2 opening slice: 2.5D and editor utility gating
  - `RaycastRenderer` now requires an explicit spatial presentation-mode opt-in before the 2.5D lane runs, so classic 2D projects cannot drift into raycast behavior by accident.
  - `RaycastRenderer::buildAuthoringAdapter()` now converts authored `SpatialMapOverlay` elevation data into a deterministic raycast blocking grid, and `EditorUtilityTask` declares per-mode requirements so spatial-only utilities stay isolated from classic projects.
  - [x] `AbilitySystemComponent` now records bounded deterministic execution history for blocked, executed, and state-machine transition outcomes.
  - [x] `GameplayAbility` activation checks now return structured reasons instead of only boolean pass/fail state.
  - [x] `AbilityStateMachine` now records deterministic entered/failed/finished transition diagnostics through the shared ability execution history.
  - [x] `AbilityInspectorPanel` now exposes replay-log-oriented render snapshot data for the current ability runtime history.
  - [x] Focused verification: `.\build\Debug\urpg_tests.exe "[ability]" --reporter compact` => 49 assertions / 8 test cases passed.
