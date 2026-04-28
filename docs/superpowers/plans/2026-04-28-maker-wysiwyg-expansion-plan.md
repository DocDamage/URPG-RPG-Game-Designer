# Maker WYSIWYG Expansion Plan

## Goal

Implement the full requested maker-feature batch as usable WYSIWYG systems, not documentation-only placeholders.

## Scope

Add runtime/editor/project-data support for:

- Project Search Everywhere
- Broken Reference Repair Tool
- Mass Edit / Batch Operations
- Project Diff / Change Review
- Template Instance Sync
- Parallax Mapping Editor
- Collision / Passability Visualizer
- World Map Route Planner
- Secret/Hidden Object Authoring
- Biome Rule System
- Dialogue Relationship Matrix
- Branch Coverage Preview
- Cutscene Blocking Tool
- Localization Context Review
- Damage Formula Visual Lab
- Enemy Troop Timeline Preview
- Skill Combo / Chain Builder
- Bestiary / Discovery System
- Formation / Positioning System
- Menu Builder
- Journal Hub
- Notification / Toast System
- Input Prompt Skinner
- Playtest Session Recorder
- Bug Report Packager
- Performance Heatmap
- Release Checklist Dashboard
- Store Page Asset Generator
- Mod Conflict Visualizer
- Mod Packaging Wizard
- Plugin-to-Native Migration Advisor

## Implementation Contract

Every feature in this batch must have:

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
