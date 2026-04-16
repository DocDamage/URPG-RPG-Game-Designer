# URPG Program Completion Status

Status Date: 2026-04-15  
Program Scope: native-first roadmap rewire plus Wave 1 absorption, Wave 2 advanced capability expansion, and compat exit hardening

## Where we are now

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
- Latest recorded local validation snapshot:
  - `ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure` => 289/289 passed
  - `ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure` => 42/42 passed

## Progress made in this cycle

- Merged native-first direction and follow-up execution commits into `main`.
- Finished repo governance cleanup:
  - removed extra remote branches
  - aligned work branch and main tip
  - disabled Dependabot PR branch churn in `.github/dependabot.yml`
  - enforced `main` branch protection policy
- Updated subsystem planning set:
  - `docs/UI_MENU_CORE_NATIVE_SPEC.md`
  - `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
  - `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
  - `docs/BATTLE_CORE_NATIVE_SPEC.md`
- Rewrote the primary roadmap into an integrated plan:
  - `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
  - includes Wave 2 advanced capability tracks (ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, editor utilities)
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
- **Added AI Copilot Core Native Ownership Slice (Wave 2 Advanced):**
  - `engine/core/message/chatbot_component.h`: Primary AI hub supporting streaming, tool-calling, and history management.
  - `engine/core/message/ai_sync_coordinator.h`: Cloud-ready history synchronization.
  - `engine/core/ai/ai_connectivity.h`: Production-ready service templates for OpenAI, Anthropic, and Local Llama.cpp.
  - `engine/core/ai/personality_registry.h`: Prompt templates for archetypal NPC behaviors (Elder, Warrior, Rogue, etc.).
  - `engine/core/audio/audio_ai_bridge.h`: Dynamic audio orchestration via AI commands.
  - `engine/core/animation/animation_ai_bridge.h`: NLP-to-keyframe translation for actor movement.
  - `engine/core/debug/debug_ai_bridge.h`: Runtime state and call-stack serialization for AI-driven debugging.
  - `engine/core/ui/chat_window.h`: Native UI supporting multi-line wrapping and real-time streaming text.
  - `tests/unit/test_ai_bridge_regex.cpp`: Unit-level validation for cross-component AI command parsing.
- Added Battle Core schema + migration completion slice:
  - schema contracts:
    - [content/schemas/battle_troops.schema.json](content/schemas/battle_troops.schema.json)
    - [content/schemas/battle_actions.schema.json](content/schemas/battle_actions.schema.json)
  - compat-to-native upgrader mapping:
    - [engine/core/battle/battle_migration.h](engine/core/battle/battle_migration.h)
    - [tests/unit/test_battle_migration.cpp](tests/unit/test_battle_migration.cpp)
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
- Latest focused validation snapshot for native UI/Menu lane:
  - `ctest --test-dir build/dev-ninja-debug -R "MenuSceneGraph|MenuRouteResolver|MenuCommandRegistry" --output-on-failure` => 15/15 passed
- Latest focused validation snapshot for Message/Text renderer integration lane:
  - `.\Debug\urpg_tests.exe "[compat]"` => 646 assertions / 140 test cases passed
  - `.\Debug\urpg_tests.exe "[compat][window]"` => 211 assertions / 52 test cases passed
  - `.\Debug\urpg_tests.exe "[compat][window][message]"` => 4 assertions / 1 test case passed
  - `.\Debug\urpg_tests.exe "[compat][window][snapshot]"` => 2 assertions / 1 test case passed

## Definition of 100% complete (for this program scope)

The scope in this document is considered 100% complete when all items below are done:

1. Compat lane is trustworthy as an import/validation bridge with explicit exit criteria satisfied.
2. Wave 1 subsystems have native runtime ownership and no longer rely on plugin-shaped core delivery.
3. Wave 1 subsystems have first-class editor workflows (inspect, preview, diagnose, migrate).
4. Native schemas and migration paths are production-ready for upgraded MZ projects.
5. Regression and release gates prove stability for both native and compat lanes.
6. Wave 2 advanced capability baseline is delivered at production quality.

## Next steps (current sprint)

1. Complete UI/Menu editor productization:
   - ship menu command authoring inspector and preview hooks wired to the new scene graph/runtime behavior.
2. Complete Message/Text renderer bridge closure:
   - consume backend `TextCommand` payloads end-to-end in renderer tiers where text draw remains placeholder,
   - align compat `Window_Message` behavior with native MessageScene runtime ownership handoff.
3. Finalize UI/Menu schema + migration mapping:
   - define import mapping from compat plugin menu evidence into native menu command metadata (including fallback routes and state rules).
4. Add integration coverage for UI/Menu runtime + editor:
   - scene-graph + resolver integration anchors beyond unit-level path checks.
5. Continue compat exit hardening:
   - keep new routed failure operations locked to JSONL/report/panel parity and maintain weekly conformance depth growth.
6. Publish explicit compat exit checklist artifact with import-confidence and migration-confidence pass criteria.

## Remaining work to reach 100%

### 1. Compat exit hardening (remaining)

- [ ] Expand routed conformance coverage beyond the current anchor scenarios across the curated 10-profile corpus.
- [ ] Keep every new failure operation locked to JSONL artifacts, report ingestion/export, and panel projection assertions.
- [ ] Complete explicit compat exit checklist with signed pass criteria for import confidence and migration confidence.

### 2. Wave 1 native runtime ownership (remaining)

- [ ] UI/Menu Core: complete production closure for command registry/scene graph/route resolver ownership (runtime slice landed; editor/schema/migration/release closure remains).
- [ ] Message/Text Core: complete production closure after landed flow/layout ownership (native MessageScene/UI renderer handoff, backend text command consumption, editor/schema/migration/release closure remains).
- [ ] Battle Core: implement native flow controller, action queue, and rule resolver ownership.
- [ ] Save/Data Core: complete catalog/serializer/recovery ownership beyond the seeded descriptor and inspector slice.

### 3. Wave 1 editor productization (remaining)

- [x] UI/Menu Core: Ship inspector and preview surfaces (in progress).
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
  - [x] Production service connectivity (OpenAI, Local Llama).
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
