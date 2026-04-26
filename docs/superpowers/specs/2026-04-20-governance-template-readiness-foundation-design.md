# Governance and Template Readiness Foundation Design

**Date:** 2026-04-20
**Scope:** Canonical governance foundation for subsystem readiness, template readiness, project audit truth, and cross-cutting minimum-bar enforcement

## Goal

Add the missing governance layer that lets URPG describe subsystem readiness, template readiness, and product confidence without drifting away from verified implementation state.

The first shipped slice should establish:

- a canonical subsystem release-readiness matrix
- a canonical template readiness matrix with explicit label rules
- a project audit model that can report real blockers and overclaims
- CI-backed truth reconciliation for subsystem/template/public readiness language
- version/changelog governance hooks that keep expanding schemas reviewable

This slice is meant to make future roadmap execution trustworthy, not to finish every template-specific feature in one pass.

## Problem

URPG now has meaningful runtime, editor, migration, compat, and presentation capability, but the repo still lacks a single evidence-gated way to answer:

- which subsystems are actually `READY`
- which templates are actually safe to claim as `READY`, `PARTIAL`, or `EXPERIMENTAL`
- what is incomplete or unsafe in a real project
- whether docs and diagnostics overclaim readiness
- whether schema changes are versioned and reviewable

The existing canonical docs and validation snapshots prove important slices, but they are not yet a formal readiness-governance system. That gap creates product risk: template support and subsystem status can drift from verified reality even when code quality improves.

## Chosen Approach

Implement the governance foundation in one direction:

1. define authoritative readiness records
2. define the rules that permit readiness labels
3. define audit output that reports violations against those rules
4. enforce the rules in CI and diagnostics surfaces

This keeps planning claims downstream of evidence rather than letting docs invent status on their own.

## Architecture

### Layer 1: Authoritative readiness records

Add canonical machine-readable and human-readable readiness sources for:

- subsystem readiness
- template readiness
- schema/version governance

These records should be compact, evidence-oriented, and tied to explicit rule fields rather than prose-only status updates.

### Layer 2: Readiness rule evaluation

Add rule definitions for:

- subsystem label promotion
- template label promotion
- minimum cross-cutting bars
- version/changelog requirements

This layer determines whether a readiness record is promotable, blocked, or degraded.

### Layer 3: Project audit reporting

Add a project-audit surface that consumes project state plus readiness rules and reports:

- missing required assets or bindings
- unresolved import leftovers
- missing localization/input/audio/accessibility requirements
- template blockers
- schema version mismatches
- export blockers
- readiness overclaims

### Layer 4: Truth reconciliation and CI

Add CI checks that compare:

- readiness records
- docs
- diagnostics labels
- template pages
- schema changes and changelog entries

The goal is to fail drift early instead of relying on periodic doc cleanups.

## Boundaries

### This slice should own

- readiness matrix definitions
- readiness label rules
- project-audit output contracts
- truth-reconciliation checks
- schema-version/changelog governance hooks

### This slice should not own

- implementing every missing template feature
- replacing subsystem-specific specs
- solving all accessibility/audio/input/localization/runtime work directly

Those remain downstream implementation lanes. This governance slice only decides how they become claimable.

## Recommended v1 Components

- `docs/release/RELEASE_READINESS_MATRIX.md`
- `docs/governance/TEMPLATE_READINESS_MATRIX.md`
- `docs/governance/TRUTH_ALIGNMENT_RULES.md`
- `docs/governance/TEMPLATE_LABEL_RULES.md`
- `docs/governance/SUBSYSTEM_STATUS_RULES.md`
- `docs/governance/SCHEMA_CHANGELOG.md`
- `content/schemas/readiness_status.schema.json`
- `tools/ci/check_release_readiness.ps1`
- `tools/docs/check_truth_alignment.ps1`
- `tools/docs/check_template_claims.ps1`
- `tools/docs/check_subsystem_badges.ps1`
- `tools/ci/check_schema_changelog.ps1`
- `tools/audit/urpg_project_audit.cpp`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`

The implementation can stage these incrementally, but the ownership model should stay unified.

## Initial Release Rules

### Subsystem `READY`

A subsystem can be promoted to `READY` only when:

- runtime owner is present
- promised editor surface is present
- promised schema/migration path is present
- diagnostics and tests exist
- documentation and public status language match the evidence

### Template `READY`

A template can be promoted to `READY` only when:

- all required subsystems are `READY`
- required editor flows exist
- project audit can evaluate its required bars
- public docs and canonical docs agree
- minimum cross-cutting bars are satisfied for the template

### Cross-cutting minimum bars

V1 should at least model required bars for:

- accessibility
- audio governance
- input remapping/controller governance
- localization completeness
- schema versioning
- performance budgets

These can begin as rule fields and audit checks before every downstream subsystem is fully mature.

## Testing and Verification

The slice is successful when:

- canonical docs can name subsystem/template status without contradiction
- CI can detect drift between claimed readiness and evidence
- schema changes cannot land silently without version/changelog discipline
- project audit can report governance blockers on a real project snapshot
- downstream roadmap work can promote readiness through explicit rules instead of prose-only status edits

## Acceptance Criteria

- readiness governance is represented in canonical docs and enforceable checks
- template readiness is no longer implied solely by feature composition
- project audit has a concrete contract for export/readiness blocker reporting
- schema-version/changelog requirements are explicit and reviewable
- the governance slice can serve as the first execution checkpoint before larger template/productization lanes
