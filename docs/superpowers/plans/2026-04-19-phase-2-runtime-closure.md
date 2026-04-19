# Phase 2 Runtime Closure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the remaining Phase 2 runtime debt in battle rewards/events, window/data runtime truth, audio semantics reconciliation, and canonical Phase 2 status docs.

**Architecture:** Work lane-by-lane, starting from failing focused tests and only then tightening runtime behavior or stale status text. Keep the deterministic compat harness model intact; implement live compat-state mutation where the runtime already owns the state, and preserve `PARTIAL` where backend or full-MZ parity still does not exist.

**Tech Stack:** C++17, Catch2, CMake, nlohmann::json, URPG compat/runtime layer

---

## File Structure

- `runtimes/compat_js/battle_manager.cpp`
  Responsible for reward application, battle-event updates, switch-condition checks, and status registry notes for the compat battle lane.
- `runtimes/compat_js/battle_manager.h`
  Responsible for the public battle-lane status comments and declarations that must match the implemented behavior.
- `tests/unit/test_battlemgr.cpp`
  Responsible for deterministic coverage of reward application, cadence checks, switch checks, and battle-event behavior.
- `runtimes/compat_js/window_compat.cpp`
  Responsible for deterministic window contents lifecycle and draw-command accumulation behavior.
- `runtimes/compat_js/window_compat.h`
  Responsible for exported window compat status comments and method-level truthfulness.
- `tests/unit/test_window_compat.cpp`
  Responsible for proving the current window contents/draw contract.
- `runtimes/compat_js/data_manager.h`
  Responsible for public comments and status claims on database loading and accessors.
- `tests/unit/test_data_manager.cpp`
  Responsible for proving current loader/accessor behavior and preventing regression to stale “empty container” assumptions.
- `runtimes/compat_js/audio_manager.h`
  Responsible for public audio compat comments and method-level deviation notes.
- `tests/unit/test_audio_manager.cpp`
  Responsible for proving the supported deterministic audio semantics.
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
  Canonical remediation hub that must reflect the final Phase 2 state.
- `docs/PROGRAM_COMPLETION_STATUS.md`
  Status document that must match the implemented and tested Phase 2 closure.
- `WORKLOG.md`
  Narrative record of the landed Phase 2 closure work.

### Task 1: Close Battle Reward and Event Semantics

**Files:**
- Modify: `runtimes/compat_js/battle_manager.cpp`
- Modify: `runtimes/compat_js/battle_manager.h`
- Test: `tests/unit/test_battlemgr.cpp`

- [ ] **Step 1: Write the failing tests**

Add assertions to `tests/unit/test_battlemgr.cpp` that require reward application and switch checks to mutate real compat state instead of merely not crashing.

```cpp
TEST_CASE("BattleManager: defeating all enemies yields expected gold and exp", "[battlemgr]") {
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    ActorData* actor = DataManager::instance().getActor(1);
    REQUIRE(actor != nullptr);
    actor->level = 1;
    actor->exp = 0;

    BattleManager bm;
    bm.setup(2, true, false);
    BattleSubject* enemy = bm.getEnemy(0);
    REQUIRE(enemy != nullptr);
    bm.applyDamage(enemy, enemy->hp);

    const int32_t initialGold = DataManager::instance().getGold();
    bm.applyGold();
    bm.applyExp();

    REQUIRE(DataManager::instance().getGold() == initialGold + 10);
    REQUIRE(actor->exp >= 20);
}

TEST_CASE("BattleManager: switch condition reads live DataManager state", "[battlemgr]") {
    DataManager::instance().setupNewGame();
    BattleManager bm;

    REQUIRE_FALSE(bm.checkSwitchCondition(33));
    DataManager::instance().setSwitch(33, true);
    REQUIRE(bm.checkSwitchCondition(33));
}
```

- [ ] **Step 2: Run the focused battle tests to verify the gap**

Run: `ctest --test-dir build --output-on-failure -R test_battlemgr`

Expected: at least one battle reward or switch-condition assertion fails, or the existing reward test still only proves non-crash behavior.

- [ ] **Step 3: Write the minimal implementation**

Update `runtimes/compat_js/battle_manager.cpp` so reward application writes through `DataManager` and switch conditions read current compat state.

```cpp
void BattleManager::applyExp() {
    const int32_t totalExp = calculateExp();
    if (totalExp <= 0) {
        return;
    }

    auto& dm = DataManager::instance();
    for (auto& actorSubject : actors_) {
        ActorData* actor = dm.getActor(actorSubject.actorId);
        if (!actor) {
            continue;
        }
        gainExp(*actor, totalExp);
        actorSubject.level = actor->level;
    }
}

void BattleManager::applyGold() {
    const int32_t totalGold = calculateGold();
    if (totalGold > 0) {
        DataManager::instance().gainGold(totalGold);
    }
}

void BattleManager::applyDrops() {
    for (const int32_t itemId : calculateDrops()) {
        DataManager::instance().gainItem(itemId, 1);
    }
}

bool BattleManager::checkSwitchCondition(int32_t switchId) {
    return DataManager::instance().getSwitch(switchId);
}
```

- [ ] **Step 4: Run the focused battle tests to verify they pass**

Run: `ctest --test-dir build --output-on-failure -R test_battlemgr`

Expected: PASS for the reward, level-up, cadence, and switch-condition cases in `test_battlemgr.cpp`.

- [ ] **Step 5: Commit**

```bash
git add runtimes/compat_js/battle_manager.cpp runtimes/compat_js/battle_manager.h tests/unit/test_battlemgr.cpp
git commit -m "fix: close compat battle reward application"
```

### Task 2: Tighten Window Contents and Data Truthfulness

**Files:**
- Modify: `runtimes/compat_js/window_compat.cpp`
- Modify: `runtimes/compat_js/window_compat.h`
- Test: `tests/unit/test_window_compat.cpp`
- Modify: `runtimes/compat_js/data_manager.h`
- Test: `tests/unit/test_data_manager.cpp`

- [ ] **Step 1: Write the failing tests**

Add tests that prove the actual contents lifecycle and current data-loader behavior instead of only checking status enums.

```cpp
TEST_CASE("Window_Base contents lifecycle allocates and rotates deterministic handles", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});

    const auto before = window.contents();
    window.createContents();
    const auto created = window.contents();
    REQUIRE(created != 0);
    REQUIRE(created != before);

    window.destroyContents();
    REQUIRE(window.contents() == 0);
}

TEST_CASE("DataManager loadDatabase populates seeded database containers", "[data_manager]") {
    DataManager dm;
    REQUIRE(dm.loadDatabase());
    REQUIRE_FALSE(dm.getActors().empty());
    REQUIRE_FALSE(dm.getSkills().empty());
    REQUIRE(dm.getActor(1) != nullptr);
}
```

- [ ] **Step 2: Run the focused window/data tests to verify the gap**

Run: `ctest --test-dir build --output-on-failure -R "test_window_compat|test_data_manager"`

Expected: either a lifecycle assertion fails, or the tests reveal that comments still describe behavior the code no longer has.

- [ ] **Step 3: Write the minimal implementation and comment reconciliation**

Update `runtimes/compat_js/window_compat.cpp` only if the lifecycle tests expose a real bug; otherwise update comments in `runtimes/compat_js/window_compat.h` and `runtimes/compat_js/data_manager.h` so they describe the current deterministic runtime.

```cpp
// Example comment updates in runtimes/compat_js/data_manager.h
// Status: PARTIAL - Loads seeded compat records and real JSON data when a data root is configured; full project parity is still out of scope
bool loadDatabase();

// Status: PARTIAL - Accessors return live seeded/loaded compat containers; coverage still depends on available project data
const std::vector<ActorData>& getActors() const;

// Example comment updates in runtimes/compat_js/window_compat.h
// Status: PARTIAL - Exposes deterministic contents-handle lifecycle and draw-command accumulation, but not a pixel-backed bitmap buffer
virtual BitmapHandle contents() const;
virtual void createContents();
virtual void destroyContents();
```

- [ ] **Step 4: Run the focused window/data tests to verify they pass**

Run: `ctest --test-dir build --output-on-failure -R "test_window_compat|test_data_manager"`

Expected: PASS for the new contents lifecycle and data-loader assertions, with no regressions in the existing window/data cases.

- [ ] **Step 5: Commit**

```bash
git add runtimes/compat_js/window_compat.cpp runtimes/compat_js/window_compat.h runtimes/compat_js/data_manager.h tests/unit/test_window_compat.cpp tests/unit/test_data_manager.cpp
git commit -m "fix: align compat window and data runtime truth"
```

### Task 3: Reconcile Audio Harness Semantics

**Files:**
- Modify: `runtimes/compat_js/audio_manager.h`
- Test: `tests/unit/test_audio_manager.cpp`

- [ ] **Step 1: Write the failing tests**

Add a focused test that proves the deterministic harness semantics the comments claim: duck/unduck state and mix scaling remain observable through the current API.

```cpp
TEST_CASE("AudioManager: duck and unduck preserve deterministic BGM state", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopBgm();
    am.setMasterVolume(1.0);
    am.playBgm("phase2_theme", 80.0, 100.0, 4);

    am.duckBgm(30.0, 2);
    REQUIRE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().name == "phase2_theme");

    am.unduckBgm(1);
    am.update();
    REQUIRE_FALSE(am.isBgmDucked());
}
```

- [ ] **Step 2: Run the focused audio tests to verify the gap**

Run: `ctest --test-dir build --output-on-failure -R test_audio_manager`

Expected: either the new test exposes a semantic mismatch, or the implementation passes and only comment/deviation text remains stale.

- [ ] **Step 3: Write the minimal implementation or comment reconciliation**

If the test fails, fix the smallest possible semantic bug in `runtimes/compat_js/audio_manager.cpp`. If it passes, update `runtimes/compat_js/audio_manager.h` deviation comments so they describe the now-proven harness behavior precisely.

```cpp
// Example comment updates in runtimes/compat_js/audio_manager.h
// Status: PARTIAL - Deterministic duck/unduck and playback metadata are live in the compat harness, but the lane still does not drive a real backend mixer
void duckBgm(double volume, int32_t duration = 0);
void unduckBgm(int32_t duration = 0);

// Status: PARTIAL - Reports deterministic harness playback metadata and position rather than live backend state
AudioData getCurrentBgm() const;
```

- [ ] **Step 4: Run the focused audio tests to verify they pass**

Run: `ctest --test-dir build --output-on-failure -R test_audio_manager`

Expected: PASS for the new duck/unduck semantics case and the existing JS-binding and SE-cleanup coverage.

- [ ] **Step 5: Commit**

```bash
git add runtimes/compat_js/audio_manager.h tests/unit/test_audio_manager.cpp
git commit -m "docs: reconcile compat audio semantics"
```

### Task 4: Reconcile Canonical Phase 2 Docs

**Files:**
- Modify: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `WORKLOG.md`

- [ ] **Step 1: Write the failing verification target**

Define the verification set in the docs task itself so the final documentation claims are gated by the exact tests used during implementation.

```text
Required verification commands:
- ctest --test-dir build --output-on-failure -R test_battlemgr
- ctest --test-dir build --output-on-failure -R "test_window_compat|test_data_manager"
- ctest --test-dir build --output-on-failure -R test_audio_manager
```

- [ ] **Step 2: Run the full Phase 2 focused verification**

Run:

```bash
ctest --test-dir build --output-on-failure -R test_battlemgr
ctest --test-dir build --output-on-failure -R "test_window_compat|test_data_manager"
ctest --test-dir build --output-on-failure -R test_audio_manager
```

Expected: all targeted Phase 2 lanes pass before any docs claim closure.

- [ ] **Step 3: Write the minimal documentation updates**

Update the canonical docs so they reflect the actual post-test state and residual limitations.

```md
<!-- docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md -->
- Battle reward application now mutates live compat state for EXP, gold, and drops.
- Window/data comments were reconciled to current deterministic runtime behavior.
- Audio lane remains `PARTIAL` because it is still harness-backed, but the supported semantics are now explicitly tested and documented.

<!-- docs/PROGRAM_COMPLETION_STATUS.md -->
- Phase 2 runtime closure: battle reward path, window/data truthfulness, and audio semantics reconciliation completed.

<!-- WORKLOG.md -->
### 2026-04-19 — Phase 2 Runtime Closure
- Closed battle reward application into live compat state.
- Reconciled window/data runtime comments with tested deterministic behavior.
- Tightened audio harness semantics coverage and aligned status text.
```

- [ ] **Step 4: Re-run the focused verification after doc updates**

Run:

```bash
ctest --test-dir build --output-on-failure -R test_battlemgr
ctest --test-dir build --output-on-failure -R "test_window_compat|test_data_manager"
ctest --test-dir build --output-on-failure -R test_audio_manager
```

Expected: PASS again, confirming the final docs were written against a still-green Phase 2 state.

- [ ] **Step 5: Commit**

```bash
git add docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md docs/PROGRAM_COMPLETION_STATUS.md WORKLOG.md
git commit -m "docs: close phase 2 runtime remediation"
```
