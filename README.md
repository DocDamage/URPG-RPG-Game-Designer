# URPG (Universal RPG Engine) v3.1

![URPG Header](https://raw.githubusercontent.com/URPG-Project/assets/main/header.png)

**URPG** is a high-performance, deterministic C++ RPG engine designed for the modern era. It combines the flexibility of JavaScript-based plugin systems (compatible with RPG Maker MZ) with the power and safety of a native C++ kernel.

## 🚀 Key Capabilities

### 🛠️ Hybrid Native/Script Architecture
- **C++ Core Kernel:** Deterministic ECS iteration, Fixed32 (Q16.16) math, and a unified `EngineAssembly` lifecycle.
- **QuickJS Compat Harness:** A fixture-backed JavaScript contract bridge for validating MZ plugin surfaces while post-Phase-2 compat exit hardening continues.
- **Least-Privilege Security:** A robust `PluginSecurityManager` that sandboxes external scripts, enforcing permission-based access to system resources.

### 🎮 Battle & Gameplay Systems
- **Native Battle Engine:** Fully implemented turn-based logic (START -> INPUT -> ACTION -> VICTORY) running at native speeds.
- **Ability Framework:** A modular Gameplay Ability System (GAS) for complex skill interactions and pattern-based fields.
- **Multi-Genre Templates:** Out-of-the-box support for ARPG, VN (Visual Novel), and Tactics combat styles.

### 🍱 Asset & Data Management
- **URSV Binary Format:** A secure, versioned, and XOR-checksummed binary format for project data and saves.
- **Tiered Recovery:** Multi-level save corruption recovery (Autosave -> Metadata -> Safe Skeleton).
- **Resource Protection:** Integrated RLE/XOR compression and obfuscation to protect game assets in exported builds.

### 🎨 Editor & Tooling
- **ImGui Workspace:** A professional-grade editor shell with Scene Hierarchy, Asset Browser, and Property Inspectors.
- **Live Hot-Reload:** Support for hot-reloading textures, scripts, and localization files without restarting the engine.
- **Auto-Documentation:** A native `DocGenerator` that produces API documentation directly from header registries.

## 📊 Project Status (April 2026)

| Track | Status | Completion | Notes |
| --- | --- | --- | --- |
| **Foundation (Phase 0)** | Complete | 100% | Core kernels, authority guards, migration/save lanes. |
| **Native Core (Phase 1)** | Complete | 100% | Event dispatch, debug runtime, EngineShell lifecycle. |
| **Compat Layer (Phase 2 Closure)** | Complete | Runtime closure done | Runtime closure completed on 2026-04-19. Residual compat work remains in post-closure exit hardening, corpus depth, and honest `PARTIAL` surface maintenance. |
| **Wave 3-7 Ecosystem** | Complete | 100% | Templates, Profiling, Polish, Workspace, ImGui Panels. |
| **Final Integration** | Complete | 100% | Unified `EngineAssembly` Gold distribution. |
| **Native Workwaves** | In Progress | ~94% | Wave 1 closure evidence and broader release-readiness proof are now recorded; current active work shifts to Wave 2 opening plus ongoing compat hardening. |

Phase 2 runtime closure is complete as of 2026-04-19. Broader Wave 1/program release-readiness proof is now recorded as of 2026-04-20. Current repo work sits in Wave 2 implementation follow-through and post-closure compat exit hardening tracked in the canonical status docs.

Canonical remediation status: the audited remediation findings tracked in `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` are now fully closed; remaining repo work is roadmap/product execution and post-closure confidence depth rather than open remediation findings.

## 🏗️ Getting Started

### Prerequisites
- CMake 3.20+
- MSVC 2022 (Windows) or GCC/Clang (Linux/macOS)
- Python 3.9+ (for build scripting)

### Quick Build (Windows)
```powershell
# Configure and build
cmake --preset=dev-windows-debug
cmake --build --preset=dev-windows-debug

# Run tests
ctest --preset=dev-windows-debug -L pr
```

### Focused Presentation Validation
```powershell
# One-command focused presentation gate
pwsh -File .\tools\ci\run_presentation_gate.ps1

# Or run the combined presentation CTest gate directly
ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure
```

This focused gate is also invoked by `tools/ci/run_local_gates.ps1` locally and by `.github/workflows/ci-gates.yml` in CI.

## 📜 Documentation

- **[Master Blueprint](URPG_Blueprint_v3_1_Integrated.md):** The authoritative technical specification.
- **[Technical Debt Remediation Plan](docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md):** Canonical cross-cutting debt, truthfulness, intake-governance, roadmap-alignment, and reconciliation plan.
- **[Native Absorption Plan](docs/NATIVE_FEATURE_ABSORPTION_PLAN.md):** Canonical product roadmap.
- **[Completion Status](docs/PROGRAM_COMPLETION_STATUS.md):** Canonical latest-status snapshot and remaining checklist.
- **[Archive Index](docs/archive/README.md):** Superseded/reference-only planning material retained for traceability.
- **Detailed Planning Annexes:** [PGMMV + Native Absorption v2](docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md), [PGMMV Support Plan](docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md), and [earlier native absorption roadmap](docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md) are retained as traceability/reference inputs and route back through the remediation plan for canonical adoption.
- **[Presentation Docs Hub](docs/presentation/README.md):** Index of presentation contracts, validation, tooling, budgets, and schema docs.
- **[Presentation Validation Guide](docs/presentation/VALIDATION.md):** Focused runtime + spatial authoring validation commands and gate definitions.
- **[Intake Governance](docs/external-intake/):** External repository and private-use asset intake plans, watchlists, license matrices, and promotion guides.

### Build Artifacts (CI)
- **Nightly Matrix:** [tests_output.txt](tests_output.txt)
- **Latest Export:** [test_export.json](test_export.json) | [test_export.csv](test_export.csv)

### Current Runtime And Compat Notes
- Battle reward distribution, switch checks, and battle-event cadence are now closed in the compat lane; troop setup and drop logic remain honest later compat slices.
  - Native Battle Core now owns live `BattleScene` phase/action/rule flow, battle diagnostics can bind real scene preview state, the presentation bridge can derive battle participants from an active `BattleScene`, and battle migration emits warnings for unsupported troop-page/action payloads instead of silently dropping them.
  - Save/Data now has explicit native schema artifacts for save policies, slot descriptors, metadata, and migrations, and the diagnostics save tab now exposes workspace-level save actions plus autosave policy, retention limits, metadata-registry fields, slot descriptors, recovery diagnostics, serialization schema summaries, selected-row state, and live policy draft/validation/apply workflow from the native runtime path.
  - Async plugin dispatch remains deterministic FIFO, but its status is now tracked as `PARTIAL` because callbacks run on the worker thread and the JS bridge is still fixture-backed.
  - Input/Touch QuickJS API registration now routes to live runtime state (no placeholder zeros) and `TouchInput` movement/tap tracking now computes `moveSpeed` + `tapCount`.
  - PluginManager failure-path diagnostics now emit deterministic JSONL artifacts (`exportFailureDiagnosticsJsonl` / `clearFailureDiagnostics`) with operation tags + sequence IDs.
  - PluginManager diagnostics JSONL now include compat severity tags (`WARN`, `SOFT_FAIL`, `HARD_FAIL`, `CRASH_PREVENTED`) for downstream report classification.
  - PluginManager failure diagnostics export now enforces bounded retention (last 2048 events) while preserving monotonic sequence IDs across trims; coverage is gated in unit tests.
  - `executeCommandByName` now routes through exact registered full keys (supports underscore-heavy command names) and rejects missing plugin/command segments via deterministic `execute_command_by_name_parse` diagnostics.
  - QuickJS compat harness now supports explicit eval failure directives (`@urpg-fail-eval`), explicit call-failure directives (`@urpg-fail-call`), deterministic context-init failure marker support (`__urpg_fail_context_init__`), and evalModule failure propagation tests for deterministic conformance.
  - Fixture command validation now fails malformed payloads deterministically (`js` must be string, `script` must be array, `dropContextBeforeCall` must be boolean, optional `entry`/`description`/`mode` metadata must be strings, `mode` values are restricted to `const` or `arg_count`, and commands cannot declare both `js` and `script`) with exported diagnostics operation tags.
  - Fixture metadata shape validation now fails malformed `dependencies`/`parameters`/`commands` containers and non-string dependency entries deterministically with exported diagnostics operation tags.
  - Fixture script runtime now supports explicit `error` op and unknown-op hard-fail behavior, surfaced through deterministic `execute_command_quickjs_call` diagnostics artifacts.
  - Dependent plugin command execution is now gated when required dependencies are missing, surfaced through deterministic `execute_command_dependency_missing` diagnostics artifacts.
  - Compat report model now ingests PluginManager diagnostics JSONL (`ingestPluginFailureDiagnosticsJsonl`) into timeline events and per-plugin error summaries.
  - Compat report ingestion now maps PluginManager compat severity tags into timeline severity levels (`WARNING`/`ERROR`/`CRITICAL`).
  - Compat report panel runtime refresh now consumes and clears PluginManager diagnostics artifacts each update cycle (`CompatReportPanel::refresh`/`update`).
  - Compat report diagnostics model hardening now updates warning/error flags when method statuses transition and sorts call-volume views by total calls (including unsupported operations).
  - Compat report panel now records bounded per-plugin session score history plus first-seen/last-updated timestamps, and `LAST_UPDATED` sorting now projects human-readable recency labels.
  - Compat module unit suites (`test_battlemgr`, `test_data_manager`, `test_audio_manager`, `test_input_manager`, `test_plugin_manager`) are active in `urpg_tests`.
- Active build wiring snapshot:
  - `urpg_core` currently builds core kernels + editor diagnostics/panel + compat report panel + the QuickJS compat harness + WindowCompat + Battle/Data/Audio/Input/Plugin compat modules.
  - WindowCompat currently includes renderer command emission, selectable-window keyboard/gamepad navigation, richer pointer press/drag/release behavior, mouse-wheel scrolling, and contents-backed bitmap metadata tracking.
  - `urpg_tests` includes the full compat unit slice in active CMake targets.
- CI gate suites:
  - Gate 1 (PR): `ctest -L pr`
  - Gate 2 (nightly): `ctest -L nightly` (integration + snapshot suites)
  - Gate 3 (weekly): `ctest -L weekly` (compat suite)
  - Nightly renderer-tier matrix (`basic`, `standard`, `advanced`) + test log artifacts in CI
  - Known-break waiver validation via `tools/ci/check_waivers.ps1`
- Migration CLI: `urpg_migrate`
- Focused validation snapshots move faster than this README.
  - See [docs/PROGRAM_COMPLETION_STATUS.md](./docs/PROGRAM_COMPLETION_STATUS.md) for the current status-date and the latest recorded focused validation commands/results.

## Immediate next steps

1. ~~Finish UI/Menu Wave 1 closure~~ (DONE 2026-04-19):
   - menu authoring/preview editor surfaces shipped with true edit workflows,
   - schema + import mapping finalized for route fallback and state rules,
   - integration anchors added for runtime+editor parity.
2. ~~Finish Message/Text renderer closure~~ (DONE 2026-04-19):
   - native `MapScene` message runner now submits `TextCommand`/`RectCommand` to `RenderLayer`,
   - `OpenGLRenderer` handles `RectCommand` explicitly,
   - `Window_Message` compat parity verified with RenderLayer emission tests.
3. ~~Finish Message/Text editor and migration productization~~ (DONE 2026-04-19):
   - `MessageInspectorModel` now supports page body/speaker/mode editing, add/remove, and apply-to-runtime,
   - `DiagnosticsWorkspace` exposes message edit, export, save/load round-trips,
   - migration maps `defaultChoiceIndex`, `command`, window/audio style fields,
   - schema updated with `default_choice_index` and `command`.
4. ~~Finish the broader Wave 1/program release-readiness proof and final status-doc reconciliation~~ (DONE 2026-04-20):
   - focused plugin/scene lane passed on the active local `build` tree,
   - focused presentation gate passed on the active local `build` tree,
   - Phase 4 governance validation passed after restoring the canonical `source_capture_status.json` report,
   - `ctest --preset dev-all --output-on-failure` passed at 630/630 after reconciling the stale Battle inspector turn-count expectation with the current native flow contract.
5. Maintain post-Phase-2 compat exit hardening:
   - keep conformance depth, JSONL/report/panel parity, and status/document truthfulness aligned with the current harness-backed scope.
6. Begin Wave 2 advanced capability implementation (ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, and editor utilities).
7. Execute the remaining checklist in `docs/PROGRAM_COMPLETION_STATUS.md` to drive completion to 100% for the current program scope.

## Build

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest --preset dev-all
```

On Windows, test executable relinks now automatically clear stale process locks (`urpg_tests.exe`, `urpg_compat_tests.exe`, `urpg_integration_tests.exe`, `urpg_snapshot_tests.exe`). Disable if needed with `-DURPG_WINDOWS_UNLOCK_TEST_BINARIES=OFF`.

## Build cache (sccache)

```powershell
$env:SCCACHE_DIR = "$PWD/.cache/sccache"
sccache --zero-stats
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
sccache --show-stats
```

`URPG_USE_SCCACHE` is enabled by default in `CMakePresets.json`.

## Git LFS

```powershell
git lfs install
git lfs track
```

## Local gates

```powershell
.\tools\ci\run_local_gates.ps1
```

## Wave 1 spec checklist sync

```powershell
.\tools\docs\sync-wave1-spec-checklist.ps1
.\tools\docs\sync-wave1-spec-checklist.ps1 -Check
```

## CodeMunch Pro Workflow

```powershell
.\tools\codemunch\bootstrap-project.ps1
.\tools\codemunch\index-project.ps1 -ProjectRoot . -Embed -OutFile ".codemunch\last-index.json"
```

See `tools/codemunch/README.md` for full usage and cross-project setup.

## ContextLattice Workflow

```powershell
.\tools\contextlattice\bootstrap-project.ps1
.\tools\contextlattice\verify.ps1
```

See `tools/contextlattice/README.md` for env and smoke-test usage.

## MemPalace to ContextLattice Bridge

```powershell
.\tools\memorybridge\bootstrap-project.ps1
.\tools\memorybridge\sync-from-mempalace.ps1 -DryRun
```

Use this for one-way sync (`MemPalace -> ContextLattice`) so ContextLattice
remains the shared canonical memory backend.

See `tools/memorybridge/README.md` for mapping and incremental sync details.

## Unified LLM Workflow

Install one global command (once):

```powershell
.\tools\workflow\install-global-llm-workflow.ps1
```

Then in any project folder run:

```powershell
llm-workflow-up
```

Strict end-to-end check:

```powershell
llm-workflow-check
```

Environment and prerequisite diagnostics:

```powershell
llm-workflow-doctor -CheckContext
```

This command auto-creates missing `tools/codemunch`, `tools/contextlattice`,
and `tools/memorybridge` from templates, loads `.env`, installs required
dependencies, normalizes provider keys for OpenAI/Kimi/Gemini/GLM usage, and
runs bootstrap/verification checks.

Provider selection examples:

```powershell
llm-workflow-up -Provider glm
llm-workflow-up -Provider gemini
llm-workflow-check -Provider kimi
llm-workflow-doctor -Provider auto -CheckContext -Strict
```

## Contributor guide

See `CONTRIBUTING.md` for workflow, LFS policy, and asset hygiene checks.
PR verification uses `.github/PULL_REQUEST_TEMPLATE.md`, including the focused presentation gate when presentation/spatial behavior is touched.

```powershell
python .\tools\assets\asset_hygiene.py --write-reports --prune-junk
.\tools\rpgmaker\validate-plugin-dropins.ps1
```

## Pre-commit

```powershell
python -m pip install pre-commit
pre-commit install
pre-commit run --all-files
```

## Migration CLI

```powershell
.\build\urpg_migrate.exe --input <project.json> --migration tools\migrate\migration_op.json --output <out.json>
```

## Next lane

Primary execution source of truth:

1. `docs/PROGRAM_COMPLETION_STATUS.md`
2. `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`

---
*Built with ❤️ by the URPG Team. Part of the RPG Game Maker ecosystem.*
