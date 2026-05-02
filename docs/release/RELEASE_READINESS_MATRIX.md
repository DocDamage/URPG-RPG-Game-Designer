# Release Readiness Matrix

Status Date: 2026-05-01
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
| `battle_core` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Native battle flow owner, diagnostics, migration contracts, runtime VFX baseline, and approved Wave 1 closure signoff are landed. | None recorded for claimed Wave 1 scope. |
| `save_data_core` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Save runtime, recovery, migration, inspection, policy editing, RPG Maker save import, and approved Wave 1 closure signoff are landed and validated. | None recorded for claimed Wave 1 scope. |
| `compat_bridge_exit` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Compat bridge exit has approved closure evidence for the bounded import, validation, migration, diagnostics, and verification scope; public compat registries under `runtimes/compat_js` have no partial method-status markers, and live QuickJS plugin execution now projects successful command evidence through diagnostics export. | None recorded for claimed bridge-exit scope; ongoing corpus maintenance remains normal maintenance work. |
| `native_level_builder` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | `LevelBuilderWorkspace` owns grid-part build/validate/playtest/package workflows, save/load/export commands, undo/redo, actionable diagnostics, diagnostic focus, spawn/objective authoring, readiness evidence commands, package readiness, certified export preflight, and supporting spatial handoff. The grid-part runtime/data model includes schemas, catalog/document/serializer/runtime compiler, reachability, dependency graph, package governance, runtime state, CTest unit/integration lanes, focused editor tests, app-shell top-level navigation, and smoke coverage for `level_builder`. | Public product release remains governed by app-level release/tagging gates. |
| `presentation_runtime` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Presentation runtime, bridge, compiled spatial editor panels, end-to-end authoring→runtime proof path, release validation harness, bounded real renderer-backed production-path proof, shell-owned MapScene/MenuScene, runtime title diagnostics, runtime options high-contrast, BattleScene actor-cue proof through `EngineShell::tick()`, deterministic non-OpenGL headless/reference capture breadth, phase-one shell-owned Map/Battle/Title/Options/LevelBuilder permutation goldens, and a registered standalone `urpg_profile_arena` target are landed. | None recorded for the claimed phase-one presentation scope; `ctest --preset dev-snapshot --output-on-failure` and `.\tools\ci\run_presentation_gate.ps1` passed. |
| `gameplay_ability_framework` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Gameplay ability runtime, inspector/editor surfaces, structured diagnostics snapshot, end-to-end release-grade tests, closure signoff artifact, gameplay_ability JSON schema (S24-T01), compat-to-native MZ mapper (S24-T02), live BattleScene activation, AbilityBattleQueue integration (S24-T03/T04), deterministic AbilityTask WaitInput/WaitEvent/WaitProjectileCollision backends, bounded active-condition evaluation, authored battle/map task-graph runtime sequencing, branch/parallel/cancel diagnostics, script-task rejection, schema examples, and `AbilityOrchestrationPanel` author/preview/save/load/apply/revert workflow are landed. | None recorded for the claimed phase-two gameplay ability scope. |
| `governance_foundation` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Governance foundation now enforces release-signoff completeness for `READY` subsystem claims through ProjectAudit, release-readiness checks, truth reconciliation, canonical signoff artifacts, reviewer/date/verification/result/docs-alignment metadata, and diagnostics parity. | None recorded for the claimed Phase 3 governance scope; future release bars or tagging policy changes require new evidence. |
| `character_identity` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Character identity runtime, editor creator panel, character creation rules, bounded validation runtime, ECS component/system integration, deterministic spawner, preset-based runtime creator screen, preview card surface, runtime-created protagonist save/load persistence, layered portrait/field/battle composition, promoted asset-library appearance-part selection, creator-supplied portrait/field/battle import rows, governed promotion paths, attribution diagnostics, and part-library accept/reject/archive/assign management snapshots are landed with schema and focused tests. | None recorded for the claimed Phase 4 character identity scope; final-quality portrait art and cohesive character art direction remain asset-content backlog, not unfinished engine code. |
| `achievement_registry` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Achievement registry with progress tracking, unlock conditions, trigger parsing, event-bus auto-unlock, editor panel, vendor-neutral trophy export payload, platform backend synchronization, packaged `AchievementPlatformProfile` application, shared provider-profile status validation, `AchievementPanel` external-service diagnostics, and `AchievementValidator` with `check_achievement_governance.ps1` CI gate is landed. | None recorded for the claimed Phase 7 achievement engine scope; proprietary store SDK credentials remain project configuration. |
| `accessibility_auditor` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Accessibility auditor with missing-label, focus-order, contrast, navigation rules, live-ingestion adapters, renderer-derived WYSIWYG contrast ingestion over `FrameRenderCommand` text/rect surfaces, and release top-level panel contrast coverage evidence is landed. | None recorded for the claimed Phase 5 accessibility scope; `ctest --preset dev-all -R "accessibility|Accessibility" --output-on-failure` and `.\tools\ci\check_accessibility_governance.ps1` passed. |
| `visual_regression_harness` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Visual regression harness with golden file management, diff heatmaps, approval tooling, a file-backed executable harness gate, backend-selectable capture entry points, OpenGL capture, deterministic HeadlessRenderer/reference capture, backend metadata with stable hashes, command-stream parity assertions, and phase-one shell-owned MapScene, BattleScene, RuntimeTitleScene, RuntimeOptionsScene, and Level Builder permutation goldens is landed. | None recorded for the claimed phase-one visual scope; `ctest --preset dev-snapshot --output-on-failure` and `.\tools\ci\run_presentation_gate.ps1` passed. |
| `audio_mix_presets` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Audio mix preset bank with default profiles, category volume mapping, ducking rules, editor panel, `AudioMixValidator` governance, live `AudioCore` backend smoke diagnostics, deterministic backend/device matrix evidence, muted release fallback diagnostics, and `AudioMixPanel` matrix exposure is landed. | None recorded for the claimed Phase 5 audio scope; `ctest --preset dev-all -R "audio|Audio" --output-on-failure` and `.\tools\ci\check_audio_governance.ps1` passed. |
| `export_validator` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Platform-specific export validator with per-target requirement checking, fixture-backed validation coverage, a now-explicit split between packager preflight readiness and post-export artifact validation, diagnostics coverage for divergent preflight/post-export states, PowerShell CI script with JSON output, canonical `urpg_pack_cli` entrypoint at `tools/pack/pack_cli.cpp`, one bounded deterministic `data.pck` manifest bundle, bounded real Windows/Web/Linux/macOS bootstrap paths, bundle-level lightweight RLE-plus-XOR payload protection plus keyed integrity-tag validation, configured project-asset auto-discovery manifest staging, raw/source/external-store discovery rejection diagnostics, missing attribution and unresolved LFS pointer enforcement, bounded repo-owned content bundling for readiness/schema/level-library roots, governed manifest-driven promoted-asset staging support, keyed SHA-256 bundle-signature validation, runtime/load-time bundle rejection, release-profile signing/notarization/artifact-policy enforcement, temp-file-plus-atomic-rename publication, and structured bundle-summary JSON reporting are landed. | None recorded for the claimed Phase 6 export validator scope; production signing certificates, notarization accounts, and platform-store publication remain project configuration. |
| `mod_registry` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Mod registry with manifest registration, dependency resolution, load-order topology, bounded validation runtime, sandboxed QuickJS activation, deterministic reload, polling hot-load, editor manager panel hot-load event log, local mod-store catalog install, `ModMarketplaceProviderProfile` validation, and `ModManagerPanel` provider diagnostics is landed. | None recorded for the claimed Phase 7 mod engine scope; external marketplace services, payments, reviews, and proprietary platform publishing remain project configuration. |
| `analytics_dispatcher` | `READY` | Yes | Yes | Yes | Yes | Yes | Yes | Opt-in analytics dispatcher with deterministic tick counters, event buffering, circular drop, local and remote telemetry upload, reviewed endpoint profiles, editor panel, bounded validator/governance coverage, `AnalyticsEndpointProfile` status validation, and `AnalyticsPanel` provider diagnostics is landed. | None recorded for the claimed Phase 7 analytics engine scope; qualified public privacy/legal approval remains separate from engine readiness and is release-owner-waived for the current distribution decision. |

## P2-001 Editor Stub Audit

Audit Date: 2026-04-26

The P2-001 editor sweep searched for empty render functions, placeholder language, mock/stub labels, and silent empty JSON snapshots under `editor/`. Verified user-facing silent empty states were converted to explicit disabled-state snapshots with an owner, reason, and unlock condition.

Follow-up release-surface governance is enforced by `tests/unit/test_editor_panel_registry.cpp`: release top-level editor owner files are scanned for placeholder, mock, fake, stub, TODO/FIXME, and dev-bootstrap terms. The only allowed release-owner hit is the Diagnostics compatibility `stubCount` metric, which is an explicit report bucket rather than production behavior. Broader non-release hits remain explicitly labeled as diagnostics, incubator tooling, headless/mock backends, compatibility status, or dev-bootstrap export paths.

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

Status Date: 2026-04-30

| Area | Status | Evidence | Remaining Gate |
| --- | --- | --- | --- |
| Project license | `VERIFIED` | Root `LICENSE` is present, installed by the native app layout, and accepted under release-owner waiver. | None for current owner-waived distribution scope. |
| Third-party notices | `VERIFIED` | Root `THIRD_PARTY_NOTICES.md` inventories shipped install components, fetched dependencies, governed manifests, and repository-only intake/reference paths; release owner accepts notice sufficiency under waiver. | Do not add raw/vendor/source asset packs without source-specific license and attribution review. |
| Credits | `VERIFIED` | Root `CREDITS.md` records URPG project credits, dependency credits, and promoted proof-lane attribution status; release owner certifies shipped paid/licensed assets are usable in distributed games. | Keep attribution updated when adding shipped assets or plugins. |
| EULA | `VERIFIED` | Root `EULA.md` exists and is accepted for public distribution by release-owner waiver. | Qualified legal counsel has not approved this text; waiver remains owner-risk acceptance. |
| Privacy policy | `VERIFIED` | Root `PRIVACY_POLICY.md` matches the opt-in analytics implementation and no-default-upload behavior and is accepted by release-owner waiver. | Keep public contact/support routing current before tagging a final release. |

## LFS Hydration Snapshot

Status Date: 2026-04-30

| Area | Status | Evidence | Remaining Gate |
| --- | --- | --- | --- |
| Fresh clone LFS hydration | `VERIFIED` | A no-smudge fresh clone of `codex/release-surface-p0` on 2026-04-30 reported `git lfs ls-files` count `0`; release-required asset validation, package dry run, configure, audit build, and release-readiness checks passed in that clone. | Do not reintroduce release-required LFS pointers; keep raw/source intake ignored unless a curated asset is promoted through manifests. |
| Current checkout LFS footprint | `VERIFIED` | The 2026-04-30 baggage passes removed current Git tracking for `imports/raw/more_assets/`, `imports/raw/itch_assets/loose/`, broad `imports/raw/third_party_assets/itch-assets/`, source-only RPG Maker/Aseprite intake, `imports/root-drop/archives/`, and the remaining root-drop archive pointers while preserving local working files as ignored raw quarantine. Curated RPG Maker plugin drop-ins, plugin release reports, and Hugging Face fixtures remain tracked because gates/tests reference them. Current branch `git lfs ls-files` count is `0`. | Prior branch history may still contain old LFS objects; avoid depending on old source/vendor history for release work. |
