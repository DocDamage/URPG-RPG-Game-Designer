# itch.io Maker WYSIWYG Expansion Plan

## Goal

Convert the RPG Maker itch.io search findings into implemented URPG WYSIWYG feature surfaces with saved data, runtime execution, editor preview, diagnostics, registry exposure, and tests.

## Scope

Add:

- Proximity Compass / Objective Radar
- Resource Manager / Unused Asset Cleaner
- Visual Inventory Builder
- Animated Pictures Timeline
- Horror FX Builder
- Cutscene Skip / Fast Forward
- Translation Helper Workspace
- Deployment Cook Dashboard
- Break Shield System
- Boost Point System
- Battle Weapon Swap
- Order Turn Battle System
- Item Concoction System
- Unison Attack Builder
- Timed Attack / QTE Builder
- Action Sequence Impact Builder
- Side Battle Status UI Builder
- Victory Screen Builder
- State Tooltip Display
- Animated Loading Screen Builder
- Popup Builder
- Animated Window Builder
- Video Player Surface
- Tactical Battle System
- Bullet Hell Maker
- Duelist Card Battle System
- Spin Top Battle System
- Recruiting Board
- Step Rewards System
- Event Spawner Solution
- Smart Event Timers / Progress
- Actor Construct
- Equip Construct / Visible Equipment
- Skill Equip System
- Front View Battler Mode

## Verification

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests --parallel 4
.\build\dev-ninja-debug\urpg_tests.exe "[maker][wysiwyg][features],[editor][panel][registry]"
git diff --check
```
