# Superseded Roadmap Notice

This file is retained only for traceability.

The active roadmap is:
- [2026-04-23-s23-plus-s33-work-completion-roadmap.md](../../superpowers/plans/2026-04-23-s23-plus-s33-work-completion-roadmap.md)

Do not use this `...s30...` copy as the current source of truth for execution, audit coverage, or roadmap updates.

---

# URPG Program Completion Roadmap (Superseded `s30` Copy)

## Purpose

The previous roadmap missed at least three categories that matter for 100% completion:
- `PARTIAL` systems that are blocked by explicit human-review gates.
- Template bar closure work (including WYSIWYG bar-level evidence).
- Long-tail productization and tooling boundary risks that were not represented as first-class slices.

This revised roadmap maps all non-ready work from `content/readiness/readiness_status.json` into executable sprints with explicit checkboxes and exit criteria.

### Validation pass performed

- `content/readiness/readiness_status.json` was re-read and every non-`READY` subsystem/template is represented.
- All listed `mainGaps`/`mainBlockers` have corresponding tickets in this roadmap.

## Scope and Constraints

- Scope is the entire remaining work shown in `content/readiness/readiness_status.json` and the canonical planning docs.
- A feature is complete only when runtime + editor + diagnostics + migration/audit pathways are productized.
- Offline ML/research tooling remains in `tools/` and must emit deterministic exported artifacts; no stealth runtime dependency growth.
- Maintain PR-friendly blast radius and clear ticket-by-ticket traceability.

## Shared Definition of Done (per sprint)

- [ ] Required artifacts exist and are committed.
- [ ] Unit/integration (or compat) coverage proves behavior.
- [ ] Sprint-specific gates execute locally.
- [ ] `docs/PROJECT_AUDIT.md` / `content/readiness/readiness_status.json` / canonical matrices align with code evidence.
- [ ] PR-lane evidence command list runs with expected results.
- [ ] Explicit residuals remain in `mainGaps`/`mainBlockers` where scope is intentionally deferred.
- [ ] Human-review conditions are logged as review artifacts, never implied.

## Sprint Strategy

- S23 is the foundation lane and should start first.
- S24 and S25 can run in parallel once S23 starts.
- S26 is continuous maintenance and must not pause.
- S27, S28, and S29 are high-value execution lanes.
- S30 and S31 are controlled expansion lanes and open after governance (S25) is stable.
- S32 and S33 are mandatory enablers:
  - human-review closure,
  - tooling + evidence-boundary hardening.

## S23 — Presentation Runtime Productization (critical-path)

### Objective
Deliver a real presentation runtime path (not mock-backed) with reload and scene-level reliability.

### Tickets
- [ ] S23-T01 Replace mock-backed presentation backend behavior with real renderer-backed integration in production path.
- [ ] S23-T02 Implement and register real `ProfileArena` hot-reload flow.
- [ ] S23-T03 Close presentation schema/migration mapping gaps with proof fixtures.
- [ ] S23-T04 Expand scene render-command coverage (Map/Battle/Chat/Status/UI).
- [ ] S23-T05 Update readiness/docs wording to remove or gate claims that exceed evidence.
- [ ] S23-T06 Add deterministic tests for command stream replay after reload.

### Exit Criteria
- [ ] Non-mock presentation path renders scene output end-to-end.
- [ ] Hot-reload does not leak profile resources.
- [ ] `presentation_runtime` `mainGaps` contains only intentionally deferred items.
- [ ] `visual_regression_harness` adds at least one full-scene golden tied to live presentation output.
- [ ] `test_spatial_editor.cpp` and `test_presentation_runtime.cpp` include reload + scene command assertions.

## S24 — Gameplay Ability Integration and Migration Truth

### Objective
Complete gameplay ability framework from editor/runtime diagnostics into real command execution.

### Tickets
- [ ] S24-T01 Finalize compat-to-native ability schema mapping.
- [ ] S24-T02 Preserve unsupported fields with deterministic fallback payloads.
- [ ] S24-T03 Integrate ability command queue into BattleScene/MapScene execution.
- [ ] S24-T04 Add deterministic state-machine execution outcomes + diagnostics.
- [ ] S24-T05 Add migration/compatibility tests for supported and fallback branches.
- [ ] S24-T06 Reconcile all public claims in readiness/status docs.

### Exit Criteria
- [ ] Ability runtime is no longer diagnostics-only in game flow paths.
- [ ] Ability queue executes with deterministic authoring→runtime behavior.
- [ ] `gameplay_ability_framework` residuals are intentional and documented.

## S25 — Governance Foundation Completion

### Objective
Prevent evidence/documentation/runtime drift and enforce artifact truth automatically.

### Tickets
- [ ] S25-T01 Finish diagnostics/export parity between CLI and UI for all governance sections.
- [ ] S25-T02 Enforce release-signoff workflow and status-date checks in PR gates.
- [ ] S25-T03 Enforce release-readiness and template-spec parity checks.
- [ ] S25-T04 Add truth-reconciler checks for template-subsystem-bar drift.
- [ ] S25-T05 Add regression tests for governance gate failure modes.
- [ ] S25-T06 Enforce issue-count/date consistency for `docs/PROJECT_AUDIT.md`.
- [ ] S25-T07 Add one positive/negative regression that breaks status-date or artifact-path assertion.

### Exit Criteria
- [ ] `governance_foundation` is no longer blocked by drift risk.
- [ ] CI fail behavior is strict for stale/missing artifacts.
- [ ] Project-audit sections are complete across CLI and diagnostics workspace.
- [ ] Human-review-gated lines are explicitly represented in checks.

## S26 — Compat Bridge Exit Maintenance (post-Phase-2)

### Objective
Keep compat trust current as curated corpus and import/verification lanes evolve.

### Tickets
- [ ] S26-T01 Maintain curated corpus workflow with deterministic health checks.
- [ ] S26-T02 Add fixtures for dependency drift and profile mismatch.
- [ ] S26-T03 Expand failure report outputs (`JSONL`, report ingestion, panel projection).
- [ ] S26-T04 Add periodic schema/changelog validation in compat import lane.

### Exit Criteria
- [ ] Weekly compat evidence remains green and auditable.
- [ ] Corpus health checks fail fast and deterministically.
- [ ] `compat_bridge_exit` future work remains bounded and explicit.

## S27 — Human Review / Promotion Closure

### Objective
Close the explicit human-review lanes safely before any READY promotion.

### Tickets
- [ ] S27-T01 Publish `battle_core` reviewer evidence packet + closure checklist.
- [ ] S27-T02 Publish `save_data_core` reviewer evidence packet + unresolved-gap log.
- [ ] S27-T03 Publish `compat_bridge_exit` review cadence and maintenance evidence.
- [ ] S27-T04 Publish `gameplay_ability_framework` review closure for schema/migration boundaries.
- [ ] S27-T05 Publish `presentation_runtime` reviewer evidence packet, or remove human-review promotion wording if closure no longer depends on review-gated promotion.
- [ ] S27-T06 Wire promotion rule: READY only after corresponding human-review ticket completion.

### Exit Criteria
- [ ] All reviewer checkpoints exist as canonical artifacts.
- [ ] No READY promotion can occur without explicit review completion state.
- [ ] Readiness records and `RELEASE_SIGNOFF_WORKFLOW.md` align on pending review gates.

## S28 — Accessibility, Audio, Mod, and Analytics Productionization

### Objective
Close partial lanes that are currently bounded to scaffolded or compat-only behavior.

### Tickets
- [ ] S28-T01 Add accessibility live-ingestion adapters for spatial/audio/battle editors.
- [ ] S28-T02 Add actionable accessibility diagnostics with file/line references for those adapters.
- [ ] S28-T03 Connect audio mix presets to live runtime backend parameters.
- [ ] S28-T03a Add the remaining `audio_mix_presets` validator-rule coverage (unknown categories, cross-preset duck conflicts) and keep governance fixtures in parity.
- [ ] S28-T04 Implement live mod loading + deterministic unload + sandboxing.
- [ ] S28-T05 Add mod-store contract + validation + failure reporting.
- [ ] S28-T06 Implement analytics upload and session aggregation.
- [ ] S28-T07 Add privacy/consent + retention/export workflows.

### Exit Criteria
- [ ] `accessibility_auditor` covers non-menu surfaces.
- [ ] Audio presets affect runtime playback.
- [ ] Live mod loading works under sandbox and determinism constraints.
- [ ] `analytics_dispatcher` has end-to-end event lifecycle.

## S29 — Visual Regression and Export Hardening

### Objective
Expand visual confidence from partial proof to broad scene/backend release coverage.

### Tickets
- [ ] S29-T01 Add full-frame and scene-specific goldens for remaining scene categories.
- [ ] S29-T02 Expand backend coverage where renderer support exists.
- [ ] S29-T03 Enforce fail-on-drift policy in CI/local lanes.
- [ ] S29-T04 Add deterministic transition goldens for approved scenes.
- [ ] S29-T05 Extend integrity checks for signing/enforcement seam in export validation.
- [ ] S29-T06 Add export edge-case and tamper test coverage.

### Exit Criteria
- [ ] Visual regression baselines are PR-lane enforced.
- [ ] Diff artifacts are actionable and stable.
- [ ] Export package checks include integrity and repeatability.

## S30 — Template Closure for READY Candidates

### Objective
Close bars and WYSIWYG workflows for `jrpg`, `visual_novel`, `turn_based_rpg`.

### Tickets
- [ ] S30-T01 Close `accessibility`, `audio`, `input`, `localization`, `performance` bars for `jrpg`.
- [ ] S30-T02 Close the same bars for `visual_novel`.
- [ ] S30-T03 Close the same bars for `turn_based_rpg`.
- [ ] S30-T04 Add template-level acceptance tests: edit -> preview -> export.
- [ ] S30-T05 Add canonical template spec artifacts for `jrpg`, `visual_novel`, and `turn_based_rpg`, and reconcile them with readiness/matrix rows.
- [ ] S30-T06 Extend `urpg_project_audit` template-spec and blocker detection coverage so these three templates can fail closed on missing prerequisites.
- [ ] S30-T07 Update `readiness_status.json` blocker evidence for these closures.

### Exit Criteria
- [ ] These three templates can be promoted with confidence.
- [ ] Evidence matches `TEMPLATE_READINESS_MATRIX.md`.
- [ ] Canonical template-facing docs and `docs/PROJECT_AUDIT.md` stay aligned with the promoted scope.

## S30-Bis — Template Bar Quality Sweep

### Objective
Add missing bar-level depth for localized/template readiness where it can block production claims.

### Tickets
- [ ] S30B-T01 Add explicit localization completeness proof per template.
- [ ] S30B-T02 Add accessibility/input parity for remaining required bars.
- [ ] S30B-T03 Add template-specific performance budget diagnostics.
- [ ] S30B-T04 Add artifact-level WYSIWYG proof links per bar.

### Exit Criteria
- [ ] Required bars do not remain open due only to missing evidence.
- [ ] `mainBlockers` are reduced to roadmap-level, date-scoped constraints.

## S31 — Template Expansion and Advanced Product Lanes

### Objective
Progress `PLANNED` and `EXPERIMENTAL` templates to bounded ship-ready slices.

### Tickets
- [ ] S31-T01 `monster_collector_rpg`: collection schema, capture mechanics, and party workflow.
- [ ] S31-T02 `cozy_life_rpg`: scheduling/social/economy lane proof.
- [ ] S31-T03 `tactics_rpg`: scenario authoring and progression framework.
- [ ] S31-T04 `arpg`: movement and growth loop productization.
- [ ] S31-T05 `metroidvania_lite`: traversal/map-unlock/progression mechanics.
- [ ] S31-T06 `2_5d_rpg`: raycast authoring and preview proof.
- [ ] S31-T06a `2_5d_rpg`: add template-specific export validation and raycast art-pipeline closure work.
- [ ] S31-T07 Add template-grade governance/spec/audit closure for `monster_collector_rpg` and closure-visibility artifacts for `arpg`.
- [ ] S31-T08 One end-to-end evidence snapshot per template.

### Exit Criteria
- [ ] No template remains `PLANNED` or `EXPERIMENTAL` purely by omission.
- [ ] Each template has executable, authorable, testable evidence.
- [ ] Each template's canonical spec, readiness record, matrix row, and project-audit contract agree.

## S32 — Long-Tail WYSIWYG Lanes

### Objective
Finish remaining partial feature blocks that directly affect full-product claims.

### Tickets
- [ ] S32-T01 Character Identity: full Create-a-Character creation lifecycle.
- [ ] S32-T02 Character appearance/preview pipeline.
- [ ] S32-T03 Achievement registry platform backend integration where scope is not intentionally out-of-tree.
- [ ] S32-T04 Reconcile outstanding `mainGaps` entries for partial systems.
- [ ] S32-T05 Confirm `export_validator` cryptographic hardening roadmap in scope before READY claims.
- [ ] S32-T06 Complete `achievement_registry` trophy export pipeline and keep platform/backend scope wording explicit where integration stays out of tree.

### Exit Criteria
- [ ] No unresolved short-cycle blockers remain in partial core systems without explicit traceability.
- [ ] `character_identity` and `achievement_registry` have end-to-end usability proof where promised.

## S33 — Offline Tooling and Artifact Boundary Hardening

### Objective
Protect runtime cleanliness while completing first approved tooling lanes.

### Tickets
- [ ] S33-T01 Complete FAISS retrieval acceptance for deterministic indexing and ingestion.
- [ ] S33-T02 Complete SAM/SAM2 segmentation acceptance with manifest-backed outputs.
- [ ] S33-T03 Complete Demucs/Encodec preprocessing acceptance with deterministic outputs.
- [ ] S33-T04 Add runtime contracts to consume tool artifacts only.
- [ ] S33-T05 Add provenance/reproducibility checks for tool outputs.
- [ ] S33-T06 Add guardrails that block toolchain/runtime boundary regression.

### Exit Criteria
- [ ] Tooling remains outside shipped runtime dependency graph.
- [ ] Runtime uses only reproducible exported artifacts.
- [ ] Artifact lineage and checksums are auditable.

## Suggested Execution Order

- [ ] S23 starts now.
- [ ] S24 and S25 run next in parallel.
- [ ] S26 runs continuously.
- [ ] S27 starts as soon as S25 begins (must complete before any READY promotion).
- [ ] S28 and S29 run after S23 proof and S25 gate stability.
- [ ] S30 and S31 run after S25 + S30-Bis gating.
- [ ] S32 closes residual WYSIWYG blockers before S33 closes the tooling boundary.
- [ ] S33 can run in parallel with later lanes once S23 proves runtime stability.
- [ ] S29 and S30-Bis should stay coupled so coverage and bar evidence grow together.

## Immediate Gating Artifacts

- [ ] `content/readiness/readiness_status.json`
- [ ] `docs/PROGRAM_COMPLETION_STATUS.md`
- [ ] `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- [ ] `docs/RELEASE_READINESS_MATRIX.md`
- [ ] `docs/TEMPLATE_READINESS_MATRIX.md`
- [ ] `docs/PROJECT_AUDIT.md`
- [ ] `docs/TRUTH_ALIGNMENT_RULES.md`
- [ ] `docs/SUBSYSTEM_STATUS_RULES.md`
- [ ] `docs/TEMPLATE_LABEL_RULES.md`
- [ ] `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- [ ] `tools/ci` governance scripts/log outputs
- [ ] `tests` PR-lane slices mapped by sprint
- [ ] Offline tooling manifests in `tools/retrieval` and related pipelines
- [ ] `docs/RELEASE_READINESS_MATRIX.md` and `docs/TEMPLATE_READINESS_MATRIX.md` updates after each closure

## Execution Anchors For Newly Added Tickets

This section exists so a low-context implementation model can execute the new tickets without guessing file names, artifact names, or the minimum verification surface.

### Shared conventions

- When a ticket changes canonical readiness or promotion wording, update all of:
  - `content/readiness/readiness_status.json`
  - `docs/RELEASE_READINESS_MATRIX.md`
  - `docs/TEMPLATE_READINESS_MATRIX.md` when template status/bars/blockers change
  - `docs/PROGRAM_COMPLETION_STATUS.md`
  - `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
  - `docs/PROJECT_AUDIT.md` when the shipped audit contract changes
- Template spec naming convention is `docs/templates/<template_id>_spec.md`.
- Template spec header must follow the existing canonical pattern:
  - title line
  - `Status Date: YYYY-MM-DD`
  - `Authority: canonical template spec for \`<template_id>\``
  - sections for `Purpose`, `Required Subsystems`, `Cross-Cutting Minimum Bars`, `Safe Scope Today`, `Main Blockers`, `Promotion Path`
- Signoff artifact naming convention in the current repo is `docs/<SUBSYSTEM>_CLOSURE_SIGNOFF.md` or `docs/<SUBSYSTEM>_SIGNOFF.md`. Follow the existing style instead of inventing a new format.
- Any ticket that retains "promotion requires human review" wording must leave the state explicitly non-promoting. Never imply that passing tests or checks equals approval.

### S27-T05 — `presentation_runtime` human-review closure

Preferred implementation path:
- Add `docs/PRESENTATION_RUNTIME_CLOSURE_SIGNOFF.md`.
- If the lane should remain human-review-gated, also add a structured `signoff` block for `presentation_runtime` in `content/readiness/readiness_status.json` matching the current governed format used by `battle_core`, `save_data_core`, `compat_bridge_exit`, and `gameplay_ability_framework`.
- If the lane should no longer be review-gated, remove the human-review promotion wording from:
  - `content/readiness/readiness_status.json`
  - `docs/RELEASE_READINESS_MATRIX.md`
  - any matching summary wording in `docs/PROGRAM_COMPLETION_STATUS.md`

Primary file anchors:
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

Presentation evidence anchors:
- `engine/core/presentation/presentation_runtime.cpp`
- `engine/core/presentation/presentation_bridge.cpp`
- `engine/core/presentation/profile_arena.cpp`
- `engine/core/presentation/release_validation.cpp`
- `tests/unit/test_presentation_runtime.cpp`
- `tests/unit/test_spatial_editor.cpp`
- `docs/presentation/VALIDATION.md`

Minimum verification:
- `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1`
- `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1`
- `powershell -ExecutionPolicy Bypass -File tools/ci/run_presentation_gate.ps1`

### S28-T03a — Remaining `audio_mix_presets` validator-rule coverage

Goal:
- Extend the validator and fixture surface so the remaining backlog called out by `docs/RELEASE_READINESS_MATRIX.md` is executable instead of prose-only.

Primary file anchors:
- `engine/core/audio/audio_mix_validator.h`
- `engine/core/audio/audio_mix_validator.cpp`
- `engine/core/audio/audio_mix_presets.h`
- `engine/core/audio/audio_mix_presets.cpp`
- `content/schemas/audio_mix_presets.schema.json`
- `content/fixtures/audio_mix_presets_fixture.json`
- `tests/unit/test_audio_mix_presets.cpp`
- `tools/ci/check_audio_governance.ps1`
- `tools/audit/urpg_project_audit.cpp`
- `docs/RELEASE_READINESS_MATRIX.md`

Concrete checks to add:
- unknown category detection
- cross-preset duck-rule conflict detection
- fixture coverage for both negative cases
- audit/governance parity if new validator output changes the shipped contract

Minimum verification:
- `powershell -ExecutionPolicy Bypass -File tools/ci/check_audio_governance.ps1`
- build + run focused audio preset tests through `urpg_tests`

### S30-T05 — Canonical specs for `jrpg`, `visual_novel`, `turn_based_rpg`

Create:
- `docs/templates/jrpg_spec.md`
- `docs/templates/visual_novel_spec.md`
- `docs/templates/turn_based_rpg_spec.md`

Use these existing files as format references:
- `docs/templates/monster_collector_rpg_spec.md`
- `docs/templates/2_5d_rpg_spec.md`

Each new spec must match the current readiness data:
- required subsystems from `content/readiness/readiness_status.json`
- current bar statuses from `content/readiness/readiness_status.json`
- safe-scope wording must stay conservative
- blockers must not exceed or understate canonical readiness blockers

Also update:
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json` if the plan intentionally changes blockers/bars
- `docs/PROGRAM_COMPLETION_STATUS.md` if product-facing template wording changes

### S30-T06 — `urpg_project_audit` template-spec coverage for the three READY candidates

Goal:
- Make the audit fail closed for `jrpg`, `visual_novel`, and `turn_based_rpg` the same way it already reasons about spec artifacts for the existing expansion-template specs.

Primary file anchors:
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `editor/diagnostics/project_audit_panel.cpp`
- `docs/PROJECT_AUDIT.md`
- `tools/ci/check_release_readiness.ps1`

Expected behavior:
- missing canonical spec file should report a concrete issue
- wrong `Authority:` line should report a concrete issue
- required-subsystem drift from readiness should report a concrete issue
- cross-cutting bar drift from readiness should report a concrete issue

Minimum verification:
- build `urpg_project_audit`
- run `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1`
- run `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1`
- run focused project-audit tests through `urpg_tests`

### S31-T06a — `2_5d_rpg` export-validation and art-pipeline closure

This ticket is not only about rendering proof. It must close the exact blockers already named in canonical readiness:
- raycast art pipeline
- map authoring adapters at production grade
- template-specific export validation

Primary file anchors:
- `docs/templates/2_5d_rpg_spec.md`
- `engine/core/render/raycast_renderer.h`
- `engine/core/export/export_validator.h`
- `engine/core/export/export_validator.cpp`
- `editor/spatial/spatial_authoring_workspace.cpp`
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `tests/unit/test_export_validator.cpp`

Minimum expected outputs:
- one canonical doc update that names the accepted 2.5D art-pipeline contract
- one export-validator rule or fixture that is specific to 2.5D template requirements
- one test that fails when the 2.5D-specific export requirement is missing

### S31-T07 — `monster_collector_rpg` governance closure and `arpg` closure visibility

This is two separate deliverables and should not be collapsed into one doc-only change.

For `monster_collector_rpg`:
- update `docs/templates/monster_collector_rpg_spec.md`
- update `tools/audit/urpg_project_audit.cpp` if new template artifacts or blockers become machine-checkable
- reconcile `docs/TEMPLATE_READINESS_MATRIX.md` and `content/readiness/readiness_status.json`

For `arpg`:
- create `docs/templates/arpg_spec.md` if the lane is being given first-class closure visibility
- ensure its blocker wording matches the current `arpg` readiness record before broadening claims

Minimum verification:
- focused project-audit tests
- `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1`
- `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1`

### S32-T06 — `achievement_registry` trophy export pipeline

Scope clarification:
- This ticket is for exportable trophy/achievement artifacts and deterministic pipeline wiring.
- It is not permission to overclaim live platform backend integration if that remains out of tree.

Primary file anchors:
- `engine/core/achievement/achievement_registry.h`
- `engine/core/achievement/achievement_registry.cpp`
- `engine/core/achievement/achievement_validator.h`
- `engine/core/achievement/achievement_validator.cpp`
- `editor/achievement/achievement_panel.cpp`
- `content/schemas/achievements.schema.json`
- `content/fixtures/achievement_registry_fixture.json`
- `tests/unit/test_achievement_registry.cpp`
- `tests/unit/test_achievement_panel.cpp`
- `tests/unit/test_achievement_triggers.cpp`
- `tools/ci/check_achievement_governance.ps1`
- `tools/audit/urpg_project_audit.cpp`

Expected closure shape:
- deterministic exported trophy/achievement artifact or manifest format
- validator coverage for that artifact shape
- fixture coverage for valid + invalid export cases
- explicit wording in canonical docs that distinguishes in-tree export support from out-of-tree platform backend integration

Minimum verification:
- `powershell -ExecutionPolicy Bypass -File tools/ci/check_achievement_governance.ps1`
- build + run focused achievement tests through `urpg_tests`

### Low-context model guardrails

- Do not invent new status labels; use only `PLANNED`, `EXPERIMENTAL`, `PARTIAL`, `READY`, `BLOCKED`.
- Do not promote any subsystem or template by editing docs alone; evidence, tests, and audit/gate behavior must move with the doc change.
- Do not add parallel planning authority. Update the canonical stack listed in `docs/TRUTH_ALIGNMENT_RULES.md`.
- When touching `content/readiness/readiness_status.json`, keep `statusDate` synchronized with every canonical doc updated in the same slice.
- Prefer adding one focused negative test per new governance rule so drift fails loudly instead of relying on prose.

## Remaining Work Mapping (non-ready coverage)

1. Non-ready subsystems mapped:
   `battle_core`, `save_data_core`, `compat_bridge_exit`, `presentation_runtime`, `gameplay_ability_framework`, `governance_foundation`, `character_identity`, `achievement_registry`, `accessibility_auditor`, `visual_regression_harness`, `audio_mix_presets`, `export_validator`, `mod_registry`, `analytics_dispatcher`.

2. Non-ready templates mapped:
   `jrpg`, `visual_novel`, `turn_based_rpg`, `tactics_rpg`, `arpg`, `monster_collector_rpg`, `cozy_life_rpg`, `metroidvania_lite`, `2_5d_rpg`.

3. No mapped non-ready item is unassigned.

## Delivery Cadence

- [ ] One sprint update per bounded code slice in `WORKLOG.md`.
- [ ] One sprint-end checkpoint includes:
  - [ ] build + PR-lane tests
  - [ ] gate scripts
  - [ ] governance and truth reconciler run
  - [ ] artifact/date drift checks
  - [ ] explicit readiness record updates

## Known Risks

- [ ] Signing/enforcement in export may expand scope if claimed as fully shipped prematurely.
- [ ] Template bar closure can regress after runtime-only fixes.
- [ ] Human-review workflows introduce controlled waiting states that must be planned in schedule.
- [ ] Tooling pipeline changes may destabilize reproducibility if outputs are not pinned.

## Exhaustiveness and miss-check (current state)

I ran a deterministic readiness cross-check using the latest `content/readiness/readiness_status.json`:

- [ ] Re-run readiness parse and confirm `status != READY` bucket parity.
- [ ] Re-run roadmap presence checks and confirm zero unmapped IDs.
- [ ] Confirm no README/ADR/project status file claims READY on unresolved items above.

Coverage baseline captured this pass:
- Non-ready subsystems: 14
- Non-ready templates: 9
- Missing subsystem IDs in this plan: 0
- Missing template IDs in this plan: 0

Non-ready subsystems:
- `battle_core`
- `save_data_core`
- `compat_bridge_exit`
- `presentation_runtime`
- `gameplay_ability_framework`
- `governance_foundation`
- `character_identity`
- `achievement_registry`
- `accessibility_auditor`
- `visual_regression_harness`
- `audio_mix_presets`
- `export_validator`
- `mod_registry`
- `analytics_dispatcher`

Non-ready templates:
- `jrpg`
- `visual_novel`
- `turn_based_rpg`
- `tactics_rpg`
- `arpg`
- `monster_collector_rpg`
- `cozy_life_rpg`
- `metroidvania_lite`
- `2_5d_rpg`

## Explicitly known partial boundaries (not missed features)

- `tools/retrieval/shared/retrieval_index.py` — abstract adapter contracts intentionally left unbound for optional retrieval backends.
- `tools/retrieval/embedding_jobs/external_embedding_adapter.py` — optional backend adapters are explicit extension points, not hidden required defaults.
- `runtimes/compat_js/window_compat.h` / `window_compat.cpp` — compatibility-mode rendering edge cases intentionally deferred in boundary tickets.
- `runtimes/compat_js/battle_manager.h` — staged compatibility behavior already represented in `compat_bridge_exit` closure work.

## Per-sprint execution contract

- [ ] Each sprint has a single canonical acceptance command set and one evidence index entry.
- [ ] Each sprint writes or updates one authoritative readiness-related artifact.
- [ ] Any partial closure keeps a single explicit `mainGaps` reason and rollback owner.
