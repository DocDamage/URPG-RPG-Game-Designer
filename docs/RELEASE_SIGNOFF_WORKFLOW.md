# Release Signoff Workflow

Status Date: 2026-04-23

This document is the canonical workflow for release-signoff discipline across the governed readiness stack.

This workflow does **not** grant release approval. In plain terms, it does not grant release approval. It records the required conservative steps and keeps machine-enforced governance aligned with human review.

## Purpose

The repo already carries readiness records, release/template matrices, project-audit surfaces, and subsystem signoff artifacts. This document ties those pieces into one canonical workflow so release-signoff discipline is not only implied by scattered docs and gates.

## Scope

This workflow currently governs:

- canonical readiness records in `content/readiness/readiness_status.json`
- release/template matrix alignment
- project-audit governance surfaces
- human-review-gated subsystem signoff artifacts
- truth-alignment and schema/governance drift checks

It does not replace subsystem-specific closure docs such as a signoff artifact for Battle Core, Save/Data Core, or Compat Bridge Exit.

## Workflow

1. Update the implementation, tests, and canonical docs for the claimed scope.
2. Reconcile `content/readiness/readiness_status.json` and the relevant matrix rows so status language stays conservative.
3. Refresh or add the required signoff artifact for any lane whose promotion language still depends on human review.
4. Run `tools/ci/check_release_readiness.ps1` to verify the governed readiness stack is internally aligned.
5. Run `tools/ci/truth_reconciler.ps1` to verify cross-doc truth alignment, signoff artifact wording, and workflow-artifact presence.
6. Hand the resulting evidence to human review for actual release judgment.

## Required Evidence

Release-signoff discipline for the current governed stack should include:

- a canonical readiness record
- aligned release/template matrix rows where applicable
- a structured signoff contract in `content/readiness/readiness_status.json` for each human-review-gated subsystem lane
- a signoff artifact for each human-review-gated subsystem lane
- current validation evidence
- canonical docs that stay below promotion language until human review actually happens

## Non-Promoting Rule

Passing the workflow checks means the repo is consistent with the current canonical workflow. It does **not** grant release approval, does not mark a subsystem `READY`, and does not replace human review.

## Enforcement Hooks

The current machine-enforced hooks for this canonical workflow are:

- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`

Those checks enforce artifact presence, status-date alignment, and conservative wording. They do not fabricate approvals or claim that a review happened when it did not.

## Structured Contract Rule

For the currently human-review-gated subsystem lanes, the machine-checked readiness record must also carry a narrow structured signoff contract:

- `required: true`
- the canonical `artifactPath`
- `promotionRequiresHumanReview: true`
- the canonical workflow path `docs/release/RELEASE_SIGNOFF_WORKFLOW.md`

This contract exists to keep artifact paths and non-promoting review requirements aligned across readiness, audit, and gates. It does **not** record that a human review happened, and it does not grant release approval.
