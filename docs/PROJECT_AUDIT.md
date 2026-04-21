# Project Audit

Status Date: 2026-04-21

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
- missing canonical input-governance artifact paths where the roadmap already defines them
- accessibility-rule violations detectable at authoring time
- unresolved import leftovers
- missing canonical localization/export artifact paths where the roadmap already defines them
- export blockers
- release-blocking overclaims

## Current State

The first shipped slice is a conservative readiness-derived scanner, not a full project scanner.

That means:

- the CLI now reads the canonical readiness dataset and emits a real audit report
- the diagnostics surfaces can render the emitted report model, including the governance detail the CLI now carries for asset-report, schema/changelog, canonical input/localization/export artifact-path checks, repo-local localization consistency evidence from the canonical report path, first-slice accessibility/audio/performance artifact-governance sections, the release-signoff workflow artifact, and human-review-gated subsystem signoff artifacts plus their structured signoff-contract state
- the output model remains intentionally small and conservative
- asset-intake and schema/changelog governance can affect the reported result only where those concerns are already represented in the canonical readiness/template governance records

It does **not** yet mean every audit category above is implemented. The current scanner is mainly grounded in:

- subsystem readiness status
- selected-template required subsystem status
- template minimum-bar status
- template main blockers
- conservative parity checks between the selected template's canonical spec artifact and the readiness record for required subsystems and cross-cutting bar rows
- structured signoff-contract checks for the currently human-review-gated subsystem lanes
- limited project-schema presence checks for localization, input/controller-governance, and export-governance sections

It may also surface asset-intake or schema/changelog governance concerns when they have already been folded into those readiness/template blocker records, and it can now flag when `project.schema.json` lacks obvious localization, input/controller, or export-governance sections for templates that still depend on those bars. It can also check for missing canonical input/localization/export artifact paths where the roadmap already defines them, and it now consumes the canonical localization consistency report at `imports/reports/localization/localization_consistency_report.json` as bounded evidence for missing-key drift. It does **not** yet crawl the repo independently for every asset-intake, schema-version, changelog, localization, input, or export condition, and it should not be described as a full project scanner.

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
- governance detail for asset report, readiness schema/changelog, and project schema presence checks
- governance detail for canonical input/localization/export and accessibility/audio/performance artifact-governance sections
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
