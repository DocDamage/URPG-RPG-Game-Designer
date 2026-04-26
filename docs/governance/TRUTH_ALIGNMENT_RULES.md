# Truth Alignment Rules

Status Date: 2026-04-23

This document defines how readiness claims stay aligned across canonical docs, template docs, diagnostics surfaces, and machine-readable readiness records.

## Core Rule

No release-facing status claim may exceed the strongest evidence-backed status present in the canonical readiness records.

## Sources of Truth

When these disagree, the canonical resolution order is:

1. `content/readiness/readiness_status.json`
2. `docs/release/RELEASE_READINESS_MATRIX.md`
3. `docs/governance/TEMPLATE_READINESS_MATRIX.md`
4. `docs/status/PROGRAM_COMPLETION_STATUS.md`
5. `docs/archive/planning/NATIVE_FEATURE_ABSORPTION_PLAN.md`

Older planning inputs, archive docs, and addenda are reference material, not status authorities.

## Alignment Requirements

The following must agree:

- subsystem readiness labels
- template readiness labels
- major public-facing completion language
- project-audit blocker vocabulary
- project-audit governance section coverage for the shipped contract
- signoff-artifact presence and human-review-gated wording for subsystems that depend on signoff-based promotion language
- structured signoff-contract fields for subsystems that depend on signoff-based promotion language
- release-signoff workflow artifact presence and wording for the governed readiness stack
- schema/version governance expectations

If a document or diagnostics surface needs nuance, it may be more conservative than the readiness records, but never more optimistic.

## Allowed Claim Patterns

- Canonical docs may say a lane is `PLANNED`, `EXPERIMENTAL`, `PARTIAL`, or `READY` only if that matches the readiness records.
- Public docs may summarize scope, but they must not upgrade the status relative to the readiness records.
- Diagnostics surfaces may provide extra detail, but they must not invent a higher readiness label.

## Drift Cases That Must Fail Checks

- a subsystem matrix row says `PARTIAL`, but a public doc calls it `READY`
- a template matrix row says `EXPERIMENTAL`, but a roadmap summary implies release-safe support
- a docs/status update marks a governance feature landed before its files, checks, or build registration exist
- schema/version governance language claims enforcement when only policy text exists

## Governance Expectations

Truth-alignment checks should prefer concrete failures over fuzzy heuristics:

- missing required files
- missing required rows
- status labels that exceed the readiness record
- missing changelog linkage for governed schema changes

This keeps the checks maintainable and reduces performative status language.
