# Release Readiness Matrix

Status Date: 2026-04-23
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
| `battle_core` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Native battle flow owner, diagnostics, migration contracts, and runtime VFX baseline are landed. Automated evidence was refreshed on 2026-04-24. | Wave 1 closure signoff exists; promotion to `READY` requires human review of residual gaps and an explicit reviewer decision. |
| `save_data_core` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Save runtime, recovery, migration, inspection, and policy editing are landed and validated. Automated evidence was refreshed on 2026-04-24. | Wave 1 closure signoff exists; promotion to `READY` requires human review of residual gaps and an explicit reviewer decision. |
| `compat_bridge_exit` | `PARTIAL` | Yes | N/A | Yes | Yes | Yes | Yes | Compat bridge exit has truthful Phase 2 closure evidence, weekly regression cadence, diagnostics/export parity, WindowCompat color parsing plus segmented gauge-gradient evidence, and an explicit signoff artifact for the bounded import and verification lane. | Compat bridge exit signoff exists; promotion to `READY` requires human review of residual gaps. Curated corpus maintenance, ongoing truth upkeep, and live runtime/backend parity remain mandatory current backlog. |
| `presentation_runtime` | `PARTIAL` | Yes | Yes | Partial | Yes | Yes | Yes | Presentation runtime, bridge, compiled spatial editor panels, end-to-end authoring→runtime proof path, release validation harness, bounded real renderer-backed production-path proof, shell-owned MapScene/MenuScene plus BattleScene actor-cue proof through `EngineShell::tick()`, and a registered standalone `urpg_profile_arena` target are landed. | Broader cross-backend renderer breadth remains adjacent validation backlog under `visual_regression_harness`. Wave 1 closure signoff exists; promotion to `READY` requires human review of residual gaps. |
| `gameplay_ability_framework` | `PARTIAL` | Yes | Yes | Partial | Yes | Yes | Yes | Gameplay ability runtime, inspector/editor surfaces, structured diagnostics snapshot, end-to-end release-grade test, closure signoff artifact, gameplay_ability JSON schema (S24-T01), compat-to-native MZ mapper (S24-T02), live BattleScene activation, AbilityBattleQueue integration (S24-T03/T04), and deterministic AbilityTask WaitInput/WaitEvent/WaitProjectileCollision backends are landed. | Scripted condition evaluator and richer task-driven authored battle/map orchestration remain mandatory current backlog. Wave 1 closure signoff exists; promotion to `READY` requires human review of residual gaps. |
| `governance_foundation` | `PARTIAL` | No | No | Partial | No | Yes | Yes | Breaking-change detection, enhanced readiness checks, template specs, bidirectional readiness/truth drift detection, canonical project schema governance sections for localization/input/exportProfiles, ProjectAudit shape checks for that vocabulary, richer artifact checks across input/localization-bundle/accessibility/audio/character/mod/analytics/performance, a bounded controller-binding runtime/panel plus input governance script/fixture contract, a canonical release-signoff workflow artifact, and structured signoff contracts for the current human-review-gated subsystem lanes are now landed. | Full release-signoff enforcement beyond the current artifact-backed audit, workflow, and drift/date/status checks remains mandatory current backlog. |
| `character_identity` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Character identity runtime, editor creator panel, bounded validation runtime, ECS component/system integration, deterministic spawner, preset-based runtime creator screen, and preview card surface are landed with schema and focused tests. | Character identity validator/governance coverage plus the bounded runtime creator screen/preview card are landed; authored creation rules, runtime-created protagonist persistence, and full layered portrait/field/battle composition remain mandatory current backlog. |
| `achievement_registry` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Achievement registry with progress tracking, unlock conditions, trigger parsing, event-bus auto-unlock, editor panel, vendor-neutral trophy export payload, and `AchievementValidator` with `check_achievement_governance.ps1` CI gate is landed. | Platform-specific achievement backend integration remains mandatory current backlog and explicitly out-of-tree.
| `accessibility_auditor` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Accessibility auditor with missing-label, focus-order, contrast, and navigation rules is landed. Live-ingestion adapters for menu (S16), spatial editor, audio mix panel, and battle inspector (S28-T01) are all landed with tests and governance coverage. | Full WYSIWYG contrast-ratio population from real renderer data remains mandatory current backlog. All four bounded adapter surfaces are now covered.
| `visual_regression_harness` | `PARTIAL` | Yes | No | No | No | Yes | Yes | Visual regression harness with golden file management, diff heatmaps, approval tooling, a file-backed executable harness gate, backend-selectable capture entry points, and one bounded OpenGL-enabled renderer-backed capture lane enforcing primitive, live-widget, full-frame scene, transition-pair, diff-heatmap, shell-owned Map/Menu goldens, and a shell-owned BattleScene actor-cue crop golden in local gates and Gate 1 CI is landed. | Broader backend execution and wider shell-owned state/permutation coverage remain mandatory current backlog. |
| `audio_mix_presets` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Audio mix preset bank with default profiles, category volume mapping, ducking rules, editor panel, and `AudioMixValidator` with `check_audio_governance.ps1` CI gate is landed. | Live audio backend integration beyond the current compat-truth harness remains mandatory current backlog. Additional validator rules (unknown categories, cross-preset duck conflicts) remain mandatory current backlog.
| `export_validator` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Platform-specific export validator with per-target requirement checking, fixture-backed validation coverage, a now-explicit split between packager preflight readiness and post-export artifact validation, diagnostics coverage for divergent preflight/post-export states, PowerShell CI script with JSON output, canonical `urpg_pack_cli` entrypoint at `tools/pack/pack_cli.cpp`, one bounded deterministic `data.pck` manifest bundle, bounded real Windows/Web/Linux/macOS bootstrap paths, bundle-level lightweight RLE-plus-XOR payload protection plus keyed integrity-tag validation, configured project-asset auto-discovery manifest staging, bounded repo-owned content bundling for readiness/schema/level-library roots, governed manifest-driven promoted-asset staging support, keyed SHA-256 bundle-signature validation, and structured bundle-summary JSON reporting are landed. `docs/specs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md` records the missing runtime/load-time enforcement contract. | Broader external discovery, full native packaging/signing/notarization, temp-file-plus-atomic-rename bundle publication, and runtime-side signature enforcement remain mandatory current backlog. |
| `mod_registry` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Mod registry with manifest registration, dependency resolution, load-order topology, bounded validation runtime, and editor manager panel is landed. | Mod validator/governance coverage is landed; live mod loading, sandboxed script execution, and mod-store integration remain mandatory current backlog. |
| `analytics_dispatcher` | `PARTIAL` | Yes | Yes | Yes | Yes | Yes | Yes | Opt-in analytics dispatcher with deterministic tick counters, event buffering, circular drop, editor panel, and bounded validator/governance coverage is landed. | Analytics validator/governance coverage is landed; telemetry upload pipeline, session aggregation, and privacy audit workflow remain mandatory current backlog. |

## P2-001 Editor Stub Audit

Audit Date: 2026-04-26

The P2-001 editor sweep searched for empty render functions, placeholder language, mock/stub labels, and silent empty JSON snapshots under `editor/`. Verified user-facing silent empty states were converted to explicit disabled-state snapshots with an owner, reason, and unlock condition.

| Panel | Verified Gap | Resolution | Regression Evidence |
| --- | --- | --- | --- |
| `AchievementPanel` | Render without `AchievementRegistry` returned `{}`. | Emits `status=disabled`, owner `editor/achievement`, and binding unlock condition. | `tests/unit/test_achievement_panel.cpp` |
| `AccessibilityPanel` | Render without `AccessibilityAuditor` returned `{}`. | Emits `status=disabled`, owner `editor/accessibility`, and binding unlock condition. | `tests/unit/test_accessibility_panel.cpp` |
| `CharacterCreatorPanel` | Render without `CharacterCreatorModel` returned `{}`. | Emits `status=disabled`, owner `editor/character`, and binding unlock condition. | `tests/unit/test_character_creator_panel.cpp` |
| `ExportDiagnosticsPanel` | Render without `ExportConfig` returned `{}`. | Emits `status=disabled`, owner `editor/export`, `readyToExport=false`, and config unlock condition. | `tests/unit/test_export_diagnostics_panel.cpp` |
| `PerfDiagnosticsPanel` | Render without model/profiler returned `{}`. | Emits separate disabled states for missing model and missing profiler. | `tests/unit/test_perf_diagnostics_panel.cpp` |
| `NewProjectWizardPanel` | Render without `NewProjectWizardModel` returned `{}`. | Emits `status=disabled`, owner `editor/project`, and binding unlock condition. | `tests/unit/test_new_project_wizard_panel.cpp` |

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

## Legal And Notices Snapshot

Status Date: 2026-04-26

| Area | Status | Evidence | Remaining Gate |
| --- | --- | --- | --- |
| Project license | `PARTIAL` | Root `LICENSE` is present and installed by the native app layout. | Confirm final binary distribution terms during legal signoff. |
| Third-party notices | `PARTIAL` | Root `THIRD_PARTY_NOTICES.md` inventories shipped install components, fetched dependencies, governed manifests, and repository-only intake/reference paths. | Qualified legal reviewer must verify license sufficiency and exact upstream notice text. |
| Credits | `PARTIAL` | Root `CREDITS.md` records URPG project credits, dependency credits, and promoted proof-lane attribution status. | Confirm final public attribution requirements for any promoted asset or plugin included in a release package. |
| EULA | `BLOCKED` | Root `EULA.md` exists as an internal-only placeholder. | Replace with or approve production EULA through qualified legal review before external distribution. |
| Privacy policy | `PARTIAL` | Root `PRIVACY_POLICY.md` matches the opt-in analytics implementation and no-default-upload behavior. | Replace internal contact routing and complete qualified legal/privacy review before public release. |
