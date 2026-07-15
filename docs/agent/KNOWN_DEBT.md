# Known Debt Map

This file is a pointer map, not the debt database.

## Canonical Sources

- Program status snapshot: `docs/PROGRAM_COMPLETION_STATUS.md`
- Known debt index: `docs/PROGRAM_COMPLETION_STATUS.md`
- Release execution plan: `docs/release/AAA_RELEASE_EXECUTION_PLAN.md`
- Release readiness matrix: `docs/release/RELEASE_READINESS_MATRIX.md`
- App release readiness matrix: `docs/APP_RELEASE_READINESS_MATRIX.md`

## Current Agent Priorities

- Prefer the active creator-product cohesion plan over broad subsystem expansion: `docs/superpowers/plans/2026-07-12-creator-product-cohesion-plan.md`.
- Treat M0-M5 as a bounded implemented foundation, not a completed product. Remaining M1/M3/M4 breadth is explicit: aggregate Perspective 2D-only changes and register durable non-Map authoring surfaces with the shared dirty-state owner, add contextual asset-drop consumers beyond Map Tiles/Props with real undo history, complete shortcut coverage, and record graphical route/layout equivalence.
- M6 now has an initial native session foundation: an editor-owned runtime child, private current-map overlay, authored player-start or selected-part target (with blank-starter fallback), explicit map/spawn/session manifest, visible target/elapsed/exit/overlay state, F5 stop/restart, document/catalog launch blocking, JSONL diagnostics ingestion, and return without discarding editor state. It remains incomplete: broader runtime blocker policy, map-entrance/checkpoint targets, diagnostic focus, and safe hot reload are still open.
- M7 now has checksummed, ignored snapshot primitives and orderly-session markers. It remains incomplete: periodic capture of in-memory dirty drafts, restore/review UI, bounded recovery policy wiring, and explicit governed-asset relinking.
- M8 must integrate event/dialogue/character/database/quest/battle/audio authoring from Map context and satisfy the WYSIWYG done rule. The Capybara repository is `reference_only`; any adopted concept needs URPG-native ownership, provenance/license evidence, deterministic tests, and no parallel browser/npm/cloud/save/pathfinding stack.
- M9 has a governed `Lantern of the Willow` contract and strict report gate, but no creator-authored passed report. M9-M10 must still prove the slice and then complete manual graphical, accessibility, input, and reference-machine performance review. Headless/model coverage is not manual signoff.
- M11 requires fresh clean-clone/local gates, exact project-selected asset hydration, external platform/toolchain evidence, and a new release-owner decision. The current branch is post-`v0.1.0` and is not a broader all-features or follow-up public release claim.
- PFU-I1 has a separate target qualification contract, strict report checker, and `tools/ci/check_pfu_i1_qualification.ps1` wrapper. It remains unsatisfied until every target step has clean, commit-matched Debug/Release evidence; the historical creator-journey report remains mixed passed/partial/deferred.
- The older `std::system`, compat audio lifetime, chatbot callback, QuickJS interruption, tracked generated-root, archive containment, and bounded LFS-reconciliation lanes are closed for their documented scope. Keep their guards passing; do not list them as current implementation blockers unless a regression is observed.
- Treat Phase 6 release-surface remediation as historical release evidence for the bounded `v0.1.0` lineage, not as proof that the current `development` branch is broadly clean. Keep `.\tools\ci\run_local_gates.ps1` passing before widening release claims.
- Treat `level_builder` as the native shippable grid-part map editor. Keep grid-part document editing, save/load/export, playtest, package readiness, diagnostics, and Perspective 2D handoff coherent.
- The release owner promoted `spatial_authoring` to a first-class Perspective 2D map editor surface on 2026-05-26. Both release IDs now deep-link into one creator-facing Map workspace; keep registry exposure, shared context, and explicit underlying document ownership aligned.
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
