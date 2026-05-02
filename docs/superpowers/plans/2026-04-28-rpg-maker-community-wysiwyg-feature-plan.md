# RPG Maker Community WYSIWYG Feature Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the RPG Maker community-requested feature set as usable URPG WYSIWYG systems with runtime behavior, editor surfaces, saved project data, diagnostics, and tests.

**Architecture:** Build three focused product families instead of one catch-all backend: event workflow tooling, player-facing UI/runtime helpers, and project/dev tooling. Each feature gets a deterministic runtime document/service under `engine/core`, an editor panel under `editor`, schema/fixture data under `content`, release top-level registry wiring, and focused Catch2 tests that prove WYSIWYG evidence.

**Tech Stack:** C++20, nlohmann/json, CMake/Ninja, Catch2, existing editor panel registry, existing asset/compat/message/event/save/map subsystems.

---

## WYSIWYG Completion Rule

No feature in this plan is complete unless it has all six bars:

- visual authoring surface
- live preview
- saved project data
- runtime execution
- diagnostics
- tests/validation

Every task below must add or extend tests that assert those bars directly.

## Feature List

1. Smart Event Workflow Suite
2. Event Template Library + Searchable Clipboard
3. Interaction Prompt System
4. Message Log / Dialogue History
5. Minimap / World Map / Fog-of-War System
6. Picture Hotspot / Picture Common Event Editor
7. Common Event Menu Builder
8. Developer Debug Overlay / Dev Console
9. Switch / Variable Inspector and Refactor Tool
10. Asset / DLC Library Manager
11. HUD Maker
12. Plugin Conflict / Dependency Resolver Upgrade

## File Structure

### Shared infrastructure

- Create: `engine/core/community/community_wysiwyg_feature.h`
- Create: `engine/core/community/community_wysiwyg_feature.cpp`
- Create: `editor/community/community_wysiwyg_panel.h`
- Create: `editor/community/community_wysiwyg_panel.cpp`
- Create: `content/schemas/community_wysiwyg_feature.schema.json`
- Create: `tests/unit/test_community_wysiwyg_features.cpp`
- Modify: `CMakeLists.txt`
- Modify: `engine/core/editor/editor_panel_registry.cpp`
- Modify: `tests/unit/test_editor_panel_registry.cpp`

The shared document should support common fields used by all twelve features: `schema_version`, `feature_type`, `id`, `display_name`, `visual_layers`, `actions`, `bindings`, `conditions`, `runtime_outputs`, and `diagnostics`.

### Event workflow family

- Create: `content/fixtures/smart_event_workflow_fixture.json`
- Create: `content/fixtures/event_template_library_fixture.json`
- Create: `content/fixtures/interaction_prompt_system_fixture.json`
- Create: `content/fixtures/picture_hotspot_common_event_fixture.json`
- Create: `content/fixtures/common_event_menu_builder_fixture.json`

### UI/runtime family

- Create: `content/fixtures/message_log_history_fixture.json`
- Create: `content/fixtures/minimap_fog_of_war_fixture.json`
- Create: `content/fixtures/hud_maker_fixture.json`

### Project/dev tooling family

- Create: `content/fixtures/developer_debug_overlay_fixture.json`
- Create: `content/fixtures/switch_variable_inspector_fixture.json`
- Create: `content/fixtures/asset_dlc_library_manager_fixture.json`
- Create: `content/fixtures/plugin_conflict_resolver_fixture.json`

---

## Wave 1: Event Workflow WYSIWYG Tools

### Task 1: Shared Community Feature Runtime

**Files:**
- Create: `engine/core/community/community_wysiwyg_feature.h`
- Create: `engine/core/community/community_wysiwyg_feature.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Add the shared runtime document**

Define:

```cpp
namespace urpg::community {

struct CommunityFeatureDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct CommunityFeatureAction {
    std::string id;
    std::string label;
    std::string trigger;
    std::string command;
    std::string target;
    std::vector<std::string> required_flags;
    std::map<std::string, std::string> parameters;
};

struct CommunityFeatureRuntimeState {
    std::set<std::string> flags;
    std::map<std::string, std::string> variables;
    std::vector<std::string> emitted_commands;
};

struct CommunityFeaturePreview {
    std::string feature_type;
    std::string trigger;
    std::vector<CommunityFeatureAction> active_actions;
    CommunityFeatureRuntimeState resulting_state;
    std::vector<CommunityFeatureDiagnostic> diagnostics;
};

class CommunityWysiwygFeatureDocument {
public:
    std::string schema_version = "urpg.community_wysiwyg.v1";
    std::string id;
    std::string feature_type;
    std::string display_name;
    std::vector<std::string> visual_layers;
    std::vector<CommunityFeatureAction> actions;

    [[nodiscard]] std::vector<CommunityFeatureDiagnostic> validate() const;
    [[nodiscard]] CommunityFeaturePreview preview(const CommunityFeatureRuntimeState& state, const std::string& trigger) const;
    [[nodiscard]] CommunityFeaturePreview execute(CommunityFeatureRuntimeState& state, const std::string& trigger) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static CommunityWysiwygFeatureDocument fromJson(const nlohmann::json& json);
};

std::vector<std::string> communityWysiwygFeatureTypes();
nlohmann::json communityFeaturePreviewToJson(const CommunityFeaturePreview& preview);

} // namespace urpg::community
```

- [ ] **Step 2: Implement validation**

Diagnostics must include:

- `invalid_schema_version`
- `missing_document_id`
- `missing_feature_type`
- `missing_display_name`
- `missing_visual_layers`
- `missing_action_id`
- `duplicate_action_id`
- `missing_action_trigger`
- `missing_action_command`
- `missing_action_target`

- [ ] **Step 3: Implement preview and execution**

`preview()` filters actions by trigger and required flags. `execute()` applies the same output into mutable runtime state by appending `command:target` strings to `emitted_commands` and writing action parameters into `variables`.

- [ ] **Step 4: Add feature type list**

Return exactly:

```cpp
{
  "smart_event_workflow",
  "event_template_library",
  "interaction_prompt_system",
  "message_log_history",
  "minimap_fog_of_war",
  "picture_hotspot_common_event",
  "common_event_menu_builder",
  "developer_debug_overlay",
  "switch_variable_inspector",
  "asset_dlc_library_manager",
  "hud_maker",
  "plugin_conflict_resolver"
}
```

### Task 2: Shared Community WYSIWYG Panel

**Files:**
- Create: `editor/community/community_wysiwyg_panel.h`
- Create: `editor/community/community_wysiwyg_panel.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Add panel snapshot model**

Define:

```cpp
struct CommunityWysiwygPanelSnapshot {
    std::string feature_type;
    std::string trigger;
    std::size_t visual_layer_count = 0;
    std::size_t active_action_count = 0;
    std::size_t emitted_command_count = 0;
    std::size_t diagnostic_count = 0;
};
```

- [ ] **Step 2: Add panel behavior**

`CommunityWysiwygPanel` must support:

- `loadDocument(CommunityWysiwygFeatureDocument document)`
- `setPreviewContext(CommunityFeatureRuntimeState state, std::string trigger)`
- `render()`
- `executePreview()`
- `saveProjectData() const`
- `preview() const`
- `snapshot() const`

### Task 3: Event Workflow Fixtures

**Files:**
- Create: `content/fixtures/smart_event_workflow_fixture.json`
- Create: `content/fixtures/event_template_library_fixture.json`
- Create: `content/fixtures/interaction_prompt_system_fixture.json`
- Create: `content/fixtures/picture_hotspot_common_event_fixture.json`
- Create: `content/fixtures/common_event_menu_builder_fixture.json`

- [ ] **Step 1: Smart Event Workflow fixture**

Must include smart chest, smart door, and event default actions:

```json
{
  "schema_version": "urpg.community_wysiwyg.v1",
  "id": "smart_event_workflow_default",
  "feature_type": "smart_event_workflow",
  "display_name": "Smart Event Workflow Suite",
  "visual_layers": ["quick_event_palette", "door_links", "chest_loot_preview"],
  "actions": [
    {
      "id": "create_smart_chest",
      "label": "Create Smart Chest",
      "trigger": "place_chest",
      "command": "create_event_template",
      "target": "chest",
      "required_flags": [],
      "parameters": { "loot_table": "starter_chest", "opened_switch": "chest_001_opened" }
    },
    {
      "id": "link_smart_door",
      "label": "Link Smart Door",
      "trigger": "link_door",
      "command": "create_transfer_pair",
      "target": "door_pair",
      "required_flags": [],
      "parameters": { "source_map": "town", "target_map": "inn" }
    }
  ]
}
```

- [ ] **Step 2: Event Template Library fixture**

Must cover categories, descriptions, tags, dependency preview, and clipboard emission.

- [ ] **Step 3: Interaction Prompt fixture**

Must cover `Open`, `Read`, `Talk`, `Inspect`, and `Enter` prompt bindings with visibility conditions.

- [ ] **Step 4: Picture Hotspot fixture**

Must cover picture id, common event id, hover effect, opaque-pixel hit testing, and runtime rebind action.

- [ ] **Step 5: Common Event Menu fixture**

Must cover menu entries, help text, preview image id, subtext, switch visibility, and common-event command execution.

### Task 4: Event Workflow Tests

**Files:**
- Create/Modify: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Add parameterized feature test**

For each Wave 1 fixture:

```cpp
const auto document = CommunityWysiwygFeatureDocument::fromJson(loadFixture(fixture));
REQUIRE(document.validate().empty());
REQUIRE_FALSE(document.visual_layers.empty());

CommunityFeatureRuntimeState state = stateForFeature(document.feature_type);
const auto trigger = document.actions.front().trigger;
const auto preview = document.preview(state, trigger);
REQUIRE(preview.active_actions.size() >= 1);

const auto executed = document.execute(state, trigger);
REQUIRE(executed.resulting_state.emitted_commands.size() >= 1);

CommunityWysiwygPanel panel;
panel.loadDocument(document);
panel.setPreviewContext(stateForFeature(document.feature_type), trigger);
panel.render();
REQUIRE(panel.snapshot().feature_type == document.feature_type);
REQUIRE(panel.saveProjectData() == document.toJson());
```

---

## Wave 2: Player-Facing Runtime/UI Systems

### Task 5: Message Log / Dialogue History

**Files:**
- Create: `content/fixtures/message_log_history_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- map-message capture
- face/speaker capture
- choice selection capture
- menu/shortcut access
- per-event conversation mode

- [ ] **Step 2: Runtime proof**

The test must execute a `message_displayed` trigger and assert emitted commands include `record_message:message_log`.

### Task 6: Minimap / World Map / Fog-of-War

**Files:**
- Create: `content/fixtures/minimap_fog_of_war_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- player/event icons
- POI markers
- zoom/fullscreen toggle
- fog-of-war reveal
- map-name window
- hide/show during cutscenes

- [ ] **Step 2: Runtime proof**

The test must execute `map_entered` and assert minimap commands and fog state variables are emitted.

### Task 7: HUD Maker

**Files:**
- Create: `content/fixtures/hud_maker_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- map HUD layout
- battle HUD layout
- actor HP/MP/TP widgets
- quest tracker widget
- minimap widget
- accessibility label output

- [ ] **Step 2: Runtime proof**

The test must execute `hud_preview` and assert `render_hud:map_hud` or equivalent command is emitted.

---

## Wave 3: Project and Developer Tooling

### Task 8: Developer Debug Overlay / Dev Console

**Files:**
- Create: `content/fixtures/developer_debug_overlay_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- gold/party/actor edits
- run common event
- start battle test
- transfer map
- dev-only save
- quick reload
- region-id overlay

- [ ] **Step 2: Runtime proof**

The test must assert debug commands only activate when a `dev_mode` flag is present.

### Task 9: Switch / Variable Inspector and Refactor

**Files:**
- Create: `content/fixtures/switch_variable_inspector_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- switch/variable dependency graph
- search usages
- rename/refactor
- unused switch scan
- nested-condition simplification preview

- [ ] **Step 2: Runtime proof**

The test must execute a `rename_variable` trigger and assert a refactor command is emitted with original and new ids.

### Task 10: Asset / DLC Library Manager

**Files:**
- Create: `content/fixtures/asset_dlc_library_manager_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- global asset/DLC source folders
- per-project mount rules
- import-on-demand
- duplicate/case-collision diagnostics
- export inclusion preview

- [ ] **Step 2: Runtime proof**

The test must execute `mount_asset_pack` and assert selected asset pack commands are emitted without copying every asset into the project.

### Task 11: Plugin Conflict / Dependency Resolver Upgrade

**Files:**
- Create: `content/fixtures/plugin_conflict_resolver_fixture.json`
- Test: `tests/unit/test_community_wysiwyg_features.cpp`

- [ ] **Step 1: Fixture requirements**

The fixture must model:

- plugin dependencies
- load-order constraints
- known incompatibilities
- native replacement suggestions
- compatibility patch recommendations

- [ ] **Step 2: Runtime proof**

The test must execute `analyze_plugins` and assert conflict diagnostics plus emitted fix suggestions.

---

## Wave 4: Registry, Schema, and Gates

### Task 12: Shared Schema

**Files:**
- Create: `content/schemas/community_wysiwyg_feature.schema.json`

- [ ] **Step 1: Add schema**

The schema must require:

- `schema_version`
- `id`
- `feature_type`
- `display_name`
- `visual_layers`
- `actions`

The `feature_type` enum must include all twelve ids.

### Task 13: Editor Registry Wiring

**Files:**
- Modify: `engine/core/editor/editor_panel_registry.cpp`
- Modify: `tests/unit/test_editor_panel_registry.cpp`

- [ ] **Step 1: Add release top-level panel ids**

Add:

- `smart_event_workflow`
- `event_template_library`
- `interaction_prompt_system`
- `message_log_history`
- `minimap_fog_of_war`
- `picture_hotspot_common_event`
- `common_event_menu_builder`
- `developer_debug_overlay`
- `switch_variable_inspector`
- `asset_dlc_library_manager`
- `hud_maker`
- `plugin_conflict_resolver`

Each entry must use `EditorPanelExposure::ReleaseTopLevel` and owner `editor/community` unless an existing narrower owner is clearly better.

- [ ] **Step 2: Update registry tests**

Assert all twelve ids are included in both `requiredTopLevelPanelIds()` and `smokeRequiredEditorPanelIds()`.

### Task 14: Final Focused Verification

**Files:**
- All changed files.

- [ ] **Step 1: Build tests**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests --parallel 4
```

Expected: build succeeds.

- [ ] **Step 2: Run focused tests**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[community][wysiwyg][features],[editor][panel][registry]"
```

Expected: all tests pass.

- [ ] **Step 3: Run whitespace check**

Run:

```powershell
git diff --check
```

Expected: no whitespace errors.

- [ ] **Step 4: Commit and push after implementation**

Run:

```powershell
git status --short
git add -A
git commit -m "feat: add community-requested wysiwyg maker features"
git push
```

Expected: worktree clean and `development` pushed.

## Execution Notes

This plan intentionally uses one shared runtime/panel contract for the first complete WYSIWYG pass. If a feature later needs deeper product behavior, split it into a specialist module only after the shared contract is green and registered.
