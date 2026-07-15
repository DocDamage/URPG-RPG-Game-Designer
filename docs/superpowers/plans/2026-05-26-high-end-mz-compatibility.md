# High-End MZ Compatibility Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend the existing bounded RPG Maker MZ compatibility bridge into a product-grade migration and parity workbench without weakening the current truth boundary.

**Architecture:** Keep `compat_bridge_exit` as the signed-off import/validation/migration bridge. Add new evidence-producing layers around it: full-project corpus manifests, richer plugin repair estimates, project migration scoring, optional runtime-parity harnesses, visual differential reports, event-command coverage analytics, and a creator-facing migration workbench. Each layer produces deterministic JSON reports that editor panels can render and CI can validate.

**Tech Stack:** C++20, nlohmann/json, Catch2, CMake/Ninja, existing `runtimes/compat_js`, `engine/core/plugin`, `engine/core/save`, `editor/compat`, `editor/plugin`, `editor/diagnostics`, and `tools/ci` gates.

---

## File Structure

- Modify `engine/core/plugin/plugin_compatibility_score.h`: add repair effort, migration suggestion, and confidence fields to plugin compatibility results.
- Modify `engine/core/plugin/plugin_compatibility_score.cpp`: compute deterministic repair effort from dependencies, permissions, unsupported APIs, fixture-only behavior, fallback paths, override conflicts, and failure diagnostics.
- Modify `editor/plugin/plugin_inspector_model.h`: expose aggregate repair effort and blocker counts in the plugin inspector snapshot.
- Modify `editor/plugin/plugin_inspector_model.cpp`: export the richer plugin inspector snapshot JSON.
- Modify `tests/unit/test_plugin_compatibility_score.cpp`: cover repair effort, migration suggestion, and machine-readable export behavior.
- Modify `tests/unit/test_plugin_inspector_panel.cpp`: cover editor snapshot exposure for repair effort once the model surface changes.
- Create `engine/core/compat/mz_project_compatibility_report.h`: declare project-level MZ compatibility report structures for maps, events, plugins, saves, assets, and visual parity.
- Create `engine/core/compat/mz_project_compatibility_report.cpp`: aggregate existing subsystem evidence into a deterministic project report.
- Create `tests/unit/test_mz_project_compatibility_report.cpp`: prove project scoring and blocker categorization.
- Create `content/compat/mz_project_corpus.schema.json`: schema for legally usable full-project corpus descriptors.
- Create `imports/fixtures/compat/mz_projects/README.md`: document legal intake rules for project fixtures.
- Create `imports/fixtures/compat/mz_projects/minimal_jrpg_project.json`: first tiny legal corpus descriptor, referencing only repo-owned fixture data.
- Create `tools/ci/check_mz_project_corpus.ps1`: validate corpus descriptors and legal/source metadata.
- Create `editor/compat/mz_migration_workbench_model.h`: creator-facing migration workbench model.
- Create `editor/compat/mz_migration_workbench_model.cpp`: combine plugin report, project report, command coverage, and export readiness into a renderable workbench snapshot.
- Create `tests/unit/test_mz_migration_workbench_model.cpp`: prove one-click workbench summary behavior.

## Task 1: Plugin Repair Effort And Suggestions

**Files:**
- Modify: `engine/core/plugin/plugin_compatibility_score.h`
- Modify: `engine/core/plugin/plugin_compatibility_score.cpp`
- Modify: `editor/plugin/plugin_inspector_model.h`
- Modify: `editor/plugin/plugin_inspector_model.cpp`
- Test: `tests/unit/test_plugin_compatibility_score.cpp`
- Test: `tests/unit/test_plugin_inspector_panel.cpp`

- [x] **Step 1: Write failing plugin score test**

Add a test that constructs one plugin with a native shim, a denied permission, fixture-only behavior, fallback path, and failure diagnostic. Assert that the result exposes a bounded repair estimate, migration suggestion count, and exported JSON fields:

```cpp
TEST_CASE("PluginCompatibilityScore estimates repair effort and migration suggestions",
          "[plugin][compatibility][mz_high_end]") {
    PluginCompatibilityAnalysisInput input;
    input.native_shim_hints = DefaultNativePluginShimHints();
    input.failure_diagnostics_jsonl =
        R"({"seq":1,"subsystem":"plugin_manager","event":"compat_failure","plugin":"WindowTweaks","operation":"execute_command_quickjs_call","message":"call failed","severity":"CRASH_PREVENTED"})";
    input.manifests = {
        manifest({
            {"name", "WindowTweaks"},
            {"permissions", nlohmann::json::array({"network.fetch"})},
            {"unsupportedApis", nlohmann::json::array({"Window_Base.drawText", "SceneManager.snapUnknown"})},
            {"fixtureOnlyBehaviors", nlohmann::json::array({"command_fixture"})},
            {"fallbackPaths", nlohmann::json::array({"window_fallback"})},
        }),
    };

    const auto report = AnalyzePluginCompatibility(input);
    const auto& window = findPlugin(report, "WindowTweaks");

    REQUIRE(window.estimated_repair_minutes == 240);
    REQUIRE(window.migration_suggestions.size() == 5);
    REQUIRE(window.migration_suggestions[0].code == "grant_or_replace_permission");
    REQUIRE(window.migration_suggestions[1].code == "replace_with_native_shim");
    REQUIRE(window.migration_suggestions[2].code == "manual_js_api_review");
    REQUIRE(window.migration_suggestions[3].code == "replace_fixture_only_behavior");
    REQUIRE(window.migration_suggestions[4].code == "remove_compat_fallback");
    REQUIRE(window.confidence == "low");

    const auto exported = PluginCompatibilityReportToJson(report);
    REQUIRE(exported["plugins"][0]["estimated_repair_minutes"] == 240);
    REQUIRE(exported["plugins"][0]["confidence"] == "low");
    REQUIRE(exported["plugins"][0]["migration_suggestions"].size() == 5);
}
```

- [x] **Step 2: Run test to verify it fails**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[plugin][compatibility][mz_high_end]" --reporter compact`

Expected: compile failure or test failure because `estimated_repair_minutes`, `migration_suggestions`, and `confidence` do not exist.

- [x] **Step 3: Implement minimal repair effort fields**

Add:

```cpp
struct PluginMigrationSuggestion {
    std::string code;
    std::string message;
    std::string target;
    int32_t estimated_minutes = 0;
};
```

Add these fields to `PluginCompatibilityResult`:

```cpp
std::vector<PluginMigrationSuggestion> migration_suggestions;
int32_t estimated_repair_minutes = 0;
std::string confidence = "high";
```

Implement suggestion helpers in `plugin_compatibility_score.cpp` so the test values are deterministic:

```cpp
void appendSuggestion(PluginCompatibilityResult& result,
                      std::string code,
                      std::string message,
                      std::string target,
                      int32_t minutes);
void finalizeRepairEstimate(PluginCompatibilityResult& result);
```

Use minute weights: denied permission 60, native shim unsupported API 45, unmapped unsupported API 90, fixture-only behavior 30, fallback path 15, blocking failure diagnostic 60, missing dependency 45, load cycle 90, override conflict 30, malformed manifest 180.

- [x] **Step 4: Run test to verify it passes**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[plugin][compatibility][mz_high_end]" --reporter compact`

Expected: PASS.

- [x] **Step 5: Add plugin inspector snapshot coverage**

Add a test in `tests/unit/test_plugin_inspector_panel.cpp` proving `estimated_repair_minutes` and `low_confidence_plugin_count` are exported in the snapshot after analysis.

- [x] **Step 6: Implement inspector aggregate fields**

Add to `PluginInspectorSnapshot`:

```cpp
int32_t estimated_repair_minutes = 0;
size_t low_confidence_plugin_count = 0;
```

Populate and export them from `PluginInspectorModel::refreshSnapshot()` and `exportSnapshotJson()`.

- [x] **Step 7: Run focused verification**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[plugin][compatibility][mz_high_end],[plugin][inspector]" --reporter compact
```

Expected: PASS.

## Task 2: Full MZ Project Corpus Descriptors

**Files:**
- Create: `content/compat/mz_project_corpus.schema.json`
- Create: `imports/fixtures/compat/mz_projects/README.md`
- Create: `imports/fixtures/compat/mz_projects/minimal_jrpg_project.json`
- Create: `tools/ci/check_mz_project_corpus.ps1`
- Test: `tests/unit/test_mz_project_corpus_schema.cpp`

- [x] **Step 1: Write failing schema test**

Create a Catch2 test that loads `content/compat/mz_project_corpus.schema.json` and validates `imports/fixtures/compat/mz_projects/minimal_jrpg_project.json` using the repo’s existing JSON schema validation helper.

- [x] **Step 2: Run failing schema test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_corpus]" --reporter compact`

Expected: FAIL because the schema and fixture do not exist.

- [x] **Step 3: Add corpus schema**

Define required fields: `schemaVersion`, `projectId`, `sourceLicense`, `legalUse`, `fixtureScope`, `maps`, `events`, `plugins`, `saves`, `assets`, `expectedCoverage`. Require `legalUse` to be `repo_owned`, `permissive_sample`, or `owner_provided_private`.

- [x] **Step 4: Add minimal legal fixture descriptor**

Create `minimal_jrpg_project.json` with repo-owned fixture metadata and zero external copyrighted payload.

- [x] **Step 5: Add CI corpus checker**

Implement `tools/ci/check_mz_project_corpus.ps1` to fail if any descriptor omits legal/source fields or points outside `imports/fixtures/compat/mz_projects`.

- [x] **Step 6: Verify corpus lane**

Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_corpus]" --reporter compact
.\tools\ci\check_mz_project_corpus.ps1
```

Expected: PASS.

## Task 3: Project-Level MZ Compatibility Report

**Files:**
- Create: `engine/core/compat/mz_project_compatibility_report.h`
- Create: `engine/core/compat/mz_project_compatibility_report.cpp`
- Test: `tests/unit/test_mz_project_compatibility_report.cpp`

- [x] **Step 1: Write failing project report test**

Create a test that builds a report from maps/events/plugins/assets/saves counts and asserts deterministic scoring:

```cpp
REQUIRE(report.project_score == 72);
REQUIRE(report.blockers == std::vector<std::string>{"unsupported_event_commands"});
REQUIRE(report.lanes["plugins"].score == 65);
```

- [x] **Step 2: Run failing test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_report]" --reporter compact`

Expected: FAIL because the report type does not exist.

- [x] **Step 3: Implement report aggregator**

Add lane scores for `maps`, `events`, `plugins`, `saves`, `assets`, `visual_parity`, and `runtime_parity`. Compute `project_score` as the average of present lanes. Emit blockers for zero-score lanes and unsupported event commands.

- [x] **Step 4: Verify report test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_report]" --reporter compact`

Expected: PASS.

## Task 4: Event Command Coverage Analytics

**Files:**
- Create: `engine/core/compat/mz_event_command_coverage.h`
- Create: `engine/core/compat/mz_event_command_coverage.cpp`
- Test: `tests/unit/test_mz_event_command_coverage.cpp`

- [x] **Step 1: Write failing command coverage test**

Assert that known supported commands such as `show_text`, `transfer_player`, `change_switch`, and `conditional_branch` are counted as supported, while an unknown command is counted as unsupported with a stable diagnostic code.

- [x] **Step 2: Run failing test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_event_commands]" --reporter compact`

Expected: FAIL because the coverage type does not exist.

- [x] **Step 3: Implement command coverage table**

Source the initial table from the current P2D command set recorded in `docs/PROGRAM_COMPLETION_STATUS.md`: `show_text`, `transfer_player`, `change_switch`, `change_variable`, `change_self_switch`, `change_gold`, `change_item`, `move_route`, `call_common_event`, and `conditional_branch`.

- [x] **Step 4: Verify command coverage**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_event_commands]" --reporter compact`

Expected: PASS.

## Task 5: Visual Differential Report Shell

**Files:**
- Create: `engine/core/compat/mz_visual_diff_report.h`
- Create: `engine/core/compat/mz_visual_diff_report.cpp`
- Test: `tests/unit/test_mz_visual_diff_report.cpp`

- [x] **Step 1: Write failing visual report test**

Assert that a scene comparison row records `scene_id`, `reference_hash`, `urpg_hash`, `pixel_delta_percent`, and a pass/fail status.

- [x] **Step 2: Run failing test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_visual_diff]" --reporter compact`

Expected: FAIL because the report type does not exist.

- [x] **Step 3: Implement deterministic report model only**

Do not claim captured RPG Maker reference frames yet. Implement the report shell so future captured artifacts can be compared without changing report contracts.

- [x] **Step 4: Verify visual report**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_visual_diff]" --reporter compact`

Expected: PASS.

## Task 6: Optional Runtime Parity Tier

**Files:**
- Create: `runtimes/compat_js/mz_runtime_parity_mode.h`
- Create: `runtimes/compat_js/mz_runtime_parity_mode.cpp`
- Test: `tests/unit/test_mz_runtime_parity_mode.cpp`

- [x] **Step 1: Write failing runtime tier test**

Assert the default mode is `bridge_only`, that `runtime_parity` requires explicit opt-in, and that diagnostics say runtime parity is not part of `compat_bridge_exit` readiness.

- [x] **Step 2: Run failing test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_runtime_parity]" --reporter compact`

Expected: FAIL because the mode type does not exist.

- [x] **Step 3: Implement opt-in boundary**

Add enum values `BridgeOnly` and `RuntimeParityExperimental`. Export a diagnostic JSON object with `release_authoritative=false` for runtime parity.

- [x] **Step 4: Verify runtime tier**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_runtime_parity]" --reporter compact`

Expected: PASS.

## Task 7: Creator Migration Workbench

**Files:**
- Create: `editor/compat/mz_migration_workbench_model.h`
- Create: `editor/compat/mz_migration_workbench_model.cpp`
- Test: `tests/unit/test_mz_migration_workbench_model.cpp`

- [x] **Step 1: Write failing workbench test**

Assert that the model combines plugin repair minutes, project score, unsupported event command count, visual diff status, and export readiness into a single JSON snapshot with `can_auto_migrate` and `manual_repair_minutes`.

- [x] **Step 2: Run failing test**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[editor][compat][mz_workbench]" --reporter compact`

Expected: FAIL because the model does not exist.

- [x] **Step 3: Implement workbench model**

Use existing plugin/project report structs. `can_auto_migrate` is true only when project score is at least 90, unsupported event command count is zero, and manual repair minutes is zero.

- [x] **Step 4: Verify workbench**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[editor][compat][mz_workbench]" --reporter compact`

Expected: PASS.

## Task 8: Governance And Docs Alignment

**Files:**
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/agent/QUALITY_GATES.md`
- Modify: `tools/ci/check_compat_health.ps1`

- [x] **Step 1: Add quality gate entries**

Add exact commands for the high-end MZ compatibility lanes:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_report],[compat][mz_event_commands],[compat][mz_visual_diff],[compat][mz_runtime_parity]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[plugin][compatibility][mz_high_end],[editor][compat][mz_workbench]" --reporter compact
.\tools\ci\check_mz_project_corpus.ps1
```

- [x] **Step 2: Update status docs without overclaiming**

State that high-end MZ compatibility is an expansion lane. Keep `compat_bridge_exit` READY for the existing signed-off bridge scope, and mark runtime parity as experimental until real reference captures and live backend evidence exist.

- [x] **Step 3: Verify truth gates**

Run:

```powershell
.\tools\ci\check_compat_health.ps1
.\tools\ci\truth_reconciler.ps1
```

Expected: PASS.

## Task 9: Final Verification

**Files:**
- No new files.

- [x] **Step 1: Run focused compat verification**

Run:

```powershell
ctest -L weekly --output-on-failure
```

Expected: PASS.

- [x] **Step 2: Run PR verification**

Run:

```powershell
ctest --preset dev-pr --output-on-failure
```

Expected: PASS.

- [x] **Step 3: Record outcome**

Update the plan checkboxes and report which high-end MZ compatibility tasks are complete. Do not claim full RPG Maker MZ runtime parity until Tasks 5 and 6 have real captured MZ references and backend parity evidence.

---

## Self-Review

Spec coverage: The plan covers the requested high-end improvements: real project corpus, live/runtime parity boundary, plugin scoring, visual differential harness, event command coverage, migration workbench, and plugin-to-native suggestions.

Truth boundary: The plan preserves the existing `compat_bridge_exit` READY claim and treats runtime parity as a new opt-in expansion lane.

Execution note: Implementing every task is a multi-commit epic. Each task is independently testable and should remain green before moving to the next one.
