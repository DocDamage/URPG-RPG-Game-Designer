# Visual Creator WYSIWYG Expansion Plan

## Goal

Implement the requested visual creator feature batch as real WYSIWYG systems, not backlog text.

## Scope

Add runtime/editor/project-data support for:

- Fast Travel System
- Platformer Game Type
- Gacha System
- Map Zoom System
- Picture-Based UI Creator
- Realtime Event Effects Builder
- Directional Shadow System
- Dynamic Environment Effects Builder
- Camera Director
- Screen Filter / Post FX Builder
- Weather Composer
- Lighting Painter
- Platformer Physics Lab
- Side-View Action Combat Kit
- Summon/Gacha Banner Builder
- Fast Travel Map Builder
- Quest Board / Mission Builder
- Relationship Event Scheduler
- Farming / Garden Plot Builder
- Fishing Minigame Builder
- Mount / Vehicle System
- Stealth System
- Puzzle Logic Board
- In-Game Phone / Messenger UI
- Crafting Recipe Graph
- Housing / Decoration Editor
- Companion Command Wheel
- Title / Save Screen Builder
- Credits / Ending Builder
- Tutorial Overlay Builder
- Accessibility Preview Lab
- Economy Balance Simulator
- Enemy Encounter Zone Painter
- Random Dungeon Room Stitcher
- Achievement Visual Builder
- Mod Marketplace Packager

## Implementation Contract

Every feature must have:

- A saved JSON project-data fixture.
- A schema-backed feature type.
- A deterministic runtime preview/execution path.
- A visual authoring panel snapshot with live preview state.
- Diagnostics for broken authoring data.
- A release top-level editor registry entry.
- Focused unit coverage proving the hooks work together.

## Verification

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests --parallel 4
.\build\dev-ninja-debug\urpg_tests.exe "[maker][wysiwyg][features],[editor][panel][registry]"
git diff --check
```
