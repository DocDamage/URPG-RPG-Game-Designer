# Technical Debt Audit Sprint Checklist - 2026-04-24

Status: refreshed current-tree audit and follow-up sprint plan  
Audit date: 2026-04-24  
Worktree: `codex/refresh-technical-debt-audit-2026-04-22`  
Scope: replace the stale 2026-04-24 sprint checklist with findings from a fresh source, build, CTest, ProjectAudit, export, visual-regression, and governance pass.

This file remains an execution checklist, not a new source of truth. If any sprint changes readiness status, update the canonical status stack in the same change:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TECHNICAL_DEBT_ACTION_PLAN.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `content/readiness/readiness_status.json`
- `WORKLOG.md`

## Audit Summary

The previous contents of this file were stale. The old high-priority item, the `Window_Base` gauge golden mismatch, is no longer current: the focused CTest case now passes. The export license audit and bundle-size concerns are also materially remediated in code and tests.

The audit baseline top technical debt was more structural: broad CTest discovery and label selection could fail while direct test executables still passed. That made release evidence fragile because `ctest -L '^weekly$'`, `ctest -L '^nightly$'`, and broad `ctest --preset dev-all -N` tripped a generated CTest parse error in `urpg_compat_tests-b12d07c_tests.cmake`. Sprint 1 closed this by regenerating clean discovery metadata, proving clean-build discovery, and adding a CI/local CTest discovery guard before label selection.

At the same time, the repo is not in a bad product-truth state. The worktree is clean, the exact `pr` lane passes, direct compat/integration/snapshot executables pass, ProjectAudit has zero export blockers, and readiness/truth/CMake governance checks pass. The remaining risk is that orchestration metadata and release-promotion governance lag behind the code.

## Commands Run

```powershell
git status --short --branch
cmake --build --preset dev-debug --target urpg_integration_tests urpg_snapshot_tests urpg_compat_tests urpg_presentation_release_validation
ctest --preset dev-all -N
ctest --preset dev-all -L '^pr$' --output-on-failure
ctest --preset dev-all -L '^weekly$' --output-on-failure
ctest --preset dev-all -L '^nightly$' --output-on-failure
ctest --preset dev-all --output-on-failure -R "Window_Base status and gauge surface"
ctest --preset dev-all -L '^regression$' --output-on-failure
.\build\dev-ninja-debug\urpg_compat_tests.exe "[compat]" --reporter compact
.\build\dev-ninja-debug\urpg_integration_tests.exe --reporter compact
.\build\dev-ninja-debug\urpg_snapshot_tests.exe --reporter compact
.\build\dev-ninja-debug\urpg_presentation_release_validation.exe
.\build\dev-ninja-debug\urpg_project_audit.exe --json
powershell -ExecutionPolicy Bypass -File tools\ci\check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools\ci\truth_reconciler.ps1
powershell -ExecutionPolicy Bypass -File tools\ci\check_cmake_completeness.ps1
powershell -ExecutionPolicy Bypass -File tools\ci\check_waivers.ps1
rg -n "runLicenseAudit|bundleAssets|uint32|AssetLicenseAuditor|data\.pck" engine/core/tools engine/core/export tools tests
rg --files engine editor runtimes tools tests | line-count sort for hot spots
```

## Current Validation Snapshot

| Check | Result |
| --- | --- |
| Git status | Clean on `codex/refresh-technical-debt-audit-2026-04-22` |
| Missing test target rebuild | Passed for integration, snapshot, compat, and presentation validation targets |
| Exact PR lane | Passed: `1162/1162` |
| Broad CTest discovery | Baseline failed; Sprint 1 closure passed with 1,244 discovered tests |
| Anchored weekly label | Baseline failed before selection; Sprint 1 closure passed 47/47 |
| Anchored nightly label | Baseline failed before selection; Sprint 1 closure passed 35/35 |
| Focused gauge visual test | Passed |
| Snapshot regression lane | Passed: 42 assertions / 7 test cases |
| Direct compat executable | Passed: 3557 assertions / 47 test cases |
| Direct integration executable | Passed: 110 assertions / 11 test cases |
| Direct snapshot executable | Passed: 150 assertions / 22 test cases |
| Presentation release validation executable | Passed |
| ProjectAudit | `PARTIAL`, 2 release blockers, 0 export blockers |
| Release readiness check | Passed |
| Truth reconciler | Passed |
| CMake completeness | Passed |
| Known-break waivers | Passed; zero active waivers |

## Finding Status

| ID | Priority | Finding | Status |
| --- | --- | --- | --- |
| TD-AUD-01 | P0 | Broad CTest discovery/label selection is broken by generated compat test metadata | Closed |
| TD-AUD-02 | P1 | Release promotion remains human-review blocked for battle/save | Evidence refreshed; human review pending |
| TD-AUD-03 | P2 | Export hardening is stronger, but runtime-side signature enforcement and native signing remain backlog | Closed as design/readiness boundary |
| TD-AUD-04 | P2 | Renderer-backed visual goldens are very large and expensive to review/store | Closed as governance guard |
| TD-AUD-05 | P2 | Product-feature coverage remains partial across multiple readiness rows | First vertical slice landed; broader lanes open |
| TD-AUD-06 | P3 | Complexity hot spots remain after the first split pass | Diagnostics and compat fixture splits landed; mixed-chain helper extraction continued |
| TD-AUD-OLD-01 | P1 | Window gauge golden mismatch | Closed |
| TD-AUD-OLD-02 | P2 | Export license audit unconditional pass | Closed |
| TD-AUD-OLD-03 | P3 | Bundle write failure handling | Mostly closed; keep atomic temp-file hardening as backlog |
| TD-AUD-OLD-04 | P3 | Bundle offsets/manifest size can wrap 32-bit fields | Closed for current uint32-bounded format |
| TD-AUD-OLD-05 | P2 | Stale audit/status docs | Closed by this rewrite for this file |
| TD-AUD-OLD-06 | P2 | Dirty worktree/release hygiene sweep | Closed for current audit: worktree is clean |

## Priority Findings

### TD-AUD-01 - Broad CTest Discovery/Label Selection Is Broken

**Priority:** P0  
**Evidence:** After rebuilding missing test binaries, broad discovery and label selection still fail:

```text
CMake Error at build/dev-ninja-debug/urpg_compat_tests-b12d07c_tests.cmake:33:
  Parse error. Function missing ending ")". Instead found unterminated
  bracket with text "Compat fixtures: curated message-text scenarios survive pl".
```

The generated file points at the discovered Catch2 test:

```cpp
TEST_CASE("Compat fixtures: curated message-text scenarios survive plugin reload",
          "[compat][fixtures]")
```

Direct execution of `urpg_compat_tests.exe "[compat]"` passes, so this is CTest metadata/orchestration debt rather than a failing compat behavior test. It still blocks the release workflow because weekly/nightly label lanes cannot be trusted while CTest parsing is broken.

**Sprint Checklist:**

- [x] Reproduce from a clean build tree, not only the current local `build/dev-ninja-debug` directory.
- [x] Inspect Catch2 generated CMake output around `urpg_compat_tests-b12d07c_tests.cmake:33`.
- [x] Determine whether the root cause is a Catch2 discovery escaping bug, generated-file corruption, test-name content, or stale CTest include state.
- [x] Add a regression guard that runs `ctest --preset dev-all -N` and fails on generated CTest parse errors.
- [x] Restore `ctest --preset dev-all -L '^weekly$' --output-on-failure`.
- [x] Restore `ctest --preset dev-all -L '^nightly$' --output-on-failure`.
- [x] Re-run the full local gate wrapper after the label lanes are fixed.
- [x] Record the fixed commands and counts in `WORKLOG.md`.

**Exit Criteria:**

- [x] Broad CTest discovery succeeds without parse errors.
- [x] Exact PR, weekly, nightly, and regression lanes all run through CTest.
- [x] Direct executable success is no longer needed as a workaround for broken CTest metadata.

**Closure Evidence (2026-04-24):** A clean `build\dev-ninja-debug-td-aud-01-clean` configure plus targeted test build discovered `1244` tests via `tools\ci\check_ctest_discovery.ps1` without generated CTest parse errors. The current `build\dev-ninja-debug` label lanes passed through CTest: PR `1162/1162`, nightly `35/35`, weekly `47/47`, and regression `1/1`. The full `tools\ci\run_local_gates.ps1` wrapper passed end to end with the new discovery guard wired before label selection. The inspected generated compat CTest file is valid in the current tree, so the old failure is treated as stale/corrupted generated discovery metadata rather than a bad compat behavior test or bad test-name content.

### TD-AUD-02 - Release Promotion Remains Human-Review Blocked

**Priority:** P1  
**Evidence:** `urpg_project_audit --json` reports `releaseBlockerCount: 2` and `exportBlockerCount: 0`. The blockers are `battle_core` and `save_data_core`, both intentionally `PARTIAL` until the release-signoff workflow records human review.

This is honest debt rather than status inflation. The risk is process stagnation: the evidence stack can stay green while the product remains unpromotable.

**Sprint Checklist:**

- [x] Re-run the battle and save validation suites listed in the signoff artifacts.
- [ ] Have a human reviewer record accept/reject notes for `battle_core`.
- [ ] Have a human reviewer record accept/reject notes for `save_data_core`.
- [x] Update `content/readiness/readiness_status.json` only for lanes with explicit approval. No approval was recorded, so no readiness promotion was made.
- [x] Re-run `urpg_project_audit --json` and confirm release-blocker changes are review-backed. Output remains `releaseBlockerCount: 2`, which matches the pending human-review state.
- [x] Update `docs/RELEASE_READINESS_MATRIX.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, and `WORKLOG.md`.

**Exit Criteria:**

- [ ] `battle_core` and `save_data_core` have current human-review decisions.
- [ ] Release blocker count reflects actual review state, not stale pending status.

**Evidence Refresh (2026-04-24):** Automated battle/save evidence is current, but the sprint cannot honestly close its human-review exit criteria without a designated human reviewer recording accept/reject decisions. The refreshed validation passed: battle core `39/5`, battle migration `207/35`, full battle tag `611/88`, save tag `509/62`, save migration `49/5`, save schema `87/6`, battle-save integration `44/4`, Wave 1 closure integration `39/4`, save policy governance, release readiness, and truth reconciliation. `urpg_project_audit --json` still reports `releaseBlockerCount: 2` and `exportBlockerCount: 0`, correctly preserving the release block until human signoff exists.

### TD-AUD-03 - Export Hardening Is Stronger But Not Release-Grade

**Priority:** P2  
**Evidence:** The old export-audit findings are no longer accurate:

- `ExportPackager::runLicenseAudit()` now calls promoted-bundle and auto-discovered asset license checks.
- Export tests cover missing promoted source license evidence, disallowed source legal disposition, malformed discovered asset license evidence, and oversized discovered payload rejection.
- `writeBundleFile()` now checks uint32 payload/manifest bounds and stream state after close.
- `ExportValidator` enforces keyed SHA-256 bundle-signature validation at post-export time.

The remaining debt is the readiness residual already named by ProjectAudit: runtime-side signature enforcement, full native signing/notarization, and shipping-grade packaging are still not landed.

**Sprint Checklist:**

- [x] Keep the old license-audit and uint32-wrap sprint items marked closed.
- [x] Add runtime-side bundle-signature enforcement design notes before any `READY` export claim.
- [x] Document the required runtime/load-time rejection tests for tampered `data.pck` once runtime enforcement exists.
- [x] Decide whether bundle writes should move to temp-file-plus-atomic-rename for stronger partial-write protection.
- [x] Keep signing/notarization language explicitly out of scope until platform-specific implementation exists.

**Exit Criteria:**

- [x] Export readiness wording distinguishes current validator-time protection from runtime enforcement.
- [x] Any future promotion is now blocked on runtime-side tamper rejection and platform packaging evidence.

**Closure Evidence (2026-04-24):** `docs/EXPORT_RUNTIME_SIGNATURE_ENFORCEMENT_DESIGN.md` now records the required runtime/load-time `data.pck` signature-enforcement contract, the test cases required once runtime enforcement exists, and the decision to move future bundle publication to temp-file-plus-atomic-rename before release-grade packaging claims. `content/readiness/readiness_status.json`, `docs/RELEASE_READINESS_MATRIX.md`, and `docs/PROGRAM_COMPLETION_STATUS.md` distinguish current validator-time keyed SHA-256 bundle-signature validation from missing runtime enforcement. A governance regression in `tests/unit/test_s32_wysiwyg_lanes.cpp` keeps that design note and boundary wording present. No `READY` export promotion was made.

### TD-AUD-04 - Renderer-Backed Visual Goldens Are Oversized

**Priority:** P2  
**Evidence:** The renderer-backed visual lane is real and passes, but several committed golden files are huge line-oriented JSON artifacts:

| Lines | File |
| ---: | --- |
| 2,880,006 | `tests/snapshot/goldens/RendererBackedCapture_battle_scene_textured_runtime_path.golden.json` |
| 2,880,006 | `tests/snapshot/goldens/S29TransitionGoldens_post_battle_entry.golden.json` |
| 2,880,006 | `tests/snapshot/goldens/S29SceneGoldens_battle_scene_full_frame.golden.json` |
| 1,728,006 | `tests/snapshot/goldens/RendererBackedCapture_chat_window_mixed_path.golden.json` |
| 1,536,006 | `tests/snapshot/goldens/RendererBackedCapture_engine_shell_mapscene_mixed_runtime_path.golden.json` |

This does not make the tests wrong. It does make reviews, diffs, storage, and accidental churn expensive.

**Sprint Checklist:**

- [x] Decide whether full-frame goldens should stay JSON or move to a compact binary/PNG-backed representation with manifest metadata.
- [x] Keep at least one human-reviewable crop or summary artifact per full-frame golden.
- [x] Ensure visual diff tooling can still produce actionable diagnostics after any format change.
- [x] Confirm `.gitattributes` and LFS policy match the chosen golden format.
- [x] Add a size/churn guard so future visual goldens do not silently balloon.

**Exit Criteria:**

- [x] Visual regression coverage remains executable and reviewable.
- [x] Large goldens have a governed storage and review strategy.

**Closure Evidence (2026-04-24):** Existing oversized renderer-backed JSON baselines remain in place to avoid churn-only rewrites, but `content/fixtures/visual_golden_governance.json` now governs every committed `*.golden.json` above 10 MiB with a reason, review strategy, human-review artifact or summary, and per-file `maxBytes` ceiling. `tools/ci/check_visual_golden_governance.ps1` enforces the manifest, the current `.gitattributes` JSON-as-text policy, and future PNG/LFS coverage. `tests/unit/test_visual_regression_harness.cpp` now contains an executable `[testing][visual_regression]` regression proving every oversized golden is governed. `docs/presentation/VISUAL_GOLDEN_GOVERNANCE.md` records the storage decision and the future compact PNG/binary migration boundary.

### TD-AUD-05 - Product-Feature Coverage Remains Partial

**Priority:** P2  
**Evidence:** ProjectAudit is truthful but still reports multiple non-ready subsystems and bars. Current partial lanes include:

- `compat_bridge_exit`: bounded fixture-backed bridge, not live plugin runtime parity.
- `presentation_runtime`: bounded OpenGL-backed proof, not cross-backend parity.
- `gameplay_ability_framework`: richer task backends are landed, but authored battle/map product depth and scripted conditions remain partial.
- `governance_foundation`: CLI diagnostics and editor panel output are not unified into one parity export path.
- `character_identity`: runtime creator screen landed, but author-defined rules, persistence, and layered compositor depth remain.
- `achievement_registry`: trophy export payload and platform backends remain deferred.
- `accessibility_auditor`: renderer-derived WYSIWYG contrast population remains future work.
- `audio_mix_presets`: live audio backend/device integration remains outside the current harness.
- `mod_registry`: live loading, sandboxed execution, reload, and store integration remain deferred.
- `analytics_dispatcher`: remote upload, cross-session aggregation, and privacy audit workflow remain deferred.

**Sprint Checklist:**

- [x] Convert the remaining partial lanes into owned vertical slices rather than broad matrix prose.
- [x] Pick one product-facing lane to advance first with runtime, editor, diagnostics, and tests.
- [x] Keep every deferred item visible in `mainGaps`, not only in narrative docs.
- [x] Do not promote any template to `READY` while required bars remain `PARTIAL` or `PLANNED`.

**Exit Criteria:**

- [x] Partial lanes have current owners and next vertical slices.
- [x] Readiness docs continue to make unfinished product behavior obvious.

**Closure Evidence (2026-04-24):** `content/readiness/partial_lane_vertical_slices.json` now maps the ten audit-listed partial lanes to current owner tracks, next vertical slices, and explicit deferred scope. The first product-facing slice advanced `achievement_registry`: `AchievementRegistry::exportTrophyPayload()` now emits a vendor-neutral trophy payload, `AchievementPanel` surfaces its summary, `content/schemas/achievement_trophy_export.schema.json` and `content/fixtures/achievement_trophy_export_fixture.json` define the governed contract, and `check_achievement_governance.ps1` validates the new artifacts. Focused tests cover runtime payload generation, editor summary exposure, governance script output, and the partial-lane vertical-slice manifest. No subsystem or template was promoted to `READY`; platform-specific achievement backend integration remains explicitly out-of-tree in `mainGaps`.

### TD-AUD-06 - Complexity Hot Spots Remain

**Priority:** P3  
**Evidence:** The first split pass helped, but several active source/test files are still large:

| Lines | File |
| ---: | --- |
| 1,700 | `tests/unit/test_diagnostics_workspace.cpp` |
| 909 | `tests/unit/test_diagnostics_workspace_migration.cpp` |
| 356 | `tests/unit/test_diagnostics_workspace_message_menu.cpp` |
| 221 | `tests/unit/test_diagnostics_workspace_audio_event.cpp` |
| 91 | `tests/unit/test_diagnostics_workspace_battle.cpp` |
| 1,367 | `tests/compat/test_compat_plugin_fixtures.cpp` |
| 830 | `tests/compat/test_compat_plugin_fixtures_content.cpp` |
| 660 | `tests/compat/test_compat_plugin_fixtures_battle.cpp` |
| 600 | `tests/compat/test_compat_plugin_fixtures_menu_presentation.cpp` |
| 2,172 | `tests/compat/test_compat_plugin_failure_mixed_chain.cpp` |
| 2,139 | `runtimes/compat_js/window_compat.cpp` |
| 371 | `runtimes/compat_js/window_sprite_compat.cpp` |

These are not immediate correctness blockers, but they slow review and make domain ownership blurry.

**Sprint Checklist:**

- [x] Split `test_diagnostics_workspace.cpp` by diagnostics tab/domain.
- [x] Continue splitting compat fixture tests by profile family and failure class.
- [x] Extract stable helpers from `window_compat.cpp` only after tests are easier to target.
- [x] Keep behavior-preserving splits separate from feature changes.
- [x] Re-run focused filters and exact PR lane after each split.

**Exit Criteria:**

- [x] Review surfaces are smaller without reducing coverage for the diagnostics workspace slice.
- [x] Test filters remain meaningful by diagnostics domain for the split slice.

**Progress Evidence (2026-04-24):** The oversized diagnostics workspace test file was split into focused migration, audio/event, message/menu, and battle translation units while leaving the ability-heavy core workspace coverage in `test_diagnostics_workspace.cpp`. `CMakeLists.txt` now registers the new files. Focused validation passed for `[editor][diagnostics][integration]`, `[menu_parity]`, `[message_actions]`, and `[battle_preview]`; `check_ctest_discovery.ps1` discovered 1,249 tests; `check_cmake_completeness.ps1` passed; and the exact PR CTest lane passed 1,167/1,167.

**Additional Progress Evidence (2026-04-24):** The curated compat fixture surface was split by profile family: content/save/library scenarios moved to `test_compat_plugin_fixtures_content.cpp`, menu/presentation/reload scenarios moved to `test_compat_plugin_fixtures_menu_presentation.cpp`, and the original DSL/orchestration fixture file dropped to 1,367 lines. `CMakeLists.txt` registers both new compat files. `urpg_compat_tests "[compat][fixtures]"` passed 3,449 assertions / 44 test cases, `ctest -L weekly` passed 47/47, `check_ctest_discovery.ps1` still discovered 1,249 tests, and `check_cmake_completeness.ps1` passed.

**Mixed-Chain Progress Evidence (2026-04-24):** `test_compat_plugin_failure_mixed_chain.cpp` still intentionally owns one aggregate weekly diagnostic stream, so it was not mechanically split. Instead, the setup phases were extracted into named local helpers: curated fixture happy-path loading, dependency-gate probe verification, weekly lifecycle fixture creation/exercise, diagnostic operation counts, dependency-gate diagnostic rows, runtime diagnostic rows, and load-plugin diagnostic rows. The file dropped from 2,477 to 2,172 lines after the helper pass, and the focused `[compat][fixtures][failure][weekly]` case passed 533 assertions / 1 test case after each extraction. Any future mixed-chain split should preserve its aggregate report-model/panel projection coverage.

**Window Compat Progress Evidence (2026-04-24):** The sprite-specific compat implementation moved from `window_compat.cpp` into `window_sprite_compat.cpp`: tracked sprite bitmap handles, `Sprite_Character`, and `Sprite_Actor` now build as a separate translation unit registered in `CMakeLists.txt`. This reduced `window_compat.cpp` from 2,492 baseline lines to 2,139 while keeping the window base/selectable/command/message surface in place. `cmake --build --preset dev-debug --target urpg_tests urpg_compat_tests`, `urpg_tests "[window]"`, the focused mixed-chain weekly compat case, `ctest --test-dir build\dev-ninja-debug -L weekly --output-on-failure`, `check_cmake_completeness.ps1`, and `check_ctest_discovery.ps1 -BuildDirectory build\dev-ninja-debug` all passed after the split.

## Superseded Old Sprint Items

### TD-AUD-OLD-01 - Restore Nightly Visual Gate

**Status:** Closed for the old gauge finding.  
**Evidence:** `ctest --preset dev-all --output-on-failure -R "Window_Base status and gauge surface"` passed. Direct snapshot regression also passed.

Nightly as a label is still blocked, but by TD-AUD-01 CTest metadata parsing, not by the gauge golden.

### TD-AUD-OLD-02 - Replace Export License Audit Stub

**Status:** Closed.  
**Evidence:** `runLicenseAudit()` now calls `auditPromotedAssetBundleLicenses()` and `auditAutoDiscoveredAssetLicenses()`. Tests cover missing, malformed, and disallowed license evidence and prove export fails before `data.pck` is emitted.

### TD-AUD-OLD-03 - Harden Bundle Write Failure Handling

**Status:** Mostly closed.  
**Evidence:** `writeBundleFile()` checks file open state and stream state after close, and callers fail when `BundleWriteResult.success` is false.

Residual backlog: temp-file plus atomic rename would make partial-write behavior stronger, but the old "always reports data.pck after failed write" finding is no longer accurate.

### TD-AUD-OLD-04 - Make Bundle Offsets Size-Safe

**Status:** Closed for the current format.  
**Evidence:** the bundle writer enforces uint32 bounds on payload sizes, cumulative offsets, and manifest length. Export tests include oversized discovered payload rejection.

### TD-AUD-OLD-05 - Refresh Stale Audit And Status Docs

**Status:** Closed for this file by this rewrite.  
**Residual:** broader canonical docs still carry 2026-04-23 status dates. That is acceptable if no status promotion happens, but the next sprint that changes status must update the whole canonical stack.

### TD-AUD-OLD-06 - Worktree And Release Hygiene Sweep

**Status:** Closed for this audit.  
**Evidence:** `git status --short --branch` shows a clean worktree.

## Recommended Execution Order

1. Fix `TD-AUD-01` first. Release orchestration cannot rely on direct executables as a substitute for broken CTest discovery.
2. Re-run full local gates after CTest discovery is fixed.
3. Run `TD-AUD-02` human-review signoff for battle/save.
4. Keep `TD-AUD-03` export hardening residuals as pre-READY backlog.
5. Keep `TD-AUD-04` visual golden governance active while adding future renderer-backed coverage.
6. Keep advancing `TD-AUD-05` through the owned vertical-slice map; the first `achievement_registry` slice is landed without `READY` promotion.
7. Continue `TD-AUD-06` complexity paydown opportunistically, separate from behavior changes.

## Global Done Rules

- [ ] Code behavior is fixed, or the claim is narrowed to match reality.
- [ ] Positive and negative tests cover the changed behavior.
- [ ] CTest discovery and label lanes pass, not only direct executables.
- [ ] `urpg_project_audit` output still reflects the correct release/export posture.
- [ ] Canonical docs and worklog evidence match the commands actually run.
- [ ] No sprint is marked complete while a known gate for that sprint is red.
