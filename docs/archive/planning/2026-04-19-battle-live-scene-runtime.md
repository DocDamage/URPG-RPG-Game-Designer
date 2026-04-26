# Battle Core Live Scene Runtime Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the live `BattleScene` path use native Battle Core flow, queue, and rule ownership instead of parallel scene-local runtime logic.

**Architecture:** Extend `BattleFlowController`, `BattleActionQueue`, and `BattleRuleResolver` just enough to support the real scene loop, then rewire `BattleScene` to orchestrate through those types while preserving current HUD/UI behavior and reward hooks. Keep the change bounded to runtime ownership, scene integration, and focused tests/docs.

**Tech Stack:** C++20, Catch2 v3, CMake/CTest, existing URPG battle/scene runtime

---

### Task 1: Expand battle-core runtime helpers for live scene execution

**Files:**
- Modify: `engine/core/battle/battle_core.h`
- Modify: `engine/core/battle/battle_core.cpp`
- Test: `tests/unit/test_battle_core.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST_CASE("BattleFlowController begins the first turn at one and returns to input after turn end", "[battle][core][flow]") {
    BattleFlowController flow;
    flow.beginBattle(true);
    REQUIRE(flow.phase() == BattleFlowPhase::Start);
    REQUIRE(flow.turnCount() == 1);

    flow.enterInput();
    flow.enterAction();
    flow.endTurn();
    REQUIRE(flow.phase() == BattleFlowPhase::TurnEnd);

    flow.enterInput();
    REQUIRE(flow.phase() == BattleFlowPhase::Input);
    REQUIRE(flow.turnCount() == 2);
}
```

```cpp
TEST_CASE("BattleRuleResolver resolves guarded scene attacks without exceeding current HP", "[battle][core][rules]") {
    BattleDamageContext context;
    context.subject.atk = 12;
    context.target.hp = 20;
    context.target.def = 4;
    context.target.guarding = true;
    context.power = 6;

    const int32_t guarded = BattleRuleResolver::resolveDamage(context);
    context.target.guarding = false;
    const int32_t normal = BattleRuleResolver::resolveDamage(context);

    REQUIRE(guarded > 0);
    REQUIRE(normal > guarded);
    REQUIRE(normal <= 20);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleFlowController transitions deterministically across phases|BattleRuleResolver resolves damage with guard, crit, and variance rules"`

Expected: FAIL because `beginBattle()` currently initializes turn count to `0` and the new flow expectation is not implemented yet.

- [ ] **Step 3: Write minimal implementation**

```cpp
void BattleFlowController::beginBattle(bool can_escape) {
    phase_ = BattleFlowPhase::Start;
    allow_escape_ = can_escape;
    turn_count_ = 1;
    escape_failures_ = 0;
}
```

```cpp
void BattleFlowController::endTurn() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::TurnEnd;
    ++turn_count_;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleFlowController transitions deterministically across phases|BattleRuleResolver resolves damage with guard, crit, and variance rules"`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add engine/core/battle/battle_core.h engine/core/battle/battle_core.cpp tests/unit/test_battle_core.cpp
git commit -m "feat: tighten battle core flow helpers for live scene runtime"
```

### Task 2: Rewire BattleScene to use Battle Core flow and queue authority

**Files:**
- Modify: `engine/core/scene/battle_scene.h`
- Modify: `engine/core/scene/battle_scene.cpp`
- Test: `tests/unit/test_battle_scene_native.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST_CASE("BattleScene routes live phase progression through Battle Core flow state", "[battle][scene][logic]") {
    auto& dm = urpg::compat::DataManager::instance();
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();
    REQUIRE(battle.flowController().phase() == urpg::battle::BattleFlowPhase::Start);

    battle.onUpdate(0.1f);
    REQUIRE(battle.flowController().phase() == urpg::battle::BattleFlowPhase::Input);
    REQUIRE(battle.getCurrentPhase() == BattlePhase::INPUT);
}
```

```cpp
TEST_CASE("BattleScene drains queued actions through native battle action ordering", "[battle][scene]") {
    auto& dm = urpg::compat::DataManager::instance();
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0, 0}, nullptr);
    battle.addEnemy("1", "Slime", 20, 0, {100, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    auto* hero = findParticipant(participants, false);
    auto* slime = findParticipant(participants, true);
    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    BattleScene::BattleAction attack{};
    attack.subject = hero;
    attack.target = slime;
    attack.command = "attack";

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(attack);
    battle.onUpdate(0.1f);

    REQUIRE(battle.nativeActionQueue().empty());
    REQUIRE(slime->hp < 20);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleScene Logic: lifecycle and automated turn progression|BattleScene: executeAction effect application"`

Expected: FAIL because `BattleScene` does not yet expose or use battle-core flow/queue state for the live path.

- [ ] **Step 3: Write minimal implementation**

```cpp
urpg::battle::BattleFlowController& BattleScene::flowController() { return m_flowController; }
const urpg::battle::BattleFlowController& BattleScene::flowController() const { return m_flowController; }

urpg::battle::BattleActionQueue& BattleScene::nativeActionQueue() { return m_nativeActionQueue; }
const urpg::battle::BattleActionQueue& BattleScene::nativeActionQueue() const { return m_nativeActionQueue; }
```

```cpp
void BattleScene::setPhase(BattlePhase phase) {
    m_currentPhase = phase;
    switch (phase) {
    case BattlePhase::INPUT:
        m_flowController.enterInput();
        break;
    case BattlePhase::ACTION:
        m_flowController.enterAction();
        break;
    case BattlePhase::TURN_END:
        m_flowController.endTurn();
        break;
    case BattlePhase::VICTORY:
        m_flowController.markVictory();
        break;
    case BattlePhase::DEFEAT:
        m_flowController.markDefeat();
        break;
    case BattlePhase::START:
        break;
    }
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleScene Logic: lifecycle and automated turn progression|BattleScene: executeAction effect application|BattleScene: Victory condition detection"`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add engine/core/scene/battle_scene.h engine/core/scene/battle_scene.cpp tests/unit/test_battle_scene_native.cpp
git commit -m "feat: route battle scene through native battle core runtime"
```

### Task 3: Publish closure evidence in canonical docs

**Files:**
- Modify: `docs/BATTLE_CORE_NATIVE_SPEC.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`

- [ ] **Step 1: Write the failing documentation check**

```text
Requirement: the canonical battle spec and status doc must state that the live BattleScene path now routes flow, queue ordering, and action resolution through Battle Core instead of a scene-local parallel authority.
```

- [ ] **Step 2: Run the manual verification and confirm the docs do not yet say this**

Run: `rg -n "BattleScene.*Battle Core|live scene path|parallel authority|flow, queue, and action resolution" docs/BATTLE_CORE_NATIVE_SPEC.md docs/PROGRAM_COMPLETION_STATUS.md`

Expected: no match for the new closure note

- [ ] **Step 3: Write minimal implementation**

```md
- Live `BattleScene` runtime authority now routes phase progression, ordered action draining, and damage resolution through `engine/core/battle/battle_core.*` instead of maintaining a parallel scene-local battle authority.
```

- [ ] **Step 4: Run verification**

Run: `rg -n "live BattleScene runtime authority now routes phase progression, ordered action draining, and damage resolution through" docs/BATTLE_CORE_NATIVE_SPEC.md docs/PROGRAM_COMPLETION_STATUS.md`

Expected: both files show the new closure evidence

- [ ] **Step 5: Commit**

```bash
git add docs/BATTLE_CORE_NATIVE_SPEC.md docs/PROGRAM_COMPLETION_STATUS.md
git commit -m "docs: record battle core live scene authority progress"
```
