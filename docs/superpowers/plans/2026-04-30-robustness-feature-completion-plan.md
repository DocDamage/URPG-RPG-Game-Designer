# Robustness Feature Completion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Turn the current internal release-candidate codebase into a more robust, more feature-filled creator product by closing the highest-value `PARTIAL` lanes without widening release claims beyond evidence.

**Architecture:** Keep the native C++ runtime as the source of truth and deepen existing subsystems instead of adding parallel feature islands. Every promoted feature must satisfy the URPG WYSIWYG done rule: visual authoring, live preview, saved project data, runtime execution, diagnostics, tests, and truthful docs/readiness records.

**Tech Stack:** C++20, CMake/Ninja, Catch2, ImGui editor snapshots, OpenGL/headless rendering paths, nlohmann/json schemas and fixtures, PowerShell CI gates, repository readiness records under `content/readiness/`.

---

## Current Source Of Truth

Read these before coding any task:

- `AGENTS.md`
- `docs/agent/INDEX.md`
- `docs/agent/ARCHITECTURE_MAP.md`
- `docs/agent/KNOWN_DEBT.md`
- `docs/agent/QUALITY_GATES.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/release/EDITOR_CONTROL_INVENTORY.md`
- `docs/features/FEATURE_ROBUSTNESS_PLAN.md`
- `content/readiness/readiness_status.json`
- `content/readiness/partial_lane_vertical_slices.json`

Current scan result on 2026-04-30:

- App-level release workflows are mostly `VERIFIED`; public release is still blocked by legal/privacy/distribution review and repository-wide source/vendor LFS access.
- Subsystem readiness has 5 `READY` native/core lanes and 11 `PARTIAL` product-depth or release-breadth lanes.
- Template readiness has 34 `READY` template rows and 3 important `PARTIAL` starter templates: `jrpg`, `visual_novel`, and `turn_based_rpg`.
- The highest-leverage robustness lanes are `presentation_runtime`, `visual_regression_harness`, `gameplay_ability_framework`, `accessibility_auditor`, `audio_mix_presets`, `export_validator`, `character_identity`, `governance_foundation`, and the asset-library promotion flow.

## Global Implementation Rules

- Do not promote a readiness row from `PARTIAL` to `READY` unless all evidence fields and docs agree.
- Do not expose a panel as `ReleaseTopLevel` unless it is wired in `apps/editor/main.cpp`, listed by the editor shell, has disabled/empty/error states, and is smoke-covered.
- Do not hide unsupported migration data. Preserve it in explicit fallback/unsupported payloads and emit diagnostics.
- Prefer focused Catch2 tests before implementation.
- Use the narrowest gate from `docs/agent/QUALITY_GATES.md` after each task.
- Update docs/readiness records in the same task that changes behavior or status.
- Keep raw imported assets under intake paths. Runtime features consume promoted, governed, packaged assets or generated artifacts only.

## Recommended Branch And Commit Flow

- Branch: `codex/robustness-feature-completion`
- Commit after each completed task group.
- Use small commits with messages like:
  - `test: expand renderer-backed coverage`
  - `feat: evaluate bounded ability conditions`
  - `feat: add asset promotion manifests`
  - `docs: align robustness readiness records`

---

## File Map

### Presentation And Visual Regression

- Modify: `tests/snapshot/test_renderer_backed_visual_capture.cpp`
- Modify: `tests/snapshot/goldens/*.golden.json`
- Modify: `engine/core/testing/visual_regression_harness.*`
- Modify if needed: `engine/core/render/headless_renderer.*`
- Modify if needed: `engine/core/platform/opengl_renderer.*`
- Modify: `tools/ci/check_renderer_backed_visual_capture.ps1`
- Modify: `docs/presentation/VALIDATION.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `content/readiness/readiness_status.json`

### Gameplay Ability Framework

- Modify: `engine/core/ability/gameplay_ability.h`
- Modify: `engine/core/ability/gameplay_ability.cpp`
- Create: `engine/core/ability/ability_condition_evaluator.h`
- Create: `engine/core/ability/ability_condition_evaluator.cpp`
- Modify: `engine/core/ability/ability_system_component.*`
- Modify: `engine/core/ability/ability_orchestration.*`
- Modify: `editor/ability/ability_inspector_panel.*`
- Modify: `editor/ability/ability_orchestration_panel.*`
- Modify: `content/schemas/gameplay_ability.schema.json`
- Test: `tests/unit/test_ability_activation.cpp`
- Test: `tests/unit/test_ability_tasks.cpp`
- Test: `tests/unit/test_ability_e2e.cpp`
- Test: `tests/unit/test_ability_inspector.cpp`

### Asset Library And Character Appearance

- Modify: `engine/core/assets/asset_library.*`
- Modify: `engine/core/assets/asset_action_view.*`
- Modify: `editor/assets/asset_library_model.*`
- Modify: `editor/assets/asset_library_panel.*`
- Create: `engine/core/assets/asset_promotion_manifest.h`
- Create: `engine/core/assets/asset_promotion_manifest.cpp`
- Create: `content/schemas/asset_promotion_manifest.schema.json`
- Modify: `engine/core/character/character_identity.*`
- Modify: `engine/core/character/character_appearance_composition.*`
- Modify: `editor/character/character_creator_model.*`
- Modify: `editor/character/character_creator_panel.*`
- Test: `tests/unit/test_asset_library.cpp`
- Test: `tests/unit/test_asset_library_panel.cpp`
- Test: `tests/unit/test_character_creator_model.cpp`
- Test: `tests/unit/test_character_creator_panel.cpp`

### Template Cross-Cutting Closure

- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/templates/jrpg_spec.md`
- Modify: `docs/templates/visual_novel_spec.md`
- Modify: `docs/templates/turn_based_rpg_spec.md`
- Modify: `tools/ci/check_template_certification.ps1`
- Modify: `tools/ci/check_template_spec_bar_drift.ps1`
- Modify or create fixtures under: `content/templates/`
- Test: `tests/unit/test_template_acceptance.cpp`
- Test: `tests/unit/test_template_bar_quality.cpp`
- Test: `tests/unit/test_s31_template_evidence_snapshots.cpp`

### Export, Legal, Packaging, Governance

- Modify: `engine/core/tools/export_packager.*`
- Modify: `engine/core/export/export_validator.*`
- Modify: `tools/pack/pack_cli.cpp`
- Modify: `tools/ci/check_platform_exports.ps1`
- Modify: `tools/ci/check_package_smoke.ps1`
- Modify: `docs/release/RELEASE_PACKAGING.md`
- Modify: `docs/release/AAA_RELEASE_READINESS_REPORT.md`
- Modify: `docs/release/LEGAL_REVIEW_SIGNOFF.md`
- Modify: `docs/release/RELEASE_SIGNOFF_WORKFLOW.md`
- Modify: `tools/ci/check_release_readiness.ps1`
- Modify: `tools/audit/project_audit_*`
- Test: `tests/unit/test_export_packager.cpp`
- Test: `tests/unit/test_export_validator.cpp`
- Test: `tests/integration/test_exported_runtime_smoke.cpp`
- Test: `tests/unit/test_project_audit_cli.cpp`

---

## Phase 0 - Baseline And Worktree Hygiene

### Task 0.1: Capture The Current Truth Baseline

**Purpose:** Record the current state before changing behavior so later readiness/doc updates are evidence-based.

**Files:**
- Read: `docs/APP_RELEASE_READINESS_MATRIX.md`
- Read: `docs/release/RELEASE_READINESS_MATRIX.md`
- Read: `content/readiness/readiness_status.json`
- Read: `content/readiness/partial_lane_vertical_slices.json`
- Read: `docs/features/FEATURE_ROBUSTNESS_PLAN.md`

- [x] **Step 1: Confirm the current git state**

Run:

```powershell
git status --short
git branch --show-current
```

Expected:

- Note any unrelated modified files.
- Do not revert user-authored changes.

- [x] **Step 2: Count current readiness statuses**

Run:

```powershell
$json = Get-Content 'content/readiness/readiness_status.json' -Raw | ConvertFrom-Json
$json.subsystems | Group-Object status | Sort-Object Name | ForEach-Object { "$($_.Name): $($_.Count)" }
$json.templates | Group-Object status | Sort-Object Name | ForEach-Object { "template $($_.Name): $($_.Count)" }
```

Expected current shape:

```text
PARTIAL: 11
READY: 5
template PARTIAL: 3
template READY: 34
```

- [x] **Step 3: List partial subsystem ids**

Run:

```powershell
$json = Get-Content 'content/readiness/readiness_status.json' -Raw | ConvertFrom-Json
$json.subsystems | Where-Object status -eq 'PARTIAL' | ForEach-Object { "- $($_.id)" }
$json.templates | Where-Object status -eq 'PARTIAL' | ForEach-Object { "- template:$($_.id)" }
```

Expected important ids:

```text
- presentation_runtime
- gameplay_ability_framework
- governance_foundation
- character_identity
- achievement_registry
- accessibility_auditor
- visual_regression_harness
- audio_mix_presets
- export_validator
- mod_registry
- analytics_dispatcher
- template:jrpg
- template:visual_novel
- template:turn_based_rpg
```

- [x] **Step 4: Commit nothing**

No file changes are expected from this task.

---

## Phase 1 - Broaden Visual Regression And Presentation Runtime Proof

### Task 1.1: Add More Shell-Owned Renderer-Backed Goldens

**Why:** `presentation_runtime` and `visual_regression_harness` are `PARTIAL` mainly because coverage is bounded. Broader renderer-backed goldens make future renderer changes safer and raise confidence in actual app frames.

**Files:**
- Modify: `tests/snapshot/test_renderer_backed_visual_capture.cpp`
- Modify: `tests/snapshot/goldens/*.golden.json`
- Modify if needed: `tools/ci/check_renderer_backed_visual_capture.ps1`
- Inspect: `engine/core/engine_shell.h`
- Inspect: `engine/core/scene/map_scene.*`
- Inspect: `engine/core/scene/menu_scene.*`
- Inspect: `engine/core/scene/battle_scene.*`
- Inspect: `engine/core/testing/visual_regression_harness.*`

- [x] **Step 1: Add failing tests for two new shell-owned captures**

Add tests in `tests/snapshot/test_renderer_backed_visual_capture.cpp` using the existing `RendererBackedCapture` style:

```cpp
TEST_CASE("RendererBackedCapture captures shell-owned map frame with dialogue and runtime assets",
          "[snapshot][renderer][shell][map]") {
    EngineShell shell;
    shell.configureForHeadlessTest();
    shell.startNewGame();
    shell.tick(0.0f);

    auto& layer = RenderLayer::getInstance();
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(layer.getFrameCommands(), 640, 400, &errorMessage);

    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->width == 640);
    REQUIRE(snapshot->height == 400);

    const auto result = compareRendererBackedGolden(harness, "engine_shell_map_dialogue_runtime_assets", *snapshot);
    REQUIRE(result.matches);
}

TEST_CASE("RendererBackedCapture captures shell-owned menu navigation focus frame",
          "[snapshot][renderer][shell][menu]") {
    EngineShell shell;
    shell.configureForHeadlessTest();
    shell.pushMenuSceneForTest();
    shell.tick(0.0f);

    auto& layer = RenderLayer::getInstance();
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(layer.getFrameCommands(), 640, 400, &errorMessage);

    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = compareRendererBackedGolden(harness, "engine_shell_menu_focus_runtime_path", *snapshot);
    REQUIRE(result.matches);
}
```

If `configureForHeadlessTest()` or `pushMenuSceneForTest()` do not exist, add test-only helper methods using the existing `EngineShell` test helper pattern rather than reaching into private scene state.

- [x] **Step 2: Run the focused snapshot/build path before regeneration**

Run:

```powershell
ctest --test-dir build\dev-ninja-debug -R "RendererBackedCapture" --output-on-failure
```

Expected:

- The new tests either fail because the golden files do not exist, or the build exposes missing helper seams before golden regeneration.

Execution note: existing `EngineShell` capture helpers were sufficient, so the implementation went through `cmake --build --preset dev-debug --target urpg_snapshot_tests` plus explicit regeneration for the two new golden ids instead of adding temporary failing helper seams.

- [x] **Step 3: Add minimal shell helper seams if the tests cannot compile**

Prefer methods guarded as ordinary deterministic test helpers, not production-only shortcuts:

```cpp
// engine/core/engine_shell.h
void configureForHeadlessTest();
void pushMenuSceneForTest();
```

Implementation should use existing scene construction and `SceneManager` APIs. It must not bypass normal `tick()` render command emission.

Execution note: no new `EngineShell` API was required; the tests use `VisualRegressionHarness::captureEngineTick()` plus `SceneManager::pushScene()`.

- [x] **Step 4: Generate new goldens explicitly**

Run:

```powershell
$env:URPG_REGEN_RENDERER_BACKED_GOLDENS='1'
ctest --test-dir build\dev-ninja-debug -R "RendererBackedCapture.*shell-owned" --output-on-failure
Remove-Item Env:\URPG_REGEN_RENDERER_BACKED_GOLDENS
```

Expected:

- New `RendererBackedCapture_*.golden.json` files are written under `tests/snapshot/goldens/`.
- The generated files are deterministic on a second run.

- [x] **Step 5: Re-run without regeneration**

Run:

```powershell
ctest --test-dir build\dev-ninja-debug -R "RendererBackedCapture.*shell-owned" --output-on-failure
```

Expected: PASS.

- [x] **Step 6: Update docs/readiness only for the new evidence**

Update:

- `docs/presentation/VALIDATION.md`
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`

Do not promote `presentation_runtime` or `visual_regression_harness` to `READY` unless cross-backend breadth is also complete.

- [x] **Step 7: Verify the presentation lane**

Run:

```powershell
.\tools\ci\run_presentation_gate.ps1
```

Expected: PASS.

- [x] **Step 8: Commit**

```powershell
git add tests/snapshot/test_renderer_backed_visual_capture.cpp tests/snapshot/goldens docs/presentation/VALIDATION.md docs/release/RELEASE_READINESS_MATRIX.md content/readiness/readiness_status.json
git commit -m "test: broaden renderer-backed shell captures"
```

### Task 1.2: Add A Headless Renderer Parity Lane For The Same Frame Intents

**Why:** The docs identify full cross-backend renderer breadth as the presentation gap. A headless parity test should prove the same shell-owned frame intent is consumed outside OpenGL.

**Files:**
- Modify: `engine/core/render/headless_renderer.*`
- Modify: `tests/unit/test_presentation_runtime.cpp` or `tests/snapshot/test_renderer_backed_visual_capture.cpp`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `content/readiness/readiness_status.json`

- [x] **Step 1: Write a failing test that submits the same command stream to `HeadlessRenderer`**

Add a test:

```cpp
TEST_CASE("HeadlessRenderer consumes shell-owned map and menu command streams", "[presentation][headless][shell]") {
    EngineShell shell;
    shell.configureForHeadlessTest();
    shell.startNewGame();
    shell.tick(0.0f);

    HeadlessRenderer renderer;
    const auto mapResult = renderer.consumeFrame(RenderLayer::getInstance().getFrameCommands(), 640, 400);
    REQUIRE(mapResult.command_count > 0);
    REQUIRE(mapResult.text_count > 0);
    REQUIRE(mapResult.rect_count > 0);
    REQUIRE(mapResult.rejected_count == 0);

    RenderLayer::getInstance().flush();
    shell.pushMenuSceneForTest();
    shell.tick(0.0f);

    const auto menuResult = renderer.consumeFrame(RenderLayer::getInstance().getFrameCommands(), 640, 400);
    REQUIRE(menuResult.command_count > 0);
    REQUIRE(menuResult.text_count > 0);
    REQUIRE(menuResult.rejected_count == 0);
}
```

If `consumeFrame` already has a different name, use the existing public method and return a small structured result instead of making the test parse private state.

- [x] **Step 2: Run focused build/test and confirm the missing surface**

Run:

```powershell
ctest --test-dir build\dev-ninja-debug -R "HeadlessRenderer consumes shell-owned" --output-on-failure
```

Expected:

- The test either fails because result counters or API support are missing, or existing counters expose the narrower missing parity surface.

Execution note: `HeadlessRenderer` already exposed frame summaries, so the added focused test covered the missing shell-owned Title/Options parity without requiring a new `consumeFrame` API.

- [x] **Step 3: Implement result counters in `HeadlessRenderer`**

Add a result type:

```cpp
struct HeadlessFrameResult {
    std::size_t command_count = 0;
    std::size_t text_count = 0;
    std::size_t rect_count = 0;
    std::size_t sprite_count = 0;
    std::size_t tile_count = 0;
    std::size_t rejected_count = 0;
    std::vector<std::string> diagnostics;
};
```

Count supported command types. Unsupported command types should increment `rejected_count` and push a diagnostic with the command kind.

Execution note: no renderer changes were needed; existing `HeadlessRenderFrameSummary` counters already record command, text, rect, sprite, and tile counts.

- [x] **Step 4: Re-run the focused test**

Run:

```powershell
ctest --test-dir build\dev-ninja-debug -R "HeadlessRenderer consumes shell-owned" --output-on-failure
```

Expected: PASS.

- [x] **Step 5: Re-run presentation gate**

Run:

```powershell
.\tools\ci\run_presentation_gate.ps1
```

Expected: PASS.

- [x] **Step 6: Commit**

```powershell
git add engine/core/render/headless_renderer.* tests docs/release/RELEASE_READINESS_MATRIX.md content/readiness/readiness_status.json
git commit -m "test: add headless renderer shell parity"
```

---

## Phase 2 - Make Gameplay Abilities More Feature-Filled

### Task 2.1: Add A Bounded Ability Condition Evaluator

**Why:** `gameplay_ability_framework` is partial because scripted conditions are preserved but intentionally not evaluated. A bounded evaluator closes the biggest gameplay-depth gap without embedding arbitrary scripting.

**Files:**
- Create: `engine/core/ability/ability_condition_evaluator.h`
- Create: `engine/core/ability/ability_condition_evaluator.cpp`
- Modify: `engine/core/ability/gameplay_ability.h`
- Modify: `engine/core/ability/gameplay_ability.cpp`
- Modify: `engine/core/ability/ability_system_component.*`
- Modify: `tests/unit/test_ability_activation.cpp`
- Modify: `CMakeLists.txt`
- Modify: `content/schemas/gameplay_ability.schema.json`

**Supported expression grammar for this task:**

- Left operand:
  - `source.hp`
  - `source.mp`
  - `source.attribute.<Name>`
  - `source.hasTag('<Tag.Name>')`
  - `target.hasTag('<Tag.Name>')`
- Operators:
  - Numeric: `>`, `>=`, `<`, `<=`, `==`, `!=`
  - Boolean: `== true`, `== false`, `!= true`, `!= false`
- Boolean connectors:
  - `&&`
  - `||`
- No parentheses in the first slice.
- No arithmetic in the first slice.
- Unsupported grammar must fail closed with `condition_parse_error`, not silently allow activation.

- [x] **Step 1: Write evaluator unit tests first**

Add tests in `tests/unit/test_ability_activation.cpp`:

```cpp
SECTION("Bounded active condition allows activation from source attributes and tags") {
    owner.setAttribute("HP", 42.0f);
    owner.setAttribute("MP", 12.0f);
    owner.addTag(GameplayTag("State.Focused"));
    fireAbility.editInfo().activeCondition = "source.hp >= 40 && source.hasTag('State.Focused') == true";

    const auto check = fireAbility.evaluateActivation(owner);
    REQUIRE(check.allowed);
    REQUIRE(check.reason.empty());
}

SECTION("Bounded active condition blocks activation with deterministic reason") {
    owner.setAttribute("HP", 8.0f);
    fireAbility.editInfo().activeCondition = "source.hp >= 40";

    const auto check = fireAbility.evaluateActivation(owner);
    REQUIRE_FALSE(check.allowed);
    REQUIRE(check.reason == "active_condition_false");
    REQUIRE(check.detail.find("source.hp >= 40") != std::string::npos);
}

SECTION("Unsupported active condition grammar fails closed") {
    fireAbility.editInfo().activeCondition = "source.hp + source.mp > 50";

    const auto check = fireAbility.evaluateActivation(owner);
    REQUIRE_FALSE(check.allowed);
    REQUIRE(check.reason == "condition_parse_error");
    REQUIRE(check.detail.find("source.hp + source.mp > 50") != std::string::npos);
}
```

- [x] **Step 2: Run and confirm failure**

Run:

```powershell
ctest --preset dev-all -R "GameplayAbility: Activation Pipeline" --output-on-failure
```

Expected:

- The first test still fails with the old `active_condition_unsupported` behavior.

Actual note: the first `ctest` invocation ran a stale already-built binary and passed; implementation continued, then `urpg_tests` was rebuilt before trusting verification.

- [x] **Step 3: Define evaluator API**

Create `engine/core/ability/ability_condition_evaluator.h`:

```cpp
#pragma once

#include "gameplay_ability.h"
#include <string>

namespace urpg::ability {

struct AbilityConditionEvaluation {
    bool parsed = true;
    bool value = true;
    std::string reason;
    std::string detail;
};

class AbilityConditionEvaluator {
  public:
    AbilityConditionEvaluation evaluate(const std::string& expression,
                                        const AbilitySystemComponent& source,
                                        const GameplayAbility::AbilityExecutionContext* context = nullptr) const;
};

} // namespace urpg::ability
```

- [x] **Step 4: Implement the minimal parser**

Implementation requirements:

- Split top-level `||` first, then `&&`.
- Trim ASCII whitespace.
- Match simple comparison rows.
- Map `source.hp` to `source.getAttribute("HP", default)`.
- Map `source.mp` to `source.getAttribute("MP", default)`.
- Map `source.attribute.Name` to `source.getAttribute("Name", default)`.
- Map `source.hasTag('Tag')` and `target.hasTag('Tag')` to booleans.
- For target lookup, use `context->targets.front().abilitySystem` for the first slice.
- Return `{parsed=false, value=false, reason="condition_parse_error"}` for unsupported syntax.

- [x] **Step 5: Wire evaluator into `GameplayAbility::evaluateActivation`**

Replace the old unsupported-condition block with:

```cpp
const auto& activeCondition = resolveActiveCondition();
if (!activeCondition.empty()) {
    const AbilityConditionEvaluator evaluator;
    const auto condition = evaluator.evaluate(activeCondition, source, nullptr);
    if (!condition.parsed) {
        result.allowed = false;
        result.reason = "condition_parse_error";
        result.detail = condition.detail;
        return result;
    }
    if (!condition.value) {
        result.allowed = false;
        result.reason = "active_condition_false";
        result.detail = condition.detail;
        return result;
    }
}
```

In the overload with `AbilityExecutionContext`, pass `&context`.

- [x] **Step 6: Update the old unsupported-condition test**

The existing test named `Unsupported active conditions are blocked with explicit diagnostics` must become the unsupported grammar test. Do not delete diagnostic assertions; change the reason to `condition_parse_error`.

- [x] **Step 7: Run focused ability tests**

Run:

```powershell
ctest --preset dev-all -R "Ability|GameplayAbility" --output-on-failure
```

Expected: PASS.

- [x] **Step 8: Update schema/docs/readiness wording**

Update `content/schemas/gameplay_ability.schema.json` to document the bounded expression grammar. Update:

- `docs/release/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`
- `docs/features/FEATURE_ROBUSTNESS_PLAN.md`

Do not promote to `READY` until broader authored task composition also lands.

- [x] **Step 9: Commit**

```powershell
git add engine/core/ability tests/unit/test_ability_activation.cpp CMakeLists.txt content/schemas/gameplay_ability.schema.json docs/release/RELEASE_READINESS_MATRIX.md content/readiness/readiness_status.json docs/features/FEATURE_ROBUSTNESS_PLAN.md
git commit -m "feat: evaluate bounded ability conditions"
```

### Task 2.2: Add A Creator-Friendly Ability Task Composition Slice

**Why:** Ability orchestration currently has one authored path. The next feature-filled slice should let creators compose common task sequences without writing code.

**Files:**
- Modify: `engine/core/ability/ability_task.*`
- Modify: `engine/core/ability/ability_orchestration.*`
- Modify: `editor/ability/ability_orchestration_panel.*`
- Modify: `tests/unit/test_ability_tasks.cpp`
- Modify: `tests/unit/test_ability_e2e.cpp`
- Modify: `content/schemas/gameplay_ability.schema.json`

**Task kinds to support in this slice:**

- `wait_input`
- `wait_event`
- `wait_projectile_collision`
- `apply_effect`
- `play_cue`
- `branch_on_condition`

- [x] **Step 1: Add failing orchestration tests**

Add a test that builds JSON like:

```json
{
  "id": "skill.chain_lightning",
  "tasks": [
    { "kind": "wait_input", "action": "Confirm", "timeout_ms": 1000 },
    { "kind": "branch_on_condition", "condition": "source.mp >= 5", "on_true": "apply_shock", "on_false": "play_fizzle" },
    { "id": "apply_shock", "kind": "apply_effect", "effect_id": "state.shocked", "target": "primary" },
    { "id": "play_fizzle", "kind": "play_cue", "cue_id": "ability.fizzle" }
  ]
}
```

Test expectations:

- Serializer preserves task order.
- Runtime preview reports each task row.
- `branch_on_condition` uses the evaluator from Task 2.1.
- Diagnostics report missing `on_true` or `on_false` ids.

- [x] **Step 2: Run focused tests and confirm failure**

Run:

```powershell
ctest --preset dev-all -R "AbilityTask|Ability end-to-end|AbilityOrchestration" --output-on-failure
```

Expected: FAIL for missing task kinds or missing preview rows.

Actual: `cmake --build --preset dev-debug --target urpg_tests` failed because `AbilityOrchestrationDocument::tasks` and panel task preview rows did not exist yet.

- [x] **Step 3: Add task document fields**

Extend task representation with:

```cpp
std::string id;
std::string kind;
std::string condition;
std::string on_true;
std::string on_false;
std::string effect_id;
std::string cue_id;
std::string target;
std::vector<std::string> diagnostics;
```

- [x] **Step 4: Add runtime preview rows**

Panel snapshots must expose:

```cpp
struct AbilityTaskPreviewRow {
    std::string id;
    std::string kind;
    std::string status;
    std::string detail;
    bool executable = false;
    std::string disabled_reason;
};
```

- [x] **Step 5: Implement branch validation**

Validation rules:

- `branch_on_condition.condition` must parse with `AbilityConditionEvaluator`.
- `on_true` and `on_false` must refer to task ids in the same document.
- Branch targets must not create an immediate two-node cycle.
- Missing branch target emits `ability_task_branch_missing_target`.
- Unsupported condition emits `ability_task_branch_condition_invalid`.

- [x] **Step 6: Re-run focused tests**

Run:

```powershell
ctest --preset dev-all -R "AbilityTask|Ability end-to-end|AbilityOrchestration" --output-on-failure
```

Expected: PASS.

- [x] **Step 7: Update docs and readiness**

Update:

- `docs/features/FEATURE_ROBUSTNESS_PLAN.md`
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`

If Task 2.1 and 2.2 are both complete, reassess whether `gameplay_ability_framework` can move from `PARTIAL` to `READY`. Only promote if no remaining `mainGaps` are still true.

- [x] **Step 8: Commit**

```powershell
git add engine/core/ability editor/ability tests/unit/test_ability_tasks.cpp tests/unit/test_ability_e2e.cpp content/schemas/gameplay_ability.schema.json docs/features/FEATURE_ROBUSTNESS_PLAN.md docs/release/RELEASE_READINESS_MATRIX.md content/readiness/readiness_status.json
git commit -m "feat: add authored ability task composition"
```

---

## Phase 3 - Make Asset Intake And Character Creation Creator-Grade

### Task 3.1: Add Governed Asset Promotion Manifests

**Why:** The asset browser can report and filter assets, but true creator robustness needs a promotion manifest that records source, license, runtime readiness, generated previews, package inclusion, and safe archive state.

**Files:**
- Create: `engine/core/assets/asset_promotion_manifest.h`
- Create: `engine/core/assets/asset_promotion_manifest.cpp`
- Create: `content/schemas/asset_promotion_manifest.schema.json`
- Modify: `engine/core/assets/asset_library.*`
- Modify: `engine/core/assets/asset_action_view.*`
- Modify: `editor/assets/asset_library_model.*`
- Modify: `editor/assets/asset_library_panel.*`
- Modify: `tests/unit/test_asset_library.cpp`
- Modify: `tests/unit/test_asset_library_panel.cpp`

**Manifest fields:**

```json
{
  "schemaVersion": "1.0.0",
  "assetId": "asset.hero.walk",
  "sourcePath": "imports/raw/example/hero.png",
  "promotedPath": "resources/assets/characters/hero.png",
  "licenseId": "BND-001",
  "status": "runtime_ready",
  "preview": {
    "kind": "image",
    "thumbnailPath": "resources/previews/hero.thumb.png",
    "width": 48,
    "height": 48
  },
  "package": {
    "includeInRuntime": true,
    "requiredForRelease": false
  },
  "diagnostics": []
}
```

- [x] **Step 1: Write manifest round-trip tests**

Add tests:

```cpp
TEST_CASE("AssetPromotionManifest round-trips runtime-ready promoted asset", "[assets][promotion]") {
    AssetPromotionManifest manifest;
    manifest.schemaVersion = "1.0.0";
    manifest.assetId = "asset.hero.walk";
    manifest.sourcePath = "imports/raw/example/hero.png";
    manifest.promotedPath = "resources/assets/characters/hero.png";
    manifest.licenseId = "BND-001";
    manifest.status = AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;

    const auto json = serializeAssetPromotionManifest(manifest);
    const auto loaded = deserializeAssetPromotionManifest(json);

    REQUIRE(loaded.assetId == manifest.assetId);
    REQUIRE(loaded.status == AssetPromotionStatus::RuntimeReady);
    REQUIRE(loaded.package.includeInRuntime);
    REQUIRE(loaded.diagnostics.empty());
}
```

- [x] **Step 2: Run focused asset build/test path**

Run:

```powershell
ctest --preset dev-all -R "AssetLibrary|assets.*promotion" --output-on-failure
```

Expected: the initial build/test path exposes the missing manifest type or behavior before the implementation lands.

- [x] **Step 3: Implement serializer and validator**

Validation rules:

- `assetId` must be non-empty.
- `status=runtime_ready` requires `promotedPath`.
- `includeInRuntime=true` requires `licenseId`.
- `requiredForRelease=true` requires `includeInRuntime=true`.
- Missing thumbnail is allowed only when preview `kind` is `none` or `pending`.

- [x] **Step 4: Surface promotion manifest data in asset action rows**

Extend asset action rows with:

```cpp
std::string promotion_status;
std::string promoted_path;
std::string license_id;
bool include_in_runtime = false;
bool required_for_release = false;
std::vector<std::string> promotion_diagnostics;
```

- [x] **Step 5: Add panel snapshot assertions**

Tests should assert:

- Runtime-ready promoted assets show `ready`.
- Assets missing license evidence recommend `add_license_evidence`.
- Missing promoted file recommends `fix_missing_file`.
- Archived duplicate assets cannot be packaged.

- [x] **Step 6: Run focused asset tests**

Run:

```powershell
ctest --preset dev-all -R "AssetLibrary|asset.*panel|asset.*promotion" --output-on-failure
```

Expected: PASS.

- [x] **Step 7: Update feature robustness docs**

Update:

- `docs/features/FEATURE_ROBUSTNESS_PLAN.md`
- `docs/asset_intake/` relevant intake docs
- `docs/release/RELEASE_PACKAGING.md` if package inclusion semantics change

- [x] **Step 8: Commit**

```powershell
git add engine/core/assets editor/assets tests/unit/test_asset_library.cpp tests/unit/test_asset_library_panel.cpp content/schemas/asset_promotion_manifest.schema.json docs/features/FEATURE_ROBUSTNESS_PLAN.md docs/asset_intake docs/release/RELEASE_PACKAGING.md
git commit -m "feat: add governed asset promotion manifests"
```

### Task 3.2: Connect Character Appearance Parts To Promoted Assets

**Why:** `character_identity` is partial mainly because broader authored asset-library tooling for creator-supplied appearance part imports remains backlog.

**Files:**
- Modify: `engine/core/character/character_appearance_composition.*`
- Modify: `engine/core/character/character_identity.*`
- Modify: `editor/character/character_creator_model.*`
- Modify: `editor/character/character_creator_panel.*`
- Modify: `tests/unit/test_character_creator_model.cpp`
- Modify: `tests/unit/test_character_creator_panel.cpp`
- Modify: `content/schemas/character_identity.schema.json`

- [x] **Step 1: Add tests for promoted appearance part selection**

Test cases:

- A promoted runtime-ready portrait layer can be selected.
- A promoted field sprite can be selected.
- An unsupported or archived asset is shown but disabled with a reason.
- The selected promoted asset ids persist in character identity JSON.

Example assertion shape:

```cpp
REQUIRE(snapshot.appearance_parts.size() >= 2);
REQUIRE(snapshot.appearance_parts[0].asset_id == "asset.character.hero.portrait.hair_01");
REQUIRE(snapshot.appearance_parts[0].enabled);
REQUIRE(snapshot.appearance_parts[1].asset_id == "asset.character.hero.raw_unlicensed");
REQUIRE_FALSE(snapshot.appearance_parts[1].enabled);
REQUIRE(snapshot.appearance_parts[1].disabled_reason == "Asset is not runtime-ready or lacks license evidence.");
```

- [x] **Step 2: Run focused character tests and confirm failure**

Run:

```powershell
ctest --preset dev-all -R "CharacterCreator|character.*identity|character.*appearance" --output-on-failure
```

Expected: FAIL for missing promoted-asset selection rows.

Execution note: focused character tests now cover promoted portrait/field/layer selection, disabled unlicensed/archived rows, and identity JSON persistence.

- [x] **Step 3: Add appearance part references to character identity**

Add fields:

```cpp
std::string portraitAssetId;
std::string fieldSpriteAssetId;
std::string battleSpriteAssetId;
std::vector<std::string> layeredPartAssetIds;
```

Serialization must preserve existing identity fields and old saves without appearance part ids.

- [x] **Step 4: Connect `CharacterCreatorModel` to asset promotion snapshots**

The model should accept promoted asset rows from `AssetLibraryModelSnapshot` or a narrow adapter. Do not make the character model scan raw intake folders.

- [x] **Step 5: Render disabled reasons in panel snapshots**

Add snapshot rows:

```cpp
struct CharacterAppearancePartRow {
    std::string asset_id;
    std::string label;
    std::string slot;
    bool enabled = false;
    std::string disabled_reason;
    std::string preview_kind;
};
```

- [x] **Step 6: Re-run focused tests**

Run:

```powershell
ctest --preset dev-all -R "CharacterCreator|character.*identity|character.*appearance" --output-on-failure
```

Expected: PASS.

- [x] **Step 7: Update readiness**

Update:

- `docs/release/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`

If this closes the only true `character_identity` gap, promote cautiously only if evidence fields remain true and docs agree.

- [x] **Step 8: Commit**

```powershell
git add engine/core/character editor/character tests/unit/test_character_creator_model.cpp tests/unit/test_character_creator_panel.cpp content/schemas/character_identity.schema.json docs/release/RELEASE_READINESS_MATRIX.md content/readiness/readiness_status.json
git commit -m "feat: bind character appearance to promoted assets"
```

---

## Phase 4 - Close Template Cross-Cutting Bars

### Task 4.1: Add Template-Specific Input Closure For JRPG And Turn-Based RPG

**Why:** Both `jrpg` and `turn_based_rpg` remain partial partly because full binding closure for menu and battle actions is not proven end-to-end.

**Files:**
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/templates/jrpg_spec.md`
- Modify: `docs/templates/turn_based_rpg_spec.md`
- Modify: `tests/unit/test_template_acceptance.cpp`
- Modify: `tests/unit/test_input_remap_store.cpp`
- Modify if needed: `engine/core/input/input_remap_store.*`

- [ ] **Step 1: Define required action set in tests**

Required actions:

```text
MoveUp
MoveDown
MoveLeft
MoveRight
Confirm
Cancel
Menu
PageLeft
PageRight
BattleAttack
BattleSkill
BattleItem
BattleDefend
BattleEscape
```

- [ ] **Step 2: Add failing template acceptance tests**

Add assertions that generated `jrpg` and `turn_based_rpg` starter projects include bindings for every required action and that each action survives save/load through `InputRemapStore`.

- [ ] **Step 3: Run focused tests**

Run:

```powershell
ctest --preset dev-all -R "template.*acceptance|InputRemapStore" --output-on-failure
```

Expected: FAIL until missing actions are wired.

- [ ] **Step 4: Add missing bindings to template manifests**

Edit the relevant `content/templates/` manifests. Bind keyboard and controller defaults. Keep touch as hit-test UI/world behavior and continue reporting `touch_binding_unsupported` for remap profiles.

- [ ] **Step 5: Re-run focused tests**

Run:

```powershell
ctest --preset dev-all -R "template.*acceptance|InputRemapStore" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update template docs and readiness bar language**

Update `input` bars for `jrpg` and `turn_based_rpg`. Promote only the input bar if the tests prove closure; do not promote the whole template until accessibility/audio/localization/performance bars are also closed.

- [ ] **Step 7: Commit**

```powershell
git add content/templates content/readiness/readiness_status.json docs/templates/jrpg_spec.md docs/templates/turn_based_rpg_spec.md tests/unit/test_template_acceptance.cpp tests/unit/test_input_remap_store.cpp
git commit -m "feat: close RPG template input bindings"
```

### Task 4.2: Add Template Localization Completeness Metrics

**Why:** The three partial templates all call out missing localization completeness metrics and font-set coverage.

**Files:**
- Modify: `engine/core/localization/locale_catalog.*`
- Create if needed: `engine/core/localization/template_localization_audit.h`
- Create if needed: `engine/core/localization/template_localization_audit.cpp`
- Modify: `tests/unit/test_locale_catalog.cpp`
- Modify: `tests/unit/test_template_bar_quality.cpp`
- Modify: `docs/templates/*.md`
- Modify: `content/readiness/readiness_status.json`

- [ ] **Step 1: Add failing tests for required localization keys**

For `jrpg`, `visual_novel`, and `turn_based_rpg`, assert:

- Required UI/menu keys exist.
- Battle text keys exist for battle templates.
- Dialogue speaker/name interpolation keys exist for visual novel.
- Font profile id is present for each locale bundle.
- Missing key count is surfaced as diagnostics, not ignored.

- [ ] **Step 2: Run focused tests**

Run:

```powershell
ctest --preset dev-all -R "locale|template.*bar" --output-on-failure
```

Expected: FAIL until audit and fixture data exist.

- [ ] **Step 3: Implement `TemplateLocalizationAudit`**

Result shape:

```cpp
struct TemplateLocalizationAuditResult {
    std::string template_id;
    std::size_t required_key_count = 0;
    std::size_t missing_key_count = 0;
    std::size_t missing_font_profile_count = 0;
    std::vector<std::string> missing_keys;
    std::vector<std::string> diagnostics;
};
```

- [ ] **Step 4: Add required key lists to template manifests**

Store required keys in template manifest data, not hardcoded in tests. The test should verify the manifest-driven audit.

- [ ] **Step 5: Re-run focused tests**

Run:

```powershell
ctest --preset dev-all -R "locale|template.*bar" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update docs/readiness**

Update localization bars for each affected template.

- [ ] **Step 7: Commit**

```powershell
git add engine/core/localization tests/unit/test_locale_catalog.cpp tests/unit/test_template_bar_quality.cpp content/templates content/readiness/readiness_status.json docs/templates
git commit -m "feat: add template localization audits"
```

### Task 4.3: Add Template Performance And Accessibility Evidence

**Why:** Template bars should not rely only on general subsystem tests. Each partial template needs template-shaped performance and accessibility proof.

**Files:**
- Modify: `tests/unit/test_template_bar_quality.cpp`
- Modify: `tests/unit/test_accessibility_auditor.cpp`
- Modify: `tests/unit/test_frame_budget.cpp`
- Modify: `content/templates/`
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/templates/*.md`

- [ ] **Step 1: Add template accessibility fixtures**

For each partial template, add fixture rows for:

- Primary menu scene labels.
- Battle command labels for battle templates.
- Visual novel backlog, skip, auto-advance, and read-speed controls.
- Color contrast pairs that are derived from render command text/rect surfaces where possible.

- [ ] **Step 2: Add performance budget fixtures**

For each partial template, add deterministic frame-budget fixtures:

- `jrpg`: map scene plus menu overlay.
- `turn_based_rpg`: battle sequence with representative troop and command menu.
- `visual_novel`: dialogue-heavy scene with backlog and branching graph traversal.

- [ ] **Step 3: Run focused tests**

Run:

```powershell
ctest --preset dev-all -R "accessibility|frame_budget|template.*bar" --output-on-failure
```

Expected: FAIL until fixtures and audit wiring are complete.

- [ ] **Step 4: Wire fixtures into template certification**

Update the certification loop so missing template accessibility/performance fixtures fail with explicit template id and missing bar name.

- [ ] **Step 5: Re-run focused tests**

Run:

```powershell
ctest --preset dev-all -R "accessibility|frame_budget|template.*bar" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update docs/readiness**

Update only the bars that have direct test evidence.

- [ ] **Step 7: Commit**

```powershell
git add tests/unit/test_template_bar_quality.cpp tests/unit/test_accessibility_auditor.cpp tests/unit/test_frame_budget.cpp content/templates content/readiness/readiness_status.json docs/templates
git commit -m "test: add template accessibility and performance evidence"
```

---

## Phase 5 - Export And Public Release Hardening

### Task 5.1: Add Explicit Release Signing/Notarization Policy Gates

**Why:** `export_validator` remains partial because full native signing/notarization, broader platform packaging, and public artifact policy are not closed.

**Files:**
- Modify: `engine/core/export/export_validator.*`
- Modify: `engine/core/tools/export_packager.*`
- Modify: `tests/unit/test_export_validator.cpp`
- Modify: `tests/unit/test_export_packager.cpp`
- Modify: `docs/release/RELEASE_PACKAGING.md`
- Modify: `docs/release/AAA_RELEASE_READINESS_REPORT.md`

- [ ] **Step 1: Add failing validator tests**

Test expected behavior:

- `ExportMode::DevBootstrap` allows unsigned marker/bootstrap artifacts and labels them non-production.
- `ExportMode::ReleaseCandidate` requires signing policy fields.
- `ExportMode::PublicRelease` requires signing and notarization policy evidence for platforms that need it.
- Missing signing evidence fails with `export_signing_policy_missing`.
- Missing notarization evidence fails with `export_notarization_policy_missing`.

- [ ] **Step 2: Run focused export tests**

Run:

```powershell
ctest --preset dev-all -R "ExportPackager|export_validator" --output-on-failure
```

Expected: FAIL until policy fields exist.

- [ ] **Step 3: Add policy model**

Add:

```cpp
struct ExportSigningPolicy {
    bool signingRequired = false;
    bool signingEvidencePresent = false;
    bool notarizationRequired = false;
    bool notarizationEvidencePresent = false;
    std::string signingProfileId;
    std::string notarizationProfileId;
};
```

- [ ] **Step 4: Wire policy checks into validator**

Validator must emit structured diagnostics and must not try to perform real signing in this task.

- [ ] **Step 5: Re-run export tests**

Run:

```powershell
ctest --preset dev-all -R "ExportPackager|export_validator" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update release docs**

Update docs to say the tool now enforces policy evidence, while actual platform account credentials and signing operations remain external release-owner responsibility unless implemented later.

- [ ] **Step 7: Commit**

```powershell
git add engine/core/export engine/core/tools tests/unit/test_export_validator.cpp tests/unit/test_export_packager.cpp docs/release/RELEASE_PACKAGING.md docs/release/AAA_RELEASE_READINESS_REPORT.md
git commit -m "feat: enforce export signing policy evidence"
```

### Task 5.2: Make Legal/Public Release Blockers Machine-Visible

**Why:** Public release is blocked by legal review. The editor and audit tooling should surface that as a first-class release checklist item, not just a document note.

**Files:**
- Modify: `tools/audit/project_audit_issue_collectors.*`
- Modify: `tools/audit/project_completeness_score.*`
- Modify: `tests/unit/test_project_audit_cli.cpp`
- Modify: `docs/release/LEGAL_REVIEW_SIGNOFF.md`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`

- [ ] **Step 1: Add failing audit test**

Test:

- Internal/private RC approval does not satisfy public release.
- Missing qualified legal review emits `release_legal_review_missing`.
- Missing public-release waiver emits `release_public_waiver_missing`.
- Audit JSON includes blocker severity for public release mode.

- [ ] **Step 2: Run focused audit tests**

Run:

```powershell
ctest --preset dev-all -R "ProjectAudit|release.*legal|project_audit" --output-on-failure
```

Expected: FAIL until audit collectors inspect legal signoff fields.

- [ ] **Step 3: Add signoff parser**

Use structured metadata from `docs/release/LEGAL_REVIEW_SIGNOFF.md` if it already exists. If it is purely prose, add a small adjacent JSON file:

```json
{
  "schemaVersion": "1.0.0",
  "privateRcApproved": true,
  "publicReleaseApproved": false,
  "publicReleaseWaiver": false,
  "reviewedByQualifiedCounsel": false
}
```

Preferred path: `docs/release/legal_review_status.json`.

- [ ] **Step 4: Wire audit collector**

Audit result should include:

```json
{
  "code": "release_legal_review_missing",
  "severity": "blocker",
  "path": "docs/release/legal_review_status.json",
  "message": "Public release requires qualified legal/privacy review or explicit public-release waiver."
}
```

- [ ] **Step 5: Re-run focused tests**

Run:

```powershell
ctest --preset dev-all -R "ProjectAudit|release.*legal|project_audit" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update docs**

Update app release matrix and signoff docs to reference the machine-readable legal status file.

- [ ] **Step 7: Commit**

```powershell
git add tools/audit tests/unit/test_project_audit_cli.cpp docs/release/LEGAL_REVIEW_SIGNOFF.md docs/release/legal_review_status.json docs/APP_RELEASE_READINESS_MATRIX.md
git commit -m "feat: surface legal review blockers in audit"
```

---

## Phase 6 - Governance And Final Alignment

### Task 6.1: Add A Partial-Lane Closure Gate

**Why:** The repo already enforces truthfulness. Add a narrower gate that verifies every `PARTIAL` lane has a next vertical slice, deferred scope, owner track, evidence command, and docs link.

**Files:**
- Modify: `content/readiness/partial_lane_vertical_slices.json`
- Create or modify: `tools/ci/check_feature_governance.ps1`
- Modify: `tests/unit/test_governance_regression.cpp`
- Modify: `docs/agent/QUALITY_GATES.md`

- [ ] **Step 1: Add failing governance test**

Test each `PARTIAL` subsystem from `readiness_status.json` has a matching entry in `partial_lane_vertical_slices.json` with:

- `id`
- `ownerTrack`
- `nextVerticalSlice`
- `deferredScope`
- `evidenceCommand`
- `docs`

- [ ] **Step 2: Run focused governance tests**

Run:

```powershell
ctest --preset dev-all -R "governance|readiness_status|feature_governance" --output-on-failure
```

Expected: FAIL because existing partial slice rows do not all carry `evidenceCommand` and `docs`.

- [ ] **Step 3: Extend JSON rows**

Example:

```json
{
  "id": "gameplay_ability_framework",
  "ownerTrack": "Gameplay ability",
  "nextVerticalSlice": "Bounded active-condition evaluator plus authored branch task composition.",
  "deferredScope": "Arbitrary scripting and external plugin compatibility remain out of scope unless explicitly planned.",
  "evidenceCommand": "ctest --preset dev-all -R \"Ability|GameplayAbility|AbilityTask\" --output-on-failure",
  "docs": [
    "docs/release/RELEASE_READINESS_MATRIX.md",
    "docs/features/FEATURE_ROBUSTNESS_PLAN.md"
  ]
}
```

- [ ] **Step 4: Implement PowerShell check**

`tools/ci/check_feature_governance.ps1` should:

- Parse both JSON files.
- Compare partial ids.
- Fail if any row lacks required fields.
- Fail if evidence command is empty.
- Fail if docs path does not exist.

- [ ] **Step 5: Wire into local gates only after focused pass**

Add the script to the appropriate existing local gate after it passes independently.

- [ ] **Step 6: Run gates**

Run:

```powershell
.\tools\ci\check_feature_governance.ps1
ctest --preset dev-all -R "governance|readiness_status|feature_governance" --output-on-failure
```

Expected: PASS.

- [ ] **Step 7: Commit**

```powershell
git add content/readiness/partial_lane_vertical_slices.json tools/ci/check_feature_governance.ps1 tests/unit/test_governance_regression.cpp docs/agent/QUALITY_GATES.md
git commit -m "test: govern partial lane closure plans"
```

### Task 6.2: Run The Right Final Gates

**Why:** After cross-cutting work, the final answer must be based on actual verification, not the plan.

**Files:**
- No code files expected unless a gate fails.
- Update docs only if verification evidence changes documented status.

- [ ] **Step 1: Run focused gates for touched surfaces**

Run the commands that correspond to completed phases:

```powershell
ctest --preset dev-all -R "Ability|GameplayAbility|AbilityTask" --output-on-failure
.\tools\ci\run_presentation_gate.ps1
ctest --preset dev-all -R "AssetLibrary|CharacterCreator|template.*bar|ExportPackager|export_validator|ProjectAudit" --output-on-failure
```

- [ ] **Step 2: Run PR-level gate**

Run:

```powershell
ctest --preset dev-all -L pr --output-on-failure
```

Expected: PASS.

- [ ] **Step 3: Run full local gate only after focused and PR gates pass**

Run:

```powershell
.\tools\ci\run_local_gates.ps1
```

Expected: PASS.

- [ ] **Step 4: If release/package behavior changed, run release gates**

Run:

```powershell
.\tools\ci\run_release_candidate_gate.ps1
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

Expected: PASS, unless public legal review remains intentionally blocked. Public release blockers must stay visible in docs and audit output.

- [ ] **Step 5: Update final evidence docs**

Update:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/APP_RELEASE_READINESS_MATRIX.md`
- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/release/AAA_RELEASE_READINESS_REPORT.md`
- `content/readiness/readiness_status.json`

Record exact commands run and results. Do not state a gate passed unless it was run in this branch.

- [ ] **Step 6: Commit final alignment**

```powershell
git add docs content/readiness
git commit -m "docs: align robustness completion evidence"
```

---

## Suggested Execution Order

The safest order is:

1. Phase 0 baseline.
2. Phase 2 ability evaluator and task composition.
3. Phase 1 visual regression breadth.
4. Phase 3 asset promotion and character appearance.
5. Phase 4 template cross-cutting bars.
6. Phase 5 export/legal hardening.
7. Phase 6 governance and final gates.

Reasoning:

- Ability work is high-value and localized.
- Visual regression work may require golden regeneration and should be isolated.
- Asset/character work touches creator flows and should happen before template closure.
- Template closure depends on asset/input/localization/accessibility evidence.
- Export/legal hardening affects public release status and should not be mixed with gameplay commits.

## Definition Of Done For This Plan

This plan is complete when:

- Every modified subsystem has focused tests.
- Every changed readiness claim has matching code, tests, docs, and gate evidence.
- No new `ReleaseTopLevel` panel lacks app-shell wiring or smoke coverage.
- Public release blockers remain visible until actually resolved.
- `ctest --preset dev-all -L pr --output-on-failure` passes.
- `.\tools\ci\run_local_gates.ps1` passes or any failure is documented as an external blocker with exact logs.

## Explicit Non-Goals

- Do not add arbitrary script execution for ability conditions in this plan.
- Do not promote raw intake folders into runtime packages without promotion manifests and license evidence.
- Do not claim public release readiness unless legal/privacy/distribution review is actually approved or explicitly waived.
- Do not solve proprietary store SDK credentialing in-tree.
- Do not rewrite the editor navigation model; use `engine/core/editor/editor_panel_registry.*` and app-shell factories.
