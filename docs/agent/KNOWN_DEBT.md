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
- Treat the current branch as post-`v0.1.0` and not ready for a broader all-features or follow-up public release claim until fresh final gates pass and the current blocker list is closed.
- Highest-priority blocker classes are: production-adjacent `std::system` command execution and argv credential exposure; compat audio raw-pointer lifetime; chatbot async callback lifetime/cancellation; QuickJS CPU interruption; Windows-only native source picking; tracked generated/local directories; and root asset-folder drift. Release bundle/script protection is closed for the in-tree authenticated bundle plus deterministic transform scope; cloud sync, live services, and store publishing are future-update scope rather than blockers for this pass. Current LFS footprint reconciliation is closed for the bounded release scope by `tools/ci/check_lfs_release_scope.ps1` and `docs/asset_intake/LFS_SCOPE_RECONCILIATION.md`, but the branch remains non-zero-LFS.
- Treat Phase 6 release-surface remediation as historical release evidence for the bounded `v0.1.0` lineage, not as proof that the current `development` branch is broadly clean. Keep `.\tools\ci\run_local_gates.ps1` passing before widening release claims.
- Treat `level_builder` as the native shippable grid-part map editor. Keep grid-part document editing, save/load/export, playtest, package readiness, diagnostics, and Perspective 2D handoff coherent.
- The release owner promoted `spatial_authoring` to a first-class Perspective 2D map editor surface on 2026-05-26. Keep its top-level registry/app-shell exposure aligned with Level Builder and the spatial child tools.
- Enforce the WYSIWYG done rule: a subsystem is not done without visual authoring, live preview, saved project data, runtime execution, diagnostics, and tests.
- Keep the AI assistant workflow visibly review-gated: approve/reject/approve-all/apply/revert controls, result diffs, and reverse patches must stay exposed in editor snapshots when AI tooling changes.
- Treat live chatbot provider integration as future work unless a concrete provider service lands behind `IChatService`; creator-command transport profiles are not the same as shipped live chatbot providers.
- Keep template expansion tied to starter manifests, runtime profiles, certification loops, readiness rows, and specs before claiming a template lane is product-ready.
- Keep bootstrap/dev surfaces visibly marked as non-production.
- Keep any future compat and migration limitations diagnostic-rich and documented; current public compat manager registries are closed for the claimed bridge scope.
- Keep editor release navigation aligned with `docs/release/EDITOR_CONTROL_INVENTORY.md`.
- Keep release-required asset checks, install smoke, package smoke, and native version metadata aligned with `README.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`, and `docs/release/RELEASE_PACKAGING.md`.
- Keep LFS wording precise: this checkout currently has 307,388 LFS-tracked paths reconciled as governed promoted-library payloads or governance evidence, and `tools/ci/check_promoted_asset_library.ps1` validates promoted bundle rows without removing GitHub payloads. Release-required app assets are verified by the release asset gate; broader shipped-game claims still need exact project selection and hydration evidence.
- Treat release authoring persistence as a guarded surface: ability draft IO must
  keep path-specific `last_io` diagnostics, failed loads must preserve the last
  valid draft, and Level Builder load/export safeguards must keep rejecting
  unsafe documents without replacing the bound document.
- Convert repeated review feedback into scripts or docs under `tools/` and `docs/agent/`.

## What Not To Do

- Do not promote `PARTIAL` lanes to ready/full based only on fixture coverage.
- Do not demote Perspective 2D / `spatial_authoring` back to nested tooling without an explicit release-owner reversal.
- Do not mark a system done when it only has deterministic contracts, schemas, or headless tests; it also needs the WYSIWYG completion evidence in `content/readiness/wysiwyg_done_rule.json`.
- Do not call the repository LFS-free, generated-artifact-free, shell-execution-free, production cloud-sync-ready, live-service-ready, storefront-ready, or proprietary RPG Maker MZ runtime-parity-ready unless fresh commands and owner evidence prove those exact claims.
- Do not introduce hidden fallback behavior for missing assets, scripts, saves, or runtime binaries.
- Do not add new large agent instructions to `AGENTS.md`; link from `docs/agent/INDEX.md` instead.
