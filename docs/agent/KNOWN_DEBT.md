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
- Treat Phase 6 release-surface remediation as verified on the current branch: keep `.\tools\ci\run_local_gates.ps1` passing before widening release claims.
- Treat `level_builder` as the native shippable map editor. Keep grid-part document editing, save/load/export, playtest, package readiness, diagnostics, and supporting spatial handoff coherent before adding parallel map-authoring surfaces.
- Keep `spatial_authoring` nested/supporting unless a release owner explicitly changes the product hierarchy.
- Enforce the WYSIWYG done rule: a subsystem is not done without visual authoring, live preview, saved project data, runtime execution, diagnostics, and tests.
- Keep the AI assistant workflow visibly review-gated: approve/reject/approve-all/apply/revert controls, result diffs, and reverse patches must stay exposed in editor snapshots when AI tooling changes.
- Treat live chatbot provider integration as future work unless a concrete provider service lands behind `IChatService`; creator-command transport profiles are not the same as shipped live chatbot providers.
- Keep template expansion tied to starter manifests, runtime profiles, certification loops, readiness rows, and specs before claiming a template lane is product-ready.
- Keep bootstrap/dev surfaces visibly marked as non-production.
- Keep any future compat and migration limitations diagnostic-rich and documented; current public compat manager registries are closed for the claimed bridge scope.
- Keep editor release navigation aligned with `docs/release/EDITOR_CONTROL_INVENTORY.md`.
- Keep release-required asset checks, install smoke, package smoke, and native version metadata aligned with `README.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`, and `docs/release/RELEASE_PACKAGING.md`.
- Treat release authoring persistence as a guarded surface: ability draft IO must
  keep path-specific `last_io` diagnostics, failed loads must preserve the last
  valid draft, and Level Builder load/export safeguards must keep rejecting
  unsafe documents without replacing the bound document.
- Convert repeated review feedback into scripts or docs under `tools/` and `docs/agent/`.

## What Not To Do

- Do not promote `PARTIAL` lanes to ready/full based only on fixture coverage.
- Do not re-promote Spatial Authoring as the primary map editor while Level Builder owns grid-part map authoring.
- Do not mark a system done when it only has deterministic contracts, schemas, or headless tests; it also needs the WYSIWYG completion evidence in `content/readiness/wysiwyg_done_rule.json`.
- Do not introduce hidden fallback behavior for missing assets, scripts, saves, or runtime binaries.
- Do not add new large agent instructions to `AGENTS.md`; link from `docs/agent/INDEX.md` instead.
