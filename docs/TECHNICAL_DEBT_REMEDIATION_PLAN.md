# Technical Debt Remediation Plan

> Document status: third full rewrite based on a post-regression-fix audit performed on 2026-04-18.
> This supersedes the earlier 2026-04-18 pass that still treated the PR and weekly lanes as red.

---

## Purpose

This document is the current technical debt source of truth for URPG. Its job is to:

- identify debt that is still active in the tree today,
- retire findings that no longer match the live repository,
- separate honest accepted stub scope from misleading completion claims,
- prioritize debt that blocks trustworthy validation and planning,
- define the order required to get the repository back to a truthful engineering baseline.

This is not a feature roadmap. It is a truth-and-closure document.

---

## Audit Method

Audit date: 2026-04-18

This pass combined:

- source inspection of current build files, docs, tests, and high-risk subsystems,
- direct comparison against the previous remediation plan written earlier the same day,
- fresh local configure/build/test verification on the repaired Ninja lane,
- targeted inspection of diagnostics, compat battle flow, and AI/status documentation.

Primary files reviewed:

- [CMakeLists.txt](../CMakeLists.txt)
- [CMakePresets.json](../CMakePresets.json)
- [README.md](../README.md)
- [docs/PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md)
- [docs/AI_COPILOT_GUIDE.md](./AI_COPILOT_GUIDE.md)
- [editor/diagnostics/diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h)
- [editor/diagnostics/diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp)
- [editor/diagnostics/event_authority_panel.cpp](../editor/diagnostics/event_authority_panel.cpp)
- [editor/audio/audio_inspector_model.h](../editor/audio/audio_inspector_model.h)
- [editor/audio/audio_inspector_panel.h](../editor/audio/audio_inspector_panel.h)
- [editor/diagnostics/migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h)
- [editor/diagnostics/migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h)
- [engine/core/ai/ai_connectivity.h](../engine/core/ai/ai_connectivity.h)
- [runtimes/compat_js/battle_manager.cpp](../runtimes/compat_js/battle_manager.cpp)
- [tests/unit/test_battlemgr.cpp](../tests/unit/test_battlemgr.cpp)
- [tests/unit/test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp)
- [tests/compat/test_compat_plugin_fixtures.cpp](../tests/compat/test_compat_plugin_fixtures.cpp)

Fresh verification commands run during this audit:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug -j 4
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure
```

Observed outcomes:

- `dev-ninja-debug` configure succeeded.
- `dev-debug` build succeeded.
- PR lane result: `458/458` passed.
- Weekly lane result: `43/43` passed.
- `README.md` still used a nonexistent `dev-windows-debug` preset before this remediation pass.
- `docs/PROGRAM_COMPLETION_STATUS.md` and `docs/DEVELOPMENT_KICKOFF.md` still published stale `289/289` and `42/42` snapshots before this remediation pass.
- This pass did not re-run `dev-vs2022`, so the validated local baseline for this document is the Ninja Debug lane only.

---

## Priority Legend

| Level | Meaning |
| --- | --- |
| P0 | The documented validation baseline is not currently green or truthful enough to trust. |
| P1 | A subsystem has a live regression or materially overstates its delivered surface. |
| P2 | Important structural debt that does not block all work, but keeps the repo brittle or misleading. |
| P3 | Lower-priority hardening or intentionally accepted stub scope. |

---

## Current State Summary

The debt picture changed materially again after the regression fixes.

What is no longer the main problem:

- The immediate MinGW/MSVC header collision that broke the Ninja lane is no longer the top blocker.
- There is now at least one reproducible local Windows configure/build path: `dev-ninja-debug` plus `dev-debug`.
- The previously failing PR and weekly tests are now fixed and both documented lanes are green on the verified Ninja path.

What is now the main problem:

1. Diagnostics workspace inventory/render/export drift is still unresolved.
2. AI/program-status docs still overclaim provider maturity and validation health.
3. Build graph hygiene and documentation drift still require cleanup.
4. Accepted stub scope still needs to stay explicit instead of drifting back into implied completion.

In short: the top debt is no longer "Windows Ninja fails immediately" or "the PR lane is red." It is now "the verified lane is green, but several editor/doc/build surfaces still misrepresent the real product state."

---

## Active Findings

### P1-01 - Diagnostics Workspace Still Overstates Its Real Surface Area

### P1-01 - Diagnostics Workspace Still Overstates Its Real Surface Area

**Impact**

- The diagnostics workspace still cannot be trusted as a truthful editor surface.
- Export, enum inventory, visibility, render path, and tests still disagree with one another.

**Evidence**

- [editor/diagnostics/diagnostics_workspace.h](../editor/diagnostics/diagnostics_workspace.h) declares nine tabs: compat, save, event authority, message, battle, menu, audio, migration wizard, abilities.
- [editor/diagnostics/diagnostics_workspace.cpp](../editor/diagnostics/diagnostics_workspace.cpp) exports only seven tab summaries, omitting `Audio` and `MigrationWizard`.
- The same file still comments out `audio_panel_.render()` and `migration_wizard_panel_.render()`.
- [editor/diagnostics/event_authority_panel.cpp](../editor/diagnostics/event_authority_panel.cpp) still has a no-op `render()` implementation.
- [tests/unit/test_diagnostics_workspace.cpp](../tests/unit/test_diagnostics_workspace.cpp) still locks the seven-tab export shape rather than asserting the full enum surface.
- [editor/audio/audio_inspector_panel.h](../editor/audio/audio_inspector_panel.h) and [editor/diagnostics/migration_wizard_panel.h](../editor/diagnostics/migration_wizard_panel.h) are still visibility/model wrappers without render bodies.
- [editor/audio/audio_inspector_model.h](../editor/audio/audio_inspector_model.h) still says it simulates projection instead of iterating real `AudioCore` state.
- [editor/diagnostics/migration_wizard_model.h](../editor/diagnostics/migration_wizard_model.h) still performs only a narrow pass and ends with `"Standard asset migration complete."`

**Root cause**

- The workspace contract was expanded faster than the actual panels.
- Tests were updated to normalize the partial shape instead of forcing reconciliation.

**Required action**

- Choose one truthful workspace contract:
  - support all nine tabs end-to-end,
  - or reduce the declared/exported/tested surface to the tabs that actually render.
- Do not export or count tabs that still have no user-visible body.
- Stop treating audio and migration as first-class diagnostics tabs unless they become first-class panels.

**Owner**

- Editor/diagnostics maintainers.

**Exit criteria**

- Enum, summary export, render path, visibility logic, and tests all agree on the same tab inventory.
- No tab counted by the workspace remains renderless.

---

### P1-02 - AI And Program-Status Docs Still Overclaim Provider Maturity

**Impact**

- The repo still overstates how "done" the AI subsystem is.
- The result is planning drift, not just documentation polish debt.

**Evidence**

- [docs/PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md) still marks `Production service connectivity (OpenAI, Local Llama)` complete.
- The same file still describes `engine/core/ai/ai_connectivity.h` as `Production-ready service templates`.
- [docs/AI_COPILOT_GUIDE.md](./AI_COPILOT_GUIDE.md) still describes `OpenAIChatService` as HTTP/SSE integration for GPT-4 series models.
- [engine/core/ai/ai_connectivity.h](../engine/core/ai/ai_connectivity.h) explicitly says the provider path is simulated, constructs a request body, and does not perform real HTTP/SSE work or invoke a live callback path.
- `LlamaLocalService` in the same file is also still comment-only scaffolding.

**Root cause**

- AI planning prose moved ahead of implementation and stayed ahead after newer truth-label passes elsewhere in the repo.

**Required action**

- Downgrade AI docs from "production connectivity" to "provider abstraction scaffolding" unless a real transport path is implemented and tested.
- Separate clearly:
  - UI/orchestration,
  - local/mock/testing providers,
  - real external-provider connectivity.
- Treat any future upgrade back to "production-ready" as evidence-gated, not aspirational wording.

**Owner**

- AI subsystem owner plus doc owner.

**Exit criteria**

- AI docs make only claims supported by code and tests.
- `PROGRAM_COMPLETION_STATUS.md` no longer marks provider connectivity complete without real transport coverage.

---

### P2-01 - Build Graph Hygiene And Doc Hygiene Still Have Intent Drift

**Impact**

- The test graph is harder to trust than it should be.
- Stale registration patterns and stale public guidance increase maintenance cost and hide intent.

**Evidence**

- [CMakeLists.txt](../CMakeLists.txt) now has the accidental duplicate `tests/unit/test_engine_shell.cpp` entry removed, but it still contains a second engine-shell test source at `tests/engine/core/test_engine_shell.cpp`, which should remain intentional and documented rather than looking like leftover churn.
- Before this remediation pass, [README.md](../README.md), [docs/PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md), and [docs/DEVELOPMENT_KICKOFF.md](./DEVELOPMENT_KICKOFF.md) all published stale local build/test guidance.
- The repo now has a real validated Ninja lane, which makes this kind of registration and documentation drift easier to observe and easier to justify cleaning up.

**Root cause**

- Registration churn accumulated faster than maintenance cleanup.
- Validation snapshots and quick-start commands were updated independently from real reruns.

**Required action**

- Keep build/test instructions and published lane counts tied to fresh reruns instead of inherited snapshots.
- Audit `urpg_tests` membership intentionally instead of letting it grow opportunistically.
- Document any intentionally duplicated coverage patterns if they remain after review.

**Owner**

- Build/test owner.

**Exit criteria**

- No accidental duplicate source registration remains in core test targets.
- Public build/test instructions match real preset names and verified lane counts.
- CMake target composition is intentional and documented where non-obvious.

---

### P3-01 - Accepted Stub Surfaces Must Stay Explicitly Accepted

**Impact**

- These are no longer the top debt items, but they remain architecture debt.
- If their scope becomes fuzzy again, they will immediately turn back into trust debt.

**Evidence**

- [engine/core/editor/plugin_api.cpp](../engine/core/editor/plugin_api.cpp) still relies on placeholder-backed entity/input/global-state exports.
- [engine/core/social/cloud_service.h](../engine/core/social/cloud_service.h) still provides only `CloudServiceStub`.
- [runtimes/compat_js/quickjs_runtime.cpp](../runtimes/compat_js/quickjs_runtime.cpp) is still explicitly scoped as a fixture-backed compat harness, not a live QuickJS runtime.

**Required action**

- Keep these surfaces labeled as stub/harness surfaces everywhere.
- Do not quietly relabel them as delivered in status docs, README summaries, or closure checklists.
- Treat any real implementation of these areas as roadmap work, not incidental cleanup.

**Owner**

- Respective subsystem owners.

**Exit criteria**

- Stub scope remains explicit and stable until real implementations exist.

---

## Findings Retired Or Downgraded Since The Previous Pass

These items should no longer dominate the remediation plan:

- The immediate Ninja MinGW/MSVC header failure is no longer the primary blocker.
- `dev-ninja-debug` now configures cleanly and `dev-debug` now builds cleanly on the audited machine.
- The `BattleManager` `removeByDamage` regression is fixed.
- The compat tactical-routing reload regression is fixed.
- The non-fatal Ninja bookkeeping warning was traced to the Windows pre-link unlock hook and removed from the Ninja lane by scoping that hook away from Ninja generator builds.
- PR and weekly lanes are green again on the verified Ninja path.
- Build reproducibility is therefore no longer the top issue for this revision; editor/doc/build-hygiene truthfulness is.

Important caveat:

- This pass only revalidated the Ninja Debug lane. Other local Windows lanes should not be upgraded in documentation without their own fresh evidence.

---

## Remediation Order

### Phase 0 - Reconcile Diagnostics Workspace Contract

Scope:

- Decide the real tab inventory.
- Either render audio/migration/event-authority meaningfully or demote them.
- Make export and tests match the chosen surface.

Deliverables:

- a truthful diagnostics workspace contract,
- tests that assert the real tab surface instead of a normalized partial export.

### Phase 1 - Deflate AI Completion Claims To Verified Reality

Scope:

- Rewrite provider-connectivity claims.
- Distinguish scaffolding from operational transport.
- Keep completion docs evidence-gated.

### Phase 2 - Finish Build/Test Hygiene Cleanup

Scope:

- Remove accidental test-target duplication.
- Keep public build/test guidance synced with real reruns.

### Phase 3 - Preserve Honest Stub Scope

Scope:

- Keep plugin/cloud/QuickJS scaffolding explicitly scoped.
- Do not allow summary docs to silently promote them.

---

## Verification Plan

No finding in this document is closed until it is backed by fresh evidence from the same repository revision.

Required closure commands:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
ctest --test-dir build/dev-ninja-debug -L weekly --output-on-failure
```

Additional closure gates by area:

- Diagnostics workspace:
  - exported snapshot reflects the real enum/rendered tab set,
  - tests assert the full declared workspace contract.
- AI documentation:
  - docs only claim behavior covered by implementation plus tests.
- README:
  - every command path is runnable as written.

---

## Definition Of Done

A remediation item is done only when all of the following are true:

- The documented Ninja Debug build path works on a fresh configure/build.
- The affected test lane is green on that same path.
- README, status docs, and subsystem guides were updated in the same change if they described the affected area.
- Exported/editor-facing surface descriptions match the real product surface.
- Any remaining stub behavior is still labeled as stub behavior.

---

## Immediate Recommended Execution Order

1. Reconcile diagnostics workspace tab/export/render/test drift.
2. Downgrade AI/provider maturity claims to what the code actually supports.
3. Finish build/test hygiene cleanup around duplicate registration, stale snapshots, and the persistent Ninja warning.
4. Preserve explicit stub scope in public-facing docs and checklists.

---

## Change Log

| Date | Change |
| --- | --- |
| 2026-04-18 | Third full rewrite after the PR and weekly regressions were fixed. Rebased priorities around diagnostics drift, AI/doc overclaiming, remaining build/doc hygiene, and the persistent non-fatal Ninja warning. |
| 2026-04-18 | Second full rewrite after the Ninja toolchain fix. Rebased priorities around truthful validation, two live test regressions, diagnostics workspace drift, and AI/status overclaiming. |
| 2026-04-18 | Retired the earlier same-day framing that still treated the Ninja header/toolchain failure as the primary active debt item. |
