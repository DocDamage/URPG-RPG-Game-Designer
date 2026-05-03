# WYSIWYG Surface Route Wiring Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Wire every WYSIWYG template-showcase surface to an app-routable editor panel route so Phase 10 can only close when all 215 surfaces are backed by registry and app factory coverage.

**Architecture:** Keep the seven canonical release top-level panels stable and route showcase-specific authoring surfaces as nested editor routes. The showcase JSON will carry `editor_panel_registry_id`; the editor panel registry will expose every referenced route as `ReleaseTopLevel` or `Nested`; the editor app panel factory inventory will prove every referenced route can be opened by the shell or by its nested owner.

**Tech Stack:** C++20, Catch2, nlohmann/json, CMake/Ninja, existing editor panel registry and editor app factory inventory.

---

## File Structure

- Modify: `content/examples/wysiwyg_template_showcase.json`
  - Add `editor_panel_registry_id` to all 215 `surfaces[]` entries.
  - Preserve existing `editor_panel` class-style values as descriptive implementation labels only.
- Modify: `engine/core/editor/editor_panel_registry.cpp`
  - Promote currently deferred showcase routes that already have real panel implementations to `Nested`.
  - Add missing showcase route registry entries for kinds handled by the generic maker/gameplay WYSIWYG panels.
  - Remove or narrow the post-build demotion loop so explicitly registered showcase routes are not silently demoted to `Deferred`.
- Modify: `engine/core/editor/editor_panel_registry.h`
  - Add `bool isRoutableEditorPanelExposure(EditorPanelExposure exposure);` so tests and app inventory use one route predicate.
- Modify: `apps/editor/editor_app_panels.h`
  - Add nested-route factory inventory functions.
- Modify: `apps/editor/editor_app_panels.cpp`
  - Add `editorAppRegisteredNestedPanelFactoryIds()`.
  - Add `editorAppMissingRoutablePanelFactoryIds()`.
  - Keep `editorAppRegisteredPanelFactoryIds()` equal to the seven canonical top-level panels.
- Modify: `tests/unit/test_template_acceptance.cpp`
  - Add strict showcase route assertions for every surface.
- Modify: `tests/unit/test_editor_panel_registry.cpp`
  - Assert the registry keeps showcase route IDs routable and does not demote them.
- Modify: `tests/unit/test_editor_app_panels.cpp`
  - Assert every release top-level and nested showcase route has an app factory.
- Modify: `tests/unit/test_s32_wysiwyg_lanes.cpp`
  - Keep the Phase 10 truth guard; it should flip to requiring `READY` only after the unwired count is zero.
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
  - Close Phase 10 only after tests prove zero unwired surfaces.
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
  - Replace the open blocker paragraph with the verified closure paragraph only after tests pass.
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`
  - Mirror `docs/PROGRAM_COMPLETION_STATUS.md`.

## Route ID Policy

Use this deterministic route policy so the implementation does not invent inconsistent names:

- For existing concrete panel families, map to the existing registry owner:
  - `dialogue_preview` -> `message_inspector`
  - `event_command_graph` -> `event_authoring`
  - `save_load_preview_lab` -> `save_inspector`
  - `map_environment_preview` -> `spatial_authoring`
  - `battle_vfx_timeline` and `battle_vfx` -> `battle_presentation`
  - `ability_sandbox` -> `ability`
  - `export_preview` -> `export_diagnostics`
  - `dungeon3d_world` -> `3d_dungeon_world`
  - `loot_generator` -> `loot_generator`
  - `monster_collection` -> `monster_collection`
  - `metroidvania_ability_gates` -> `metroidvania_gates`
  - `platformer_physics_lab` -> `platformer_physics_lab`
  - `side_view_action_combat` -> `side_view_action_combat_kit`
  - `summon_gacha_banner` -> `summon_gacha_banner_builder`
  - `gacha_system` -> `gacha_system`
  - `achievement_visual_builder` -> `achievement_visual_builder`
  - `world_map_route_planner` -> `world_map_route_planner`
  - `fast_travel_map_builder` -> `fast_travel_map_builder`
  - `map_zoom_system` -> `map_zoom_system`
  - `farming_garden_plot` -> `farming_garden_plot_builder`
  - `puzzle_logic_board` -> `puzzle_logic_board`
  - `horror_environment_fx` -> `horror_fx_builder`
- For all remaining showcase kinds, use `showcase_<kind>` as the route ID, for example `showcase_crew_management`, `showcase_starship_route`, and `showcase_timed_input`.
- All `showcase_<kind>` routes are `Nested`, category `Template Showcase`, owner `editor/maker`, and reason `Nested template-showcase route handled by the maker WYSIWYG panel factory.`
- Do not add `showcase_<kind>` IDs to `requiredTopLevelPanelIds()`; the top-level shell remains the canonical seven unless a product decision separately expands release navigation.

---

### Task 1: Add Strict Showcase Route Tests

**Files:**
- Modify: `tests/unit/test_template_acceptance.cpp`
- Modify: `tests/unit/test_editor_app_panels.cpp`
- Test: `tests/unit/test_template_acceptance.cpp`
- Test: `tests/unit/test_editor_app_panels.cpp`

- [ ] **Step 1: Add registry and app factory includes to the template acceptance test**

Add these includes near the existing engine includes:

```cpp
#include "apps/editor/editor_app_panels.h"
#include "engine/core/editor/editor_panel_registry.h"
```

- [ ] **Step 2: Add a local route exposure helper**

Add this helper inside the anonymous namespace in `tests/unit/test_template_acceptance.cpp`:

```cpp
bool isTemplateShowcaseRoutableExposure(urpg::editor::EditorPanelExposure exposure) {
    return exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel ||
           exposure == urpg::editor::EditorPanelExposure::Nested;
}
```

- [ ] **Step 3: Extend the surface loop with strict route assertions**

Inside `TEST_CASE("WYSIWYG template showcase examples bind completed systems to starter projects", ...)`, in the inner `for (const auto& surface : example["surfaces"])` loop, immediately after the existing `editor_panel` non-empty assertion, add:

```cpp
            const auto registryId = surface.value("editor_panel_registry_id", "");
            REQUIRE_FALSE(registryId.empty());
            const auto* registryEntry = urpg::editor::findEditorPanelRegistryEntry(registryId);
            REQUIRE(registryEntry != nullptr);
            REQUIRE(isTemplateShowcaseRoutableExposure(registryEntry->exposure));
            const auto appFactoryIds = urpg::editor_app::editorAppRoutablePanelFactoryIds();
            REQUIRE(std::find(appFactoryIds.begin(), appFactoryIds.end(), registryId) != appFactoryIds.end());
```

- [ ] **Step 4: Add app route factory declarations**

Add these declarations to `apps/editor/editor_app_panels.h`:

```cpp
std::vector<std::string> editorAppRegisteredNestedPanelFactoryIds();
std::vector<std::string> editorAppRoutablePanelFactoryIds();
std::vector<std::string> editorAppMissingRoutablePanelFactoryIds();
```

- [ ] **Step 5: Add failing app route factory tests**

Add this test to `tests/unit/test_editor_app_panels.cpp`:

```cpp
TEST_CASE("editor app panels have route factories for release and nested showcase panels", "[editor][app][panel]") {
    REQUIRE(urpg::editor_app::editorAppMissingRoutablePanelFactoryIds().empty());

    const auto routableFactoryIds = urpg::editor_app::editorAppRoutablePanelFactoryIds();
    REQUIRE(std::find(routableFactoryIds.begin(), routableFactoryIds.end(), "message_inspector") !=
            routableFactoryIds.end());
    REQUIRE(std::find(routableFactoryIds.begin(), routableFactoryIds.end(), "event_authoring") !=
            routableFactoryIds.end());
    REQUIRE(std::find(routableFactoryIds.begin(), routableFactoryIds.end(), "showcase_crew_management") !=
            routableFactoryIds.end());
}
```

- [ ] **Step 6: Run tests and confirm they fail for missing route IDs/functions**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests -- -j1
ctest --preset dev-all -R "template|editor app panels" --output-on-failure
```

Expected result before implementation: compile failure for missing `editorAppRoutablePanelFactoryIds()` or test failure because showcase surfaces do not have `editor_panel_registry_id`.

- [ ] **Step 7: Commit failing tests**

```powershell
git add tests/unit/test_template_acceptance.cpp tests/unit/test_editor_app_panels.cpp apps/editor/editor_app_panels.h
git commit -m "test: require wysiwyg showcase route wiring"
```

### Task 2: Add Editor App Nested Factory Inventory

**Files:**
- Modify: `apps/editor/editor_app_panels.cpp`
- Test: `tests/unit/test_editor_app_panels.cpp`

- [ ] **Step 1: Implement nested factory inventory**

Replace `apps/editor/editor_app_panels.cpp` with this shape, preserving the existing top-level IDs:

```cpp
#include "apps/editor/editor_app_panels.h"

#include "engine/core/editor/editor_panel_registry.h"

#include <algorithm>

namespace urpg::editor_app {

std::vector<std::string> editorAppRegisteredPanelFactoryIds() {
    return {"diagnostics", "assets", "ability", "patterns", "mod", "analytics", "level_builder"};
}

std::vector<std::string> editorAppRegisteredNestedPanelFactoryIds() {
    std::vector<std::string> ids = {
        "message_inspector",
        "event_authoring",
        "save_inspector",
        "spatial_authoring",
        "battle_presentation",
        "export_diagnostics",
        "3d_dungeon_world",
        "loot_generator",
        "monster_collection",
        "metroidvania_gates",
        "platformer_physics_lab",
        "side_view_action_combat_kit",
        "summon_gacha_banner_builder",
        "gacha_system",
        "achievement_visual_builder",
        "world_map_route_planner",
        "fast_travel_map_builder",
        "map_zoom_system",
        "farming_garden_plot_builder",
        "puzzle_logic_board",
        "horror_fx_builder",
    };
    return ids;
}

std::vector<std::string> editorAppRoutablePanelFactoryIds() {
    auto ids = editorAppRegisteredPanelFactoryIds();
    auto nested = editorAppRegisteredNestedPanelFactoryIds();
    ids.insert(ids.end(), nested.begin(), nested.end());
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return ids;
}

std::vector<std::string> editorAppMissingReleasePanelFactoryIds() {
    auto factoryIds = editorAppRegisteredPanelFactoryIds();
    std::sort(factoryIds.begin(), factoryIds.end());

    std::vector<std::string> missing;
    for (const auto& panelId : editor::requiredTopLevelPanelIds()) {
        if (!std::binary_search(factoryIds.begin(), factoryIds.end(), panelId)) {
            missing.push_back(panelId);
        }
    }
    return missing;
}

std::vector<std::string> editorAppMissingRoutablePanelFactoryIds() {
    auto factoryIds = editorAppRoutablePanelFactoryIds();

    std::vector<std::string> missing;
    for (const auto& entry : editor::editorPanelRegistry()) {
        if (entry.exposure != editor::EditorPanelExposure::ReleaseTopLevel &&
            entry.exposure != editor::EditorPanelExposure::Nested) {
            continue;
        }
        if (!std::binary_search(factoryIds.begin(), factoryIds.end(), entry.id)) {
            missing.push_back(entry.id);
        }
    }
    return missing;
}

} // namespace urpg::editor_app
```

- [ ] **Step 2: Run app panel tests**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests -- -j1
ctest --preset dev-all -R "editor app panels" --output-on-failure
```

Expected result at this point: app panel tests may still fail because the registry does not yet contain every `showcase_<kind>` ID or because deferred routes remain deferred.

- [ ] **Step 3: Commit factory inventory**

```powershell
git add apps/editor/editor_app_panels.cpp apps/editor/editor_app_panels.h tests/unit/test_editor_app_panels.cpp
git commit -m "feat: inventory nested wysiwyg route factories"
```

### Task 3: Generate And Apply Showcase Route IDs

**Files:**
- Modify: `content/examples/wysiwyg_template_showcase.json`
- Test: `tests/unit/test_template_acceptance.cpp`

- [ ] **Step 1: Run a one-off route assignment script**

Run this PowerShell command from the repo root to add `editor_panel_registry_id` to all surfaces. It keeps JSON formatting deterministic enough for review:

```powershell
@'
import json
from pathlib import Path

path = Path("content/examples/wysiwyg_template_showcase.json")
doc = json.loads(path.read_text(encoding="utf-8"))

known = {
    "dialogue_preview": "message_inspector",
    "event_command_graph": "event_authoring",
    "save_load_preview_lab": "save_inspector",
    "map_environment_preview": "spatial_authoring",
    "battle_vfx_timeline": "battle_presentation",
    "battle_vfx": "battle_presentation",
    "ability_sandbox": "ability",
    "export_preview": "export_diagnostics",
    "dungeon3d_world": "3d_dungeon_world",
    "loot_generator": "loot_generator",
    "monster_collection": "monster_collection",
    "metroidvania_ability_gates": "metroidvania_gates",
    "platformer_physics_lab": "platformer_physics_lab",
    "side_view_action_combat": "side_view_action_combat_kit",
    "summon_gacha_banner": "summon_gacha_banner_builder",
    "gacha_system": "gacha_system",
    "achievement_visual_builder": "achievement_visual_builder",
    "world_map_route_planner": "world_map_route_planner",
    "fast_travel_map_builder": "fast_travel_map_builder",
    "map_zoom_system": "map_zoom_system",
    "farming_garden_plot": "farming_garden_plot_builder",
    "puzzle_logic_board": "puzzle_logic_board",
    "horror_environment_fx": "horror_fx_builder",
}

for example in doc["examples"]:
    for surface in example["surfaces"]:
        kind = surface["kind"]
        surface["editor_panel_registry_id"] = known.get(kind, f"showcase_{kind}")

path.write_text(json.dumps(doc, indent=2) + "\n", encoding="utf-8")
'@ | python -
```

- [ ] **Step 2: Confirm zero missing route IDs**

Run:

```powershell
$json = Get-Content content/examples/wysiwyg_template_showcase.json -Raw | ConvertFrom-Json
$surfaces = @($json.examples | ForEach-Object { $_.surfaces } | ForEach-Object { $_ })
$missing = @($surfaces | Where-Object { -not $_.editor_panel_registry_id })
"surfaces=$($surfaces.Count) missing_registry_id=$($missing.Count)"
```

Expected output:

```text
surfaces=215 missing_registry_id=0
```

- [ ] **Step 3: Run template acceptance test**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests -- -j1
ctest --preset dev-all -R "WYSIWYG template showcase" --output-on-failure
```

Expected result at this point: failure for unknown registry IDs until Task 4 adds the missing `showcase_<kind>` entries.

- [ ] **Step 4: Commit showcase route ID data**

```powershell
git add content/examples/wysiwyg_template_showcase.json
git commit -m "data: add wysiwyg showcase route ids"
```

### Task 4: Register Missing Showcase Routes As Nested

**Files:**
- Modify: `engine/core/editor/editor_panel_registry.cpp`
- Test: `tests/unit/test_editor_panel_registry.cpp`
- Test: `tests/unit/test_template_acceptance.cpp`

- [ ] **Step 1: Add a routable exposure helper**

In `engine/core/editor/editor_panel_registry.h`, add:

```cpp
bool isRoutableEditorPanelExposure(EditorPanelExposure exposure);
```

In `engine/core/editor/editor_panel_registry.cpp`, add after `editorPanelRegistry()`:

```cpp
bool isRoutableEditorPanelExposure(EditorPanelExposure exposure) {
    return exposure == EditorPanelExposure::ReleaseTopLevel || exposure == EditorPanelExposure::Nested;
}
```

- [ ] **Step 2: Replace silent demotion with explicit top-level guard**

Delete this loop from `buildRegistry()`:

```cpp
    for (auto& entry : registry) {
        if (entry.exposure == EditorPanelExposure::ReleaseTopLevel && !isReleaseTopLevelPanelId(entry.id)) {
            entry.exposure = EditorPanelExposure::Deferred;
        }
    }
```

Then change every non-canonical showcase route that should not appear in the top-level shell from `EditorPanelExposure::ReleaseTopLevel` to `EditorPanelExposure::Nested`.

- [ ] **Step 3: Promote existing concrete showcase routes from Deferred to Nested**

Apply these exposure changes in `engine/core/editor/editor_panel_registry.cpp`:

```cpp
{"event_authoring", "Event Authoring", "Gameplay", EditorPanelExposure::Nested, "editor/events",
 "Nested event command graph route required by WYSIWYG template showcase."},
{"export_diagnostics", "Export Diagnostics", "Release", EditorPanelExposure::Nested, "editor/export",
 "Nested export preview route required by WYSIWYG template showcase."},
{"battle_presentation", "Battle Presentation", "Battle", EditorPanelExposure::Nested, "editor/battle",
 "Nested battle VFX timeline route required by WYSIWYG template showcase."},
```

- [ ] **Step 4: Add generated `showcase_<kind>` registry entries**

For every `editor_panel_registry_id` in `content/examples/wysiwyg_template_showcase.json` that starts with `showcase_`, add one registry entry to `buildRegistry()` before the dev-only entries:

```cpp
{"showcase_crew_management", "Crew Management", "Template Showcase", EditorPanelExposure::Nested,
 "editor/maker", "Nested template-showcase route handled by the maker WYSIWYG panel factory."},
```

Use the title-casing rule: remove `showcase_`, split underscores, capitalize each word, and preserve common terms as normal words, for example `showcase_timed_input` -> `Timed Input`.

- [ ] **Step 5: Add every `showcase_<kind>` ID to nested app factories**

Append every generated `showcase_<kind>` ID to `editorAppRegisteredNestedPanelFactoryIds()` in `apps/editor/editor_app_panels.cpp`.

- [ ] **Step 6: Add a registry test that reads the showcase**

Add this test to `tests/unit/test_editor_panel_registry.cpp`:

```cpp
TEST_CASE("WYSIWYG showcase route ids are registered and routable", "[editor][panel][registry][wysiwyg]") {
    const auto showcasePath =
        std::filesystem::path(URPG_SOURCE_DIR) / "content" / "examples" / "wysiwyg_template_showcase.json";
    const auto showcase = readTextFile(showcasePath);
    const auto parsed = nlohmann::json::parse(showcase);

    for (const auto& example : parsed["examples"]) {
        for (const auto& surface : example["surfaces"]) {
            const auto routeId = surface.value("editor_panel_registry_id", "");
            INFO(routeId);
            REQUIRE_FALSE(routeId.empty());
            const auto* entry = urpg::editor::findEditorPanelRegistryEntry(routeId);
            REQUIRE(entry != nullptr);
            REQUIRE(urpg::editor::isRoutableEditorPanelExposure(entry->exposure));
        }
    }
}
```

Also add `#include <nlohmann/json.hpp>` to `tests/unit/test_editor_panel_registry.cpp`.

- [ ] **Step 7: Run registry and template tests**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests -- -j1
ctest --preset dev-all -R "WYSIWYG showcase|template|Editor panel registry|editor app panels" --output-on-failure
```

Expected result: all selected tests pass and no showcase route is unknown or deferred.

- [ ] **Step 8: Commit registry wiring**

```powershell
git add engine/core/editor/editor_panel_registry.h engine/core/editor/editor_panel_registry.cpp apps/editor/editor_app_panels.cpp tests/unit/test_editor_panel_registry.cpp
git commit -m "feat: register wysiwyg showcase routes"
```

### Task 5: Flip Phase 10 Back To Ready Only After Zero Unwired Surfaces

**Files:**
- Modify: `tests/unit/test_s32_wysiwyg_lanes.cpp`
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/status/PROGRAM_COMPLETION_STATUS.md`

- [ ] **Step 1: Strengthen the Phase 10 guard**

In `tests/unit/test_s32_wysiwyg_lanes.cpp`, replace the conditional `if (unwiredSurfaceCount == 0) ... else ...` block with unconditional closure assertions:

```cpp
    REQUIRE(unwiredSurfaceCount == 0);
    REQUIRE(phase10Line.find("`READY`") != std::string::npos);
    REQUIRE(phase10Line.find("`MANDATORY_OPEN`") == std::string::npos);
    REQUIRE(phase10Line.find("registry-backed editor panel routes") != std::string::npos);
    REQUIRE(programStatus.find("Phase 10 WYSIWYG roadmap closure is complete") != std::string::npos);
```

- [ ] **Step 2: Update release inventory Phase 10 row**

In `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`, set `phase_10_wysiwyg_roadmap_completion` to:

```markdown
| `phase_10_wysiwyg_roadmap_completion` | `READY` | None for the WYSIWYG route-wiring scope. Phase 11 offline tooling and Phase 12 curated release/tag work remain mandatory open lanes. | Every WYSIWYG showcase surface names a valid registry-backed editor panel route, every route is exposed as `ReleaseTopLevel` or `Nested`, app factory inventory covers every route, WYSIWYG done-rule evidence remains recorded across `READY` subsystem records, and focused regression coverage lives in `tests/unit/test_s32_wysiwyg_lanes.cpp`, `tests/unit/test_template_acceptance.cpp`, `tests/unit/test_editor_panel_registry.cpp`, and `tests/unit/test_editor_app_panels.cpp`. | `ctest --preset dev-all -R "WYSIWYG|readiness_status|template|Editor panel registry|editor app panels" --output-on-failure`; `.\tools\ci\truth_reconciler.ps1`; `.\tools\ci\run_local_gates.ps1` before release tagging. | May be claimed ready for the WYSIWYG route-wiring scope; no final 100-percent release claim until Phase 11 and Phase 12 are complete. |
```

- [ ] **Step 3: Update program status docs**

Replace the Phase 10 paragraph in both `docs/PROGRAM_COMPLETION_STATUS.md` and `docs/status/PROGRAM_COMPLETION_STATUS.md` with:

```markdown
Phase 10 WYSIWYG roadmap closure is complete for the current 100-percent program scope. The closure evidence is the enforced WYSIWYG done rule in `content/readiness/wysiwyg_done_rule.json`, READY subsystem evidence fields for visual authoring, live preview, saved project data, runtime execution, diagnostics, and tests, template showcase `editor_panel_registry_id` bindings for every surface, registry-backed `ReleaseTopLevel` or `Nested` route exposure, app factory inventory for every route, and focused regressions in `tests/unit/test_s32_wysiwyg_lanes.cpp`, `tests/unit/test_template_acceptance.cpp`, `tests/unit/test_editor_panel_registry.cpp`, and `tests/unit/test_editor_app_panels.cpp`. Phase 11 offline tooling and Phase 12 curated content/release-tag work remain mandatory before a final 100-percent release claim.
```

- [ ] **Step 4: Run Phase 10 focused verification**

Run:

```powershell
ctest --preset dev-all -R "WYSIWYG|readiness_status|template|Editor panel registry|editor app panels" --output-on-failure
.\tools\ci\truth_reconciler.ps1
.\tools\ci\check_mandatory_completion_scope.ps1
git diff --check
```

Expected result: all commands pass.

- [ ] **Step 5: Commit Phase 10 closure**

```powershell
git add tests/unit/test_s32_wysiwyg_lanes.cpp docs/release/100_PERCENT_COMPLETION_INVENTORY.md docs/PROGRAM_COMPLETION_STATUS.md docs/status/PROGRAM_COMPLETION_STATUS.md
git commit -m "docs: close phase 10 surface route wiring"
```

### Task 6: Final Build And Merge Discipline

**Files:**
- No new source files.
- Verify entire touched surface.

- [ ] **Step 1: Run focused build**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests -- -j1
```

Expected result: build succeeds. If MinGW archive/link fragility appears, rerun once with `-- -j1`; if it still fails, record the exact failing archive command in the final handoff and at least verify all touched `.cpp.obj` targets compile.

- [ ] **Step 2: Run focused CTest**

Run:

```powershell
ctest --preset dev-all -R "WYSIWYG|readiness_status|template|Editor panel registry|editor app panels" --output-on-failure
```

Expected result: all matching tests pass.

- [ ] **Step 3: Run docs and whitespace gates**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
.\tools\ci\check_mandatory_completion_scope.ps1
git diff --check
```

Expected result: all commands pass.

- [ ] **Step 4: Inspect final status**

Run:

```powershell
git status --short --branch
git log --oneline -5
```

Expected result: branch has only intended commits and no unstaged changes.

- [ ] **Step 5: Merge and push only after verification**

Run from `C:\dev\URPG Maker` after the worktree branch is clean:

```powershell
git merge --ff-only codex/wysiwyg-surface-route-wiring
git push origin main
```

Expected result: `main` fast-forwards and pushes the verified route wiring.

## Self-Review

- Spec coverage: The plan covers all 215 showcase surfaces by adding `editor_panel_registry_id`, registry exposure, app factory inventory, strict acceptance tests, Phase 10 truth docs, and focused verification.
- Placeholder scan: The plan has no placeholder markers, no open-ended "add tests" step without concrete test code, and no deferred mapping rule.
- Type consistency: The planned helper names are consistent: `editorAppRegisteredNestedPanelFactoryIds`, `editorAppRoutablePanelFactoryIds`, `editorAppMissingRoutablePanelFactoryIds`, and `isRoutableEditorPanelExposure`.
