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
- **Fresh build-quality finding:** once the missing nightly/weekly targets were built in the current debug tree, the tree exposed 11 unique warnings instead of failing cleanly. The current debt is no longer "warning policy missing"; it is "warning backlog now visible."
- **TD-01 audit noise reduced:** `check_cmake_completeness.ps1` no longer exempts seven ability/editor tests that are already compiled into `urpg_tests`, and the stale `plugin_host.*`, `script_bridge.*`, `scripting_console.*`, `doc_generator.*`, and `battle_tactics_window.*` surfaces have all been relocated into explicit incubator paths under `tools/`; the remaining whitelist is now limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- **TD-01 truthfulness improved, but not fully collapsed:** the stale `EngineAssembly` and `MainAssembly` seam now lives under explicit incubator paths with `EngineShell` as the live entry point, but the fresh sweep found duplicate copies under both `tools/incubator/runtime/` and `tools/incubator/core/`.
- **TD-02 first slice landed:** `RenderLayer` now owns frame commands as value payloads, `EngineShell` submits the frame-owned buffer through `RendererBackend::processFrameCommands()`, common runtime/compat callers no longer need heap-backed per-frame command storage, and `OpenGLRenderer` / `HeadlessRenderer` now accept frame-owned commands without reboxing the hot path back into `shared_ptr` vectors.
- **TD-02 second slice landed:** `OpenGLRenderer` now turns `RectCommand` into immediate-color triangle draws and `TextCommand` into bounded `stb_easy_font` triangle batches, while focused tests pin both frame-owned/legacy command intake and the pre-init no-op contract before a live GL context exists.
- **TD-04 bounded parity is now more explicit:** missing battlebacks emit a named `MISSING_BATTLEBACK` diagnostic; unsupported migrated action `effects[]` are preserved in `_compat_effect_fallbacks`; `CombatFormula` now supports a bounded arithmetic/stat subset with named fallback reasons; compat `BattleManager` uses that subset and surfaces unsupported formulas in `PARTIAL` status text; troop battle events now support variable conditional branches against live DataManager state.
- **TD-05 ownership/lifetime hardening landed:** `AssetLoader` caches are now bounded, `ThreadRegistry` access is synchronized and covered by a concurrent test, the plugin API raw global `World*` was replaced by scoped/thread-local binding, and compat sprite bitmap handles now reload and release deterministically.
- **TD-06 synthetic boundary is now clearer:** `ResourceProtector` no longer implies real compression; it explicitly exposes that compression is not implemented yet and tests pin the current passthrough/XOR behavior.
- **TD-09 low-risk contract gaps closed:** non-empty `activeCondition` values now produce explicit `active_condition_unsupported` diagnostics with replay-visible detail; `passiveCondition` is explicitly documented/tested as out of current runtime scope; `canApplyEffect()` is explicitly documented/tested as an always-true effect-admission contract until effect-level gating exists.
- **TD-10 truthfulness improved but strategy still open:** AI/chat and cloud seams are explicitly labeled `NOT LIVE`, `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` now returns `false` instead of pretending success, and the in-tree cloud double is now named `LocalInMemoryCloudService` with the old `CloudServiceStub` name retained only as a compatibility alias.
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
- [x] TD-01: Mark `EngineAssembly` and `MainAssembly` as non-canonical seams pointing to `EngineShell`
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
- [x] TD-06: Make `ResourceProtector` explicit about compression not being implemented
- [x] TD-09: Emit explicit `active_condition_unsupported` diagnostics with replay-visible detail
- [x] TD-09: Add tests that pin current `activeCondition` and `passiveCondition` boundaries
- [x] TD-09: Document `passiveCondition` as out of current runtime scope and pin that contract in tests
- [x] TD-09: Document/test `canApplyEffect()` as the current always-true effect-admission contract
- [x] TD-10: Mark AI/chat request paths as `NOT LIVE`
- [x] TD-10: Change `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` to return `false`
- [x] TD-11: Replace Copilot substring matching with predicate-based constraint validation

**Still needed**

- [ ] TD-01: Collapse duplicate incubator copies of `EngineAssembly` / `MainAssembly` into one clearly named non-canonical path
- [ ] TD-01: Remove or archive the broken-include `tools/incubator/runtime/engine_assembly.h` variant instead of letting duplicate stale seams drift
- [ ] TD-02: Collect allocator or profiler evidence that the frame-owned render path actually removed hot-loop heap churn under scene load
- [ ] TD-03: Make the keep/delete strategy decision for the compat/plugin/editor harness path
- [ ] TD-04: Decide and document the long-term formula strategy beyond the bounded subset
- [ ] TD-04: Close remaining partial battle parity gaps, especially transition/audio routing, broader troop-page coverage, and battle-scene visual placeholders
- [ ] TD-05: Either document `AssetLoader` as single-thread-only or synchronize its remaining process-global caches
- [ ] TD-05: Close the remaining compat sprite semantics TODOs (`characterIndex` source rects, sprite update animation state, actor motions)
- [x] TD-06: Add at least one real export smoke path
- [x] TD-06: Add at least one renderer-backed visual capture path once TD-02 lands
- [ ] TD-07: Burn down the currently visible warning backlog and decide whether to gate a focused lane with warnings-as-errors
- [ ] TD-09: Decide whether scripted ability conditions will gain a real evaluator or remain permanently unsupported-but-explicit
- [ ] TD-10: Decide whether the AI/cloud layer is being cut or wired to real transport
- [ ] TD-10: Either add a real chat/cloud provider or finish restricting the remaining local-only cloud surfaces beyond the renamed in-memory double
- [ ] TD-08: Split oversized compat and diagnostics files along domain boundaries

---

## Executive Summary

URPG is materially healthier than the 2026-04-21 snapshot, but the remaining debt is now tighter and more specific. The biggest problem is no longer broad repository chaos; it is that several public-looking or semi-public seams still sit beside the real, test-validated path and can still mislead contributors about what is live.

The live path is `EngineShell` plus a bounded native/compat slice. Around that core, the tree still carries:

- Duplicate stale top-level assembly seams under `tools/incubator/core/` and `tools/incubator/runtime/`
- A render path whose hottest ownership debt is materially reduced, but which still needs allocator proof plus textured sprite/tile/UI follow-through
- Compat/runtime surfaces that are honestly labeled `PARTIAL`, but still do not amount to live MZ-equivalent execution
- Bounded export, security, and visual-validation lanes that prove contract shape more than shipping behavior
- A smaller ownership backlog now concentrated in process-global caches and unfinished compat sprite semantics
- A Gameplay Ability Framework whose scripted conditions are now explicit and test-backed, but still not actually evaluated
- An AI/cloud connectivity layer (`OpenAIChatService`, `LlamaLocalService`, `AISyncCoordinator`, `LocalInMemoryCloudService`) that is clearly marked `NOT LIVE` but still exposed as production-looking types
- A repo-wide warning policy that now exposes a real warning-cleanup backlog

`CopilotKernel` is no longer an active debt leader in the current tree; the predicate-enforcement gap is closed.

---

## Priority Summary

> **Effort key:** S = hours, M = 1–3 days, L = 1–2 weeks, XL = multi-week  
> **Risk key:** Correctness risk if left unaddressed — High / Medium / Low

| ID | Severity | Effort | Risk | Blocks | Title |
|---|---|---|---|---|---|
| TD-01 | `P1` | M | Medium | TD-03 | Duplicate stale top-level assembly seams remain non-build-real |
| TD-02 | `P2` | L | Medium | — | Render-path redesign is materially landed, but allocator proof and textured/UI follow-through remain open |
| TD-03 | `P2` | XL | Medium | — | JS/plugin/editor compat stack is harness-first, not live |
| TD-04 | `P2` | L | High | — | Battle parity remains bounded by a subset evaluator, placeholder visuals, and partial routing |
| TD-05 | `P3` | S | Medium | — | Remaining ownership debt is concentrated in global cache/thread-affinity assumptions and compat sprite TODOs |
| TD-06 | `P2` | M | Low | — | Export, security, and visual validation are synthetic |
| TD-07 | `P3` | S | Low | — | Warning policy landed, but warning cleanup and stricter gate posture still lag |
| TD-08 | `P3` | L | Low | — | Complexity concentrated in oversized compat/diagnostics files |
| TD-09 | `P2` | M | Medium | — | GAF scripted conditions are explicit and test-backed, but still unsupported |
| TD-10 | `P2` | M | Medium | — | AI/cloud connectivity layer is entirely simulated with no live transport |

> **Highest-urgency combination:** TD-04, TD-03, and TD-10 now define the biggest "what is actually real?" product risk, while TD-01 and TD-07 remain trust multipliers around them.

---

## ⚡ Next Quick Wins (< 1 Day Each)

The original eight quick wins from v3 are landed. The highest-signal next sub-day fixes are now warning cleanup and stale-seam collapse:

1. **Align `scene_adapters.h` translator declarations with `scene_translator.h`** — fixes the current mismatched-tag and hidden-overload warning cluster in the presentation surface. (TD-07)
2. **Mark `Window_Command::processOk()` as `override`** in `runtimes/compat_js/window_compat.h`. (TD-07)
3. **Initialize `summary_line` explicitly in `MigrationWizardModel::SubsystemResult` construction** so aggregate-init warnings stop surfacing through integration builds. (TD-07)
4. **Remove the unused `WriteText()` helper and explicitly consume or annotate `fadeSeconds`** in the currently warning-producing integration/audio code. (TD-07)
5. **Collapse the duplicate incubator `EngineAssembly` / `MainAssembly` copies into one clearly named path** under `tools/incubator/`. (TD-01)
6. **Add an explicit thread-affinity contract or synchronization note to `AssetLoader`'s static caches** so the remaining ownership assumptions are visible in code. (TD-05)

---

## Risk Matrix

```
          │ Low Effort │ High Effort
──────────┼────────────┼────────────
High Risk │  TD-09*    │  TD-04
          │            │  TD-03
──────────┼────────────┼────────────
Low Risk  │  TD-05     │  TD-02
          │  TD-07**   │  TD-01
          │  TD-06     │  TD-10
          │  TD-08***  │
```

\* TD-09 is no longer silent, but content that depends on scripted conditions still cannot actually use them.  
\*\* TD-07 is low-severity debt, but the new warning baseline is only useful if the surfaced warnings are actually burned down.  
\*\*\* TD-08 (file size) is low risk *today* but compounds the cost of every other fix.

---

## Phased Remediation Roadmap

### Phase 1 — Immediate (This Sprint)

**Goal:** Close the newly narrowed trust and hygiene gaps around the otherwise healthier tree.

| Action | Items | Owner |
|---|---|---|
| Collapse duplicate incubator `EngineAssembly` / `MainAssembly` copies and keep `EngineShell` as the only canonical runtime entry point | TD-01 | — |
| Burn down the currently visible targeted-build warnings and decide whether to add a warnings-as-errors lane once clean | TD-07 | — |
| Add an explicit thread-affinity contract or synchronization to `AssetLoader`, and close the remaining compat sprite semantics TODOs | TD-05 | — |
| Keep docs and headers explicit that synthetic export/security/AI/cloud lanes are bounded and not shipping-equivalent | TD-06, TD-10 | — |
| Record the long-term stance on scripted ability conditions now that the unsupported path is explicit | TD-09 | — |

### Phase 2 — Near-Term (Next 2–4 Weeks)

**Goal:** Decide which partial surfaces are being expanded versus permanently bounded.

| Action | Items | Owner |
|---|---|---|
| Collect allocator/profiler evidence for the new frame-owned render path and decide how far textured/UI follow-through should go | TD-02 | — |
| Make the TD-03 scripting/plugin keep/delete decision and align file layout accordingly | TD-03 | — |
| Decide formula scope beyond the current bounded subset and keep unsupported cases explicit in migration/runtime output | TD-04 | — |
| Decide whether `activeCondition` / `passiveCondition` will gain a real evaluator or remain deliberately unsupported | TD-09 | — |
| Make the TD-10 AI/cloud keep/cut decision and align code layout accordingly | TD-10 | — |
| Start splitting `DiagnosticsWorkspace` and `urpg_project_audit` by domain now that the largest ownership/truth gaps are smaller | TD-08 | — |

### Phase 3 — Medium-Term (4–8 Weeks)

**Goal:** Expand only the surfaces the project actually intends to ship and delete or quarantine the rest.

| Action | Items | Owner |
|---|---|---|
| Expand battle/runtime parity only where the product roadmap actually needs it: formula features, troop-page breadth, transition/audio routing, and battle-scene placeholder visuals | TD-04 | — |
| If TD-03 stays harness-first, rename/relocate uniformly and delete dead editor/plugin bridge seams; if not, wire a real runtime | TD-03 | — |
| If harness: rename, relocate, and document as a harness everywhere; delete dead `ScriptBridge`/`PluginHost` paths | TD-03 | — |
| If live: complete QuickJS integration and retire fixture-backed execution | TD-03 | — |
| Wire GAF scripted conditions to the chosen evaluator or permanently scope them out of authored data | TD-09 | — |
| Implement a real `ICloudService` provider or remove the simulation layer from production-facing headers | TD-10 | — |
| Expand beyond the first real export smoke path and first renderer-backed visual capture path | TD-06 | — |
| Implement real `ResourceProtector::compress()` or remove it from the API surface | TD-06 | — |
| Begin splitting oversized compat manager files by responsibility boundary (not just size) | TD-08 | — |
| Schedule a dedicated ASan/LSan and Valgrind pass after Phase 2 lands | — | — |

---

## Detailed Findings

### TD-01 — `P1` Duplicate stale top-level assembly/editor seams still exist and are not build-real

**Effort:** M | **Risk:** Medium | **Blocks:** TD-03, TD-07

The repo no longer exposes these seams from production-looking engine/editor paths, but it still carries duplicate non-canonical assembly copies that are neither current nor part of the real runtime path.

**Evidence:**

- `tools/incubator/runtime/engine_assembly.h` and `tools/incubator/core/engine_assembly.h` both still declare `EngineAssembly`; the same duplicate pattern exists for `main_assembly.h`.
- `tools/incubator/runtime/engine_assembly.h:3` includes `../../../editor/editor_shell.h`, but the live header is `engine/core/editor/editor_shell.h`. The runtime incubator copy therefore drifts even harder than the core incubator copy.
- Both incubator copies still call `core::threading::ThreadRoles::instance()` plus `editor::EditorShell::instance().startup()/update()/render()`, but the live tree exposes `ThreadRegistry`, not `ThreadRoles`, and `EditorShell` does not provide that singleton lifecycle API.
- `tools/incubator/runtime/main_assembly.h:22-41` and `tools/incubator/core/main_assembly.h:22-41` both keep the same synthetic loop with a 10-frame cutoff and placeholder termination logic.
- `tools/ci/check_cmake_completeness.ps1` no longer needs orphan exemptions for the stale plugin/script harness seam after `plugin_host.*`, `script_bridge.*`, and `scripting_console.*` were relocated into `tools/incubator/editor/`; `doc_generator.*` and `battle_tactics_window.*` were already relocated into `tools/docs/incubator/` and `tools/incubator/ui/` so they no longer present as production engine/editor/UI orphans. The only remaining whitelist entry is the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- `CMakeLists.txt:426-685` registers tools, tests, and `urpg_presentation_release_validation`, but no target that compiles either incubating `EngineAssembly` or `MainAssembly` seam.

**Why it matters:**

- New contributors can still choose the wrong top-level seam, and there are now two nearly identical wrong places to do it.
- CI green does not mean all public-looking runtime or editor entry points are alive.
- The completeness gate is now much more truthful about stale runtime/editor seams, but it still does not prove that the remaining non-canonical assembly headers are singular, coherent, or retired.

**Recommended direction:**

- Declare `EngineShell` the only canonical top-level runtime surface.
- Collapse the duplicate incubator copies into one clearly named non-canonical location.
- Remove or archive the broken-include runtime copy instead of letting both versions drift.
- Keep the completeness whitelist limited to genuine standalone tools, not stale runtime/editor seams.
- Gate: `check_cmake_completeness.ps1` passes with no production-looking seam exemptions.

**Definition of Done:**

- [x] `EngineAssembly` and `MainAssembly` are relocated into an explicit incubator area under `tools/` with a pointer to `EngineShell`
- [x] Production-looking editor/plugin assembly surfaces are either compiled as real targets or moved out of canonical runtime/editor paths
- [x] `check_cmake_completeness.ps1` passes with no stale production-seam whitelist entries
- [ ] Only one incubator location remains for the stale assembly seam
- [ ] All CI and local gates reference `EngineShell` as the single top-level entry point in comments
- [ ] Broken include / nonexistent API references inside the incubator seam are removed or archived instead of silently drifting

---

### TD-02 — `P2` Render-path redesign is materially landed, but allocator proof and renderer-backed validation remain open

**Effort:** L | **Risk:** Medium | **Depends on:** nothing

URPG's most important optimization debt is concentrated in the render command path.

**Evidence:**

- `engine/core/render/render_layer.h:136-364` now stores frame-owned `FrameRenderCommand` values and keeps legacy `shared_ptr` conversion as a compatibility bridge rather than the canonical hot path.
- `engine/core/platform/renderer_backend.h:51-58` now accepts frame-owned command vectors through `processFrameCommands(...)`, with the legacy heap-backed entry point retained only for older overrides.
- `engine/core/scene/map_scene.cpp` and `runtimes/compat_js/window_compat.cpp` now submit text, rect, sprite, and tile work through `toFrameRenderCommand(...)` instead of making heap ownership the frame-loop contract.
- `engine/core/platform/opengl_renderer.cpp` now emits immediate-color triangles for `RectCommand` and bounded `stb_easy_font`-derived triangle batches for `TextCommand`; sprite/tile textured submission remains outside this TD-02 slice.
- `engine/core/platform/opengl_renderer.cpp:126-130` reports texture loading success without actually loading a texture.
- `engine/core/ui/ui_window.cpp:75-84` and `engine/core/ui/ui_command_list.cpp:66` mark text and gauge rendering as placeholders.

**Why it matters:**

- The hottest `shared_ptr` ownership debt is no longer the canonical frame path, but the remaining compatibility bridge means allocator savings are improved by structure rather than proven by instrumentation.
- Text and rect submission now have a real bounded backend path, but richer UI rendering and textured sprite/tile follow-through still need explicit validation.
- One bounded renderer-backed capture lane now exists, but the project still lacks broad scene coverage and CI-enforced golden validation for end-to-end visual truth.

**Recommended direction:**

- Keep pushing frame-owned command flow as the canonical path and shrink the legacy adapter surface over time.
- Add allocator instrumentation or profiling evidence so TD-02 closure is based on measured hot-loop behavior, not just structural inspection.
- Use the new bounded text/rect path as the floor, then decide separately whether richer font, sprite/tile, or UI rendering needs a larger renderer redesign.
- Expand the new renderer-backed visual capture lane beyond its first bounded rect/text smoke coverage before treating the render subsystem as broadly visually validated.

**Definition of Done:**

- [x] `RenderLayer` command storage no longer uses `shared_ptr` as the canonical per-frame path
- [x] `renderer_backend.h` abstraction is updated to accept frame-owned command view
- [x] `OpenGLRenderer` produces real draw calls for `TextCommand` and `RectCommand` (bounded immediate-mode implementation)
- [ ] Allocation instrumentation or profiler-backed allocator data confirms the render path no longer performs per-frame heap allocation under normal scene load

---

### TD-03 — `P2` JS/plugin/editor compatibility stack remains harness-first, not live runtime

**Effort:** XL | **Risk:** Medium | **Depends on:** TD-01 cleanup

The compat runtime is better documented than before, but it is still a harness, not a live production scripting/plugin lane.

**Evidence:**

- `runtimes/compat_js/quickjs_runtime.cpp:4-5` explicitly states the file is fixture-backed and "not a live QuickJS runtime".
- `runtimes/compat_js/plugin_manager.cpp:1038-1040` describes its runtime bridge as fixture-backed command execution.
- `runtimes/compat_js/plugin_manager.cpp:1209-1238` documents that plugin loading, reload, directory scan, and execution depend on fixture JSON plugins and stub JS contexts.
- `tools/incubator/editor/script_bridge.cpp:10-32` keeps both runtime/context pointers at `nullptr` and returns placeholder `42` from `eval()`.
- `tools/incubator/editor/plugin_host.cpp:56-65` says live DLL/JS loading is not implemented and fails unregistered plugins.
- `tools/incubator/editor/script_bridge.h:51-53` labels the QuickJS state as placeholder pointers.

**Why it matters:**

- The compat layer is valuable for deterministic contract coverage, but it is not interchangeable with a live script runtime.
- The stale editor-layer bridge and the active compat harness describe similar responsibilities with very different levels of truth.
- Both surfaces existing simultaneously invites confusion about which scripting path is real.

**Recommended direction — choose one path explicitly:**

**Option A — Permanently bounded harness:**
- Rename all harness entry points and files with a `_harness` or `_fixture` suffix.
- Delete `ScriptBridge`, `PluginHost`, and the stale editor plugin path entirely.
- Document the harness boundary in every public header and in the contributor guide.

**Option B — Pursue live integration:**
- Complete QuickJS initialization in `quickjs_runtime.cpp`.
- Wire `ScriptBridge::eval()` to a real context.
- Retire fixture-backed plugin execution behind a compile-time flag until it can be deleted.

> Either choice is acceptable. The current state — where both paths exist and neither is authoritative — is not.

**Definition of Done:**

- [ ] A single scripting/plugin strategy is documented in the contributor guide
- [ ] All harness code is either deleted or uniformly named and placed in a `harness/` or `fixtures/` directory
- [ ] No public production-looking header holds `nullptr` runtime pointers without a clear comment on intent

---

### TD-04 — `P2` Battle parity remains bounded by a subset evaluator, placeholder visuals, and partial routing

**Effort:** L | **Risk:** High | **Depends on:** nothing for baseline correctness; TD-03 only matters for full live-script parity

The deterministic native and compat battle slice has improved substantially, but full RPG Maker-style formula, event, transition, and visual parity is still not there.

**Evidence:**

- `runtimes/compat_js/battle_manager.h:138-146,260-262,284-291,405` marks transition/audio routing, formula breadth, troop-page execution breadth, and playback as `PARTIAL`.
- `runtimes/compat_js/battle_manager.cpp:355-371` retains battle transition/background/audio state for readback but does not route it to live scene/audio backends.
- `runtimes/compat_js/battle_manager.cpp` still limits troop page execution to a bounded interpreter subset even after the variable-branch expansion.
- `engine/core/scene/combat_formula.h` now implements a real recursive-descent parser for a bounded arithmetic/stat subset, but anything outside that subset still falls back with named reasons rather than executing full MZ formula semantics.
- `engine/core/battle/battle_migration.h:258-261` still inserts a placeholder `common_event` effect when a page has only unmapped event commands.
- `engine/core/scene/battle_scene.cpp:499,531-539` still uses white-texture placeholder boxes for damage popups, guard markers, and state icons.

**Why it matters:**

- Content with nontrivial formula strings or broader troop-page scripting still collapses to a bounded supported subset.
- Current tests prove deterministic closure for a bounded slice, not full MZ combat semantics.
- Some battle-scene visuals are still contract placeholders, so even the native scene path remains only partially presentation-real.
- **This is a correctness gap, not just a performance or polish issue.**

**Recommended direction:**

- Decide whether the current bounded formula evaluator is the long-term floor or only an interim subset.
- If formula breadth is in scope, expand from the current parser to the additional operators/functions/semantics the product actually needs instead of treating "formula support" as all-or-nothing.
- If broader parity is not in scope, keep unsupported formulas, troop-page commands, and battle-scene placeholders explicit in migration/runtime diagnostics so authors never mistake the current lane for full MZ behavior.
- Route transition/background/audio metadata into real scene/audio behavior if those hooks are meant to be product surfaces; otherwise keep the `PARTIAL` boundary explicit and conservative.
- Replace battle-scene white-texture stand-ins with a real bounded visual contract or with explicit diagnostics/comments that state the placeholder scope.

**Definition of Done:**

- [ ] A formula strategy decision is recorded in the contributor guide or audit document
- [ ] The bounded formula evaluator is either expanded deliberately or declared the permanent supported subset
- [ ] Unsupported formula strings, troop-page commands, and migration fallbacks remain named and non-silent
- [ ] No `PARTIAL` marker in `battle_manager.h` lacks an associated test verifying the partial boundary
- [ ] Battle-scene placeholder popup/state/guard visuals are either replaced or explicitly scoped as a deliberate temporary contract

**Progress on 2026-04-22:**

- `BattleScene` no longer silently falls back to `Grassland`; it emits `MISSING_BATTLEBACK`.
- `CombatFormula` now supports a bounded arithmetic/stat subset and returns named fallback reasons for unsupported or malformed expressions.
- `BattleManager::applySkill()` / `applyItem()` now use that bounded subset when present and expose unsupported formulas in method status text instead of silently degrading.
- Compat troop-page handling now supports variable conditional branches against constants and live variable operands.
- Migration now preserves unsupported action `effects[]` as `_compat_effect_fallbacks` rather than hiding them behind a generic warning.
- **Remaining gap:** full formula/event parity is still not there, transition/audio routing remains partial, and the native battle scene still carries placeholder popup/state/guard visuals.

---

### TD-05 — `P3` Remaining ownership debt is narrower and now concentrated in global cache/thread-affinity assumptions and compat sprite TODOs

**Effort:** S | **Risk:** Medium | **Depends on:** nothing

No fresh must-fix leak at the level of the earlier AudioManager SE issue remains in this slice, but a smaller set of ownership assumptions still deserves cleanup before these paths are treated as hardened.

**Evidence:**

- `engine/core/render/asset_loader.h:24-47` now exposes bounded, clearable static caches, but they remain process-global and unsynchronized.
- `engine/core/render/asset_loader.cpp` uses those static caches directly with no explicit thread-affinity comment or locking contract.
- `engine/core/scene/map_scene.cpp` and `engine/core/scene/battle_scene.cpp` still load runtime textures through `AssetLoader`, so the global-cache assumption remains reachable from live scene code.
- `engine/core/editor/plugin_api.cpp` no longer stores a raw process-global `World*`; it now uses a scoped thread-local binding stack. That earlier high-risk item is closed.
- `engine/core/threading/thread_roles.h` now uses `std::shared_mutex`, and concurrent tests exist. That earlier map-race item is closed.
- `runtimes/compat_js/window_compat.cpp:2365,2370,2469` still leaves `Sprite_Character` source-rect updates, sprite animation/update semantics, and `Sprite_Actor` motion-start behavior as TODOs even though deterministic reload/release is now handled.

**Important nuance:**

- `Window_Base` contents-handle cleanup itself still looks correct: `window_compat.cpp:1007-1012` erases entries from `contentsBitmaps_`.
- The SE leak from the previous audit is closed (see Resolved Debt section).

**Why it matters:**

- The remaining `AssetLoader` debt is less about unbounded growth now and more about hidden global-state/thread-affinity assumptions.
- The earlier `PluginAPI` and `ThreadRegistry` ownership hazards are no longer the headline risk in this category.
- The remaining compat sprite TODOs mean bitmap-backed sprite work still lacks finished source-rect and motion semantics even after lifetime cleanup landed.

**Recommended direction:**

- Either document `AssetLoader` as main-thread-only or make its remaining global caches explicitly thread-safe.
- Keep `clearCaches()` as the explicit reset boundary and surface that contract in the code/docs that use it.
- Finish compat sprite source-rect, update-loop, and motion semantics before treating those classes as broader sprite-authoring/runtime surfaces.

**Definition of Done:**

- [x] `AssetLoader` caches have a documented size bound and an explicit `clear()` path
- [x] No raw process-global `World*` remains in plugin API code
- [x] `ThreadRegistry` map access is synchronized
- [x] At least one multi-threaded `ThreadRegistry` test covers concurrent registration and lookup
- [ ] `AssetLoader` thread-affinity / synchronization expectations are explicit
- [ ] Remaining compat sprite TODOs are resolved before broader bitmap/animation loading is wired

**Progress on 2026-04-22:**

- `AssetLoader` now uses bounded caches for successful and missing texture lookups, exposes `clearCaches()` as an explicit reset boundary for tests/reset flows, and tests pin both eviction/re-warning and manual reset behavior.
- `ThreadRegistry` map access is synchronized with `std::shared_mutex`, and concurrent registration/query coverage exists.
- The raw global plugin `World*` binding has been replaced with a scoped thread-local binding stack.
- `Sprite_Character` and `Sprite_Actor` bitmap handles now reload and release deterministically across identity changes and destruction.
- **Result:** the highest-risk TD-05 ownership items from the prior pass are now substantially reduced. The remaining work is smaller and mostly about making the surviving global-state and sprite-semantics assumptions explicit or complete.

---

### TD-06 — `P2` Export, security, and visual validation are still synthetic

**Effort:** M | **Risk:** Low | **Depends on:** TD-02 (for renderer-backed visual capture)

The release path is now partially real, but most of the lane still proves bounded contract shape more than shipping behavior.

**Evidence:**

- `engine/core/tools/export_packager.cpp` still runs a license audit that always succeeds and still emits a placeholder `data.pck`.
- `engine/core/tools/export_packager.cpp` now supports one bounded real Windows launch smoke path when a real runtime binary is explicitly provided, but Linux, macOS, and Web synthesis still write placeholder artifacts.
- `tools/export/export_smoke_app.cpp` now provides a dedicated headless native launcher for that bounded Windows smoke lane, and `tests/unit/test_export_packager.cpp` launches the staged export and proves execution through a marker file.
- `engine/core/export/export_validator.cpp:7-71` still validates directory contents and filename patterns, not launchability, signing, dependency resolution, or runtime behavior.
- `engine/core/security/resource_protector.h:22-25` returns the input buffer unchanged from `compress()`.
- `engine/core/testing/visual_regression_harness.cpp` now includes a bounded hidden SDL/OpenGL capture helper, and `tests/snapshot/test_renderer_backed_visual_capture.cpp` plus committed clear-frame/full-frame-rect/inset-rect goldens prove one OpenGL-enabled local nightly-lane renderer-backed rect/text capture path.

**Why it matters:**

- These lanes are useful contract guards, but they are still not production release validation.
- A green result here should be read as `one bounded launch smoke path plus synthetic validation elsewhere`, not `shipping pipeline proven`.
- `ResourceProtector` presenting itself as a compression layer while passing data through unchanged is actively misleading.

**Recommended direction:**

- Keep audit and doc claims tightly bounded: use "synthetic pipeline intact" language, not "export validated."
- Expand beyond the first real export smoke path once one target can stage a launchable artifact.
- Expand the first renderer-backed visual capture path now that `OpenGLRenderer` has a bounded real text/rect draw path.
- Either implement real compression in `ResourceProtector::compress()` or rename it to `passthrough()` / remove it from the public API until it is real.

**Definition of Done:**

- [ ] Docs and CI output use "synthetic" qualifier everywhere these lanes are cited
- [ ] `ResourceProtector::compress()` either compresses or is removed/renamed
- [x] At least one export target produces a binary that passes a basic launch smoke test
- [x] Visual regression harness captures at least one real renderer frame (post TD-02)

**Progress on 2026-04-22:**

- `ResourceProtector` no longer presents passthrough bytes as if they were compressed; the API now explicitly exposes that compression is not implemented.
- Tests now pin the current synthetic behavior as passthrough plus reversible XOR obfuscation.
- `ExportPackager` now has one bounded real Windows smoke lane: it can stage a real headless launcher plus runtime DLLs when given an explicit runtime-binary path, and focused tests prove the staged `game.exe` actually launches and writes its smoke marker.
- A first bounded OpenGL-enabled local renderer-backed visual capture lane is now landed through the visual regression harness.
- **Remaining gap:** export packaging still uses placeholder asset bundling overall, non-Windows executable synthesis remains synthetic, security hardening is still limited, and visual validation still lacks CI golden enforcement plus broader scene coverage.

---

### TD-07 — `P3` Warning policy landed, but warning cleanup and stricter gate posture still lag

**Effort:** S | **Risk:** Low | **Depends on:** nothing

URPG's build governance is in better shape than it was in the previous pass. The remaining debt is no longer missing policy; it is the backlog the new policy exposed.

**Evidence:**

- `CMakeLists.txt:10-17` now enables repo-wide warnings through `urpg_project_warnings` with `/W4` on MSVC and `-Wall -Wextra` on GCC/Clang-family compilers.
- `CMakeLists.txt` now pins `stb`; the floating-dependency debt from the prior pass is closed.
- A fresh targeted build of `urpg_integration_tests`, `urpg_snapshot_tests`, `urpg_compat_tests`, and `urpg_presentation_release_validation` succeeds but surfaces 11 unique warnings, including:
  - presentation translator signature / `class` vs `struct` mismatches in `engine/core/presentation/scene_adapters.h`
  - a missing `override` on `Window_Command::processOk()` in `runtimes/compat_js/window_compat.h`
  - aggregate-initialization warning surfacing through `editor/diagnostics/migration_wizard_model.h`
  - unused `fadeSeconds` in `engine/core/audio/audio_core.h`
  - unused test helper noise in `tests/integration/test_wave1_closure_integration.cpp`
- Exact label discovery is now cleanly anchored: `^pr$` => 918, `^nightly$` => 18, `^weekly$` => 44.
- `tools/ci/run_local_gates.ps1` now mirrors the key Gate 1 governance checks that were missing in the previous pass.
- `tools/ci/check_cmake_completeness.ps1` now burns down the stale plugin/script seam exemptions; the remaining whitelist is limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.

**Why it matters:**

- Warning flags only raise code quality if the warning backlog is treated as actionable instead of normalized noise.
- The tree is now close enough to a useful stricter posture that each lingering warning matters more, not less.

**Recommended direction:**

- Burn down the currently visible warning set now that repo-wide flags are in place.
- Keep exact-label discovery and local/CI gate parity anchored so future count drift is easy to spot.
- Consider introducing a focused warnings-as-errors lane once the currently visible backlog reaches zero.
- Consider adding a clang-tidy or cppcheck preset as an optional opt-in gate.

**Definition of Done:**

- [x] `stb` is pinned to a specific revision
- [x] Repo-wide warning flags are present in `CMakeLists.txt`
- [x] `run_local_gates.ps1` runs the key Gate 1 governance checks that were missing in the prior pass
- [x] All ctest label patterns in CI and local scripts are anchored
- [ ] The currently visible targeted-build warning backlog is reduced to zero or explicitly waived
- [ ] A focused warnings-as-errors lane is either enabled or deliberately deferred with rationale
- [ ] `check_cmake_completeness.ps1` TODO is resolved or tracked as a separate issue

**Progress on 2026-04-22:**

- `stb` is pinned.
- Repo-wide warning flags are enabled via a shared compile-options target.
- CI and local gate label filters are anchored.
- `run_local_gates.ps1` now includes the key Gate 1 governance checks that were missing in the audit snapshot.
- The fresh targeted build proves the current debt has shifted to warning cleanup, not policy absence.
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

- Wait until the TD-03 scripting strategy decision is made before splitting plugin manager files — the split boundaries depend on what stays vs. what is deleted.
- Decompose `DiagnosticsWorkspace` and `urpg_project_audit` around tab and report domains now; these are not gated on other decisions.
- Split compat manager files by responsibility boundary, not by line count alone.
- In the next audit: check whether the largest five files are actually shrinking.

**Definition of Done:**

- [ ] TD-03 strategy decision made (prerequisite for plugin manager splits)
- [ ] `DiagnosticsWorkspace` and `urpg_project_audit` split into domain-aligned files (each under 1,000 lines)
- [ ] Each of the top-five compat files has a split plan tracked as a backlog item
- [ ] Next audit confirms at least two of the top-five files have decreased in size

---

### TD-09 — `P2` GAF scripted conditions are explicit and test-backed, but still unsupported

**Effort:** M | **Risk:** Medium | **Depends on:** TD-03 (for full live-script wiring), but the explicit unsupported path is independent

The Gameplay Ability Framework still has no evaluator for scripted activation or passive cancellation conditions, but the current gap is no longer silent. The debt has shifted from "hidden bypass" to "authored feature still unsupported."

**Evidence:**

- `engine/core/ability/gameplay_ability.h` still declares `activeCondition` ("Must evaluate true to activate") and `passiveCondition` ("While active, if false, ability cancels") as first-class fields on `ActivationInfo`.
- `engine/core/ability/gameplay_ability.cpp` now blocks non-empty `activeCondition` values with the explicit reason `active_condition_unsupported` and a detail string that includes the authored expression.
- `passiveCondition` is still not evaluated anywhere in the runtime update path; the current contract is that it is out of scope until a real evaluator exists.
- `AbilitySystemComponent::canApplyEffect()` still returns `true` unconditionally and is only documented/test-pinned as an always-true admission gate for now.
- `tests/unit/test_ability_activation.cpp` now exercises the non-empty `activeCondition` path, proves the explicit diagnostic, and pins the current passive-condition non-behavior.

**Why it matters:**

- Any ability authored with a scripted condition string still cannot actually use that authored data.
- The active-condition failure mode is now visible and honest, which is much better than before, but it is still a product limitation until an evaluator exists or the field is scoped out.
- Passive cancellation still does not happen, so authored passive strings remain misleading unless the scope decision is made explicit in docs and tooling.

**Recommended direction:**

First, decide scope: are scripted conditions evaluated against the QuickJS runtime (TD-03 dependency), against a native evaluator, or permanently treated as out-of-scope authoring data?

**Regardless of the long-term strategy:**
- Keep the explicit unsupported diagnostic path; do not regress back to silent behavior.
- Keep at least one focused activation test that exercises a non-empty `activeCondition`.

**Once a scope decision is chosen:**
- Wire `resolveActiveCondition()` to the chosen evaluator.
- Wire `passiveCondition` to a cancellation check in `update()`.
- Implement `canApplyEffect()` tag gate once effects have Tag requirements.

**Definition of Done:**

- [x] Abilities with a non-empty `activeCondition` produce a named diagnostic — they do not silently block or silently pass
- [x] `passiveCondition` is either evaluated in `update()` or its absence is documented as out-of-scope with a matching test that asserts the field is not read
- [x] `canApplyEffect()` either checks effect Tag requirements or is documented as always-true with a test pinning that behavior
- [x] At least one ability activation test covers a non-empty `activeCondition` value
- [ ] A long-term evaluator-or-scope decision is recorded for `activeCondition` / `passiveCondition`

**Progress on 2026-04-22:**

- Non-empty `activeCondition` values now produce explicit `active_condition_unsupported` diagnostics with detail strings that surface in execution history and replay diagnostics.
- Focused activation tests now cover the non-empty `activeCondition` path, pin that `passiveCondition` is ignored by the current runtime, and prove that passive strings do not block execution.
- `canApplyEffect()` is now explicitly documented and test-pinned as an always-true admission gate; modifier `requiredTag` checks still apply later during attribute resolution.
- **Remaining gap:** the unsupported path is now honest, but authored scripted conditions still do not execute.

---

### TD-10 — `P2` AI/cloud connectivity layer is entirely simulated with no live transport

**Effort:** M | **Risk:** Medium | **Depends on:** nothing (cut decision) or TD-03-adjacent (if wiring to live scripting)

The engine declares a full AI chat and cloud-sync integration surface — OpenAI, Llama.cpp local inference, cross-device cloud sync, knowledge update hooks — but every call site either does nothing or routes through an in-memory stub with no real transport.

**Evidence:**

- `engine/core/ai/ai_connectivity.h`: `OpenAIChatService::requestResponse()` constructs a JSON body, documents the required HTTP steps, and then ends with a commented-out `callback(...)` call. **The function does nothing when called.** `LlamaLocalService::requestResponse()` is identical — commented-out callback, no inference.
- `engine/core/social/cloud_service.h`: `LocalInMemoryCloudService::syncToCloud()` stores data in an in-process `std::map`. `LocalInMemoryCloudService::initialize()` only allows `CloudProvider::LocalSimulated`; live-looking providers fail with a `NOT LIVE` result instead of pretending to connect. No network operation occurs.
- `engine/core/message/ai_sync_coordinator.cpp` now returns `false` from `checkForRemoteKnowledgeUpdates()` with an explicit `NOT LIVE` comment; the misleading always-true behavior from the prior pass is gone.
- `engine/core/ai/personality_registry.h` is a fixed string-template selector, not a dynamic registry or provider-backed catalog.
- `engine/core/message/ai_sync_coordinator.h:13-14` already documents that, with the in-tree `LocalInMemoryCloudService` (`CloudServiceStub` compatibility alias), the current behavior remains local in-memory rather than live cloud sync. `CMakeLists.txt` compiles `engine/core/message/ai_sync_coordinator.cpp` (line 199), and the tree now includes direct unit coverage for local-only initialization plus `AISyncCoordinator` sync/restore behavior.

**Why it matters:**

- Callers of `OpenAIChatService::requestResponse()` receive no response and no error. This is worse than a clear "not implemented" return — it silently drops the request.
- The cloud sync surface presents a realistic API (`initialize`, `syncToCloud`, `fetchFromCloud`, `listRemoteKeys`) backed by an in-memory map that is discarded at process exit.
- The gap between the declared interface and the actual behavior is wide enough that a future developer wiring up a real game feature against this API will believe the integration is working until they check network traffic.
- `PersonalityRegistry` is hardcoded to four string templates — it is not a registry, it is a switch statement. It will not scale without redesign.

**Recommended direction — choose one path explicitly:**

**Option A — Cut the simulation layer:**
- Delete `OpenAIChatService`, `LlamaLocalService`, and the production-facing cross-device sync facade from production headers.
- Keep `IChatService` and `ICloudService` as abstract interfaces only.
- Document in the contributor guide: "Live AI/cloud integration requires a production IChatService/ICloudService implementation. None is in-tree."

**Option B — Pursue live integration:**
- Wire `OpenAIChatService` to a real HTTP client (libcurl or cpp-httplib, both already viable given the SDL2 dependency).
- Make `LocalInMemoryCloudService` fail loudly rather than silently succeed when no real provider is configured.
- Add explicit tests for chat request failure/success behavior and for non-stub cloud provider wiring.

> Either choice closes the gap. The current state — where calling a chat or sync function does nothing but the interface implies a real operation — is not acceptable in production-proximate code.

**Definition of Done:**

- [ ] A single AI/cloud strategy is documented in the contributor guide
- [ ] `OpenAIChatService::requestResponse()` either makes a real HTTP request or is removed from the production headers
- [x] `CloudServiceStub` was renamed to `LocalInMemoryCloudService` with `CloudServiceStub` retained only as a compatibility alias, and live-provider initialization now fails loudly instead of pretending to connect
- [x] `checkForRemoteKnowledgeUpdates()` no longer pretends success; it now returns `false` with a clear `NOT LIVE` comment
- [ ] AI/chat transport and cloud-provider behavior have direct tests, or the related production-facing headers are moved to a clearly labeled `experiments/` or `future/` directory

**Progress on 2026-04-22:**

- The AI/chat request paths are now explicitly marked `NOT LIVE`.
- `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` now returns `false` instead of implying successful remote polling.
- Cloud behavior is still local/in-memory only, and no live transport was added.
- The in-tree double now exposes its local-only scope at the type level (`LocalInMemoryCloudService`) and has direct unit coverage for local-only initialization and coordinator sync/restore behavior.
- **Remaining gap:** the keep/cut strategy decision for this entire layer is still open.

---

### TD-11 — Closed in current tree: CopilotKernel now uses predicate-based constraint enforcement

The core implementation gap from the previous pass is closed. `CanonConstraint` now carries a predicate, `validateProposal()` iterates real predicates, and `tests/test_copilot_kernel.cpp` proves predicate-based rejection instead of substring mocks. Any follow-up work from here is rule-coverage expansion, not technical-debt replacement of the enforcement mechanism itself.

---

## Cross-Cutting Themes

This pass reduces to five big themes:

1. **Build-truth drift is now narrower but sharper** — the remaining problem is duplicate/non-canonical entry seams plus a warning backlog, not missing baseline governance. (TD-01, TD-07)
2. **Hot-loop and visual-completeness debt are still concentrated in the render/UI stack** — ownership is better, but textured/UI follow-through and allocator proof remain open. (TD-02, TD-06)
3. **Harness-versus-runtime ambiguity is still the defining product truth problem** — the compat and plugin layers are useful, but still bounded by harness semantics. (TD-03, TD-04)
4. **Ownership debt is smaller and more localized** — the earlier headline risks are reduced; what remains is surviving process-global state and unfinished compat sprite semantics. (TD-05)
5. **Unsupported features are now more honest, but still unresolved as product decisions** — GAF conditions and AI/cloud transport are clearer than before, yet still need either real implementations or deliberate scoping cuts. (TD-09, TD-10)

---

## Recommended Action Order

| Step | Action | Phase | TD Items |
|---|---|---|---|
| 1 | Collapse duplicate incubator `EngineAssembly` / `MainAssembly` copies and keep `EngineShell` as the only canonical runtime entry point | 1 | TD-01 |
| 2 | Burn down the currently visible warning backlog and decide whether to add a focused warnings-as-errors lane | 1 | TD-07 |
| 3 | Make `AssetLoader` thread-affinity explicit and close the remaining compat sprite TODOs | 1 | TD-05 |
| 4 | Decide the TD-03 scripting/plugin strategy and align code layout accordingly | 2 | TD-03 |
| 5 | Decide battle/formula scope beyond the current subset and close the most visible battle-scene placeholders | 2 | TD-04 |
| 6 | Record the long-term evaluator-or-scope decision for scripted ability conditions | 2 | TD-09 |
| 7 | Collect allocator/profiler evidence for the frame-owned render path and decide how far textured/UI follow-through should go | 2 | TD-02 |
| 8 | Make an explicit keep/cut decision for the AI/cloud layer; either wire real transport or delete/quarantine the simulation facades | 2–3 | TD-10 |
| 9 | Tighten export/security/visual-validation claims and expand at least one more real lane in each area | 3 | TD-06 |
| 10 | Begin splitting `DiagnosticsWorkspace` and `urpg_project_audit`, then plan the compat-manager splits behind TD-03 | 3 | TD-08 |
| 11 | Schedule ASan/LSan and Valgrind pass after the next render/allocator follow-through slice lands | 3 | — |

**Progress note on 2026-04-22:**

- Step 2's enabling work is landed, and the remaining work is warning cleanup.
- Step 3's highest-risk slice is landed; the remaining work is the smaller `AssetLoader` contract and compat sprite TODO cleanup.
- Step 5 is partially landed through the bounded formula subset, explicit fallback surfacing, and better battle migration diagnostics, but not full parity.
- Step 6 is partially landed through explicit unsupported-condition diagnostics and tests, but not full evaluation.
- Step 7 is partially landed through value-owned `FrameRenderCommand` storage, frame-buffer backend consumption, real immediate-mode text/rect submission, focused pre-init/no-context test coverage, and one bounded nightly renderer-backed visual capture lane; the remaining TD-02 follow-through is allocator proof plus richer textured/UI coverage.
- Step 8 is partially landed through truthfulness fixes, but not the keep/cut decision.

---

## Conclusion

URPG is not primarily suffering from missing documentation or missing tests. It is suffering from a smaller, harder class of debt: **non-canonical entry seams, harness-versus-runtime ambiguity, partial visual/runtime closure, a warning backlog exposed by the new warning baseline, and a handful of remaining product-scope decisions that code can now name honestly but not yet execute.**

The fresh sweep changed the audit in four important ways:

- **TD-07 shifted from policy debt to cleanup debt.** Warning flags, pinned dependencies, and anchored label filters are in place; the new problem is the warning backlog they surfaced.
- **TD-05 is materially smaller than the earlier snapshot implied.** The highest-risk cache/global/thread issues are no longer the category headline; the remaining work is explicit thread-affinity and compat sprite follow-through.
- **TD-11 is closed in substance.** Copilot constraints now use predicates, so that mechanism is no longer an active debt leader.
- **TD-01 is sharper than the current doc previously stated.** The stale assembly seam is truthful about being non-canonical, but it still exists twice and one copy has a broken include path.

That is a healthier kind of debt. The repo already has the governance discipline to expose and close it. The most leveraged next actions are to collapse the duplicate incubator seam, burn down the warning backlog now that the warning baseline exists, and make the remaining partial runtime surfaces either more real or more explicitly out of scope.

> **Next audit trigger:** Re-run this audit after Phase 2 completes. Add an ASan/Valgrind pass to the next cycle — the current finding list is based entirely on code inspection and may be undercounting live memory issues.
