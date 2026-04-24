# Presentation Validation Guide

## Purpose
This document records the focused validation lanes for the native presentation subsystem so local verification and release review can use the same commands.

## Focused CTest Gates

### Presentation Unit Lane
- **CTest name**: `urpg_presentation_unit_lane`
- **Command**:
  - `ctest -C Debug -R urpg_presentation_unit_lane --output-on-failure`
- **What it covers**:
  - weighted fog and Post-FX blending
  - `PresentationRuntime::BuildPresentationFrame()`
  - dialogue readability/Post-FX override resolution
  - `PresentationBridge` frame generation against an active scene

### Spatial Editor Authoring Lane
- **CTest name**: `urpg_spatial_editor_lane`
- **Command**:
  - `ctest -C Debug -R urpg_spatial_editor_lane --output-on-failure`
- **What it covers**:
  - elevation brush editing behavior
  - prop gizmo screen-to-world projection
  - camera-aware placement against elevation grids
  - spatial editor schema round-trip and environment-command authoring assertions
  - spatial authoring output flowing into `PresentationRuntime` frame generation (`[presentation][spatial][e2e]`)

### Presentation Release Validation Harness
- **CTest name**: `urpg_presentation_release_validation`
- **Command**:
  - `ctest -C Debug -R urpg_presentation_release_validation --output-on-failure`
- **Standalone executable**:
  - local-profile build directory, for example `build\dev-vs2022\Debug\urpg_presentation_release_validation.exe`
  - MinGW local profile: `build\dev-mingw-debug\urpg_presentation_release_validation.exe`
- **What it covers**:
  - 100-actor MapScene stress frame generation
  - actor command count validation
  - elevation-aware actor Y resolution
  - Classic2D fallback flattening

### Renderer-Backed Snapshot Capture Lane
- **Executable surface**:
  - `urpg_snapshot_tests.exe "[snapshot][renderer][visual_capture]"`
  - `pwsh -File .\tools\ci\check_renderer_backed_visual_capture.ps1 -BuildDirectory build\dev-ninja-debug`
- **What it covers**:
  - hidden SDL/OpenGL context creation for a bounded OpenGL-enabled local smoke/golden path
  - real `OpenGLRenderer` rect/text frame submission captured into `SceneSnapshot`
  - committed clear-frame, full-frame-rect, and inset-rect golden enforcement plus renderer-backed golden comparison round-trip through `VisualRegressionHarness`
  - committed `MapScene` dialogue-overlay cropped golden coverage, proving a live scene can feed the OpenGL visual harness rather than only synthetic primitive-command builders
  - committed compat `Window_Base` status/gauge cropped golden coverage, proving a live gauge/text surface is enforced through the same harness without depending on unrendered sprite/icon paths
  - committed `ChatWindow` cropped golden coverage, proving one live mixed textured-plus-text widget path renders through the same OpenGL snapshot harness
  - committed `MapScene` world placeholder golden coverage, proving the current OpenGL backend no longer silently drops tile/sprite commands in renderer-backed validation
  - committed direct sprite frame-command textured golden coverage, proving preloaded logical sprite ids can render through the frame-command lane
  - committed direct tile frame-command textured golden coverage, proving preloaded logical tileset ids can render through the current fixed-sheet tile lane
  - committed textured `MapScene` batch golden coverage, proving the real `SpriteBatcher -> renderBatches()` whole-scene path now renders through the OpenGL backend inside the snapshot lane
  - committed textured `BattleScene` runtime golden coverage, proving a second live whole-scene batch composition with battler sprites and HUD cue quads through the same path
  - committed `EngineShell`-owned `MapScene` mixed runtime golden coverage, proving one real top-level tick can drive update, batch rendering, and frame-command submission through the same OpenGL capture lane
  - committed `EngineShell`-owned `BattleScene` actor-cue crop golden coverage, proving the top-level shell tick can also drive a live battle scene with battler sprites and cue/status quads through the real OpenGL-backed capture path
  - committed full-frame `MenuScene`, `MapScene`, and `BattleScene` regression goldens, proving the lane now carries scene-shaped full-frame drift anchors instead of only cropped widget and primitive proof
  - committed transition-pair and diff-heatmap stability goldens, proving deterministic transition capture and repeatable renderer-backed diff artifact generation
- **Notes**:
  - this lane is intentionally outside the focused PR presentation gate
  - local validation now runs this lane through `tools/ci/run_local_gates.ps1`
  - CI Gate 1 now enforces this lane through a dedicated non-headless `build/ci-renderer-backed` snapshot build executed under `xvfb`
  - `tools/ci/run_presentation_gate.ps1` now also builds `urpg_snapshot_tests` and runs a dedicated `ctest -L regression` fail-on-drift stage so golden drift stops the focused presentation gate immediately
  - the current renderer truth is split intentionally: the `SpriteBatcher -> renderBatches()` path now has real textured OpenGL coverage, and frame-command sprite/tile submission now has bounded direct texture resolution for preloaded logical ids while unresolved sprite/tile ids still use deterministic placeholders
  - it now proves one bounded primitive lane, three live overlay/widget slices, one bounded placeholder world slice, two bounded direct frame-command textured slices, two real textured whole-scene batch slices, and shell-owned Map/Menu/Battle runtime slices, not broad final-form coverage for every world/render path

## Combined Presentation Gate
- **Command**:
  - `ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure`
- **Helper script**:
  - `pwsh -File .\tools\ci\run_presentation_gate.ps1`
  - the helper now auto-detects a supported local build profile (`Ninja`, `Visual Studio 2022`, or `MinGW Makefiles`) and configures it on first run if needed
  - the helper now also runs the snapshot `regression` label as a fail-on-drift visual-regression stage
- **When to run**:
  - after presentation-runtime changes
  - after spatial editor/projection changes that touch presentation runtime behavior
  - before marking presentation-polish or runtime-hardening work complete

## Related Test Surfaces
- `build\dev-vs2022\Debug\urpg_tests.exe "[presentation]"` or the equivalent active local-profile build directory
- `build\dev-vs2022\Debug\urpg_tests.exe "[editor][spatial]"` or the equivalent active local-profile build directory
- `build\dev-vs2022\Debug\urpg_snapshot_tests.exe "[snapshot][renderer][visual_capture]"` or the equivalent active OpenGL-enabled local-profile build directory

## Notes
- The focused presentation gate is intended to complement, not replace, the broader `pr`, `nightly`, and `weekly` suites.
- `tools/ci/run_local_gates.ps1` now invokes the focused presentation gate by default before the broader repo-wide labeled CTest passes.
- `.github/workflows/ci-gates.yml` now invokes `tools/ci/run_presentation_gate.ps1` inside the `gate1-pr` job after the shared build step, so the focused presentation gate runs in CI on PR/push/workflow-dispatch paths.
- `.github/workflows/ci-gates.yml` also now invokes `tools/ci/check_renderer_backed_visual_capture.ps1` inside the `gate1-pr` job through a dedicated OpenGL-enabled snapshot build, so renderer-backed goldens are enforced in CI instead of only locally.
- `tools/ci/run_presentation_gate.ps1` now validates presentation doc-link integrity directly via `tools/docs/check-presentation-doc-links.ps1`, and the broader local/CI gates inherit that check.
- If presentation behavior changes without these gates changing state, update this document and the release checklist so the validation story stays truthful.
