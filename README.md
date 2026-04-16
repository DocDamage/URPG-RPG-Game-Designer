# URPG (Universal RPG Engine) v3.1

![URPG Header](https://raw.githubusercontent.com/URPG-Project/assets/main/header.png)

**URPG** is a high-performance, deterministic C++ RPG engine designed for the modern era. It combines the flexibility of JavaScript-based plugin systems (compatible with RPG Maker MZ) with the power and safety of a native C++ kernel.

## 🚀 Key Capabilities

### 🛠️ Hybrid Native/Script Architecture
- **C++ Core Kernel:** Deterministic ECS iteration, Fixed32 (Q16.16) math, and a unified `EngineAssembly` lifecycle.
- **QuickJS Integration:** A high-speed JavaScript bridge allowing for extensive plugin extensibility without sacrificing engine stability.
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
| **Compat Layer (Phase 2)** | Complete | 100% | Full suite of MZ-compatible stubs with QuickJS. |
| **Wave 3-7 Ecosystem** | Complete | 100% | Templates, Profiling, Polish, Workspace, ImGui Panels. |
| **Final Integration** | Complete | 100% | Unified `EngineAssembly` Gold distribution. |
| **Native Workwaves** | In Progress | ~85% | Native ownership for Message/Text and Battle Core. |

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

## 📜 Documentation

- **[Master Blueprint](URPG_Blueprint_v3_1_Integrated.md):** The authoritative technical specification.
- **[Native Absorption Plan](docs/NATIVE_FEATURE_ABSORPTION_PLAN.md):** Roadmap for migrating legacy features.
- **[Completion Status](docs/PROGRAM_COMPLETION_STATUS.md):** Granular checklist of every implemented feature.

### Build Artifacts (CI)
- **Nightly Matrix:** [tests_output.txt](tests_output.txt)
- **Latest Export:** [test_export.json](test_export.json) | [test_export.csv](test_export.csv)

---
*Built with ❤️ by the URPG Team. Part of the RPG Game Maker ecosystem.*
  - Burned down BattleManager compat status: `processEscape` advanced from `PARTIAL` to `FULL` with deterministic MZ-style escape ratio/failure ramp semantics.
  - Burned down PluginManager compat status: `executeCommandAsync` advanced from `PARTIAL` to `FULL` with deterministic FIFO task-queue execution + callback ordering.
  - Input/Touch QuickJS API registration now routes to live runtime state (no placeholder zeros) and `TouchInput` movement/tap tracking now computes `moveSpeed` + `tapCount`.
  - PluginManager failure-path diagnostics now emit deterministic JSONL artifacts (`exportFailureDiagnosticsJsonl` / `clearFailureDiagnostics`) with operation tags + sequence IDs.
  - PluginManager diagnostics JSONL now include compat severity tags (`WARN`, `SOFT_FAIL`, `HARD_FAIL`, `CRASH_PREVENTED`) for downstream report classification.
  - PluginManager failure diagnostics export now enforces bounded retention (last 2048 events) while preserving monotonic sequence IDs across trims; coverage is gated in unit tests.
  - `executeCommandByName` now routes through exact registered full keys (supports underscore-heavy command names) and rejects missing plugin/command segments via deterministic `execute_command_by_name_parse` diagnostics.
  - QuickJS stub lane now supports explicit eval failure directives (`@urpg-fail-eval`), explicit runtime call-failure directives (`@urpg-fail-call`), deterministic context-init failure marker support (`__urpg_fail_context_init__`), and evalModule failure propagation tests for deterministic conformance.
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
  - `urpg_core` currently builds core kernels + editor diagnostics/panel + compat report panel + QuickJS + WindowCompat + Battle/Data/Audio/Input/Plugin compat modules.
  - `urpg_tests` includes the full compat unit slice in active CMake targets.
- CI gate suites:
  - Gate 1 (PR): `ctest -L pr`
  - Gate 2 (nightly): `ctest -L nightly` (integration + snapshot suites)
  - Gate 3 (weekly): `ctest -L weekly` (compat suite)
  - Nightly renderer-tier matrix (`basic`, `standard`, `advanced`) + test log artifacts in CI
  - Known-break waiver validation via `tools/ci/check_waivers.ps1`
- Migration CLI: `urpg_migrate`
- Catch2/CTest baseline (Debug snapshot, 2026-04-15):
  - `urpg_tests`: 3907 assertions / 287 test cases

## Immediate next steps

1. Close compat-lane exit criteria for trustworthy import, diagnostics, and migration confidence.
2. Finish Message/Text renderer closure after the landed bridge:
   - consume `RenderLayer::TextCommand` in backend tiers where text command rendering is still placeholder,
   - connect `Window_Message` parity behavior to native message scene runtime ownership path.
3. Finish UI/Menu Wave 1 closure after the landed runtime interaction slices:
   - ship menu authoring/preview editor surfaces,
   - finalize schema + import mapping for route fallback/state rules,
   - add integration anchors beyond unit tests.
4. Continue Wave 1 runtime/editor/schema closure for Message/Text, Battle, and Save/Data.
5. Begin Wave 2 advanced capability implementation (ability framework, pattern editor, modular level assembly, sprite pipeline, procedural toolkit, optional 2.5D lane, timeline orchestration, and editor utilities).
6. Execute the full remaining checklist in `docs/PROGRAM_COMPLETION_STATUS.md` to drive completion to 100% for the current program scope.

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
