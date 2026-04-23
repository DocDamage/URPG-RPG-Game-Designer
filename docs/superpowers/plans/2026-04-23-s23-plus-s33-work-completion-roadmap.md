# URPG Program Completion Roadmap (100% Target)

## Purpose

This roadmap targets full program completion across all non-ready subsystems and templates tracked in `content/readiness/readiness_status.json`. It corrects three gaps present in the previous plan:

- `PARTIAL` subsystems blocked by explicit human-review gates were not represented as actionable work.
- Template bar closure (including WYSIWYG bar-level evidence) was missing from the execution surface.
- Long-tail productization and tooling boundary risks were not first-class slices.

Every non-`READY` subsystem and template from `content/readiness/readiness_status.json` is mapped to an executable sprint with checkboxes and exit criteria. All `mainGaps`/`mainBlockers` entries have corresponding tickets.

## Canonical References

Use these as the linked source-of-truth stack for status, governance, and promotion work:

- Readiness record: [`content/readiness/readiness_status.json`](../../../content/readiness/readiness_status.json)
- Program status: [`docs/PROGRAM_COMPLETION_STATUS.md`](../../PROGRAM_COMPLETION_STATUS.md)
- Technical debt / remediation hub: [`docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`](../../TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- Release readiness matrix: [`docs/RELEASE_READINESS_MATRIX.md`](../../RELEASE_READINESS_MATRIX.md)
- Template readiness matrix: [`docs/TEMPLATE_READINESS_MATRIX.md`](../../TEMPLATE_READINESS_MATRIX.md)
- Project audit contract: [`docs/PROJECT_AUDIT.md`](../../PROJECT_AUDIT.md)
- Truth alignment rules: [`docs/TRUTH_ALIGNMENT_RULES.md`](../../TRUTH_ALIGNMENT_RULES.md)
- Subsystem status rules: [`docs/SUBSYSTEM_STATUS_RULES.md`](../../SUBSYSTEM_STATUS_RULES.md)
- Template label rules: [`docs/TEMPLATE_LABEL_RULES.md`](../../TEMPLATE_LABEL_RULES.md)
- Release-signoff workflow: [`docs/RELEASE_SIGNOFF_WORKFLOW.md`](../../RELEASE_SIGNOFF_WORKFLOW.md)

Current review/signoff artifacts:

- [`docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`](../../BATTLE_CORE_CLOSURE_SIGNOFF.md)
- [`docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`](../../SAVE_DATA_CORE_CLOSURE_SIGNOFF.md)
- [`docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md`](../../COMPAT_BRIDGE_EXIT_SIGNOFF.md)
- [`docs/GAF_CLOSURE_SIGNOFF.md`](../../GAF_CLOSURE_SIGNOFF.md)

Current template-spec artifacts already in-tree:

- [`docs/templates/monster_collector_rpg_spec.md`](../../templates/monster_collector_rpg_spec.md)
- [`docs/templates/cozy_life_rpg_spec.md`](../../templates/cozy_life_rpg_spec.md)
- [`docs/templates/metroidvania_lite_spec.md`](../../templates/metroidvania_lite_spec.md)
- [`docs/templates/2_5d_rpg_spec.md`](../../templates/2_5d_rpg_spec.md)

Supporting validation docs:

- Presentation validation: [`docs/presentation/VALIDATION.md`](../../presentation/VALIDATION.md)
- Sprint audit checklist: [`docs/superpowers/plans/sprint-audit-checklist.md`](./sprint-audit-checklist.md)

## Scope and Constraints

- Scope covers all remaining work shown in `content/readiness/readiness_status.json` and the canonical planning docs.
- A feature is complete only when runtime, editor, diagnostics, and migration/audit pathways are productized.
- Offline ML/research tooling remains in `tools/` and must emit deterministic exported artifacts; no stealth runtime dependency growth is permitted.
- Maintain PR-friendly blast radius and clear ticket-by-ticket traceability.

## Shared Definition of Done (per sprint)

- [ ] Required artifacts exist and are committed.
- [ ] Unit/integration (or compat) coverage proves behavior.
- [ ] Sprint-specific gates execute locally.
- [ ] [`docs/PROJECT_AUDIT.md`](../../PROJECT_AUDIT.md), [`content/readiness/readiness_status.json`](../../../content/readiness/readiness_status.json), and canonical matrices align with code evidence.
- [ ] PR-lane evidence command list runs with expected results.
- [ ] Explicit residuals remain in `mainGaps`/`mainBlockers` where scope is intentionally deferred.
- [ ] Human-review conditions are logged as review artifacts, never implied.

## Sprint Overview

| Sprint | Lane | Depends On | Parallel With |
|--------|------|-----------|---------------|
| S23 | Presentation Runtime Productization (critical path) | — | S24, S25 |
| S24 | Gameplay Ability Integration and Migration Truth | S23 start | S25 |
| S25 | Governance Foundation Completion | S23 start | S24 |
| S26 | Compat Bridge Exit Maintenance (continuous) | — | all |
| S27 | Human Review / Promotion Closure | S25 start | S28, S29 |
| S28 | Accessibility, Audio, Mod, and Analytics Productionization | S23 proof, S25 stable | S29 |
| S29 | Visual Regression and Export Hardening | S23 proof, S25 stable | S28, S30B |
| S30 | Template Closure for READY Candidates | S25 + S30B | S31 |
| S30B | Template Bar Quality Sweep | S25 stable | S29 |
| S31 | Template Expansion and Advanced Product Lanes | S25 + S30B | S32 |
| S32 | Long-Tail WYSIWYG Lanes | S23 proof | S33 |
| S33 | Offline Tooling and Artifact Boundary Hardening | S23 proof | S32 |

**Execution notes:**
- S23 starts first and is the critical-path anchor.
- S26 runs continuously and must never pause.
- S27 must complete before any READY promotion.
- S29 and S30B stay coupled so coverage and bar evidence grow together.
- S33 can run in parallel with later lanes once S23 proves runtime stability.

---

## S23 — Presentation Runtime Productization (critical path)

### Objective
Deliver a real presentation runtime path (not mock-backed) with reload and scene-level reliability.

### Tickets
- [x] S23-T01 Replace mock-backed presentation backend behavior with real renderer-backed integration in the production path.
- [x] S23-T02 Implement and register real `ProfileArena` hot-reload flow.
- [x] S23-T03 Close presentation schema/migration mapping gaps with proof fixtures.
- [x] S23-T04 Expand scene render-command coverage (Map / Battle / Chat / Status / UI).
- [x] S23-T05 Update readiness/docs wording to remove or gate claims that exceed evidence.
- [x] S23-T06 Add deterministic tests for command-stream replay after reload.

### Exit Criteria
- [x] Non-mock presentation path renders scene output end-to-end.
- [x] Hot-reload does not leak profile resources.
- [x] `presentation_runtime` `mainGaps` contains only intentionally deferred items.
- [x] `visual_regression_harness` adds at least one full-scene golden tied to live presentation output.
- [x] `test_spatial_editor.cpp` and `test_presentation_runtime.cpp` include reload + scene command assertions.

---

## S24 — Gameplay Ability Integration and Migration Truth

### Objective
Complete the gameplay ability framework from editor/runtime diagnostics through to real command execution.

### Tickets
- [x] S24-T01 Finalize compat-to-native ability schema mapping.
- [x] S24-T02 Preserve unsupported fields with deterministic fallback payloads.
- [x] S24-T03 Integrate ability command queue into BattleScene/MapScene execution.
- [x] S24-T04 Add deterministic state-machine execution outcomes and diagnostics.
- [x] S24-T05 Add migration/compatibility tests for supported and fallback branches.
- [x] S24-T06 Reconcile all public claims in readiness/status docs.

### Exit Criteria
- [x] Ability runtime is no longer diagnostics-only in game flow paths.
- [x] Ability queue executes with deterministic authoring → runtime behavior.
- [x] `gameplay_ability_framework` residuals are intentional and documented.

---

## S25 — Governance Foundation Completion

### Objective
Prevent evidence/documentation/runtime drift and enforce artifact truth automatically.

### Tickets
- [x] S25-T01 Finish diagnostics/export parity between CLI and UI for all governance sections.
- [x] S25-T02 Enforce release-signoff workflow and status-date checks in PR gates.
- [x] S25-T03 Enforce release-readiness and template-spec parity checks.
- [x] S25-T04 Add truth-reconciler checks for template-subsystem-bar drift.
- [x] S25-T05 Add regression tests for governance gate failure modes.
- [x] S25-T06 Enforce issue-count/date consistency for `docs/PROJECT_AUDIT.md`.
- [x] S25-T07 Add one positive/negative regression that breaks status-date or artifact-path assertions.

### Exit Criteria
- [x] `governance_foundation` is no longer blocked by drift risk.
- [x] CI failure behavior is strict for stale or missing artifacts.
- [x] Project-audit sections are complete across CLI and diagnostics workspace.
- [x] Human-review-gated lines are explicitly represented in checks.

---

## S26 — Compat Bridge Exit Maintenance (continuous)

### Objective
Keep compat trust current as the curated corpus and import/verification lanes evolve.

### Tickets
- [x] S26-T01 Maintain curated corpus workflow with deterministic health checks.
- [x] S26-T02 Add fixtures for dependency drift and profile mismatch.
- [x] S26-T03 Expand failure report outputs (`JSONL`, report ingestion, panel projection).
- [x] S26-T04 Add periodic schema/changelog validation in the compat import lane.

### Exit Criteria
- [x] Weekly compat evidence remains green and auditable.
- [x] Corpus health checks fail fast and deterministically.
- [x] `compat_bridge_exit` future work remains bounded and explicit.

---

## S27 — Human Review / Promotion Closure

### Objective
Close explicit human-review lanes safely before any READY promotion.

### Tickets
- [x] S27-T01 Publish `battle_core` reviewer evidence packet and closure checklist.
- [x] S27-T02 Publish `save_data_core` reviewer evidence packet and unresolved-gap log.
- [x] S27-T03 Publish `compat_bridge_exit` review cadence and maintenance evidence.
- [x] S27-T04 Publish `gameplay_ability_framework` review closure for schema/migration boundaries.
- [x] S27-T05 Publish `presentation_runtime` reviewer evidence packet, or remove human-review promotion wording if closure no longer depends on review-gated promotion.
- [x] S27-T06 Wire promotion rule: READY only after corresponding human-review ticket is complete.

### Exit Criteria
- [x] All reviewer checkpoints exist as canonical artifacts.
- [x] No READY promotion can occur without explicit review completion state.
- [x] Readiness records and [`docs/RELEASE_SIGNOFF_WORKFLOW.md`](../../RELEASE_SIGNOFF_WORKFLOW.md) align on pending review gates.

---

## S28 — Accessibility, Audio, Mod, and Analytics Productionization

### Objective
Close partial lanes currently bounded to scaffolded or compat-only behavior.

### Tickets
- [x] S28-T01 Add accessibility live-ingestion adapters for spatial/audio/battle editors.
- [x] S28-T02 Add actionable accessibility diagnostics with file/line references for those adapters.
- [x] S28-T03 Connect audio mix presets to live runtime backend parameters.
- [x] S28-T04 Add remaining `audio_mix_presets` validator-rule coverage (unknown categories, cross-preset duck conflicts) and keep governance fixtures in parity.
- [x] S28-T05 Implement live mod loading, deterministic unload, and sandboxing.
- [x] S28-T06 Add mod-store contract, validation, and failure reporting.
- [x] S28-T07 Implement analytics upload and session aggregation.
- [x] S28-T08 Add privacy/consent and retention/export workflows.

### Exit Criteria
- [x] `accessibility_auditor` covers non-menu surfaces.
- [x] Audio presets affect runtime playback.
- [x] Live mod loading works under sandbox and determinism constraints.
- [x] `analytics_dispatcher` has an end-to-end event lifecycle.

---

## S29 — Visual Regression and Export Hardening

### Objective
Expand visual confidence from partial proof to broad scene/backend release coverage.

### Tickets
- [x] S29-T01 Add full-frame and scene-specific goldens for remaining scene categories.
- [x] S29-T02 Expand backend coverage where renderer support exists.
- [x] S29-T03 Enforce fail-on-drift policy in CI/local lanes.
- [x] S29-T04 Add deterministic transition goldens for approved scenes.
- [x] S29-T05 Extend integrity checks for signing/enforcement seam in export validation.
- [x] S29-T06 Add export edge-case and tamper test coverage.

### Exit Criteria
- [x] Visual regression baselines are PR-lane enforced.
- [x] Diff artifacts are actionable and stable.
- [x] Export package checks include integrity and repeatability.

---

## S30 — Template Closure for READY Candidates

### Objective
Close bars and WYSIWYG workflows for `jrpg`, `visual_novel`, and `turn_based_rpg`.

### Tickets
- [x] S30-T01 Close `accessibility`, `audio`, `input`, `localization`, and `performance` bars for `jrpg`.
- [x] S30-T02 Close the same bars for `visual_novel`.
- [x] S30-T03 Close the same bars for `turn_based_rpg`.
- [x] S30-T04 Add template-level acceptance tests: edit → preview → export.
- [x] S30-T05 Add canonical template spec artifacts for `jrpg`, `visual_novel`, and `turn_based_rpg`; reconcile with readiness/matrix rows.
- [x] S30-T06 Extend `urpg_project_audit` template-spec and blocker detection coverage so these three templates can fail closed on missing prerequisites.
- [x] S30-T07 Update `readiness_status.json` blocker evidence for these closures.

### Exit Criteria
- [x] These three templates can be promoted with confidence.
- [x] Evidence matches [`docs/TEMPLATE_READINESS_MATRIX.md`](../../TEMPLATE_READINESS_MATRIX.md).
- [x] Canonical template-facing docs and [`docs/PROJECT_AUDIT.md`](../../PROJECT_AUDIT.md) stay aligned with the promoted scope.

---

## S30B — Template Bar Quality Sweep

### Objective
Add missing bar-level depth for localized/template readiness where gaps could block production claims.

### Tickets
- [x] S30B-T01 Add explicit localization completeness proof per template.
- [x] S30B-T02 Add accessibility/input parity for remaining required bars.
- [x] S30B-T03 Add template-specific performance budget diagnostics.
- [x] S30B-T04 Add artifact-level WYSIWYG proof links per bar.

### Exit Criteria
- [x] Required bars do not remain open solely due to missing evidence.
- [x] `mainBlockers` are reduced to roadmap-level, date-scoped constraints.

---

## S31 — Template Expansion and Advanced Product Lanes

### Objective
Progress `PLANNED` and `EXPERIMENTAL` templates to bounded ship-ready slices.

### Tickets
- [x] S31-T01 `monster_collector_rpg`: collection schema, capture mechanics, and party workflow.
- [x] S31-T02 `cozy_life_rpg`: scheduling, social, and economy lane proof.
- [x] S31-T03 `tactics_rpg`: scenario authoring and progression framework.
- [x] S31-T04 `arpg`: movement and growth loop productization.
- [x] S31-T05 `metroidvania_lite`: traversal, map-unlock, and progression mechanics.
- [x] S31-T06 `2_5d_rpg`: raycast authoring and preview proof.
- [x] S31-T07 `2_5d_rpg`: add template-specific export validation and raycast art-pipeline closure work.
- [x] S31-T08 Add template-grade governance/spec/audit closure for `monster_collector_rpg` and closure-visibility artifacts for `arpg`.
- [x] S31-T09 One end-to-end evidence snapshot per template.

### Exit Criteria
- [x] No template remains `PLANNED` or `EXPERIMENTAL` purely by omission.
- [x] Each template has executable, authorable, and testable evidence.
- [x] Each template's canonical spec, readiness record, matrix row, and project-audit contract agree.

---

## S32 — Long-Tail WYSIWYG Lanes

### Objective
Finish remaining partial feature blocks that directly affect full-product claims.

### Tickets
- [x] S32-T01 Character Identity: full Create-a-Character creation lifecycle.
- [x] S32-T02 Character appearance/preview pipeline.
- [x] S32-T03 Achievement registry platform backend integration where scope is not intentionally out-of-tree.
- [x] S32-T04 Reconcile outstanding `mainGaps` entries for partial systems.
- [x] S32-T05 Confirm `export_validator` cryptographic hardening roadmap in scope before READY claims.
- [x] S32-T06 Complete `achievement_registry` trophy export pipeline; keep platform/backend scope wording explicit where integration stays out-of-tree.

### Exit Criteria
- [x] No unresolved short-cycle blockers remain in partial core systems without explicit traceability.
- [x] `character_identity` and `achievement_registry` have end-to-end usability proof where promised.

---

## S33 — Offline Tooling and Artifact Boundary Hardening

### Objective
Protect runtime cleanliness while completing the first approved tooling lanes.

### Tickets
- [x] S33-T01 Complete FAISS retrieval acceptance for deterministic indexing and ingestion.
- [x] S33-T02 Complete SAM/SAM2 segmentation acceptance with manifest-backed outputs.
- [x] S33-T03 Complete Demucs/Encodec preprocessing acceptance with deterministic outputs.
- [x] S33-T04 Add runtime contracts to consume tool artifacts only.
- [x] S33-T05 Add provenance/reproducibility checks for tool outputs.
- [x] S33-T06 Add guardrails that block toolchain/runtime boundary regression.

### Exit Criteria
- [x] Tooling remains outside the shipped runtime dependency graph.
- [x] Runtime uses only reproducible exported artifacts.
- [x] Artifact lineage and checksums are auditable.

---

## Immediate Gating Artifacts

- [x] [`content/readiness/readiness_status.json`](../../../content/readiness/readiness_status.json)
- [x] [`docs/PROGRAM_COMPLETION_STATUS.md`](../../PROGRAM_COMPLETION_STATUS.md)
- [x] [`docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`](../../TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- [x] [`docs/RELEASE_READINESS_MATRIX.md`](../../RELEASE_READINESS_MATRIX.md)
- [x] [`docs/TEMPLATE_READINESS_MATRIX.md`](../../TEMPLATE_READINESS_MATRIX.md)
- [x] [`docs/PROJECT_AUDIT.md`](../../PROJECT_AUDIT.md)
- [x] [`docs/TRUTH_ALIGNMENT_RULES.md`](../../TRUTH_ALIGNMENT_RULES.md)
- [x] [`docs/SUBSYSTEM_STATUS_RULES.md`](../../SUBSYSTEM_STATUS_RULES.md)
- [x] [`docs/TEMPLATE_LABEL_RULES.md`](../../TEMPLATE_LABEL_RULES.md)
- [x] [`docs/RELEASE_SIGNOFF_WORKFLOW.md`](../../RELEASE_SIGNOFF_WORKFLOW.md)
- [x] `tools/ci` governance scripts and log outputs
- [x] `tests` PR-lane slices mapped by sprint
- [x] Offline tooling manifests in `tools/retrieval` and related pipelines

---

## Execution Anchors for Newly Added Tickets

This section gives a low-context implementation model everything needed to execute new tickets without guessing file names, artifact names, or the minimum verification surface.

### Shared Conventions

When a ticket changes canonical readiness or promotion wording, update **all** of:
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TEMPLATE_READINESS_MATRIX.md` (when template status, bars, or blockers change)
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/PROJECT_AUDIT.md` (when the shipped audit contract changes)

**Naming conventions:**
- Template spec files: `docs/templates/<template_id>_spec.md`
- Template spec header must follow the canonical pattern: title line, `Status Date: YYYY-MM-DD`, `Authority: canonical template spec for \`<template_id>\``, then sections for `Purpose`, `Required Subsystems`, `Cross-Cutting Minimum Bars`, `Safe Scope Today`, `Main Blockers`, and `Promotion Path`.
- Signoff artifacts: `docs/<SUBSYSTEM>_CLOSURE_SIGNOFF.md` or `docs/<SUBSYSTEM>_SIGNOFF.md` — follow existing style, do not invent new formats.
- Any ticket that retains "promotion requires human review" wording must leave the state explicitly non-promoting. Passing tests or checks never implies approval.

---

### S27-T05 — `presentation_runtime` Human-Review Closure

**Preferred implementation path:**
- Add `docs/PRESENTATION_RUNTIME_CLOSURE_SIGNOFF.md`.
- If the lane should remain human-review-gated, add a structured `signoff` block for `presentation_runtime` in `content/readiness/readiness_status.json` matching the format used by `battle_core`, `save_data_core`, `compat_bridge_exit`, and `gameplay_ability_framework`.
- If the lane should no longer be review-gated, remove human-review promotion wording from `content/readiness/readiness_status.json`, `docs/RELEASE_READINESS_MATRIX.md`, and any matching summary wording in `docs/PROGRAM_COMPLETION_STATUS.md`.

**Primary file anchors:**
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

**Presentation evidence anchors:**
- `engine/core/presentation/presentation_runtime.cpp`
- `engine/core/presentation/presentation_bridge.cpp`
- `engine/core/presentation/profile_arena.cpp`
- `engine/core/presentation/release_validation.cpp`
- `tests/unit/test_presentation_runtime.cpp`
- `tests/unit/test_spatial_editor.cpp`
- `docs/presentation/VALIDATION.md`

**Minimum verification:**
```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/run_presentation_gate.ps1
```

---

### S28-T04 — Remaining `audio_mix_presets` Validator-Rule Coverage

**Goal:** Extend the validator and fixture surface so the remaining backlog called out by `docs/RELEASE_READINESS_MATRIX.md` is executable instead of prose-only.

**Primary file anchors:**
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

**Concrete checks to add:**
- Unknown category detection.
- Cross-preset duck-rule conflict detection.
- Fixture coverage for both negative cases.
- Audit/governance parity if new validator output changes the shipped contract.

**Minimum verification:**
```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_audio_governance.ps1
```
Then build and run focused audio preset tests through `urpg_tests`.

---

### S30-T05 — Canonical Specs for `jrpg`, `visual_novel`, `turn_based_rpg`

**Create:**
- `docs/templates/jrpg_spec.md`
- `docs/templates/visual_novel_spec.md`
- `docs/templates/turn_based_rpg_spec.md`

**Format references:** `docs/templates/monster_collector_rpg_spec.md` and `docs/templates/2_5d_rpg_spec.md`.

Each new spec must reflect current readiness data:
- Required subsystems from `content/readiness/readiness_status.json`.
- Current bar statuses from `content/readiness/readiness_status.json`.
- Safe-scope wording must stay conservative.
- Blockers must not exceed or understate canonical readiness blockers.

**Also update:**
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json` if the plan intentionally changes blockers/bars.
- `docs/PROGRAM_COMPLETION_STATUS.md` if product-facing template wording changes.

---

### S30-T06 — `urpg_project_audit` Template-Spec Coverage for READY Candidates

**Goal:** Make the audit fail closed for `jrpg`, `visual_novel`, and `turn_based_rpg` the same way it already does for existing expansion template specs.

**Primary file anchors:**
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `editor/diagnostics/project_audit_panel.cpp`
- `docs/PROJECT_AUDIT.md`
- `tools/ci/check_release_readiness.ps1`

**Expected behavior:**
- Missing canonical spec file → concrete issue reported.
- Wrong `Authority:` line → concrete issue reported.
- Required-subsystem drift from readiness → concrete issue reported.
- Cross-cutting bar drift from readiness → concrete issue reported.

**Minimum verification:**
```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
Then run focused project-audit tests through `urpg_tests`.

---

### S31-T07 — `2_5d_rpg` Export Validation and Art-Pipeline Closure

This ticket closes the exact blockers named in canonical readiness, not just rendering proof:
- Raycast art pipeline.
- Map authoring adapters at production grade.
- Template-specific export validation.

**Primary file anchors:**
- `docs/templates/2_5d_rpg_spec.md`
- `engine/core/render/raycast_renderer.h`
- `engine/core/export/export_validator.h`
- `engine/core/export/export_validator.cpp`
- `editor/spatial/spatial_authoring_workspace.cpp`
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `tests/unit/test_export_validator.cpp`

**Minimum expected outputs:**
- One canonical doc update naming the accepted 2.5D art-pipeline contract.
- One export-validator rule or fixture specific to 2.5D template requirements.
- One test that fails when the 2.5D-specific export requirement is missing.

---

### S31-T08 — `monster_collector_rpg` Governance Closure and `arpg` Closure Visibility

These are two separate deliverables and must not be collapsed into a single doc-only change.

**For `monster_collector_rpg`:**
- Update `docs/templates/monster_collector_rpg_spec.md`.
- Update `tools/audit/urpg_project_audit.cpp` if new template artifacts or blockers become machine-checkable.
- Reconcile `docs/TEMPLATE_READINESS_MATRIX.md` and `content/readiness/readiness_status.json`.

**For `arpg`:**
- Create `docs/templates/arpg_spec.md` if the lane is being given first-class closure visibility.
- Ensure blocker wording matches the current `arpg` readiness record before broadening claims.

**Minimum verification:**
```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
Then run focused project-audit tests through `urpg_tests`.

---

### S32-T06 — `achievement_registry` Trophy Export Pipeline

**Scope clarification:** This ticket covers exportable trophy/achievement artifacts and deterministic pipeline wiring. It does not grant permission to overclaim live platform backend integration if that remains out-of-tree.

**Primary file anchors:**
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

**Expected closure shape:**
- Deterministic exported trophy/achievement artifact or manifest format.
- Validator coverage for that artifact shape.
- Fixture coverage for valid and invalid export cases.
- Explicit wording in canonical docs distinguishing in-tree export support from out-of-tree platform backend integration.

**Minimum verification:**
```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_achievement_governance.ps1
```
Then build and run focused achievement tests through `urpg_tests`.

---

## Low-Context Model Guardrails

- Use only `PLANNED`, `EXPERIMENTAL`, `PARTIAL`, `READY`, or `BLOCKED` as status labels. Do not invent new ones.
- Do not promote any subsystem or template by editing docs alone — evidence, tests, and audit/gate behavior must move with the doc change.
- Do not add parallel planning authority. Update the canonical stack in `docs/TRUTH_ALIGNMENT_RULES.md`.
- When touching `content/readiness/readiness_status.json`, keep `statusDate` synchronized with every canonical doc updated in the same slice.
- Prefer adding one focused negative test per new governance rule so drift fails loudly instead of relying on prose.

---

## Remaining Work Mapping (Non-Ready Coverage)

**Non-ready subsystems (14):**
`battle_core`, `save_data_core`, `compat_bridge_exit`, `presentation_runtime`, `gameplay_ability_framework`, `governance_foundation`, `character_identity`, `achievement_registry`, `accessibility_auditor`, `visual_regression_harness`, `audio_mix_presets`, `export_validator`, `mod_registry`, `analytics_dispatcher`

**Non-ready templates (9):**
`jrpg`, `visual_novel`, `turn_based_rpg`, `tactics_rpg`, `arpg`, `monster_collector_rpg`, `cozy_life_rpg`, `metroidvania_lite`, `2_5d_rpg`

No mapped non-ready item is unassigned.

---

## Closeout Snapshot (2026-04-23)

- Readiness coverage rerun: 14 non-ready subsystems, 9 non-ready templates, and zero unmapped IDs.
- `powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1` => passed
- `powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1` => passed
- `powershell -ExecutionPolicy Bypass -File tools/ci/run_local_gates.ps1` => passed
- README/docs search found no accidental `READY` claims for non-ready subsystem/template ids outside promotion-path language.
- Template-spec drift warnings are cleared from the local gate wrapper.
- Compat corpus health warnings are cleared from the local gate wrapper.
- Wrapper warning output is clean: template-spec drift, compat corpus health, and vendored SDL2 deprecation warnings are all cleared.

---

## Delivery Cadence

- [x] One sprint update per bounded code slice in `WORKLOG.md`.
- [x] Sprint-end checkpoint includes:
  - [x] Build + PR-lane tests.
  - [x] Gate scripts.
  - [x] Governance and truth-reconciler run.
  - [x] Artifact/date drift checks.
  - [x] Explicit readiness record updates.

---

## Known Risks

- [ ] Signing/enforcement in export may expand scope if claimed as fully shipped prematurely.
- [ ] Template bar closure can regress after runtime-only fixes.
- [ ] Human-review workflows introduce controlled waiting states that must be planned into the schedule.
- [ ] Tooling pipeline changes may destabilize reproducibility if outputs are not pinned.

---

## Exhaustiveness Check

Coverage baseline captured this pass:
- Non-ready subsystems: 14
- Non-ready templates: 9
- Missing subsystem IDs in this plan: 0
- Missing template IDs in this plan: 0

Ongoing validation steps:
- [x] Re-run readiness parse and confirm `status != READY` bucket parity.
- [x] Re-run roadmap presence checks and confirm zero unmapped IDs.
- [x] Confirm no README/ADR/project status file claims READY on unresolved items above.

---

## Explicitly Known Partial Boundaries (Not Missed Features)

- `tools/retrieval/shared/retrieval_index.py` — abstract adapter contracts intentionally left unbound for optional retrieval backends.
- `tools/retrieval/embedding_jobs/external_embedding_adapter.py` — optional backend adapters are explicit extension points, not hidden required defaults.
- `runtimes/compat_js/window_compat.h` / `window_compat.cpp` — compatibility-mode rendering edge cases intentionally deferred in boundary tickets.
- `runtimes/compat_js/battle_manager.h` — staged compatibility behavior already represented in `compat_bridge_exit` closure work.

---

## Per-Sprint Execution Contract

- [ ] Each sprint has a single canonical acceptance command set and one evidence index entry.
- [ ] Each sprint writes or updates one authoritative readiness-related artifact.
- [ ] Any partial closure keeps a single explicit `mainGaps` reason and rollback owner.
