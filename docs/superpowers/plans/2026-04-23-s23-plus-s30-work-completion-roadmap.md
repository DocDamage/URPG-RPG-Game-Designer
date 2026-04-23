# URPG Program Completion Roadmap (100% Target)

## Purpose

The remaining work is not just closure cleanup; it is product-grade completion work that must prove runtime capability and WYSIWYG usability together.

This roadmap maps all non-trivial remaining gaps into bounded sprints with explicit checkboxes and acceptance criteria so progress is executable and reviewable lane-by-lane.

## Scope and Constraints

- Scope is the entire remaining work shown in `content/readiness/readiness_status.json` and the active canonical planning artifacts.
- A feature is not complete until both these are true:
  - Runtime path is implemented and exercised by tests.
  - Editor + diagnostics + import/export surfaces exist for creator usage.
- Offline ML/research tooling remains in `tools/` and must emit stable artifacts; no new heavy runtime dependency creep.
- Branches should remain PR-friendly with focused ticket-by-ticket progression and minimal blast radius.

## Shared Definition of Done (applies every sprint)

- [ ] Required artifacts exist and are committed.
- [ ] Unit/integration (or compat) coverage proves the new behavior.
- [ ] Gates in `docs/superpowers/plans` for that sprint executed locally.
- [ ] `PROJECT_AUDIT.md` / `content/readiness/readiness_status.json` / canonical matrices are reconciled to actual code evidence.
- [ ] PR-lane evidence command list runs without expected deltas.
- [ ] Residual gaps are documented in `mainGaps` where any partial remains.

## Sprint Strategy

- S23 covers the largest production risk and should be completed before opening S27 or S30 lanes.
- S24 and S25 can run in parallel once S23 has started, but S24 should not close without readiness/diagnostics reconciliation from S25.
- S26 is ongoing maintenance and should run continuously once every sprint starts.
- S28, S29, S30, and S31 are template/feature expansion and should be run as bounded slices with strict release-gate hooks.

## S23 — Presentation Runtime Productization (critical-path)

### Objective
Deliver real presentation runtime ownership beyond mock-backed behavior and register it as real, observable, and reload-capable.

### Tickets
- [ ] S23-T01 Replace mock-backed presentation backend wiring with real runtime integration points.
- [ ] S23-T02 Enable real `ProfileArena` hot-reload across startup and project reload paths.
- [ ] S23-T03 Close schema migration and compatibility mapping for presentation-owned payloads.
- [ ] S23-T04 Expand scene-level command coverage for Map/Battle/Chat/Status UI render commands.
- [ ] S23-T05 Remove or explicitly gate remaining partial claims in docs and readiness records.
- [ ] S23-T06 Add regression tests proving command stream determinism under presentation hot-rereload.

### Exit Criteria
- [ ] A bounded scene can render through the real presentation pipeline without mock seam fallback.
- [ ] Profile arena reload emits stable diagnostics and does not leak resources.
- [ ] `presentation_runtime` `mainGaps` reduced to only explicitly accepted future work, if any.
- [ ] `visual_regression_harness` receives at least one new full-scene golden tied to the live presentation path.

## S24 — Gameplay Ability Integration and Migration Truth

### Objective
Complete the gameplay ability framework as a real part of core gameplay flow, not only diagnostics and structure.

### Tickets
- [ ] S24-T01 Finalize compatibility-to-native schema contract for ability JSON.
- [ ] S24-T02 Implement compat migration mapping for unsupported/partial ability fields with preserved fallback payloads.
- [ ] S24-T03 Integrate ability command queue with BattleScene and MapScene execution path.
- [ ] S24-T04 Implement deterministic state-machine driven execution with real event outcomes.
- [ ] S24-T05 Add diagnostics/telemetry for blocked vs executed ability transitions.
- [ ] S24-T06 Update docs and readiness artifacts to remove inflated support language.

### Exit Criteria
- [ ] `gameplay_ability_framework` no longer relies on passive placeholders for core flow.
- [ ] Ability execution can be authored, tested, and previewed in editor workflows.
- [ ] Regressions lock migration and execution parity for supported ability payloads.

## S25 — Governance Foundation Completion

### Objective
Finish governance enforcement so status, docs, audit, and runtime evidence cannot drift silently.

### Tickets
- [ ] S25-T01 Complete diagnostics export parity across CLI and UI for all governance sections.
- [ ] S25-T02 Enforce release-signoff workflow in PR gates (artifact presence + status-date validation).
- [ ] S25-T03 Close remaining release-readiness/template spec parity checks.
- [ ] S25-T04 Add truth-reconciler guardrails to catch template-subsystem-bar drift automatically.
- [ ] S25-T05 Add regression tests for `check_*_governance` scripts and gate failures.
- [ ] S25-T06 Update `RELEASE_READINESS_MATRIX.md` and `TEMPLATE_READINESS_MATRIX.md` where proofs changed.

### Exit Criteria
- [ ] CI fails when governance artifacts are missing or stale.
- [ ] `governance_foundation` reaches `PARTIAL` with no untracked drift risk.
- [ ] Project audit sections are complete and surfaced in both CLI and diagnostics workspace.

## S26 — Compat Bridge Exit Maintenance (post-Phase-2)

### Objective
Keep compat exit trustworthy through regular expansion and evidence quality.

### Tickets
- [ ] S26-T01 Add/update curated corpus maintenance job in the existing weekly lane.
- [ ] S26-T02 Add regression fixtures for dependency-drift and profile mismatch scenarios.
- [ ] S26-T03 Expand failure-report artifacts (`JSONL`, report ingest, panel projection) for new compatibility cases.
- [ ] S26-T04 Add periodic schema/changelog validation in the compat import lane.

### Exit Criteria
- [ ] Weekly compat evidence remains green and interpretable by humans.
- [ ] Curated corpus health checks are deterministic and fail fast.
- [ ] `compat_bridge_exit` remaining gaps are explicitly bounded to depth and curation policy.

## S27 — Accessibility + Audio + Mod + Analytics Productionization

### Objective
Close partial lanes that currently rely on bounded scaffolding or compat-only surfaces.

### Tickets
- [ ] S27-T01 Add accessibility ingest adapters for spatial, audio, and battle editors.
- [ ] S27-T02 Emit accessibility audit payloads with actionable file/line references in those new adapters.
- [ ] S27-T03 Connect audio mix presets to live backend parameters in supported runtime paths.
- [ ] S27-T04 Implement live mod loading path with sandboxing boundaries and deterministic unload behavior.
- [ ] S27-T05 Add mod store ingestion contract + validation and failure reporting.
- [ ] S27-T06 Implement analytics upload + session aggregation + opt-in privacy controls.
- [ ] S27-T07 Add retention/export workflows for analytics auditability.

### Exit Criteria
- [ ] `accessibility_auditor` has non-menu surfaces covered by adapters + diagnostics.
- [ ] `audio_mix_presets` controls affect actual backend behavior.
- [ ] `mod_registry` can load live content with bounded sandbox controls.
- [ ] `analytics_dispatcher` data lifecycle exists beyond local buffering.

## S28 — Visual Regression and Export Hardening

### Objective
Turn current partial visual validation into broad, maintainable release-confidence coverage.

### Tickets
- [ ] S28-T01 Add committed full-frame and scene-specific goldens for remaining scene types.
- [ ] S28-T02 Expand backend coverage (where real render backends are supported in tree).
- [ ] S28-T03 Enforce fail-on-drift policy in relevant CI lanes.
- [ ] S28-T04 Extend harness harnessing to include approved deterministic scene transitions.
- [ ] S28-T05 Harden export package integrity workflow and optional signing validation seam.
- [ ] S28-T06 Add export artifact validation tests for packaging edge-cases and tamper signals.

### Exit Criteria
- [ ] New golden baselines are part of the PR lane.
- [ ] Visual regression failures produce actionable diffs in existing workflow output.
- [ ] Export packaging has deterministic verification against integrity tags and protected payload metadata.

## S29 — Template Closure for READY Candidates

### Objective
Finish core template bars for `jrpg`, `visual_novel`, `turn_based_rpg` so they can be promoted with confidence.

### Tickets
- [ ] S29-T01 Close `accessibility`, `audio`, `input`, `localization`, `performance` bars for `jrpg`.
- [ ] S29-T02 Close the same bars for `visual_novel`.
- [ ] S29-T03 Close the same bars for `turn_based_rpg`.
- [ ] S29-T04 Add template-level WYSIWYG acceptance tests for each lane (edit -> preview -> export).
- [ ] S29-T05 Add blocked-item evidence updates in `content/readiness/readiness_status.json`.

### Exit Criteria
- [ ] Bars for the three templates become `READY` only when authoring workflows are proven.
- [ ] Template artifacts align with `TEMPLATE_READINESS_MATRIX.md`.

## S30 — Template Expansion and Advanced Product Lanes

### Objective
Move experimental templates into concrete ship-ready lanes with clear blockers removed.

### Tickets
- [ ] S30-T01 `monster_collector_rpg`: collection schema, capture mechanics, and party assembly workflow.
- [ ] S30-T02 `cozy_life_rpg`: scheduling, relationships, and economy/recipe loops.
- [ ] S30-T03 `tactics_rpg`: scenario authoring + tactical combat progression.
- [ ] S30-T04 `arpg`: traversal/growth loop and productized movement authority.
- [ ] S30-T05 `metroidvania_lite`: progression gating, map-unlock, and movement affordances.
- [ ] S30-T06 `2_5d_rpg`: raycast map authoring pipeline + production-grade preview.
- [ ] S30-T07 Add one end-to-end release evidence snapshot per template.

### Exit Criteria
- [ ] No template marked `PLANNED` or `EXPERIMENTAL` without bounded closure scope.
- [ ] Each template has a single executable evidence path that includes schema + editor + runtime.

## S31 — Long-Tail WYSIWYG Lanes (close remaining partials)

### Objective
Complete long-tail partial products that block complete productization.

### Tickets
- [ ] S31-T01 Expand Character Identity runtime for full create-a-character workflow.
- [ ] S31-T02 Add appearance preview pipeline for character authoring.
- [ ] S31-T03 Integrate achievement conditions with platform trophy backends.
- [ ] S31-T04 Improve offline tools/runtime artifact trust boundaries for generated/segmented/audio pipelines.
- [ ] S31-T05 Reconcile all remaining `mainGaps` fields for partially landed systems.

### Exit Criteria
- [ ] `character_identity` has end-to-end authoring lifecycle closure.
- [ ] `achievement_registry` includes backend integration where scope is not explicitly out-of-tree.
- [ ] No hidden "future work" remains in `mainGaps` without explicit acceptance notes.

## Suggested Execution Order (3-2-1 cadence)

- [ ] Sprint 23 must start immediately.
- [ ] S24 and S25 can run next in parallel with tight cross-review between runtime and governance artifacts.
- [ ] S26 is continuous and should be maintained each sprint.
- [ ] S27 follows once accessibility/audio/mod/analytics entry contracts are identified in S25.
- [ ] S28 should run once S23 proves the real backend command stream is stable.
- [ ] S29 and S30 should open once S25 proves governance and status artifacts are stable.
- [ ] S31 closes any remaining long-tail blockers after higher-risk product slices are done.

## Immediate Gating Artifacts to Track Across Sprints

- [ ] `content/readiness/readiness_status.json`
- [ ] `docs/PROGRAM_COMPLETION_STATUS.md`
- [ ] `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- [ ] `docs/RELEASE_READINESS_MATRIX.md`
- [ ] `docs/TEMPLATE_READINESS_MATRIX.md`
- [ ] `PROJECT_AUDIT.md`
- [ ] `tools/ci` governance scripts and check logs
- [ ] `tests` PR-lane slices tied to each sprint ticket

## Delivery Cadence

- [ ] One sprint update per bounded code slice in `WORKLOG.md`.
- [ ] One sprint-end checkpoint includes:
  - [ ] build + PR-lane tests
  - [ ] gate scripts
  - [ ] governance and truth reconciler run
  - [ ] artifact/date drift checks
- [ ] No status updates claiming `READY` before all checkboxes in the corresponding sprint and governance checks are complete.

