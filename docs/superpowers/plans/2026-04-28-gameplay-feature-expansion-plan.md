# Gameplay Feature Expansion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build ten game-facing URPG maker systems with runtime execution, editor authoring, saved JSON data, diagnostics/export evidence, and focused verification.

**Architecture:** Each feature gets a runtime document/service, a small editor panel exposing a preview snapshot, JSON schema/fixture data, and unit tests. Existing subsystems are extended instead of duplicated: quests extend `engine/core/quest`, progression extends class/ability systems, crafting uses economy/vendor code, and cutscenes reuse timeline/dialogue/audio/event commands.

**Tech Stack:** C++20, nlohmann/json, CMake/Ninja, Catch2, existing URPG editor panel and diagnostics conventions.

---

## File Structure

- `engine/core/quest/quest_objective_graph.*`: quest graph document, node/link validation, runtime evaluation preview.
- `editor/quest/quest_panel.*`: visual quest authoring snapshot, selected quest/node preview, diagnostics.
- `content/schemas/quest_objective_graph.schema.json`: saved quest graph contract.
- `content/fixtures/quest_objective_graph_fixture.json`: valid quest graph proof fixture.
- `tests/unit/test_quest_registry.cpp`: quest graph authoring/runtime/diagnostics coverage.

Later tasks repeat the same structure for `progression`, `relationship`, `crafting`, `monster`, `encounter`, `items`, `npc`, `metroidvania`, and `timeline`.

## Task 1: Quest + Objective Graph

**Files:**
- Create: `engine/core/quest/quest_objective_graph.h`
- Create: `engine/core/quest/quest_objective_graph.cpp`
- Modify: `editor/quest/quest_panel.h`
- Modify: `editor/quest/quest_panel.cpp`
- Create: `content/schemas/quest_objective_graph.schema.json`
- Create: `content/fixtures/quest_objective_graph_fixture.json`
- Modify: `tests/unit/test_quest_registry.cpp`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Add failing tests**

Add tests that load a quest graph with start/objective/reward nodes, validate duplicate/missing links, preview runnable nodes from `QuestWorldState`, apply a runtime tick into `QuestRegistry`, and expose the editor snapshot.

- [x] **Step 2: Implement runtime document**

Create `QuestObjectiveGraphDocument` with `fromJson`, `toJson`, `validate`, `preview`, and `applyReadyObjectives` methods.

- [x] **Step 3: Extend QuestPanel**

Bind a graph document, render graph nodes/links/diagnostics/preview state, and allow the panel to prove selected-node authoring state.

- [x] **Step 4: Add schema and fixture**

Add the schema and a valid fixture covering node types, links, conditions, rewards, localization keys, and diagnostics metadata.

- [x] **Step 5: Verify**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests --parallel 4
.\build\dev-ninja-debug\urpg_tests.exe "[quest][graph]"
```

Expected: all quest graph tests pass.

## Task 2: Skill Tree / Class Progression

- [x] Create `engine/core/progression/skill_tree.*`, `editor/progression/skill_tree_panel.*`, schema/fixture, and tests tagged `[progression][skill_tree]`. The runtime unlocks nodes by points/prerequisites, grants ability IDs, serializes learned nodes, and exposes diagnostics for unreachable or missing ability references.

## Task 3: Relationship / Affinity

- [x] Create `engine/core/relationship/relationship_affinity.*`, extend the relationship panel, add schema/fixture, and tests tagged `[relationship][affinity]`. The runtime applies gifts/dialogue choices/faction modifiers, unlocks thresholds, persists affinity, and exposes dialogue/quest gate diagnostics.

## Task 4: Crafting + Gathering + Economy

- [x] Extend crafting/economy/vendor systems with a recipe execution document, gathering source tables, station requirements, quality tiers, vendor pricing preview, schema/fixture, editor panel snapshot, and tests tagged `[crafting][economy][loop]`.

## Task 5: Monster Capture + Party Management

- [x] Create `engine/core/monster/monster_collection.*`, `editor/monster/monster_collection_panel.*`, schema/fixture, and tests tagged `[monster][capture]`. The runtime defines species, capture formulas, party/storage movement, evolution requirements, save state, and diagnostics.

## Task 6: Enemy Encounter Designer

- [x] Extend encounter tables with biome/region spawn rules, troop previews, rewards, scaling, schema/fixture, editor snapshot, and tests tagged `[balance][encounter][designer]`.

## Task 7: Item Affix / Loot Generator

- [x] Extend loot affix generation with rarity tables, deterministic seed preview, schema/fixture, editor snapshot, generated item value math, and tests tagged `[items][loot][generator]`.

## Task 8: NPC Schedule + Daily Routine

- [x] Extend NPC schedule runtime with time blocks, route/location validation, shop/dialogue hooks, schema/fixture, editor snapshot, and tests tagged `[npc][schedule][routine]`.

## Task 9: Metroidvania Ability Gates

- [x] Create `engine/core/metroidvania/ability_gate_system.*`, editor gate panel, schema/fixture, and tests tagged `[metroidvania][ability_gates]`. The runtime validates region gates, ability unlocks, map reachability, and blocked path diagnostics.

## Task 10: Cutscene / Event Timeline

- [x] Extend timeline/event command systems with cutscene tracks for dialogue, choices, camera, audio, variables, events, and waits. Add schema/fixture, editor preview, runtime playback, diagnostics, and tests tagged `[timeline][cutscene]`.

## Final Verification

After all tasks:

```powershell
cmake --build --preset dev-debug --parallel 4
ctest --preset dev-all --output-on-failure
git diff --check
```

Then stage, commit, and push all changes.
