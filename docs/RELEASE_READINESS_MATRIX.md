# Release Readiness Matrix

Status Date: 2026-04-20  
Authority: canonical subsystem readiness reference for release-facing status labels

This matrix governs whether a subsystem can be described as `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, or `PLANNED`.

This document does not replace subsystem specs. It summarizes whether a subsystem is currently safe to expose in release-facing status language based on evidence already recorded in the canonical docs, tests, and build graph.

## Status Values

| Status | Meaning |
| --- | --- |
| `READY` | Required runtime/editor/schema/migration/diagnostics/test evidence exists for the scope being claimed. |
| `PARTIAL` | Meaningful implementation exists, but one or more required release bars are still missing. |
| `EXPERIMENTAL` | Usable for exploration or bounded demos, but not safe for product claims. |
| `BLOCKED` | Cannot be promoted because a prerequisite owner, contract, or validation lane is missing. |
| `PLANNED` | Explicit roadmap scope exists, but there is no readiness-grade implementation claim yet. |

## Evidence Fields

Every subsystem row is evaluated against these fields:

| Field | Requirement for `READY` |
| --- | --- |
| Runtime Owner | A native owner exists for the claimed scope. |
| Editor Surface | Required when the subsystem claims authoring or inspection workflows. |
| Schema / Migration | Required when the subsystem persists authorable or importable data. |
| Diagnostics | Required when the subsystem claims release-facing observability or diagnostics support. |
| Tests / Validation | Focused tests or validation lanes cover the claimed scope. |
| Docs Aligned | Canonical docs do not overclaim the subsystem status. |

## Current Matrix

| Subsystem | Status | Runtime Owner | Editor Surface | Schema / Migration | Diagnostics | Tests / Validation | Docs Aligned | Evidence Summary | Main Gaps |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `ui_menu_core` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Wave 1 closure recorded in canonical roadmap/status docs with build/test registration and round-trip coverage. | None recorded for claimed scope. |
| `message_text_core` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Wave 1 closure recorded with native runtime, preview/inspector workflows, schema/migration, and renderer handoff evidence. | None recorded for claimed scope. |
| `battle_core` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Native battle flow owner, diagnostics, migration contracts, and runtime VFX baseline are landed. | Wave 1 closure is not yet complete in the canonical roadmap. |
| `save_data_core` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Save runtime, recovery, migration, inspection, and policy editing are landed and validated. | Compat import/migration completion and release-grade closure are still open. |
| `compat_bridge_exit` | `PARTIAL` | Yes | N/A | Yes | Yes | Yes | Yes | Phase 2 closure is complete and weekly compat cadence exists. | Remaining work is exit sign-off, corpus maintenance, and truth upkeep. |
| `presentation_runtime` | `PARTIAL` | Yes | Partial | Partial | Partial | Yes | Yes | Registered runtime/release-validation lanes and compiled spatial panels are in tree. | Product-wide readiness matrix, richer authoring proof, and broader polish governance are still missing. |
| `gameplay_ability_framework` | `PARTIAL` | Yes | Yes | Partial | Partial | Yes | Yes | Core runtime pieces and inspector surfaces are landed. | Release-facing closure and product bars are not yet formally signed off. |
| `governance_foundation` | `PLANNED` | No | No | Partial | No | No | Yes | Canonical roadmap/status now explicitly track the lane. | Matrix records, CI checks, audit tooling, and diagnostics entry points are still being introduced. |

## Promotion Rules

A subsystem may be promoted to `READY` only when:

1. Its claimed scope has a native runtime owner.
2. Any promised editor surface exists in the build graph.
3. Any promised schema or migration contract exists and is documented.
4. Diagnostics and validation exist where claimed.
5. Canonical docs, public-facing docs, and readiness records agree.

If any of those conditions fail, the row must remain below `READY`.

## Notes

- `READY` applies only to the scope actually evidenced, not to every imaginable extension of the subsystem.
- A subsystem can be `READY` while adjacent template/productization lanes remain `PARTIAL`.
- This matrix is intended to work alongside `content/readiness/readiness_status.json` and the readiness-rule checks under `tools/ci/` and `tools/docs/`.
