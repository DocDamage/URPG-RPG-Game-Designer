# Template Label Rules

Status Date: 2026-04-28

This document defines when a template may be labeled `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, or `PLANNED`.

## `READY`

A template may be labeled `READY` only when:

1. Every required subsystem is `READY`.
2. Required editor or setup flows exist for the template scope.
3. The project audit can detect missing prerequisites for the template.
4. Canonical docs and template-facing docs agree on the label.
5. Required cross-cutting bars for accessibility, audio, input, localization, and performance are satisfied for the claimed scope.

## `PARTIAL`

Use `PARTIAL` when:

- meaningful support exists today
- the template is usable within a bounded scope
- one or more required subsystems or cross-cutting bars are still missing for a `READY` claim

## `EXPERIMENTAL`

Use `EXPERIMENTAL` when:

- the template is a documented first-class lane with real implementation or validation seed work
- the engine can already demonstrate some of the template shape
- release-facing productization, auditability, or cross-cutting bars are not yet in place

## `BLOCKED`

Use `BLOCKED` when:

- a required subsystem is missing
- a prerequisite runtime owner does not exist
- a contradiction in current evidence prevents a safe claim

## `PLANNED`

Use `PLANNED` when:

- the roadmap names the template
- the repo does not yet have enough evidence for a support claim beyond intent

## Label Safety Rules

- Never promote a template because the engine is theoretically composable into that shape.
- Never promote a template because one marketing or planning doc uses optimistic language.
- When in doubt, choose the more conservative label.
