# Phase 2 Runtime Closure Design

**Date:** 2026-04-19

**Goal**

Close Phase 2 of the technical debt remediation plan by finishing the remaining high-value runtime-closure work in the compat layer, verifying it with focused tests, and reconciling stale labels and documentation so Phase 2 status reflects the actual codebase.

## Scope

This design covers the four approved Phase 2 lanes:

1. Battle correctness and reward application
2. Window and data runtime closure
3. Audio semantics reconciliation
4. Final Phase 2 reconciliation across tests, status labels, and docs

This design does not introduce a live QuickJS runtime, replace the deterministic compat harness with a production backend, or expand scope into Phase 3 diagnostics productization or Phase 5 hardening.

## Current Repo Reality

The remediation document still describes some older Phase 2 gaps that no longer match the repository:

- QuickJS is already explicitly scoped as a fixture-backed compat harness rather than a live runtime.
- Menu serialization already has native round-trip coverage and should be treated as verification/alignment work, not a rewrite.
- Data loading is materially more capable than older header comments suggest.
- Audio already has deterministic harness-side SE cleanup and live binding coverage, but the remaining semantics still need a reconciliation pass.

The remaining Phase 2 work is therefore not a single rewrite. It is a targeted closure pass across the runtime surfaces that still have behavior gaps, stale labels, or weak tests.

## Approach

Phase 2 should be completed sequentially in four lanes. Each lane follows the same pattern:

1. Add or tighten tests around the audited claim.
2. Implement the minimum runtime behavior needed to satisfy the claim.
3. Reconcile method-status comments and exported labels with the code that now exists.
4. Re-run the focused test lane before moving to the next lane.

This keeps the work bounded and avoids reopening already-closed surfaces.

## Lane 1: Battle Correctness and Reward Application

### Problem

The battle lane still contains the clearest unresolved runtime debt:

- reward application methods remain documented as stubbed
- tests still contain at least one "no crash" style assertion where stateful behavior should be proven
- event-condition and reward semantics need stronger proof against live compat state

### Files in scope

- `runtimes/compat_js/battle_manager.h`
- `runtimes/compat_js/battle_manager.cpp`
- `tests/unit/test_battlemgr.cpp`

### Design

Battle closure will focus on deterministic live-state mutation rather than broad battle-system expansion.

The implementation should:

- make `applyExp()` mutate party or actor progression state in a deterministic, testable way
- make `applyGold()` mutate party gold in live compat state
- make `applyDrops()` route deterministic drops into party inventory
- tighten `checkSwitchCondition()` and related battle-event checks so they read real compat state where that state already exists
- preserve honest `PARTIAL` labels where formula interpretation, full interpreter coverage, or full MZ parity still do not exist

### Acceptance criteria

- reward application changes observable compat state
- battle reward tests assert concrete EXP, level, gold, and inventory outcomes
- battle-event condition tests assert live switch and cadence behavior where supported
- `STUB` labels in the reward lane are removed if real behavior now exists

## Lane 2: Window and Data Runtime Closure

### Problem

The window/data lane has a mix of real progress and stale claims:

- some `Window_Base` comments still describe "tracks intent only" behavior that may no longer match the runtime
- `contents()` and draw-related semantics need clearer closure or tighter labels
- `DataManager` header comments still describe loader-empty behavior that no longer matches the seeded loader path

### Files in scope

- `runtimes/compat_js/window_compat.h`
- `runtimes/compat_js/window_compat.cpp`
- `tests/unit/test_window_compat.cpp`
- `runtimes/compat_js/data_manager.h`
- `runtimes/compat_js/data_manager.cpp`
- `tests/unit/test_data_manager.cpp`

### Design

This lane should not attempt a full pixel-buffer renderer or full MZ asset pipeline. It should close the highest-value deterministic runtime gaps and make the remaining limits explicit.

The implementation should:

- verify whether `contents()` and contents lifecycle behavior are truly deterministic runtime features or still mere handle allocation
- improve draw accumulation semantics where current behavior is weaker than the audit exit criteria require
- upgrade or narrow `drawIcon`, `drawGauge`, `drawCharacter`, and related comments based on the code that actually exists after recent work
- align `DataManager` comments and tests with the current seeded and JSON-backed loading path

### Acceptance criteria

- window tests prove the current deterministic contents/draw contract rather than relying on label-only assertions
- data-manager tests prove non-empty loader behavior where the code now supports it
- stale comments about empty containers or pure intent-tracking are removed or narrowed
- any remaining `PARTIAL` label includes a specific real limitation

## Lane 3: Audio Semantics Reconciliation

### Problem

Audio is further along than the original audit description, but the lane still needs a close-out pass so the remaining limitations are explicit and tested against current behavior.

### Files in scope

- `runtimes/compat_js/audio_manager.h`
- `runtimes/compat_js/audio_manager.cpp`
- `tests/unit/test_audio_manager.cpp`

### Design

This lane is primarily verification and truthfulness work. It should not chase full live-backend parity. It should ensure the deterministic harness semantics now implemented are the semantics documented and tested.

The implementation should:

- verify the current behavior for playback position, duck/unduck, mix scaling, and QuickJS bindings
- add focused tests only where the current semantics are still under-specified
- keep labels at `PARTIAL` wherever the behavior still does not drive a real backend mixer

### Acceptance criteria

- audio tests cover the actual supported deterministic semantics
- comments and status text no longer imply older missing behavior that has already been implemented
- no audio method is upgraded beyond what the harness truly supports

## Lane 4: Final Phase 2 Reconciliation

### Problem

Even after runtime work lands, Phase 2 is not complete until tests, comments, and canonical docs agree on what was actually closed.

### Files in scope

- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`

Additional files may be touched if status labels or comments in runtime headers remain inconsistent after the code/test pass.

### Design

This lane is the truthfulness close-out pass.

The implementation should:

- run the focused battle, window, data, and audio test lanes
- update remediation/status docs to reflect what actually closed in this pass
- record the residual limits that still keep any surface at `PARTIAL`
- avoid claiming total parity where the runtime remains deterministic compat scaffolding

### Acceptance criteria

- the remediation hub no longer lists already-closed gaps as open
- worklog entries match the final implemented behavior
- Phase 2 status can be read from docs without contradicting the code or tests

## Testing Strategy

Testing should stay lane-local and deterministic.

- Battle: extend `tests/unit/test_battlemgr.cpp` with reward-application and event-condition assertions.
- Window: extend `tests/unit/test_window_compat.cpp` for contents lifecycle and draw accumulation behavior.
- Data: extend `tests/unit/test_data_manager.cpp` for current loader and accessor behavior.
- Audio: extend `tests/unit/test_audio_manager.cpp` only where semantics still lack direct proof.

Each lane should run its focused test file first, then the broader related tag or binary lane if the local change warrants it.

## Risks and Controls

### Risk: stale audit text drives unnecessary rewrites

Control: verify code and tests before changing behavior; prefer comment/status reconciliation where runtime work is already landed.

### Risk: over-claiming closure after deterministic harness improvements

Control: retain `PARTIAL` labels whenever the surface still lacks real backend parity or full MZ interpreter semantics.

### Risk: battle reward implementation mutates the wrong global state shape

Control: drive the implementation from existing `DataManager` and compat-state access patterns already used in tests.

### Risk: window closure drifts into renderer refactoring

Control: restrict work to deterministic compat runtime semantics and tested exported behavior, not renderer architecture redesign.

## Out of Scope

- Real QuickJS embedding
- Full MZ plugin runtime parity
- Full battle interpreter coverage
- Full pixel-backed window rendering
- Phase 3 diagnostics workspace productization
- Phase 5 threading, ownership, or performance hardening

## Completion Definition

Phase 2 is complete when:

- the remaining runtime-closure gaps in battle, window/data, and audio have either been implemented or precisely bounded
- the relevant focused tests prove live deterministic behavior rather than only non-crash behavior
- runtime comments, compat labels, and canonical docs agree on what the engine actually supports

