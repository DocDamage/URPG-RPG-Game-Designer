# Project Audit

Status Date: 2026-04-28

`urpg_project_audit` is the current conservative project-facing audit command for answering a simple question:

What is incomplete or unsafe in this project right now?

## Audit Scope

The governance-foundation slice defines the audit contract for:

- missing required subsystem setup
- template blockers
- schema version mismatches
- missing localization keys
- missing input bindings
- missing audio event registrations
- missing canonical input-remap and controller-binding governance artifact paths where the roadmap already defines them
- accessibility-rule violations detectable at authoring time
- unresolved import leftovers
- missing canonical localization bundle/report/export artifact paths where the roadmap already defines them
- export blockers
- release-blocking overclaims

## Current State

The first shipped slice is a conservative readiness-derived scanner, not a full project scanner.

That means:

- the CLI now reads the canonical readiness dataset and emits a real audit report
- the diagnostics surfaces can render the emitted report model, including the governance detail the CLI now carries for asset-report counters (`normalizedCount`, `promotedCount`, promoted visual/audio lane counts, and WYSIWYG smoke-proof count), schema/changelog, canonical project-schema governance sections (`localization`, `input`, and `exportProfiles`), canonical input/localization-bundle/export artifact-path checks, repo-local localization consistency evidence from the canonical report path, first-slice accessibility/audio/performance/character/mod/analytics artifact-governance sections, the bounded controller-binding runtime/panel plus input governance script/fixture contract, the release-signoff workflow artifact, and human-review-gated subsystem signoff artifacts plus their structured signoff-contract state
- the output model remains intentionally small and conservative
- asset-intake and schema/changelog governance can affect the reported result only where those concerns are already represented in the canonical readiness/template governance records

It does **not** yet mean every audit category above is implemented. The current scanner is mainly grounded in:

- subsystem readiness status
- selected-template required subsystem status
- template minimum-bar status
- template main blockers
- conservative parity checks between the selected template's canonical spec artifact and the readiness record for required subsystems and cross-cutting bar rows
- structured signoff-contract checks for the currently human-review-gated subsystem lanes
- limited project-schema governance-shape checks for `localization`, `input`, and `exportProfiles`

It may also surface asset-intake or schema/changelog governance concerns when they have already been folded into those readiness/template blocker records, and it can now flag when `project.schema.json` lacks the canonical `localization`, `input`, or `exportProfiles` governance sections, or when those sections are malformed for the bounded contract the audit understands. It fails the asset-governance lane when normalized intake, promoted intake, promoted visual lane, promoted audio lane, or WYSIWYG smoke-proof counters regress to zero in the canonical asset-intake report. It can also check for missing canonical input-remap/controller-binding/localization-bundle/export artifact paths where the roadmap already defines them; the bounded input contract now includes `InputRemapStore`, `ControllerBindingRuntime`, `ControllerBindingPanel`, `check_input_governance.ps1`, and the canonical input fixture path, while the bounded localization contract uses `localization_bundle.schema.json`, `LocaleCatalog`, `CompletenessChecker`, `check_localization_consistency.ps1`, and the canonical localization consistency report. The bounded export contract now includes the compiled `ExportPackager` implementation and canonical `tools/pack/pack_cli.cpp` packaging entrypoint, while broader signing/notarization/runtime-enforcement claims remain outside the audit's shipped checks. It also consumes the canonical localization consistency report at `imports/reports/localization/localization_consistency_report.json` as bounded evidence for missing-key drift. It does **not** yet crawl the repo independently for every asset-intake, schema-version, changelog, localization, input, or export condition, and it should not be described as a full project scanner.

## Intended Output Shape

The audit report is expected to carry:

- `schemaVersion`
- `statusDate`
- `headline`
- `summary`
- issue list
- release blocker count
- export blocker count
- template status context
- governance detail for asset report counters, readiness schema/changelog, and project schema governance-shape checks
- governance detail for canonical input/localization-bundle/export and accessibility/audio/performance artifact-governance sections
- governance detail for the bounded controller-binding runtime/panel plus input governance script/fixture contract within `inputArtifacts`
- governance detail for canonical character, mod, and analytics artifact-governance sections
- governance detail for repo-local localization consistency evidence from the canonical report path
- governance detail for the canonical release-signoff workflow artifact
- governance detail for human-review-gated subsystem signoff artifacts, including per-artifact expected-artifact rows and structured signoff-contract state
- governance detail for canonical template spec artifacts tied to the selected template context, including per-artifact expected-artifact rows and conservative required-subsystem and cross-cutting-bar parity checks against readiness

That governance detail is intended to stay in parity between the CLI/report model and the diagnostics rendering path, including the bounded `localizationEvidence` status, counters, and bundle payload loaded from the canonical localization consistency report. It still does **not** change the core truth constraint: rendering richer governance detail does not make this a full project scanner.

Current CLI support:

- `--json`
- `--input <path>`
- `--asset-report <path>`
- `--localization-report <path>`
- `--template <id>`

## Truth Rule

The audit command must never imply that a project is release-ready simply because no implemented checks fired. Missing coverage should remain visible as governance scope, not silently treated as success. The same rule applies to asset-intake and schema/changelog governance: absence of a reported issue only means no implemented readiness-derived check fired for the current records.
