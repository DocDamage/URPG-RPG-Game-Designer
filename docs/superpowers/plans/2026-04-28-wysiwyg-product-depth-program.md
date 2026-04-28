# WYSIWYG Product Depth Program Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build URPG's remaining WYSIWYG product-depth lanes as complete vertical slices with saved project data, visual authoring, runtime execution, diagnostics, and verification.

**Architecture:** Each feature lane must touch the runtime model, editor panel/model, JSON schema or fixture data, runtime preview/execution hooks, diagnostics, and tests in the same slice. Existing deterministic core systems remain the source of truth; editor surfaces should expose and save the same data the runtime consumes.

**Tech Stack:** C++20 core runtime, ImGui editor panels, JSON project documents, CMake/Ninja, Catch2 tests, OpenGL/headless rendering paths, offline artifact generators kept out of runtime dependencies.

---

## File Structure

- `engine/core/render/dungeon3d_world.h/.cpp`: 3D dungeon/world project data, runtime preview, diagnostics, JSON round-trip, WYSIWYG verification and template bindings.
- `editor/spatial/dungeon3d_world_panel.h/.cpp`: creator-facing 3D dungeon/world authoring snapshot and manual verification actions.
- `content/schemas/dungeon3d_world.schema.json`: saved-project contract for the 3D world surface.
- `content/fixtures/dungeon3d_world_fixture.json`: working authored fixture used by tests and examples.
- `tests/unit/test_dungeon3d_world.cpp`: complete coverage for 3D WYSIWYG authoring state, runtime commands, diagnostics, and JSON persistence.
- `README.md`: product-level truth for implemented functionality and remaining external lanes.

Future slices will follow the same vertical pattern for these surfaces:

- Battle animation/VFX timeline editor: `engine/core/battle/*`, `editor/battle/*`, schemas, fixtures, tests.
- Map lighting/weather/region preview: `engine/core/render/*`, `editor/map/*`, schemas, fixtures, tests.
- Dialogue preview with portraits, choices, variables, localization: `engine/core/message/*`, `editor/message/*`, schemas, fixtures, tests.
- Event-command visual graph authoring: `engine/core/events/*`, `editor/events/*`, schemas, fixtures, tests.
- Ability sandbox: `engine/core/ability/*`, `editor/ability/*`, schemas, fixtures, tests.
- Save/load preview lab: `engine/core/save/*`, `editor/save/*`, schemas, fixtures, tests.
- Export preview: `engine/core/export/*`, `editor/export/*`, schemas, fixtures, tests.
- Renderer/visual validation: `engine/core/testing/visual_regression_harness.*`, renderer backends, snapshot fixtures, CI gates.
- Accessibility/audio/performance: `engine/core/accessibility/*`, `engine/core/audio/*`, `engine/core/performance/*`, diagnostics and matrix reports.
- Platform/services: keep proprietary SDK credentials, store services, payments, analytics privacy review, and cloud AI providers external/project-configured.
- Offline tooling: FAISS retrieval, SAM/SAM2 segmentation, Demucs/Encodec audio artifact generation remain tooling-only and do not become runtime dependencies.

### Task 1: 3D WYSIWYG Authoring Verification and Template Binding

**Files:**
- Modify: `engine/core/render/dungeon3d_world.h`
- Modify: `engine/core/render/dungeon3d_world.cpp`
- Modify: `editor/spatial/dungeon3d_world_panel.h`
- Modify: `editor/spatial/dungeon3d_world_panel.cpp`
- Modify: `content/schemas/dungeon3d_world.schema.json`
- Modify: `content/fixtures/dungeon3d_world_fixture.json`
- Test: `tests/unit/test_dungeon3d_world.cpp`

- [ ] **Step 1: Add failing tests for WYSIWYG authoring and template binding**

Add assertions that authored 3D projects expose visual authoring layers, manual verification state, and enabled template runtime commands.

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[dungeon3d]"`

Expected before implementation: FAIL because preview fields and methods do not exist.

- [ ] **Step 2: Add runtime data and diagnostics**

Add saved verification steps and template bindings to `Dungeon3DWorldDocument`, derive preview metrics and commands from that data, validate duplicate/missing IDs, and persist everything through JSON.

- [ ] **Step 3: Add editor panel exposure**

Expose visual authoring layers, verification completion, template bindings, and a `markVisualVerification` editor action in `Dungeon3DWorldPanel`.

- [ ] **Step 4: Update schema and fixture**

Persist `visual_verification_steps` and `template_bindings` in the schema and sample fixture so the runtime/editor are driven by saved project data.

- [ ] **Step 5: Verify**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests --parallel 4
.\build\dev-ninja-debug\urpg_tests.exe "[dungeon3d]"
git diff --check
```

Expected: build succeeds, the dungeon3d suite passes, and `git diff --check` reports no whitespace errors.

### Task 2: Battle Animation/VFX Timeline WYSIWYG Slice

**Files:**
- Modify: `engine/core/battle/*timeline*`
- Modify: `editor/battle/*timeline*`
- Modify: relevant schema/fixture files
- Test: relevant Catch2 battle/editor tests

- [x] **Step 1: Require timeline data to save clips, tracks, keys, preview cursor, diagnostics, and runtime playback commands.**
- [x] **Step 2: Add editor controls for clip/track/key editing and live runtime preview.**
- [x] **Step 3: Add tests proving saved timeline data produces the same runtime playback events shown by the editor.**

### Task 3: Map Lighting, Weather, Region, Tactical Overlay, and Spawn Preview Slice

**Files:**
- Modify: `engine/core/render/*`
- Modify: `editor/map/*`
- Modify: relevant schema/fixture files
- Test: renderer/editor/map tests

- [x] **Step 1: Bind authored tile, lighting, weather, region, tactical overlay, and spawn-table data into a single preview contract.**
- [x] **Step 2: Add editor diagnostics for missing renderer coverage and conflicting overlays.**
- [ ] **Step 3: Add cross-backend visual validation snapshots for supported renderer paths.**

### Task 4: Dialogue, Event Graph, Ability Sandbox, Save Lab, and Export Preview Hardening

**Files:**
- Modify: existing `engine/core/message`, `engine/core/events`, `engine/core/ability`, `engine/core/save`, `engine/core/export` files
- Modify: matching `editor/*` panels
- Test: matching unit/editor/snapshot tests

- [ ] **Step 1: Confirm each surface saves project data and executes through runtime contracts.**
- [ ] **Step 2: Add missing editor actions and diagnostics where panels only snapshot data.**
- [ ] **Step 3: Add tests tying editor preview state to runtime execution state.**

Dialogue preview slice status:

- [x] Saved dialogue preview choices now persist target pages, command hooks, and variable writes.
- [x] Runtime preview now emits command traces for showing pages, speakers, portraits, text, choices, selected choices, confirmed choices, variable writes, choice commands, and next-page routing.
- [x] Editor panel now exposes choice selection/confirmation actions and post-choice state snapshots.
- [x] Focused tests tie saved fixture data, editor preview state, runtime command traces, diagnostics, and JSON round-trip behavior together.

Event-command visual graph slice status:

- [x] Saved event command graph edges now support sequence and conditional traversal with switch/variable conditions.
- [x] Runtime preview now emits command and edge traces and applies switch/variable writes through the same graph traversal shown by the editor.
- [x] Editor panel now exposes add/update node, move node, and connect edge actions, plus selected-node details and post-run state counts.
- [x] Focused tests tie panel graph mutations, saved JSON, dependency projection, conditional traversal, diagnostics, and runtime execution together.

### Task 5: Renderer, Accessibility, Audio, Performance, Platform, and Offline Tooling Lanes

**Files:**
- Modify: existing renderer/testing/accessibility/audio/performance/tooling files
- Create: offline artifact tools only when a lane has no existing tool entry point
- Test: focused tests and local gates per lane

- [ ] **Step 1: Extend renderer parity tests across backend permutations that can run locally.**
- [ ] **Step 2: Add contrast extraction from renderer-derived UI surfaces.**
- [ ] **Step 3: Keep proprietary SDKs, store credentials, payments, cloud AI, analytics privacy review, and external marketplaces as project-configured/external boundaries.**
- [ ] **Step 4: Keep FAISS, SAM/SAM2, Demucs, and Encodec as offline artifact generation tools and prove no runtime dependency creep is introduced.**
