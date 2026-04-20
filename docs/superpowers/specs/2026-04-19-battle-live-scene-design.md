# Battle Core Live Scene Authority Design

**Date:** 2026-04-19
**Scope:** Battle Core runtime/HUD bridge parity through the live `BattleScene` path

## Goal

Move live battle runtime authority toward `engine/core/battle/battle_core.*` by making `BattleScene` orchestrate phase progression, action ordering, and battle rule resolution through native battle-core types instead of maintaining a parallel scene-local authority model.

## Problem

`BattleScene` currently owns:

- live phase transitions
- scene-local queued actions
- action sorting for turn execution
- damage/guard handling and some battle result rules

At the same time, `BattleFlowController`, `BattleActionQueue`, and `BattleRuleResolver` already exist as native battle-core owners for those same concepts. This leaves runtime authority split between `engine/core/scene/battle_scene.*` and `engine/core/battle/battle_core.*`, which weakens the closure claim in the Battle Core roadmap/spec.

## Chosen Approach

Use a single implementation slice to:

1. Bind `BattleScene` phase transitions to `BattleFlowController`
2. Replace scene-local ordered execution with `BattleActionQueue`
3. Move live action rule evaluation behind `BattleRuleResolver` helpers
4. Keep `BattleScene` as the real runtime entry point and HUD/UI orchestrator

This is intentionally narrower than full battle schema or presentation-translator closure. The slice is about runtime authority in the live scene path.

## Architecture

### Native battle core responsibilities

`BattleFlowController` becomes the authoritative owner for:

- battle start
- input/action/turn-end progression
- victory/defeat terminal state

`BattleActionQueue` becomes the authoritative owner for:

- deterministic ordering of submitted scene actions
- draining ordered actions during the action phase

`BattleRuleResolver` becomes the owner for:

- converting `BattleScene::BattleAction` + participant stat context into deterministic HP/MP effects
- guard mitigation and escape-related live helper behavior already represented in battle-core policy

### BattleScene responsibilities after this change

`BattleScene` remains responsible for:

- UI windows and command callbacks
- participant storage and animation state
- target selection UX
- reward application through existing compat-backed data services
- battle-log and popup presentation

`BattleScene` should no longer be the place where phase or queue authority is invented independently from battle core.

## Boundaries

### In scope

- `engine/core/battle/battle_core.h`
- `engine/core/battle/battle_core.cpp`
- `engine/core/scene/battle_scene.h`
- `engine/core/scene/battle_scene.cpp`
- focused unit tests for battle core and battle scene integration
- canonical docs reflecting the new ownership step

### Out of scope

- battle schema/migration expansion
- presentation translator or 2.5D battle presentation work
- battle inspector/panel productization changes unless needed for compile/test alignment
- replacing compat-backed reward or database lookups in this slice

## Data Flow

1. UI callbacks still assemble a pending `BattleScene::BattleAction`
2. When actor input completes, `BattleScene` converts those actions into `BattleActionQueue` entries
3. `BattleScene` drains ordered actions from `BattleActionQueue` during `BattleFlowPhase::Action`
4. Live action execution asks `BattleRuleResolver` for deterministic outcomes
5. Scene applies the resolved effects to participants and persists actor state through existing runtime services
6. End-condition checks update `BattleFlowController`, and the scene mirrors battle-core terminal state to scene-visible APIs

## Error Handling and Safety

- Dead or retargeted single-target actions must resolve deterministically without dereferencing stale participants.
- Empty action queues during action phase must transition cleanly to turn-end through battle-core flow state.
- Guard-only and no-op actions must not bypass phase progression or leave stale queue state behind.
- Terminal phases must remain idempotent if `onUpdate()` keeps running while timers drain.

## Testing

Add or update tests to prove:

- battle-core queue ordering is usable by the live scene path
- live `BattleScene` phase progression is driven by battle-core flow state
- attack/guard/damage application routes through battle-core rule helpers
- victory/defeat/turn-end transitions still work through the real scene update loop

## Expected Outcome

After this slice, Battle Core can honestly claim that the real battle scene path is using native battle-core runtime authority for flow, ordering, and action resolution instead of only exposing those types to diagnostics/tests.
