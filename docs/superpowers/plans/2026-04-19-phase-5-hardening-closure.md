# Phase 5 Hardening Closure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the remaining Phase 5 remediation work by turning the already-landed ownership/concurrency fixes into audited, gated, release-ready closure evidence.

**Architecture:** Phase 5 no longer needs broad feature work. The repo already landed the core fixes for plugin callback marshalling, `MapScene` audio ownership, retained tile render caching, and the presentation-polish tasks in `PLAN.md`. The remaining work is a hardening pass: audit all callback delivery sites, remove residual implicit ownership shortcuts, prove delta-proportional render behavior through focused validation, strengthen Phase 4 intake-governance gates so Phase 5 cannot regress them, then reconcile the canonical status docs and release checklist from one evidence set.

**Tech Stack:** C++17, Catch2, CMake/CTest, PowerShell, markdown governance docs

---

## File Structure

- `runtimes/compat_js/plugin_manager.h`
  Responsible for the public async threading contract and method-level deviation language.
- `runtimes/compat_js/plugin_manager.cpp`
  Responsible for worker-thread execution, deferred callback queueing, and owning-thread-only drain behavior.
- `tests/unit/test_plugin_manager.cpp`
  Responsible for regressions proving FIFO callback delivery, foreign-thread rejection, and queue integrity through error/shutdown paths.
- `engine/core/scene/map_scene.h`
  Responsible for `MapScene` runtime ownership boundaries, injected audio-service access, and retained render-cache state.
- `engine/core/scene/map_scene.cpp`
  Responsible for actual audio-service binding, retained tile-command rebuild behavior, and render submission during update.
- `tests/unit/test_scene_manager.cpp`
  Responsible for focused ownership and retained-render behavior regressions in the scene lane.
- `engine/core/presentation/release_validation.cpp`
  Responsible for executable presentation hardening evidence and command-count sanity checks.
- `tests/unit/test_spatial_editor.cpp`
  Responsible for editor/presentation integration assertions that `BuildPresentationFrame()` still emits the expected environment-command surface.
- `docs/presentation/performance_budgets.md`
  Responsible for the documented interpretation of the retained-render budget and current measured baseline.
- `tools/ci/check_phase4_intake_governance.ps1`
  Responsible for preventing regression of the Phase 4 governance gates that Phase 5 depends on.
- `docs/external-intake/license-matrix.md`
  Responsible for canonical repo dispositions and allowed adoption modes.
- `docs/asset_intake/ASSET_PROMOTION_GUIDE.md`
  Responsible for the product-lane promotion contract for private-use assets.
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
  Canonical remediation hub and Phase 5 authority.
- `docs/PROGRAM_COMPLETION_STATUS.md`
  Canonical current-state status snapshot.
- `RELEASE_CHECKLIST.md`
  Release-readiness checklist that should explicitly reflect the final Phase 5 gate set.
- `WORKLOG.md`
  Narrative record of the hardening pass and the validation evidence used to close it.

### Task 1: Complete the Plugin Callback Thread-Affinity Audit

**Files:**
- Modify: `runtimes/compat_js/plugin_manager.h`
- Modify: `runtimes/compat_js/plugin_manager.cpp`
- Modify: `tests/unit/test_plugin_manager.cpp`
- Modify: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`

- [ ] **Step 1: Add a failing audit regression for callback-queue integrity after a rejected foreign-thread drain**

```cpp
SECTION("Async callbacks remain queued after rejected foreign-thread drain until owning thread dispatches them") {
    pm.registerPlugin({"AsyncPlugin", "1.0", "", ""});
    pm.registerCommand(
        "AsyncPlugin",
        "echoInt",
        [](const std::vector<urpg::Value>& args) -> urpg::Value {
            return args.empty() ? urpg::Value::Int(-1) : args.front();
        }
    );

    std::mutex callbackMutex;
    std::condition_variable callbackCv;
    std::vector<int64_t> callbackValues;

    pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(7)}, [&](const urpg::Value& result) {
        std::lock_guard<std::mutex> lock(callbackMutex);
        callbackValues.push_back(std::get<int64_t>(result.v));
        callbackCv.notify_one();
    });
    pm.executeCommandAsync("AsyncPlugin", "echoInt", {urpg::Value::Int(8)}, [&](const urpg::Value& result) {
        std::lock_guard<std::mutex> lock(callbackMutex);
        callbackValues.push_back(std::get<int64_t>(result.v));
        callbackCv.notify_one();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    std::thread foreignDispatcher([&]() {
        REQUIRE(pm.dispatchPendingAsyncCallbacks() == 0);
    });
    foreignDispatcher.join();

    REQUIRE(pm.getLastError() == "dispatchPendingAsyncCallbacks must be called on the owning thread");
    REQUIRE(pm.dispatchPendingAsyncCallbacks() == 2);
    REQUIRE(callbackCv.wait_for(
        std::unique_lock<std::mutex>(callbackMutex),
        std::chrono::seconds(2),
        [&]() { return callbackValues.size() == 2; }
    ));
    REQUIRE(callbackValues[0] == 7);
    REQUIRE(callbackValues[1] == 8);
}
```

- [ ] **Step 2: Run the focused plugin-manager lane to verify the new audit case fails before implementation**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PluginManager: Command execution"
```

Expected: FAIL in the new callback-queue-integrity section because the current audit coverage is incomplete.

- [ ] **Step 3: Tighten the async contract comments and queue/drain implementation if the audit exposes any unguarded delivery path**

```cpp
// Threading contract:
// - executeCommandAsync() runs handlers on the worker thread
// - completed callbacks are queued, never delivered directly from the worker
// - dispatchPendingAsyncCallbacks() may only be called from the owning thread
// - rejected foreign-thread drain attempts must not consume or reorder queued callbacks
void executeCommandAsync(...);

int32_t PluginManager::dispatchPendingAsyncCallbacks() {
    if (std::this_thread::get_id() != owningThreadId_) {
        lastError_ = "dispatchPendingAsyncCallbacks must be called on the owning thread";
        return 0;
    }

    std::queue<PendingCallback> drained;
    {
        std::lock_guard<std::mutex> lock(completedCallbacksMutex_);
        drained.swap(completedCallbacks_);
    }

    int32_t dispatched = 0;
    while (!drained.empty()) {
        auto pending = std::move(drained.front());
        drained.pop();
        pending.callback(pending.result);
        ++dispatched;
    }
    return dispatched;
}
```

- [ ] **Step 4: Update the remediation evidence block with the callback-site audit result**

```md
- Async callback audit (2026-04-19): `executeCommandAsync()` is the only worker-thread completion path; completed results are queued and can only be drained through `dispatchPendingAsyncCallbacks()` on the owning thread.
- `tests/unit/test_plugin_manager.cpp` now proves rejected foreign-thread drains preserve FIFO queue contents until the owning thread dispatches them.
```

- [ ] **Step 5: Re-run the focused plugin-manager lane and record the exact passing command**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PluginManager: Command execution"
```

Expected: PASS with the async FIFO, foreign-thread rejection, and queue-integrity sections all green.

- [ ] **Step 6: Commit**

```bash
git add runtimes/compat_js/plugin_manager.h runtimes/compat_js/plugin_manager.cpp tests/unit/test_plugin_manager.cpp docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md
git commit -m "test: finish plugin callback thread-affinity audit"
```

### Task 2: Remove Residual Implicit Ownership from `MapScene`

**Files:**
- Modify: `engine/core/scene/map_scene.h`
- Modify: `engine/core/scene/map_scene.cpp`
- Modify: `tests/unit/test_scene_manager.cpp`

- [ ] **Step 1: Add a failing test that proves `MapScene` uses an explicit audio binding path instead of silently depending on constructor-created global-ish state**

```cpp
TEST_CASE("MapScene: injected AudioCore can be replaced without stale state leakage", "[scene][map][audio]") {
    MapScene map("001", 10, 10);
    auto firstAudio = std::make_shared<urpg::audio::AudioCore>();
    auto secondAudio = std::make_shared<urpg::audio::AudioCore>();

    map.setAudioCore(firstAudio);
    map.processAiAudioCommands("[ACTION: CROSSFADE, ASSET: Boss_Dark, VOL: 0.8, FADE: 3.0]");
    REQUIRE(firstAudio->currentBGM() == "Boss_Dark");

    map.setAudioCore(secondAudio);
    map.processAiAudioCommands("[ACTION: PLAY_SE, ASSET: Confirm_01, VOL: 1.0, FADE: 0.0]");

    REQUIRE(firstAudio->activeSourceCount() == 0);
    REQUIRE(secondAudio->activeSourceCount() == 1);
}
```

- [ ] **Step 2: Run the focused scene lane to verify whether the current `MapScene` constructor/setup path still hides ownership assumptions**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MapScene:|SceneManager:"
```

Expected: FAIL if the current constructor-created `AudioCore` instance leaks behavior across rebinding or if the test reveals stale scene ownership.

- [ ] **Step 3: Convert `MapScene` to an explicit, swappable audio-service binding model**

```cpp
class MapScene : public GameScene {
public:
    void setAudioCore(std::shared_ptr<urpg::audio::AudioCore> audioCore);
    std::shared_ptr<urpg::audio::AudioCore> audioCore() const { return m_audioCore; }
private:
    std::shared_ptr<urpg::audio::AudioCore> m_audioCore;
};

void MapScene::setAudioCore(std::shared_ptr<urpg::audio::AudioCore> audioCore) {
    m_audioCore = std::move(audioCore);
}

void MapScene::processAiAudioCommands(const std::string& aiResponse) {
    auto commands = urpg::ai::AudioKnowledgeBridge::parseAudioCommands(aiResponse);
    if (commands.empty() || !m_audioCore) {
        return;
    }
    // route commands only through the currently bound service
}
```

- [ ] **Step 4: Re-run the focused scene tests and expand the current audio-injection coverage if needed**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MapScene: AI audio commands use injected AudioCore service|MapScene: injected AudioCore can be replaced without stale state leakage|SceneManager:"
```

Expected: PASS with both the original injection test and the rebinding regression green.

- [ ] **Step 5: Commit**

```bash
git add engine/core/scene/map_scene.h engine/core/scene/map_scene.cpp tests/unit/test_scene_manager.cpp
git commit -m "refactor: harden map scene audio ownership"
```

### Task 3: Turn Retained Render Caching into Measurable Phase 5 Evidence

**Files:**
- Modify: `tests/unit/test_scene_manager.cpp`
- Modify: `engine/core/presentation/release_validation.cpp`
- Modify: `tests/unit/test_spatial_editor.cpp`
- Modify: `docs/presentation/performance_budgets.md`

- [ ] **Step 1: Add a failing test that proves unchanged `MapScene` frames do not rebuild retained tile commands across multiple updates**

```cpp
TEST_CASE("MapScene: retained tile commands stay pointer-stable across unchanged frames", "[scene][map][render]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("001", 2, 2);
    map.onUpdate(0.0f);
    const auto firstFrame = layer.getCommands();

    map.onUpdate(0.0f);
    const auto secondFrame = layer.getCommands();

    REQUIRE(secondFrame.size() == firstFrame.size());
    for (size_t i = 0; i < 4; ++i) {
        REQUIRE(secondFrame[i].get() == firstFrame[i].get());
    }
}
```

- [ ] **Step 2: Run the focused scene/presentation lanes and confirm the new retained-cache assertion fails before hardening**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MapScene: retained tile|presentation_(unit_lane|release_validation)|spatial_editor_lane"
```

Expected: FAIL if the current tests only prove value equality and do not yet prove object reuse or measurable retained behavior.

- [ ] **Step 3: Extend release validation so it emits explicit environment-command and retained-work checks that match the current Phase 5 runtime**

```cpp
size_t actorCommandCount = 0;
size_t fogCommandCount = 0;
size_t postFxCommandCount = 0;
size_t lightCommandCount = 0;

for (const auto& cmd : intent.commands) {
    if (cmd.type == PresentationCommand::Type::DrawActor) {
        ++actorCommandCount;
    } else if (cmd.type == PresentationCommand::Type::SetFog) {
        ++fogCommandCount;
    } else if (cmd.type == PresentationCommand::Type::SetPostFX) {
        ++postFxCommandCount;
    } else if (cmd.type == PresentationCommand::Type::SetLight) {
        ++lightCommandCount;
    }
}

std::cout << "[CHECK] Environment Commands: fog=" << fogCommandCount
          << " postfx=" << postFxCommandCount
          << " lights=" << lightCommandCount << std::endl;
assert(fogCommandCount <= 1);
assert(postFxCommandCount <= 1);
```

- [ ] **Step 4: Update the performance-budget doc so the retained-render contract is explicit instead of implied**

```md
### MapScene Retained Render Contract
- Tile render commands are retained and only rebuilt when tile/passability data changes.
- Unchanged frames must reuse the existing retained tile command objects.
- Validation evidence:
  - `tests/unit/test_scene_manager.cpp` proves unchanged frames keep the same retained tile-command pointers.
  - `engine/core/presentation/release_validation.cpp` verifies actor/environment command counts remain within the current Phase 5 command envelope.
```

- [ ] **Step 5: Re-run the focused presentation and scene lanes and record the exact passing commands**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "MapScene: retained tile|presentation_(unit_lane|release_validation)|spatial_editor_lane"
ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure
```

Expected: PASS, with retained-cache pointer stability and current Phase 5 environment command counts both verified.

- [ ] **Step 6: Commit**

```bash
git add tests/unit/test_scene_manager.cpp engine/core/presentation/release_validation.cpp tests/unit/test_spatial_editor.cpp docs/presentation/performance_budgets.md
git commit -m "test: add measurable phase 5 render hardening evidence"
```

### Task 4: Expand the Governance Gate So Phase 5 Cannot Reopen Phase 4 Debt

**Files:**
- Modify: `tools/ci/check_phase4_intake_governance.ps1`
- Modify: `docs/external-intake/license-matrix.md`
- Modify: `docs/asset_intake/ASSET_PROMOTION_GUIDE.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`

- [ ] **Step 1: Add a failing gate assertion for “production candidate” adoption without wrapper/facade language and promotion provenance**

```powershell
$assetPromotionGuide = Get-Content "docs/asset_intake/ASSET_PROMOTION_GUIDE.md" -Raw
if ($assetPromotionGuide -notmatch "promotion record" -or
    $assetPromotionGuide -notmatch "source manifest" -or
    $assetPromotionGuide -notmatch "bundle manifest") {
    Write-Host "Asset promotion guide does not require provenance-preserving promotion records."
    $hasError = $true
}

$licenseMatrix = Get-Content "docs/external-intake/license-matrix.md" -Raw
if ($licenseMatrix -notmatch "wrapper" -or $licenseMatrix -notmatch "facade") {
    Write-Host "External intake governance does not explicitly require wrappers/facades for production-candidate adoption."
    $hasError = $true
}
```

- [ ] **Step 2: Run the Phase 4 governance check to verify the new assertions fail before the doc/gate updates**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1
```

Expected: FAIL until the canonical governance docs state the wrapper/facade and provenance requirements explicitly enough for the new checks.

- [ ] **Step 3: Update the governance docs and gate together**

```md
Production-candidate external repositories may only enter URPG through URPG-owned wrappers/facades.
Direct architecture absorption is not an allowed Phase 5 closure path.
Promoted private-use assets must retain:
- source manifest
- bundle manifest
- promotion record
- attribution/provenance linkage
```

- [ ] **Step 4: Re-run the governance gate and capture the exact passing command**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1
```

Expected: PASS with the stronger wrapper/facade and provenance checks active.

- [ ] **Step 5: Commit**

```bash
git add tools/ci/check_phase4_intake_governance.ps1 docs/external-intake/license-matrix.md docs/asset_intake/ASSET_PROMOTION_GUIDE.md docs/PROGRAM_COMPLETION_STATUS.md
git commit -m "docs: harden phase 4 governance gates for phase 5 closure"
```

### Task 5: Publish the Phase 5 Closure Pack

**Files:**
- Modify: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `RELEASE_CHECKLIST.md`
- Modify: `WORKLOG.md`

- [ ] **Step 1: Add the final failing documentation check by searching for stale “next unfinished remediation phase is Phase 5” language before changing status**

Run:

```powershell
rg -n "next unfinished remediation phase is Phase 5|Phase 5 hardening work|Phase 5 hardening follow-through" docs WORKLOG.md PLAN.md RELEASE_CHECKLIST.md -S
```

Expected: FIND the current live Phase 5 wording that must be reconciled as part of closure.

- [ ] **Step 2: Update the canonical remediation and status docs only after all technical and governance lanes are green**

```md
### Phase 5 — Harden Ownership, Concurrency, and Performance

**Status (2026-04-19):** Closed.

**Closure evidence:**
- Plugin callback thread-affinity audit completed and enforced.
- `MapScene` audio ownership and retained-render behavior verified through focused scene regressions.
- Presentation release validation reflects the current environment-command envelope.
- Phase 4 intake governance gate now blocks wrapper/facade and provenance regressions.
```

- [ ] **Step 3: Update `RELEASE_CHECKLIST.md` so the final Phase 5 release gate is explicit**

```md
- [ ] Focused plugin callback audit lane passes.
- [ ] Focused `MapScene` ownership/render lane passes.
- [ ] Focused presentation gate passes.
- [ ] Phase 4 intake-governance validation passes.
- [ ] Canonical status docs match the validated closure state.
```

- [ ] **Step 4: Run the full verification set before declaring Phase 5 complete**

Run:

```powershell
ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PluginManager: Command execution|MapScene:|SceneManager:"
ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1
ctest --preset dev-all --output-on-failure
```

Expected: PASS for all four commands. Do not mark Phase 5 closed until all four are green from the same final pass.

- [ ] **Step 5: Record the closure in `WORKLOG.md`**

```md
### 2026-04-19 — Phase 5 Hardening Closure
- **Action**: Completed the plugin callback thread-affinity audit, hardened `MapScene` ownership evidence, expanded presentation release validation, and strengthened the Phase 4 intake-governance gate.
- **Action**: Re-ran the focused plugin/scene/presentation/governance commands and the repo-wide `ctest --preset dev-all --output-on-failure` lane.
- **Result**: Phase 5 is now CLOSED and the canonical status/docs/release checklist reflect the same validation snapshot.
```

- [ ] **Step 6: Commit**

```bash
git add docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md docs/PROGRAM_COMPLETION_STATUS.md RELEASE_CHECKLIST.md WORKLOG.md
git commit -m "docs: close phase 5 hardening"
```

## Self-Review

- Spec coverage: this plan covers the remaining Phase 5 closure work still implied by the canonical remediation hub and status stack: plugin callback thread-affinity proof, `MapScene` ownership/render hardening evidence, governance-gate enforcement, and final status/release reconciliation.
- Placeholder scan: no `TODO`, `TBD`, “implement later”, or “write tests for the above” placeholders remain; each task names exact files, test commands, expected outcomes, and commit boundaries.
- Type consistency: all file paths, function names, and command names match the current repo surfaces inspected for this plan, including `dispatchPendingAsyncCallbacks()`, `setAudioCore()`, `processAiAudioCommands()`, and `BuildPresentationFrame()`.
