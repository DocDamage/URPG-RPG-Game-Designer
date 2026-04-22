# Release Readiness Matrix

Status Date: 2026-04-21  
Authority: canonical subsystem readiness reference for release-facing status labels

This matrix governs whether a subsystem can be described as `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, or `PLANNED`.

This document does not replace subsystem specs. It summarizes whether a subsystem is currently safe to expose in release-facing status language based on evidence already recorded in the canonical docs, tests, and build graph.

## Status Values

| Status | Meaning |
| --- | --- |
| `READY` | Required runtime/editor/schema/migration/diagnostics/test evidence exists for the scope being claimed. |
| `PARTIAL` | Meaningful implementation exists, but one or more required release bars are still missing. |
| `EXPERIMENTAL` | A documented first-class lane has early implementation or validation anchors, but it is not yet safe for release-facing product claims. |
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
| `battle_core` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Native battle flow owner, diagnostics, migration contracts, and runtime VFX baseline are landed. | Wave 1 closure signoff exists; promotion to `READY` requires human review of residual gaps. |
| `save_data_core` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Save runtime, recovery, migration, inspection, and policy editing are landed and validated. | Wave 1 closure signoff exists; promotion to `READY` requires human review of residual gaps. |
| `compat_bridge_exit` | `PARTIAL` | Yes | N/A | Yes | Yes | Yes | Yes | Compat bridge exit has truthful Phase 2 closure evidence, weekly regression cadence, diagnostics/export parity, and an explicit signoff artifact for the bounded import and verification lane. | Compat bridge exit signoff exists; promotion to `READY` requires human review of residual gaps. Curated corpus maintenance and ongoing truth upkeep remain future work. |
| `presentation_runtime` | `PARTIAL` | Yes | Yes | Partial | Yes | Yes | Yes | Presentation runtime, bridge, compiled spatial editor panels, end-to-end authoring→runtime proof path, and release validation harness are landed. | Real renderer backend integration, profile arena hot-reload, and broader polish governance remain future work. |
| `gameplay_ability_framework` | `PARTIAL` | Yes | Yes | Partial | Yes | Yes | Yes | Gameplay ability runtime, inspector/editor surfaces, structured diagnostics snapshot, end-to-end release-grade test, and closure signoff artifact are landed. | Schema contracts, compat-to-native migration mapping, and real gameplay loop integration remain future work. Promotion to `READY` requires human review of residual gaps. |
| `governance_foundation` | `PARTIAL` | No | No | Partial | No | Yes | Yes | Breaking-change detection, enhanced readiness checks, template specs, bidirectional readiness/truth drift detection, richer project-audit artifact checks, a canonical release-signoff workflow artifact, and structured signoff contracts for the current human-review-gated subsystem lanes are now landed. | Full release-signoff enforcement beyond the current artifact-backed audit, workflow, and drift/date/status checks remains future work. |
| `character_identity` | `PARTIAL` | Yes | Yes | Yes | No | Yes | Yes | Character identity runtime, editor creator panel, ECS component/system integration, and deterministic spawner are landed with schema and focused tests. | Full Create-a-Character workflow (runtime character creation UI and appearance preview pipeline) remains future work. |
| `achievement_registry` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Achievement registry with progress tracking, unlock conditions, trigger parsing, event-bus auto-unlock, editor panel, and `AchievementValidator` with `check_achievement_governance.ps1` CI gate is landed. | Trophy export pipeline and platform-specific achievement backend integration remain future work.
| `accessibility_auditor` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Accessibility auditor with missing-label, focus-order, contrast, and navigation rules is landed. Menu inspector live-ingestion adapter and `check_accessibility_governance.ps1` CI gate are landed. | Menu adapter is the first bounded editor surface; spatial, audio, and battle adapters remain future work. Full surface coverage beyond menu requires additional dedicated adapters.
| `visual_regression_harness` | `PARTIAL` | Yes | No | No | No | Yes | Yes | Visual regression harness with golden file management, diff heatmaps, approval tooling, and a file-backed executable harness gate is landed. | Integration with real render output capture and CI golden gate remain future work. |
| `audio_mix_presets` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Audio mix preset bank with default profiles, category volume mapping, ducking rules, editor panel, and `AudioMixValidator` with `check_audio_governance.ps1` CI gate is landed. | Live audio backend integration beyond the current compat-truth harness remains future work. Additional validator rules (unknown categories, cross-preset duck conflicts) remain future work.
| `export_validator` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Platform-specific export validator with per-target requirement checking, synthetic pack-and-validate pipeline, PowerShell CI script with JSON output, and diagnostics panel with post-export artifact surfacing is landed. | Real asset bundling and binary synthesis remain stubbed; synthetic emitted tree is used for validation. |
| `mod_registry` | `PARTIAL` | Yes | Yes | Yes | No | Yes | Yes | Mod registry with manifest registration, dependency resolution, load-order topology, and editor manager panel is landed. | Live mod loading, sandboxed script execution, and mod-store integration remain future work. |
| `analytics_dispatcher` | `PARTIAL` | Yes | Yes | Yes | No | Yes | Yes | Opt-in analytics dispatcher with deterministic tick counters, event buffering, circular drop, and editor panel is landed. | Telemetry upload pipeline, session aggregation, and privacy audit workflow remain future work. |

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
