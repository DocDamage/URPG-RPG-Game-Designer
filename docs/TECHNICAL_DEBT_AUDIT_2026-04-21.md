# Technical Debt Audit

**Status Date:** 2026-04-22
**Document version:** v4
**Supersedes:** v3 (2026-04-22)

This rewrite is based on a fresh code-first audit of the current tree.

This document is an audit snapshot and prioritization aid, not a replacement for the canonical remediation hub in `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`. Where this audit recommends execution order or Definition-of-Done wording, treat it as input to the remediation plan rather than a second source of authority.

---

## Audit Frame

This pass reviewed:

- Stale assembly, editor, plugin, and threading seams
- Render hot-path allocation behavior and renderer completeness
- Memory-growth, ownership, and lifetime-risk patterns
- Compat harness boundaries versus live runtime behavior
- Export, security, and visual-validation realism
- Build, CI, and maintainability hotspots

**Commands run during this pass:**

```
powershell -ExecutionPolicy Bypass -File tools/ci/check_cmake_completeness.ps1
cmake --build --preset dev-debug --target urpg_integration_tests urpg_snapshot_tests urpg_compat_tests urpg_presentation_release_validation
ctest --test-dir build/dev-ninja-debug -N -L "^pr$"
ctest --test-dir build/dev-ninja-debug -N -L "^nightly$"
ctest --test-dir build/dev-ninja-debug -N -L "^weekly$"
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "^urpg_presentation_release_validation$"
ctest --test-dir build/dev-ninja-debug --output-on-failure -R "^Integration:"
```

Targeted `rg` scans for `TODO`, `stub`, `placeholder`, raw ownership, render-command allocation sites, and threading/global-state patterns.

**Known limits of this audit:**

| Gap | Impact | Recommended Next Step |
|---|---|---|
| No ASan / LSan run | Memory errors and leaks not confirmed | Schedule a dedicated sanitizer pass before any memory findings are closed |
| No Valgrind pass | Heap behavior is inspection-based only | Run Valgrind or platform-equivalent heap tooling on a Linux/WSL build once text/rect backend is real |
| No profiler trace | Hot-path cost estimates are structural, not measured | Attach a frame profiler after arena-backed render commands land |

> These are not reasons to defer fixes — they are signals that the current findings may be *undercounting* real issues.

---

## Verification Snapshot

- `check_cmake_completeness.ps1` passes, and the remaining hardcoded whitelist is now limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- Fresh exact-label discovery in the current debug tree enumerates **918** `pr` tests, **18** `nightly` tests, **44** `weekly` tests, and **980** tests total.
- A targeted build of `urpg_integration_tests`, `urpg_snapshot_tests`, `urpg_compat_tests`, and `urpg_presentation_release_validation` succeeds locally.
- Focused execution of `urpg_presentation_release_validation` and all 11 integration tests passes.
- That same targeted build surfaces **11 unique compiler warnings** across presentation translator interfaces, compat window overrides, aggregate initialization, and unused test/audio helpers.

**Marker snapshot (code/docs under active tree, excluding archives/planning mirrors/build artifacts):**

| Marker | Count |
|---|---|
| `TODO` | 20 |
| `placeholder` | 25 |
| `stub` | 28 |
| `not implemented` | 7 |
| `NOT LIVE` | 12 |

**Largest code hotspots:**

| File | Lines | Primary concern |
|---|---|---|
| `tests/compat/test_compat_plugin_failure_diagnostics.cpp` | 4,033 | Scope sprawl |
| `tests/compat/test_compat_plugin_fixtures.cpp` | 2,859 | Scope sprawl |
| `tests/unit/test_diagnostics_workspace.cpp` | 2,376 | Scope sprawl |
| `runtimes/compat_js/window_compat.cpp` | 2,370 | Harness complexity |
| `runtimes/compat_js/plugin_manager.cpp` | 2,350 | Harness complexity |
| `tools/audit/urpg_project_audit.cpp` | 2,183 | Domain mixing |
| `editor/diagnostics/diagnostics_workspace.cpp` | 1,972 | Domain mixing |
| `runtimes/compat_js/battle_manager.cpp` | 1,921 | Harness complexity |
| `runtimes/compat_js/data_manager.cpp` | 1,744 | Harness complexity |

---

## Resolved Debt ✓

These items were open in the previous pass and are now closed. They are recorded here to preserve audit continuity and recognize real progress.

| ID | Title | Resolution |
|---|---|---|
| TD-R01 | AudioManager SE-channel leak | `runtimes/compat_js/audio_manager.cpp:737-745` now destroys finished SE channels during `update()`. Confirmed closed by code inspection. |
| TD-R02 | CopilotKernel predicate gap | `engine/core/copilot/copilot_kernel.h` now uses registered predicates, and `tests/test_copilot_kernel.cpp` proves real predicate-based rejection instead of substring mocks. |

> **Pattern to replicate:** The SE fix used a clear ownership model (destroy on `update()`). Apply this same "ownership contract finalized at bounded lifecycle point" pattern to the sprite handle and asset cache issues in TD-05.

## Session Progress Update — 2026-04-22

This audit remains a valid snapshot of what was open on 2026-04-21, but several items moved forward during the 2026-04-22 execution session. Treat the sections below as the current progress overlay on top of the original audit.

**Session highlights:**

- **TD-07 quick wins landed:** `stb` is pinned, repo-wide warning flags are enabled through a shared warnings target, Gate label filters were anchored to `^pr$`, `^nightly$`, and `^weekly$`, and `tools/ci/run_local_gates.ps1` now includes `check_save_policy_governance.ps1`, `check_breaking_changes.ps1`, and `check_cmake_completeness.ps1`.
- **TD-07 warning cleanup landed:** once the missing nightly/weekly targets were built in the current debug tree, the tree exposed 11 unique warnings instead of failing cleanly; that in-repo backlog is now burned down, vendored SDL warning noise is quarantined at the SDL target, and the stale `dev-ninja-debug` Ninja-state warning was resolved by refreshing the corrupted `.ninja_log`.
- **TD-01 audit noise reduced:** `check_cmake_completeness.ps1` no longer exempts seven ability/editor tests that are already compiled into `urpg_tests`, and the stale `plugin_host.*`, `script_bridge.*`, `scripting_console.*`, `doc_generator.*`, and `battle_tactics_window.*` surfaces have all been relocated into explicit incubator paths under `tools/`; the remaining whitelist is now limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- **TD-01 top-level seam retired:** the stale `EngineAssembly` / `MainAssembly` pair and their non-canonical runtime forwarding aliases have now been removed entirely, leaving `EngineShell` as the only surviving top-level runtime path.
- **TD-02 is now explicitly closed as a measured render-path redesign lane:** `RenderLayer` now owns frame commands as value payloads, `EngineShell` submits the frame-owned buffer through `RendererBackend::processFrameCommands()`, common runtime/compat callers no longer need heap-backed per-frame command storage, `OpenGLRenderer` / `HeadlessRenderer` accept frame-owned commands without reboxing the hot path back into `shared_ptr` vectors, and new `RenderLayer` telemetry plus a warmed-up `EngineShell` scene-load test prove the active path stops growing frame-command storage and never rebuilds the legacy pointer view in steady state.
- **TD-04 bounded parity is now more explicit:** missing battlebacks emit a named `MISSING_BATTLEBACK` diagnostic; unsupported migrated action `effects[]` are preserved in `_compat_effect_fallbacks`; `CombatFormula` now supports a bounded arithmetic/stat subset with named fallback reasons; compat `BattleManager` uses that subset and surfaces unsupported formulas in `PARTIAL` status text; troop battle events now support variable conditional branches against live DataManager state.
- **TD-05 ownership/lifetime hardening landed:** `AssetLoader` caches are now bounded, explicitly single-thread-affine, and protected against cross-thread reuse; `ThreadRegistry` access is synchronized and covered by a concurrent test; the plugin API raw global `World*` was replaced by scoped/thread-local binding; and compat sprite semantics now cover deterministic bitmap ownership plus source-rect and motion lifecycle behavior.
- **TD-06 is now explicitly closed as a bounded release-truth lane:** `ResourceProtector` now performs lightweight real RLE compression plus XOR obfuscation, `ExportPackager` carries one bounded real Windows launch smoke path, and `VisualRegressionHarness` carries one bounded real renderer-backed capture lane. Broader shipping-grade packaging and coverage are now future feature work rather than unresolved current debt.
- **TD-09 is now explicitly closed as an unsupported-contract lane:** non-empty `activeCondition` values now produce explicit `active_condition_unsupported` diagnostics with replay-visible detail; `passiveCondition` is explicitly documented/tested as out of current runtime scope; `canApplyEffect()` is explicitly documented/tested as an always-true effect-admission contract until effect-level gating exists; and scripted ability-condition strings are treated as deliberately unsupported in-tree data rather than an undecided evaluator lane.
- **TD-10 is now explicitly closed as a bounded local-only contract:** AI/chat and cloud seams are explicitly labeled `NOT LIVE`, `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` now returns `false` instead of pretending success, the in-tree cloud double is now scoped explicitly as `LocalInMemoryCloudService`, and the accepted in-tree runtime shape is now documented as `IChatService` plus deterministic `MockChatService` and `ICloudService` plus `LocalInMemoryCloudService`, with live providers treated as out-of-tree or future feature work.
- **TD-11 landed:** `CopilotKernel` constraint evaluation now uses real predicate-based checks rather than substring matching.

**Focused verification completed this session:**

- `cmake --build --preset dev-debug --target urpg_tests`
- `cmake --build --preset dev-debug --target urpg_integration_tests urpg_snapshot_tests urpg_compat_tests urpg_presentation_release_validation`
- Focused `ctest --test-dir build/dev-ninja-debug --output-on-failure -R ...` lanes covering Copilot, battle scene, AssetLoader, ThreadRegistry, Plugin API, compat sprites, battle migration, combat formulas, DataManager, BattleManager formula fallback, ResourceProtector, and troop event variable-branch behavior all passed.
- `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "^urpg_presentation_release_validation$"` passed.
- `ctest --test-dir build/dev-ninja-debug --output-on-failure -R "^Integration:"` passed all 11 integration tests.

### Session Checklist — 2026-04-22

**Completed this session**

- [x] TD-07: Pin `stb` to a fixed revision instead of `master`
- [x] TD-07: Enable repo-wide warning flags through a shared warnings target
- [x] TD-07: Anchor CI/local `ctest` label filters to `^pr$`, `^nightly$`, and `^weekly$`
- [x] TD-07: Bring `run_local_gates.ps1` closer to Gate 1 by adding the missing governance checks
- [x] TD-01: Mark the former `EngineAssembly` / `MainAssembly` path as non-canonical relative to `EngineShell`, then retire it
- [x] TD-01: Relocate `doc_generator.*` out of `engine/core/editor/` into an explicit incubator path
- [x] TD-01: Relocate `battle_tactics_window.*` out of `engine/core/ui/` into an explicit incubator path
- [x] TD-01: Relocate `plugin_host.*`, `script_bridge.*`, and `scripting_console.*` out of `engine/core/editor/` into an explicit incubator path
- [x] TD-04: Replace silent battleback fallback with named `MISSING_BATTLEBACK` diagnostics
- [x] TD-04: Preserve unsupported migrated action `effects[]` as `_compat_effect_fallbacks`
- [x] TD-04: Add bounded `CombatFormula` subset support with named fallback reasons
- [x] TD-04: Make compat `BattleManager` use the bounded formula subset and expose unsupported-formula fallback status
- [x] TD-04: Add variable conditional-branch support to compat troop battle events
- [x] TD-05: Bound `AssetLoader` success and missing-texture caches
- [x] TD-05: Synchronize `ThreadRegistry` and add concurrent registration/query coverage
- [x] TD-05: Replace raw global plugin `World*` binding with scoped/thread-local binding
- [x] TD-05: Make compat sprite bitmap handles reload/release deterministically
- [x] TD-06: Replace fake `ResourceProtector` compression with bounded real in-tree protection
- [x] TD-09: Emit explicit `active_condition_unsupported` diagnostics with replay-visible detail
- [x] TD-09: Add tests that pin current `activeCondition` and `passiveCondition` boundaries
- [x] TD-09: Document `passiveCondition` as out of current runtime scope and pin that contract in tests
- [x] TD-09: Document/test `canApplyEffect()` as the current always-true effect-admission contract
- [x] TD-10: Mark AI/chat request paths as `NOT LIVE`
- [x] TD-10: Change `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` to return `false`
- [x] TD-11: Replace Copilot substring matching with predicate-based constraint validation

**Still needed**

- [x] TD-01: Collapse duplicate incubator copies of `EngineAssembly` / `MainAssembly`
- [x] TD-01: Retire the dead non-canonical `EngineAssembly` / `MainAssembly` seam and its runtime alias path entirely
- [x] TD-02: Collect allocator or profiler evidence that the frame-owned render path actually removed hot-loop heap churn under scene load
- [x] TD-03: Record the compat harness as the single supported in-tree scripting/plugin strategy
- [x] TD-04: Record the bounded in-tree formula and troop-event subset as the supported contract
- [x] TD-04: Close the current in-tree battle parity debt by routing compat audio, attempting compat battleback selection, and replacing battle-scene placeholder HUD quads
- [x] TD-05: Either document `AssetLoader` as single-thread-only or synchronize its remaining process-global caches
- [x] TD-05: Close the remaining compat sprite semantics TODOs (`characterIndex` source rects, sprite update animation state, actor motions)
- [x] TD-06: Add at least one real export smoke path
- [x] TD-06: Add at least one renderer-backed visual capture path once TD-02 lands
- [x] TD-07: Burn down the currently visible warning backlog
- [x] TD-07: Gate a focused lane with warnings-as-errors
- [x] TD-09: Record scripted ability conditions as permanently unsupported-but-explicit in the current in-tree contract
- [x] TD-10: Record the bounded local-only AI/cloud contract as the supported in-tree strategy
- [x] TD-10: Treat live chat/cloud providers as out-of-tree or future feature work rather than unresolved current debt
- [ ] TD-08: Split oversized compat and diagnostics files along domain boundaries

---

## Executive Summary

URPG is materially healthier than the 2026-04-21 snapshot, but the remaining debt is now tighter and more specific. The biggest problem is no longer broad repository chaos; it is that several public-looking or semi-public seams still sit beside the real, test-validated path and can still mislead contributors about what is live.

The live path is `EngineShell` plus a bounded native/compat slice. Around that core, the tree still carries:

- A render path whose hottest ownership debt is now measured and closed as a bounded frame-command redesign lane, while broader visual breadth and golden enforcement remain separate TD-06 work
- Compat/runtime surfaces that are honestly labeled `PARTIAL`, but still do not amount to live MZ-equivalent execution
- Bounded export, security, and visual-validation lanes that prove contract shape more than shipping behavior
- A smaller ownership backlog now concentrated in the explicit `AssetLoader` thread-affinity contract and a few remaining harness-truthfulness seams
- A Gameplay Ability Framework whose scripted conditions are now explicit and test-backed, but still not actually evaluated
- An AI/cloud connectivity layer whose in-tree scope is now intentionally bounded and explicit (`IChatService`, `MockChatService`, `AISyncCoordinator`, `LocalInMemoryCloudService`), with live providers moved out of current debt scope
- A repo-wide warning policy that now exposes a real warning-cleanup backlog

`CopilotKernel` is no longer an active debt leader in the current tree; the predicate-enforcement gap is closed.

---

## Priority Summary

> **Effort key:** S = hours, M = 1–3 days, L = 1–2 weeks, XL = multi-week
> **Risk key:** Correctness risk if left unaddressed — High / Medium / Low

| ID | Severity | Effort | Risk | Blocks | Title |
|---|---|---|---|---|---|
| TD-01 | `P1` | M | Medium | TD-03 | Duplicate stale top-level assembly seams remain non-build-real |
| TD-02 | `P3` | S | Low | — | Render-path redesign is closed as a measured frame-command lane; broader visual breadth remains TD-06 work |
| TD-03 | `P3` | M | Low | — | JS/plugin/editor compat stack is now explicitly bounded as a harness-first bridge rather than an unresolved strategy question |
| TD-04 | `P3` | M | Medium | — | Battle parity is now deliberately bounded and documented; future expansion beyond the supported subset is feature work rather than hidden debt |
| TD-05 | `P3` | S | Medium | — | Remaining ownership debt is concentrated in global cache/thread-affinity assumptions and compat sprite TODOs |
| TD-06 | `P3` | S | Low | — | Export, security, and visual validation are closed as bounded real smoke/capture/protection lanes |
| TD-07 | `P3` | S | Low | — | Warning policy landed, but warning cleanup and stricter gate posture still lag |
| TD-08 | `P3` | L | Low | — | Complexity concentrated in oversized compat/diagnostics files |
| TD-09 | `P3` | S | Low | — | GAF scripted condition strings are explicitly unsupported in-tree with bounded diagnostics |
| TD-10 | `P3` | S | Low | — | AI/cloud connectivity is now explicitly bounded to local-only in-tree contracts; live transport is future or out-of-tree work |

> **Highest-urgency combination:** TD-08 now defines the biggest remaining maintainability risk, while TD-07 remains the key trust multiplier around it.

---

## ⚡ Next Quick Wins (< 1 Day Each)

The original eight quick wins from v3 are landed. The highest-signal next sub-day fixes are now post-cleanup guardrail maintenance and the remaining product-scope decisions:

1. **Keep the focused warnings-as-errors lane green** now that the current in-repo warning backlog is burned down. (TD-07)
2. **Keep `AssetLoader`'s single-thread-affine cache contract explicit** anywhere new runtime callers are added. (TD-05)
3. **Keep compat sprite JS registrations truthful** until the harness grows real per-instance script receivers. (TD-05)
4. **Keep the declared local-only AI/cloud contract explicit** if future docs or code touch the lane. (TD-10 maintenance)
5. **Split the largest compat/diagnostics files now that the highest-risk seam cleanup is out of the way.** (TD-08)
6. **Keep contributor docs explicit that the active scripting/plugin path is the compat harness, not a second editor-side bridge.** (TD-03 maintenance)

---

## Risk Matrix

```
          │ Low Effort │ High Effort
──────────┼────────────┼────────────
High Risk │            │
──────────┼────────────┼────────────
Low Risk  │  TD-05     │  TD-01
          │  TD-07**   │  TD-08***
          │  TD-09     │
          │  TD-02     │
          │  TD-06     │
          │  TD-10     │
          │  TD-03     │
          │            │
```

\*\* TD-07 is low-severity debt, but the new warning baseline is only useful if the surfaced warnings are actually burned down.
\*\*\* TD-08 (file size) is low risk *today* but compounds the cost of every other fix.

---

## Phased Remediation Roadmap

### Phase 1 — Immediate (This Sprint)

**Goal:** Close the newly narrowed trust and hygiene gaps around the otherwise healthier tree.

| Action | Items | Owner |
|---|---|---|
| Keep the new warnings-as-errors lane healthy now that the targeted-build warning backlog is clean | TD-07 | — |
| Add an explicit thread-affinity contract or synchronization to `AssetLoader`, and close the remaining compat sprite semantics TODOs | TD-05 | — |
| Keep docs and headers explicit that bounded export/security/AI/cloud lanes are not shipping-equivalent | TD-06, TD-10 | — |
| Keep the explicit unsupported scripted-condition contract intact unless a future evaluator is deliberately introduced | TD-09 | — |

### Phase 2 — Near-Term (Next 2–4 Weeks)

**Goal:** Decide which partial surfaces are being expanded versus permanently bounded.

| Action | Items | Owner |
|---|---|---|
| Keep the measured TD-02 frame-command path honest if render-layer ownership or backend submission changes | TD-02 | — |
| Keep the declared compat harness strategy explicit in code/docs and treat live scripting as future feature scope | TD-03 | — |
| Keep the declared battle/formula subset explicit in migration/runtime output and treat broader parity as new feature scope | TD-04 | — |
| Keep `activeCondition` / `passiveCondition` documented as deliberately unsupported in the current in-tree runtime | TD-09 | — |
| Keep the TD-10 local-only AI/cloud contract explicit in contributor-facing docs and headers | TD-10 | — |
| Start splitting `DiagnosticsWorkspace` and `urpg_project_audit` by domain now that the largest ownership/truth gaps are smaller | TD-08 | — |

### Phase 3 — Medium-Term (4–8 Weeks)

**Goal:** Expand only the surfaces the project actually intends to ship and delete or quarantine the rest.

| Action | Items | Owner |
|---|---|---|
| Expand battle/runtime parity only if the product roadmap explicitly reopens the now-bounded battle surface | TD-04 | — |
| TD-03 is now harness-first: keep only the active compat/runtime fixture path, name it clearly as a harness, and document that boundary everywhere | TD-03 | — |
| Treat any future scripted-condition evaluator as new feature work rather than latent current-tree debt | TD-09 | — |
| Treat any future live `ICloudService` or chat provider as new feature work, not latent current-tree debt | TD-10 | — |
| Keep the bounded TD-06 export/security/visual contract explicit if future work expands those lanes | TD-06 | — |
| Begin splitting oversized compat manager files by responsibility boundary (not just size) | TD-08 | — |
| Schedule a dedicated ASan/LSan and Valgrind pass after Phase 2 lands | — | — |

---

## Detailed Findings

### TD-01 — `P1` Stale top-level assembly/editor seams were non-build-real and are now retired

**Effort:** M | **Risk:** Medium | **Blocks:** TD-03, TD-07

The repo no longer exposes these seams from production-looking engine/editor paths, and the dead non-canonical top-level assembly pair is now retired entirely rather than left as an alternate entry story.

**Evidence:**

- The dead `tools/incubator/core/engine_assembly.h`, `tools/incubator/core/main_assembly.h`, `tools/incubator/runtime/engine_assembly.h`, and `tools/incubator/runtime/main_assembly.h` files have been removed.
- `tools/ci/check_cmake_completeness.ps1` no longer needs orphan exemptions for the retired plugin/script harness seam; `doc_generator.*` and `battle_tactics_window.*` were already relocated into `tools/docs/incubator/` and `tools/incubator/ui/` so they no longer present as production engine/editor/UI orphans. The only remaining whitelist entry is the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- `CMakeLists.txt:426-685` registers tools, tests, and `urpg_presentation_release_validation`, but no target ever compiled the retired `EngineAssembly` / `MainAssembly` seam.

**Why it matters:**

- New contributors no longer have a dead alternate top-level assembly path to choose by mistake.
- CI green does not mean all public-looking runtime or editor entry points are alive.
- The remaining scripting/plugin decision is now narrower: only the active compat harness remains as a scoped keep/live-integration question.

**Recommended direction:**

- Declare `EngineShell` the only canonical top-level runtime surface.
- Keep any surviving harness code clearly scoped to the active compat/runtime path unless a real productized script runtime is built.
- Keep the completeness whitelist limited to genuine standalone tools, not stale runtime/editor seams.
- Gate: `check_cmake_completeness.ps1` passes with no production-looking seam exemptions.

**Definition of Done:**

- [x] The former `EngineAssembly` / `MainAssembly` seam was quarantined under `tools/` long enough to make its non-canonical status explicit, then retired
- [x] Production-looking editor/plugin assembly surfaces are either compiled as real targets or moved out of canonical runtime/editor paths
- [x] `check_cmake_completeness.ps1` passes with no stale production-seam whitelist entries
- [x] The dead top-level incubator assembly seam is retired entirely
- [ ] All CI, local gates, and remaining non-archival docs reference `EngineShell` as the single top-level entry point
- [x] Broken / nonexistent API references inside the remaining core incubator seam are removed, repaired, or archived

---

### TD-02 — Closed in current tree: frame-command render-path redesign is measured and stable in steady state

The render-command ownership lane that originally anchored TD-02 is no longer open debt. The active path is now value-owned, backend-facing, and backed by telemetry proving warmed-up scene load does not keep allocating frame-command storage or rebuilding the legacy pointer view.

**Evidence:**

- `engine/core/render/render_layer.h:136-364` now stores frame-owned `FrameRenderCommand` values and keeps legacy `shared_ptr` conversion as a compatibility bridge rather than the canonical hot path.
- `engine/core/platform/renderer_backend.h:51-58` now accepts frame-owned command vectors through `processFrameCommands(...)`, with the legacy heap-backed entry point retained only for older overrides.
- `engine/core/scene/map_scene.cpp` and `runtimes/compat_js/window_compat.cpp` now submit text, rect, sprite, and tile work through `toFrameRenderCommand(...)` instead of making heap ownership the frame-loop contract.
- `engine/core/platform/opengl_renderer.cpp` now emits immediate-color triangles for `RectCommand` and bounded `stb_easy_font`-derived triangle batches for `TextCommand`; the textured sprite/tile path remains separate from this ownership redesign and is no longer being used to keep TD-02 open.
- `tests/unit/test_engine_shell.cpp` now proves that, after one warm-up frame under representative map-scene load, subsequent `EngineShell` ticks produce no additional `RenderLayer` frame-command capacity growth and no legacy-view rebuilds.
- Existing focused tests already pin the surrounding bounded command surface: `tests/engine/core/test_render_assets.cpp` covers frame-owned/legacy intake plus pre-init backend behavior, and `tests/unit/test_window_compat.cpp` covers text, gauge, and sprite command emission through the value-owned render path.

**Why it matters:**

- The active hot path now has measured steady-state behavior instead of just structural promises.
- UI-adjacent command ownership is already broader than the original TD-02 wording implied: text, gauges, message windows, characters, icons, tiles, and map-scene command submission all reach the frame-owned path.
- Broader renderer-backed scene coverage and CI-enforced goldens are still real work, but they belong to TD-06 rather than this ownership/redesign lane.

**Recommended direction:**

- Keep frame-owned command flow as the canonical path and treat any future regressions in render-layer growth or legacy conversion as TD-02 regressions.
- Keep broader renderer-backed scene/golden validation work under TD-06 rather than reopening this ownership lane.

**Definition of Done:**

- [x] `RenderLayer` command storage no longer uses `shared_ptr` as the canonical per-frame path
- [x] `renderer_backend.h` abstraction is updated to accept frame-owned command view
- [x] `OpenGLRenderer` produces real draw calls for `TextCommand` and `RectCommand` (bounded immediate-mode implementation)
- [x] Allocation instrumentation or profiler-backed allocator data confirms the render path no longer performs per-frame heap growth under normal scene load

---

### TD-03 — `P3` JS/plugin/editor compatibility stack is now explicitly closed as a harness-first bridge

**Effort:** XL | **Risk:** Medium | **Depends on:** TD-01 cleanup

The compat runtime is better documented than before, and the dead parallel editor-side seam is now retired. The remaining decision is no longer open in-tree: the surviving active path is intentionally the harness-first compat bridge, not a live production scripting/plugin lane.

**Evidence:**

- `runtimes/compat_js/quickjs_runtime.cpp:4-5` explicitly states the file is fixture-backed and "not a live QuickJS runtime".
- `runtimes/compat_js/plugin_manager.h` now states that the compat plugin manager is the active and only remaining in-tree scripting/plugin execution path and that it is intentionally fixture-backed.
- `runtimes/compat_js/plugin_manager.cpp:1038-1040` describes its runtime bridge as fixture-backed command execution.
- `runtimes/compat_js/plugin_manager.cpp:1209-1238` documents that plugin loading, reload, directory scan, and execution depend on fixture JSON plugins and stub JS contexts.
- The previously parallel editor-side incubator `ScriptBridge` / `PluginHost` / `ScriptingConsole` seam has now been removed, so the compat harness is the only remaining scripting/plugin execution path in-tree.

**Why it matters:**

- The compat layer is valuable for deterministic contract coverage, import, migration, and validation, but it is not interchangeable with a live script runtime.
- The old debt was the unresolved strategy question. That ambiguity is now gone: the harness-first bridge is the accepted in-tree scope.

**Supported direction:**

- Keep only the compat/runtime fixture path and document that boundary in public headers and contributor-facing docs.
- Treat any future live QuickJS or live plugin runtime integration as new feature work, not unfinished debt hidden behind the current compat lane.
- Keep method-status and deviation text conservative whenever a compat surface is still harness-backed.

**Definition of Done:**

- [x] A single scripting/plugin strategy is documented in contributor-facing docs
- [x] The surviving harness code is documented and placed as the single active compat/runtime harness path
- [x] Public compat headers explicitly describe the harness-first intent where it matters

---

### TD-04 — `P3` Battle parity is now explicitly bounded and documented as the supported in-tree contract

**Effort:** L | **Risk:** High | **Depends on:** nothing for baseline correctness; TD-03 only matters for full live-script parity

The deterministic native and compat battle slice is no longer carrying hidden TD-04 ambiguity in-tree. The supported contract is now explicit: a bounded arithmetic/stat formula subset, a bounded troop-event interpreter subset with named fallbacks, compat battle audio routed through the deterministic harness, compat battleback lookup attempted on native scene startup, and colored bounded HUD cues in the native battle scene. Full RPG Maker-style formula/event/transition parity is not the current in-tree contract and should be treated as future feature work unless reopened deliberately.

**Evidence:**

- `engine/core/scene/combat_formula.h` implements the supported recursive-descent arithmetic/stat subset, and unsupported formulas return named fallback reasons instead of silently pretending to be full MZ semantics.
- `runtimes/compat_js/battle_manager.cpp` now routes battle BGM / victory ME / defeat ME into the compat `AudioManager` harness during battle lifecycle, while `setBattleTransition` remains explicitly metadata-only and tested as such.
- `runtimes/compat_js/battle_manager.cpp` keeps troop-page execution on a bounded interpreter subset, and unsupported or unmapped migration/runtime cases remain preserved as named fallbacks rather than silently dropped.
- `engine/core/scene/battle_scene.cpp` now attempts compat-selected battleback lookup before the fallback path and renders colored bounded HUD cues for gauges, damage popups, guard markers, and state icons instead of white-texture stand-ins.
- `tests/unit/test_battlemgr.cpp` and `tests/unit/test_battle_scene_native.cpp` now pin the partial boundaries and the bounded visual contract directly.

**Why it matters:**

- The old debt was not merely "missing features"; it was uncertainty about whether the current slice was pretending to be broader than it really was.
- That ambiguity is now gone: the bounded subset is the supported in-tree surface, and unsupported cases remain named.
- Future full-parity expansion is still possible, but it is no longer required to keep the current repo honest.

**Recommended direction:**

- Keep unsupported formulas, troop-page commands, and migration/runtime fallbacks named if future battle work expands the surface.
- Treat transition-type routing and broader MZ battle-event semantics as future feature scope, not silent implied debt.
- If product plans later require broader formula or event semantics, reopen TD-04 as explicit expansion work rather than relabeling the current bounded lane.

**Definition of Done:**

- [x] A formula strategy decision is recorded in the contributor guide or audit document
- [x] The bounded formula evaluator is declared the supported in-tree subset unless future roadmap work reopens it
- [x] Unsupported formula strings, troop-page commands, and migration fallbacks remain named and non-silent
- [x] No `PARTIAL` marker in `battle_manager.h` lacks an associated test verifying the partial boundary
- [x] Battle-scene placeholder popup/state/guard visuals are replaced with a bounded colored-quad contract

**Progress on 2026-04-22:**

- `BattleScene` no longer silently falls back to `Grassland`; it emits `MISSING_BATTLEBACK`.
- `BattleScene` now also attempts compat-selected battleback lookup before the fallback path and renders colored bounded HUD quads for gauges, damage popups, guard markers, and state icons.
- `CombatFormula` now supports a bounded arithmetic/stat subset and returns named fallback reasons for unsupported or malformed expressions.
- `BattleManager::applySkill()` / `applyItem()` now use that bounded subset when present and expose unsupported formulas in method status text instead of silently degrading.
- Compat troop-page handling now supports variable conditional branches against constants and live variable operands.
- Compat battle BGM / victory ME / defeat ME now drive the deterministic `AudioManager` harness during battle start/end and live BGM changes instead of staying metadata-only.
- Migration now preserves unsupported action `effects[]` as `_compat_effect_fallbacks` rather than hiding them behind a generic warning.
- **Closure decision:** the current bounded formula/event/harness battle surface is the supported in-tree contract. Future full-parity expansion, including transition-type routing or broader MZ semantics, is no longer tracked as hidden TD-04 debt.

---

### TD-05 — `P3` Remaining ownership debt is narrower and now concentrated in explicit single-thread cache policy and residual sprite-harness limits

**Effort:** S | **Risk:** Medium | **Depends on:** nothing

No fresh must-fix leak at the level of the earlier AudioManager SE issue remains in this slice, but a smaller set of ownership assumptions still deserves cleanup before these paths are treated as hardened.

**Evidence:**

- `engine/core/render/asset_loader.h:24-47` now exposes bounded, clearable static caches with an explicit single-thread-affine contract.
- `engine/core/render/asset_loader.cpp` now guards cache access with a mutex and rejects cross-thread reuse with a diagnostic instead of silently racing.
- `engine/core/scene/map_scene.cpp` and `engine/core/scene/battle_scene.cpp` still load runtime textures through `AssetLoader`, so the global-cache assumption remains reachable from live scene code.
- `engine/core/editor/plugin_api.cpp` no longer stores a raw process-global `World*`; it now uses a scoped thread-local binding stack. That earlier high-risk item is closed.
- `engine/core/threading/thread_roles.h` now uses `std::shared_mutex`, and concurrent tests exist. That earlier map-race item is closed.
- `runtimes/compat_js/window_compat.cpp` now implements deterministic `Sprite_Character` source-rect derivation and `Sprite_Actor` motion lifecycle behavior, but the QuickJS harness still cannot expose live per-instance sprite receivers and therefore truthfully registers several JS-facing sprite methods as `STUB`.

**Important nuance:**

- `Window_Base` contents-handle cleanup itself still looks correct: `window_compat.cpp:1007-1012` erases entries from `contentsBitmaps_`.
- The SE leak from the previous audit is closed (see Resolved Debt section).

**Why it matters:**

- The remaining `AssetLoader` debt is less about unbounded growth now and more about whether the explicit single-thread cache contract remains acceptable long term.
- The earlier `PluginAPI` and `ThreadRegistry` ownership hazards are no longer the headline risk in this category.
- The remaining compat sprite debt is now mostly about harness truthfulness and broader product strategy rather than missing native bitmap/source-rect/motion semantics.

**Recommended direction:**

- Keep `AssetLoader`'s single-thread-affine cache contract explicit anywhere new runtime callers are added.
- Keep `clearCaches()` as the explicit reset boundary and surface that contract in the code/docs that use it.
- Keep compat sprite JS registrations honest about harness limitations until a real per-instance script receiver model exists.

**Definition of Done:**

- [x] `AssetLoader` caches have a documented size bound and an explicit `clear()` path
- [x] No raw process-global `World*` remains in plugin API code
- [x] `ThreadRegistry` map access is synchronized
- [x] At least one multi-threaded `ThreadRegistry` test covers concurrent registration and lookup
- [x] `AssetLoader` thread-affinity / synchronization expectations are explicit
- [x] Remaining compat sprite TODOs are resolved before broader bitmap/animation loading is wired

**Progress on 2026-04-22:**

- `AssetLoader` now uses bounded caches for successful and missing texture lookups, exposes `clearCaches()` as an explicit reset boundary for tests/reset flows, guards cache access with a mutex, and rejects cross-thread reuse with a clear diagnostic.
- `ThreadRegistry` map access is synchronized with `std::shared_mutex`, and concurrent registration/query coverage exists.
- The raw global plugin `World*` binding has been replaced with a scoped thread-local binding stack.
- `Sprite_Character` and `Sprite_Actor` bitmap handles now reload and release deterministically across identity changes and destruction, `Sprite_Character` computes stable source rects from sheet state, and `Sprite_Actor` returns non-looping motions to idle with deterministic loop handling.
- JS-facing sprite registrations that cannot be truthful in the fixture-backed QuickJS harness are now explicitly marked `STUB`, and the unsatisfiable sprite-only curated compat profiles were removed.
- **Result:** the highest-risk TD-05 ownership items from the prior pass are now substantially reduced. The remaining work is smaller and mostly about whether the current single-thread asset-cache policy and sprite-harness scope are the desired long-term product shape.

---

### TD-06 — Closed in current tree: export, security, and visual validation are bounded real contract lanes

The in-tree release-truth lane that originally anchored TD-06 is no longer open debt. The current tree now carries one bounded real export smoke path, one bounded real renderer-backed visual capture path, and lightweight real in-tree resource protection. Broader shipping-grade packaging breadth and hardening are future work, not unresolved current debt.

**Evidence:**

- `engine/core/tools/export_packager.cpp` now supports one bounded real Windows launch smoke path when a real runtime binary is explicitly provided, plus bounded real bootstrap outputs for Web, Linux, and macOS instead of fake binary marker files.
- `tools/export/export_smoke_app.cpp` now provides a dedicated headless native launcher for that bounded Windows smoke lane, and `tests/unit/test_export_packager.cpp` launches the staged export and proves execution through a marker file.
- `engine/core/export/export_validator.cpp` now still validates emitted artifact shape conservatively, but it also verifies keyed integrity tags for `data.pck` payload entries during post-export validation instead of treating any existing bundle file as valid.
- `engine/core/security/resource_protector.h` now routes `compress()` through the in-tree `AssetCompressor` for lightweight RLE compression, keeps XOR obfuscation as the bounded shipped hardening layer, and now also supplies the keyed integrity-tag helper used by the export bundle path.
- `engine/core/tools/export_packager.cpp` now also stages bounded repo-owned content roots and governed manifest-driven promoted assets from `imports/manifests/asset_bundles/*.json` when those manifests explicitly reference existing repo-local files under `imports/normalized/`.
- `engine/core/testing/visual_regression_harness.cpp` now includes a bounded hidden SDL/OpenGL capture helper, and `tests/snapshot/test_renderer_backed_visual_capture.cpp` plus committed goldens now prove not just rect/text primitives but multiple truthful live-surface and whole-scene renderer-backed paths.

**Why it matters:**

- These lanes are now useful bounded real contract guards rather than fake release signals.
- A green result here should be read as `bounded platform bootstraps, one real Windows launch smoke path, multiple renderer-backed visual proof paths, lightweight in-tree RLE+XOR protection, keyed bundle integrity validation, and governed explicit asset staging`, not `shipping pipeline proven`.
- The remaining breadth and hardening questions are about automatic project discovery, signing/notarization, and cryptographic/runtime enforcement, not hidden behavior mismatches in current code.

**Recommended direction:**

- Keep audit and doc claims tightly bounded: this is a truthful bounded in-tree release lane, not a full shipping/export certification pipeline.
- Treat automatic project-wide asset discovery, full native packaging/signing/notarization, and cryptographic runtime enforcement as future feature work or mandatory backlog depending on program scope; do not collapse those larger steps into the bounded current export contract.

**Definition of Done:**

- [x] Docs and CI output use bounded/truthful qualifiers instead of implying a shipping-grade release pipeline
- [x] `ResourceProtector::compress()` now performs lightweight real in-tree compression
- [x] At least one export target produces a binary that passes a basic launch smoke test
- [x] Visual regression harness captures at least one real renderer frame (post TD-02)

**Progress on 2026-04-22:**

- `ResourceProtector` now performs lightweight real RLE compression and reversible XOR obfuscation.
- Tests now pin compression/decompression round-trips plus reversible XOR behavior over compressed bytes.
- `ExportPackager` now has one bounded real Windows smoke lane: it can stage a real headless launcher plus runtime DLLs when given an explicit runtime-binary path, and focused tests prove the staged `game.exe` actually launches and writes its smoke marker.
- The same packager path now emits bounded real Web, Linux, and macOS bootstrap structures, records keyed integrity tags for bundle payloads, and stages governed promoted assets when explicit asset-bundle manifests point at repo-local normalized files.
- `ExportValidator` now treats `data.pck` corruption or simple tampering as a real post-export validation failure via keyed integrity checks.
- The renderer-backed validation lane now covers multiple truthful live-surface and whole-scene paths rather than only the original primitive rect/text slice.
- The accepted in-tree strategy is now explicit: these bounded real lanes are the shipped contract, while automatic project-wide asset discovery, signing/notarization, and cryptographic/runtime enforcement remain outside the current bounded export scope.

---

### TD-07 — `P3` Warning policy landed, the in-repo backlog is burned down, and a focused strict lane is now in place

**Effort:** S | **Risk:** Low | **Depends on:** nothing

URPG's build governance is in better shape than it was in the previous pass. The debt is no longer missing policy, an active in-repo warning backlog, or a missing stricter posture decision; the remaining work is to keep the new strict lane healthy as the tree evolves.

**Evidence:**

- `CMakeLists.txt:10-17` now enables repo-wide warnings through `urpg_project_warnings` with `/W4` on MSVC and `-Wall -Wextra` on GCC/Clang-family compilers.
- `CMakeLists.txt` now pins `stb`; the floating-dependency debt from the prior pass is closed.
- The earlier targeted-build warning cluster in presentation adapters, compat window overrides, migration/audio/test aggregate initialization, message/AI stubs, plugin-manager/diagnostics tests, OpenGL/UI code, and several ECS/scene headers has now been burned down in repo code.
- Exact label discovery is now cleanly anchored: `^pr$` => 918, `^nightly$` => 18, `^weekly$` => 44.
- `tools/ci/run_local_gates.ps1` now mirrors the key Gate 1 governance checks that were missing in the previous pass.
- `CMakeLists.txt` now exposes `URPG_WARNINGS_AS_ERRORS`, letting the shared `urpg_project_warnings` target promote project warnings to `/WX` or `-Werror` without weakening vendored-code handling.
- `.github/workflows/ci-gates.yml` Gate 1 now configures and builds a focused `build/ci-warnings-as-errors` tree that covers the core app, audit/export tools, release validation, and all registered test executables.
- `tools/ci/run_local_gates.ps1` now mirrors that strict lane locally through an opt-out `-SkipWarningsAsErrorsGate` switch instead of leaving stricter posture as CI-only discipline.
- `tools/ci/check_cmake_completeness.ps1` now burns down the stale plugin/script seam exemptions; the remaining whitelist is limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- Vendored SDL warning noise is now quarantined at the non-imported SDL target level, so project warnings are not weakened to accommodate upstream code.
- The recurring `ninja: warning: premature end of file; recovering` warning in `build/dev-ninja-debug` was traced to a corrupted `.ninja_log` and is resolved in the active tree after regenerating that state file.

**Why it matters:**

- Warning flags only raise code quality if the backlog they surfaced is actually burned down and kept clean.
- The tree now has a real strict lane, so future regressions have a focused place to fail without broadening vendored-code fallout.

**Recommended direction:**

- Keep exact-label discovery and local/CI gate parity anchored so future count drift is easy to spot.
- Keep the focused warnings-as-errors lane scoped to project-owned targets so vendored-code noise stays quarantined.
- Consider adding a clang-tidy or cppcheck preset as an optional opt-in gate.

**Definition of Done:**

- [x] `stb` is pinned to a specific revision
- [x] Repo-wide warning flags are present in `CMakeLists.txt`
- [x] `run_local_gates.ps1` runs the key Gate 1 governance checks that were missing in the prior pass
- [x] All ctest label patterns in CI and local scripts are anchored
- [x] The currently visible targeted-build warning backlog is reduced to zero or explicitly waived
- [x] A focused warnings-as-errors lane is enabled for project-owned targets
- [ ] `check_cmake_completeness.ps1` TODO is resolved or tracked as a separate issue

**Progress on 2026-04-22:**

- `stb` is pinned.
- Repo-wide warning flags are enabled via a shared compile-options target.
- CI and local gate label filters are anchored.
- A focused strict lane is now wired in both Gate 1 and `run_local_gates.ps1` via `URPG_WARNINGS_AS_ERRORS=ON`.
- `run_local_gates.ps1` now includes the key Gate 1 governance checks that were missing in the audit snapshot.
- The fresh targeted build originally proved the current debt had shifted to warning cleanup, not policy absence.
- The later cleanup pass burned down the in-repo warning cluster, quarantined vendored SDL warning noise at the SDL target, and fixed the stale Ninja-state warning in the active `dev-ninja-debug` tree.
- `check_cmake_completeness.ps1` no longer hides stale runtime/editor seams, but it still carries a narrow TODO for the standalone profiling tool whitelist.

---

### TD-08 — `P3` Complexity concentrated in oversized compat and diagnostics files

**Effort:** L | **Risk:** Low | **Depends on:** TD-03 decision (shapes how compat is split)

The maintainability risk is localized but will compound every future fix.

**Largest files observed in this pass:**

| File | Lines |
|---|---|
| `tests/compat/test_compat_plugin_failure_diagnostics.cpp` | 4,033 |
| `tests/compat/test_compat_plugin_fixtures.cpp` | 2,859 |
| `tests/unit/test_diagnostics_workspace.cpp` | 2,376 |
| `runtimes/compat_js/window_compat.cpp` | 2,370 |
| `runtimes/compat_js/plugin_manager.cpp` | 2,350 |
| `tools/audit/urpg_project_audit.cpp` | 2,183 |
| `editor/diagnostics/diagnostics_workspace.cpp` | 1,972 |
| `runtimes/compat_js/battle_manager.cpp` | 1,921 |
| `runtimes/compat_js/data_manager.cpp` | 1,744 |
| `tests/unit/test_plugin_manager.cpp` | 1,660 |

**Why it matters:**

- These are the places where every future fix becomes slower to review and riskier to land.
- The pattern clusters around the same subsystems already carrying the most partial or harness behavior.

**Recommended direction:**

- Use the now-fixed harness-first TD-03 boundary when planning plugin manager splits — split around compat-harness responsibilities, not around a hypothetical future live runtime.
- Decompose `DiagnosticsWorkspace` and `urpg_project_audit` around tab and report domains now; these are not gated on other decisions.
- Split compat manager files by responsibility boundary, not by line count alone.
- In the next audit: check whether the largest five files are actually shrinking.

**Definition of Done:**

- [x] TD-03 strategy decision made (prerequisite for plugin manager splits)
- [ ] `DiagnosticsWorkspace` and `urpg_project_audit` split into domain-aligned files (each under 1,000 lines)
- [ ] Each of the top-five compat files has a split plan tracked as a backlog item
- [ ] Next audit confirms at least two of the top-five files have decreased in size

---

### TD-09 — Closed in current tree: scripted ability-condition strings are explicitly unsupported in-tree

The Gameplay Ability Framework no longer carries an undecided scripted-condition lane in-tree. The accepted contract is now explicit: string-scripted `activeCondition` and `passiveCondition` values may appear in authored data for diagnostics and future compatibility, but the current runtime does not evaluate them.

**Evidence:**

- `engine/core/ability/gameplay_ability.h` now documents `activeCondition` and `passiveCondition` as authored diagnostic/future-compatibility fields rather than live in-tree evaluator inputs.
- `engine/core/ability/gameplay_ability.cpp` now blocks non-empty `activeCondition` values with the explicit reason `active_condition_unsupported` and a detail string that includes the authored expression.
- `passiveCondition` is still not evaluated anywhere in the runtime update path; the current contract is that it is out of scope until a real evaluator exists.
- `AbilitySystemComponent::canApplyEffect()` still returns `true` unconditionally and is only documented/test-pinned as an always-true admission gate for now.
- `tests/unit/test_ability_activation.cpp` now exercises the non-empty `activeCondition` path, proves the explicit diagnostic, and pins the current passive-condition non-behavior.

**Why it matters:**

- Any ability authored with a scripted condition string gets an explicit bounded result instead of silent bypass or false promise.
- The scope decision is now recorded: no in-tree evaluator is promised by the current GAF contract.
- Passive cancellation still does not happen, but that is now a documented current boundary rather than an undecided omission.

**Accepted in-tree direction:**

- Keep the explicit unsupported diagnostic path; do not regress back to silent behavior.
- Keep at least one focused activation test that exercises a non-empty `activeCondition`.
- Keep docs explicit that scripted condition strings are diagnostics-only / unsupported data in the current in-tree runtime.
- Treat any future evaluator, passive cancellation hook, or effect-tag gate as deliberate future feature work rather than latent current debt.

**Definition of Done:**

- [x] Abilities with a non-empty `activeCondition` produce a named diagnostic — they do not silently block or silently pass
- [x] `passiveCondition` is either evaluated in `update()` or its absence is documented as out-of-scope with a matching test that asserts the field is not read
- [x] `canApplyEffect()` either checks effect Tag requirements or is documented as always-true with a test pinning that behavior
- [x] At least one ability activation test covers a non-empty `activeCondition` value
- [x] A long-term evaluator-or-scope decision is recorded for `activeCondition` / `passiveCondition`

**Progress on 2026-04-22:**

- Non-empty `activeCondition` values now produce explicit `active_condition_unsupported` diagnostics with detail strings that surface in execution history and replay diagnostics.
- Focused activation tests now cover the non-empty `activeCondition` path, pin that `passiveCondition` is ignored by the current runtime, and prove that passive strings do not block execution.
- `canApplyEffect()` is now explicitly documented and test-pinned as an always-true admission gate; modifier `requiredTag` checks still apply later during attribute resolution.
- The accepted in-tree strategy is now explicit: scripted condition strings remain unsupported in-tree unless a future feature slice deliberately reopens evaluator work.

---

### TD-10 — Closed in current tree: AI/cloud is explicitly bounded to local-only in-tree contracts

The code-side ambiguity that originally anchored this debt lane is gone. The tree no longer carries in-tree provider implementations pretending to be almost-live integrations; instead it exposes a bounded local-only contract consisting of `IChatService` plus deterministic `MockChatService`, and `ICloudService` plus `LocalInMemoryCloudService`. Any real networked provider is now treated as out-of-tree or future feature work rather than unresolved current debt.

**Evidence:**

- `engine/core/ai/ai_connectivity.h`: now serves as boundary documentation only. It explicitly says no live OpenAI, Anthropic, llama.cpp, or other provider is implemented in-tree, and points contributors at out-of-tree `IChatService` implementations instead of exposing fake provider classes.
- `engine/core/social/cloud_service.h`: `LocalInMemoryCloudService::syncToCloud()` stores data in an in-process `std::map`. `LocalInMemoryCloudService::initialize()` only allows `CloudProvider::LocalSimulated`; live-looking providers fail with a `NOT LIVE` result instead of pretending to connect. No network operation occurs.
- `engine/core/message/ai_sync_coordinator.cpp` now returns `false` from `checkForRemoteKnowledgeUpdates()` with an explicit `NOT LIVE` comment; the misleading always-true behavior from the prior pass is gone.
- `engine/core/ai/personality_registry.h` is a fixed string-template selector, not a dynamic registry or provider-backed catalog.
- `engine/core/message/ai_sync_coordinator.h:13-14` already documents that, with the in-tree `LocalInMemoryCloudService`, the current behavior remains local in-memory rather than live cloud sync. `CMakeLists.txt` compiles `engine/core/message/ai_sync_coordinator.cpp` (line 199), and the tree now includes direct unit coverage for local-only initialization plus `AISyncCoordinator` sync/restore behavior.

**Why it matters:**

- Contributors now get an explicit, test-backed answer about what is real in-tree: deterministic mock chat and process-local cloud persistence only.
- The remaining local-only cloud API is still intentionally small and non-shipping, but it is no longer pretending to be a hidden near-future production backend.
- `PersonalityRegistry` remains simple, but it is no longer coupled to a misleading story about live provider transport being just around the corner.

**Accepted in-tree direction:**

- Keep `IChatService` as the provider abstraction and `MockChatService` as the only shipped deterministic in-tree implementation.
- Keep `ICloudService` as the backing-store abstraction and `LocalInMemoryCloudService` as the only shipped deterministic in-tree implementation.
- Treat live AI/chat transport and live cloud sync as future or out-of-tree feature work with their own delivery bar, rather than latent obligations of the current tree.
- Keep contributor-facing docs and headers explicit that the shipped path is local-only and `NOT LIVE`.

**Definition of Done:**

- [x] A single bounded local-only AI/cloud strategy is documented in contributor-facing docs and headers
- [x] In-tree provider-specific fake transport classes are gone; `ai_connectivity.h` is now boundary documentation only
- [x] The in-tree cloud double is now named `LocalInMemoryCloudService`, and live-provider initialization now fails loudly instead of pretending to connect
- [x] `checkForRemoteKnowledgeUpdates()` no longer pretends success; it now returns `false` with a clear `NOT LIVE` comment
- [x] AI/chat transport and cloud-provider behavior are covered at the accepted in-tree boundary: deterministic mock chat and local-only cloud sync/coordinator behavior

**Progress on 2026-04-22:**

- The AI/chat request paths are now explicitly marked `NOT LIVE`.
- `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` now returns `false` instead of implying successful remote polling.
- Cloud behavior remains local/in-memory only by design.
- The in-tree double now exposes its local-only scope at the type level (`LocalInMemoryCloudService`) and has direct unit coverage for local-only initialization and coordinator sync/restore behavior.
- The contributor-facing strategy is now explicit: this local-only surface is the supported in-tree contract, and live providers are outside current debt scope.

---

### TD-11 — Closed in current tree: CopilotKernel now uses predicate-based constraint enforcement

The core implementation gap from the previous pass is closed. `CanonConstraint` now carries a predicate, `validateProposal()` iterates real predicates, and `tests/test_copilot_kernel.cpp` proves predicate-based rejection instead of substring mocks. Any follow-up work from here is rule-coverage expansion, not technical-debt replacement of the enforcement mechanism itself.

---

## Cross-Cutting Themes

This pass reduces to five big themes:

1. **Build-truth drift is now narrower but sharper** — the remaining problem is harness-versus-runtime scoping plus stricter warning-posture decisions, not missing baseline governance. (TD-03, TD-07)
2. **Hot-loop ownership debt in the render/UI stack is materially reduced** — broader visual validation breadth is now a future expansion lane rather than unresolved current debt. (TD-06)
3. **Harness-versus-runtime ambiguity is now narrower and no longer centered on AI/cloud scope** — both TD-03 and TD-10 are explicitly bounded, so future live scripting or live AI/cloud work would be new feature work, not unresolved current scope. (TD-10)
4. **Ownership debt is smaller and more localized** — the earlier headline risks are reduced; what remains is surviving process-global state and unfinished compat sprite semantics. (TD-05)
5. **Unsupported features are now more honest, and the biggest remaining open lane is no longer in GAF scope** — AI/cloud transport and scripted ability conditions are both now deliberately scoped in-tree. (TD-09)

---

## Recommended Action Order

| Step | Action | Phase | TD Items |
|---|---|---|---|
| 1 | Keep the focused warnings-as-errors lane healthy now that the visible backlog is clean | 1 | TD-07 |
| 2 | Make `AssetLoader` thread-affinity explicit in follow-on callers and keep compat sprite JS registrations honest | 1 | TD-05 |
| 3 | Keep the declared TD-03 compat harness strategy explicit if future docs/code touch the lane | 2 | TD-03 |
| 4 | Keep the declared bounded battle/formula scope honest if future work touches it | 2 | TD-04 |
| 5 | Keep the explicit unsupported scripted-condition contract intact unless future feature work reopens evaluator scope | 2 | TD-09 |
| 6 | Keep the measured frame-command path honest while expanding broader visual validation elsewhere | 2 | TD-02, TD-06 |
| 7 | Keep the bounded local-only AI/cloud contract explicit if future code or docs revisit the lane | 2–3 | TD-10 |
| 8 | Keep the bounded export/security/visual-validation contract explicit if future work expands those lanes | 3 | TD-06 |
| 9 | Begin splitting `DiagnosticsWorkspace` and `urpg_project_audit`, then plan any compat-manager splits against the now-fixed harness boundary | 3 | TD-08 |
| 10 | Schedule ASan/LSan and Valgrind pass after the next render/allocator follow-through slice lands | 3 | — |

**Progress note on 2026-04-22:**

- The former Step 1 seam-retirement work is landed, and Step 2 now begins with the warning-posture decision.
- Step 3's highest-risk slice is landed; the remaining work is the smaller `AssetLoader` contract and compat sprite TODO cleanup.
- Step 5 is now closed as a bounded in-tree contract through the explicit formula/event subset, compat audio routing, battleback lookup, and native HUD visual replacement.
- Step 6 is partially landed through explicit unsupported-condition diagnostics and tests, but not full evaluation.
- Step 7 is now closed as a measured render-path redesign lane through value-owned `FrameRenderCommand` storage, frame-buffer backend consumption, real immediate-mode text/rect submission, focused pre-init/no-context coverage, and warmed-up scene-load telemetry proving steady-state no-growth/no-legacy-conversion behavior.
- Step 8 is now closed as a local-only strategy lane; any future live AI/cloud integration would be new feature work.

---

## Conclusion

URPG is not primarily suffering from missing documentation or missing tests. It is suffering from a smaller, harder class of debt: **non-canonical entry seams, harness-versus-runtime ambiguity, partial visual/runtime closure, post-cleanup warning-policy decisions, and a handful of remaining product-scope decisions that code can now name honestly but not yet execute.**

The fresh sweep changed the audit in four important ways:

- **TD-07 shifted from policy debt to posture debt.** Warning flags, pinned dependencies, and anchored label filters are in place, and the surfaced in-repo warning backlog is now cleaned up; the remaining question is whether to add a stricter warnings-as-errors lane.
- **TD-05 is materially smaller than the earlier snapshot implied.** The highest-risk cache/global/thread issues are no longer the category headline; the remaining work is explicit thread-affinity and compat sprite follow-through.
- **TD-11 is closed in substance.** Copilot constraints now use predicates, so that mechanism is no longer an active debt leader.
- **TD-01 is effectively closed as an implementation debt lane.** The dead alternate top-level assembly seam is retired, and the editor-side scripting/plugin seam is no longer a live TD-03 strategy question because the compat harness is now the single accepted in-tree path.

That is a healthier kind of debt. The repo already has the governance discipline to expose and close it. The most leveraged next actions are to decide the stricter post-cleanup warning posture, resolve the remaining harness/product boundary decisions, and make the remaining partial runtime surfaces either more real or more explicitly out of scope.

> **Next audit trigger:** Re-run this audit after Phase 2 completes. Add an ASan/Valgrind pass to the next cycle — the current finding list is based entirely on code inspection and may be undercounting live memory issues.
