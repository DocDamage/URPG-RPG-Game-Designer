# AAA Release Execution Plan

This plan converts the game/app completion audit into ordered implementation work. Tasks are grouped so a developer or coding agent can complete them one by one without guessing.

## Phase 0 - Build Blockers And App-Breaking Issues

### Task P0-01

- Task ID: `P0-01`
- Title: Make runtime title menu operable
- Files to edit: `engine/core/scene/runtime_title_scene.h`, `engine/core/scene/runtime_title_scene.cpp`
- Files to inspect: `engine/core/scene/menu_scene.*`, `engine/core/input/input_core.h`, `tests/unit/test_engine_shell.cpp`
- Dependencies: None
- Risk level: High
- Exact implementation steps:
  1. Add selected-command index to `RuntimeTitleScene`.
  2. Override `handleInput(const InputCore&)`.
  3. Map `MoveUp`/`MoveDown` to focus movement over enabled and disabled commands.
  4. Map `Confirm` to `activateCommand()` for the selected command.
  5. Render visible focus/disabled state in `onUpdate()`.
  6. Preserve existing callback behavior for New Game, Continue, Exit.
- Acceptance criteria: Title screen can be navigated and activated through runtime input, not only direct test calls.
- Verification command or manual test: `ctest --preset dev-all -R "RuntimeTitleScene" --output-on-failure`

### Task P0-02

- Task ID: `P0-02`
- Title: Fix input just-pressed lifecycle
- Files to edit: `engine/core/input/input_core.h`, `engine/core/engine_shell.h`
- Files to inspect: `engine/core/scene/map_scene.cpp`, `engine/core/scene/menu_scene.cpp`, `engine/core/platform/sdl_surface.cpp`
- Dependencies: `P0-01`
- Risk level: High
- Exact implementation steps:
  1. Split input state into current/previous or add per-frame transition tracking.
  2. Add `beginFrame()` or `endFrame()` style advancement API.
  3. Ensure `Pressed` becomes `Held` after one tick.
  4. Ensure `Released` does not remain just-released forever.
  5. Call advancement once per `EngineShell::tick()` after scene input handling.
  6. Update tests that currently rely on sticky `Pressed`.
- Acceptance criteria: Holding Confirm does not repeatedly activate one-shot commands; held movement still works.
- Verification command or manual test: `ctest --preset dev-all -R "input|RuntimeTitleScene|MapScene|menu" --output-on-failure`

### Task P0-03

- Task ID: `P0-03`
- Title: Wire runtime input remap settings into startup
- Files to edit: `apps/runtime/main.cpp`, `engine/core/input/input_remap_store.*`, `engine/core/runtime_startup_services.*`
- Files to inspect: `engine/core/settings/app_settings_store.*`, `editor/input/input_remap_panel.*`
- Dependencies: `P0-02`
- Risk level: Medium
- Exact implementation steps:
  1. Resolve `RuntimeSettings::input_mapping_path` relative to project root.
  2. Load mappings through `InputRemapStore`.
  3. Apply mappings to `EngineShell::getInput()`.
  4. Emit startup diagnostics on missing/malformed mapping files.
  5. Fall back to defaults when no mapping file exists.
- Acceptance criteria: Saved input mappings change runtime controls after restart.
- Verification command or manual test: `ctest --preset dev-all -R "input.*remap|settings|startup" --output-on-failure`

### Task P0-04

- Task ID: `P0-04`
- Title: Require real executable staging for release exports
- Files to edit: `engine/core/tools/export_packager.cpp`, `engine/core/tools/export_packager_executable_staging.cpp`, `engine/core/tools/export_packager.h`
- Files to inspect: `engine/core/export/export_validator.*`, `tests/unit/test_export_packager.cpp`, `tools/ci/check_platform_exports.ps1`
- Dependencies: None
- Risk level: High
- Exact implementation steps:
  1. Add explicit `ExportMode` or release flag distinguishing test bootstrap from release export.
  2. Fail release-mode native exports when `runtimeBinaryPath` is missing.
  3. Fail release-mode Web export until a real WASM/runtime artifact exists.
  4. Keep bounded bootstrap mode only for tests/dev smoke.
  5. Update export logs to state whether output is playable or bootstrap-only.
- Acceptance criteria: Release export cannot silently produce marker/bootstrap artifacts.
- Verification command or manual test: `ctest --preset dev-all -R "ExportPackager|ExportValidator" --output-on-failure`

## Phase 1 - Unwired UI, Routes, Handlers, And Feature Surfaces

### Task P1-01

- Task ID: `P1-01`
- Title: Define production editor panel exposure map
- Files to edit: `engine/core/editor/editor_panel_registry.cpp`, `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Files to inspect: all `editor/*/*panel.*`, `apps/editor/main.cpp`, `docs/APP_RELEASE_READINESS_MATRIX.md`
- Dependencies: None
- Risk level: High
- Exact implementation steps:
  1. Classify every registry panel as `ReleaseTopLevel`, `Nested`, `DevOnly`, or `Deferred`.
  2. Remove legacy pending-wiring wording from panels intended for release.
  3. Add explicit reasons for deferred/dev-only panels.
  4. Align docs with actual exposure.
- Acceptance criteria: No release-claimed panel remains hidden without a documented release exclusion.
- Verification command or manual test: confirm no legacy pending-wiring wording remains in `engine/core/editor` or `docs`.

### Task P1-02

- Task ID: `P1-02`
- Title: Register intended production panels in editor shell
- Files to edit: `apps/editor/main.cpp`
- Files to inspect: `engine/core/editor/editor_panel_registry.*`, target panel constructors/models
- Dependencies: `P1-01`
- Risk level: High
- Exact implementation steps:
  1. Extend `EditorPanelRuntime` with instances/models for selected production panels.
  2. Bind required runtime/model dependencies.
  3. Add render factories for each selected top-level panel.
  4. Ensure `--list-panels` and smoke workflow include them.
- Acceptance criteria: Every production top-level panel appears in `urpg_editor --headless --list-panels`.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_editor.exe --headless --list-panels --frames 1`

### Task P1-03

- Task ID: `P1-03`
- Title: Wire Ability Inspector command callbacks
- Files to edit: `apps/editor/main.cpp`, `editor/ability/ability_inspector_panel.*`
- Files to inspect: `engine/core/ability/*`, `tests/unit/test_ability_inspector.cpp`
- Dependencies: `P1-02`
- Risk level: High
- Exact implementation steps:
  1. Register `preview_selected`.
  2. Register `apply_draft_to_runtime`.
  3. Register `save_draft` to project-local ability asset storage.
  4. Register `load_draft` from the same storage.
  5. Surface success/error feedback in panel snapshot and ImGui UI.
- Acceptance criteria: Preview, Apply, Save, and Load buttons perform real behavior or report actionable errors.
- Verification command or manual test: `ctest --preset dev-all -R "Ability Inspector|Ability end-to-end" --output-on-failure`

### Task P1-04

- Task ID: `P1-04`
- Title: Implement runtime Options flow
- Files to edit: `engine/core/scene/runtime_title_scene.*`, new `engine/core/scene/options_scene.*`, `apps/runtime/main.cpp`
- Files to inspect: `engine/core/settings/app_settings_store.*`, `editor/audio/audio_mix_panel.*`, `editor/input/input_remap_panel.*`
- Dependencies: `P0-01`, `P0-03`
- Risk level: High
- Exact implementation steps:
  1. Add an Options scene or modal command state.
  2. Expose display, audio, input, and accessibility settings.
  3. Persist changed settings.
  4. Add Cancel/Back navigation.
  5. Add disabled/error feedback for unsupported settings.
- Acceptance criteria: Options is enabled on the title screen and persists changes.
- Verification command or manual test: Manual: launch runtime, navigate to Options, change volume/window setting, restart, confirm persisted value.

### Task P1-05

- Task ID: `P1-05`
- Title: Decide and wire Analytics Flush behavior
- Files to edit: `apps/editor/main.cpp`, `editor/analytics/analytics_panel.*`, `engine/core/analytics/analytics_uploader.*`
- Files to inspect: `PRIVACY_POLICY.md`, `docs/release/LEGAL_REVIEW_SIGNOFF.md`
- Dependencies: None
- Risk level: Medium
- Exact implementation steps:
  1. Decide release behavior: local export only or remote upload.
  2. If local export, implement upload handler that writes a consent-gated JSONL artifact.
  3. If remote upload, implement endpoint config and privacy review gates.
  4. Disable/hide Flush when no release-approved handler exists.
- Acceptance criteria: "Flush Upload" is never a dead button in release UI.
- Verification command or manual test: `ctest --preset dev-all -R "analytics|privacy|consent" --output-on-failure`

## Phase 2 - Incomplete Implementations, Stubs, Placeholders, Mock Data

### Task P2-01

- Task ID: `P2-01`
- Title: Replace release map placeholder asset IDs with project data
- Files to edit: `engine/core/scene/map_scene.*`, runtime project/content loader files
- Files to inspect: `engine/core/render/asset_loader.*`, `content/schemas/project.schema.json`, `engine/core/export/runtime_bundle_loader.*`
- Dependencies: `P0-04`
- Risk level: High
- Exact implementation steps:
  1. Define project map asset references for player sprite and tileset.
  2. Load those references during runtime scene creation.
  3. Replace hardcoded `hero_sprite` and `default_tileset`.
  4. Emit missing asset diagnostics before rendering placeholders.
- Acceptance criteria: Runtime map uses bundled/project assets, not test-only logical IDs.
- Verification command or manual test: `ctest --preset dev-all -R "MapScene|AssetLoader|Runtime map asset" --output-on-failure`

### Task P2-02

- Task ID: `P2-02`
- Title: Make battleback fallback release-safe
- Files to edit: `engine/core/scene/battle_scene.cpp`, battle authoring validation files
- Files to inspect: `engine/core/battle/battle_presentation_profile.*`, `content/fixtures/battle_authoring_fixture.json`
- Dependencies: `P2-01`
- Risk level: Medium
- Exact implementation steps:
  1. Remove implicit reliance on `Grassland.png` unless packaged.
  2. Require battleback in battle profile or supply a real bundled fallback.
  3. Promote missing battleback to export/runtime validation error for release mode.
  4. Keep diagnostic-only behavior for tests/dev if needed.
- Acceptance criteria: No battle scene silently depends on absent RPG Maker default paths.
- Verification command or manual test: `ctest --preset dev-all -R "battle.*assets|battle.*authoring" --output-on-failure`

### Task P2-03

- Task ID: `P2-03`
- Title: Separate dev/test bootstrap exports from production exports
- Files to edit: `engine/core/tools/export_packager_executable_staging.cpp`, `engine/core/tools/export_packager.h`, docs under `docs/release/`
- Files to inspect: `tests/unit/test_export_packager.cpp`, `tools/export/export_smoke_app.cpp`
- Dependencies: `P0-04`
- Risk level: High
- Exact implementation steps:
  1. Rename synthetic paths to `DevBootstrap`.
  2. Add metadata to generated files identifying bootstrap/non-production mode.
  3. Prevent release packaging scripts from accepting dev bootstrap outputs.
  4. Update tests to assert both dev and release behavior.
- Acceptance criteria: Release artifacts cannot be mistaken for playable packages unless they contain real runtime binaries.
- Verification command or manual test: `.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke`

### Task P2-04

- Task ID: `P2-04`
- Title: Audit and close QuickJS/WindowCompat public stubs
- Files to edit: `runtimes/compat_js/window_compat.*`, `runtimes/compat_js/quickjs_runtime.cpp`
- Files to inspect: `tests/compat/*`, `docs/PROGRAM_COMPLETION_STATUS.md`
- Dependencies: None
- Risk level: Medium
- Exact implementation steps:
  1. List all public `STUB`/`PARTIAL` registered methods.
  2. For release-required methods, implement behavior.
  3. For non-required methods, ensure diagnostics and docs mark them unsupported.
  4. Add regression tests per method boundary.
- Acceptance criteria: No release-required compat method is stubbed or silently no-op.
- Verification command or manual test: `ctest -L weekly --output-on-failure`

## Phase 3 - Error Handling, Validation, Loading States, Empty States

### Task P3-01

- Task ID: `P3-01`
- Title: Fix AudioCore false-active playback state
- Files to edit: `engine/core/audio/audio_core.h`
- Files to inspect: `engine/core/audio/audio_runtime_backend.*`, `editor/audio/audio_inspector_panel.h`
- Dependencies: None
- Risk level: High
- Exact implementation steps:
  1. Check return values from `m_backend->play`.
  2. Roll back handles and current BGM/BGS/ME state on failure.
  3. Return success/failure from playback APIs where callers need feedback.
  4. Surface backend diagnostics in editor/runtime snapshots.
- Acceptance criteria: Missing audio assets do not appear active.
- Verification command or manual test: `ctest --preset dev-all -R "audio|AudioCore: GlobalState" --output-on-failure`

### Task P3-02

- Task ID: `P3-02`
- Title: Harden audio config parsing
- Files to edit: `engine/core/audio/audio_core.h`
- Files to inspect: `engine/core/global_state_hub.*`, `engine/core/settings/app_settings_store.*`
- Dependencies: None
- Risk level: Medium
- Exact implementation steps:
  1. Replace raw `std::stof` with safe parser.
  2. Clamp values and default invalid strings.
  3. Record diagnostics for invalid config.
  4. Add tests for malformed config values.
- Acceptance criteria: Malformed `audio.*` config cannot crash runtime.
- Verification command or manual test: `ctest --preset dev-all -R "audio|settings|GlobalState" --output-on-failure`

### Task P3-03

- Task ID: `P3-03`
- Title: Add release UI disabled/empty/error states for newly exposed panels
- Files to edit: selected `editor/*/*panel.*`
- Files to inspect: `docs/release/EDITOR_CONTROL_INVENTORY.md`, `tests/unit/test_*panel*.cpp`
- Dependencies: `P1-02`
- Risk level: Medium
- Exact implementation steps:
  1. For each exposed panel, define loading/empty/error/ready snapshot states.
  2. Disable controls with reasons when dependencies are unbound.
  3. Render user-visible errors in ImGui, not only JSON snapshots.
  4. Add tests for each state.
- Acceptance criteria: No exposed panel opens blank or with dead controls.
- Verification command or manual test: `ctest --preset dev-all -R "editor|panel|empty|loading|error" --output-on-failure`

### Task P3-04

- Task ID: `P3-04`
- Title: Surface runtime startup warnings in UI
- Files to edit: `apps/runtime/main.cpp`, `engine/core/scene/runtime_title_scene.*`
- Files to inspect: `engine/core/runtime_startup_services.cpp`, `engine/core/diagnostics/startup_diagnostics.*`
- Dependencies: `P0-01`
- Risk level: Medium
- Exact implementation steps:
  1. Pass startup report into title scene or diagnostics overlay.
  2. Display warnings such as missing localization catalog.
  3. Add command/detail view for critical startup errors where runtime can continue.
- Acceptance criteria: Startup issues are visible in-app, not only `stderr`.
- Verification command or manual test: Launch `urpg_runtime --headless --frames 1` and non-headless smoke with missing localization; verify title diagnostics.

## Phase 4 - Persistence/API/Storage/Data Integrity

### Task P4-01

- Task ID: `P4-01`
- Title: Persist runtime-created protagonist data
- Files to edit: `engine/core/character/*`, `engine/core/save/*`, character creator runtime files
- Files to inspect: `docs/PROGRAM_COMPLETION_STATUS.md`, `tests/unit/test_character_creation_screen.cpp`
- Dependencies: `P1-02`
- Risk level: Medium
- Exact implementation steps:
  1. Define save schema fields for runtime-created character identity.
  2. Write character identity during save.
  3. Restore identity during load/continue.
  4. Validate migration/default behavior for old saves.
- Acceptance criteria: Character created at runtime survives save/load.
- Verification command or manual test: `ctest --preset dev-all -R "character|save|migration" --output-on-failure`

### Task P4-02

- Task ID: `P4-02`
- Title: Enforce runtime bundle signatures at load time
- Files to edit: `engine/core/export/runtime_bundle_loader.*`, `engine/core/runtime_startup_services.cpp`
- Files to inspect: `engine/core/export/export_bundle_contract.*`, `engine/core/tools/export_packager_bundle_writer.cpp`
- Dependencies: `P0-04`
- Risk level: High
- Exact implementation steps:
  1. Validate bundle signature before using any `data.pck` payload.
  2. Validate entry integrity tags before exposing entries.
  3. Fail closed on tampering.
  4. Emit startup diagnostics with bundle path and reason.
  5. Add tamper tests through runtime startup.
- Acceptance criteria: Tampered `data.pck` prevents runtime boot/use.
- Verification command or manual test: `ctest --preset dev-all -R "runtime bundle|exported runtime|tamper" --output-on-failure`

### Task P4-03

- Task ID: `P4-03`
- Title: Apply persisted audio settings to runtime AudioCore
- Files to edit: `apps/runtime/main.cpp`, `engine/core/runtime_startup_services.cpp`, `engine/core/audio/audio_core.h`
- Files to inspect: `engine/core/settings/app_settings_store.*`
- Dependencies: `P3-02`
- Risk level: Medium
- Exact implementation steps:
  1. Read audio settings from runtime settings.
  2. Apply category volumes to `AudioCore` or `GlobalStateHub`.
  3. Ensure Options changes write back to settings.
  4. Add regression tests.
- Acceptance criteria: Runtime audio volume settings affect playback after restart.
- Verification command or manual test: `ctest --preset dev-all -R "audio|settings|persistence" --output-on-failure`

## Phase 5 - Asset, Manifest, Packaging, And Release-Readiness Fixes

### Task P5-01

- Task ID: `P5-01`
- Title: Define release-required asset manifest
- Files to edit: `imports/manifests/asset_bundles/*.json`, `docs/asset_intake/*`, `content/schemas/project.schema.json`
- Files to inspect: `resources/`, `imports/normalized/`, `tools/ci/run_release_candidate_gate.ps1`
- Dependencies: `P2-01`, `P2-02`
- Risk level: High
- Exact implementation steps:
  1. List every asset required for title, map, battle, UI, audio, icons, fonts.
  2. Mark each as release-required or dev-only.
  3. Ensure each release-required asset is license-cleared and packaged.
  4. Add CI verification for existence and hydration.
- Acceptance criteria: Release-required runtime surfaces do not depend on raw/vendor/LFS-only packs.
- Verification command or manual test: `.\tools\ci\run_release_candidate_gate.ps1`

### Task P5-02

- Task ID: `P5-02`
- Title: Complete public legal/privacy/distribution review evidence
- Files to edit: `docs/release/LEGAL_REVIEW_SIGNOFF.md`, `THIRD_PARTY_NOTICES.md`, `PRIVACY_POLICY.md`, `EULA.md`, `CREDITS.md`
- Files to inspect: all asset manifests and licenses
- Dependencies: `P5-01`
- Risk level: Blocker
- Exact implementation steps:
  1. Review third-party notices against shipped assets/dependencies.
  2. Review privacy policy against analytics behavior.
  3. Record qualified review approval or explicit public-release waiver.
  4. Update release report.
- Acceptance criteria: Public release legal blocker is closed in canonical release docs.
- Verification command or manual test: `rg -n "PARTIAL|qualified legal|public-release waiver|NOT RELEASE-READY" docs/release docs/APP_RELEASE_READINESS_MATRIX.md`

### Task P5-03

- Task ID: `P5-03`
- Title: Run and record remote release-candidate workflow
- Files to edit: `docs/release/AAA_RELEASE_READINESS_REPORT.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`
- Files to inspect: `.github/workflows/ci-gates.yml`
- Dependencies: all prior Phase 5 tasks
- Risk level: Blocker
- Exact implementation steps:
  1. Trigger manual GitHub Actions release-candidate workflow.
  2. Confirm same gates as local RC pass remotely.
  3. Record URL, commit SHA, date, and result.
- Acceptance criteria: Remote workflow blocker is closed with evidence URL.
- Verification command or manual test: Manual GitHub Actions run.

### Task P5-04

- Task ID: `P5-04`
- Title: Clear internal project audit release blockers
- Files to edit: readiness/signoff docs and records
- Files to inspect: `docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`, `docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`, `content/readiness/readiness_status.json`
- Dependencies: relevant implementation fixes for battle/save
- Risk level: Blocker
- Exact implementation steps:
  1. Complete human review for battle and save/data signoff.
  2. Record accept/reject decisions.
  3. Update readiness status only with evidence.
  4. Re-run project audit.
- Acceptance criteria: `releaseBlockerCount` is `0`.
- Verification command or manual test: `.\build\dev-ninja-debug\urpg_project_audit.exe --json`

## Phase 6 - Tests, Regression Coverage, And Final Verification

### Task P6-01

- Task ID: `P6-01`
- Title: Add end-to-end runtime title/input tests
- Files to edit: `tests/unit/test_runtime_title_scene.cpp` or existing runtime title tests
- Files to inspect: `engine/core/scene/runtime_title_scene.*`, `engine/core/input/input_core.h`
- Dependencies: `P0-01`, `P0-02`
- Risk level: Medium
- Exact implementation steps:
  1. Test focus movement.
  2. Test disabled command feedback.
  3. Test Confirm activates selected command once.
  4. Test Continue unavailable/available states.
- Acceptance criteria: Title navigation regressions fail tests.
- Verification command or manual test: `ctest --preset dev-all -R "RuntimeTitleScene" --output-on-failure`

### Task P6-02

- Task ID: `P6-02`
- Title: Add editor reachability smoke coverage for production panels
- Files to edit: `tests/unit/test_editor_panel_registry.cpp`, editor smoke tests
- Files to inspect: `apps/editor/main.cpp`, `engine/core/editor/editor_panel_registry.*`
- Dependencies: `P1-02`, `P3-03`
- Risk level: Medium
- Exact implementation steps:
  1. Assert every release top-level panel registers.
  2. Render every panel at least once headlessly.
  3. Assert no panel reports missing required host binding.
- Acceptance criteria: A newly hidden or dead production panel fails CI.
- Verification command or manual test: `ctest --preset dev-all -R "editor.*panel|editor.*smoke" --output-on-failure`

### Task P6-03

- Task ID: `P6-03`
- Title: Add export mode regression tests
- Files to edit: `tests/unit/test_export_packager.cpp`, `tests/integration/test_exported_runtime_smoke.cpp`
- Files to inspect: `engine/core/tools/export_packager*`
- Dependencies: `P0-04`, `P2-03`, `P4-02`
- Risk level: High
- Exact implementation steps:
  1. Assert release mode rejects missing runtime binary.
  2. Assert dev bootstrap mode remains available only when requested.
  3. Assert tampered bundle fails runtime startup.
  4. Assert staged real runtime launches smoke path.
- Acceptance criteria: Test suite prevents accidental placeholder release exports.
- Verification command or manual test: `ctest --preset dev-all -R "export|exported_runtime|package" --output-on-failure`

### Task P6-04

- Task ID: `P6-04`
- Title: Run full local validation gates
- Files to edit: none unless failures occur
- Files to inspect: failed logs
- Dependencies: all implementation tasks
- Risk level: High
- Exact implementation steps:
  1. Configure clean release/dev builds.
  2. Run local gates.
  3. Run presentation gate.
  4. Run release-candidate gate.
  5. Run pre-commit.
- Acceptance criteria: All local release gates pass without waivers.
- Verification command or manual test:

```powershell
pre-commit run --all-files
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1
```

### Task P6-05

- Task ID: `P6-05`
- Title: Final release readiness signoff
- Files to edit: `docs/release/AAA_RELEASE_READINESS_REPORT.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`, release signoff docs
- Files to inspect: CI run logs, audit JSON, release package outputs
- Dependencies: `P5-02`, `P5-03`, `P5-04`, `P6-04`
- Risk level: Blocker
- Exact implementation steps:
  1. Record exact commit SHA.
  2. Record all verification commands and results.
  3. Record remote workflow URL.
  4. Confirm project audit has zero release blockers.
  5. Create approved prerelease/release tag only after signoff.
- Acceptance criteria: Canonical release report no longer says `NOT RELEASE-READY`.
- Verification command or manual test:

```powershell
.\build\dev-ninja-debug\urpg_project_audit.exe --json
git tag -l
```
