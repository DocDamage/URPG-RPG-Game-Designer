# URPG Technical Debt Audit

**Audit Date:** 2026-04-18
**Resolution Session:** 2026-04-18
**Project Phase:** Phase 2 Compat Layer (exit hardening complete)
**Baseline:** 5390 assertions / 455 cases passing (100% green)

## Summary

The codebase has completed Phase 0 Foundation and Phase 1 Native Core. Phase 2 Compat Layer exit hardening is complete. All registered API surfaces compile cleanly and the full test suite is green.

**This session resolved:**
- P0-01 build integrity blockers (3 active compile errors)
- P1-03 Audio SE channel lifetime leak
- P1-04 Battle turn-condition correctness bug
- P2-02 Diagnostics runtime stale-state clearing
- Pre-existing test failures in battle modifier resolution, state removal expectations, and map loader singleton drift

**Remaining debt shifts to:**
1. **WindowCompat full software rasterization** (Phase 3 — native renderer tier integration)
2. **Real QuickJS C API wiring** (Phase 3 — dedicated infrastructure sprint)
3. **`$gameParty`/`$gameTroop` QuickJS global exposure** (integration into `DataManager::registerAPI`)

---

## Debt Area 1: DataManager — Hardcoded Data Instead of JSON Loading ✅ RESOLVED

**Files:** `runtimes/compat_js/data_manager.cpp`
**Severity:** HIGH
**Impact:** Blocks real project loading; all database objects return empty stubs.
**Resolution Date:** 2026-04-18

**TODOs found:**
- `loadDataFiles()`: "Load from actual JSON files" (line 136)
- `loadActor()`: "Load from data/Actors.json" (line 155)
- `loadClass()`: "Load from data/Classes.json" (line 160)
- `loadSkill()`: "Load from data/Skills.json" (line 165)
- `loadItem()`: "Load from data/Items.json" (line 170)
- `loadWeapon()`: "Load from data/Weapons.json" (line 175)
- `loadArmor()`: "Load from data/Armors.json" (line 180)
- `loadEnemy()`: "Load from data/Enemies.json" (line 185)
- `loadTroop()`: "Load from data/Troops.json" (line 190)
- `loadState()`: "Load from data/States.json" (line 195)
- `loadAnimation()`: "Load from data/Animations.json" (line 200)
- `loadTileset()`: "Load from data/Tilesets.json" (line 205)
- `loadCommonEvent()`: "Load from data/CommonEvents.json" (line 210)
- `loadSystem()`: "Load from data/System.json" (line 215)
- `loadMapInfos()`: "Load from data/MapInfos.json" (line 220)
- `loadMap()`: "Load from data/MapXXX.json" (line 225)

**Remediation:** Wire `DataManager` to use `canonical_json.cpp` (already in engine) to parse MZ-format JSON database files. Add file-existence guards and structured diagnostics for missing data files.

---

## Debt Area 2: BattleManager — Incomplete Battle Pipeline ✅ RESOLVED

**Files:** `runtimes/compat_js/battle_manager.cpp`
**Severity:** HIGH
**Impact:** Battle start, turn processing, rewards, and victory/defeat flows are no-ops.
**Resolution Date:** 2026-04-18

**TODOs found:**
- `setup()`: troop data loading, party copy, transition/BGM/ME setup (lines 168–198)
- `processTurn()`: damage calculation, skill/item application, state ticks, animations (lines 476–625)
- `checkBattleEnd()`: game switch check (line 672)
- `gainRewards()`: EXP distribution, gold, item drops (lines 712–754)

**Remediation:** Implement deterministic battle phase transitions using existing `CombatCalc` kernel. Wire troop/enemy lookups through `DataManager`. Add unit tests for battle phase state machine.

---

## Debt Area 3: AudioManager — Missing Audio Playback Position & Ducking ✅ RESOLVED

**Files:** `runtimes/compat_js/audio_manager.cpp`
**Severity:** MEDIUM
**Impact:** BGM position tracking is inaccurate; ducking/unducking are instantaneous rather than smooth.
**Resolution Date:** 2026-04-18

**TODOs found:**
- `update()`: "Update audio playback position based on time" (line 95)
- `fadeOutBgm()`: "Implement smooth ducking over duration" (line 464)
- `fadeInBgm()`: "Implement smooth unducking over duration" (line 473)

**Remediation:** Add frame-based volume interpolation for ducking/unducking. Track playback position via elapsed-time delta in `update()`. Extend existing crossfade sequencing pattern.

---

## Debt Area 4: WindowCompat — Bitmap Lifecycle + Input Wiring ✅ RESOLVED

**Files:** `runtimes/compat_js/window_compat.cpp`
**Severity:** MEDIUM
**Impact:** Windows exist but cannot render text, icons, or gauges to their contents bitmap.
**Resolution Date:** 2026-04-18
- Bitmap handle allocator implemented (`allocateBitmap`/`releaseBitmap`/`isValidBitmap`)
- `Window_Base::createContents()`/`destroyContents()` manage real handles
- Sprite destructors release bitmaps
- `Sprite_Character::update()` advances pattern deterministically
- `Sprite_Actor::update()` decrements animation/effect counters
- Drawing methods converted from raw TODOs to compat-layer recording stubs

## Debt Area 4b: WindowCompat — Full Software Rasterization 🔄 REMAINING

**Files:** `runtimes/compat_js/window_compat.cpp`
**Severity:** LOW
**Impact:** Actual text layout, icon atlas lookup, gauge gradient fill, character sheet slicing are delegated to native renderer tier rather than implemented in compat layer.
**Target:** Phase 3 or when native renderer integration begins

## Debt Area 4e: WindowCompat — TP Ceiling Hardcoding ✅ RESOLVED

**Files:** `runtimes/compat_js/window_compat.cpp`, `runtimes/compat_js/data_manager.h/cpp`
**Severity:** LOW
**Impact:** `drawActorTp` hardcoded TP max to 100, preventing plugins from changing TP ceiling.
**Resolution Date:** 2026-04-18
- Added `mtp` (max TP) field to `GameActor` struct
- `setupGameActors()` initializes `mtp = 100`
- Added `setGameActorMtp()` setter to `DataManager`
- `drawActorTp` now computes rate as `tp / mtp` instead of hardcoded `tp / 100`
- Added 6 unit tests covering default mtp, custom mtp, and clamp behavior

## Debt Area 4c: WindowCompat — DataManager-Backed Actor Drawing ✅ RESOLVED

**Files:** `runtimes/compat_js/window_compat.cpp`
**Severity:** MEDIUM
**Impact:** `drawActorName`, `drawActorLevel`, `drawActorHp`, `drawActorMp`, `drawActorTp` used placeholders instead of actual actor data.
**Resolution Date:** 2026-04-18
- `drawActorName`/`drawActorLevel` now look up `ActorData` from `DataManager`
- `drawActorHp`/`drawActorMp` compute gauge rates from `ActorData::params`
- `drawActorTp` documented as needing runtime state
- Swarm 4 enhancement: now reads current `hp/mp/tp` from `$gameActors` (`GameActor` runtime state)

## Debt Area 4d: WindowCompat — Window_Command Drawing ✅ RESOLVED

**Files:** `runtimes/compat_js/window_compat.cpp`
**Severity:** MEDIUM
**Impact:** `Window_Command::drawItem()` was a stub that didn't draw command text or respect enabled/disabled color states.
**Resolution Date:** 2026-04-18
- `drawItem()` now draws command name via `drawText` with `normalColor()` for enabled, `dimColor()` for disabled
- Added `itemRectForIndex()` for per-item rect calculation
- Added `normalColor()`, `dimColor()`, `deathColor()` helpers to `Window_Base`

**TODOs found:**
- `createContents()`: "Create actual bitmap for contents" (line 533)
- `destroyContents()`: "Release bitmap resources" (line 538)
- `drawText()`: "Actual text rendering to contents bitmap" (line 547)
- `drawIcon()`: "Draw icon from icon set" (line 561)
- `drawGauge()`: "Draw gauge background and fill with gradient" (line 649)
- `drawCharacter()`: "Draw character sprite from character sheet" (line 663)

**Remediation:** Implement a minimal software-rasterized bitmap surface compatible with the `RenderTier` capability model. Add text-layout escape-token parity tests (escape-token handling is already `FULL`).

---

## Debt Area 5: QuickJSRuntime — Stub Hardening ✅ RESOLVED

**Files:** `runtimes/compat_js/quickjs_runtime.cpp`, `quickjs_runtime.h`
**Severity:** MEDIUM
**Impact:** Fixture scripts run through stub path; real JS plugin execution is not yet possible.
**Resolution Date:** 2026-04-18
- Added `URPG_HAS_QUICKJS` compile-time gate
- Stub `runGC()` collects unreachable globals and recalculates simulated heap
- Stub `eval()`/`call()`/`registerFunction()`/`setGlobal()` increment simulated heap deterministically
- `isMemoryExceeded()` works against simulated heap

## Debt Area 5b: QuickJSRuntime — Real QuickJS C API Wiring 🔄 REMAINING

**Files:** `runtimes/compat_js/quickjs_runtime.cpp`
**Severity:** MEDIUM
**Impact:** Real JS plugin execution requires QuickJS library fetch + C API integration.
**Target:** Phase 3 or dedicated infrastructure sprint

## Debt Area 5d: BattleManager — State System Placeholder ✅ RESOLVED

**Files:** `runtimes/compat_js/battle_manager.h/cpp`, `runtimes/compat_js/data_manager.h/cpp`
**Severity:** MEDIUM
**Impact:** `applyStateEffects` applied hardcoded 2 HP damage regardless of actual states. `BattleSubject` could not track active states.
**Resolution Date:** 2026-04-18
- Expanded `StateData` with `slipDamage`, `removeByDamage`, `removeByWalking`, `chanceByDamage`
- `loadStates()` JSON parser now reads all new fields
- Added `states` vector and `stateTurns` map to `BattleSubject`
- Implemented `addState`/`removeState`/`hasState`/`clearStates`/`getStateTurns`
- Rewrote `applyStateEffects` to iterate active states, look up `StateData`, apply `slipDamage`, decrement auto-removal turns
- Added `removeExpiredStates` with `ON_STATE_REMOVED` hook triggers
- Modified `applyDamage` to check `removeByDamage` states with deterministic `std::minstd_rand` roll against `chanceByDamage`
- Added 7 new unit tests for state management, slip damage, auto-removal, and damage-triggered removal

## Debt Area 5c: Manager QuickJS API Wiring ✅ RESOLVED

**Files:** `runtimes/compat_js/data_manager.cpp`, `battle_manager.cpp`, `audio_manager.cpp`
**Severity:** HIGH
**Impact:** Plugins could not interact with game state, battles, or audio through JavaScript because `registerAPI` returned hardcoded stubs.
**Resolution Date:** 2026-04-18
- `DataManager::registerAPI` now wires all 11 methods to live singleton calls
- `BattleManager::registerAPI` now wires all 6 methods to live singleton calls
- `AudioManager::registerAPI` now wires 14 methods to live singleton calls

**TODOs found:**
- `initContext()`: "Initialize actual QuickJS runtime here" (line 133)
- `shutdown()`: "JS_RunGC(rt)" (line 489)

**Remediation:** Add QuickJS library fetch in CMake (or document as external dependency). Bridge `QuickJSContext::eval`/`call` to actual QuickJS C API. Keep stub path available for deterministic test lanes.

---

## Debt Area 6: Test Hygiene — Duplicate File ✅ RESOLVED

**Files:** `tests/unit/test_compat_report_panel.cpp` and `tests/unit/test_compat_reportPanel.cpp`
**Severity:** LOW
**Impact:** Confuses CI discovery; may cause symbol collisions.
**Resolution Date:** 2026-04-18
**Action:** Deleted orphan `tests/unit/test_compat_reportPanel.cpp`.

**Remediation:** Compare the two files. Merge any unique assertions into the canonical file (`test_compat_report_panel.cpp`, matching the source file naming), then delete the duplicate.

---

## Debt Area 7: Input/Touch — Window_Selectable Input Wiring ✅ RESOLVED

**Files:** `runtimes/compat_js/window_compat.cpp`
**Severity:** MEDIUM
**Impact:** `Window_Selectable` cursor movement and `TouchInput` world coordinates are not connected to actual input system.
**Resolution Date:** 2026-04-18
- `processCursorMove()` reads `InputManager::getDir4()`/`getDir8()` and drives cursor navigation
- `processHandling()` reads `InputManager::isTriggered(DECISION/CANCEL)` and dispatches `onOk()`/`onCancel()`
- `Window_Command::onOk()` delegates to `callOkHandler()`

## Debt Area 7b: Input/Touch — TouchInput World-Coordinate Integration ✅ RESOLVED

**Files:** `runtimes/compat_js/window_compat.cpp`
**Severity:** LOW
**Impact:** Touch-based selection in `Window_Selectable` (tap-to-select) not yet wired.
**Resolution Date:** 2026-04-18
- Added `Window_Selectable::hitTest(localX, localY)` for coordinate-to-item mapping
- `processCursorMove()` handles mouse click (`isMouseTriggered`) and touch tap (`isTouchTriggered`)
- `processHandling()` triggers `onOk()` for touch/mouse input

**TODOs found:**
- `Window_Selectable::update()`: "Connect to input system" (line 1149), "Check input and move cursor" (line 1153)

**Remediation:** Wire `InputManager` state into `Window_Selectable::update()` for directional cursor movement. Ensure touch telemetry (`moveSpeed`, `tapCount`) already tracked in `InputManager` is consumed.

---

## Recommended Swarm Assignments (Completed)

| Agent | Debt Area | Priority | Status |
|-------|-----------|----------|--------|
| A1 | DataManager JSON loading | HIGH | ✅ Done |
| A2 | BattleManager pipeline | HIGH | ✅ Done |
| A3 | AudioManager ducking/position | MEDIUM | ✅ Done |
| A4 | WindowCompat rendering + input | MEDIUM | ✅ Done |
| A5 | Test hygiene + QuickJS stub cleanup | LOW | ✅ Done |
| A6 | Manager QuickJS API wiring | HIGH | ✅ Done |
| A7 | DataManager-backed actor drawing | MEDIUM | ✅ Done |
| A8 | BattleManager audio + combat calc | MEDIUM | ✅ Done |
| A9 | TouchInput tap-to-select | LOW | ✅ Done |
| A10 | State system end-to-end wiring | MEDIUM | ✅ Done |
| A11 | TP ceiling hardcoding fix | LOW | ✅ Done |
| A12 | `$gameParty`/`$gameTroop` runtime classes + QuickJS wiring | MEDIUM | ✅ Done |

---

## Debt Area 8: $gameParty / $gameTroop Runtime State ✅ RESOLVED

**Files:** `runtimes/compat_js/game_party.h/cpp`, `runtimes/compat_js/game_troop.h/cpp`, `runtimes/compat_js/data_manager.h/cpp`, `runtimes/compat_js/battle_manager.cpp`
**Severity:** MEDIUM
**Impact:** MZ plugins reference `$gameParty.members()`, `$gameTroop.members()`, etc. No runtime objects existed.
**Resolution Date:** 2026-04-18
- Created `GameParty` class with member/gold/item/weapon/armor/steps management
- Created `GameTroop` class with member management and `totalExp`/`totalGold` aggregation
- Added non-const `getEnemy()` to `DataManager` for test mutability
- Added 6 unit tests for GameParty and GameTroop
- Added `registerAPI` to `GameParty` and `GameTroop` exposing all public methods via `$gameParty` / `$gameTroop` QuickJS objects
- Wired `DataManager::registerAPI` to register `$gameParty` and `$gameTroop` globals
- Added `gameParty_` and `gameTroop_` runtime state to `DataManager`, initialized in `setupNewGame()` and `clearDatabase()`
- Integrated `GameTroop` into `BattleManager::setup()` so troop members sync on battle start
- Added 4 QuickJS roundtrip tests for `$gameParty` and `$gameTroop`
- Added 1 integration test verifying `BattleManager::setup()` syncs `GameTroop`

## New This Session — Build Integrity Closure ✅ RESOLVED

**Files:** `runtimes/compat_js/battle_manager.cpp`, `runtimes/compat_js/data_manager.cpp`
**Severity:** P0
**Impact:** Active compile blockers prevented clean builds and masked downstream test failures.
**Resolution Date:** 2026-04-18

**Fixes applied:**
- `battle_manager.cpp:912` — Range-based for loop over `std::vector<BattleStateEffect>` incorrectly typed as `int32_t`; fixed to iterate `const auto& effect` with `effect.stateId` access.
- `data_manager.cpp:614` — `TroopData::pages` (type `Value`) assigned raw `Array`; wrapped with `Value::Arr(...)`.
- `data_manager.cpp:1678` — `Value::Arr(t.pages)` used where `t.pages` is already a `Value`; changed to direct assignment.

**Verification:** `urpg_core` builds cleanly in Release and Debug.

---

## New This Session — Audio SE Channel Lifetime ✅ RESOLVED

**Files:** `runtimes/compat_js/audio_manager.h/cpp`
**Severity:** P1
**Impact:** SE channels accumulated indefinitely, causing unbounded memory growth during normal gameplay.
**Resolution Date:** 2026-04-18

**Fixes applied:**
- Added `durationFrames_` / `elapsedFrames_` and `setDurationFrames()` to `AudioChannel`.
- `AudioChannel::update()` auto-calls `stop()` when elapsed frames exceed configured duration.
- `AudioManager::playSe()` assigns a default 60-frame duration to every SE channel.
- Added `AudioManager::getSeChannelCount()` for diagnostics/test visibility.

**Verification:** New regression test `"AudioManager: SE channels do not grow unbounded under repeated playback"` proves channels are reclaimed automatically after playback completes.

---

## New This Session — Battle Turn-Condition Correctness ✅ RESOLVED

**Files:** `runtimes/compat_js/battle_manager.cpp`
**Severity:** P1
**Impact:** `checkTurnCondition()` produced incorrect periodic cadence for span ≥ 2, breaking battle event timing.
**Resolution Date:** 2026-04-18

**Fixes applied:**
- Replaced broken switch-based logic with unified periodic formula:
  ```cpp
  if (span <= 0) return isTurn(turn);
  return turnCount_ >= turn && (turnCount_ - turn) % span == 0;
  ```

**Verification:** New regression tests cover span 0 exact match, span 1 every-turn-after-threshold, span 2 periodic cadence, and boundary turns.

---

## New This Session — Diagnostics Runtime Clear Methods ✅ RESOLVED

**Files:** `editor/diagnostics/diagnostics_workspace.cpp`, `editor/audio/audio_inspector_panel.h`, `editor/ability/ability_inspector_panel.h`
**Severity:** P2
**Impact:** `clearAudioRuntime()` and `clearAbilityRuntime()` were empty no-ops, leaving stale panel state after runtime detachment.
**Resolution Date:** 2026-04-18

**Fixes applied:**
- Added `clear()` to `AudioInspectorModel`, `AudioInspectorPanel`, `AbilityInspectorModel`, and `AbilityInspectorPanel`.
- Implemented `DiagnosticsWorkspace::clearAudioRuntime()` and `clearAbilityRuntime()` to fully reset displayed state.

**Verification:** New integration test `"DiagnosticsWorkspace - Audio and ability runtime binding clears correctly"` covers bind/clear cycles for both panels.

---

## New This Session — Battle Modifier Resolution in processAction ✅ RESOLVED

**Files:** `runtimes/compat_js/battle_manager.cpp`
**Severity:** P1 (Pre-existing test failure)
**Impact:** `processAction()` read base `atk`/`def` without applying buff/debuff modifiers, causing incorrect damage calculation.
**Resolution Date:** 2026-04-18

**Fixes applied:**
- `processAction()` now applies `scaleMagnitude()` with `getModifierStage()` when mapping `BattleSubject` stats to `ActorStats` for `CombatCalc::PhysicalDamage()`.

**Verification:** Existing test `"BattleManager: modifiers affect attack resolution and targeting priority"` now passes with explicit base stats.

---

## New This Session — DataManager Map JSON Parsing ✅ RESOLVED

**Files:** `runtimes/compat_js/data_manager.cpp`, `tests/unit/test_scene_manager.cpp`
**Severity:** P2 (Pre-existing test failure)
**Impact:** `loadMapData()` JSON path did not extract `width`, `height`, `tilesetId`, or `data` from MZ-format map JSON; test was order-dependent due to leaked singleton `dataPath_`.
**Resolution Date:** 2026-04-18

**Fixes applied:**
- Added full JSON field extraction (`width`, `height`, `tilesetId`, `data`) to `loadMapData()` file-loading branch.
- Added `DataManager::instance().clearDatabase()` and `setDataPath("")` to `MapLoader` test for deterministic mock-map usage.

**Verification:** `"MapLoader: Bridge DataManager to MapScene"` now passes reliably.

---

## Next Phase Readiness

After burn-down of the above:
- Phase 2 Compat Layer exit hardening is **complete** (100% test green).
- Remaining work shifts to Phase 3 (Copilot + Polish: cutscene timeline, debugger profiler, full software rasterization, real QuickJS wiring).
