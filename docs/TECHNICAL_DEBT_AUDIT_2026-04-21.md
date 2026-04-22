# Technical Debt Audit

**Status Date:** 2026-04-22  
**Document version:** v3  
**Supersedes:** v2 (2026-04-21)

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
ctest --test-dir build/dev-ninja-debug -N -L pr
ctest --test-dir build/dev-ninja-debug -N -L "^pr$"
ctest --test-dir build/dev-ninja-debug -N -L nightly
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
- `ctest -N -L pr` enumerates **892** tests.
- `ctest -N -L "^pr$"` enumerates **891** tests — the one-test delta is label-regex bleed (see TD-07).
- `ctest -N -L nightly` enumerates **14** tests, including `urpg_presentation_release_validation`.

**Largest code hotspots:**

| File | Lines | Primary concern |
|---|---|---|
| `tests/compat/test_compat_plugin_failure_diagnostics.cpp` | 4,033 | Scope sprawl |
| `tests/compat/test_compat_plugin_fixtures.cpp` | 2,859 | Scope sprawl |
| `tests/unit/test_diagnostics_workspace.cpp` | 2,376 | Scope sprawl |
| `runtimes/compat_js/plugin_manager.cpp` | 2,350 | Harness complexity |
| `runtimes/compat_js/window_compat.cpp` | 2,275 | Harness complexity |
| `editor/diagnostics/diagnostics_workspace.cpp` | 2,198 | Domain mixing |
| `tools/audit/urpg_project_audit.cpp` | 2,183 | Domain mixing |

---

## Resolved Debt ✓

These items were open in the previous pass and are now closed. They are recorded here to preserve audit continuity and recognize real progress.

| ID | Title | Resolution |
|---|---|---|
| TD-R01 | AudioManager SE-channel leak | `runtimes/compat_js/audio_manager.cpp:737-745` now destroys finished SE channels during `update()`. Confirmed closed by code inspection. |

> **Pattern to replicate:** The SE fix used a clear ownership model (destroy on `update()`). Apply this same "ownership contract finalized at bounded lifecycle point" pattern to the sprite handle and asset cache issues in TD-05.

## Session Progress Update — 2026-04-22

This audit remains a valid snapshot of what was open on 2026-04-21, but several items moved forward during the 2026-04-22 execution session. Treat the sections below as the current progress overlay on top of the original audit.

**Session highlights:**

- **TD-07 quick wins landed:** `stb` is pinned, repo-wide warning flags are enabled through a shared warnings target, Gate label filters were anchored to `^pr$`, `^nightly$`, and `^weekly$`, and `tools/ci/run_local_gates.ps1` now includes `check_save_policy_governance.ps1`, `check_breaking_changes.ps1`, and `check_cmake_completeness.ps1`.
- **TD-01 audit noise reduced:** `check_cmake_completeness.ps1` no longer exempts seven ability/editor tests that are already compiled into `urpg_tests`, and the stale `plugin_host.*`, `script_bridge.*`, `scripting_console.*`, `doc_generator.*`, and `battle_tactics_window.*` surfaces have all been relocated into explicit incubator paths under `tools/`; the remaining whitelist is now limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- **TD-01 truthfulness improved:** the stale `EngineAssembly` and `MainAssembly` seam now lives under `tools/incubator/runtime/` with `EngineShell` as the live entry point, and the stale editor/plugin seam no longer hides behind completeness-script exemptions.
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
- Focused `ctest --test-dir build/dev-ninja-debug --output-on-failure -R ...` lanes covering Copilot, battle scene, AssetLoader, ThreadRegistry, Plugin API, compat sprites, battle migration, combat formulas, DataManager, BattleManager formula fallback, ResourceProtector, and troop event variable-branch behavior all passed.

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

- [x] TD-01: Resolve the remaining non-canonical `EngineAssembly` / `MainAssembly` top-level seam in production-looking paths
- [x] TD-01: Eliminate the `check_cmake_completeness.ps1` orphan whitelist for stale non-canonical seams
- [x] TD-02: Redesign the render command path away from heap-backed per-frame `shared_ptr<RenderCommand>`
- [x] TD-02: Implement bounded real text and rect rendering in `OpenGLRenderer`
- [ ] TD-03: Make the keep/delete strategy decision for the compat/plugin/editor harness path
- [ ] TD-04: Decide and document the long-term formula strategy beyond the bounded subset
- [ ] TD-04: Close remaining partial battle parity gaps, especially transition/audio routing and broader troop-page coverage
- [x] TD-05: Document the final `AssetLoader` cache management contract, including clear/eviction expectations
- [x] TD-06: Add at least one real export smoke path
- [x] TD-06: Add at least one renderer-backed visual capture path once TD-02 lands
- [ ] TD-10: Decide whether the AI/cloud layer is being cut or wired to real transport
- [ ] TD-10: Either add a real chat/cloud provider or finish restricting the remaining local-only cloud surfaces beyond the renamed in-memory double
- [ ] TD-08: Split oversized compat and diagnostics files along domain boundaries

---

## Executive Summary

URPG is still stronger on documentation honesty and automated governance than on runtime closure. The most important debt is no longer "the repo is chaotic." It is that the **living, test-validated path and the public-looking top-level path are still not the same thing.**

The live path is `EngineShell` plus a bounded set of native and compat subsystems. Around that core, the tree still carries:

- A stale top-level assembly/editor/plugin stack that is not part of the real build graph
- A render path whose hottest ownership debt is materially reduced, but which still needs allocator proof, textured sprite/tile follow-through, and broader visual validation hardening
- Compat/runtime surfaces that are honestly labeled `PARTIAL`, but still do not amount to live MZ-equivalent execution
- Synthetic export, security, and visual-validation lanes that prove contract shape more than production behavior
- Cache and lifetime patterns that are not yet hardened for long-running or multi-threaded use
- A Gameplay Ability Framework where scripted activation and passive conditions are declared in every ability's data model but silently bypassed at runtime — abilities that should fail scripted gates always pass
- An AI/cloud connectivity layer (`OpenAIChatService`, `LlamaLocalService`, `AISyncCoordinator`, `LocalInMemoryCloudService`) with no live HTTP transport, commented-out callbacks, and only a local in-memory provider path in tree
- A CopilotKernel "canon validator" whose constraint enforcement amounts to searching the proposal description string for the word `"violate"` — tests pass against mock behavior, not real predicate logic

---

## Priority Summary

> **Effort key:** S = hours, M = 1–3 days, L = 1–2 weeks, XL = multi-week  
> **Risk key:** Correctness risk if left unaddressed — High / Medium / Low

| ID | Severity | Effort | Risk | Blocks | Title |
|---|---|---|---|---|---|
| TD-01 | `P1` | M | Medium | TD-03, TD-07 | Stale top-level assembly/editor path is not build-real |
| TD-02 | `P2` | L | Medium | — | Render-path redesign is materially landed, but allocator proof and textured sprite/tile follow-through remain open |
| TD-03 | `P2` | XL | Medium | — | JS/plugin/editor compat stack is harness-first, not live |
| TD-04 | `P2` | L | High | — | Battle and formula parity bounded by partial interpreters |
| TD-05 | `P2` | M | High | — | Memory-growth and lifetime ownership debt in caches and globals |
| TD-06 | `P2` | M | Low | — | Export, security, and visual validation are synthetic |
| TD-07 | `P3` | S | Low | — | Build reproducibility and CI fidelity below baseline |
| TD-08 | `P3` | L | Low | — | Complexity concentrated in oversized compat/diagnostics files |
| TD-09 | `P2` | M | High | — | GAF scripted conditions are declared but never evaluated |
| TD-10 | `P2` | M | Medium | — | AI/cloud connectivity layer is entirely simulated with no live transport |
| TD-11 | `P3` | S | Low | — | CopilotKernel canon validation uses substring matching, not predicates |

> **Highest-urgency combination:** TD-04, TD-05, and TD-09 carry the most correctness and stability risk and are not blocked by anything else. They should run in parallel with TD-01.

---

## ⚡ Quick Wins (< 1 Day Each)

These items have a high signal-to-effort ratio and can be landed without touching risky code paths:

1. **Pin `stb` to a specific `GIT_TAG`** — one line in `CMakeLists.txt`. Eliminates floating dependency risk immediately. (TD-07)
2. **Add `/W4` (MSVC) and `-Wall -Wextra` (GCC/Clang) to `CMakeLists.txt`** — a single `target_compile_options` block. Begins surfacing latent issues without touching logic. (TD-07)
3. **Add `check_save_policy_governance.ps1` and `check_cmake_completeness.ps1` to `run_local_gates.ps1`** — closes the local/CI parity gap with a few lines of script. (TD-07)
4. **Anchor ctest label regex** — change `-L pr` to `-L "^pr$"` everywhere in CI and docs to eliminate the 1-test bleed. (TD-07)
5. **Add a single-sentence doc comment to `EngineAssembly` and `MainAssembly`** declaring them non-canonical until repaired, with a pointer to `EngineShell`. Lowers onboarding risk at zero code cost. (TD-01)
6. **Replace `BattleScene::initBattle()` hardcoded `"img/battlebacks1/Grassland.png"` fallback** with a named `MISSING_BATTLEBACK` diagnostic log when no troop/map configuration provides a battleback. The silent fallback masks content configuration errors. (TD-04)
7. **Add a `// NOT LIVE — simulated only` banner comment to `OpenAIChatService::requestResponse()` and `LlamaLocalService::requestResponse()`** — both functions have commented-out callbacks and do nothing when called. This is a one-line comment that prevents future callers from assuming live behavior. (TD-10)
8. **Rename `CopilotKernel::validateProposal()` internal logic** to make it explicit that the current constraint check is a string-search mock, not predicate evaluation. One comment or renamed internal variable prevents the test from being read as production canon enforcement. (TD-11)

**Status on 2026-04-22:** all eight quick wins above are now landed, although TD-01, TD-04, TD-10, and TD-11 still have broader follow-through open beyond the quick-win slice.

---

## Risk Matrix

```
          │ Low Effort │ High Effort
──────────┼────────────┼────────────
High Risk │  TD-05*    │  TD-04
          │  TD-09**   │  TD-03
──────────┼────────────┼────────────
Low Risk  │  TD-07     │  TD-02
          │  TD-06     │  TD-01
          │  TD-08***  │  TD-10
          │  TD-11     │
```

\* TD-05 is moderate effort but the cache and global issues are the most likely to silently become bugs.  
\*\* TD-09 (GAF scripted conditions) is moderate effort but causes silent ability misbehavior in any content that uses condition strings.  
\*\*\* TD-08 (file size) is low risk *today* but compounds the cost of every other fix.

---

## Phased Remediation Roadmap

### Phase 1 — Immediate (This Sprint)

**Goal:** Eliminate the most misleading surfaces and close the cheapest governance gaps. Nothing here requires a major architecture decision, but some items do touch runtime code.

| Action | Items | Owner |
|---|---|---|
| Declare `EngineShell` the only canonical top-level runtime in docs and a header comment | TD-01 | — |
| Add all eight Quick Wins above | TD-07, TD-01, TD-04, TD-10, TD-11 | — |
| Replace `AssetLoader`'s unbounded static caches with a size-bounded structure or route through existing `AssetCache` | TD-05 | — |
| Add at least one concurrent `ThreadRegistry` registration test | TD-05 | — |
| Replace the `"active_condition_unimplemented"` silent bypass in GAF with a named diagnostic log | TD-09 | — |
| Tighten audit and docs to reflect `synthetic pipeline intact ≠ shipping pipeline proven` language for export/security/visual validation | TD-06 | — |

### Phase 2 — Near-Term (Next 2–4 Weeks)

**Goal:** Close the render allocation loop, finish the most critical ownership contracts, and resolve the GAF scripted condition wiring.

| Action | Items | Owner |
|---|---|---|
| Redesign `RenderLayer` to use value-type or arena-backed frame command storage instead of `shared_ptr<RenderCommand>` | TD-02 | — |
| Update `renderer_backend.h` abstraction to accept frame-owned command view, not heap-backed vector | TD-02 | — |
| Implement bounded real text and rect draw paths in `OpenGLRenderer` (landed) | TD-02 | — |
| Remove or relocate stale `EngineAssembly` / `MainAssembly` / `EditorShell` / `PluginHost` / `ScriptBridge` from production-looking paths | TD-01 | done |
| Burn down the stale-seam `check_cmake_completeness.ps1` orphan whitelist | TD-01 | — |
| Replace raw global `World*` in `plugin_api.cpp` with scoped RAII binding or explicit context passing | TD-05 | — |
| Finish sprite-handle ownership (`Sprite_Character`, `Sprite_Actor` release/reload TODOs) | TD-05 | — |
| Wire `resolveActiveCondition()` and `passiveCondition` to scripting runtime or native evaluator | TD-09 | — |
| Make the TD-10 AI/cloud keep/cut decision and align code layout accordingly | TD-10 | — |
| Make unsupported formula categories and troop-page gaps explicit in migration output (even if not yet fixed) | TD-04 | — |

### Phase 3 — Medium-Term (4–8 Weeks)

**Goal:** Close correctness gaps, reduce structural complexity, and productize or delete simulation facades.

| Action | Items | Owner |
|---|---|---|
| Implement a real expression evaluator for formula strings and wire it into damage resolution | TD-04 | — |
| Decide: live JS/plugin runtime or permanently bounded harness. Align code layout accordingly | TD-03 | — |
| If harness: rename, relocate, and document as a harness everywhere; delete dead `ScriptBridge`/`PluginHost` paths | TD-03 | — |
| If live: complete QuickJS integration and retire fixture-backed execution | TD-03 | — |
| Wire GAF scripted conditions to the chosen runtime (TD-03 or native evaluator) | TD-09 | — |
| Implement a real `ICloudService` provider or remove the simulation layer from production-facing headers | TD-10 | — |
| Replace `CopilotKernel` string-match constraints with a real predicate/rule system | TD-11 | — |
| Expand beyond the first real export smoke path and first renderer-backed visual capture path | TD-06 | — |
| Implement real `ResourceProtector::compress()` or remove it from the API surface | TD-06 | — |
| Begin splitting oversized compat manager files by responsibility boundary (not just size) | TD-08 | — |
| Schedule a dedicated ASan/LSan and Valgrind pass after Phase 2 lands | — | — |

---

## Detailed Findings

### TD-01 — `P1` Stale top-level assembly/editor path is still present and not build-real

**Effort:** M | **Risk:** Medium | **Blocks:** TD-03, TD-07

The repo still exposes a public-looking engine/editor assembly layer that is neither current nor part of the real runtime path.

**Evidence:**

- `tools/incubator/runtime/engine_assembly.h:28-37` calls `ThreadRoles::instance()`, `ScriptBridge::instance().startup()`, `PluginHost::instance().discoverPlugins("plugins/")`, and `EditorShell::instance().startup()`.
- `engine/core/editor/editor_shell.h:32-50` does not provide `instance()`, `startup()`, `shutdown()`, `update()`, or `render()`. It only manages panel storage and `renderUI()`.
- `engine/core/threading/thread_roles.h:35-40` defines `ThreadRegistry`, not `ThreadRoles`; the `.cpp` implements only `RegisterCurrentThread`, `IsCurrentThread`, `CurrentScriptAccess`, and `CanRunScriptDirectly`.
- `tools/incubator/runtime/main_assembly.h:22-41` still uses a synthetic loop with a 10-frame cutoff and placeholder termination logic.
- `tools/ci/check_cmake_completeness.ps1` no longer needs orphan exemptions for the stale plugin/script harness seam after `plugin_host.*`, `script_bridge.*`, and `scripting_console.*` were relocated into `tools/incubator/editor/`; `doc_generator.*` and `battle_tactics_window.*` were already relocated into `tools/docs/incubator/` and `tools/incubator/ui/` so they no longer present as production engine/editor/UI orphans. The only remaining whitelist entry is the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.
- `CMakeLists.txt:426-663` registers tools, tests, and `urpg_presentation_release_validation`, but no target that compiles the incubating `EngineAssembly` or `MainAssembly` seam.

**Why it matters:**

- New contributors can still choose the wrong top-level seam.
- CI green does not mean all public-looking runtime or editor entry points are alive.
- The completeness gate is now much more truthful about stale runtime/editor seams, but it still does not prove that the remaining non-canonical top-level assembly headers are live.

**Recommended direction:**

- Declare `EngineShell` the only canonical top-level runtime surface.
- Remove or relocate the remaining stale assembly code from production-looking paths.
- Keep the completeness whitelist limited to genuine standalone tools, not stale runtime/editor seams.
- Gate: `check_cmake_completeness.ps1` passes with no production-looking seam exemptions.

**Definition of Done:**

- [x] `EngineAssembly` and `MainAssembly` are relocated into an explicit incubator path under `tools/` with a pointer to `EngineShell`
- [x] Production-looking editor/plugin assembly surfaces are either compiled as real targets or moved out of canonical runtime/editor paths
- [x] `check_cmake_completeness.ps1` passes with no stale production-seam whitelist entries
- [ ] All CI and local gates reference `EngineShell` as the single top-level entry point in comments

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

### TD-04 — `P2` Battle and formula parity still bounded by partial interpreters and fallback logic

**Effort:** L | **Risk:** High | **Depends on:** nothing for baseline correctness; TD-03 only matters for full live-script parity

The deterministic native and compat battle slice has improved, but full RPG Maker-style formula and event parity is still not there.

**Evidence:**

- `runtimes/compat_js/battle_manager.h:160-167,297-300,326-328,405` marks transition/audio routing, formula parsing, troop-page execution, and playback as `PARTIAL`.
- `runtimes/compat_js/battle_manager.cpp:355-371` retains battle transition/background/audio state for readback but does not route it to live scene/audio backends.
- `runtimes/compat_js/battle_manager.cpp:424-426` limits troop page execution to a bounded command subset.
- `engine/core/scene/combat_formula.h:38-49` performs token replacement then falls back to `evaluateDamage()` instead of evaluating the formula string.
- `engine/core/scene/battle_scene.cpp:789-796` routes the non-`BattleRuleResolver` branch through `CombatFormula::evaluateDamage()` rather than a real parsed formula.

**Why it matters:**

- Content with nontrivial formula strings or broader troop-page scripting silently collapses to simplified behavior.
- Current tests prove deterministic closure for a bounded slice, not full MZ combat semantics.
- **This is a correctness gap, not just a performance or polish issue.**

**Recommended direction:**

First, make the decision explicit: are formula strings in scope for native battle parity or not?

**If yes:**
- Implement a real expression evaluator. A recursive descent parser for the subset of operators MZ formulas use (`+`, `-`, `*`, `/`, `b.atk`, `a.mat`, etc.) is sufficient and self-contained.
- Wire it into `CombatFormula` as a replacement for the token-replacement fallback.
- Add test cases for at least five representative formula strings, including multi-operand and stat-reference cases.

**If no:**
- Replace the silent fallback in `CombatFormula` with an explicit `UnsupportedFormulaError` or a warning log that names the unrecognized formula string.
- Surface this gap prominently in migration and audit output so content authors are not surprised.
- Do not let unsupported cases degrade silently into `evaluateDamage()`.

**Definition of Done:**

- [ ] A formula strategy decision is recorded in the contributor guide or audit document
- [ ] Either a real expression evaluator is wired and covered by tests, OR every unsupported formula string produces a named, logged, non-silent diagnostic
- [ ] No `PARTIAL` marker in `battle_manager.h` lacks an associated test verifying the partial boundary

**Progress on 2026-04-22:**

- `BattleScene` no longer silently falls back to `Grassland`; it emits `MISSING_BATTLEBACK`.
- `CombatFormula` now supports a bounded arithmetic/stat subset and returns named fallback reasons for unsupported or malformed expressions.
- `BattleManager::applySkill()` / `applyItem()` now use that bounded subset when present and expose unsupported formulas in method status text instead of silently degrading.
- Compat troop-page handling now supports variable conditional branches against constants and live variable operands.
- Migration now preserves unsupported action `effects[]` as `_compat_effect_fallbacks` rather than hiding them behind a generic warning.
- **Remaining gap:** full formula/event parity is still not there, and transition/audio routing remains partial.

---

### TD-05 — `P2` Memory-growth and lifetime ownership debt in caches, globals, and sprite handles

**Effort:** M | **Risk:** High | **Depends on:** nothing

No fresh must-fix leak at the level of the earlier AudioManager SE issue, but several ownership paths are weaker than they should be.

**Evidence:**

- `engine/core/render/asset_loader.h:24-29` exposes process-global texture and missing-texture caches with no size bound, no clear API, and no synchronization.
- `engine/core/render/asset_loader.cpp:9-41` uses those static caches directly; every new successful or failed path is retained for the life of the process.
- `engine/core/scene/map_scene.cpp:275` and `engine/core/scene/battle_scene.cpp:1017` load runtime textures through `AssetLoader`, making the unbounded cache reachable from scene code.
- `engine/core/editor/plugin_api.cpp:15,60-66,82-100` stores the active ECS world in a raw process-global `World*`.
- `engine/core/threading/thread_roles.h:43` and `thread_roles.cpp:5-27` use an unsynchronized `unordered_map` as thread-role state.
- `tests/unit/test_thread_roles.cpp:12-18` only exercises same-thread registration and lookup; there is no concurrent test coverage.
- `runtimes/compat_js/window_compat.h:534,608` give `Sprite_Character` and `Sprite_Actor` private `BitmapHandle bitmap_` members, but `window_compat.cpp:2303-2322` and `2374-2380` leave release, reload, source-rect, and animation ownership as TODOs.

**Important nuance:**

- `Window_Base` contents-handle cleanup itself looks correct: `window_compat.cpp:1007-1012` erases entries from `contentsBitmaps_`.
- The SE leak from the previous audit is closed (see Resolved Debt section).

**Why it matters:**

- The asset cache issue is a true memory-growth risk in long-running sessions.
- The unsynchronized thread-role map is a latent data race waiting for concurrent use to materialize.
- Sprite handle TODOs mean bitmap-backed sprite work lacks a finished lifetime contract.

**Recommended direction:**

- Replace `AssetLoader`'s global caches with bounded, clearable, thread-aware storage. Route through the existing LRU-style `AssetCache` if it covers the access pattern, or add an explicit `evict()`/`clear()` API.
- Replace the raw global plugin `World*` with scoped RAII binding or explicit context passing. A thread-local or scope-guard pattern prevents both use-after-free and cross-context contamination.
- Add a mutex or `shared_mutex` around the `ThreadRegistry` map, or switch to a lock-free structure. Add at least one concurrent registration test before treating the registry as production concurrency infrastructure.
- Complete sprite-handle ownership (release, reload, source-rect, animation) before adding real bitmap loading to compat sprites.

**Definition of Done:**

- [ ] `AssetLoader` caches have a documented size bound and an explicit `clear()` or eviction path
- [ ] No raw process-global `World*` remains in plugin API code
- [ ] `ThreadRegistry` map access is synchronized
- [ ] At least one multi-threaded `ThreadRegistry` test covers concurrent registration and lookup
- [ ] `Sprite_Character` and `Sprite_Actor` bitmap ownership TODOs are resolved before real bitmap loading is wired

**Progress on 2026-04-22:**

- `AssetLoader` now uses bounded caches for successful and missing texture lookups, exposes `clearCaches()` as an explicit reset boundary for tests/reset flows, and tests pin both eviction/re-warning and manual reset behavior.
- `ThreadRegistry` map access is synchronized with `std::shared_mutex`, and concurrent registration/query coverage exists.
- The raw global plugin `World*` binding has been replaced with a scoped thread-local binding stack.
- `Sprite_Character` and `Sprite_Actor` bitmap handles now reload and release deterministically across identity changes and destruction.
- **Result:** the highest-risk TD-05 ownership items from this audit are now substantially reduced.

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

### TD-07 — `P3` Build reproducibility and CI fidelity below baseline

**Effort:** S | **Risk:** Low | **Depends on:** nothing

URPG's governance stack is healthier than its C++ quality enforcement and gate exactness.

**Evidence:**

- `CMakeLists.txt:10-12` sets only `/FS` and `/EHsc` for MSVC; no repo-wide warning policy like `/W4`, `-Wall`, or `-Wextra` is present.
- `CMakeLists.txt:386-393` fetches `stb` from `GIT_TAG master`.
- `ctest -N -L pr` enumerates 892 tests; `ctest -N -L "^pr$"` enumerates 891. The unanchored label regex pulls in `urpg_presentation_release_validation`.
- `.github/workflows/ci-gates.yml:55-72` runs `check_save_policy_governance.ps1`, `check_breaking_changes.ps1`, and `check_cmake_completeness.ps1` in Gate 1.
- `tools/ci/run_local_gates.ps1:27-109` does not mirror that policy set; it omits at least `check_save_policy_governance.ps1` and `check_cmake_completeness.ps1`.
- `tools/ci/check_cmake_completeness.ps1` now burns down the stale plugin/script seam exemptions; the remaining whitelist is limited to the standalone `engine/core/presentation/profile_arena.cpp` profiling tool.

**Why it matters:**

- Local `All gates passed` does not mean `same policy surface as Gate 1`.
- Floating dependencies and weak warning posture slow long-term hardening in a large C++ tree.

**Recommended direction:**

- Pin `stb` to a specific commit SHA or tagged release.
- Add `/W4` (MSVC) and `-Wall -Wextra` (GCC/Clang) as repo-wide compile options; suppress only with documented, targeted pragmas.
- Anchor all ctest label regexes to `"^pr$"`, `"^nightly$"` etc. in both CI and local scripts.
- Make `run_local_gates.ps1` an exact mirror of Gate 1, or add an explicit comment enumerating what differs and why.
- Consider adding a clang-tidy or cppcheck preset as an optional opt-in gate.

**Definition of Done:**

- [ ] `stb` pinned to a specific tag or SHA
- [ ] Repo-wide warning flags present in `CMakeLists.txt`
- [ ] `run_local_gates.ps1` runs the same checks as Gate 1 (or documents the delta)
- [ ] All ctest label patterns in CI and local scripts are anchored
- [ ] `check_cmake_completeness.ps1` TODO is resolved or tracked as a separate issue

**Progress on 2026-04-22:**

- `stb` is pinned.
- Repo-wide warning flags are enabled via a shared compile-options target.
- CI and local gate label filters are anchored.
- `run_local_gates.ps1` now includes the key Gate 1 governance checks that were missing in the audit snapshot.
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
| `runtimes/compat_js/plugin_manager.cpp` | 2,350 |
| `runtimes/compat_js/window_compat.cpp` | 2,275 |
| `editor/diagnostics/diagnostics_workspace.cpp` | 2,198 |
| `tools/audit/urpg_project_audit.cpp` | 2,183 |
| `runtimes/compat_js/battle_manager.cpp` | 1,806 |
| `runtimes/compat_js/data_manager.cpp` | 1,718 |
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

### TD-09 — `P2` GAF scripted conditions are declared but never evaluated

**Effort:** M | **Risk:** High | **Depends on:** TD-03 (for full live-script wiring), but the diagnostic path is independent

The Gameplay Ability Framework has a first-class data model for scripted activation and passive cancellation conditions, but neither condition type is evaluated at runtime. Every ability with a non-empty `activeCondition` or `passiveCondition` string silently bypasses those gates.

**Evidence:**

- `engine/core/ability/gameplay_ability.h:34-35` declares `activeCondition` ("Must evaluate true to activate") and `passiveCondition` ("While active, if false, ability cancels") as first-class fields on `ActivationInfo`.
- `engine/core/ability/gameplay_ability.cpp:50-54`: `evaluateActivation()` checks whether `resolveActiveCondition()` returns a non-empty string, then **unconditionally** sets `result.allowed = false` with reason `"active_condition_unimplemented"` and returns — no scripting evaluation occurs.
- `passiveCondition` is never read anywhere in the codebase. `update()` does not check it; no cancellation path interrogates it.
- `AbilitySystemComponent::canApplyEffect()` returns `true` unconditionally with the comment: "placeholder for future logic where Effects have their own Tag requirements."
- No test in `tests/unit/` or `tests/integration/` exercises an ability with a non-empty `activeCondition` or `passiveCondition`.

**Why it matters:**

- Any ability authored with a scripted condition string is permanently blocked (returns `"active_condition_unimplemented"`), or permanently ignores passive cancellation. There is no middle ground.
- Content authors cannot tell from any runtime signal whether a condition they wrote is being evaluated or bypassed. The failure mode for an active condition is ability-blocked; the failure mode for a passive condition is ability-never-cancels — both are silent.
- This is a correctness gap, not just a missing feature. The data model is live and callers are writing conditions that are not enforced.

**Recommended direction:**

First, decide scope: are scripted conditions evaluated against the QuickJS runtime (TD-03 dependency), against the native expression evaluator (TD-04 dependency), or against a bounded DSL?

**Regardless of scripting strategy:**
- Replace the `"active_condition_unimplemented"` silent bypass with an explicit log-diagnostic and a clearly named result code, so content authors can see the gap at runtime.
- Add at least one test that registers an ability with a non-empty `activeCondition` and asserts the expected behavior (blocked with diagnostic, or evaluated to true/false).

**Once a scripting strategy is chosen:**
- Wire `resolveActiveCondition()` to the chosen evaluator.
- Wire `passiveCondition` to a cancellation check in `update()`.
- Implement `canApplyEffect()` tag gate once effects have Tag requirements.

**Definition of Done:**

- [ ] Abilities with a non-empty `activeCondition` produce a named, logged diagnostic — they do not silently block or silently pass
- [x] `passiveCondition` is either evaluated in `update()` or its absence is documented as out-of-scope with a matching test that asserts the field is not read
- [x] `canApplyEffect()` either checks effect Tag requirements or is documented as always-true with a test pinning that behavior
- [ ] At least one ability activation test covers a non-empty `activeCondition` value

**Progress on 2026-04-22:**

- Non-empty `activeCondition` values now produce explicit `active_condition_unsupported` diagnostics with detail strings that surface in execution history and replay diagnostics.
- Focused activation tests now cover the non-empty `activeCondition` path, pin that `passiveCondition` is ignored by the current runtime, and prove that passive strings do not block execution.
- `canApplyEffect()` is now explicitly documented and test-pinned as an always-true admission gate; modifier `requiredTag` checks still apply later during attribute resolution.

---

### TD-10 — `P2` AI/cloud connectivity layer is entirely simulated with no live transport

**Effort:** M | **Risk:** Medium | **Depends on:** nothing (cut decision) or TD-03-adjacent (if wiring to live scripting)

The engine declares a full AI chat and cloud-sync integration surface — OpenAI, Llama.cpp local inference, cross-device cloud sync, knowledge update hooks — but every call site either does nothing or routes through an in-memory stub with no real transport.

**Evidence:**

- `engine/core/ai/ai_connectivity.h`: `OpenAIChatService::requestResponse()` constructs a JSON body, documents the required HTTP steps, and then ends with a commented-out `callback(...)` call. **The function does nothing when called.** `LlamaLocalService::requestResponse()` is identical — commented-out callback, no inference.
- `engine/core/social/cloud_service.h`: `LocalInMemoryCloudService::syncToCloud()` stores data in an in-process `std::map`. `LocalInMemoryCloudService::initialize()` only allows `CloudProvider::LocalSimulated`; live-looking providers fail with a `NOT LIVE` result instead of pretending to connect. No network operation occurs.
- `engine/core/message/ai_sync_coordinator.cpp:61`: `checkForRemoteKnowledgeUpdates()` returns `true` unconditionally. The comment labels it "Phase 4 development: Placeholder."
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
- [ ] `checkForRemoteKnowledgeUpdates()` either performs a real check or is deleted/replaced with `return false` and a clear comment
- [ ] AI/chat transport and cloud-provider behavior have direct tests, or the related production-facing headers are moved to a clearly labeled `experiments/` or `future/` directory

**Progress on 2026-04-22:**

- The AI/chat request paths are now explicitly marked `NOT LIVE`.
- `AISyncCoordinator::checkForRemoteKnowledgeUpdates()` now returns `false` instead of implying successful remote polling.
- Cloud behavior is still local/in-memory only, and no live transport was added.
- The in-tree double now exposes its local-only scope at the type level (`LocalInMemoryCloudService`) and has direct unit coverage for local-only initialization and coordinator sync/restore behavior.
- **Remaining gap:** the keep/cut strategy decision for this entire layer is still open.

---

### TD-11 — `P3` CopilotKernel canon validation uses substring matching, not predicate-based constraint enforcement

**Effort:** S | **Risk:** Low | **Depends on:** nothing

The `CopilotKernel` is presented as a canon constraint enforcement layer for the Producer Copilot system. Its constraint objects carry `id`, `description`, `severity`, and `isHardConstraint` fields — but the actual validation logic ignores all of them and instead checks whether the proposal description string contains the word `"violate"`.

**Evidence:**

- `engine/core/copilot/copilot_kernel.h:47`: `validateProposal()` checks `proposal.description.find("TODO") != std::string::npos` as a baseline failure case.
- `engine/core/copilot/copilot_kernel.h:52-56`: Hard constraint enforcement reduces to `proposal.description.find("violate") != std::string::npos`. The registered `CanonConstraint` objects are never interrogated.
- `tests/test_copilot_kernel.cpp`: The three test cases validate empty descriptions, descriptions containing "violate," and a description without "violate." All pass against the mock logic. The `CanonConstraint` added in the test (`"Map cannot have empty cells"`) is never evaluated by the validator.
- `CanonConstraint` has no predicate field — there is no way to attach a real check function to a constraint, only description strings.

**Why it matters:**

- This is a low-risk finding today because the Copilot system is not yet in a live editorial path. But the test suite passes and appears to validate real constraint enforcement. A future developer reading the tests will reasonably believe constraints are being checked, and build editor workflows on that assumption.
- The current design cannot be extended to real constraint enforcement without replacing the core of `validateProposal()`.

**Recommended direction:**

- Add a `predicate` field to `CanonConstraint` (e.g., `std::function<bool(const EditProposal&)>`) so constraints can carry real check logic.
- Replace the `find("violate")` dispatch in `validateProposal()` with iteration over registered constraints using their predicates.
- Update the test to register a constraint with a real predicate and verify it is called.
- No API surface needs to change; this is an implementation change inside the kernel.

**Definition of Done:**

- [ ] `CanonConstraint` has a callable predicate field
- [ ] `validateProposal()` iterates registered constraint predicates, not description substrings
- [ ] At least one test registers a constraint with a real predicate and verifies it fires on a proposal that violates it

**Progress on 2026-04-22:**

- This finding's core implementation gap is now closed: `CopilotKernel` validates proposals against registered predicate-based constraints, and tests exercise real constraint behavior instead of substring mocks.
- Follow-up work, if any, is now about expanding rule coverage rather than replacing the enforcement model.

---

## Cross-Cutting Themes

This pass reduces to five big themes:

1. **Build-truth drift** — The living runtime path and the public-looking top-level path are still different things. (TD-01, TD-07)
2. **Hot-loop overhead** — The render and UI path is the strongest current optimization target. (TD-02)
3. **Harness-versus-runtime ambiguity** — The compat layer is valuable, but still bounded by harness semantics. (TD-03, TD-04)
4. **Ownership and lifetime debt** — The riskiest memory issues are less about a single obvious leak and more about unbounded caches, globals, and unfinished lifetime contracts. (TD-05)
5. **Silent pass-through in feature-gated systems** — Several subsystems (GAF scripted conditions, AI connectivity, CopilotKernel constraints) have a declared contract that callers expect to be enforced, but the implementation silently skips or bypasses it. This is a correctness trap: code compiles, some surrounding tests pass, and behavior appears plausible until the feature is actually exercised. (TD-09, TD-10, TD-11)

---

## Recommended Action Order

| Step | Action | Phase | TD Items |
|---|---|---|---|
| 1 | Apply all Quick Wins (pin stb, add warnings, sync local gates, anchor label regex, doc comments on stale assembly, battleback diagnostic, AI connectivity banners, CopilotKernel comment) | 1 | TD-07, TD-01, TD-04, TD-10, TD-11 |
| 2 | Bound `AssetLoader` caches; synchronize `ThreadRegistry`; add concurrent registry test | 1 | TD-05 |
| 3 | Declare `EngineShell` canonical; mark or remove stale assembly/editor/plugin paths | 1 | TD-01 |
| 4 | Wire GAF `resolveActiveCondition()` and `passiveCondition` to the scripting runtime or gate with an explicit `UnsupportedConditionError` | 1–2 | TD-09 |
| 5 | Redesign `RenderLayer` for arena/value command storage; update `renderer_backend.h` abstraction | 2 | TD-02 |
| 6 | Implement real text, rect, and UI draw paths in `OpenGLRenderer` | 2 | TD-02 |
| 7 | Replace raw global `World*`; finish sprite-handle ownership | 2 | TD-05 |
| 8 | Make the TD-03 scripting strategy decision and begin aligning code layout | 2–3 | TD-03 |
| 9 | Decide formula scope; implement real evaluator or make fallback explicit and named | 2–3 | TD-04 |
| 10 | Make an explicit keep/cut decision for the AI/cloud layer; either wire real HTTP transport or delete the simulation facades | 2–3 | TD-10 |
| 11 | Replace `CopilotKernel` string-match constraints with a real predicate/rule system | 3 | TD-11 |
| 12 | Tighten export/security/visual-validation claims; add one real lane for each | 3 | TD-06 |
| 13 | Schedule ASan/LSan and Valgrind pass after Phase 2 render changes land | 3 | — |
| 14 | Begin splitting `DiagnosticsWorkspace` and `urpg_project_audit` | 3 | TD-08 |

**Progress note on 2026-04-22:**

- Steps 1, 2, and 11 have landed in substance.
- Step 4 is partially landed through explicit unsupported-condition diagnostics and tests, but not full evaluation.
- Steps 5 and 6 are now landed in bounded form through value-owned `FrameRenderCommand` storage, frame-buffer backend consumption, real immediate-mode text/rect submission, focused pre-init/no-context test coverage, and one bounded nightly renderer-backed visual capture lane; the remaining TD-02 follow-through is allocator proof plus richer textured/UI coverage.
- Step 9 is partially landed through bounded evaluator support plus explicit fallback surfacing, but not full parity.
- Step 10 is partially landed through truthfulness fixes, but not the keep/cut decision.

---

## Conclusion

URPG is not primarily suffering from missing documentation or missing tests. It is suffering from a smaller, harder class of debt: **build-truth drift, harness-versus-runtime ambiguity, hot-loop allocation overhead, silent feature bypasses, and a set of intentionally partial surfaces that still live next to production-looking APIs.**

This pass added three new finding categories that were not visible in the prior pass:

- **GAF scripted conditions (TD-09):** Abilities with scripted activation or cancellation conditions silently bypass those conditions at runtime. This is not a documentation gap — the data model is there and callers expect the condition to be enforced.
- **AI/cloud connectivity (TD-10):** `OpenAIChatService` and `LlamaLocalService` call back with nothing, while `AISyncCoordinator` is only meaningfully real against the in-memory stub path. The simulation is not labeled clearly enough to prevent future integration work from being misdirected.
- **CopilotKernel constraints (TD-11):** The canon constraint system checks if a proposal description contains the word `"violate"`. Tests pass. Nothing is actually constrained.

All three shared the same failure mode at audit time: a feature contract was declared and tested, but the underlying enforcement was a pass-through. During the 2026-04-22 execution session, TD-11 was closed in substance, TD-09 was improved to emit explicit unsupported-condition diagnostics, and TD-10 was made more truthful by labeling the seams as not live and removing the false-positive remote-update return.

That is fixable. The repo already has the governance discipline to do it. The Quick Wins above can move immediately with near-zero risk. The Phase 1 and Phase 2 items address the most misleading surfaces and the most impactful structural changes without requiring a strategy decision on the JS runtime.

The single most leveraged action is making the code paths themselves as truthful and production-shaped as the surrounding documentation already is.

> **Next audit trigger:** Re-run this audit after Phase 2 completes. Add an ASan/Valgrind pass to the next cycle — the current finding list is based entirely on code inspection and may be undercounting live memory issues.
