# Known Debt Map

This file is a pointer map, not the debt database.

## Canonical Sources

- Program status snapshot: `docs/PROGRAM_COMPLETION_STATUS.md`
- Known debt index: `docs/PROGRAM_COMPLETION_STATUS.md`
- Release execution plan: `docs/release/AAA_RELEASE_EXECUTION_PLAN.md`
- Release readiness matrix: `docs/release/RELEASE_READINESS_MATRIX.md`
- App release readiness matrix: `docs/APP_RELEASE_READINESS_MATRIX.md`

## Current Agent Priorities

- Prefer closing release-plan tasks over broad refactors.
- Enforce the WYSIWYG done rule: a subsystem is not done without visual authoring, live preview, saved project data, runtime execution, diagnostics, and tests.
- Keep bootstrap/dev surfaces visibly marked as non-production.
- Keep any future compat and migration limitations diagnostic-rich and documented; current public compat manager registries are closed for the claimed bridge scope.
- Keep editor release navigation aligned with `docs/release/EDITOR_CONTROL_INVENTORY.md`.
- Convert repeated review feedback into scripts or docs under `tools/` and `docs/agent/`.

## What Not To Do

- Do not promote `PARTIAL` lanes to ready/full based only on fixture coverage.
- Do not mark a system done when it only has deterministic contracts, schemas, or headless tests; it also needs the WYSIWYG completion evidence in `content/readiness/wysiwyg_done_rule.json`.
- Do not introduce hidden fallback behavior for missing assets, scripts, saves, or runtime binaries.
- Do not add new large agent instructions to `AGENTS.md`; link from `docs/agent/INDEX.md` instead.
