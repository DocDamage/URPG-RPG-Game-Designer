# Technical Debt Audit - 2026-04-23

## Scope

This audit refreshes the current technical-debt picture after the 2026-04-21 audit and the later remediation/action-plan updates. It focuses on repository truth: build/test registration, readiness/audit output, TODO/stub signals, governance drift, and oversized code surfaces.

This is not a full dynamic validation pass. I did not run the full test suite, sanitizer builds, packaging smoke execution, or renderer-backed snapshot execution. Those remain required before promotion decisions.

## Commands Run

```powershell
git status --short
.\tools\ci\check_waivers.ps1
.\tools\ci\check_cmake_completeness.ps1
ctest --preset dev-all -N
.\build\dev-ninja-debug\urpg_project_audit.exe --json
rg -n "TODO|FIXME|HACK|XXX|TBD" engine editor runtimes tests tools
rg -n "not implemented|stub|placeholder|mock-backed|fixture-backed|future work|out of scope" engine editor runtimes tests tools
git ls-files .env* gitleaks.toml .gitignore .gitattributes
```

## Baseline Results

| Check | Result |
|---|---|
| Git status | Clean |
| Waivers | Passed; `tools/ci/known_break_waivers.json` has zero active waivers |
| CMake completeness | Passed |
| CTest discovery | `ctest --preset dev-all -N` enumerated 1,220 tests |
| Project audit | `Readiness audit for jrpg (PARTIAL)` |
| Project audit blockers | 29 issues, 2 release blockers, 0 export blockers |
| Tracked `.env*` files | None tracked |

## Coverage Cross-Check

A second pass compared this audit against `content/readiness/readiness_status.json`, `docs/release/RELEASE_READINESS_MATRIX.md`, `docs/governance/TEMPLATE_READINESS_MATRIX.md`, `docs/archive/planning/NATIVE_FEATURE_ABSORPTION_PLAN.md`, and `docs/status/PROGRAM_COMPLETION_STATUS.md`.

That pass found adjacent feature/product gaps that were not explicit enough in the first ten findings. They are now tracked as `TD-2026-04-23-11` and `TD-Sprint-06`.

## Priority Findings

### TD-2026-04-23-01 - Release Readiness Is Still Human-Review Gated

**Priority:** P1
**Evidence:** `urpg_project_audit` reports two release blockers: `battle_core` and `save_data_core` are required for the selected `jrpg` template but remain `PARTIAL`.

The code and docs are healthier than before because the residual is explicit rather than hidden. The remaining debt is the promotion workflow itself: the signoff artifacts exist, but they deliberately do not grant `READY`.

**Next step:** Execute the human-review closure path in `docs/release/RELEASE_SIGNOFF_WORKFLOW.md` for battle and save, then update `content/readiness/readiness_status.json` only if the review accepts the residuals.

**Checklist:**

- [ ] Re-run the targeted battle and save validation suites listed in the signoff artifacts.
- [ ] Have a human reviewer record accept/reject notes for `battle_core`.
- [ ] Have a human reviewer record accept/reject notes for `save_data_core`.
- [ ] Update `content/readiness/readiness_status.json` only for lanes whose residuals are accepted.
- [ ] Re-run `urpg_project_audit --json` and confirm the release blocker count changes only for approved lanes.
- [ ] Update `docs/release/RELEASE_READINESS_MATRIX.md` and `docs/status/PROGRAM_COMPLETION_STATUS.md` in the same change.

### TD-2026-04-23-02 - Project Schema And Audit Vocabulary Are Misaligned

**Priority:** P1
**Evidence:** `urpg_project_audit` reports:

- `project_schema.localization_missing`
- `project_schema.input_governance_missing`
- `project_schema.export_governance_missing`

`content/schemas/project.schema.json` still only defines the narrow base project/determinism contract while the audit expects localization, input/controller, and export governance sections for templates that depend on those bars.

**Next step:** Start `TD-S02` from `docs/archive/planning/TECHNICAL_DEBT_ACTION_PLAN.md`: choose the canonical property names, extend the schema, add fixtures/tests, and update audit/docs in one change.

**Checklist:**

- [ ] Choose canonical property names for localization, input/controller, and export governance.
- [ ] Extend `content/schemas/project.schema.json` with bounded validation for those sections.
- [ ] Add a canonical project fixture that exercises the new sections.
- [ ] Update `tools/audit/urpg_project_audit.cpp` to validate the same vocabulary.
- [ ] Add positive and negative project-audit tests for missing and malformed governance sections.
- [ ] Update `docs/audits/PROJECT_AUDIT.md`, `docs/release/RELEASE_READINESS_MATRIX.md`, and readiness records.

### TD-2026-04-23-03 - Localization Governance Has Two Partial Stories

**Priority:** P1
**Evidence:** The canonical localization consistency report exists and is parseable, but it reports `status: no_bundles`. The audit also expects missing artifacts:

- `content/schemas/localization_catalog.schema.json`
- `tools/localization/extract_localization.cpp`
- `tools/localization/writeback_localization.cpp`

The current runtime/schema/report lane is not the same as the audit/tooling lane.

**Next step:** Complete `TD-S03`: either add the catalog/extract/writeback artifacts or narrow `urpg_project_audit` to the already-shipped `localization_bundle` contract.

**Checklist:**

- [ ] Decide whether `localization_catalog` is real scope or stale audit vocabulary.
- [ ] If real scope, add `content/schemas/localization_catalog.schema.json`.
- [ ] If real scope, add `tools/localization/extract_localization.cpp`.
- [ ] If real scope, add `tools/localization/writeback_localization.cpp`.
- [ ] If narrowing scope, update `urpg_project_audit` to expect the shipped `localization_bundle` contract.
- [ ] Add tests for no-bundle, clean-bundle, missing-key, and malformed-report cases.
- [ ] Re-run `tools/ci/check_localization_consistency.ps1` and record the expected report state.

### TD-2026-04-23-04 - Export Has Real Bounded Packaging But No Canonical Pack CLI

**Priority:** P1
**Evidence:** `export_validator` is still `PARTIAL`, and the project audit reports `export_artifact.cli_missing` for `tools/pack/pack_cli.cpp`. Current implementation includes bounded packager/validator coverage, keyed SHA-256 bundle validation, and platform bootstrap tests, but docs correctly keep broader discovery, signing/notarization, and runtime-side enforcement as backlog.

**Next step:** Complete `TD-S04`: choose a canonical CLI path or formally document the replacement, then keep packager preflight, post-export validation, and release signing language separated.

**Checklist:**

- [ ] Decide whether the canonical CLI is `tools/pack/pack_cli.cpp` or another documented entry point.
- [ ] Add the CLI or update `urpg_project_audit` to expect the real canonical replacement.
- [ ] Add CLI smoke coverage for at least one successful export and one validation failure.
- [ ] Preserve deterministic ordering and hash/signature output in tests.
- [ ] Keep preflight readiness, post-export validation, and release signing/notarization wording separate.
- [ ] Update `docs/release/RELEASE_READINESS_MATRIX.md` and `docs/status/PROGRAM_COMPLETION_STATUS.md`.

### TD-2026-04-23-05 - Compat Runtime Contract Is Honest But Still Harness-Shaped

**Priority:** P2
**Evidence:** `runtimes/compat_js/quickjs_runtime.cpp` explicitly says it is a fixture-backed harness, not a live QuickJS runtime. The compat bridge is `PARTIAL`, and code comments/status labels still identify harness-backed surfaces.

This is no longer status-inflation debt, but it remains product debt if live plugin execution is expected.

**Next step:** Complete `TD-S01`: decide whether the shipped contract is a deterministic compat harness or a live runtime, then enforce that wording and failure behavior in code, tests, docs, and readiness.

**Checklist:**

- [ ] Decide the shipped in-tree compat contract: deterministic harness or live runtime.
- [ ] Update `quickjs_runtime.cpp` diagnostics and comments to match that contract.
- [ ] Update `plugin_manager.cpp` load/execute diagnostics and async callback wording.
- [ ] Reconcile status notes in `battle_manager.h` and `window_compat.cpp`.
- [ ] Add tests that unsupported live-JS behavior fails loudly and deterministically.
- [ ] Add a wording guard so "live runtime" claims cannot drift back without evidence.

### TD-2026-04-23-06 - WindowCompat/UI Fidelity Still Contains High-Use TODOs

**Priority:** P2
**Evidence:** Active TODO scan finds residual compat UI gaps:

- `runtimes/compat_js/window_compat.h`: icon-set bitmap rendering, gauge gradients, character-sheet rendering
- `runtimes/compat_js/window_compat.cpp`: color object parsing
- `runtimes/compat_js/battle_manager.h`: troop/party setup and battleback/audio playback scope notes

These are mostly truthfully marked `PARTIAL`, but they sit on plugin-facing surfaces.

**Next step:** Complete `TD-S07`: decide the minimal pixel/bitmap/gauge contract, implement JS color-object parsing, and add plugin-profile regressions.

**Checklist:**

- [ ] Decide whether `contents()` exposes a minimal pixel buffer or a documented non-pixel handle contract.
- [ ] Implement JS color-object parsing for `changeTextColor` and `drawGauge`.
- [ ] Implement gradient-capable gauge rendering or narrow the gauge contract explicitly.
- [ ] Add regressions for gauge rendering semantics, contents lifecycle, and color parsing.
- [ ] Re-audit `Window_Base`, `Window_Selectable`, and related status labels.
- [ ] Update compat docs/readiness wording after implementation or narrowing.

### TD-2026-04-23-07 - Asset Intake Is Governed But Not Activated

**Priority:** P2
**Evidence:** `imports/reports/asset_intake/source_capture_status.json` reports:

- `total_sources: 5`
- `cataloged_not_mirrored: 5`
- `normalized: 0`
- `promoted: 0`

Governance is truthful, but no asset lane is yet promoted into runtime/editor/export proof.

**Next step:** Complete `TD-S05`: promote one visual and one audio lane with manifests, reports, and a WYSIWYG smoke proof.

**Checklist:**

- [ ] Select one visual source lane and one audio source lane for first promotion.
- [ ] Mirror source artifacts with provenance and license notes preserved.
- [ ] Normalize the selected assets into repo-ready outputs.
- [ ] Add promotion manifests and refresh `imports/reports`.
- [ ] Wire at least one promoted visual asset into editor/runtime/export smoke coverage.
- [ ] Wire at least one promoted audio asset into resolver/runtime/export smoke coverage.
- [ ] Add audit coverage that fails if normalized or promoted counts regress to zero.

### TD-2026-04-23-08 - Gameplay Ability Runtime Depth Lags Editor Breadth

**Priority:** P2
**Evidence:** `gameplay_ability_framework` remains `PARTIAL`. The readiness gap explicitly calls out missing async task backends: `WaitInput`, `WaitEvent`, and projectile collision. Scripted `activeCondition`/`passiveCondition` strings remain intentionally unsupported.

**Next step:** Complete `TD-S06`: add the next deterministic task backend or narrow the shipped contract and keep readiness/signoff wording conservative.

**Checklist:**

- [ ] Implement `AbilityTask_WaitInput`, or formally narrow the in-tree task contract.
- [ ] Implement `AbilityTask_WaitEvent`, or formally narrow the in-tree task contract.
- [ ] Implement `AbilityTask_WaitProjectileCollision`, or formally narrow the in-tree task contract.
- [ ] Add deterministic positive and blocked/timeout tests for each shipped task backend.
- [ ] Extend battle/map integration coverage for task-driven abilities.
- [ ] Reconfirm `activeCondition` and `passiveCondition` are intentionally unsupported or add a bounded evaluator.
- [ ] Update `GAF_CLOSURE_SIGNOFF.md`, readiness records, and release matrix.

### TD-2026-04-23-09 - Presentation And Visual Validation Are Bounded, Not Broad

**Priority:** P2
**Evidence:** `presentation_runtime` remains `PARTIAL`; its current claim is bounded OpenGL-backed proof with shell-owned MapScene/MenuScene evidence, not cross-backend parity. `profile_arena.cpp` remains the only explicit CMake completeness whitelist entry.

**Next step:** Treat cross-backend breadth as visual-regression backlog, and either register/archive `profile_arena.cpp` or keep its standalone status explicitly documented.

**Checklist:**

- [ ] Decide whether `profile_arena.cpp` should be registered, moved to tools, or left as an explicit standalone exception.
- [ ] Add/update documentation for that decision.
- [ ] Add at least one non-OpenGL backend parity plan item or explicitly mark it out of current scope.
- [ ] Expand renderer-backed visual coverage only where the backend can actually execute.
- [ ] Re-run presentation gate and renderer-backed checks on a non-headless build before status changes.

### TD-2026-04-23-10 - Complexity Hot Spots Need Refactoring Budget

**Priority:** P3
**Evidence:** Largest active source/test files by line count:

| Lines | File |
|---:|---|
| 4,559 | `tests/compat/test_compat_plugin_failure_diagnostics.cpp` |
| 3,267 | `tests/compat/test_compat_plugin_fixtures.cpp` |
| 3,113 | `tests/unit/test_diagnostics_workspace.cpp` |
| 2,844 | `runtimes/compat_js/window_compat.cpp` |
| 2,558 | `runtimes/compat_js/plugin_manager.cpp` |
| 2,532 | `editor/diagnostics/diagnostics_workspace.cpp` |
| 2,486 | `tools/audit/urpg_project_audit.cpp` |
| 2,203 | `runtimes/compat_js/battle_manager.cpp` |

These are understandable accumulation points, but they slow review and make domain-specific regression ownership harder.

**Next step:** Continue the existing TD-08 split plan. Start with tests: split compat diagnostics and fixture tests by failure domain before splitting runtime code.

**Checklist:**

- [ ] Split `tests/compat/test_compat_plugin_failure_diagnostics.cpp` by failure domain.
- [ ] Split `tests/compat/test_compat_plugin_fixtures.cpp` by fixture family or scenario type.
- [ ] Split `tests/unit/test_diagnostics_workspace.cpp` by diagnostics tab/domain.
- [ ] Extract domain helpers from `tools/audit/urpg_project_audit.cpp`.
- [ ] Split `window_compat.cpp`, `plugin_manager.cpp`, and `battle_manager.cpp` only after test splits lower review risk.
- [ ] Keep behavior unchanged during splitting and verify with focused test filters.

### TD-2026-04-23-11 - Product Feature Coverage Gaps Remain Outside The Core Debt List

**Priority:** P2
**Evidence:** The readiness and template matrices still list feature/functionality gaps not fully covered by the first ten audit findings:

- `governance_foundation`: CLI diagnostics and editor panel output are not unified into a single parity export path.
- `character_identity`: authored creation rules, exported-game protagonist persistence, and layered portrait/field/battle composition are not landed.
- `achievement_registry`: trophy export format and platform achievement backend integration are not defined in-tree.
- `accessibility_auditor`: full WYSIWYG contrast ratios from real renderer data are not populated.
- `audio_mix_presets`: real audio-device/backend integration remains outside the current compat-truth harness.
- `mod_registry`: live mod loading, reload, sandboxed script execution, and mod-store integration are not landed.
- `analytics_dispatcher`: telemetry upload, cross-session aggregation, and formal privacy audit workflow are not landed.
- Template lanes still have genre-specific missing functionality: tactics scenario authoring/progression, ARPG movement contracts, monster collection schema/capture, cozy scheduling/social/crafting/economy, metroidvania traversal/map unlocks, and 2.5D art pipeline/authoring/export validation.
- Offline tooling has activation gaps: retrieval tooling is partially present, but the roadmap still calls for first approved FAISS, SAM/SAM2, and Demucs/Encodec production lanes to feed stable exported artifacts.

These gaps are mostly documented honestly as `PARTIAL`, `EXPERIMENTAL`, or `PLANNED`, so this is not status-inflation debt. It is feature coverage debt: the engine still lacks several product-facing capabilities needed for broader template readiness.

**Next step:** Add a dedicated feature-coverage sprint that turns these into explicit backlog owners instead of leaving them distributed across readiness rows.

**Checklist:**

- [ ] Create a feature-coverage backlog grouped by subsystem lane and template lane.
- [ ] Decide which gaps are in the next release scope and which remain explicitly out of scope.
- [ ] For `governance_foundation`, define the single parity export path for CLI diagnostics and editor panel output.
- [ ] For `character_identity`, choose the next creator depth slice: rules, persistence, or compositor.
- [ ] For `achievement_registry`, define the trophy export payload contract even if platform backends remain out-of-tree.
- [ ] For `accessibility_auditor`, define how renderer-derived contrast data flows into the auditor.
- [ ] For `audio_mix_presets`, decide the first real backend/device integration boundary.
- [ ] For `mod_registry`, define the live loading and sandbox boundary before adding mod-store work.
- [ ] For `analytics_dispatcher`, define upload, aggregation, and privacy-review ownership.
- [ ] For template lanes, choose one planned/experimental template to advance with a narrow vertical slice.
- [ ] For offline tooling, decide whether the current retrieval/vision/audio acceptance-contract work is enough for the next milestone or whether production pipelines must be activated.
- [ ] Update readiness docs so each accepted/deferred feature is visible in `mainGaps`, `mainBlockers`, or roadmap scope.

## Sprint Plan

| Sprint | Theme | Primary findings | Goal |
|---|---|---|---|
| `TD-Sprint-01` | Governance Contract Lock | TD-02, TD-03 | Align project schema, localization contract, audit vocabulary, and docs so audit warnings reflect real gaps. |
| `TD-Sprint-02` | Export And Release Truth | TD-04, TD-01 | Add or reconcile the canonical export CLI, then run battle/save human-review promotion workflow. |
| `TD-Sprint-03` | Compat Runtime And Window Fidelity | TD-05, TD-06 | Lock the compat runtime contract and reduce high-use WindowCompat TODO behavior. |
| `TD-Sprint-04` | Runtime Depth And Asset Activation | TD-07, TD-08 | Promote first governed asset lanes and deepen gameplay ability task runtime coverage. |
| `TD-Sprint-05` | Presentation Breadth And Complexity Paydown | TD-09, TD-10 | Resolve the `profile_arena.cpp` exception decision and split oversized compat/diagnostics/audit surfaces. |
| `TD-Sprint-06` | Feature Coverage And Template Depth | TD-11 | Turn scattered subsystem/template/offline-tooling feature gaps into owned vertical slices. |

### TD-Sprint-01 - Governance Contract Lock

- [ ] Close TD-02 project schema/audit vocabulary alignment.
- [ ] Close TD-03 localization contract unification.
- [ ] Re-run `urpg_project_audit --json` and confirm the project-schema/localization artifact warnings are either gone or intentionally renamed.
- [ ] Update `docs/archive/planning/TECHNICAL_DEBT_ACTION_PLAN.md`, `docs/audits/PROJECT_AUDIT.md`, and `docs/status/PROGRAM_COMPLETION_STATUS.md`.

### TD-Sprint-02 - Export And Release Truth

- [ ] Close TD-04 canonical export CLI decision and implementation/documentation.
- [ ] Re-run export packager/validator focused tests.
- [ ] Run battle/save signoff review steps from TD-01.
- [ ] Re-run `urpg_project_audit --json` and record release blocker count after review.
- [ ] Update readiness only for human-approved lanes.

### TD-Sprint-03 - Compat Runtime And Window Fidelity

- [ ] Close TD-05 compat runtime contract decision.
- [ ] Close the highest-use TD-06 WindowCompat TODOs or narrow their contracts explicitly.
- [ ] Re-run focused QuickJS/plugin/window compat tests.
- [ ] Re-run weekly compat discovery or document why it was deferred.
- [ ] Update compat readiness and signoff docs.

### TD-Sprint-04 - Runtime Depth And Asset Activation

- [ ] Close the first visual and audio promotion path from TD-07.
- [ ] Add WYSIWYG smoke proof for promoted assets.
- [ ] Implement or formally narrow at least one missing gameplay ability task from TD-08.
- [ ] Re-run ability, asset, export, and relevant editor tests.
- [ ] Refresh `imports/reports/asset_intake/source_capture_status.json`.

### TD-Sprint-05 - Presentation Breadth And Complexity Paydown

- [x] Resolve the `profile_arena.cpp` registration/archive/exception decision from TD-09.
- [x] Expand visual-regression breadth only where the backend is real.
- [x] Split the first two oversized compat test files from TD-10.
- [x] Split or helper-extract one audit/diagnostics production file after tests are separated.
- [x] Re-run CMake completeness, focused refactor test filters, and `ctest --preset dev-all -N`.

### TD-Sprint-06 - Feature Coverage And Template Depth

- [ ] Convert TD-11 into a grouped backlog with owners for subsystem, template, and offline-tooling lanes.
- [ ] Select one product-facing vertical slice to advance first.
- [ ] Add or update one spec/readiness row for that chosen slice.
- [ ] Add at least one runtime/editor/export or tooling artifact that proves the slice is real.
- [ ] Add focused tests or acceptance-contract checks for the slice.
- [ ] Re-run `urpg_project_audit --json` and confirm the new scope is represented without optimistic promotion language.

## Recommended Action Order

1. Close `TD-S02` project schema/audit alignment. It reduces false-positive noise and sharpens all later audits.
2. Close `TD-S03` localization contract unification. Decide whether the catalog tooling is real scope or stale audit expectation.
3. Close `TD-S04` export CLI/hardening contract. A canonical command matters more than another internal helper.
4. Run human review for battle/save signoffs and update readiness only if accepted.
5. Tackle `TD-S07` WindowCompat fidelity, because it is the highest-value remaining plugin-facing implementation gap.
6. Start file-splitting with compat test fixtures and `urpg_project_audit.cpp`, keeping behavior unchanged.
7. Run `TD-Sprint-06` before any broader template-readiness claims so missing feature lanes are owned instead of implied.

## Positive Signals

- No active known-break waivers.
- CMake completeness passes.
- Test discovery is broad and current at 1,220 registered tests.
- `.env*` files are present locally but not tracked.
- The current debt story is mostly honest: several major gaps are explicitly marked `PARTIAL` instead of being hidden behind optimistic `READY` claims.

## Residual Audit Risk

This audit is inspection-heavy. The next full cycle should add:

- full `ctest --preset dev-all --output-on-failure`
- `.\tools\ci\run_local_gates.ps1`
- warnings-as-errors preset validation
- ASan/UBSan or platform-equivalent sanitizer run
- renderer-backed snapshot execution on a non-headless OpenGL build
- export smoke execution for every supported target contract
