# Program Backlog Strict Execution Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Execute the entire remaining post-remediation backlog in one strict order, with file-level starting points, exact verification commands, and commit boundaries.

**Architecture:** Finish Wave 1 as complete verticals before opening broad new work. The order is: close UI/Menu end-to-end, then Message/Text, then Save/Data, then Battle, then prove release-readiness, then start Wave 2 in the order already implied by the canonical roadmap. Do not start a later lane early unless the listed gate for the previous lane is green.

**Tech Stack:** C++17, Catch2, CMake/CTest, PowerShell, JSON Schema, markdown status/governance docs

---

## File Structure

- `docs/PROGRAM_COMPLETION_STATUS.md`
  Canonical latest-status snapshot for the remaining backlog and current sprint order.
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
  Canonical product roadmap for Wave 1 and Wave 2 ordering.
- `docs/UI_MENU_CORE_NATIVE_SPEC.md`
  UI/Menu closure requirements and authoring/runtime expectations.
- `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
  Message/Text closure requirements.
- `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
  Save/Data closure requirements.
- `docs/BATTLE_CORE_NATIVE_SPEC.md`
  Battle closure requirements.
- `editor/ui/menu_inspector_model.h`
- `editor/ui/menu_inspector_model.cpp`
- `editor/ui/menu_inspector_panel.h`
- `editor/ui/menu_inspector_panel.cpp`
- `editor/ui/menu_preview_panel.h`
- `editor/ui/menu_preview_panel.cpp`
  Primary UI/Menu editor productization files.
- `engine/core/ui/menu_command_registry.h`
- `engine/core/ui/menu_route_resolver.h`
- `engine/core/ui/menu_scene_graph.h`
- `engine/core/ui/menu_serializer.h`
- `engine/core/ui/menu_serializer.cpp`
- `engine/core/ui/menu_migration.h`
  Primary UI/Menu runtime, schema, and migration files.
- `editor/diagnostics/diagnostics_workspace.h`
- `editor/diagnostics/diagnostics_workspace.cpp`
  Workspace-level integration surface for menu, message, save, and battle diagnostics.
- `editor/message/message_inspector_model.h`
- `editor/message/message_inspector_model.cpp`
- `editor/message/message_inspector_panel.h`
- `editor/message/message_inspector_panel.cpp`
  Existing Message/Text editor surface that needs productization closure.
- `engine/core/message/message_core.h`
- `engine/core/message/message_core.cpp`
- `engine/core/message/message_migration.h`
- `engine/core/message/message_migration.cpp`
- `engine/core/message/dialogue_serializer.h`
- `engine/core/message/dialogue_registry.cpp`
  Primary Message/Text runtime and migration files.
- `engine/core/scene/map_scene.h`
- `engine/core/scene/map_scene.cpp`
  Current in-game message handoff point and save/load integration point.
- `editor/save/save_inspector_model.h`
- `editor/save/save_inspector_model.cpp`
- `editor/save/save_inspector_panel.h`
- `editor/save/save_inspector_panel.cpp`
  Current Save/Data editor surface.
- `engine/core/save/save_catalog.h`
- `engine/core/save/save_catalog.cpp`
- `engine/core/save/save_serialization_hub.h`
- `engine/core/save/save_serialization_hub.cpp`
- `engine/core/save/save_runtime.h`
- `engine/core/save/save_runtime.cpp`
  Primary Save/Data ownership files.
- `engine/core/battle/battle_core.h`
- `engine/core/battle/battle_core.cpp`
- `engine/core/battle/battle_migration.h`
- `engine/core/battle/battle_migration.cpp`
  Primary Battle runtime and migration files.
- `editor/battle/battle_preview_panel.h`
- `editor/battle/battle_preview_panel.cpp`
- `editor/battle/battle_inspector_panel.h`
- `editor/battle/battle_inspector_panel.cpp`
  Battle editor and preview surface.
- `engine/core/scene/battle_scene.h`
- `engine/core/scene/battle_scene.cpp`
  Current scene-level battle integration path that still needs convergence with native battle ownership.
- `engine/core/presentation/release_validation.cpp`
- `RELEASE_CHECKLIST.md`
- `tools/ci/`
  Release-readiness and gate proof files.
- `engine/core/ability/gameplay_ability.h`
- `engine/core/ability/ability_system_component.h`
- `engine/core/ability/ability_state_machine.h`
- `editor/ability/ability_inspector_panel.h`
  Wave 2 gameplay ability anchor files.
- `engine/core/ability/pattern_field.h`
- `engine/core/ability/pattern_field.cpp`
- `engine/core/ability/pattern_field_serializer.h`
- `editor/ability/pattern_field_model.h`
- `editor/ability/pattern_field_model.cpp`
- `editor/ability/pattern_field_panel.h`
- `editor/ability/pattern_field_panel.cpp`
  Wave 2 pattern editor anchor files.
- `engine/core/level/level_assembly.h`
- `engine/core/level/level_assembly.cpp`
- `tests/unit/test_level_assembly.cpp`
  Wave 2 modular level assembly anchor files.
- `content/schemas/sprite_atlas.schema.json`
- `engine/core/render/sprite_animator.h`
- `engine/core/render/sprite_animator.cpp`
  Wave 2 sprite pipeline anchor files.
- `engine/core/level/procedural_toolkit.h`
  Wave 2 procedural toolkit anchor file.
- `engine/core/animation/animation_clip.h`
- `engine/core/animation/animation_system.h`
- `engine/core/animation/timeline_kernel.h`
- `tests/unit/test_animation_system.cpp`
  Wave 2 timeline/animation anchor files.
- `editor/productivity/editor_utility_task.h`
  Wave 2 editor utility anchor file.

## Global Rules

- Finish tasks in order. Do not begin Task 4 before Task 3 is green and committed.
- Use failing-test-first for each lane.
- Keep `docs/PROGRAM_COMPLETION_STATUS.md`, `README.md`, `WORKLOG.md`, and `RELEASE_CHECKLIST.md` aligned whenever a lane changes closure state.
- Re-run the lane-local tests before each lane commit.
- Re-run `ctest --preset dev-all --output-on-failure` before any release-readiness claim.

## Phase A: Wave 1 Full Closure

### Task 1: Finish UI/Menu Editor Productization

**Starting files:**
- Modify: `editor/ui/menu_inspector_model.h`
- Modify: `editor/ui/menu_inspector_model.cpp`
- Modify: `editor/ui/menu_inspector_panel.h`
- Modify: `editor/ui/menu_inspector_panel.cpp`
- Modify: `editor/ui/menu_preview_panel.h`
- Modify: `editor/ui/menu_preview_panel.cpp`
- Modify: `editor/diagnostics/diagnostics_workspace.h`
- Modify: `editor/diagnostics/diagnostics_workspace.cpp`
- Test: `tests/unit/test_menu_inspector_model.cpp`
- Test: `tests/unit/test_menu_preview_panel.cpp`
- Test: `tests/unit/test_diagnostics_workspace.cpp`

- [ ] Review the current UI/Menu scope and confirm the remaining gaps against the canonical docs.

Run:

```powershell
Get-Content docs/UI_MENU_CORE_NATIVE_SPEC.md
Get-Content docs/PROGRAM_COMPLETION_STATUS.md
```

- [ ] Add or extend failing tests for true edit, preview, and diagnostics workflow behavior.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MenuInspectorModel|MenuPreviewPanel|DiagnosticsWorkspace - .*menu"
```

Expected: at least one failing or missing case before implementation.

- [ ] Implement the missing editor productization behavior in the menu inspector, preview panel, and workspace wiring.

- [ ] Re-run the UI/Menu editor lane.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MenuInspectorModel|MenuPreviewPanel|DiagnosticsWorkspace - .*menu"
```

Expected: PASS.

- [ ] Commit UI/Menu editor productization.

```bash
git add editor/ui/menu_inspector_model.h editor/ui/menu_inspector_model.cpp editor/ui/menu_inspector_panel.h editor/ui/menu_inspector_panel.cpp editor/ui/menu_preview_panel.h editor/ui/menu_preview_panel.cpp editor/diagnostics/diagnostics_workspace.h editor/diagnostics/diagnostics_workspace.cpp tests/unit/test_menu_inspector_model.cpp tests/unit/test_menu_preview_panel.cpp tests/unit/test_diagnostics_workspace.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: finish menu editor productization"
```

**Gate to continue:** menu editing, preview, and diagnostics workflow tests are green.

### Task 2: Finalize UI/Menu Schema and Migration Closure

**Starting files:**
- Modify: `engine/core/ui/menu_serializer.h`
- Modify: `engine/core/ui/menu_serializer.cpp`
- Modify: `engine/core/ui/menu_migration.h`
- Modify: `content/schemas/menu_scene_graph.schema.json`
- Modify: `content/schemas/menu_commands.schema.json`
- Test: `tests/unit/test_menu_legacy_import.cpp`
- Test: `tests/unit/test_menu_migration_logic.cpp`
- Test: `tests/unit/test_migration_wizard.cpp`

- [ ] Inspect the existing menu schema and migration coverage.

Run:

```powershell
Get-Content content/schemas/menu_scene_graph.schema.json
Get-Content content/schemas/menu_commands.schema.json
```

- [ ] Add failing tests for any remaining unsupported compat-to-native mappings and fallback diagnostics.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MenuSceneSerializer:|menu_migration|MigrationWizard.*menu"
```

- [ ] Implement the missing schema and migration behavior.

- [ ] Re-run the UI/Menu migration lane.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MenuSceneSerializer:|menu_migration|MigrationWizard.*menu"
```

Expected: PASS.

- [ ] Commit UI/Menu schema and migration closure.

```bash
git add engine/core/ui/menu_serializer.h engine/core/ui/menu_serializer.cpp engine/core/ui/menu_migration.h content/schemas/menu_scene_graph.schema.json content/schemas/menu_commands.schema.json tests/unit/test_menu_legacy_import.cpp tests/unit/test_menu_migration_logic.cpp tests/unit/test_migration_wizard.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: close menu schema and migration gaps"
```

**Gate to continue:** menu schemas are stable and migration diagnostics are covered.

### Task 3: Lock UI/Menu Runtime-to-Editor Integration Parity

**Starting files:**
- Modify: `engine/core/ui/menu_command_registry.h`
- Modify: `engine/core/ui/menu_route_resolver.h`
- Modify: `engine/core/ui/menu_scene_graph.h`
- Modify: `editor/diagnostics/diagnostics_workspace.cpp`
- Test: `tests/unit/test_menu_core.cpp`
- Test: `tests/unit/test_menu_orchestration.cpp`
- Test: `tests/unit/test_diagnostics_workspace.cpp`

- [ ] Add failing parity tests from authored menu data through runtime route resolution and workspace export.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "test_menu_core|test_menu_orchestration|DiagnosticsWorkspace - .*menu"
```

- [ ] Implement missing parity behavior in the command registry, route resolver, scene graph, or workspace export.

- [ ] Re-run the full UI/Menu lane.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[ui][menu]"
.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration]"
```

Expected: PASS on both commands.

- [ ] Commit UI/Menu end-to-end closure.

```bash
git add engine/core/ui/menu_command_registry.h engine/core/ui/menu_route_resolver.h engine/core/ui/menu_scene_graph.h editor/diagnostics/diagnostics_workspace.cpp tests/unit/test_menu_core.cpp tests/unit/test_menu_orchestration.cpp tests/unit/test_diagnostics_workspace.cpp docs/PROGRAM_COMPLETION_STATUS.md RELEASE_CHECKLIST.md WORKLOG.md
git commit -m "feat: close menu runtime editor parity"
```

**Gate to continue:** UI/Menu is complete across runtime, editor, schema, migration, diagnostics, and tests.

### Task 4: Finish Message/Text Runtime and Renderer Closure

**Starting files:**
- Modify: `engine/core/message/message_core.h`
- Modify: `engine/core/message/message_core.cpp`
- Modify: `engine/core/scene/map_scene.h`
- Modify: `engine/core/scene/map_scene.cpp`
- Modify: `engine/core/platform/opengl_renderer.cpp`
- Modify: `engine/core/render/render_layer.h`
- Test: `tests/unit/test_message_text_core.cpp`
- Test: `tests/unit/test_window_compat.cpp`
- Test: `tests/unit/test_scene_manager.cpp`

- [ ] Inspect the current handoff between `MessageFlowRunner`, `MapScene`, `Window_Message`, and renderer text commands.

Run:

```powershell
rg -n "MessageFlowRunner|Window_Message|drawText|TextCommand|messageRunner|startDialogue" engine/core tests/unit -S
```

- [ ] Add failing tests for native message runtime ownership and renderer consumption where placeholder behavior still exists.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "message_text_core|Window_Message|MapScene:"
```

- [ ] Implement the missing runtime and renderer behavior.

- [ ] Re-run the focused Message/Text runtime lane.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[message]"
.\build\dev-mingw-debug\urpg_tests.exe "[compat][window][message]"
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MapScene:"
```

Expected: PASS.

- [ ] Commit Message/Text runtime closure.

```bash
git add engine/core/message/message_core.h engine/core/message/message_core.cpp engine/core/scene/map_scene.h engine/core/scene/map_scene.cpp engine/core/platform/opengl_renderer.cpp engine/core/render/render_layer.h tests/unit/test_message_text_core.cpp tests/unit/test_window_compat.cpp tests/unit/test_scene_manager.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: close message runtime renderer handoff"
```

**Gate to continue:** in-game message flow is native-owned and renderer-consumed end-to-end.

### Task 5: Finish Message/Text Editor and Migration Productization

**Starting files:**
- Modify: `editor/message/message_inspector_model.h`
- Modify: `editor/message/message_inspector_model.cpp`
- Modify: `editor/message/message_inspector_panel.h`
- Modify: `editor/message/message_inspector_panel.cpp`
- Modify: `engine/core/message/message_migration.h`
- Modify: `engine/core/message/message_migration.cpp`
- Modify: `editor/diagnostics/diagnostics_workspace.cpp`
- Test: `tests/unit/test_message_inspector_model.cpp`
- Test: `tests/unit/test_message_inspector_panel.cpp`
- Test: `tests/unit/test_message_migration.cpp`
- Test: `tests/unit/test_diagnostics_workspace.cpp`
- Test: `tests/unit/test_migration_wizard.cpp`

- [ ] Add failing tests for message inspector workflows, diagnostics projection, and migration edge cases still missing.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MessageInspector|message_migration|DiagnosticsWorkspace - .*message|MigrationWizard.*message"
```

- [ ] Implement the missing editor and migration behavior.

- [ ] Re-run the focused Message/Text productization lane.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[message][editor]"
.\build\dev-mingw-debug\urpg_tests.exe "[message][migration]"
.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration][message_actions]"
```

Expected: PASS.

- [ ] Commit Message/Text editor productization.

```bash
git add editor/message/message_inspector_model.h editor/message/message_inspector_model.cpp editor/message/message_inspector_panel.h editor/message/message_inspector_panel.cpp engine/core/message/message_migration.h engine/core/message/message_migration.cpp editor/diagnostics/diagnostics_workspace.cpp tests/unit/test_message_inspector_model.cpp tests/unit/test_message_inspector_panel.cpp tests/unit/test_message_migration.cpp tests/unit/test_diagnostics_workspace.cpp tests/unit/test_migration_wizard.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: finish message editor and migration workflows"
```

**Gate to continue:** Message/Text is complete across runtime, editor, schema, migration, diagnostics, and tests.

### Task 6: Finish Save/Data Native Ownership

**Starting files:**
- Modify: `engine/core/save/save_catalog.h`
- Modify: `engine/core/save/save_catalog.cpp`
- Modify: `engine/core/save/save_serialization_hub.h`
- Modify: `engine/core/save/save_serialization_hub.cpp`
- Modify: `engine/core/save/save_runtime.h`
- Modify: `engine/core/save/save_runtime.cpp`
- Modify: `editor/save/save_inspector_model.h`
- Modify: `editor/save/save_inspector_model.cpp`
- Modify: `editor/save/save_inspector_panel.h`
- Modify: `editor/save/save_inspector_panel.cpp`
- Modify: `editor/diagnostics/diagnostics_workspace.cpp`
- Test: `tests/unit/test_save_catalog.cpp`
- Test: `tests/unit/test_data_manager.cpp`
- Test: `tests/unit/test_diagnostics_workspace.cpp`

- [ ] Inspect the remaining Save/Data ownership and recovery gaps.

Run:

```powershell
rg -n "SaveCatalog|SaveSerializationHub|RuntimeSaveLoader|SaveSessionCoordinator|recovery" engine/core editor tests/unit -S
```

- [ ] Add failing tests for serializer/recovery ownership, catalog truth, and editor projection gaps.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "save_catalog|data_manager|DiagnosticsWorkspace - .*save"
```

- [ ] Implement the missing native ownership behavior.

- [ ] Re-run the Save/Data lane.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[save]"
.\build\dev-mingw-debug\urpg_tests.exe "[data_manager]"
.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration]"
```

Expected: PASS.

- [ ] Commit Save/Data closure.

```bash
git add engine/core/save/save_catalog.h engine/core/save/save_catalog.cpp engine/core/save/save_serialization_hub.h engine/core/save/save_serialization_hub.cpp engine/core/save/save_runtime.h engine/core/save/save_runtime.cpp editor/save/save_inspector_model.h editor/save/save_inspector_model.cpp editor/save/save_inspector_panel.h editor/save/save_inspector_panel.cpp editor/diagnostics/diagnostics_workspace.cpp tests/unit/test_save_catalog.cpp tests/unit/test_data_manager.cpp tests/unit/test_diagnostics_workspace.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: close save data native ownership"
```

**Gate to continue:** Save/Data is complete across runtime, editor, diagnostics, and recovery behavior.

### Task 7: Finish Battle Native Runtime Ownership

**Starting files:**
- Modify: `engine/core/battle/battle_core.h`
- Modify: `engine/core/battle/battle_core.cpp`
- Modify: `engine/core/scene/battle_scene.h`
- Modify: `engine/core/scene/battle_scene.cpp`
- Modify: `engine/core/battle/battle_migration.h`
- Modify: `engine/core/battle/battle_migration.cpp`
- Test: `tests/unit/test_battle_core.cpp`
- Test: `tests/unit/test_scene_manager.cpp`
- Test: `tests/unit/test_battle_migration.cpp`

- [ ] Inspect current divergence between native battle core and `BattleScene`.

Run:

```powershell
rg -n "BattleFlowController|BattleActionQueue|BattleRuleResolver|BattleScene|compat::BattleManager" engine/core tests/unit -S
```

- [ ] Add failing tests for the missing convergence points.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "battle_core|BattleScene:|battle_migration"
```

- [ ] Implement the missing native runtime ownership behavior.

- [ ] Re-run the Battle runtime lane.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[battle][core]"
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleScene:"
.\build\dev-mingw-debug\urpg_tests.exe "[battle][migration]"
```

Expected: PASS.

- [ ] Commit Battle runtime closure.

```bash
git add engine/core/battle/battle_core.h engine/core/battle/battle_core.cpp engine/core/scene/battle_scene.h engine/core/scene/battle_scene.cpp engine/core/battle/battle_migration.h engine/core/battle/battle_migration.cpp tests/unit/test_battle_core.cpp tests/unit/test_scene_manager.cpp tests/unit/test_battle_migration.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: close battle runtime ownership"
```

**Gate to continue:** battle flow, queue, rules, scene integration, and migration are green.

### Task 8: Finish Battle Editor and Diagnostics Productization

**Starting files:**
- Modify: `editor/battle/battle_preview_panel.h`
- Modify: `editor/battle/battle_preview_panel.cpp`
- Modify: `editor/battle/battle_inspector_panel.h`
- Modify: `editor/battle/battle_inspector_panel.cpp`
- Modify: `editor/diagnostics/diagnostics_workspace.cpp`
- Test: `tests/unit/test_battle_preview_panel.cpp`
- Test: `tests/unit/test_battle_inspector_panel.cpp`
- Test: `tests/unit/test_diagnostics_workspace.cpp`

- [ ] Add failing tests for battle preview fidelity, inspector workflow behavior, and workspace export parity.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "battle_preview_panel|battle_inspector_panel|DiagnosticsWorkspace - .*battle"
```

- [ ] Implement the missing battle editor productization behavior.

- [ ] Re-run the battle editor lane.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[battle][editor]"
.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration]"
```

Expected: PASS.

- [ ] Commit Battle editor closure.

```bash
git add editor/battle/battle_preview_panel.h editor/battle/battle_preview_panel.cpp editor/battle/battle_inspector_panel.h editor/battle/battle_inspector_panel.cpp editor/diagnostics/diagnostics_workspace.cpp tests/unit/test_battle_preview_panel.cpp tests/unit/test_battle_inspector_panel.cpp tests/unit/test_diagnostics_workspace.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: finish battle editor workflows"
```

**Gate to continue:** Battle is complete across runtime, editor, schema, migration, diagnostics, and tests.

## Phase B: Compat Hardening and Release Proof

### Task 9: Expand Compat Exit Hardening Evidence

**Starting files:**
- Modify: `docs/COMPAT_EXIT_CHECKLIST.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `RELEASE_CHECKLIST.md`
- Modify: compat diagnostics tests under `tests/compat/`
- Modify: `tests/unit/test_diagnostics_workspace.cpp`

- [ ] Add any missing routed conformance, JSONL/report/panel parity, and truth-maintenance assertions.

Run:

```powershell
rg -n "JSONL|report|panel|compat_failure|curated" tests/compat tests/unit -S
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "compat|DiagnosticsWorkspace"
```

- [ ] Implement or extend the missing compat hardening coverage.

- [ ] Re-run the focused compat hardening lane.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "compat"
.\build\dev-mingw-debug\urpg_tests.exe "[editor][diagnostics][integration]"
```

Expected: PASS.

- [ ] Commit compat hardening evidence.

```bash
git add docs/COMPAT_EXIT_CHECKLIST.md docs/PROGRAM_COMPLETION_STATUS.md RELEASE_CHECKLIST.md tests/compat tests/unit/test_diagnostics_workspace.cpp WORKLOG.md
git commit -m "test: expand compat exit hardening evidence"
```

**Gate to continue:** compat is still truthful and fully evidenced as an import/migration bridge.

### Task 10: Publish Release-Readiness Proof

**Starting files:**
- Modify: `engine/core/presentation/release_validation.cpp`
- Modify: `RELEASE_CHECKLIST.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `README.md`
- Modify: `WORKLOG.md`
- Modify: `tools/ci/` scripts if a missing lane gate exists

- [ ] Reconcile all canonical status docs to the actually completed Wave 1 state.

- [ ] Run the full validation stack.

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PluginManager: Command execution|MapScene:|SceneManager:"
ctest --test-dir build/dev-mingw-debug -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1
ctest --preset dev-all --output-on-failure
```

Expected: all commands pass.

- [ ] Commit release-readiness closure.

```bash
git add engine/core/presentation/release_validation.cpp RELEASE_CHECKLIST.md docs/PROGRAM_COMPLETION_STATUS.md README.md WORKLOG.md tools/ci
git commit -m "docs: publish wave 1 release readiness"
```

**Gate to continue:** Wave 1 is fully closed and release-defensible.

## Phase C: Wave 2 Strict Opening Order

### Task 11: Gameplay Ability Replay-Safe Diagnostics

**Starting files:**
- Modify: `engine/core/ability/gameplay_ability.h`
- Modify: `engine/core/ability/gameplay_ability.cpp`
- Modify: `engine/core/ability/ability_system_component.h`
- Modify: `engine/core/ability/ability_state_machine.h`
- Modify: `editor/ability/ability_inspector_panel.h`
- Modify: `editor/ability/ability_inspector_panel.cpp`
- Test: `tests/unit/test_ability_activation.cpp`

- [ ] Add failing tests for replay-safe diagnostics and deterministic ability/effect execution.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[ability]"
```

- [ ] Implement the missing diagnostics and replay-safety behavior.

- [ ] Re-run the ability lane and commit.

```bash
git add engine/core/ability/gameplay_ability.h engine/core/ability/gameplay_ability.cpp engine/core/ability/ability_system_component.h engine/core/ability/ability_state_machine.h editor/ability/ability_inspector_panel.h editor/ability/ability_inspector_panel.cpp tests/unit/test_ability_activation.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: harden gameplay ability diagnostics"
```

### Task 12: Pattern Field Validation and Inspector Preview

**Starting files:**
- Modify: `engine/core/ability/pattern_field.h`
- Modify: `engine/core/ability/pattern_field.cpp`
- Modify: `engine/core/ability/pattern_field_serializer.h`
- Modify: `editor/ability/pattern_field_model.h`
- Modify: `editor/ability/pattern_field_model.cpp`
- Modify: `editor/ability/pattern_field_panel.h`
- Modify: `editor/ability/pattern_field_panel.cpp`
- Modify: `content/schemas/pattern_field.schema.json`
- Test: `tests/unit/test_pattern_field.cpp`

- [ ] Add failing tests for validation, presets, and inspector preview behavior.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[pattern]"
```

- [ ] Implement the missing pattern validation and preview workflows.

- [ ] Re-run and commit.

```bash
git add engine/core/ability/pattern_field.h engine/core/ability/pattern_field.cpp engine/core/ability/pattern_field_serializer.h editor/ability/pattern_field_model.h editor/ability/pattern_field_model.cpp editor/ability/pattern_field_panel.h editor/ability/pattern_field_panel.cpp content/schemas/pattern_field.schema.json tests/unit/test_pattern_field.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: finish pattern field validation workflows"
```

### Task 13: Modular Level Assembly Productization

**Starting files:**
- Modify: `engine/core/level/level_assembly.h`
- Modify: `engine/core/level/level_assembly.cpp`
- Test: `tests/unit/test_level_assembly.cpp`

- [ ] Add failing tests for deterministic placement validation and connector metadata rules.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[level_assembly]"
```

- [ ] Implement the missing deterministic validation behavior.

- [ ] Re-run and commit.

```bash
git add engine/core/level/level_assembly.h engine/core/level/level_assembly.cpp tests/unit/test_level_assembly.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: harden modular level assembly validation"
```

### Task 14: Sprite Pipeline Runtime Consumption and Preview

**Starting files:**
- Modify: `content/schemas/sprite_atlas.schema.json`
- Modify: `engine/core/render/sprite_animator.h`
- Modify: `engine/core/render/sprite_animator.cpp`
- Modify: sprite tooling under `tools/sprite_pipeline/`

- [ ] Add failing tests or smoke validation for runtime consumption and preview/tuning artifact generation.

Run:

```powershell
rg -n "sprite_atlas|sprite_animator|atlas" engine tools tests/unit content/schemas -S
```

- [ ] Implement the missing runtime consumption path and preview artifact generation.

- [ ] Commit the sprite pipeline slice.

```bash
git add content/schemas/sprite_atlas.schema.json engine/core/render/sprite_animator.h engine/core/render/sprite_animator.cpp tools/sprite_pipeline docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: productize sprite pipeline runtime artifacts"
```

### Task 15: Procedural Toolkit Scenario Generation

**Starting files:**
- Modify: `engine/core/level/procedural_toolkit.h`
- Test: supporting procedural tests under `tests/unit/`

- [ ] Add failing tests for deterministic encounter/scenario generation from seeds.

Run:

```powershell
rg -n "procedural|dungeon|fov" engine/core tests/unit -S
```

- [ ] Implement the missing seeded scenario generation behavior.

- [ ] Commit the procedural toolkit slice.

```bash
git add engine/core/level/procedural_toolkit.h tests/unit docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: add procedural scenario generation"
```

### Task 16: Timeline and Animation Orchestration

**Starting files:**
- Modify: `engine/core/animation/animation_clip.h`
- Modify: `engine/core/animation/animation_system.h`
- Modify: `engine/core/animation/timeline_kernel.h`
- Modify: `engine/core/animation/animation_ai_bridge.cpp`
- Test: `tests/unit/test_animation_system.cpp`

- [ ] Add failing tests for deterministic event-to-animation binding and transient effect spawning.

Run:

```powershell
.\build\dev-mingw-debug\urpg_tests.exe "[animation]"
```

- [ ] Implement the missing timeline/event orchestration behavior.

- [ ] Re-run and commit.

```bash
git add engine/core/animation/animation_clip.h engine/core/animation/animation_system.h engine/core/animation/timeline_kernel.h engine/core/animation/animation_ai_bridge.cpp tests/unit/test_animation_system.cpp docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: finish timeline animation orchestration"
```

### Task 17: 2.5D and Editor Utilities

**Starting files:**
- Modify: `engine/core/render/raycast_renderer.h`
- Modify: `editor/productivity/editor_utility_task.h`
- Modify: any project-mode gating docs or config touched by 2.5D

- [ ] Add explicit gating and isolation checks so 2.5D remains an opt-in lane.

Run:

```powershell
rg -n "raycast|2.5D|editor_utility" engine editor docs tests/unit -S
```

- [ ] Implement the missing project-mode isolation and selected editor utility stabilization.

- [ ] Commit the final Wave 2 baseline slice.

```bash
git add engine/core/render/raycast_renderer.h editor/productivity/editor_utility_task.h docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "feat: gate optional 2.5d and editor utility lane"
```

## Final Validation Gate

- [ ] Run the repo-wide validation suite before declaring the backlog slice complete.

Run:

```powershell
ctest --preset dev-all --output-on-failure
```

Expected: PASS.

- [ ] Reconcile the canonical docs one final time.

Files:
- `README.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `RELEASE_CHECKLIST.md`
- `WORKLOG.md`

## Acceptance Summary

This plan is complete only when:

- UI/Menu, Message/Text, Save/Data, and Battle are each closed vertically across runtime, editor, schema, migration, diagnostics, and tests.
- Compat remains truthful and fully evidenced as an import/migration bridge.
- Release-readiness is proven by passing focused gates and `ctest --preset dev-all --output-on-failure`.
- Wave 2 opens only after Wave 1 release proof is finished, and proceeds in the order listed above.
