# Game Maker Onboarding Finish Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish the remaining game-maker asset browser and onboarding gaps in one integrated pass.

**Architecture:** Keep implementation model-first and deterministic. Main-menu and onboarding behavior stays in `editor/project`, asset preview and scope behavior stays in `editor/assets`, catalog scope diagnostics stay in `engine/core/map`, and docs summarize shipped behavior rather than planned behavior.

**Tech Stack:** C++20, ImGui panels, nlohmann JSON snapshots, Catch2, CMake/Ninja, repo docs.

---

### Task 1: Main Menu Open And Locate

**Files:**
- Modify: `editor/project/main_menu_panel.*`
- Test: `tests/unit/test_main_menu_panel.cpp`

- [ ] Add deterministic model methods for `chooseOpenProjectRequest`, `locateMissingProject`, and `cancelMissingProjectLocate`.
- [ ] Expose pending actions and missing-project remediation state in snapshots.
- [ ] Render Open/Locate/Hide controls in the visible main menu.
- [ ] Verify with `.\build\dev-ninja-debug\urpg_tests.exe "[project][main_menu]"`.

### Task 2: Adaptive Onboarding Snapshots

**Files:**
- Modify: `editor/project/new_project_wizard_model.*`
- Test: `tests/unit/test_new_project_wizard_panel.cpp`

- [ ] Add a deterministic question-flow snapshot derived from selected game-maker template `gameType`, `questionProfile`, defaults, mechanics, UI theme, and help-tip setting.
- [ ] Cover JRPG, action RPG, tactical RPG, visual novel hybrid, cozy/life, monster collector, and platform/adventure manifests.
- [ ] Verify with `.\build\dev-ninja-debug\urpg_tests.exe "[project][editor][panel][onboarding]"`.

### Task 3: Preview Drawer Metadata

**Files:**
- Modify: `editor/assets/asset_library_panel.*`
- Test: `tests/unit/test_asset_library_panel.cpp`

- [ ] Add preview drawer summary fields for image, spritesheet, UI theme, audio, font, manifest, and unknown records.
- [ ] Keep preview failures non-blocking and snapshot-only.
- [ ] Verify with `.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"`.

### Task 4: Catalog Loader Scope Diagnostics

**Files:**
- Modify: `engine/core/map/grid_part_catalog_loader.*`
- Test: `tests/unit/test_grid_part_catalog.cpp`

- [ ] Add explicit diagnostics for lazy group/scope loads: scope name, active catalog count, active part count, full-library flag.
- [ ] Verify starter -> full library -> starter switching does not retain inactive full-library rows.
- [ ] Verify with `.\build\dev-ninja-debug\urpg_tests.exe "[grid_part]"`.

### Task 5: Docs And Release Truth

**Files:**
- Modify: `README.md`
- Modify: `docs/agent/KNOWN_DEBT.md`
- Modify: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`
- Modify: `tools/assets/README.md`
- Modify: `docs/SCHEMA_CHANGELOG.md`

- [ ] Update docs with shipped default/full catalog split, onboarding flow, preview scope, settings toggles, and remaining debt.
- [ ] Verify with `rg -n "game_maker_all_parts|asset browser|onboarding|left-side drawer|full-library" README.md docs tools/assets/README.md`.

### Task 6: Final Verification

**Files:** no new files unless fixing failures.

- [ ] Run focused build and tests:

```powershell
cmake --build --preset dev-debug --target urpg_editor urpg_tests
.\build\dev-ninja-debug\urpg_tests.exe "[project][main_menu]"
.\build\dev-ninja-debug\urpg_tests.exe "[project][editor][panel][onboarding]"
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library]"
.\build\dev-ninja-debug\urpg_tests.exe "[grid_part]"
.\build\dev-ninja-debug\urpg_editor.exe --project-root . --safe-mode --list-panels
.\tools\launcher\start_editor_guarded.ps1 -Visible -VisibleFrames 120
git diff --check
```

## Self-Review

- Spec coverage: covers open/locate, onboarding branching, preview metadata, catalog diagnostics, docs, and final verification.
- Placeholder scan: no placeholder implementation steps.
- Type consistency: all touched surfaces already exist; new methods stay model-first and snapshot-driven.
