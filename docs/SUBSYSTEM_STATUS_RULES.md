# Subsystem Status Rules

Status Date: 2026-05-01

This document defines when a subsystem may be labeled `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, or `PLANNED`.

## `READY`

A subsystem may be labeled `READY` only when the claimed scope has:

1. a named native runtime owner where runtime ownership is part of the claim
2. any promised editor or diagnostics surface present in the build graph
3. any promised schema or migration contract present and documented
4. focused tests or validation evidence for the claimed scope
5. canonical docs aligned with that evidence

## `PARTIAL`

Use `PARTIAL` when:

- the subsystem is real and materially useful
- at least one required release bar for the claimed scope is still missing

## `EXPERIMENTAL`

Use `EXPERIMENTAL` when:

- there is meaningful implementation or validation seed work for a documented first-class lane
- the subsystem is not yet safe for broad product claims

## `BLOCKED`

Use `BLOCKED` when:

- a prerequisite owner, build registration, or contract is missing
- promotion would contradict the current evidence

## `PLANNED`

Use `PLANNED` when:

- the subsystem is on the roadmap
- no release-facing implementation claim should be made yet

## Label Safety Rules

- A passing focused test lane does not automatically imply subsystem-wide `READY`.
- A design doc or plan is not evidence of readiness by itself.
- A subsystem can be `READY` for a bounded scope while future expansions remain `PLANNED`.
- If a subsystem's readiness language says promotion still requires human review, a signoff artifact must exist and stay explicitly non-promoting until that review happens.
