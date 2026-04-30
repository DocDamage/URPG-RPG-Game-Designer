# URPG Agent Guide

This file is the table of contents for agent work in URPG. Keep it short. Deeper and more frequently changing guidance lives under `docs/agent/` and the canonical subsystem docs linked there.

## Project Snapshot

URPG is a deterministic C++20 RPG engine with a native core, RPG Maker MZ QuickJS compatibility layer, OpenGL/headless rendering paths, an ImGui editor, CMake/Ninja builds, and Catch2 tests.

## Start Here

- Agent knowledge index: `docs/agent/INDEX.md`
- Architecture map: `docs/agent/ARCHITECTURE_MAP.md`
- Validation command map: `docs/agent/QUALITY_GATES.md`
- Execution workflow: `docs/agent/EXECUTION_WORKFLOW.md`
- Current debt and release truth: `docs/agent/KNOWN_DEBT.md`

## Common Commands

Configure and build:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
```

Run tests:

```powershell
ctest --preset dev-all
ctest --preset dev-pr
ctest --preset dev-spatial
ctest --preset dev-export
ctest --preset dev-project-audit
ctest --preset dev-process
ctest -L nightly
ctest -L weekly
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_local_gates.ps1
```

## Non-Negotiable Rules

- Do not revert or delete user-authored changes unless explicitly requested.
- Prefer existing subsystem patterns over new abstractions.
- Use `rg`/`rg --files` for repo search.
- Use `apply_patch` for manual file edits.
- Keep generated reports under the repo’s existing report folders.
- Treat canonical docs as source of truth; update docs when behavior or status changes.

## Where Code Lives

- `engine/core/`: native runtime kernels.
- `editor/`: editor panels and models.
- `runtimes/compat_js/`: QuickJS and RPG Maker MZ compatibility surfaces.
- `tests/unit/`, `tests/integration/`, `tests/snapshot/`, `tests/compat/`: Catch2 suites by scope.
- `tools/ci/`: local and CI gates.
- `tools/docs/`: documentation and knowledge-health checks.

## Style Baseline

- C++20, extensions off.
- Headers use `#pragma once`.
- Namespaces root at `urpg`.
- Classes/structs use `PascalCase`; functions use `camelCase`; files use `snake_case`.
- Follow `.editorconfig`: spaces, LF, final newline, no trailing whitespace.

## Before Finishing

Run the smallest verification that covers the changed surface and report the command. If a plan lists a verification command, use that exact command unless it is impossible, and explain any mismatch.
