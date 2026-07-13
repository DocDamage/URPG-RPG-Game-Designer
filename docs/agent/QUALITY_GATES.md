# Quality Gates

Use the narrowest command that covers the changed surface. If a release plan specifies a command, run that command exactly.

`ctest -R` matching is case-sensitive. Use the casing from discovered CTest names (`ctest --test-dir <build-dir> -N`) and avoid lowercase or underscore aliases unless those aliases are present in the discovered test list.

Use anchored label presets for common lanes. Raw `ctest -L pr` is regex-based and also matches labels such as `presentation`; prefer `ctest --preset dev-pr`.

On Windows, configure the default Ninja debug tree with `.\tools\ci\configure_dev_ninja_debug.ps1`. It detects stale CMake caches where standalone Clang or Clang-only debug flags leaked into the GCC preset and runs a fresh preset configure before builds.

## Command Map

| Change Area | Command |
| --- | --- |
| General PR-level changes | `ctest --preset dev-pr --output-on-failure` |
| Creator-journey baseline | `ctest --test-dir build/dev-ninja-debug -R "creator journey" --output-on-failure`; then `.\tools\ci\check_creator_journey.ps1 -BuildDirectory build/dev-ninja-debug` |
| Creator shell, session, project creation, wizard, and checklist (M1/M5) | `ctest --preset dev-all -R "ProjectCreationService|NewProjectWizard|project template generator|CreatorChecklist|Runtime project preflight|Startup|MainMenu|EditorProjectSession|EditorDirtyStateRegistry|settings|editor app panels" --output-on-failure` |
| Virtual external asset catalog (M2) | `python -m unittest tools.assets.tests.test_asset_db tools.assets.tests.test_catalog_interchange -v`; then `.\build\dev-ninja-debug\urpg_tests.exe "[assets][local_catalog]" --reporter compact` and the focused catalog performance target when catalog query/index code changes |
| Governed asset preview/archive/attachment/drag workflow (M3) | `python -m unittest tools.assets.tests.test_global_asset_import -v`; then `.\build\dev-ninja-debug\urpg_tests.exe "[assets][thumbnail],[assets][archive],[assets][drag_drop],[assets][promotion],[assets][attachment]" --reporter compact` |
| Unified Map context/history/persistence/layout (M4) | `ctest --preset dev-spatial --output-on-failure`; then run `.\build\dev-ninja-debug\urpg_tests.exe "[spatial][map_authoring]" --reporter compact` and record manual `level_builder`/`spatial_authoring` route equivalence when changing visible layout or routing |
| Runtime startup/settings/input | `ctest --preset dev-all -R "startup|settings|input" --output-on-failure` |
| Runtime input, pause/resume, and title/menu navigation | `ctest --preset dev-all -R "startup|settings|input|SceneManager|RuntimeTitleScene" --output-on-failure` |
| Map scene/render assets | `ctest --preset dev-all -R "MapScene|AssetLibrary|Runtime map asset" --output-on-failure` |
| Battle assets/authoring | `ctest --preset dev-all -R "battle.*assets|battle.*authoring" --output-on-failure` |
| Compat JS / WindowCompat / plugin fixtures | `ctest -L weekly --output-on-failure` |
| High-end MZ compatibility expansion | `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_report],[compat][mz_event_commands],[compat][mz_visual_diff],[compat][mz_runtime_parity]" --reporter compact`; `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_corpus],[compat][mz_parity_evidence],[editor][compat][mz_workbench]" --reporter compact`; `.\build\dev-ninja-debug\urpg_tests.exe "[plugin][compatibility][mz_high_end]" --reporter compact`; `.\tools\ci\check_mz_project_corpus.ps1` |
| Export packager/validator | `ctest --preset dev-export --output-on-failure` |
| Release bundle/script protection | `.\build\dev-ninja-debug\urpg_export_unit_tests.exe "ExportPackager transforms script payloads when obfuscateScripts is enabled" --reporter compact`; `.\build\dev-ninja-debug\urpg_export_unit_tests.exe "[runtime_bundle]" --reporter compact`; run the focused export packager protection tests touched by the change |
| AI assistant/tool review surface | `ctest --preset dev-all -R "AI (knowledge|task|tool|assistant)|Chatbot component" --output-on-failure` |
| Snapshot/golden visual baselines | `ctest --preset dev-snapshot --output-on-failure` |
| Presentation/spatial/rendering | `ctest --preset dev-spatial --output-on-failure`; then `.\tools\ci\run_presentation_gate.ps1` when touching rendering/presentation gates |
| Native Level Builder / grid-part editor | `.\build\dev-ninja-debug\urpg_tests.exe "[grid_part][editor]"`; then `ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure` |
| Grid-part runtime/compiler/package governance | `ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure` |
| Release authoring persistence / save-load paths | `ctest --preset dev-all -R "settings|persistence|save|load|grid_part|Ability" --output-on-failure` |
| Release-required assets, promoted library, and LFS scope | `.\tools\ci\check_release_required_assets.ps1`; `.\tools\ci\check_promoted_asset_library.ps1`; `.\tools\ci\check_lfs_release_scope.ps1`; then `ctest --preset dev-all -R "AssetLibrary|Runtime map asset|preflight|asset" --output-on-failure` |
| Repository truth guards | `.\tools\ci\check_no_generated_tracked_files.ps1`; `.\tools\ci\check_no_production_system_calls.ps1`; `.\tools\ci\check_lfs_release_scope.ps1` |
| Release-surface regression tests | `ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|editor app panels|Community WYSIWYG|curated save-data lifecycle" --output-on-failure` |
| WYSIWYG readiness/done-rule changes | `ctest --preset dev-all -R "WYSIWYG|readiness_status" --output-on-failure` |
| Native version metadata | `.\build\dev-ninja-release\urpg_runtime.exe --version`; then `.\build\dev-ninja-release\urpg_editor.exe --version` |
| Native package layout | `.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke` |
| Full local gate | `.\tools\ci\run_local_gates.ps1` |

## Knowledge Health

Run this when editing `AGENTS.md`, `docs/agent/`, or release execution plans:

```powershell
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

The check enforces that `AGENTS.md` stays small, required agent index docs exist, and runnable `ctest -R` commands in active markdown docs match at least one discovered test when a build directory is available. For alternations, each top-level or grouped branch must independently match a discovered test.
