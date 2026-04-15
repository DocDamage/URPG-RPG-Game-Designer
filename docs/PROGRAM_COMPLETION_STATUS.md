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
- Added Message/Text Core runtime ownership slice:
  - `engine/core/message/message_core.h`
  - `engine/core/message/message_core.cpp`
  - route-mode mapping for `speaker`/`narration`/`system`
  - native rich-text escape tokenization/layout and deterministic metrics
  - message flow runner with page advance + choice prompt state snapshot/restore
  - unit anchor coverage in `tests/unit/test_message_text_core.cpp`
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

## Definition of 100% complete (for this program scope)

The scope in this document is considered 100% complete when all items below are done:

1. Compat lane is trustworthy as an import/validation bridge with explicit exit criteria satisfied.
2. Wave 1 subsystems have native runtime ownership and no longer rely on plugin-shaped core delivery.
3. Wave 1 subsystems have first-class editor workflows (inspect, preview, diagnose, migrate).
4. Native schemas and migration paths are production-ready for upgraded MZ projects.
5. Regression and release gates prove stability for both native and compat lanes.
6. Wave 2 advanced capability baseline is delivered at production quality.

## Remaining work to reach 100%

### 1. Compat exit hardening (remaining)

- [ ] Expand routed conformance coverage beyond the current anchor scenarios across the curated 10-profile corpus.
- [ ] Keep every new failure operation locked to JSONL artifacts, report ingestion/export, and panel projection assertions.
- [ ] Complete explicit compat exit checklist with signed pass criteria for import confidence and migration confidence.

### 2. Wave 1 native runtime ownership (remaining)

- [ ] UI/Menu Core: implement native command registry, scene graph ownership, and route resolver.
- [ ] Message/Text Core: implement native flow runner and rich-text layout engine ownership.
- [ ] Battle Core: implement native flow controller, action queue, and rule resolver ownership.
- [ ] Save/Data Core: complete catalog/serializer/recovery ownership beyond the seeded descriptor and inspector slice.

### 3. Wave 1 editor productization (remaining)

- [ ] Ship inspectors for each Wave 1 subsystem with real authoring workflows.
- [ ] Ship preview surfaces for menu, message, and battle presentation contracts.
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
