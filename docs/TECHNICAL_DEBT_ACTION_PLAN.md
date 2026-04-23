# URPG Technical Debt Action Plan

Date: 2026-04-23  
Status: active execution annex  
Scope: turn the current technical-debt audit into bounded sprint work with explicit checklists, dependencies, and exit criteria

This file is an execution annex to the canonical debt/status stack:

- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`

If this file conflicts with those documents, update the canonical documents first or in the same change. This plan does not create a parallel planning authority.

## Audit Inputs

This action plan closes the debt themes surfaced by the current repo audit:

- fixture-backed compat/plugin runtime remains intentionally non-live
- project schema is thinner than the repo's own readiness/audit contract
- localization runtime and localization governance/tooling are not yet using one canonical contract
- export pipeline still lacks one canonical packaging CLI plus broader discovery/signing closure
- governed asset intake is cataloged but not yet mirrored, normalized, or promoted
- gameplay ability runtime depth is ahead of async task backend depth
- `WindowCompat` still has fidelity gaps around gauges, bitmap contents, and JS color parsing
- multiple lanes remain human-review-gated instead of promotion-ready

## Planning Rules

- Every sprint must close code, tests, audit wiring, and canonical docs together.
- No status promotion is allowed on test evidence alone when the readiness record still requires human review.
- If a gap is intentionally kept out of scope, the code, tests, and docs must all say so in the same wording.
- Offline ML and research tooling stays under `tools/` and may only affect shipped runtime behavior through exported artifacts.

## Global Definition of Done

- [ ] Code path is implemented, or the claim is intentionally narrowed and enforced.
- [ ] Targeted tests exist for positive and negative behavior.
- [ ] `urpg_project_audit` reflects the new contract without false positives or false negatives.
- [ ] `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/RELEASE_READINESS_MATRIX.md`, and `content/readiness/readiness_status.json` are updated to match the code.
- [ ] Relevant local gates and sprint-specific checks are recorded in `WORKLOG.md`.
- [ ] Any remaining residual is explicit in `mainGaps`, `mainBlockers`, or reviewer signoff artifacts.

## Sprint Map

| Sprint | Theme | Depends On | Can Run In Parallel With |
| --- | --- | --- | --- |
| `TD-S01` | Compat Runtime Contract Lock | - | `TD-S02` |
| `TD-S02` | Project Schema and Audit Contract Alignment | - | `TD-S01` |
| `TD-S03` | Localization Governance Unification | `TD-S02` | `TD-S04`, `TD-S06` |
| `TD-S04` | Export Hardening and Canonical CLI | `TD-S02` | `TD-S03`, `TD-S05`, `TD-S06` |
| `TD-S05` | Asset Intake Activation | - | `TD-S04`, `TD-S06`, `TD-S07` |
| `TD-S06` | Gameplay Ability Runtime Depth | `TD-S01` | `TD-S03`, `TD-S04`, `TD-S05` |
| `TD-S07` | WindowCompat and UI Fidelity | `TD-S01` | `TD-S05`, `TD-S06` |
| `TD-S08` | Promotion and Human Review Closure | `TD-S03`, `TD-S04`, `TD-S05`, `TD-S06`, `TD-S07` | - |

## Recommended Order

1. Start `TD-S01` and `TD-S02` immediately so the repo stops carrying ambiguous runtime/governance contracts.
2. Start `TD-S03` once the project schema and audit naming are stable.
3. Run `TD-S04`, `TD-S05`, `TD-S06`, and `TD-S07` as bounded parallel slices after the contract work is settled.
4. Run `TD-S08` only after the evidence stack is green and reviewer packets are current.

---

## TD-S01 - Compat Runtime Contract Lock

### Objective

Make the in-tree compat/plugin runtime contract explicit, stable, and impossible to overclaim.

### Tasks

- [ ] Decide one shipped contract for the in-tree compat runtime:
  `fixture-backed import/verification harness` or `bounded live runtime subset`.
- [ ] Update `runtimes/compat_js/quickjs_runtime.cpp` to reflect the chosen contract in comments, diagnostics, and failure semantics.
- [ ] Update `runtimes/compat_js/plugin_manager.cpp` to reflect the same contract in method status text, load/execute diagnostics, and async callback wording.
- [ ] Reconcile `runtimes/compat_js/battle_manager.h` and `runtimes/compat_js/window_compat.cpp` status notes so they do not imply a broader runtime than the chosen contract supports.
- [ ] Add or update focused tests that prove unsupported live-JS behavior fails loudly and deterministically.
- [ ] Add a wording guard in docs or CI so future "live runtime" language cannot drift back in without matching test evidence.
- [ ] Reconcile the chosen contract across `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/RELEASE_READINESS_MATRIX.md`, and `content/readiness/readiness_status.json`.

### Exit Criteria

- [ ] One explicit compat runtime contract exists in code, tests, and docs.
- [ ] No remaining repo wording implies live plugin execution beyond the verified surface.
- [ ] Unsupported behavior is rejected or labeled, not silently stubbed behind optimistic wording.

---

## TD-S02 - Project Schema and Audit Contract Alignment

### Objective

Bring the canonical project schema up to the same governance surface the repo audit already expects.

### Tasks

- [ ] Decide the canonical project-governance shape for `localization`, `input` or `controllerBindings`, and `export` or `exportProfiles`.
- [ ] Extend `content/schemas/project.schema.json` to include the chosen governance sections with bounded validation rules.
- [ ] Add or update a canonical project fixture covering the new sections.
- [ ] Update `tools/audit/urpg_project_audit.cpp` so it validates the same section names and shapes the schema actually defines.
- [ ] Add unit coverage for project-schema positive and negative cases, including missing-governance and malformed-governance scenarios.
- [ ] Update `docs/PROJECT_AUDIT.md`, `docs/RELEASE_READINESS_MATRIX.md`, and `docs/PROGRAM_COMPLETION_STATUS.md` so they describe the same schema contract.
- [ ] Re-run readiness/audit checks and capture the expected reduction in project-artifact warnings.

### Exit Criteria

- [ ] `project.schema.json` and `urpg_project_audit` use the same governance vocabulary.
- [ ] Project-audit warnings for localization/input/export are caused by real missing project data, not schema/audit drift.
- [ ] The new schema remains conservative and does not promote unfinished product bars to `READY`.

---

## TD-S03 - Localization Governance Unification

### Objective

Make localization runtime, schema, tooling, audit, and diagnostics use one canonical artifact contract.

### Tasks

- [ ] Choose the canonical localization schema/tooling names and keep them consistent across code, audit, and docs.
- [ ] Either add the audit-expected artifacts (`content/schemas/localization_catalog.schema.json`, `tools/localization/extract_localization.cpp`, `tools/localization/writeback_localization.cpp`) or narrow the audit to the already-shipped bundle contract.
- [ ] Keep `tools/ci/check_localization_consistency.ps1` aligned with the chosen canonical artifact names and report shape.
- [ ] Ensure the canonical localization report remains readable by `urpg_project_audit` and by diagnostics/export pathways without custom translation glue.
- [ ] Add or update focused tests for missing-bundle, malformed-bundle, and clean-report cases.
- [ ] Reconcile `docs/PROJECT_AUDIT.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, and readiness records so they distinguish runtime support from governance/tooling closure.

### Exit Criteria

- [ ] The repo has one canonical localization contract for runtime, audit, tooling, and docs.
- [ ] Localization warnings in the audit reflect real missing governance work, not mismatched file names.
- [ ] Diagnostics workspace and CLI stay in parity for localization evidence.

---

## TD-S04 - Export Hardening and Canonical CLI

### Objective

Close the remaining export debt around the canonical entrypoint, automatic asset discovery, and stronger release-hardening seams.

### Tasks

- [ ] Decide the canonical packager entrypoint and make it real:
  add `tools/pack/pack_cli.cpp` or update the audit/docs to the official replacement.
- [ ] Implement automatic project-wide asset discovery and bundling instead of relying only on narrow manifest-driven staging.
- [ ] Preserve deterministic bundle ordering and stable manifest/hash behavior across identical exports.
- [ ] Extend export hardening beyond the current lightweight integrity story with explicit signing/notarization/runtime-enforcement seams.
- [ ] Add negative tests for tampered bundles, missing staged assets, and partial or malformed export roots.
- [ ] Update `tools/audit/urpg_project_audit.cpp`, `docs/RELEASE_READINESS_MATRIX.md`, and `docs/PROGRAM_COMPLETION_STATUS.md` so export wording matches the shipped contract exactly.
- [ ] Keep post-export validation, smoke export, and preflight readiness terminology sharply separated.

### Exit Criteria

- [ ] The repo has one canonical export CLI or one canonical documented replacement.
- [ ] Automatic asset discovery is real and covered by tests.
- [ ] Export hardening language no longer outruns the actual implementation.

---

## TD-S05 - Asset Intake Activation

### Objective

Move asset intake from truthful catalog-only governance into real mirrored, normalized, and promoted content flow.

### Tasks

- [ ] Select the first promotable intake sources from the current catalog:
  at least one visual pack and one audio pack.
- [ ] Mirror source artifacts into governed intake roots with provenance and license notes preserved.
- [ ] Normalize at least one source pack into repo-ready outputs and record the transformation chain in manifests/reports.
- [ ] Promote a bounded subset into canonical runtime/editor/export-facing roots.
- [ ] Update `imports/manifests`, `imports/reports`, and asset-governance docs with real promotion records.
- [ ] Add audit coverage that fails if a promoted lane regresses to zero normalized or zero promoted assets.
- [ ] Add one WYSIWYG-facing smoke proof that a promoted asset appears in editor, runtime, and export paths.

### Exit Criteria

- [ ] `source_capture_status.json` is no longer zero for normalized and promoted counts.
- [ ] At least one visual asset lane and one audio asset lane are promotion-backed and auditable.
- [ ] Asset-governance records stay truthful after real promotion work starts.

---

## TD-S06 - Gameplay Ability Runtime Depth

### Objective

Close the gap between the broad gameplay ability authoring surface and the currently narrow async task/runtime backend.

### Tasks

- [ ] Implement `AbilityTask_WaitInput`.
- [ ] Implement `AbilityTask_WaitEvent`.
- [ ] Implement `AbilityTask_WaitProjectileCollision`, or formally narrow the in-tree task contract if projectile collision will remain out of scope.
- [ ] Add deterministic runtime tests for each shipped task backend, including positive completion and blocked/timeout paths where applicable.
- [ ] Extend battle/map integration coverage so task-driven abilities remain stable through diagnostics refresh and normal scene update loops.
- [ ] Decide whether `activeCondition` and `passiveCondition` remain intentionally unsupported or gain a bounded evaluator; align code, tests, and docs either way.
- [ ] Reconcile `content/readiness/readiness_status.json`, `docs/GAF_CLOSURE_SIGNOFF.md`, and `docs/RELEASE_READINESS_MATRIX.md` with the true residuals after the task work lands.

### Exit Criteria

- [ ] Gameplay ability async execution is deeper than `WaitTime` only.
- [ ] Remaining unsupported ability behavior is explicit and intentionally documented.
- [ ] The GAF readiness story is based on the real shipped backend depth, not just editor breadth.

---

## TD-S07 - WindowCompat and UI Fidelity

### Objective

Reduce the remaining fidelity debt in `WindowCompat` so plugin-facing UI behavior is less stub-shaped and more testable.

### Tasks

- [ ] Implement a real gradient-capable gauge primitive, or explicitly narrow the gauge contract and enforce it consistently.
- [ ] Replace handle-only `contents()` behavior with a minimal backing pixel buffer, or codify a non-pixel compat contract and update all dependent tests/docs accordingly.
- [ ] Implement color-object parsing in JS-facing `changeTextColor` and `drawGauge` paths instead of leaving TODO-only behavior.
- [ ] Add plugin-profile regressions covering gauge rendering semantics, contents lifecycle, and color parsing.
- [ ] Re-audit `FULL`, `PARTIAL`, and `STUB` labels across `Window_Base`, `Window_Selectable`, and related battle/window surfaces.
- [ ] Reconcile readiness/docs wording with the new fidelity level so the window surface is neither understated nor overstated.

### Exit Criteria

- [ ] `WindowCompat` no longer carries silent TODO behavior in high-use UI paths.
- [ ] Status labels for window methods are backed by tests and implementation details.
- [ ] Plugin-facing UI limitations are narrow, named, and deterministic.

---

## TD-S08 - Promotion and Human Review Closure

### Objective

Convert the current review-gated lanes into explicit review decisions instead of leaving them permanently stuck behind stale promotion language.

### Tasks

- [ ] Complete human review for `battle_core`.
- [ ] Complete human review for `save_data_core`.
- [ ] Complete human review for `compat_bridge_exit`.
- [ ] Complete human review for `presentation_runtime`.
- [ ] Complete human review for `gameplay_ability_framework`.
- [ ] Record the review outcome in each canonical signoff artifact with explicit approved, residual, and rejected sections.
- [ ] Promote readiness states only where review is complete and the evidence stack is green.
- [ ] Keep any non-approved lane in `PARTIAL` with updated blockers instead of silently drifting toward `READY`.
- [ ] Re-run `urpg_project_audit`, release-readiness checks, and targeted suites before any status-promotion commit.

### Exit Criteria

- [ ] Every human-review-gated lane has an explicit current decision.
- [ ] `content/readiness/readiness_status.json` and `docs/RELEASE_READINESS_MATRIX.md` reflect real review state, not pending assumptions.
- [ ] No remaining status promotion depends on an implicit or stale human review.

---

## Always-On Checklist

- [ ] Keep `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`, this action plan, and `docs/PROGRAM_COMPLETION_STATUS.md` aligned at the end of each sprint.
- [ ] Keep `urpg_project_audit` outputs conservative; prefer real blockers over optimistic assumptions.
- [ ] Do not mark a sprint complete if the code landed but the audit/docs still describe the old contract.
- [ ] Do not let this annex become a parallel authority; fold any major scope change back into the canonical remediation and status docs.
