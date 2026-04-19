# Technical Debt Remediation Plan

> **Document status:** Sixth-pass revision — canonical remediation hub as of 2026-04-18.
> Incorporates stale-state debt, placeholder export-surface debt, documentation/test drift findings, the external-repository intake program defined in [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md), and the private-use asset intake program defined in [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md).
> This revision also absorbs the newly added PGMMV and native-absorption planning inputs into the remediation program so roadmap expansion, truthfulness, and execution governance share one canonical hub.

---

## Table of Contents

1. [Purpose](#purpose)
2. [Audit Method](#audit-method)
3. [Priority Legend](#priority-legend)
4. [Finding Status at a Glance](#finding-status-at-a-glance)
5. [Current State Summary](#current-state-summary)
6. [Priority Findings](#priority-findings)
   - [P0-01 — Build Integrity: Active Compile Blockers](#p0-01--build-integrity-active-compile-blockers)
   - [P1-01 — QuickJS Layer Is A Stub Kernel, Not A Runtime](#p1-01--quickjs-layer-is-a-stub-kernel-not-a-runtime)
   - [P1-02 — Compat Status Inflation Across Multiple Subsystems](#p1-02--compat-status-inflation-across-multiple-subsystems)
   - [P1-03 — Audio SE Channel Lifetime Leak](#p1-03--audio-se-channel-lifetime-leak)
   - [P1-04 — Battle Turn-Condition Correctness Bug](#p1-04--battle-turn-condition-correctness-bug)
   - [P2-01 — Diagnostics Workspace Reports More Than It Renders](#p2-01--diagnostics-workspace-reports-more-than-it-renders)
   - [P2-02 — Diagnostics Runtime Binding Is One-Way and Leaves Stale State](#p2-02--diagnostics-runtime-binding-is-one-way-and-leaves-stale-state)
   - [P2-03 — Audio and Migration Diagnostics Are Scaffolding, Not Product](#p2-03--audio-and-migration-diagnostics-are-scaffolding-not-product)
   - [P2-04 — Test and Build Registration Drift](#p2-04--test-and-build-registration-drift)
   - [P2-05 — Menu Serialization Is Import-Only, Not Round-Trippable](#p2-05--menu-serialization-is-import-only-not-round-trippable)
   - [P2-06 — Presentation and Spatial Claims Exceed Build Graph Evidence](#p2-06--presentation-and-spatial-claims-exceed-build-graph-evidence)
   - [P2-07 — Native Plugin API Is A Mock Bridge, Not A Real Integration](#p2-07--native-plugin-api-is-a-mock-bridge-not-a-real-integration)
   - [P2-08 — Cloud Sync Documentation Overstates Implementation Readiness](#p2-08--cloud-sync-documentation-overstates-implementation-readiness)
   - [P2-09 — Async Plugin Callbacks Lack Thread-Affinity Guarantees](#p2-09--async-plugin-callbacks-lack-thread-affinity-guarantees)
   - [P2-10 — Native-Absorption And PGMMV Planning Drift Sits Outside Canonical Remediation Governance](#p2-10--native-absorption-and-pgmmv-planning-drift-sits-outside-canonical-remediation-governance)
   - [P3-01 — Hidden Ownership Shortcuts in Runtime Services](#p3-01--hidden-ownership-shortcuts-in-runtime-services)
   - [P3-02 — External Repository Intake Needs Canonical Governance](#p3-02--external-repository-intake-needs-canonical-governance)
   - [P3-03 — Private-Use Asset Intake Needs Canonical Governance](#p3-03--private-use-asset-intake-needs-canonical-governance)
7. [Findings That Change Prioritization](#findings-that-change-prioritization)
8. [Remediation Principles](#remediation-principles)
9. [Phase Plan](#phase-plan)
   - [Phase 0 — Restore Build Integrity and Baseline Truth](#phase-0--restore-build-integrity-and-baseline-truth)
   - [Phase 1 — Make Status Labels and Surface Claims Honest](#phase-1--make-status-labels-and-surface-claims-honest)
   - [Phase 2 — Fix Correctness and Runtime Closure in the Highest-Value Paths](#phase-2--fix-correctness-and-runtime-closure-in-the-highest-value-paths)
   - [Phase 3 — Make Editor and Diagnostics Surfaces Truthful, Stateful, and Usable](#phase-3--make-editor-and-diagnostics-surfaces-truthful-stateful-and-usable)
   - [Phase 4 — Reconcile Build Graph, Tests, Docs, Exported Surfaces, and Canonical Planning](#phase-4--reconcile-build-graph-tests-docs-exported-surfaces-and-canonical-planning)
   - [Phase 5 — Harden Ownership, Concurrency, and Performance](#phase-5--harden-ownership-concurrency-and-performance)
10. [Phase Summary and Dependencies](#phase-summary-and-dependencies)
11. [Verification Plan](#verification-plan)
12. [Risk Register](#risk-register)
13. [Documentation Alignment Tasks](#documentation-alignment-tasks)
14. [Documentation File Tree](#documentation-file-tree)
15. [Ownership Matrix](#ownership-matrix)
16. [Definition of Done](#definition-of-done)
17. [Recommended Execution Order](#recommended-execution-order)
18. [Change Log](#change-log)

---

## Purpose

This plan is the single source of truth for technical debt remediation across the URPG engine. It exists to:

- Restore a reliable green build and test baseline.
- Make compat, editor, and completion claims truthful.
- Finish wiring features and panels that already have models, docs, or specs behind them.
- Reduce drift between source, build graph, tests, exported APIs, and documentation.
- Prioritize debt that affects **correctness, reachability, and trust** over cosmetic placeholder cleanup.
- Ensure external repositories are evaluated through a governed intake path before they influence URPG runtime, fixtures, tooling, or asset lanes.
- Ensure private-use asset acceleration happens through a governed provenance-preserving intake path so editor/runtime realism improves without reintroducing licensing, source-traceability, or content-root drift.
- Ensure newly introduced roadmap scope - especially PGMMV intake, native absorption, HD-2D / 2.5D, authoring UX, accessibility, localization, and product-glue tracks - enters execution through the same truthfulness and Definition-of-Done gates as every other remediation lane.

---

## Audit Method

This revision reflects five audit passes across:

- Build blockers and active compile paths.
- `TODO`, stub, and placeholder-heavy runtime and editor code.
- Build and test registration in [CMakeLists.txt](../CMakeLists.txt).
- Status-label inflation across compat surfaces.
- UI surfaces that exist in summaries/models but are not actually rendered or kept fresh.
- Exported, plugin, and cloud-facing interfaces that claim more than they currently route.
- Documentation that claims completion ahead of build, test, or product evidence.
- Newly added roadmap and support plans whose scope affects execution ordering, status vocabulary, and release claims.

---

## Priority Legend

| Level | Meaning |
|-------|---------|
| **P0** | Build is broken or spec-safe path does not compile. All downstream work is speculative until closed. |
| **P1** | Incorrect runtime behavior or status labels that actively mislead. Causes bugs in shipped code or misinforms planning. |
| **P2** | Significant gaps between claims and reality. Affects trust, diagnostics accuracy, or long-term maintainability. |
| **P3** | Structural brittleness. Does not break behavior today but will resist change, testing, or performance work. |

---

## Finding Status at a Glance

Quick-reference table for current finding status. Use this to assess open work without reading each finding in full.

| ID | Title | Status | Phase |
|----|-------|--------|-------|
| P0-01 | Build Integrity: Active Compile Blockers | ⚠ Open | Phase 0 |
| P1-01 | QuickJS Layer Is A Stub Kernel, Not A Runtime | ✅ Remediated (Path B) | Phase 2 WS 2.1 |
| P1-02 | Compat Status Inflation Across Multiple Subsystems | ⚠ Partially Remediated | Phase 1 |
| P1-03 | Audio SE Channel Lifetime Leak | ⚠ Partially Remediated | Phase 2 WS 2.3 |
| P1-04 | Battle Turn-Condition Correctness Bug | ✅ Remediated | Phase 2 WS 2.2 |
| P2-01 | Diagnostics Workspace Reports More Than It Renders | ✅ Remediated | Phase 3 |
| P2-02 | Diagnostics Runtime Binding Is One-Way and Leaves Stale State | ✅ Remediated | Phase 3 |
| P2-03 | Audio and Migration Diagnostics Are Scaffolding, Not Product | ⚠ Partially Remediated | Phase 3 |
| P2-04 | Test and Build Registration Drift | ✅ Remediated | Phase 4 |
| P2-05 | Menu Serialization Is Import-Only, Not Round-Trippable | ✅ Remediated | Phase 2 WS 2.5 |
| P2-06 | Presentation and Spatial Claims Exceed Build Graph Evidence | ✅ Remediated (Incubating declared) | Phase 4 |
| P2-07 | Native Plugin API Is A Mock Bridge, Not A Real Integration | ✅ Remediated (stub-labeled) | Phase 1 / Phase 5 |
| P2-08 | Cloud Sync Documentation Overstates Implementation Readiness | ✅ Remediated | Phase 1 |
| P2-09 | Async Plugin Callbacks Lack Thread-Affinity Guarantees | ✅ Remediated | Phase 5 |
| P2-10 | Native-Absorption and PGMMV Planning Drift Sits Outside Canonical Remediation Governance | ⚠ Partially Remediated | Phase 4.3 |
| P3-01 | Hidden Ownership Shortcuts in Runtime Services | ✅ Remediated | Phase 5 |
| P3-02 | External Repository Intake Needs Canonical Governance | ⚠ Partially Remediated | Phase 4.1 |
| P3-03 | Private-Use Asset Intake Needs Canonical Governance | ⚠ Partially Remediated | Phase 4.2 |

**Legend:** ✅ Remediated — work is complete and verified. ⚠ Open or Partially Remediated — work is in progress or not yet started.


---

## Current State Summary

The debt picture is broader than "unfinished features." Four distinct categories are present simultaneously:

**Stubbed subsystems described as complete.** [quickjs_runtime.cpp](../runtimes/compat_js/quickjs_runtime.cpp) and key slices of [data_manager.cpp](../runtimes/compat_js/data_manager.cpp) and [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp) were the highest-risk examples at audit time. [window_compat.cpp](../runtimes/compat_js/window_compat.cpp) has since moved materially beyond the earlier placeholder/input-stub state, but it still remains a compat-layer partial implementation rather than a production-native UI runtime.

**Partially functional subsystems that are oversold.** [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp) is now honestly labeled `PARTIAL`, but it still remains a deterministic compat harness rather than a live audio backend and must continue to be described that way.

**Editor scaffolding without a rendered product surface.** The Audio, Migration Wizard, and Event Authority panels have controller/model scaffolding but no usable, rendered body.

**Exported interfaces that route to placeholders.** [plugin_api.cpp](../engine/core/editor/plugin_api.cpp) and [cloud_service.h](../engine/core/social/cloud_service.h) look stable from headers and docs but route to local scratch state or stubs.

**External repositories with real leverage but no canonical intake gate in the remediation program.** [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) now defines the repo-watchlist, legal, fixture, and adoption workflow for twelve external repositories. That work must be treated as part of debt remediation because ungoverned ingestion would create new trust, licensing, and architecture drift immediately.

**Private-use asset acceleration without an integrated governance lane would create a second intake blind spot.** [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) now defines a structured path for staging, normalizing, promoting, and documenting private-use assets from both direct-ingest and discovery repositories. That work also belongs inside remediation because ad hoc asset drops would pollute fixtures, blur provenance, and make "more realistic" editor/runtime surfaces less trustworthy rather than more complete.

**Planning authority is now at risk of splitting across multiple documents.** [URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md](./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md) and [URPG_PGMMV_SUPPORT_PLAN.md](./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md) add real scope, but until that scope is normalized into this remediation hub and the canonical status docs, the repo has parallel phase vocabularies, parallel truthfulness rules, and no shared Definition-of-Done hooks for the new work.

**What this plan does NOT cover:**
- New feature development not related to closing existing gaps.
- Cosmetic refactors not tied to correctness or trust.
- Runtime performance work beyond what is described in Phase 5.

> **Note on [save_runtime.cpp](../engine/core/save/save_runtime.cpp):** Earlier audit passes treated this as a headline stub. It is not. It already has real file I/O, recovery-tier handling, and metadata hydration. It should be treated as a **verification and integration discipline** target, not a rewrite target.

---

## Priority Findings

Each finding is structured as: **Impact → Root Cause → Required Action → Owner → Exit Criteria.**

---

### P0-01 — Build Integrity: Active Compile Blockers

**Impact:** The current tree does not build cleanly. Until `urpg_core` compiles, every downstream remediation finding is partially speculative because the repo cannot prove regressions or closure.

**Root cause:**
- Conflicting `getAllSwitches()` declarations in [global_state_hub.h](../engine/core/global_state_hub.h).
- [map_scene.cpp](../engine/core/scene/map_scene.cpp) calls `m_activeChatbot->chat(...)`, but [chatbot_component.h](../engine/core/message/chatbot_component.h) only exposes `getResponse` and `streamResponse`.
- [map_scene.cpp](../engine/core/scene/map_scene.cpp) references undefined draw-path variables `drawX`, `drawY`, and `z`.

**Required action:**
- Resolve conflicting declarations in [global_state_hub.h](../engine/core/global_state_hub.h).
- Align the chatbot call site in [map_scene.cpp](../engine/core/scene/map_scene.cpp) with the interface exposed by [chatbot_component.h](../engine/core/message/chatbot_component.h).
- Remove or define undefined draw-path variables in [map_scene.cpp](../engine/core/scene/map_scene.cpp).
- Add a mandatory CI build gate for `urpg_core` and all registered tests.

**Owner:** Engine/core maintainers.

**Exit criteria:**
- `urpg_core` builds locally and in CI without warnings-as-errors suppressions.
- The map/chat path compiles against real, agreed-upon interfaces.
- CI gate enforces this going forward.

---

### P1-01 — QuickJS Layer Is A Stub Kernel, Not A Runtime

**Status (2026-04-17):** Remediated via Path B. The QuickJS compat layer is now explicitly documented and tested as a fixture-backed compat-contract harness, not a production JS runtime.

**Impact:** The compat JS subsystem cannot execute real scripts. Any feature or compat claim that depends on JS execution is fictitious until this is resolved or explicitly scoped as a test harness.

**Root cause:**
- [quickjs_runtime.cpp](../runtimes/compat_js/quickjs_runtime.cpp) describes itself as implementation stubs and does not initialize a real QuickJS runtime.
- `initialize()` contains a `TODO` for actual runtime creation.
- `eval()` primarily parses fixture directives (`@urpg-export`, `@urpg-fail-eval`, `@urpg-fail-call`) rather than executing a JS engine path.
- The fixture lane is a strong compat-contract testing harness, but it is not a production script runtime.

**Required action:** Choose one of two explicit paths and commit to it in code and documentation:
  - **Path A (Real Runtime):** Integrate a real QuickJS execution path. `initialize()` must create an actual JS context; `eval()` must execute scripts against it.
  - **Path B (Explicit Test Harness):** Formally document and label this file as a fixture-backed compat-contract harness, not a production runtime. Update all docs, headers, and status labels to reflect this scope.

**Owner:** Compat/runtime maintainers.

**Exit criteria:**
- Path A: `eval()` executes real JS; initialization creates a live QuickJS context.
- Path B: All references to "JS runtime support" in docs and status labels are updated to "fixture-backed contract harness."
- No path treats the fixture lane as evidence of live runtime capability.

**Progress evidence (2026-04-17):**
- [quickjs_runtime.h](../runtimes/compat_js/quickjs_runtime.h) now labels the surface as a "fixture-backed compat-contract harness" with an explicit note that "live QuickJS runtime integration is out of scope for this file."
- [quickjs_runtime.cpp](../runtimes/compat_js/quickjs_runtime.cpp) now uses scope-honest comments (`Note: Live QuickJS runtime initialization belongs in a separate runtime module, not this harness`) instead of TODOs that implied imminent real-JS integration.
- [test_quickjs_runtime.cpp](../tests/unit/test_quickjs_runtime.cpp) already includes an explicit regression that proves `eval("1 + 1")` returns `Nil` rather than evaluating to `2`, documenting the harness-only semantics in test code.

---

### P1-02 — Compat Status Inflation Across Multiple Subsystems

**Impact:** `CompatStatus::FULL` labels on stub or placeholder methods cause downstream code and planning documents to treat these subsystems as production-ready. This makes bugs harder to triage and planning harder to trust.

**Root cause — per file:**

| File | Problem |
|------|---------|
| [data_manager.cpp](../runtimes/compat_js/data_manager.cpp) | `loadDatabase()` now seeds compat containers and accessors return live seeded data where available, but full project-data parity and save/load persistence remain out of scope. |
| [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp) | Troop loading and drop handling remain unfinished, but reward distribution, battle-event cadence, and switch checks are now closed in the compat lane and backed by focused tests. |
| [window_compat.cpp](../runtimes/compat_js/window_compat.cpp) | Renderer command emission, selectable-window pointer semantics, and compat bitmap metadata are now real; `Window_Base::contents()` lifecycle truthfulness is explicitly test-backed, but the contents bitmap still has no pixel buffer and some drag/multi-touch nuances remain simplified. |
| [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp) | AudioManager is honestly labeled `PARTIAL` and covers deterministic playback position, duck/unduck ramps, applied mix scaling, current-BGM observability, and live compat bindings, but it still models a deterministic harness rather than a live mixer/backend. |

**Required action:**
- Audit all `CompatStatus::FULL` claims in the files above plus [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp).
- Downgrade to `PARTIAL` or `STUB` wherever behavior is fixture-backed, placeholder-backed, or disconnected from engine state.
- Explicitly scope [plugin_api.cpp](../engine/core/editor/plugin_api.cpp) and [cloud_service.h](../engine/core/social/cloud_service.h) as stub or scaffolding surfaces.
- Align all completion documentation with the resulting truth.

**Owner:** Compat/runtime maintainers; tech lead for doc alignment sign-off.

**Exit criteria:**
- No method or exported surface is labeled production-ready while routing to a disconnected placeholder.
- Compat reports are trustworthy inputs to planning decisions.
- Docs stop conflating fixture scaffolding with live runtime support.

**Status (2026-04-19):** Partially remediated. `loadDatabase()` now seeds compat containers and `Window_Base::contents()` lifecycle behavior is covered by explicit tests, but the lane remains `PARTIAL` because there is still no pixel buffer and no full project-data parity.

**Progress evidence (2026-04-19):**
- The focused Phase 2 verification lane in `build/dev-mingw-debug` was re-run after the doc edits and passed with battle, window/data, and audio coverage all green under `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleManager:|Window_Base contents lifecycle allocates and rotates deterministic handles|DataManager loadDatabase populates seeded database containers|AudioManager:"`.
- [input_manager.cpp](../runtimes/compat_js/input_manager.cpp): all 79 inflated `CompatStatus::FULL` labels downgraded to `PARTIAL` because the compat layer is fixture-backed (no OS/platform input polling behind the exposed MZ API).
- [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp): playback/state registry labels downgraded from `FULL` to `PARTIAL`; later 2026-04-18 closure work replaced the earlier stubbed QuickJS bindings with live deterministic compat dispatch while preserving honest `PARTIAL` deviations.
- [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp): `processAction` downgraded to `PARTIAL`; stubbed JS bindings downgraded to `STUB`.
- [window_compat.cpp](../runtimes/compat_js/window_compat.cpp): registry labels and stubbed JS bindings downgraded to `STUB`/`PARTIAL` where behavior is placeholder-backed.
- [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp): 30 inflated `CompatStatus::FULL` labels downgraded to `PARTIAL` (plugin lifecycle, command registry, parameters, dependencies, event handlers, execution state, error handling, and diagnostics are all fixture-backed compat-bridge logic, not live engine integration).
- [data_manager.cpp](../runtimes/compat_js/data_manager.cpp): all inflated `CompatStatus::FULL` labels downgraded to `PARTIAL` (`loadDatabase()` now seeds compat containers and the accessors return live seeded data where available; save/load is in-memory only; plugin commands are an in-memory callback map).
- Corresponding unit tests in `test_audio_manager.cpp`, `test_data_manager.cpp`, `test_plugin_manager.cpp`, and `test_window_compat.cpp` updated to assert honest statuses.
- Continued 2026-04-18/2026-04-19 closure in [window_compat.cpp](../runtimes/compat_js/window_compat.cpp): `Window_Selectable` now supports keyboard/gamepad navigation, pointer press/drag/release hit-testing, drag retargeting, drag-scroll, and mouse-wheel scrolling through `InputManager`; `Window_Base::contents()` now allocates compat bitmap metadata records whose dimensions stay synchronized with live rect/padding state and is now backed by explicit deterministic-handle tests.

---

### P1-03 — Audio SE Channel Lifetime Leak

**Status (2026-04-19):** Partially remediated. SE channels now complete deterministically in compat audio updates and are reclaimed without requiring an explicit `stopSe()` call, but the lane remains a deterministic compat harness rather than a live backend.

**Impact:** Sound-effect channels accumulate indefinitely during normal gameplay, causing a real performance and resource leak. This is not a cosmetic TODO — it can degrade runtime behavior in any session that plays multiple sound effects.

**Root cause:**
- [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp) creates a fresh SE channel for every sound effect.
- Cleanup only removes SE channels when `!isPlaying()`.
- `AudioChannel::update()` never advances playback state to completion and contains no duration or end-of-playback logic.
- Result: SE channels only get cleaned up if `stopSe()` is explicitly called. No implicit completion path exists.

**Required action:**
- Implement end-of-playback detection in `AudioChannel::update()` (duration tracking or platform completion callback).
- Drive SE cleanup from this completion signal, not only from `stopSe()`.
- Add a test that proves SE channel count does not grow unbounded under repeated playback without explicit stop calls.

**Owner:** Compat/runtime maintainers.

**Exit criteria:**
- SE channels are reclaimed automatically on playback completion.
- `test_audio_manager.cpp` includes a channel-growth regression test.
- No unbounded channel accumulation under repeated SE playback in integration scenarios.

**Progress evidence (2026-04-16):**
- [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp) now gives compat SE channels a deterministic completion path in `AudioChannel::update()` so the existing cleanup pass can reclaim them.
- [audio_manager.h](../runtimes/compat_js/audio_manager.h) now tracks per-channel completion frames for one-shot compat audio playback.
- [test_audio_manager.cpp](../tests/unit/test_audio_manager.cpp) now includes a regression that proves an `se_*` channel is gone after playback completes and `AudioManager::update()` runs.

**Progress evidence (2026-04-19):**
- [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp) now advances deterministic playback position during compat updates, applies frame-based duck/unduck ramps, applies master/bus volume changes to active playback, and surfaces current-BGM metadata through the compat API.
- [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp) now exposes live QuickJS-facing compat bindings for BGM/BGS/ME/SE control, current-BGM metadata, mix volume, and ducking helpers instead of leaving those paths stubbed.
- [test_audio_manager.cpp](../tests/unit/test_audio_manager.cpp) now covers playback-position progression, applied mix scaling, deterministic duck/unduck ramps, current-BGM observability, and expanded QuickJS bridge routing.

---

### P1-04 — Battle Turn-Condition Correctness Bug

**Impact:** `checkTurnCondition()` produces incorrect turn-cadence results in shipped battle logic, even in a green build. Span `1` always passes after the first threshold; periodic cadence for span `2` is not correctly modeled. This is a behavioral correctness bug, not missing content.

**Root cause:** The cadence comparison logic in [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp) treated span values inconsistently, leaving periodic intervals effectively hard-coded instead of honoring the requested cadence once the turn threshold was reached.

**Required action:**
- Review `checkTurnCondition()` against [BATTLE_CORE_NATIVE_SPEC.md](./BATTLE_CORE_NATIVE_SPEC.md) to confirm the intended cadence semantics.
- Fix the comparison logic to correctly model span-based periodic conditions.
- Add a focused regression test covering span `1`, span `2`, and edge cases around the first-turn threshold.

**Owner:** Compat/runtime maintainers.

**Exit criteria:**
- `checkTurnCondition()` matches the behavior described in the battle spec for all span values.
- Regression tests cover at least: span 1 at first threshold, span 1 after first threshold, span 2 periodic cadence, and boundary turns.

**Status (2026-04-16):** Remediated. `checkTurnCondition()` now treats `span == 0` as exact-turn matching, rejects negative spans, gates periodic checks on the threshold turn, and applies modulo cadence for positive spans so threshold and repeat-turn behavior align with the battle spec.

**Progress evidence (2026-04-16):**
- [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp) now uses one generalized cadence path for positive spans after the threshold turn instead of special-casing incorrect arithmetic for span `1` and span `2`.
- [test_battlemgr.cpp](../tests/unit/test_battlemgr.cpp) now includes a focused regression that proves exact-turn, every-turn-after-threshold, and every-other-turn cadence behavior across successive turns.
- (2026-04-19) The focused `BattleManager:` compatibility lane now also covers reward distribution, switch checks, and battle-event cadence under `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleManager:|Window_Base contents lifecycle allocates and rotates deterministic handles|DataManager loadDatabase populates seeded database containers|AudioManager:"`; drop logic remains a separate open slice.

---

### P2-01 — Diagnostics Workspace Reports More Than It Renders

**Impact:** The workspace claims nine usable diagnostic tabs, but several tabs either render nothing or are commented out. This misleads users and planning documents about actual editor capability.

**Root cause:**
- [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) exports and summarizes nine tabs.
- The `render()` path comments out actual rendering for Audio and Migration Wizard tabs.
- [event_authority_panel.cpp](../editor/diagnostics/event_authority_panel.cpp) refreshes data but renders nothing when visible.
- [audio_inspector_panel.h](../editor/audio/audio_inspector_panel.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) expose controller/state wrappers but no render surface.

**Required action:** Apply one of two resolutions per unrendered tab:
- **Render it:** Implement a real panel body.
- **Hide it:** Remove or mark the tab as "not yet available" in the UI until the body exists.

Do not leave the workspace counting tabs that do not render.

**Owner:** Editor maintainers.

**Exit criteria:**
- Every tab counted in workspace summaries renders a real, non-empty panel body.
- Any tab without a body is either removed from the count or clearly labeled as unavailable.
- `test_diagnostics_workspace.cpp` asserts the current, accurate workspace shape.

**Status (2026-04-16):** Remediated at the workspace-truthfulness level. All tabs counted in workspace summaries now participate in the active-tab render path through panel-owned render snapshots, while deeper panel productization remains tracked separately under P2-03.

**Progress evidence (2026-04-16):**
- [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now calls `audio_panel_.render()` and `migration_wizard_panel_.render()` instead of leaving those tabs commented out.
- [audio_inspector_panel.h](../editor/audio/audio_inspector_panel.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now expose testable render snapshots so the workspace can prove those tabs rendered when active and visible.
- [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp), [test_audio_inspector.cpp](../tests/unit/test_audio_inspector.cpp), and [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) now verify the workspace can activate and render both tabs and that their panel snapshots reflect the current model state, including live audio handle rows.
- [event_authority_panel.h](../editor/diagnostics/event_authority_panel.h) and [event_authority_panel.cpp](../editor/diagnostics/event_authority_panel.cpp) now expose and populate the visible row body plus event-id/severity/mode filtering, selection, row-navigation, and selected-row detail state in the render snapshot so the panel no longer refreshes data silently when visible.
- [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp), [test_event_authority_panel.cpp](../tests/unit/test_event_authority_panel.cpp), and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verify the event-authority tab renders when active and surfaces the expected visible-row body through both the panel snapshot and workspace export, alongside severity, filtering, selection, row-navigation, selected-row detail, and navigation-target state.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports richer `save` active-tab detail through the diagnostics JSON snapshot, including save summary counts, selected slot id, and per-row reserved/corruption/recovery/map metadata. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the save tab export carries the same selected-row and row-detail state instead of only a bare slot list.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports richer `compat` active-tab detail through the diagnostics JSON snapshot, including project score, selected-plugin/detail-view state, expanded plugin summary fields, and structured recent events. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the compat tab export carries real event/plugin detail instead of only plugin count plus a raw event total.

---

### P2-02 — Diagnostics Runtime Binding Is One-Way and Leaves Stale State

**Impact:** Diagnostics can report outdated or incorrect runtime state after a runtime has changed or been detached, even without a crash. This erodes trust in the workspace as a diagnostic tool over time.

**Root cause:**
- [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) uses one-shot refresh-style binding for audio via `bindAudioRuntime()`.
- `clearMenuRuntime()`, `clearAudioRuntime()`, and `clearAbilityRuntime()` are empty no-ops.
- There is no mechanism to invalidate or re-sync workspace state when a runtime is detached or replaced.

**Required action:**
- Implement the `clear*Runtime()` methods so that detaching a runtime actually resets the corresponding workspace state.
- Ensure rebinding a runtime produces a fresh, consistent view rather than layering new data on stale state.
- Test the detach/rebind cycle explicitly.

**Owner:** Editor maintainers.

**Exit criteria:**
- `clearMenuRuntime()`, `clearAudioRuntime()`, and `clearAbilityRuntime()` fully reset displayed state.
- Diagnostics tests cover the detach-then-rebind path and assert state freshness.
- Stale data cannot persist in the workspace after a runtime change.

**Status (2026-04-16):** Remediated. Audio and ability diagnostics now clear projected runtime state on detach, and focused workspace coverage proves detach/rebind cycles produce fresh summaries instead of stale carry-over data.

**Progress evidence (2026-04-16):**
- [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now routes `clearAudioRuntime()` and `clearAbilityRuntime()` through concrete panel/model reset paths instead of leaving them as no-ops.
- [audio_inspector_model.h](../editor/audio/audio_inspector_model.h) now retains projected `AudioCore` count/volume state and exposes a `clear()` path so audio summaries can both reflect bound runtime state and reset back to empty on detach.
- [ability_inspector_model.h](../editor/ability/ability_inspector_model.h), [ability_inspector_model.cpp](../editor/ability/ability_inspector_model.cpp), and [ability_inspector_panel.cpp](../editor/ability/ability_inspector_panel.cpp) now expose a clear path for projected abilities and active tags.
- [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now includes an explicit detach/rebind regression covering audio and ability runtimes across clear and rebind cycles.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports `abilities` active-tab detail through the diagnostics JSON snapshot, including projected ability rows and active gameplay tags. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the abilities tab export clears and repopulates correctly across the existing detach/rebind regression.

---

### P2-03 — Audio and Migration Diagnostics Are Scaffolding, Not Product

**Impact:** Tests for these subsystems validate scaffolding behavior (empty summaries, visibility toggles, model counts) rather than live behavior. This creates false confidence that these areas are verified when they are not.

**Root cause:**
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) is still header-only orchestration and only recently began surfacing per-subsystem execution summaries, so richer workflow/state behavior remains thin.
- [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) exposes snapshot-backed render data, but it is still closer to a diagnostic summary than a fuller wizard workflow surface.
- Audio inspector coverage used to stop at empty summaries and visibility toggles, leaving live row projection unverified until the model was taught to iterate `AudioCore` state directly.
- Migration wizard tests now cover subsystem execution reporting, but they still do not exercise deeper user-facing workflow actions beyond snapshot/state truthfulness.

**Required action:**
- Replace simulated projection in `audio_inspector_model` with iteration over real `AudioCore` state, or document it explicitly as a simulation layer.
- Update tests to assert live behavior once real behavior is implemented; retain scaffolding tests only if they are complementary, not substitutes.
- Implement or explicitly defer migration wizard panel rendering.

**Owner:** Editor maintainers.

**Exit criteria:**
- `audio_inspector_model` reflects real `AudioCore` state or is clearly labeled as a simulation layer in code and docs.
- Tests for both subsystems assert the highest-fidelity behavior that exists, not the lowest.

**Status (2026-04-17):** Partially remediated. The audio inspector now projects live `AudioCore` active-source rows, and the migration wizard now reports which subsystem migrations actually ran, including message migration, but richer wizard-product workflow remains the primary open slice under this finding.

**Progress evidence (2026-04-17):**
- [audio_core.h](../engine/core/audio/audio_core.h) now exposes read-only active-source snapshots, including asset id, category, and channel state, so editor diagnostics can inspect real runtime state without mutating the mixer.
- [audio_inspector_model.h](../editor/audio/audio_inspector_model.h) now iterates those snapshots into real `AudioHandleRow` entries instead of reporting count-only placeholder state.
- [test_audio_inspector.cpp](../tests/unit/test_audio_inspector.cpp) now asserts live handle projection, asset id, category, volume, pitch, and active-state fields rather than only empty summaries.
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) now orchestrates message, menu, and battle migration passes and records per-subsystem summary logs instead of only emitting one generic completion line.
- [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now surfaces summary-log count, headline text, the rendered summary-log lines, and a concrete clear/reset path so the workspace can assert both completed runs and reset state.
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now carry structured per-subsystem results for message, menu, and battle migration, so deeper workflow/productization work can consume typed wizard state instead of parsing free-form log text.
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now carry selection state for subsystem results, giving follow-on wizard UI a concrete interaction model instead of only passive snapshots.
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) now defaults selection to the first migrated subsystem after a run, and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now surfaces selected-subsystem detail fields so a follow-on UI can render meaningful details immediately instead of only an id.
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now support next/previous subsystem-result navigation and surface explicit can-navigate state in the render snapshot so workflow consumers can step through results in order.
- [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now carries the selected subsystem's warning count, error count, and completion state in the render snapshot, so follow-on UI can render a fuller selected-status card directly from snapshot data.
- [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) now stores each subsystem result's own summary line, and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now carries that selected subsystem summary line directly in the snapshot, so follow-on UI does not need to infer it from the loose `summary_logs` list.
- [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) and [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now expose `clearMigrationWizardRuntime()` so the top-level diagnostics surface can reset wizard state consistently with the other panels.
- [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verify structured subsystem results, subsystem execution reporting, default subsystem selection, selected-subsystem detail/status/summary fields, rendered wizard summary text, and clean reset behavior rather than only aggregate counts.
- (2026-04-17) Added `rerunSubsystem(id, project_data)` to [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h), giving the wizard a concrete user-facing workflow action beyond passive snapshot truthfulness. Panel render snapshot now carries `can_rerun_selected_subsystem`. Added 3 new tests covering re-run of existing subsystems, adding missing subsystems via re-run, and snapshot reflection of the action.
- (2026-04-17) Added `clearSubsystemResult(id)` and `getReportJson()` to [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h), giving the wizard selective reset and structured export capabilities. Panel render snapshot now carries `can_clear_selected_subsystem` and `exported_report_json`. Added 5 new tests covering selective result clearing, selection updates on clear, JSON report serialization, and snapshot reflection of both actions.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports migration-wizard active-tab detail through the top-level diagnostics JSON snapshot, including summary logs, structured subsystem results, selected-subsystem detail, navigation affordances, and the wizard's exported report JSON. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that `migration_wizard` exports the same richer workflow state the panel already renders.
- (2026-04-17) [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) and [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now expose migration wizard workflow actions directly at the workspace layer: subsystem selection, next/previous navigation, subsystem re-run, selective clear, and report export/save/load. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies that the top-level diagnostics surface can drive those actions and observe the resulting selected-subsystem/report changes through the exported workspace snapshot.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes the migration wizard render snapshot immediately after workspace-driven wizard actions when the wizard tab is active and visible, so [`exportAsJson()`](../editor/diagnostics/diagnostics_workspace.cpp) stays truthful without requiring a separate manual render step. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts this snapshot-coherence behavior for selection, rerun, selective clear, and full clear.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes the migration wizard snapshot after failed report loads too, so invalid workspace-level imports do not leave stale wizard state in the exported diagnostics snapshot after the model clears itself. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts this failed-load truthfulness path.
- (2026-04-17) [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now also covers the happy-path `DiagnosticsWorkspace` wizard report save/load round-trip, asserting that selected-subsystem state and exported report JSON survive a workspace-level file round-trip rather than only model/panel-local file I/O.
- (2026-04-17) [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) now treats “clear the last remaining subsystem result” as a full reset, so selective clear no longer leaves a zero-result wizard falsely marked complete. [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now assert the empty-state panel/workspace snapshot after that last-result clear path, including disabled save/clear affordances.
- (2026-04-17) [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h), [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h), and [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) now expose selection-scoped wizard actions (`rerunSelectedSubsystem(...)`, `clearSelectedSubsystemResult()` and workspace forwards), so the richer wizard workflow can act directly on the current selected subsystem instead of requiring consumers to feed the selected id back into id-based actions. The model wrappers now copy the selected id before mutating selection-backed state, and [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) plus [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now cover those selected-action flows.
- (2026-04-17) [migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) now repairs orphaned `selected_subsystem_id` values during report import when the saved selection does not exist in the loaded `subsystem_results`, falling back to the first real subsystem instead of exporting an impossible selected-id shell. [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now cover this stale-selection import path at the model, panel snapshot, and workspace export layers.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports the migration wizard panel snapshot's aggregate `total_files_processed`, `warning_count`, and `error_count` fields through the top-level `migration_wizard` active-tab detail instead of leaving consumers to infer them from tab summary or subsystem rows. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies those aggregate counts for both a live migration run and a loaded wizard report.
- (2026-04-17) [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now retains bound project data from `onProjectUpdateRequested(...)` and exposes no-argument rerun actions (`rerunBoundProject()`, `rerunBoundSelectedSubsystem()`), reducing the prior one-shot-wrapper behavior. Its render snapshot now carries `has_bound_project_data`, `can_rerun_bound_migration`, and `can_rerun_bound_selected_subsystem`, and [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) / [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now forward those retained-binding rerun actions at the workspace layer. [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verify that cleared subsystem state can be rebuilt from the bound runtime without resupplying project JSON.
- (2026-04-17) [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) now clears any previously retained bound project data when `loadReportFromFile()` imports a report, so file-loaded wizard state no longer inherits stale bound-runtime rerun affordances from an earlier live binding. [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verify this detachment path at both panel and workspace export levels.
- (2026-04-17) [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now also asserts that the invalid-report import path clears the migration wizard's exported bound-runtime affordances and disables bound rerun actions, not only the report rows themselves. This closes the remaining coverage gap in the failed-load branch introduced by retained bound project data.
- (2026-04-17) [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now also assert the opposite save-path contract: `saveReportToFile()` preserves the migration wizard's retained bound runtime and leaves bound rerun affordances usable immediately after save. This closes the remaining untested branch in the wizard's file workflow now that load and failed-load explicitly detach bound runtime.
- (2026-04-17) [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp) now also asserts the panel-local failed-load contract for retained bound runtime, proving that invalid `loadReportFromFile()` input clears bound rerun affordances even before going through `DiagnosticsWorkspace`. This closes the remaining panel-only coverage gap in the wizard file/binding path.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes the audio inspector render snapshot immediately after workspace-driven audio bind/clear actions when the audio tab is active and visible, so [`exportAsJson()`](../editor/diagnostics/diagnostics_workspace.cpp) does not retain stale audio rows after clear/rebind cycles. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts this audio snapshot-coherence behavior.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes the event-authority render snapshot immediately after workspace-driven diagnostics ingest/clear actions when the event-authority tab is active and visible, so [`exportAsJson()`](../editor/diagnostics/diagnostics_workspace.cpp) does not retain stale event rows after ingest/clear cycles. The event-authority active-tab export now also includes `has_data`, matching the panel snapshot contract. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts this event-authority snapshot-coherence behavior.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes snapshot-backed tabs immediately when they become the active visible diagnostics tab, covering at least `event_authority`, `audio`, and `migration_wizard`. This closes the remaining activation-time stale-export gap for panels whose workspace export reads `lastRenderSnapshot()` rather than direct model state. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts this activation-time snapshot truthfulness path.
- (2026-04-17) [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now also covers hidden-to-visible restoration for a snapshot-backed tab, proving the same activation helper keeps exported detail truthful when the workspace is re-shown, not only when the active tab changes.
- (2026-04-17) [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) and [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now expose event-authority workflow actions directly at the workspace layer: event-id/level/mode filters, clear-filters, row selection, and next/previous row navigation. The workspace now uses refresh+render for data/filter changes but render-only for selection/navigation so exported event-authority state stays truthful without wiping the current row selection. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies this workspace-level event-authority workflow surface.
- (2026-04-17) [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) and [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now expose message-inspector workflow actions directly at the workspace layer: route filtering, clear-route-filter, “issues only” toggling, and row selection. [message_inspector_model.cpp](../editor/message/message_inspector_model.cpp) now preserves selected page id across filter/rebuild cycles when the selected page remains visible, preventing workspace-driven filter changes from dropping `selected_page_id` unnecessarily. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies this workspace-level message workflow surface.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports audio active-tab detail through the same diagnostics JSON snapshot, including master volume and live handle rows from the audio inspector panel snapshot. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the `audio` tab exports live row/category data instead of only the tab summary shell.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports menu active-tab detail through the same diagnostics JSON snapshot, including menu inspector summary counts, visible rows, structured issues, selected command id, and menu preview visibility/title state. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the `menu` tab exports inspector and preview state instead of only the active-tab name.
- (2026-04-17) [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) and [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now expose menu-inspector workflow actions directly at the workspace layer: command-id filtering, clear-filter, “issues only” toggling, and row selection. [menu_inspector_model.cpp](../editor/ui/menu_inspector_model.cpp) now preserves selected command id across filter rebuilds when the selected command remains visible, and the `menu` active-tab export now includes `command_id_filter`, `show_issues_only`, and a structured `selected_row` payload. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies this workspace-level menu workflow surface.
- (2026-04-17) [menu_inspector_panel.h](../editor/ui/menu_inspector_panel.h) and [menu_inspector_panel.cpp](../editor/ui/menu_inspector_panel.cpp) now capture a real render snapshot for the menu diagnostics surface, replacing the remaining placeholder registry/scene/selection sections with model-backed panel state and selected-command detail. [menu_inspector_model.cpp](../editor/ui/menu_inspector_model.cpp) now preserves selected command identity even when the selected command is filtered or hidden, so workspace-driven menu filtering stays actionable instead of dropping selection state. [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes and exports the `menu` tab from that rendered snapshot when active, and [test_menu_inspector_panel.cpp](../tests/unit/test_menu_inspector_panel.cpp), [test_menu_inspector_model.cpp](../tests/unit/test_menu_inspector_model.cpp), and [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verify the panel snapshot, hidden-selection persistence, and snapshot-backed workspace export path.
- (2026-04-17) [menu_preview_panel.h](../editor/ui/menu_preview_panel.h) and [menu_preview_panel.cpp](../editor/ui/menu_preview_panel.cpp) now capture a real menu-preview render snapshot, including active scene id, visible pane entries, selected-command ids, and pane command lists instead of exposing only panel title/visibility. [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now refreshes and exports that preview snapshot through the `menu` tab's `preview` payload when active, and [test_menu_preview_panel.cpp](../tests/unit/test_menu_preview_panel.cpp) plus [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verify the richer preview workflow surface.
- (2026-04-17) [diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) and [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now expose `dispatchMenuPreviewAction(InputAction)` at the workspace layer, allowing the diagnostics menu tab to drive preview-side navigation/confirm flows instead of only observing state. The workspace now keeps both [menu_inspector_model.cpp](../editor/ui/menu_inspector_model.cpp) and [menu_preview_panel.cpp](../editor/ui/menu_preview_panel.cpp) coherent from the same mutable `MenuSceneGraph`, and the preview snapshot/export now carries `last_blocked_command_id` and `last_blocked_reason` so blocked confirm behavior is diagnosable from the surfaced workflow. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies command navigation, pane navigation, and blocked-command export through the workspace-level preview action surface.
- (2026-04-17) [menu_inspector_model.h](../editor/ui/menu_inspector_model.h) and [menu_inspector_model.cpp](../editor/ui/menu_inspector_model.cpp) now expose `SelectCommandById`, allowing the inspector surface to follow live graph-driven command changes without depending on a visible-row index from a prior filter state. [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now applies the active preview command back into the inspector after workspace-level preview actions, so `selected_command_id` / `selected_row` stay aligned with preview navigation instead of dropping to null after graph-side movement. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies this cross-surface coherence path.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now also projects inspector-side row selection back into the mutable `MenuSceneGraph`, activating the selected pane and command in the preview workflow instead of leaving preview state stale after `selectMenuRow()`. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies this reverse-sync path by asserting that workspace-level inspector selection updates the exported preview `selected_command_id`.
- (2026-04-17) [menu_inspector_model.h](../editor/ui/menu_inspector_model.h) and [menu_inspector_model.cpp](../editor/ui/menu_inspector_model.cpp) now expose `SelectCommandRow(pane_id, command_id)` so inspector selection can target a specific pane/command pair instead of ambiguously matching only by command id. [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now uses the active preview pane id plus command id when syncing preview-driven navigation back into the inspector, preventing duplicate command ids across panes from selecting the wrong inspector row. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now verifies that preview-side pane navigation selects the `side_pane` row rather than the first matching command id from `main_pane`.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports battle active-tab detail through the same diagnostics JSON snapshot, including battle inspector summary data, visible action rows, structured inspector issues, selected subject id, preview snapshot values, and preview issues. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the `battle` tab exports inspector and preview state instead of only the tab summary shell.
- (2026-04-17) [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) now exports message-text active-tab detail through the same diagnostics JSON snapshot, including message inspector summary counts, visible page rows, structured issues, and selected page id. [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) now asserts that the `message_text` tab exports real inspector state instead of only the active-tab shell.

---

### P2-04 — Test and Build Registration Drift

**Impact:** Tests that are not registered are not run. Tests that assert wrong counts or stale structure pass while hiding real regressions. Duplicate test files create confusion about what is actually tested.

**Root cause:**
- No enforcement mechanism existed to ensure every test file on disk was registered in [CMakeLists.txt](../CMakeLists.txt), allowing tests to silently go unrun.
- Manually maintained registration lists drift over time as files are added, renamed, or split without a corresponding CMakeLists.txt update.
- Duplicate test files accumulate when new coverage is added alongside rather than replacing old test files (e.g., `test_compat_reportPanel.cpp` vs `test_compat_report_panel.cpp`).
- Test assertions against workspace shape or tab counts are not automatically updated when the workspace structure changes, so they silently pass while covering a stale surface.
- `test_menu_orchestration.cpp` was present on disk but not registered in [CMakeLists.txt](../CMakeLists.txt) and therefore never executed.

**Required action:**
- Keep `test_diagnostics_workspace.cpp` aligned with the current 9-tab workspace/export shape as diagnostics surfaces evolve.
- Keep `test_spatial_editor.cpp` registered in [CMakeLists.txt](../CMakeLists.txt) and maintain its focused gate coverage as the active spatial editor regression surface.
- Keep [test_compat_report_panel.cpp](../tests/unit/test_compat_report_panel.cpp) as the registered Catch test surface; the stale unregistered `test_compat_reportPanel.cpp` file has been removed.
- Extend `test_audio_manager.cpp` to cover the channel-growth scenario (see P1-03).

**Owner:** Respective subsystem maintainers; release owner for CMakeLists.txt audit.

**Exit criteria:**
- No registered test asserts a stale count or outdated structure.
- No orphan test files exist outside CMakeLists.txt registration without explicit "incubating" documentation.
- No duplicate test files for the same subject exist.

**Progress evidence (2026-04-17):**
- Removed duplicate `tests/unit/test_engine_shell.cpp` entry from [CMakeLists.txt](../CMakeLists.txt).
- Registered missing `tests/unit/test_menu_orchestration.cpp` in [CMakeLists.txt](../CMakeLists.txt) and fixed its broken scene-stack navigation section.
- Added SE channel-growth regression test to [test_audio_manager.cpp](../tests/unit/test_audio_manager.cpp) (see P1-03).

**Progress evidence (2026-04-18):**
- Published [docs/COMPAT_EXIT_CHECKLIST.md](./COMPAT_EXIT_CHECKLIST.md) with explicit import-confidence and migration-confidence pass criteria, signed-off-by section, and related-document links.

---

### P2-05 — Menu Serialization Is Import-Only, Not Round-Trippable

**Impact:** Menu authoring is a one-way lane. Menus can be imported but not exported, which prevents round-trip tooling, integration testing, and any workflow that depends on save-reload fidelity.

**Status (2026-04-16):** Remediated.

**Root cause:**
- [menu_serializer.cpp](../engine/core/ui/menu_serializer.cpp) can deserialize and import legacy data.
- `Serialize()` returns an empty object because the public graph API cannot enumerate registered scenes.
- [menu_preview_panel.cpp](../editor/ui/menu_preview_panel.cpp) exists but is not part of a surfaced workflow.

**Required action:**
- Implement `Serialize()` once the public graph API exposes scene enumeration, or implement the enumeration API as a prerequisite.
- Surface `menu_preview_panel.cpp` in a clear, reachable workflow or remove it.
- Add a round-trip test: serialize a known menu definition, deserialize it, and assert structural equivalence.

**Owner:** Engine/core maintainers (serialization); editor maintainers (preview panel).

**Exit criteria:**
- `Serialize()` produces non-empty output for at least one registered menu structure.
- A round-trip test (serialize → deserialize → compare) passes.
- The preview panel is either surfaced in a workflow or removed with rationale.

**Resolution evidence (2026-04-16):**
- `MenuSceneGraph` now exposes registered-scene enumeration and `MenuSceneSerializer::Serialize()` emits a non-empty native menu scene definition.
- `tests/unit/test_menu_legacy_import.cpp` now includes serialize → deserialize round-trip coverage for native menu graphs.
- `MenuPreviewPanel` is now surfaced through `DiagnosticsWorkspace`, and menu diagnostics integration coverage asserts the panel is visible on the menu workflow tab.

---

### P2-06 — Presentation and Spatial Claims Exceed Build Graph Evidence

**Impact:** Documentation describes presentation and spatial work at a higher completion level than the build graph supports. This makes it impossible to trust project status documents as planning inputs.

**Root cause:**
- [NATIVE_FEATURE_ABSORPTION_PLAN.md](./NATIVE_FEATURE_ABSORPTION_PLAN.md) and [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md) describe presentation and spatial work with stronger completion posture than the build graph supports.
- `editor/spatial/*` is not registered as compiled editor sources in [CMakeLists.txt](../CMakeLists.txt) (only header files exist). While `engine/core/presentation/presentation_runtime.cpp`, `engine/core/presentation/release_validation.cpp`, [test_presentation_runtime.cpp](../tests/unit/test_presentation_runtime.cpp), and [test_spatial_editor.cpp](../tests/unit/test_spatial_editor.cpp) are registered, the bulk of `engine/core/presentation/*` headers are not directly compiled, and no `editor/spatial/*.cpp` sources are built.

**Required action:**
- Make an explicit, recorded decision for `engine/core/presentation/*` and `editor/spatial/*`:
  - **Productized:** Add to CMakeLists.txt, register tests, update docs to reflect real build coverage.
  - **Incubating:** Label explicitly in documentation and remove from completion status claims.
- This decision should be made by the tech lead and documented in an ADR if it changes the current implied status.

**Owner:** Tech lead or release owner.

**Exit criteria:**
- `engine/core/presentation/*` and `editor/spatial/*` are either built, registered, and tested — or explicitly documented as incubating.
- Completion status documents no longer describe unbuilt or unregistered work as shipped.

**Status (2026-04-17):** Remediated via documentation-truth alignment and ADR.

**Decision:**
- `editor/spatial/*` is **Incubating**. It contains only header files (`elevation_brush_panel.h`, `prop_placement_panel.h`) with no `.cpp` implementation registered in any build target. It cannot be treated as productized until compiled sources exist.
- `engine/core/presentation/*` is **Incubating** as a subsystem. The actively compiled surfaces are `presentation_runtime.cpp` (in `urpg_core`) and `release_validation.cpp` (as standalone executable `urpg_presentation_release_validation`). `profile_arena.cpp` exists on disk but is intentionally not added to `urpg_core` while the subsystem remains header-heavy. The existing unit tests (`test_presentation_runtime.cpp`, `test_spatial_editor.cpp`) are retained and registered.

**Progress evidence (2026-04-17):**
- Updated [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md) to label `editor/spatial/*` and `engine/core/presentation/*` as incubating, with an explicit note that only `presentation_runtime.cpp` and `release_validation.cpp` are compiled.
- Updated [NATIVE_FEATURE_ABSORPTION_PLAN.md](./NATIVE_FEATURE_ABSORPTION_PLAN.md) to label Spatial Presentation and Editor Tooling as **Incubating**.
- Updated [SPATIAL_EDITOR_TOOLS.md](./presentation/SPATIAL_EDITOR_TOOLS.md) with an explicit incubating / header-only notice.
- Recorded decision in [ADR-011-presentation-spatial-status.md](./adr/ADR-011-presentation-spatial-status.md).

---

### P2-07 — Native Plugin API Is A Mock Bridge, Not A Real Integration

**Impact:** Third-party plugin developers relying on [plugin_api.h](../engine/core/editor/plugin_api.h) will encounter an API surface that looks stable but behaves like a mock. This is a trust and integration correctness risk for any external plugin work.

**Status (2026-04-16):** Remediated via explicit stub labeling. Routing remains stub-backed by design.

**Root cause:**
- [plugin_api.h](../engine/core/editor/plugin_api.h) presents a stable C-style plugin API.
- [plugin_api.cpp](../engine/core/editor/plugin_api.cpp) routes global variables and switches into local static maps, not real engine state.
- Entity creation uses a local incrementing counter; ECS operations are comments.
- Key input always returns `false`; mouse position always returns `(0, 0)`.

**Required action:**
- Either route plugin API calls to real engine state, or add prominent `STUB` or `NOT_YET_ROUTED` annotations to every method that does not.
- Update [plugin_api.h](../engine/core/editor/plugin_api.h) header comments and any plugin-facing documentation to reflect current routing truth.
- Do not remove the stable API surface; only route it honestly.

**Owner:** Engine/core maintainers.

**Exit criteria:**
- Every method in [plugin_api.cpp](../engine/core/editor/plugin_api.cpp) either routes to real engine state or is labeled `STUB` in both implementation and documentation.
- Plugin-facing documentation does not describe mock behavior as production-ready.

**Resolution evidence (2026-04-16):**
- [plugin_api.h](../engine/core/editor/plugin_api.h) now labels entity, global-state, and input sections as `STUB` or scratch-state-only behavior.
- [plugin_api.cpp](../engine/core/editor/plugin_api.cpp) now uses explicit `STUB` comments at each disconnected export site.
- [test_plugin_api.cpp](../tests/unit/test_plugin_api.cpp) continues to lock the scratch-state and placeholder-backed behavior in unit coverage.

---

### P2-08 — Cloud Sync Documentation Overstates Implementation Readiness

**Impact:** Documentation describes cloud sync as a live workflow path. Developers and stakeholders reading this documentation may plan or build against a capability that does not exist.

**Status (2026-04-16):** Remediated via documentation alignment. In-tree behavior remains stub-backed by design.

**Root cause:**
- [cloud_service.h](../engine/core/social/cloud_service.h) only provides an in-memory `CloudServiceStub`.
- [AI_SUBSYSTEM_CLOSURE_CHECKLIST.md](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md) marks encrypted sync as ready for `ICloudService` integration.
- [AI_COPILOT_GUIDE.md](./AI_COPILOT_GUIDE.md) describes cloud sync as a workflow path for preserving conversations across devices.
- [URPG_Blueprint_v3_1_Integrated.md](../URPG_Blueprint_v3_1_Integrated.md) is more accurate, noting the interface is stubbed.

**Required action:**
- Update [AI_SUBSYSTEM_CLOSURE_CHECKLIST.md](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md) and [AI_COPILOT_GUIDE.md](./AI_COPILOT_GUIDE.md) to state clearly that cloud sync is backed by an in-memory stub, not a production service.
- Do not describe the `CloudServiceStub` as an integration-ready path.
- Add a note to [cloud_service.h](../engine/core/social/cloud_service.h) header marking it explicitly as a stub pending real backend integration.

**Owner:** Tech lead or release owner (doc alignment); engine/core maintainers (header annotation).

**Exit criteria:**
- No AI-facing or developer-facing doc describes cloud sync as operational.
- `cloud_service.h` header clearly marks `CloudServiceStub` as not for production use.

**Resolution evidence (2026-04-16):**
- [cloud_service.h](../engine/core/social/cloud_service.h) already marks `CloudServiceStub` as in-memory and not a production cloud integration path.
- [AI_SUBSYSTEM_CLOSURE_CHECKLIST.md](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md) now frames encrypted sync as plumbing-only coverage backed by `CloudServiceStub`.
- [AI_COPILOT_GUIDE.md](./AI_COPILOT_GUIDE.md) now states that the in-tree path is local-memory stub behavior, not operational cross-device persistence.

---

### P2-09 — Async Plugin Callbacks Lack Thread-Affinity Guarantees

**Impact:** Callbacks from the async worker thread in [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp) may touch editor or gameplay state that assumes a main-thread context, creating a latent race condition or reentrancy risk.

**Status (2026-04-16):** Remediated via explicit deferred callback dispatch.

**Root cause:**
- [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp) runs an async worker thread and directly invokes plugin callbacks from that thread.
- No main-thread marshalling or UI-thread safety boundary is visible around those invocations.

**Required action:**
- Audit all callback invocation sites in [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp) for thread-affinity assumptions.
- If any callback touches engine state that requires main-thread access, add an explicit marshalling mechanism (e.g., queue-post to main thread).
- Document the intended threading contract for plugin callbacks.

**Owner:** Compat/runtime maintainers.

**Exit criteria:**
- Thread-affinity rules for plugin callbacks are documented.
- Callbacks that touch main-thread state are marshalled correctly.
- No direct cross-thread invocations of UI or gameplay state exist in [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp).

**Resolution evidence (2026-04-16):**
- [plugin_manager.h](../runtimes/compat_js/plugin_manager.h) now documents that async command execution is worker-threaded while callback delivery is deferred until `dispatchPendingAsyncCallbacks()` is called on the owning thread.
- [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp) now queues completed async callbacks and no longer invokes them directly from the worker thread.
- [test_plugin_manager.cpp](../tests/unit/test_plugin_manager.cpp) now asserts callbacks do not fire before dispatch and still execute in FIFO order after main-thread dispatch.

---

### P2-10 — Native-Absorption And PGMMV Planning Drift Sits Outside Canonical Remediation Governance

**Impact:** The repo now contains legitimate new execution scope for PGMMV intake, native feature absorption, HD-2D / 2.5D, authoring UX, accessibility, localization, and product-glue systems, but that scope currently lives beside the remediation hub instead of inside it. That creates parallel planning authorities, inconsistent status vocabulary, and a real risk that roadmap claims outrun build, test, migration, and documentation truth gates.

**Root cause:**
- [URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md](./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md) introduces a broad integrated roadmap with new phases, tracks, and status rules.
- [URPG_PGMMV_SUPPORT_PLAN.md](./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md) defines a full PGMMV intake/migration lane with its own milestones, acceptance matrix, and issue breakdown.
- This remediation hub, [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md), and [NATIVE_FEATURE_ABSORPTION_PLAN.md](./NATIVE_FEATURE_ABSORPTION_PLAN.md) did not previously absorb that scope into one canonical governance chain.
- The result is not just duplicate docs; it is duplicate execution logic for planning, truthfulness, and release readiness.

**Required action:**
- Treat the new roadmap files as planning-input documents, not parallel long-term authorities.
- Fold their substantive scope into the canonical remediation/status/roadmap set so phases, tracks, and exit criteria line up across:
  - this remediation hub
  - [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md)
  - [NATIVE_FEATURE_ABSORPTION_PLAN.md](./NATIVE_FEATURE_ABSORPTION_PLAN.md)
- Assign explicit remediation-governed placement for the newly added lanes:
  - PGMMV intake and migration truthfulness
  - plugin-evidence scanning and native replacement mapping
  - HD-2D / 2.5D presentation productization
  - authoring UX
  - accessibility and localization
  - RPG glue systems
- Require any standalone roadmap doc that introduces new execution scope to be either absorbed into canonical docs or marked as superseded/reference-only in the same change.

**Owner:** Tech lead or release owner (planning governance), with engine/editor/runtime/domain leads for the affected lanes.

**Exit criteria:**
- Remediation, status, and roadmap docs share one phase vocabulary, one truthfulness model, and one execution-order story.
- PGMMV/native-absorption scope is represented in remediation phases, documentation-alignment tasks, and Definition of Done instead of sitting outside them.
- No new roadmap document can introduce execution scope without canonical ownership, doc-alignment hooks, and release-truth gates.

---

### P3-01 — Hidden Ownership Shortcuts in Runtime Services

**Impact:** Static-local service instances and per-frame full rebuilds make testing harder, determinism unreliable, and performance tuning more expensive. These are not today's highest-risk issues, but they compound other debt over time.

**Status (2026-04-16):** Remediated. `MapScene` now uses an injected audio service and retains tile render commands behind a dirty-flagged cache so unchanged frames stop rebuilding tile command objects.

**Root cause:**
- [map_scene.cpp](../engine/core/scene/map_scene.cpp) uses a local `static AudioCore` in `processAiAudioCommands()` instead of a passed-in or engine-owned dependency.
- [map_scene.cpp](../engine/core/scene/map_scene.cpp) rebuilds the entire render layer every update tick rather than building incrementally.

**Required action:**
- Replace the static-local `AudioCore` with a dependency-injected service.
- Profile per-frame render-layer rebuild cost and implement dirty-flagging or incremental rebuild.

**Owner:** Engine/core maintainers.

**Exit criteria:**
- `processAiAudioCommands()` operates on an injected `AudioCore`, not a static local.
- Per-frame render work is proportional to scene delta, not to full scene size.

**Progress evidence (2026-04-16):**
- [map_scene.h](../engine/core/scene/map_scene.h) now exposes `setAudioCore()` and stores the scene audio service as state instead of reaching for a static-local singleton.
- [map_scene.cpp](../engine/core/scene/map_scene.cpp) now routes AI audio commands through the bound audio service.
- [test_scene_manager.cpp](../tests/unit/test_scene_manager.cpp) now asserts AI audio commands mutate the injected `AudioCore`.
- [map_scene.h](../engine/core/scene/map_scene.h) now tracks a dirty flag for retained tile render commands and invalidates that cache when map tile data changes.
- [map_scene.cpp](../engine/core/scene/map_scene.cpp) now rebuilds cached tile render commands only when the map changes, then resubmits the retained commands plus the live player sprite each frame.
- [test_scene_manager.cpp](../tests/unit/test_scene_manager.cpp) now asserts unchanged frames reuse the same retained tile commands while map edits force a rebuilt tile command with the new tile index.

---

### P3-02 — External Repository Intake Needs Canonical Governance

**Status (2026-04-17):** Partially remediated. Canonical governance artifacts have been created and linked.

**Impact:** External repositories can accelerate compat, import, localization, exporter, and editor work, but ungoverned ingestion would create new license ambiguity, fixture sprawl, architectural contamination, and false confidence. Without integrating intake into remediation, the program can "fix" debt while silently creating new debt.

**Root cause:**
- [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) now exists as a detailed subsidiary plan, but this remediation hub did not previously assign it a phase, owner, or Definition-of-Done hooks.
- The intake program covers legal review, fixture corpus curation, wrapped dependency policy, and adopt/defer/ignore decisions for twelve external repositories.
- Several existing remediation lanes already depend on exactly these outcomes:
  - compat fixture expansion
  - legacy import foundation
  - localization bootstrap
  - exporter/packager upgrades
  - asset/reference pack intake
  - editor UX reference mining

**Required action:**
- Treat [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) as the execution-detail plan for external repository intake, with this remediation hub defining when it starts and what gates it must satisfy.
- Launch intake governance inside Phase 4 before any production-candidate code or asset adoption proceeds:
  - repo watchlist
  - license matrix
  - audit templates
  - fixture/attribution schemas
  - adoption matrix
- Require every external repo outcome to be one of:
  - `reference_only`
  - `fixture_only`
  - `production_candidate`
  - `asset_reference`
- Require wrapped integration or clean-room reimplementation for any production candidate; do not permit architecture bleed-through into engine core.

**Owner:** Tech lead or release owner (program governance), content/pipeline lead (asset/reference intake), subsystem owners for compat/import/export/editor follow-through.

**Exit criteria:**
- The intake governance artifacts from [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) are active and linked from remediation work.
- No external repository is copied, vendored, or mined into URPG product lanes without a recorded legal and technical disposition.
- External-repo-derived fixtures, wrappers, and adoption decisions are visible planning inputs rather than ad hoc local experiments.

**Progress evidence (2026-04-17):**
- [docs/external-intake/repo-watchlist.md](./external-intake/repo-watchlist.md) created and linked from [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md).
- [docs/external-intake/license-matrix.md](./external-intake/license-matrix.md) created and linked.
- [docs/external-intake/repo-audit-template.md](./external-intake/repo-audit-template.md) created and linked.
- [docs/external-intake/urpg_feature_adoption_matrix.md](./external-intake/urpg_feature_adoption_matrix.md) created and linked.

---

### P3-03 — Private-Use Asset Intake Needs Canonical Governance

**Status (2026-04-17):** Partially remediated. Canonical governance artifacts have been created and linked.

**Impact:** Private-use asset ingestion can quickly improve editor feedback and runtime realism, but without governance it would create provenance drift, noisy content roots, license/context confusion, and false confidence about how "real" those surfaces are. The project would appear more complete while becoming harder to audit, curate, or safely evolve.

**Root cause:**
- [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) now exists as a detailed subsidiary plan, but this remediation hub did not previously assign it a finding, phase workstream, owner, or Definition-of-Done hooks.
- The plan distinguishes two intake modes that need different handling:
  - direct-ingest sources intended for controlled staging and promotion
  - discovery/source-mining sources intended to feed a tracked acquisition backlog rather than direct copying
- The plan also introduces concrete governance artifacts and content lanes that were not yet anchored in the remediation program:
  - `third_party/github_assets/*`
  - `imports/staging/asset_intake/*`
  - `imports/normalized/*`
  - asset source, promotion, and bundle manifests
  - `docs/asset_intake/*`

**Required action:**
- Treat [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) as the execution-detail plan for private-use asset intake, with this remediation hub defining the gating conditions and ownership.
- Launch private asset intake governance inside Phase 4 before promoted private-use assets are allowed to influence editor/runtime product lanes:
  - source-capture and audit workflow
  - provenance-preserving staging and normalization structure
  - asset source, promotion, and bundle manifests
  - intake reporting
  - source registry and promotion guidance docs
- Require the plan's direct-ingest and discovery repositories to be handled differently:
  - direct-ingest repos may enter controlled staging and selective promotion
  - discovery repos must become tracked acquisition/backlog inputs rather than direct asset drops
- Use the governed pipeline to feed fast-win product lanes only after provenance is recorded:
  - UI sound feedback
  - prototype sprite passes
  - curated fantasy-environment vertical slices

**Owner:** Tech lead or release owner (program governance), content/pipeline lead (asset intake flow), asset/tooling owners for manifest/report infrastructure, affected editor/runtime owners for product-lane integration.

**Exit criteria:**
- The intake artifacts from [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) are active and linked from remediation work.
- Private-use assets are staged, normalized, and promoted through recorded provenance rather than ad hoc copies into runtime/editor trees.
- Direct-ingest and discovery-source repos have distinct tracked handling paths.
- At least one editor-facing path and one runtime-facing path consume promoted assets through the governed pipeline rather than through local-only drops.

**Progress evidence (2026-04-17):**
- [docs/asset_intake/ASSET_SOURCE_REGISTRY.md](./asset_intake/ASSET_SOURCE_REGISTRY.md) created and linked from [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md).
- [docs/asset_intake/ASSET_PROMOTION_GUIDE.md](./asset_intake/ASSET_PROMOTION_GUIDE.md) created and linked.
- [docs/asset_intake/ASSET_CATEGORY_GAPS.md](./asset_intake/ASSET_CATEGORY_GAPS.md) created and linked.
- [`imports/staging/asset_intake/`](../imports/staging/asset_intake/), [`imports/normalized/`](../imports/normalized/), [`imports/manifests/`](../imports/manifests/), [`imports/reports/`](../imports/reports/), and [`third_party/github_assets/`](../third_party/github_assets/) scaffolded and referenced.

---

## Findings That Change Prioritization

These findings revise the execution order compared to earlier audit passes:

- **[save_runtime.cpp](../engine/core/save/save_runtime.cpp)** should be treated as a verification-and-alignment target, not a full rewrite target. It has real file I/O, recovery-tier handling, and metadata hydration already.
- **[audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp)** should be split into two tracks: status honesty and lifecycle correctness now (Phase 1–2), richer playback semantics later.
- **The QuickJS lane** should be explicitly scoped as fixture-backed compat scaffolding until real runtime integration exists. Treating it as a runtime while it is a test harness inflates every compat claim that depends on it.
- **Diagnostics debt** is not just a visibility problem — it is a state freshness and truthfulness problem. Treat it as such.
- **Plugin API and cloud-sync surfaces** are documentation-truth and export-surface debt, not silently deferrable "future work." They need honest labels now, even if real routing comes later.
- **Native-absorption and PGMMV roadmap expansion** is not a sidecar planning exercise once it lands in-repo. It becomes remediation-governed truthfulness work immediately and must be normalized before wide execution begins.

---

## Remediation Principles

These principles govern every remediation decision. When in doubt, refer back to them.

1. No subsystem should be marked `FULL` unless the public bridge calls real behavior end to end.
2. A feature is not landed unless it is built, test-registered, and reachable from a real runtime or editor entry point.
3. Exported or documented interfaces must not look production-ready if they still route to scratch-state placeholders.
4. UI summaries and exported diagnostics must reflect what the UI can actually render and keep current.
5. Tests should prove live behavior where product claims are strong. Placeholder-normalizing tests must not be mistaken for closure.
6. Any remaining placeholder behavior must be documented as an intentional waiver. Silent acceptance of placeholder behavior is not done.
7. New roadmap scope is not canonical until remediation, status, and roadmap docs agree on phase placement, truthfulness language, and release gates.
8. A test that asserts placeholder or scaffolding behavior must be explicitly labeled as such. It may complement, but never substitute for, a test that asserts live product behavior.

---

## Phase Plan

### Phase 0 — Restore Build Integrity and Baseline Truth

**Goal:** Get the repo compiling cleanly. Stop documentation from assuming closure while the build is broken.

**Depends on:** Nothing. This phase is the prerequisite for all others.

**Effort estimate:** Small–Medium (hours to a day or two, depending on declaration conflict complexity).

**Scope:**
- Fix active compile blockers in [global_state_hub.h](../engine/core/global_state_hub.h) (see P0-01).
- Align the chatbot and map-scene interfaces in [map_scene.cpp](../engine/core/scene/map_scene.cpp), [map_scene.h](../engine/core/scene/map_scene.h), and [chatbot_component.h](../engine/core/message/chatbot_component.h).
- Remove undefined draw-path variables in [map_scene.cpp](../engine/core/scene/map_scene.cpp).
- Add a mandatory CI build gate for `urpg_core` and registered test targets.

**Exit criteria:**
- `urpg_core` builds locally and in CI.
- CI build gate is active and enforced.
- The map/chat path compiles against real, current interfaces.

**Related documentation:** [README](../README.md), [PLAN](../PLAN.md), [WORKLOG](../WORKLOG.md), [RELEASE_CHECKLIST](../RELEASE_CHECKLIST.md), [RISKS](../RISKS.md), [Master Blueprint](../URPG_Blueprint_v3_1_Integrated.md).

---

### Phase 1 — Make Status Labels and Surface Claims Honest

**Goal:** Stop overstating compat, editor, export, and completion coverage. Every label and doc claim should be a reliable signal, not noise.

**Depends on:** Phase 0 (build must be green before label audits are meaningful).

**Effort estimate:** Medium (systematic audit across multiple files; no new behavior, only honest labeling).

**Scope:**
- Audit all `CompatStatus::FULL` claims in [quickjs_runtime.cpp](../runtimes/compat_js/quickjs_runtime.cpp), [data_manager.cpp](../runtimes/compat_js/data_manager.cpp), [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp), [window_compat.cpp](../runtimes/compat_js/window_compat.cpp), [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp), and [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp) (see P1-02).
- Downgrade to `PARTIAL` or `STUB` wherever behavior is fixture-backed, placeholder-backed, or disconnected from engine state.
- Explicitly scope [plugin_api.cpp](../engine/core/editor/plugin_api.cpp) and [cloud_service.h](../engine/core/social/cloud_service.h) as stub or scaffolding surfaces (see P2-07, P2-08).
- Align [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md) and AI-facing docs with the revised labels.

**Exit criteria:**
- No method or exported surface is labeled production-ready while routing to a disconnected placeholder.
- Compat and diagnostics reports are trustworthy planning inputs.
- Docs stop conflating fixture scaffolding with live runtime support.
- Cloud sync and plugin API docs reflect stub status.

**Related documentation:** [PROGRAM_COMPLETION_STATUS](./PROGRAM_COMPLETION_STATUS.md), [DEVELOPMENT_KICKOFF](./DEVELOPMENT_KICKOFF.md), [KNOWN_BREAK_WAIVERS](./KNOWN_BREAK_WAIVERS.md), [EXTERNAL_FEATURE_UPGRADE_SHORTLIST_2026-04-15](./EXTERNAL_FEATURE_UPGRADE_SHORTLIST_2026-04-15.md), [AI_COPILOT_GUIDE](./AI_COPILOT_GUIDE.md), [AI_SUBSYSTEM_CLOSURE_CHECKLIST](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md).

---

### Phase 2 — Fix Correctness and Runtime Closure in the Highest-Value Paths

**Goal:** Close the runtime debt that causes incorrect behavior, resource leaks, or feature failure even after the build is green.

**Depends on:** Phase 0 (build must be green to verify fixes). Phase 1 is recommended first (honest labels clarify scope).

**Effort estimate:** Large (multiple workstreams; each can proceed independently after Phase 0).

#### Workstream 2.1 — QuickJS Scope Clarity (see P1-01)
- Either integrate a real QuickJS execution path in [quickjs_runtime.cpp](../runtimes/compat_js/quickjs_runtime.cpp), or document it explicitly as a fixture/runtime-contract harness.

#### Workstream 2.2 — Battle Correctness (see P1-04)
- Fix `checkTurnCondition()` cadence semantics in [battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp).
- Battle reward distribution, switch checks, and battle-event cadence/switch coverage are now closed in the compat lane; troop setup and drop logic remain the open battle slices.

#### Workstream 2.3 — Audio Lifecycle Correctness (see P1-03)
- Fix SE channel lifetime cleanup in [audio_manager.cpp](../runtimes/compat_js/audio_manager.cpp).
- Deterministic duck/unduck, mix-scaling, current-BGM API observability, and live compat bindings are covered, but the lane remains harness-backed `PARTIAL` behavior rather than live-backend parity.

#### Workstream 2.4 — Data and Window Runtime Closure (see P1-02)
- `loadDatabase()` in [data_manager.cpp](../runtimes/compat_js/data_manager.cpp) is no longer empty or mock; it seeds compat containers that focused tests now exercise directly.
- [window_compat.cpp](../runtimes/compat_js/window_compat.cpp) is still partial because there is no pixel buffer, but `Window_Base::contents()` lifecycle truthfulness is now backed by explicit deterministic-handle tests.

#### Workstream 2.5 — Menu Round-Trip Closure (see P2-05)
- Implement real serialization in [menu_serializer.cpp](../engine/core/ui/menu_serializer.cpp).
- Ensure menu authoring supports a complete import/export/reload cycle.

**Exit criteria:**
- Battle reward distribution, battle-event cadence, and switch checks are covered by focused compat tests.
- Audio SE channels do not grow unbounded under repeated playback, and the lane stays honestly labeled `PARTIAL`.
- `loadDatabase()` and `Window_Base::contents()` lifecycle behavior are backed by explicit tests; the remaining window pixel-buffer gap stays documented.
- Menu definitions round-trip through serialization.
- Runtime bridges act on real engine state or are honestly labeled otherwise.
- Verification gate for this task: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleManager:|Window_Base contents lifecycle allocates and rotates deterministic handles|DataManager loadDatabase populates seeded database containers|AudioManager:"`

**Related documentation:** [BATTLE_CORE_NATIVE_SPEC](./BATTLE_CORE_NATIVE_SPEC.md), [SAVE_DATA_CORE_NATIVE_SPEC](./SAVE_DATA_CORE_NATIVE_SPEC.md), [MESSAGE_TEXT_CORE_NATIVE_SPEC](./MESSAGE_TEXT_CORE_NATIVE_SPEC.md), [UI_MENU_CORE_NATIVE_SPEC](./UI_MENU_CORE_NATIVE_SPEC.md), [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST](./WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md), [WAVE2_AUDIO_STATE_SYNC_PLAN](./WAVE2_AUDIO_STATE_SYNC_PLAN.md).

---

### Phase 3 — Make Editor and Diagnostics Surfaces Truthful, Stateful, and Usable

**Goal:** Bring the editor/diagnostics surface in line with what the workspace claims to offer, and keep those views fresh as runtime state changes.

**Depends on:** Phase 0 (build). Phases 1 and 2 are recommended prior to maximize the usefulness of real runtime data in the workspace.

**Effort estimate:** Medium–Large.

**Scope:**
- Maintain honest render reachability for [event_authority_panel.cpp](../editor/diagnostics/event_authority_panel.cpp), the Audio tab, and the Migration Wizard tab, then continue productizing those panels beyond snapshot-backed bodies (see P2-03).
- Implement `clearMenuRuntime()`, `clearAudioRuntime()`, and `clearAbilityRuntime()` in [diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) (see P2-02).
- Replace one-shot bindings and simulated projections in [audio_inspector_model.h](../editor/audio/audio_inspector_model.h) and [migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h).
- Verify menu and compat diagnostics surfaces are actionable, not just data-backed.
- Remove or relabel tabs that do not yet have usable bodies.

**Exit criteria:**
- Every tab counted in workspace summaries renders a real, non-empty panel body.
- Diagnostics state is reset correctly on runtime detach.
- Export snapshots match the actual surfaced workspace.
- Diagnostics tests assert the current workspace shape and runtime-clearing behavior.

**Related documentation:** [AI_COPILOT_GUIDE](./AI_COPILOT_GUIDE.md), [AI_SUBSYSTEM_CLOSURE_CHECKLIST](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md), [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST](./WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md), [WAVE2_AUDIO_STATE_SYNC_PLAN](./WAVE2_AUDIO_STATE_SYNC_PLAN.md), [NATIVE_FEATURE_ABSORPTION_PLAN](./NATIVE_FEATURE_ABSORPTION_PLAN.md).

---

### Phase 4 — Reconcile Build Graph, Tests, Docs, Exported Surfaces, and Canonical Planning

**Goal:** Eliminate drift between what exists in source control, what is compiled, what is tested, what is exported, and what the docs claim.

**Depends on:** Phases 0–3 (surface area must be stabilized before a final reconciliation is accurate).

**Effort estimate:** Medium (primarily audit and cleanup rather than new implementation).

**Scope:**
- Make an explicit productized vs. incubating decision for `engine/core/presentation/*` and `editor/spatial/*` and update [CMakeLists.txt](../CMakeLists.txt) accordingly (see P2-06).
- Register or delete [test_spatial_editor.cpp](../tests/unit/test_spatial_editor.cpp) with rationale (see P2-04).
- Keep the registered `test_compat_report_panel.cpp` surface and prevent reintroduction of stale duplicate compat-report test files (see P2-04).
- Align plugin/cloud/export documentation with the actual routing state of [plugin_api.h](../engine/core/editor/plugin_api.h), [plugin_api.cpp](../engine/core/editor/plugin_api.cpp), and [cloud_service.h](../engine/core/social/cloud_service.h) (see P2-07, P2-08).
- Stand up external repository intake governance from [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) so future compat/import/export/editor improvements are sourced through an explicit legal, fixture, and adoption program rather than ad hoc repo ingestion (see P3-02).
- Stand up private-use asset intake governance from [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) so editor/runtime realism improvements, placeholder replacement, and vertical-slice asset upgrades are sourced through explicit staging, provenance, and promotion rules rather than ad hoc asset drops (see P3-03).
- Reconcile the newly added PGMMV/native-absorption roadmap inputs with the canonical remediation/status/roadmap set so the repo does not carry parallel execution authorities (see P2-10).

#### Workstream 4.1 — External Repository Intake Governance (see P3-02)
- Treat [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) as the detailed execution plan for external intake.
- Create and maintain the canonical intake artifacts it calls for:
  - `docs/external-intake/repo-watchlist.md`
  - `docs/external-intake/license-matrix.md`
  - `docs/external-intake/repo-audit-template.md`
  - fixture and attribution schemas
  - `docs/external-intake/urpg_feature_adoption_matrix.md`
- Require every one of the twelve repos in the intake set to reach an explicit adopt/defer/ignore-style disposition before production-candidate integration work begins.
- Use the intake program to feed later remediation/product lanes:
  - compat corpus expansion
  - legacy importer work
  - localization extraction
  - exporter upgrades
  - asset/reference pack handling
  - editor UX reference mining

#### Workstream 4.2 — Private-Use Asset Intake Governance (see P3-03)
- Treat [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) as the detailed execution plan for private-use asset intake.
- Create and maintain the canonical intake artifacts it calls for:
  - `third_party/github_assets/*`
  - `imports/staging/asset_intake/*`
  - `imports/normalized/*`
  - `imports/manifests/asset_sources/*`
  - `imports/manifests/asset_bundles/*`
  - `imports/reports/asset_intake/*`
  - `docs/asset_intake/ASSET_SOURCE_REGISTRY.md`
  - `docs/asset_intake/ASSET_PROMOTION_GUIDE.md`
  - `docs/asset_intake/ASSET_CATEGORY_GAPS.md`
- Require all source repos in the asset intake program to reach an explicit handling path:
  - direct-ingest repos enter controlled staging, audit, normalization, and selective promotion
  - discovery repos become tracked sourcing backlog inputs, not direct-import lanes
- Use the intake program to feed fast-win product lanes without losing provenance:
  - editor UI sound feedback
  - prototype sprite/content replacement
  - curated fantasy-environment vertical slices
  - future asset gap reporting and promotion decisions

#### Workstream 4.3 — Canonical Roadmap Integration And Planning Governance (see P2-10)
- Treat [URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md](./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md) and [URPG_PGMMV_SUPPORT_PLAN.md](./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md) as execution-detail inputs that must be absorbed into canonical docs, not left as parallel planning authorities.
- Normalize the phase story across this remediation hub, [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md), and [NATIVE_FEATURE_ABSORPTION_PLAN.md](./NATIVE_FEATURE_ABSORPTION_PLAN.md) so new lanes inherit the same build/test/doc/release truth gates.
- Pull the newly added roadmap obligations into canonical remediation ownership:
  - PGMMV intake and migration truthfulness
  - plugin-evidence scanning and native replacement mapping
  - HD-2D / 2.5D productization
  - authoring UX parity
  - accessibility and localization
  - item-instance / equipment individuality and RPG glue systems
- Record which plan files remain authoritative, which become execution-detail references, and which are superseded once the canonical docs absorb their content.
- Make documentation alignment explicit for any future plan doc that adds phases, tracks, or acceptance criteria.

**Exit criteria:**
- Docs no longer describe unbuilt, unregistered, or stub-exported work as shipped.
- No stale duplicate tests remain.
- Active tests, workspace surfaces, and exported APIs agree with current product reality.
- All decisions about presentation/spatial status are recorded (ADR if scope-changing).
- External repository intake is governed by recorded legal and technical dispositions rather than ad hoc copying or local-only experiments.
- Private-use asset intake is governed by recorded provenance, staging, normalization, and promotion rules rather than ad hoc drops into product-facing paths.
- Newly added roadmap/support plans have been absorbed into the canonical remediation/status/roadmap stack or explicitly marked as superseded/reference-only inputs.
- Compat exit checklist artifact ([docs/COMPAT_EXIT_CHECKLIST.md](./COMPAT_EXIT_CHECKLIST.md)) is published and satisfies part of the documentation-alignment exit criteria for the compat lane.

**Related documentation:** [README](../README.md), [Master Blueprint](../URPG_Blueprint_v3_1_Integrated.md), [Archive Index](./archive/README.md), [NATIVE_FEATURE_ABSORPTION_PLAN](./NATIVE_FEATURE_ABSORPTION_PLAN.md), [PROGRAM_COMPLETION_STATUS](./PROGRAM_COMPLETION_STATUS.md), [URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18](./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md), [URPG_PGMMV_SUPPORT_PLAN](./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md), [URPG_repo_intake_plan](../URPG_repo_intake_plan.md), [URPG_private_asset_intake_plan](../URPG_private_asset_intake_plan.md), [presentation/schema_changelog](./presentation/schema_changelog.md), [presentation/SPATIAL_EDITOR_TOOLS](./presentation/SPATIAL_EDITOR_TOOLS.md), [presentation/performance_budgets](./presentation/performance_budgets.md), [presentation/test_matrix/MapScene_Contract](./presentation/test_matrix/MapScene_Contract.md), [presentation/test_matrix/MenuScene_Contract](./presentation/test_matrix/MenuScene_Contract.md), [presentation/test_matrix/BattleScene_Contract](./presentation/test_matrix/BattleScene_Contract.md), [presentation/test_matrix/OverlayUI_Contract](./presentation/test_matrix/OverlayUI_Contract.md).

**Architecture Decision Records:** [ADR-001](./adr/ADR-001.md) through [ADR-010-presentation-completion](./adr/ADR-010-presentation-completion.md).

---

### Phase 5 — Harden Ownership, Concurrency, and Performance

**Goal:** Pay down the structural debt that will keep the system brittle even after feature closure.

**Depends on:** Phases 0–4 (all functional and surface debt should be resolved first so hardening targets stable, real code).

**Effort estimate:** Medium.

**Scope:**
- Review and fix worker-thread callback delivery in [plugin_manager.cpp](../runtimes/compat_js/plugin_manager.cpp) (see P2-09).
- Replace the hidden static `AudioCore` in [map_scene.cpp](../engine/core/scene/map_scene.cpp) with an injected dependency (see P3-01).
- Reduce per-frame full render-layer rebuilds in [map_scene.cpp](../engine/core/scene/map_scene.cpp) (see P3-01).
- Permit production-candidate external repository adoption only after the Phase 4 intake-governance gates are active, using wrappers and URPG-owned facades rather than direct architecture absorption (see P3-02).
- Permit promoted private-use assets to enter editor/runtime product lanes only after the Phase 4 asset-intake governance gates are active, with provenance, bundle/source manifests, and promotion records intact (see P3-03).

**Exit criteria:**
- Async callback paths have documented and enforced thread-affinity rules.
- Runtime services are passed through clear ownership boundaries — no static-local service instances in scene code.
- Per-frame render work is proportional to scene delta.

**Related documentation:** [presentation/performance_budgets](./presentation/performance_budgets.md), [RISKS](../RISKS.md), [RELEASE_CHECKLIST](../RELEASE_CHECKLIST.md).

---

## Phase Summary and Dependencies

| Phase | Goal | Depends On | Effort | Primary Owner |
|-------|------|-----------|--------|---------------|
| 0 | Build integrity | — | Small–Med | Engine/core |
| 1 | Honest labels | Phase 0 | Medium | Compat/runtime + Tech lead |
| 2 | Correctness & runtime closure | Phase 0 (Phase 1 recommended) | Large | Compat/runtime |
| 3 | Editor/diagnostics truth | Phases 0–2 recommended | Med–Large | Editor |
| 4 | Build/test/doc reconciliation + roadmap alignment + external and private asset intake governance | Phases 0–3 | Medium | Tech lead/release owner |
| 5 | Ownership & concurrency hardening | Phases 0–4 | Medium | Engine/core + Compat/runtime |

**Dependency diagram:**

```
Phase 0 ──► Phase 1 ──► Phase 2 ──► Phase 3 ──► Phase 4 ──► Phase 5
              │                         │
              └─────────────────────────┘
              (Phase 1 recommended before Phase 2,
               both recommended before Phase 3)
```

Workstreams within Phase 2 (2.1–2.5) are **independent** and can proceed in parallel after Phase 0 is complete.

---

## Verification Plan

Every remediation phase must update the **verification surface** — not just the implementation.

### Build Verification

- Build `urpg_core` locally and in CI on every merge.
- Build all registered test targets as part of the CI gate (no registered test should be skipped by default).

### Required Regression Tests

Add or repair focused regressions for:

| Subject | Test file | Coverage target |
|---------|-----------|-----------------|
| Build blockers (map/chat/global-state path) | [test_global_state.cpp](../tests/unit/test_global_state.cpp) | Compiles and links without errors |
| Battle turn-condition cadence | [test_battlemgr.cpp](../tests/unit/test_battlemgr.cpp) | Span 1 and span 2 cadence; first-threshold edge case |
| SE channel cleanup and audio lifecycle | [test_audio_manager.cpp](../tests/unit/test_audio_manager.cpp) | Channel count stable under repeated SE playback |
| Diagnostics workspace tab count and render availability | [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) | Correct count; all tabs render; runtime clear resets state |
| Menu serialization round-trip | [test_menu_core.cpp](../tests/unit/test_menu_core.cpp) | Serialize → deserialize → structural equivalence |
| QuickJS scope truth | [test_quickjs_runtime.cpp](../tests/unit/test_quickjs_runtime.cpp) | Reflects whether fixture-backed or real-runtime-backed |
| Plugin API routing truth | [test_plugin_api.cpp](../tests/unit/test_plugin_api.cpp) | Methods are labeled correctly; no silent mock behavior |
| Cloud service scoping | — | `CloudServiceStub` annotation visible in header |

### Candidate Test Files

- [test_global_state.cpp](../tests/unit/test_global_state.cpp)
- [test_quickjs_runtime.cpp](../tests/unit/test_quickjs_runtime.cpp)
- [test_data_manager.cpp](../tests/unit/test_data_manager.cpp)
- [test_battlemgr.cpp](../tests/unit/test_battlemgr.cpp)
- [test_audio_manager.cpp](../tests/unit/test_audio_manager.cpp)
- [test_window_compat.cpp](../tests/unit/test_window_compat.cpp)
- [test_menu_core.cpp](../tests/unit/test_menu_core.cpp)
- [test_menu_inspector_model.cpp](../tests/unit/test_menu_inspector_model.cpp)
- [test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp)
- [test_event_authority_panel.cpp](../tests/unit/test_event_authority_panel.cpp)
- [test_audio_inspector.cpp](../tests/unit/test_audio_inspector.cpp)
- [test_migration_wizard.cpp](../tests/unit/test_migration_wizard.cpp)
- [test_spatial_editor.cpp](../tests/unit/test_spatial_editor.cpp) *(register or delete — see P2-04)*

---

## Risk Register

These are risks **to the remediation effort itself**, distinct from the technical findings above.

| # | Risk | Likelihood | Impact | Mitigation |
|---|------|-----------|--------|-----------|
| R1 | Phase 0 compile fixes reveal deeper interface mismatches requiring architectural decisions | Medium | High | Time-box investigation to one sprint; escalate to tech lead if scope expands beyond header/call-site fixes. |
| R2 | QuickJS real-runtime integration (WS 2.1) is much larger than estimated, blocking other Phase 2 work | Medium | Medium | Decide Path A vs. Path B (P1-01) in Phase 1 before starting Phase 2. Path B has near-zero implementation cost. |
| R3 | Status label downgrades (Phase 1) cause stakeholder concern about perceived regression | Medium | Low | Frame as accuracy improvements, not capability regressions. Prepare a one-page summary of "what this changes in practice." |
| R4 | Diagnostics runtime binding fixes (Phase 3) expose further stale-state bugs not currently visible | Medium | Medium | Treat as expected discovery, not scope creep. Log new findings in a Phase 3 addendum. |
| R5 | Plugin manager threading issues (P2-09) are more widespread than current audit reveals | Low | High | Run a targeted thread-safety audit of all plugin callback invocation sites before closing Phase 5. |
| R6 | Documentation alignment tasks (Phase 4) are deprioritized after feature work lands | High | Medium | Require documentation updates as part of the Definition of Done for every finding closure (see below). |
| R7 | New planning documents (roadmaps, support plans, ADRs) are added faster than they can be absorbed into the canonical doc set, re-introducing the parallel-planning-authority problem closed by P2-10 | Medium | Medium | Require any new plan doc to include a `canonical-placement` field in its header (absorbed / reference-only / superseded) before merge. Enforce at PR review. |
| R8 | QuickJS Path A (real runtime) is chosen late in Phase 2, after compat fixture tests have been written against Path B semantics, requiring significant test rework | Low | High | Document the Path A vs. Path B decision as a time-boxed milestone at the start of Phase 2, not an open question deferrable to mid-phase. Assign the decision explicitly to the Compat/runtime tech lead. |

---

## Documentation Alignment Tasks

The following documents must be updated whenever remediation work lands in the area they describe. Documentation updates are **not optional post-merge tasks** — they are part of the Definition of Done.

- [README](../README.md)
- [Master Blueprint](../URPG_Blueprint_v3_1_Integrated.md)
- [PROGRAM_COMPLETION_STATUS](./PROGRAM_COMPLETION_STATUS.md)
- [NATIVE_FEATURE_ABSORPTION_PLAN](./NATIVE_FEATURE_ABSORPTION_PLAN.md)
- [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST](./WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)
- [WAVE2_AUDIO_STATE_SYNC_PLAN](./WAVE2_AUDIO_STATE_SYNC_PLAN.md)
- [AI_COPILOT_GUIDE](./AI_COPILOT_GUIDE.md)
- [AI_SUBSYSTEM_CLOSURE_CHECKLIST](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md)
- [URPG_repo_intake_plan](../URPG_repo_intake_plan.md)
- [URPG_private_asset_intake_plan](../URPG_private_asset_intake_plan.md)
- [Archive README](./archive/README.md)
- [URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18](./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md)
- [URPG_PGMMV_SUPPORT_PLAN](./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md)
- `docs/external-intake/*`
- `docs/asset_intake/*`
- Subsystem specs under [docs](.)
- [WORKLOG](../WORKLOG.md)
- This remediation plan

---

## Documentation File Tree

This is the current documentation and planning file tree for the repo. It is intentionally limited to human-facing governance, roadmap, status, specification, checklist, architecture, and archived planning material so remediation work has one linked index for document alignment.

### Repo root

- [README.md](../README.md) — public repo overview, build entrypoints, and top-level documentation index.
- [CONTRIBUTING.md](../CONTRIBUTING.md) — contributor workflow, hygiene rules, and PR expectations.
- [PLAN.md](../PLAN.md) — active presentation-core project plan and task state.
- [RELEASE_CHECKLIST.md](../RELEASE_CHECKLIST.md) — release-readiness checklist with presentation and remediation references.
- [RISKS.md](../RISKS.md) — active risk register derived from the archived presentation architecture plan.
- [WORKLOG.md](../WORKLOG.md) — chronological execution log and documentation sync history.
- [URPG_Blueprint_v3_1_Integrated.md](../URPG_Blueprint_v3_1_Integrated.md) — master product/technical blueprint.
- [URPG_Blueprint_v3_1_Integrated.docx](../URPG_Blueprint_v3_1_Integrated.docx) — binary Word copy of the master blueprint.
- [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) — external-repository intake execution plan governed under P3-02 / Phase 4.1.
- [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) — private-use asset intake execution plan governed under P3-03 / Phase 4.2.

### `docs/`

- [AI_COPILOT_GUIDE.md](./AI_COPILOT_GUIDE.md) — AI copilot usage and in-tree capability guidance.
- [AI_SUBSYSTEM_CLOSURE_CHECKLIST.md](./AI_SUBSYSTEM_CLOSURE_CHECKLIST.md) — closure checklist for the AI subsystem.
- [BATTLE_CORE_NATIVE_SPEC.md](./BATTLE_CORE_NATIVE_SPEC.md) — battle-core native ownership specification.
- [COMPAT_EXIT_CHECKLIST.md](./COMPAT_EXIT_CHECKLIST.md) — compat-lane exit criteria and sign-off checklist.
- [DEVELOPMENT_KICKOFF.md](./DEVELOPMENT_KICKOFF.md) — execution kickoff status and canonical-doc entry guidance.
- [EXTERNAL_FEATURE_UPGRADE_SHORTLIST_2026-04-15.md](./EXTERNAL_FEATURE_UPGRADE_SHORTLIST_2026-04-15.md) — external feature upgrade shortlist.
- [EXTERNAL_FEATURE_UPGRADE_TICKETS_2026-04-15.md](./EXTERNAL_FEATURE_UPGRADE_TICKETS_2026-04-15.md) — ticket breakdown for external feature upgrades.
- [HUGGINGFACE_CURATED_IMPORTS.md](./HUGGINGFACE_CURATED_IMPORTS.md) — curated Hugging Face import/governance notes.
- [KNOWN_BREAK_WAIVERS.md](./KNOWN_BREAK_WAIVERS.md) — documented waivers for accepted breaks or temporary deviations.
- [MESSAGE_TEXT_CORE_NATIVE_SPEC.md](./MESSAGE_TEXT_CORE_NATIVE_SPEC.md) — message/text native ownership specification.
- [NATIVE_FEATURE_ABSORPTION_PLAN.md](./NATIVE_FEATURE_ABSORPTION_PLAN.md) — canonical product roadmap.
- [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md) — canonical latest-status snapshot and remaining work checklist.
- [SAVE_DATA_CORE_NATIVE_SPEC.md](./SAVE_DATA_CORE_NATIVE_SPEC.md) — save/data native ownership specification.
- [TECHNICAL_DEBT_REMEDIATION_PLAN.md](./TECHNICAL_DEBT_REMEDIATION_PLAN.md) — canonical remediation, truthfulness, governance, and documentation-alignment hub.
- [UI_MENU_CORE_NATIVE_SPEC.md](./UI_MENU_CORE_NATIVE_SPEC.md) — UI/menu native ownership specification.
- [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md](./WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md) — Wave 1 subsystem closure checklist.
- [WAVE2_AUDIO_STATE_SYNC_PLAN.md](./WAVE2_AUDIO_STATE_SYNC_PLAN.md) — Wave 2 audio state synchronization plan.

### `docs/adr/`

- [ADR-001.md](./adr/ADR-001.md) — architecture decision record.
- [ADR-002.md](./adr/ADR-002.md) — architecture decision record.
- [ADR-003.md](./adr/ADR-003.md) — architecture decision record.
- [ADR-004.md](./adr/ADR-004.md) — architecture decision record.
- [ADR-005.md](./adr/ADR-005.md) — architecture decision record.
- [ADR-006.md](./adr/ADR-006.md) — architecture decision record.
- [ADR-007.md](./adr/ADR-007.md) — architecture decision record.
- [ADR-008.md](./adr/ADR-008.md) — architecture decision record.
- [ADR-009.md](./adr/ADR-009.md) — architecture decision record.
- [ADR-010.md](./adr/ADR-010.md) — architecture decision record.
- [ADR-010-presentation-completion.md](./adr/ADR-010-presentation-completion.md) — presentation completion ADR.
- [ADR-011-presentation-spatial-status.md](./adr/ADR-011-presentation-spatial-status.md) — presentation/spatial status and truthfulness ADR.

### `docs/archive/`

- [README.md](./archive/README.md) — archive index and canonical-vs-archived authority guidance.

### `docs/archive/planning/`

- [URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md](./archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md) — archived planning annex for PGMMV/native absorption v2.
- [URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md](./archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md) — superseded native-absorption planning input retained for traceability.
- [URPG_PGMMV_SUPPORT_PLAN.md](./archive/planning/URPG_PGMMV_SUPPORT_PLAN.md) — archived PGMMV support planning annex.
- [urpg_first_class_presentation_architecture_plan_v2.md](./archive/planning/urpg_first_class_presentation_architecture_plan_v2.md) — archived presentation architecture source plan.

### `docs/asset_intake/`

- [ASSET_CATEGORY_GAPS.md](./asset_intake/ASSET_CATEGORY_GAPS.md) — asset-gap inventory and sourcing gaps.
- [ASSET_PROMOTION_GUIDE.md](./asset_intake/ASSET_PROMOTION_GUIDE.md) — promotion/staging guidance for private-use assets.
- [ASSET_SOURCE_REGISTRY.md](./asset_intake/ASSET_SOURCE_REGISTRY.md) — asset-source provenance registry.

### `docs/external-intake/`

- [license-matrix.md](./external-intake/license-matrix.md) — external-repo license tracking matrix.
- [repo-audit-template.md](./external-intake/repo-audit-template.md) — audit template for external repositories.
- [repo-watchlist.md](./external-intake/repo-watchlist.md) — tracked external repositories and intake watchlist.
- [urpg_feature_adoption_matrix.md](./external-intake/urpg_feature_adoption_matrix.md) — disposition matrix for external feature adoption.

### `docs/presentation/`

- [README.md](./presentation/README.md) — presentation-docs hub.
- [performance_budgets.md](./presentation/performance_budgets.md) — presentation performance budgets and targets.
- [schema_changelog.md](./presentation/schema_changelog.md) — presentation schema change log.
- [SPATIAL_EDITOR_TOOLS.md](./presentation/SPATIAL_EDITOR_TOOLS.md) — spatial editor tools documentation.
- [VALIDATION.md](./presentation/VALIDATION.md) — presentation validation commands and gate definitions.

### `docs/presentation/test_matrix/`

- [README.md](./presentation/test_matrix/README.md) — presentation contract-test matrix index.
- [BattleScene_Contract.md](./presentation/test_matrix/BattleScene_Contract.md) — battle-scene contract.
- [MapScene_Contract.md](./presentation/test_matrix/MapScene_Contract.md) — map-scene contract.
- [MenuScene_Contract.md](./presentation/test_matrix/MenuScene_Contract.md) — menu-scene contract.
- [OverlayUI_Contract.md](./presentation/test_matrix/OverlayUI_Contract.md) — overlay UI contract.

### `docs/superpowers/plans/`

- [2026-04-17-migration-wizard-rendered-workflow.md](./superpowers/plans/2026-04-17-migration-wizard-rendered-workflow.md) — generated implementation plan retained for execution traceability.

---

## Ownership Matrix

| Area | Responsible Team | Notes |
|------|-----------------|-------|
| Build blockers, menu serialization, plugin export routing, map-scene ownership cleanup | Engine/core maintainers | P0-01, P2-05, P2-07, P3-01 |
| QuickJS, DataManager, BattleManager, WindowCompat, AudioManager, PluginManager | Compat/runtime maintainers | P1-01, P1-02, P1-03, P1-04, P2-09 |
| Diagnostics workspace, Event Authority, Audio inspector, Migration Wizard, compat surfaces | Editor maintainers | P2-01, P2-02, P2-03 |
| Completion-state truthfulness, waiver review, doc alignment, exported-surface scoping, presentation/spatial decision | Tech lead or release owner | P2-06, P2-07, P2-08, Phase 4 overall |
| Test registration and stale test cleanup | Respective subsystem maintainer + release owner for CMakeLists.txt | P2-04 |
| Canonical roadmap integration, phase vocabulary, and PGMMV/native-absorption truthfulness | Tech lead or release owner + affected domain leads | P2-10, Phase 4.3 |
| External repository intake governance, legal disposition, fixture/adoption quarantine | Tech lead/release owner + content/pipeline lead + affected subsystem owner | P3-02, Phase 4.1 |
| Private-use asset intake governance, provenance retention, staging/normalization/promotion discipline | Tech lead/release owner + content/pipeline lead + asset/tooling owners + affected editor/runtime owner | P3-03, Phase 4.2 |

---

## Definition of Done

A remediation item is **done only when all of the following are true**:

- [ ] The code compiles cleanly (no suppressed warnings that mask the fix).
- [ ] The relevant tests are registered in [CMakeLists.txt](../CMakeLists.txt) and passing.
- [ ] If existing tests were changed, the rationale for the change is recorded in the commit message or PR description.
- [ ] Status labels and completion docs match real behavior — updated in the **same commit or PR** as the implementation.
- [ ] Surfaced editor/runtime functionality is actually reachable in the product via a documented entry point.
- [ ] Exported, plugin, and cloud-facing surfaces are scoped honestly in headers and docs.
- [ ] Any new plan or roadmap document introduced by the change is either absorbed into the canonical remediation/status/roadmap set or explicitly marked as execution-detail/reference-only in the same PR.
- [ ] External code, fixtures, and assets are not adopted unless they have a recorded disposition under [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) and the corresponding intake artifacts.
- [ ] Private-use assets are not promoted into editor/runtime product lanes unless they have recorded provenance, source/bundle manifests, and promotion records under [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) and the corresponding intake artifacts.
- [ ] Any remaining placeholder behavior is documented as an explicit, intentional waiver — not silently accepted as complete.
- [ ] No new P0 or P1 findings are introduced by the change (or if they are, they are logged as new findings in this document).
- [ ] The tech lead or release owner has signed off that documentation alignment is complete for the changed area.

---

## Recommended Execution Order

1. **Phase 0** — Restore build integrity and baseline truth. *(Prerequisite for everything.)*
2. **Phase 1** — Make status labels and surface claims honest. *(Clarifies scope for Phase 2.)*
3. **Phase 2** — Fix correctness and runtime closure in the highest-value paths. *(Workstreams 2.1–2.5 can run in parallel.)*
4. **Phase 3** — Make editor and diagnostics surfaces truthful, stateful, and usable.
5. **Phase 4** — Reconcile build graph, tests, docs, exported surfaces, and stand up both external repository intake governance and private-use asset intake governance.
6. **Phase 5** — Harden ownership, concurrency, performance, and only then admit production-candidate external integrations and promoted private-use asset lanes through governed wrappers/facades and provenance-preserving promotion paths.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-16 | Initial remediation hub created from first-pass audit. |
| 2026-04-16 | Document rewritten after second-pass audit: new findings added, priorities rebalanced, stubbed systems distinguished from partially functional systems. |
| 2026-04-16 | Document rewritten after third-pass audit: stale-state diagnostics debt, placeholder export-surface debt, and documentation/test drift findings added. |
| 2026-04-16 | Fourth-pass improvement: added Table of Contents, Priority Legend, per-finding structured format (Impact/Root Cause/Action/Owner/Exit Criteria), Phase Summary and Dependency table, Risk Register, Ownership Matrix, effort estimates, explicit out-of-scope notes, and strengthened Definition of Done. |
| 2026-04-16 | Integrated the external repository intake program from [URPG_repo_intake_plan.md](../URPG_repo_intake_plan.md) into findings, Phase 4 governance work, ownership, documentation alignment, and Definition of Done. |
| 2026-04-16 | Integrated the private-use asset intake program from [URPG_private_asset_intake_plan.md](../URPG_private_asset_intake_plan.md) into findings, Phase 4 governance work, ownership, documentation alignment, and Definition of Done. |
| 2026-04-17 | Agent swarm pass 1: compat status honesty (P1-02), QuickJS scope clarity (P1-01), test/build registration drift fixes (P2-04), intake governance artifact creation (P3-02/P3-03), and documentation alignment (P2-06). |
| 2026-04-17 | Agent swarm pass 2: input manager status honesty (P1-02), migration wizard productization with `rerunSubsystem` (P2-03), data manager runtime closure with real `loadDatabase()` and JS bindings (Phase 2), and doc sync/intake linking (P3-02/P3-03). |
| 2026-04-17 | Agent swarm pass 3: plugin manager and data manager status honesty (P1-02), migration wizard further productization with `clearSubsystemResult` and `getReportJson` (P2-03), presentation/spatial incubation decision and ADR-011 (P2-06). |
| 2026-04-17 | Continued Phase 3 diagnostics productization: event-authority panel gained row navigation, visible-row body export, severity filtering, and mode filtering, with corresponding workspace export and focused regression updates. |
| 2026-04-17 | Local build hardening: constrained MinGW SDL discovery to the active compiler root, forced vendored SDL for MSVC to block MSYS header leakage, added missing standard headers surfaced by stricter toolchains, restored missing testing support headers, and hardened the presentation helper against stale local build trees. |
| 2026-04-18 | Agent swarm pass 4: window compat rendering closure, battle manager runtime closure, message inspector panel productization, compat exit checklist publication. |
| 2026-04-18 | Audio compat closure/documentation sync: deterministic playback-position, duck/unduck, mix-scaling, and live QuickJS AudioManager bindings landed; remediation/status/docs updated to keep the lane honestly `PARTIAL` and deterministic-harness-scoped. |
| 2026-04-18 | Window compat follow-through: landed `Window_Selectable` pointer press/drag/release semantics, drag-scroll, mouse-wheel scrolling, and contents-backed compat bitmap metadata with synchronized dimensions; updated status/remediation docs to reflect the narrower remaining gap. |
| 2026-04-18 | Integrated the newly added PGMMV/native-absorption planning scope into the remediation hub via canonical-planning governance, Phase 4 roadmap-alignment work, documentation-alignment hooks, and Definition-of-Done requirements. |
| 2026-04-18 | Sixth-pass improvement: fixed UTF-8/cp1252 encoding artifacts throughout document; added Finding Status at a Glance dashboard; added R7 (planning-doc sprawl) and R8 (QuickJS path decision timing) to Risk Register; corrected P2-04 root cause description; fixed incomplete test path in Verification Plan; fixed P1-02 source file table entry; added Principle 8 on test coverage honesty; updated Change Log. |
