# URPG (Universal RPG Engine) — Agent Guide

> This file is written for AI coding agents. Expect the reader to know nothing about the project. All information below is derived from the actual repository contents.

---

## Project Overview

URPG is a high-performance, deterministic C++20 RPG engine that combines a native C++ kernel with a JavaScript compatibility layer for RPG Maker MZ plugins. It is built around a custom ECS (Entity Component System), deterministic Fixed32 math, tiered rendering, and an ImGui-based editor workspace.

- **Language**: C++20 (required, extensions off)
- **Build System**: CMake 3.23+
- **Primary Platforms**: Windows (MSVC 2022, MinGW, Ninja), Linux/macOS (GCC/Clang)
- **Graphics**: OpenGL (optional; headless CI builds skip it)
- **Windowing / Input**: SDL2
- **JSON**: nlohmann/json
- **Testing**: Catch2 v3
- **Scripting Runtime**: QuickJS (in `runtimes/compat_js/`)

---

## Directory Layout

```
engine/
  api/              High-level game API (EngineAPI singleton)
  core/             Core kernels: ECS, render, audio, save, battle, message, events, etc.
  gameplay/         Combat calc, status effects, scene helpers
  runtimes/bridge/  Value bridge for native/script interop
editor/             ImGui editor panels and models (diagnostics, battle, save, message, ability, spatial, ui)
runtimes/compat_js/ QuickJS runtime, MZ-compat modules (Window, Battle, Data, Audio, Input, Plugin managers)
tests/
  unit/             Fast isolated unit tests (~80 files)
  integration/      Cross-subsystem tests
  snapshot/         Canonical output / render snapshot tests
  compat/           RPG Maker MZ compatibility & plugin authority tests
  engine/core/      Low-level engine tests
tools/
  ci/               Gate scripts, waiver checks, test-binary unlock helpers
  docs/             Doc link checker, Wave 1 spec checklist sync
  assets/           Asset hygiene, asset DB, duplicate pruning
  rpgmaker/         Plugin drop-in validation
  migrate/          Migration CLI source and example JSON specs
  workflow/         LLM workflow bootstrapping, aider wrappers
  codemunch/        Project indexing (created by bootstrap)
  contextlattice/   Context memory backend (created by bootstrap)
  memorybridge/     MemPalace -> ContextLattice sync
docs/
  adr/              Architecture Decision Records (ADR-001 through ADR-011)
  presentation/     Presentation subsystem docs, validation, performance budgets
  external-intake/  Asset intake plans, license matrices
  archive/          Superseded planning material
```

---

## Build and Test Commands

### Configure

```powershell
# Recommended local preset (Ninja + Debug)
cmake --preset dev-ninja-debug

# Visual Studio 2022
cmake --preset dev-vs2022

# MinGW Debug
cmake --preset dev-mingw-debug

# CI headless (no OpenGL, Release, sccache)
cmake --preset ci
```

### Build

```powershell
# Ninja Debug
cmake --build --preset dev-debug

# Ninja Release
cmake --build --preset dev-release

# VS2022 Debug
cmake --build --preset dev-vs2022-debug

# CI Release
cmake --build --preset ci-release
```

### Test

```powershell
# Run all PR-level tests
ctest --preset dev-all

# Or labeled gates
ctest -L pr               # Gate 1 — fast unit + presentation lanes
ctest -L nightly          # Gate 2 — integration + snapshot + release validation
ctest -L weekly           # Gate 3 — compat suite

# Focused presentation gate (single command)
.\tools\ci\run_presentation_gate.ps1

# Full local validation pipeline
.\tools\ci\run_local_gates.ps1
```

### Build Cache (sccache)

```powershell
$env:SCCACHE_DIR = "$PWD/.cache/sccache"
sccache --zero-stats
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
sccache --show-stats
```

`sccache` is enabled by default in `CMakePresets.json` and the `ci` preset.

---

## Code Style Guidelines

### EditorConfig (enforced)

- Charset: UTF-8
- End of line: LF
- Insert final newline
- Trim trailing whitespace
- Indent: spaces
- C/C++ (`*.cpp`, `*.h`, `*.hpp`): indent size **4**
- Python / PowerShell / JSON / YAML / Markdown / CMake: indent size **2**
- Makefiles: tabs

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Files | `snake_case.h` / `.cpp` | `ability_system_component.h` |
| Namespaces | `urpg` root, nested by domain | `urpg::ecs`, `urpg::editor` |
| Classes / Structs | `PascalCase` | `AbilitySystemComponent` |
| Functions / Methods | `camelCase` | `getAttribute`, `runFullMigration` |
| Private members | `m_` prefix or trailing `_` | `m_model`, `workspace_` |
| Type aliases / Enums | `PascalCase` | `EntityID`, `SceneType` |
| Enum values | `SCREAMING_SNAKE_CASE` or `PascalCase` | `SceneType::BOOT`, `CompatStatus::FULL` |

### C++ Style

- C++20 required; no extensions (`CMAKE_CXX_EXTENSIONS OFF`)
- MSVC: `/FS /EHsc`
- Headers use `#pragma once`
- Doxygen `/** @brief */` comments are common
- Modern C++: `std::optional`, smart pointers, `enum class`, template variadics
- Heavy use of `nlohmann::json` for diagnostics and serialization

### Python / PowerShell Tooling

- Python files under `tools/` are linted and formatted by **Ruff** (via pre-commit).
- PowerShell is the primary orchestration language for CI/local gates.

---

## Testing Instructions

### Framework

- **Catch2 v3** (`#include <catch2/catch_test_macros.hpp>`), linked as `Catch2::Catch2WithMain`.
- Discovery: `catch_discover_tests(... DISCOVERY_MODE PRE_TEST)` registers individual `TEST_CASE`s as CTest tests.
- Tags drive CI lanes (e.g., `[battle]`, `[save]`, `[render]`, `[compat]`, `[presentation]`, `[editor]`, `[spatial]`).

### Test Executables

| Executable | Sources | CTest Label |
|---|---|---|
| `urpg_tests` | `tests/unit/*.cpp`, `tests/engine/core/*.cpp`, root-level `test_*.cpp` | `pr` |
| `urpg_integration_tests` | `tests/integration/*.cpp` | `nightly` |
| `urpg_snapshot_tests` | `tests/snapshot/*.cpp` | `nightly` |
| `urpg_compat_tests` | `tests/compat/*.cpp` | `weekly` |
| `urpg_presentation_release_validation` | `engine/core/presentation/release_validation.cpp` | `nightly;presentation` |

### Special Test Lanes

- `urpg_presentation_unit_lane`: runs `urpg_tests "[presentation]"` — label `pr;presentation`
- `urpg_spatial_editor_lane`: runs `urpg_tests "[editor][spatial]"` — label `pr;presentation;editor`

### Writing Tests

- Use `TEST_CASE("descriptive name", "[tags]")` and `SECTION("...")`.
- Use `REQUIRE` / `REQUIRE_FALSE` for assertions.
- Tests commonly use inline JSON strings and temporary directories; compat tests reference real plugin JSON manifests under `tests/compat/fixtures/plugins/`.

---

## CI / CD

Three gates are defined in `.github/workflows/ci-gates.yml`:

- **Gate 1 (`pr`)**: Runs on PRs, pushes to `main`, and `workflow_dispatch`. Builds with `ci` preset, validates waivers and doc links, runs `ctest -L pr`, and runs the focused presentation gate.
- **Gate 2 (`nightly`)**: Daily at 06:00 UTC. Matrix across `renderer_tier: [basic, standard, advanced]`. Runs `ctest -L nightly` and uploads logs.
- **Gate 3 (`weekly`)**: Mondays at 07:00 UTC. Validates curated RPG Maker plugin drop-ins, then runs `ctest -L weekly`.

Additional workflows:

- `.github/workflows/security-scans.yml`: Gitleaks (secret scanning) and CodeQL (C++ / Python) on PR/push and weekly.

---

## Security Considerations

- **Plugin Sandboxing**: `PluginSecurityManager` enforces permission-based access for external JS scripts running in the QuickJS compat harness.
- **Resource Protection**: Exported builds use RLE/XOR compression and obfuscation to protect game assets.
- **Save Integrity**: URSV binary format uses versioning and XOR checksums. Multi-level recovery (Autosave -> Metadata -> Safe Skeleton) mitigates corruption.
- **Secret Scanning**: Gitleaks runs on every PR/push and weekly.
- **CodeQL**: Static analysis for C++ and Python runs on PR/push and weekly.
- **Known-Break Waivers**: `tools/ci/check_waivers.ps1` validates that any accepted known breaks have IDs, issue URLs, scopes, and non-expired dates.

---

## Development Conventions

### Before Opening a PR

```powershell
.\tools\ci\run_local_gates.ps1
pre-commit run --all-files
```

Conditional additional checks:

- Presentation / spatial / rendering changes: ` .\tools\ci\run_presentation_gate.ps1`
- RPG Maker MZ DLC changes: `.\tools\rpgmaker\validate-plugin-dropins.ps1`
- Asset imports / reorganization: `python .\tools\assets\asset_hygiene.py --write-reports`

### Binary and Asset Policy

- Large binaries must be tracked in **Git LFS** (`.gitattributes` is the source of truth).
- Do not commit OS noise files (`__MACOSX`, `.DS_Store`, `Thumbs.db`, `._*`).
- Keep imports under `imports/` and vendor packs under `third_party/`.
- Put generated reports under `imports/reports/` or tool-specific `reports/` folders.

### Commit Hygiene

- One concern per commit.
- Include a short verification note in the commit message body when useful.
- Never rewrite or delete user-authored changes you did not make unless explicitly requested.

---

## Key Tooling

| Tool | Purpose |
|---|---|
| `urpg_migrate` | Standalone C++ CLI for project migration (`--input`, `--migration`, `--output`) |
| `tools/ci/run_local_gates.ps1` | Full local CI pipeline |
| `tools/ci/run_presentation_gate.ps1` | Focused presentation / spatial-editor validation |
| `tools/docs/sync-wave1-spec-checklist.ps1` | Canonical spec checklist code-gen and drift check |
| `tools/docs/check-presentation-doc-links.ps1` | Markdown link linter for `docs/presentation/` |
| `tools/assets/asset_hygiene.py` | Junk-file, oversize-file, and duplicate detection |
| `tools/rpgmaker/validate-plugin-dropins.ps1` | Curated plugin lint and conflict detection |
| `tools/workflow/bootstrap-llm-workflow.ps1` | Bootstraps `codemunch` + `contextlattice` + `memorybridge` |

---

## Important Architectural Notes

- **Presentation Core Ownership**: Presentation Core is the sole source of truth for visual interpretation; the Render Backend consumes `FrameIntent` and cannot access Game State directly.
- **Decoupled Threading**: Game Thread -> Presentation Thread -> Render Thread with immutable `PresentationFrameIntent` and arena allocation.
- **Zero Heap in Hot Loop**: Arena allocation is preferred in performance-critical paths.
- **Non-Destructive Migration**: Legacy projects are upgraded via inference -> decoration -> upgrade stages; original data remains untouched.
- **Subsystem Landing Policy**: A subsystem is not considered "landed" unless it is built, test-registered, and reachable from a real runtime or editor entry point.
- **Spatial Editor Status**: `editor/spatial/` and `engine/core/presentation/` contain header-only incubating code that is not registered in the main `CMakeLists.txt` build. See ADR-011.

---

## Canonical Status Documents

- `docs/PROGRAM_COMPLETION_STATUS.md` — latest status snapshot and remaining checklist
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` — cross-cutting debt and reconciliation plan
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` — product roadmap
- `docs/presentation/VALIDATION.md` — focused validation commands and gate definitions
- `docs/AI_COPILOT_GUIDE.md` — LLM integration patterns and command parsing

---

*Last updated: 2026-04-19*
