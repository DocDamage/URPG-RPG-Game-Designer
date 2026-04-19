# URPG Program Completion Status

Status Date: 2026-04-19  
Program Scope: native-first roadmap rewire plus Wave 1 absorption, Wave 2 advanced capability expansion, and post-Phase-2 compat exit hardening

Cross-cutting debt, truthfulness, and intake-governance source of truth: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

Canonical planning chain:
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` governs cross-cutting truthfulness, reconciliation, and Definition-of-Done requirements.
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` is the canonical product roadmap.
- `docs/PROGRAM_COMPLETION_STATUS.md` is the canonical latest-status snapshot.
- `docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`, `docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md`, and `docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md` are detailed planning inputs retained for traceability until their remaining deltas are absorbed into the canonical docs above.

Phase 2 runtime closure is complete as of 2026-04-19. Remaining compat work in this document is post-closure exit hardening, corpus depth, and ongoing truth-maintenance work, not unfinished baseline closure of the audited Phase 2 runtime slice.

Phase 3 diagnostics productization is complete as of 2026-04-19, Phase 4 governance/reconciliation closure is complete as of 2026-04-19, and Phase 5 hardening closure is also complete as of 2026-04-19. The canonical remediation findings table is now fully green. Remaining work in this document is post-closure compat hardening depth, roadmap execution, and ongoing truth-maintenance work rather than an unfinished remediation phase.

## Where we are now

- Phase 2 runtime closure is complete:
  - battle reward/event cadence and switch coverage are closed in the compat lane
  - `DataManager::loadDatabase()` seeded-container behavior is explicitly exercised
  - `Window_Base::contents()` lifecycle truthfulness is explicitly exercised
  - AudioManager remains honestly `PARTIAL` as a deterministic harness-backed surface with live compat-state observability
- Phase 3 diagnostics surfaces are complete:
  - audio diagnostics project live `AudioCore` state and now expose selected-row navigation through the workspace export surface
  - migration wizard diagnostics expose rendered workflow actions, selected-result detail, issue-focused navigation, report save/load, and bound-runtime rerun flows
  - event-authority, menu, message, battle, and ability diagnostics remain snapshot/export truthful alongside their current workflow surfaces
- Presentation planning is now aligned around **Phase 5 â€” Environment & Presentation Polish**:
  - **Incubating:** `editor/spatial/*` panels (`ElevationBrushPanel`, `PropPlacementPanel`) are header-only with no compiled `.cpp` sources registered in the build graph.
  - **Incubating:** `engine/core/presentation/*` is predominantly header-only abstraction; only `presentation_runtime.cpp` and `release_validation.cpp` are actively compiled. `profile_arena.cpp` exists but is not registered in `urpg_core`.
  - `test_spatial_editor.cpp` and `test_presentation_runtime.cpp` are registered and passing, anchoring the future productization path.
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
- Phase 5 hardening closure is complete:
  - async plugin callbacks are now deferred, FIFO, owning-thread-only, and covered by focused queue-integrity and stale-error-clearing regressions
  - `MapScene` audio ownership is now explicit and observable instead of relying on a constructor-created service instance
  - retained tile render commands are now explicitly documented and verified as pointer-stable across unchanged frames
  - the Phase 4 governance gate now enforces wrapper/facade-only production-candidate adoption plus provenance-preserving asset-promotion records
- **Planning Governance:** standalone PGMMV/native-absorption roadmap files are now treated as reference annexes under [`docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`](./TECHNICAL_DEBT_REMEDIATION_PLAN.md) and indexed in [`docs/archive/README.md`](./archive/README.md), not parallel execution authorities.

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
  - native shell loop and scene scaffolding
  - compat global-state bridges for battle and data-manager flows
  - save slot descriptor loading and save-inspector slot-label projection
  - tactical routed battle fixture coverage across plugin reload
  - WindowCompat text bridge now submits renderer-facing `RenderLayer::TextCommand` payloads from `Window_Base::drawText`
  - compat `Window_Message` surface landed for dialogue-body alignment parity (`left`/`center`/`right`)
  - snapshot-style wrapped centered/right `drawTextEx` draw-history coverage landed
  - `Window_Selectable` pointer semantics now cover press/drag/release hit-testing, drag retargeting, drag-scroll, and mouse-wheel scrolling through `InputManager`
  - `Window_Base::contents()` now allocates compat bitmap records with tracked dimensions that stay synchronized with rect/padding changes instead of exposing handle-only state
  - compat status truth pass is now reflected in the canonical docs; remaining work is to keep those labels and docs aligned as post-closure hardening continues
  - AudioManager compat closure advanced: deterministic playback position now advances during `update()`, duck/unduck ramps are frame-based, master/bus volume changes affect active playback, and the QuickJS `AudioManager` bridge now routes live compat state for BGM/BGS/ME/SE plus volume/ducking helpers
  - 2026-04-19 Phase 2 runtime closure reconciled the battle reward/event and switch coverage, window `contents()` lifecycle truthfulness, `DataManager::loadDatabase()` seeded-container behavior, and audio semantics documentation against the focused dev-mingw-debug verification lane
  - 2026-04-19 Phase 4 governance closure reconciled external-repo dispositions, private-asset source manifests, asset-gap truthfulness, and canonical status docs, and added a local validation gate for the intake-governance lane
- Latest recorded local validation snapshot:
  - recorded under the `dev-ninja-debug` preset: `ctest --preset dev-all --output-on-failure` => 569/569 passed
- Latest focused Phase 5 hardening validation snapshot:
  - `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PluginManager: Command execution|MapScene:|SceneManager:"` => 8/8 passed
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1` => passed
- Latest focused presentation validation snapshot:
  - `ctest --test-dir build/dev-mingw-debug -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure` => 3/3 passed
  - includes the dedicated `[presentation]` unit lane, the standalone release-validation harness, and the spatial editor authoring lane
  - CI `gate1-pr` now invokes the focused presentation gate explicitly via `tools/ci/run_presentation_gate.ps1` after the shared build step
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
  - includes Wave 2 advanced capability tracks (ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, editor utilities)
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
  - unit coverage in `tests/unit/test_battle_core.cpp`
- Added Battle Core editor inspector + preview diagnostics slice:
  - `editor/battle/battle_inspector_model.h`
  - `editor/battle/battle_inspector_model.cpp`
  - `editor/battle/battle_preview_panel.h`
  - `editor/battle/battle_preview_panel.cpp`
  - `editor/battle/battle_inspector_panel.h`
  - `editor/battle/battle_inspector_panel.cpp`
  - diagnostics workspace `battle` tab wiring
  - unit coverage in:
    - `tests/unit/test_battle_inspector_model.cpp`
    - `tests/unit/test_battle_preview_panel.cpp`
    - `tests/unit/test_battle_inspector_panel.cpp`
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
  - unit coverage in `tests/unit/test_battle_migration.cpp` and `tests/unit/test_battle_core.cpp`
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
2. Complete Message/Text renderer bridge closure:
   - consume backend `TextCommand` payloads end-to-end in renderer tiers where text draw remains placeholder,
   - align compat `Window_Message` behavior with native MessageScene runtime ownership handoff.
3. Finalize UI/Menu schema + migration mapping:
   - extend the now-landed fallback/state-rule import coverage from direct menu metadata into broader compat plugin evidence paths.
4. Add integration coverage for UI/Menu runtime + editor:
   - extend beyond the landed diagnostics-workspace/menu-import anchors into broader scene-graph + resolver parity checks.
5. Continue post-Phase-2 compat exit hardening:
   - keep new routed failure operations locked to JSONL/report/panel parity and maintain weekly conformance depth growth.
6. Keep canonical status/docs aligned with current validation evidence:
   - update README/remediation/kickoff/checklist language whenever closure state or residual compat scope changes.

## Remaining work to reach 100%

### 1. Compat exit hardening (post-Phase-2 remaining work)

Phase 2 runtime closure is already complete. The remaining compat work below is about confidence depth, ongoing evidence, and truthful residual scoping.

- [ ] Expand routed conformance coverage beyond the current anchor scenarios across the curated 10-profile corpus.
- [ ] Keep every new failure operation locked to JSONL artifacts, report ingestion/export, and panel projection assertions.
- [x] Complete explicit compat exit checklist with signed pass criteria for import confidence and migration confidence.
- [x] Keep runtime status labels and public docs aligned with actual implementation scope; do not relabel fixture-backed or placeholder-backed paths as `FULL` without closing the underlying TODOs.

### 2. Wave 1 native runtime ownership (remaining)

- [x] UI/Menu Core: complete production closure for command registry/scene graph/route resolver ownership (runtime slice landed; editor/schema/migration/release closure remains).
- [ ] Message/Text Core: complete production closure after landed flow/layout ownership (native MessageScene/UI renderer handoff, backend text command consumption, editor/schema/migration/release closure remains).
- [ ] Battle Core: implement native flow controller, action queue, and rule resolver ownership.
- [ ] Save/Data Core: complete catalog/serializer/recovery ownership beyond the seeded descriptor and inspector slice.

### 3. Wave 1 editor productization (remaining)

- [x] UI/Menu Core: inspector and preview surfaces are shipped; remaining work is deeper authoring/productization beyond the landed diagnostics workflow.
- [ ] Message/Text Core: Ship inspector and preview surfaces.
- [ ] Battle Core: Ship preview surfaces.
- [ ] Ship diagnostics and validation wiring directly in editor panels for Wave 1 schemas.

### 4. Schema and migration completion (remaining)

- [ ] Finalize Wave 1 schema contracts for runtime and editor ownership boundaries.
- [ ] Implement importer/upgrader mapping from compat/plugin evidence into native schemas.
- [ ] Add migration diagnostics and safe fallback paths for unsupported constructs.

### 5. Validation and release readiness (remaining)

- [ ] Add native-first test suites for each Wave 1 subsystem (unit plus integration anchors).
- [ ] Maintain weekly compat regression while Wave 1 native ownership replaces plugin-shaped behavior.
- [ ] Publish a release-readiness pass report proving gate stability and migration safety.

### 6. Wave 2 advanced capability baseline (remaining)

- [ ] Gameplay Ability Framework delivered with tags, conditions, and state-machine-integrated execution.
- [ ] Pattern Field Editor delivered for visual coordinate/pattern authoring.
- [ ] Modular Level Assembly lane delivered for connector-based block workflows.
- [ ] Sprite Pipeline Toolkit delivered (atlas/crop/preview).
- [ ] Procedural Content Toolkit delivered (generation + FOV baseline).
- [ ] Optional 2.5D presentation lane delivered behind explicit project-mode boundaries.
- [ ] Timeline/Animation orchestration and transient effect events delivered.
- [ ] Selected editor productivity utilities delivered and stabilized.

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
