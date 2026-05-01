# WYSIWYG Gameplay Systems Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build twelve creator-facing gameplay systems with WYSIWYG editor access, live preview, saved data, runtime execution, diagnostics, and tests.

**Architecture:** Add a shared deterministic gameplay authoring document/runtime under `engine/core/gameplay/` plus a shared editor panel under `editor/gameplay/`. Each feature is represented by a canonical `feature_type`, fixture, registry entry, and test case, so every lane is complete on the same evidence contract.

**Tech Stack:** C++20, nlohmann/json, CMake/Ninja, Catch2, existing URPG editor registry.

---

## File Structure

- `engine/core/gameplay/gameplay_wysiwyg_system.h/.cpp`: shared runtime document, diagnostics, preview, execution, JSON.
- `editor/gameplay/gameplay_wysiwyg_panel.h/.cpp`: shared WYSIWYG panel model for all twelve systems.
- `content/schemas/gameplay_wysiwyg_system.schema.json`: saved-data contract.
- `content/fixtures/*_fixture.json`: one fixture per new gameplay feature.
- `tests/unit/test_gameplay_wysiwyg_systems.cpp`: runtime/editor/registry coverage for all twelve systems.
- `engine/core/editor/editor_panel_registry.cpp`: release top-level panel ids for the twelve systems.
- `tests/unit/test_editor_panel_registry.cpp`: smoke list coverage for the twelve systems.
- `CMakeLists.txt`: compile new runtime/editor/test files.

## Tasks

- [ ] Add runtime document and deterministic rule execution.
- [ ] Add editor WYSIWYG panel with preview snapshot and save-project-data support.
- [ ] Add one schema and twelve fixtures.
- [ ] Register twelve editor surfaces.
- [ ] Add tests proving diagnostics, preview, runtime execution, saved data, and registry access.
- [ ] Run focused verification and `git diff --check`.
- [ ] Stage, commit, and push.

## Verification

```powershell
cmake --build --preset dev-debug --target urpg_tests --parallel 4
.\build\dev-ninja-debug\urpg_tests.exe "[gameplay][wysiwyg][systems],[editor][panel][registry]"
git diff --check
```
