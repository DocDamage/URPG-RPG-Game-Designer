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

## Combined Presentation Gate
- **Command**:
  - `ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure`
- **Helper script**:
  - `pwsh -File .\tools\ci\run_presentation_gate.ps1`
  - the helper now auto-detects a supported local build profile (`Ninja`, `Visual Studio 2022`, or `MinGW Makefiles`) and configures it on first run if needed
- **When to run**:
  - after presentation-runtime changes
  - after spatial editor/projection changes that touch presentation runtime behavior
  - before marking presentation-polish or runtime-hardening work complete

## Related Test Surfaces
- `build\dev-vs2022\Debug\urpg_tests.exe "[presentation]"` or the equivalent active local-profile build directory
- `build\dev-vs2022\Debug\urpg_tests.exe "[editor][spatial]"` or the equivalent active local-profile build directory

## Notes
- The focused presentation gate is intended to complement, not replace, the broader `pr`, `nightly`, and `weekly` suites.
- `tools/ci/run_local_gates.ps1` now invokes the focused presentation gate by default before the broader repo-wide labeled CTest passes.
- `.github/workflows/ci-gates.yml` now invokes `tools/ci/run_presentation_gate.ps1` inside the `gate1-pr` job after the shared build step, so the focused presentation gate runs in CI on PR/push/workflow-dispatch paths.
- `tools/ci/run_presentation_gate.ps1` now validates presentation doc-link integrity directly via `tools/docs/check-presentation-doc-links.ps1`, and the broader local/CI gates inherit that check.
- If presentation behavior changes without these gates changing state, update this document and the release checklist so the validation story stays truthful.
