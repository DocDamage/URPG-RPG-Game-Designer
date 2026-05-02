# 100 Percent Completion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the documented future/deferred scope so URPG can truthfully claim 100% completion for the current native-first program scope, then cut a verified release tag.

**Architecture:** Treat `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/release/RELEASE_READINESS_MATRIX.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`, and `content/readiness/readiness_status.json` as the completion contract. Each partial lane must either become `READY` with runtime/editor/schema/diagnostics/tests/docs evidence, or be removed from the claimed 100% product scope with an explicit owner decision and release-safe wording.

**Tech Stack:** C++20, CMake/Ninja, Catch2, ImGui/headless editor snapshots, OpenGL/headless render paths, QuickJS, PowerShell CI scripts, nlohmann/json, CPack, GitHub Actions, optional offline Python tooling isolated under `tools/`.

**Definition of 100% for this plan:** All subsystem rows in `docs/release/RELEASE_READINESS_MATRIX.md` and `content/readiness/readiness_status.json` that remain in the claimed program scope are `READY`; all template rows in `content/readiness/readiness_status.json` that remain advertised are `READY`; release-candidate gates pass on the exact commit to tag; public release exits are recorded; an annotated release or prerelease tag exists.

---

## File Structure

These are the main files and areas this plan expects to touch. Keep each task scoped to its lane and update the readiness docs in the same patch as the implementation.

- `content/readiness/readiness_status.json`: canonical machine-readable subsystem/template status. Update only when evidence exists.
- `docs/release/RELEASE_READINESS_MATRIX.md`: human-readable subsystem status table.
- `docs/APP_RELEASE_READINESS_MATRIX.md`: app-level release gate table.
- `docs/PROGRAM_COMPLETION_STATUS.md`: program completion summary and remaining work.
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`: roadmap alignment, only after implementation changes the plan truth.
- `tools/ci/truth_reconciler.ps1`: readiness/doc truth enforcement.
- `tools/ci/run_local_gates.ps1`: full local gate.
- `tools/ci/run_release_candidate_gate.ps1`: final release-candidate gate.
- `engine/core/render/`, `engine/core/presentation/`, `tests/snapshot/`, `tools/visual/`: presentation and visual regression breadth.
- `engine/core/ability/`, `editor/ability/`, `content/schemas/gameplay_ability*.schema.json`, `tests/unit/test_*ability*.cpp`: gameplay ability product-depth closure.
- `engine/core/governance/`, `tools/ci/check_*governance*.ps1`, `tests/unit/test_project_audit.cpp`: release-signoff enforcement.
- `editor/character/`, `engine/core/character/`, `engine/core/assets/`, `tools/assets/`, `tests/unit/test_character*.cpp`: character appearance import and part-library management.
- `editor/accessibility/`, `engine/core/accessibility/`, `tests/unit/test_accessibility*.cpp`: complete UI contrast coverage.
- `engine/core/audio/`, `editor/audio/`, `tests/unit/test_audio*.cpp`: backend/device audio matrix evidence.
- `engine/core/export/`, `tools/pack/`, `cmake/packaging.cmake`, `tools/ci/check_*smoke.ps1`: platform packaging, signing, notarization, and artifact policy.
- `engine/core/mod/`, `editor/mod/`, `content/schemas/mod*.schema.json`, `tests/unit/test_mod*.cpp`: external mod marketplace boundaries.
- `engine/core/analytics/`, `editor/analytics/`, `docs/release/LEGAL_REVIEW_SIGNOFF.md`: production privacy review evidence and analytics release posture.
- `tools/retrieval/`, `tools/vision/`, `tools/audio/`: offline retrieval, segmentation, and audio tooling lanes.
- `docs/asset_intake/`, `imports/manifests/`, `imports/reports/asset_intake/`, `resources/`: asset promotion and governed release bundles.

---

## Phase 0 - Baseline Audit And Scope Lock

### Task P0-001: Generate the exact remaining-work inventory

**Files:**
- Create: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Inspect: `content/readiness/readiness_status.json`, `docs/release/RELEASE_READINESS_MATRIX.md`, `docs/APP_RELEASE_READINESS_MATRIX.md`

- [ ] **Step 1: Capture current partial rows**

Run:

```powershell
$r = Get-Content content/readiness/readiness_status.json -Raw | ConvertFrom-Json
$r.subsystems | Where-Object status -eq 'PARTIAL' | Select-Object id,status,mainGaps | Format-List
$r.templates | Where-Object status -ne 'READY' | Select-Object id,status,mainBlockers | Format-List
```

Expected: the output lists `presentation_runtime`, `gameplay_ability_framework`, `governance_foundation`, `character_identity`, `achievement_registry`, `accessibility_auditor`, `visual_regression_harness`, `audio_mix_presets`, `export_validator`, `mod_registry`, `analytics_dispatcher`, plus any non-ready templates.

- [ ] **Step 2: Create the inventory document**

Create `docs/release/100_PERCENT_COMPLETION_INVENTORY.md` with one table containing: lane id, current status, exact blocker, required implementation evidence, required verification command, and release claim impact.

- [ ] **Step 3: Add the inventory link to program status**

Add a short paragraph near the top of `docs/PROGRAM_COMPLETION_STATUS.md` pointing to `docs/release/100_PERCENT_COMPLETION_INVENTORY.md` as the active remaining-work inventory for the 100% completion push.

- [ ] **Step 4: Verify documentation discoverability**

Run:

```powershell
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

Expected: pass, or fail only on unrelated pre-existing documentation issues that are recorded before continuing.

### Task P0-002: Decide claim scope for platform/vendor-dependent lanes

**Files:**
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `content/readiness/readiness_status.json`
- Inspect: `docs/release/LEGAL_REVIEW_SIGNOFF.md`, `docs/release/RELEASE_PACKAGING.md`

- [ ] **Step 1: Classify each external dependency**

For `achievement_registry`, `mod_registry`, `analytics_dispatcher`, and `export_validator`, classify each remaining blocker as one of:

```text
engine-owned
project-configuration
release-owner-review
external-service-credential
platform-policy
```

- [ ] **Step 2: Convert non-engine blockers into explicit release boundaries**

For blockers that require proprietary store SDKs, marketplace accounts, payment processors, review services, signing certificates, notarization accounts, or legal counsel, record the boundary as project configuration rather than unfinished engine code when the engine already has validated profile/config seams.

- [ ] **Step 3: Keep engine-owned gaps in scope**

Do not promote any row to `READY` if its remaining gap is engine-owned, such as missing runtime sequencing, missing UI contrast ingestion, missing renderer backend coverage, missing artifact policy enforcement, or missing release-signoff enforcement.

- [ ] **Step 4: Run truth reconciliation**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
```

Expected: pass after the docs and JSON agree on which lanes are engine-owned versus project-configuration boundaries.

---

## Phase 1 - Visual And Presentation Completion

### Task P1-001: Expand visual capture backends beyond the bounded OpenGL path

**Files:**
- Modify: `engine/core/render/`
- Modify: `engine/core/presentation/`
- Modify: `tools/visual/`
- Modify: `tests/snapshot/`
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`

- [ ] **Step 1: Write failing backend-coverage tests**

Add snapshot tests that require the generic capture API to execute at least these backend modes:

```text
opengl
headless
software_or_reference
```

Each test must assert that capture returns non-empty frame dimensions, command count, stable hash, and a backend id matching the requested backend.

- [ ] **Step 2: Run the focused visual tests**

Run:

```powershell
ctest --preset dev-snapshot --output-on-failure
```

Expected: fail because non-OpenGL backend capture breadth is incomplete or explicitly rejected.

- [ ] **Step 3: Implement the backend capture adapters**

Implement the missing adapters using existing render command structures. Keep the headless/reference path deterministic and reject unsupported operations with explicit diagnostics rather than silent empty frames.

- [ ] **Step 4: Add command-stream parity assertions**

For MapScene, MenuScene, RuntimeTitleScene, RuntimeOptionsScene, BattleScene, and Level Builder preview, assert that each backend sees the same high-level render command families even when pixel output differs.

- [ ] **Step 5: Promote rows only after evidence**

Update `visual_regression_harness` and `presentation_runtime` from `PARTIAL` to `READY` only after the backend coverage tests and presentation gate pass.

- [ ] **Step 6: Verify**

Run:

```powershell
ctest --preset dev-snapshot --output-on-failure
.\tools\ci\run_presentation_gate.ps1
```

Expected: both pass.

### Task P1-002: Broaden shell-owned scene permutation goldens

**Files:**
- Modify: `tests/snapshot/`
- Modify: `content/fixtures/visual_golden_governance.json`
- Modify: `docs/presentation/`

- [ ] **Step 1: Add failing permutation cases**

Add golden tests for these shell-owned permutations:

```text
MapScene: idle, dialogue active, menu transition, missing asset diagnostic
BattleScene: actor cue, enemy cue, reward transition, failure diagnostic
RuntimeTitleScene: no saves, saves available, settings opened
RuntimeOptionsScene: default, high contrast, remapped input
LevelBuilder: empty document, valid document, invalid document diagnostics
```

- [ ] **Step 2: Run visual governance checks**

Run:

```powershell
ctest --preset dev-snapshot --output-on-failure
.\tools\ci\run_presentation_gate.ps1
```

Expected: fail until new goldens are generated, governed, and approved through the existing golden workflow.

- [ ] **Step 3: Generate governed goldens**

Use the existing visual regression harness to create the goldens. Add entries to `content/fixtures/visual_golden_governance.json` for any large baselines with reason, review strategy, and owner.

- [ ] **Step 4: Verify**

Run the same commands from Step 2. Expected: pass with all new permutations covered.

---

## Phase 2 - Gameplay Ability Framework Completion

### Task P2-001: Implement full authored task-graph runtime sequencing

**Files:**
- Modify: `engine/core/ability/ability_task.h`
- Modify: `engine/core/ability/gameplay_ability.h`
- Modify: `engine/core/ability/authored_ability_asset.h`
- Modify: `engine/core/scene/battle_scene.*`
- Modify: `engine/core/scene/map_scene.*`
- Test: `tests/unit/test_gameplay_ability*.cpp`
- Test: `tests/unit/test_scene_manager.cpp`

- [ ] **Step 1: Write failing task-graph tests**

Add tests for a saved authored ability containing:

```text
sequence: cost -> wait_input -> projectile_collision -> apply_effect -> cooldown
branch: condition_true -> apply_effect_a, condition_false -> apply_effect_b
parallel: wait_event and delay complete before final effect
cancel: input cancel records cancellation and skips cooldown if configured
```

- [ ] **Step 2: Run focused ability tests**

Run:

```powershell
ctest --preset dev-all -R "Ability|ability|BattleScene|MapScene" --output-on-failure
```

Expected: fail on missing runtime task-graph sequencing.

- [ ] **Step 3: Implement deterministic task execution**

Add a runtime task-graph executor that consumes the existing authored JSON/task composition data. It must record ordered diagnostics for started, waiting, completed, branched, cancelled, and failed task nodes.

- [ ] **Step 4: Wire battle and map activation**

Route authored battle/map abilities through the same executor. Preserve existing bounded active-condition behavior and keep arbitrary script strings rejected with explicit unsupported diagnostics.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-all -R "Ability|ability|BattleScene|MapScene" --output-on-failure
```

Expected: pass.

### Task P2-002: Complete creator-facing ability authoring workflow

**Files:**
- Modify: `editor/ability/ability_inspector_panel.*`
- Modify: `editor/diagnostics/diagnostics_workspace.*`
- Modify: `content/schemas/gameplay_ability*.schema.json`
- Test: `tests/unit/test_ability_inspector*.cpp`
- Test: `tests/unit/test_diagnostics_workspace.cpp`

- [ ] **Step 1: Add failing editor snapshot tests**

Add snapshot tests proving creators can author, preview, validate, save, load, apply, and revert task-graph abilities without editing JSON by hand.

- [ ] **Step 2: Run focused editor tests**

Run:

```powershell
ctest --preset dev-all -R "Ability|DiagnosticsWorkspace|ability inspector" --output-on-failure
```

Expected: fail until the task-graph authoring controls and previews are exposed.

- [ ] **Step 3: Implement editor controls**

Expose task node rows, branch rows, dependency ordering, validation messages, preview execution, save/load/apply, and revert state through deterministic panel snapshots.

- [ ] **Step 4: Update schema examples**

Add one valid sequence, one valid branch, one valid parallel graph, and one invalid cycle fixture. The invalid fixture must produce deterministic validation diagnostics.

- [ ] **Step 5: Promote readiness**

After runtime and editor tests pass, update `gameplay_ability_framework` to `READY` in `content/readiness/readiness_status.json` and `docs/release/RELEASE_READINESS_MATRIX.md`.

---

## Phase 3 - Governance And Release Signoff Enforcement

### Task P3-001: Enforce release-signoff completeness in ProjectAudit

**Files:**
- Modify: `engine/core/project/project_audit.*`
- Modify: `tools/ci/check_release_readiness.ps1`
- Modify: `tools/ci/truth_reconciler.ps1`
- Test: `tests/unit/test_project_audit.cpp`
- Test: `content/fixtures/project_governance_fixture.json`

- [ ] **Step 1: Write failing audit tests**

Add tests where a subsystem claims `READY` but lacks one of: signoff artifact, reviewer, reviewed date, verification command, evidence command result, docs alignment. Expected audit result: `releaseBlockerCount > 0`.

- [ ] **Step 2: Run project audit tests**

Run:

```powershell
ctest --preset dev-project-audit --output-on-failure
```

Expected: fail until enforcement exists.

- [ ] **Step 3: Implement audit enforcement**

Make `urpg_project_audit.exe --json` emit blocker rows for missing or stale signoff evidence on claimed `READY` lanes.

- [ ] **Step 4: Wire CI script failure**

Make release-readiness scripts fail when ProjectAudit reports signoff blockers for claimed `READY` rows.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-project-audit --output-on-failure
.\build\dev-ninja-debug\urpg_project_audit.exe --json
.\tools\ci\check_release_readiness.ps1
```

Expected: tests pass and audit JSON reports `releaseBlockerCount: 0` only when all claimed ready rows have evidence.

### Task P3-002: Promote governance foundation after enforcement passes

**Files:**
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`

- [ ] **Step 1: Update status rows**

Promote `governance_foundation` to `READY` only after Task P3-001 verification passes.

- [ ] **Step 2: Run truth checks**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

Expected: both pass.

---

## Phase 4 - Character Identity And Asset Authoring Completion

### Task P4-001: Build appearance-part import tooling

**Files:**
- Modify: `engine/core/assets/`
- Modify: `engine/core/character/`
- Modify: `editor/character/`
- Modify: `tools/assets/`
- Modify: `content/schemas/character*.schema.json`
- Test: `tests/unit/test_character_creator*.cpp`
- Test: `tests/unit/test_asset_library*.cpp`

- [ ] **Step 1: Write failing import tests**

Add tests for creator-supplied portrait, field, and battle appearance parts. Each test must verify source metadata, normalized asset id, category, dimensions, runtime-ready flag, attribution state, and blocked reason when metadata is incomplete.

- [ ] **Step 2: Run focused tests**

Run:

```powershell
ctest --preset dev-all -R "Character|character|AssetLibrary|asset library" --output-on-failure
```

Expected: fail until import tooling exists.

- [ ] **Step 3: Implement import normalization**

Implement deterministic import records that copy or reference approved files under governed promoted-asset locations, never raw intake paths. Record attribution, source, license status, preview dimensions, and runtime eligibility.

- [ ] **Step 4: Expose part-library management**

Add editor snapshot rows for import preview, accept, reject, archive, assign to portrait/field/battle layers, and validation diagnostics.

- [ ] **Step 5: Verify**

Run the command from Step 2. Expected: pass.

### Task P4-002: Promote character identity readiness

**Files:**
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/asset_intake/ASSET_CATEGORY_GAPS.md`

- [ ] **Step 1: Update asset gap docs**

Record which appearance categories are now governed and which raw/source categories remain quarantine-only.

- [ ] **Step 2: Promote status**

Promote `character_identity` to `READY` only after the import, library management, persistence, and runtime preview tests pass.

- [ ] **Step 3: Verify**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
ctest --preset dev-all -R "Character|character|AssetLibrary|asset library" --output-on-failure
```

Expected: both pass.

---

## Phase 5 - Accessibility, Audio, And Template Bars

### Task P5-001: Complete renderer-derived contrast coverage across release UI

**Files:**
- Modify: `engine/core/accessibility/`
- Modify: `editor/accessibility/`
- Modify: `tests/unit/test_accessibility*.cpp`
- Modify: `tools/ci/check_accessibility_governance.ps1`

- [ ] **Step 1: Write failing coverage tests**

Add a test that enumerates every release top-level panel and fails unless the accessibility auditor ingests text/rect contrast evidence for that panel or records a justified no-text/no-rect exemption.

- [ ] **Step 2: Run accessibility tests**

Run:

```powershell
ctest --preset dev-all -R "accessibility|Accessibility" --output-on-failure
.\tools\ci\check_accessibility_governance.ps1
```

Expected: fail until every release UI surface has contrast evidence or a valid exemption.

- [ ] **Step 3: Implement missing adapters**

Add adapters for panels and render-command surfaces that lack contrast evidence. Exempt only surfaces that render no text and no contrast-relevant UI primitives.

- [ ] **Step 4: Verify and promote**

Run the commands from Step 2. Promote `accessibility_auditor` to `READY` after pass.

### Task P5-002: Add audio backend/device matrix evidence

**Files:**
- Modify: `engine/core/audio/`
- Modify: `editor/audio/`
- Modify: `tools/ci/check_audio_governance.ps1`
- Test: `tests/unit/test_audio*.cpp`

- [ ] **Step 1: Define matrix fixtures**

Add fixtures for at least:

```text
null backend
SDL backend available
SDL backend unavailable
missing output device
stereo output
muted release fallback
```

- [ ] **Step 2: Add failing tests**

Tests must prove audio preset application reports backend/device status, does not falsely mark failed playback active, and records release-safe muted fallback where no device exists.

- [ ] **Step 3: Verify failure**

Run:

```powershell
ctest --preset dev-all -R "audio|Audio" --output-on-failure
.\tools\ci\check_audio_governance.ps1
```

Expected: fail until matrix evidence is wired.

- [ ] **Step 4: Implement diagnostics and editor exposure**

Expose backend id, device state, preset application result, fallback policy, and last playback diagnostic in `AudioMixPanel` snapshots.

- [ ] **Step 5: Verify and promote**

Run Step 3 commands. Promote `audio_mix_presets` to `READY` after pass.

### Task P5-003: Promote template rows blocked by cross-cutting bars

**Files:**
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/templates/`
- Modify: `content/templates/`
- Modify: `tools/ci/check_template_claims.ps1`

- [ ] **Step 1: Recompute template bars**

After accessibility and audio are ready, rerun template claim checks and identify templates still blocked by performance or dedicated governance.

- [ ] **Step 2: Add missing performance evidence**

For each non-ready advertised template, add or update bounded starter performance budgets and validation evidence.

- [ ] **Step 3: Verify**

Run:

```powershell
.\tools\ci\check_template_claims.ps1
.\tools\ci\truth_reconciler.ps1
```

Expected: all advertised template rows are `READY` or explicitly removed from advertised scope.

---

## Phase 6 - Export, Platform Packaging, And Release Artifact Policy

### Task P6-001: Enforce external asset discovery and promoted bundle boundaries

**Files:**
- Modify: `engine/core/export/`
- Modify: `tools/pack/`
- Modify: `tools/ci/check_release_required_assets.ps1`
- Modify: `docs/asset_intake/ASSET_PROMOTION_GUIDE.md`
- Test: `tests/unit/test_export*.cpp`

- [ ] **Step 1: Write failing export tests**

Add tests proving export rejects raw intake paths, unpromoted external store paths, missing attribution, unresolved LFS pointers, and source-only files.

- [ ] **Step 2: Run export tests**

Run:

```powershell
ctest --preset dev-export --output-on-failure
.\tools\ci\check_release_required_assets.ps1
```

Expected: fail until broader external discovery and rejection diagnostics are complete.

- [ ] **Step 3: Implement discovery diagnostics**

Add explicit diagnostics for discovered-but-unpromoted assets, source-only paths, missing attribution, and external-store references that require project configuration.

- [ ] **Step 4: Verify**

Run Step 2 commands. Expected: pass.

### Task P6-002: Add signing/notarization profile enforcement

**Files:**
- Modify: `engine/core/export/`
- Modify: `tools/pack/pack_cli.cpp`
- Modify: `tools/ci/check_package_smoke.ps1`
- Modify: `docs/release/RELEASE_PACKAGING.md`
- Test: `tests/unit/test_export*.cpp`

- [ ] **Step 1: Define profile schema**

Add or extend export profile data with:

```text
target platform
signing mode
certificate reference
notarization mode
release artifact policy
owner approval field
```

- [ ] **Step 2: Add failing tests**

Tests must prove release mode fails without required signing/notarization profile data for platforms that require it, while dev/package-smoke mode remains available with explicit non-production metadata.

- [ ] **Step 3: Run export/package tests**

Run:

```powershell
ctest --preset dev-export --output-on-failure
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

Expected: fail until release profile enforcement exists.

- [ ] **Step 4: Implement enforcement**

Make pack/export code refuse to label artifacts as release-ready when signing/notarization prerequisites are absent. Do not fabricate credentials.

- [ ] **Step 5: Verify and promote**

Run Step 3 commands. Promote `export_validator` to `READY` only when release artifact policy is enforced and docs match behavior.

---

## Phase 7 - Mod, Achievement, Analytics, And External Service Boundaries

### Task P7-001: Convert proprietary service requirements into validated provider profiles

**Files:**
- Modify: `engine/core/achievement/`
- Modify: `engine/core/mod/`
- Modify: `engine/core/analytics/`
- Modify: `content/schemas/*profile*.schema.json`
- Modify: `docs/release/RELEASE_PACKAGING.md`
- Test: `tests/unit/test_achievement*.cpp`
- Test: `tests/unit/test_mod*.cpp`
- Test: `tests/unit/test_analytics*.cpp`

- [ ] **Step 1: Add profile validation tests**

Add tests for missing credentials, unsupported provider, dry-run provider, command provider, and reviewed production provider for achievements, mod marketplace, and analytics endpoints.

- [ ] **Step 2: Run focused tests**

Run:

```powershell
ctest --preset dev-all -R "Achievement|achievement|Mod|mod|Analytics|analytics" --output-on-failure
```

Expected: fail until provider profile validation is consistent across lanes.

- [ ] **Step 3: Implement shared validation vocabulary**

Use consistent profile statuses:

```text
disabled
dry_run
configured_unreviewed
configured_reviewed
missing_credentials
unsupported_provider
```

- [ ] **Step 4: Expose editor diagnostics**

Panel snapshots must show profile status, credential source category, last test result, review status, and whether release packaging can include that provider.

- [ ] **Step 5: Verify**

Run Step 2 command. Expected: pass.

### Task P7-002: Promote external-service lanes by scoped evidence

**Files:**
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`

- [ ] **Step 1: Promote engine-owned completion**

Promote `achievement_registry`, `mod_registry`, and `analytics_dispatcher` to `READY` when the engine validates provider profiles and docs state that proprietary account credentials remain project configuration.

- [ ] **Step 2: Keep legal/privacy truth explicit**

For analytics, keep qualified legal review separate from engine readiness. If owner waiver remains the decision, `docs/release/LEGAL_REVIEW_SIGNOFF.md` must continue to state that it is not counsel approval.

- [ ] **Step 3: Verify**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
ctest --preset dev-all -R "Achievement|achievement|Mod|mod|Analytics|analytics" --output-on-failure
```

Expected: pass.

---

## Phase 8 - Offline Tooling Boundary

### Task P8-001: Add FAISS-compatible retrieval tooling without runtime dependency creep

**Files:**
- Create: `tools/retrieval/README.md`
- Create: `tools/retrieval/build_index.py`
- Create: `tools/retrieval/query_index.py`
- Create: `tools/retrieval/requirements.txt`
- Create: `content/schemas/retrieval_index_manifest.schema.json`
- Test: `tests/unit/test_offline_tool_manifests.cpp`

- [ ] **Step 1: Write manifest tests**

Add C++ tests that validate retrieval manifest JSON examples without importing Python dependencies into runtime code.

- [ ] **Step 2: Implement scripts**

`build_index.py` must accept source paths, emit chunk manifests with stable ids, and write index metadata. `query_index.py` must load the manifest and return source path, chunk id, score, and excerpt.

- [ ] **Step 3: Verify boundary**

Run:

```powershell
ctest --preset dev-all -R "offline|manifest|retrieval" --output-on-failure
```

Expected: runtime tests validate manifests; Python dependency installation is documented but not required for runtime tests.

### Task P8-002: Add SAM/SAM2-compatible segmentation manifest lane

**Files:**
- Create: `tools/vision/README.md`
- Create: `tools/vision/segment_assets.py`
- Create: `tools/vision/requirements.txt`
- Create: `content/schemas/segmentation_manifest.schema.json`
- Test: `tests/unit/test_offline_tool_manifests.cpp`

- [ ] **Step 1: Add manifest fixtures**

Create positive and negative segmentation manifest fixtures covering source image, mask output, cutout output, manual override, reviewed state, and rerun protection.

- [ ] **Step 2: Implement script contract**

`segment_assets.py` must preserve manual override files and emit skipped/reused/generated rows.

- [ ] **Step 3: Verify**

Run:

```powershell
ctest --preset dev-all -R "offline|manifest|segmentation" --output-on-failure
```

Expected: pass.

### Task P8-003: Add audio separation/compression manifest lane

**Files:**
- Create: `tools/audio/README.md`
- Create: `tools/audio/process_audio_assets.py`
- Create: `tools/audio/requirements.txt`
- Create: `content/schemas/audio_tool_manifest.schema.json`
- Test: `tests/unit/test_offline_tool_manifests.cpp`

- [ ] **Step 1: Add manifest tests**

Add fixtures for Demucs-style stems, Encodec-style compression experiments, generated prototype markers, source attribution, reviewed status, and release eligibility.

- [ ] **Step 2: Implement script contract**

`process_audio_assets.py` must emit manifests and mark generated/prototype outputs as non-release until reviewed and promoted.

- [ ] **Step 3: Verify**

Run:

```powershell
ctest --preset dev-all -R "offline|manifest|audio" --output-on-failure
```

Expected: pass.

---

## Phase 9 - Asset Promotion And Release Content Completion

### Task P9-001: Promote governed release bundles for remaining asset categories

**Files:**
- Modify: `docs/asset_intake/ASSET_CATEGORY_GAPS.md`
- Modify: `docs/asset_intake/ASSET_SOURCE_REGISTRY.md`
- Modify: `imports/manifests/asset_bundles/*.json`
- Modify: `tools/ci/check_release_required_assets.ps1`
- Modify: `resources/`

- [ ] **Step 1: Select categories for the 100% claim**

Choose the exact asset categories that must be release-ready for the advertised product. At minimum, review UI sounds, prototype sprites, UI frames/chrome, VFX sheets, environmental SFX, BGM, and cohesive UI skin.

- [ ] **Step 2: Promote only cleared assets**

For each selected category, create a bundle manifest with source, attribution, license status, release eligibility, checksum, and install/package destination.

- [ ] **Step 3: Add asset gate tests**

Extend `check_release_required_assets.ps1` so each selected category must have hydrated files, no LFS pointer, attribution, manifest membership, and package inclusion evidence.

- [ ] **Step 4: Verify**

Run:

```powershell
.\tools\ci\check_release_required_assets.ps1
ctest --preset dev-all -R "AssetLoader|Runtime map asset|preflight|asset" --output-on-failure
```

Expected: pass.

### Task P9-002: Update asset docs and release package evidence

**Files:**
- Modify: `docs/asset_intake/ASSET_CATEGORY_GAPS.md`
- Modify: `docs/release/RELEASE_PACKAGING.md`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`

- [ ] **Step 1: Remove stale fixture-only claims**

For categories promoted in Task P9-001, change status from `fixture_only` or `partial` to the exact new status supported by bundle evidence.

- [ ] **Step 2: Keep unpromoted categories out of release claims**

For categories not selected, state they remain raw/quarantine/future-source scope and are not part of the 100% claim.

- [ ] **Step 3: Verify docs**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

Expected: pass.

---

## Phase 10 - Final Readiness Promotion And Release Tag

### Task P10-001: Promote all completed subsystem and template rows

**Files:**
- Modify: `content/readiness/readiness_status.json`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`

- [ ] **Step 1: Run the full readiness query**

Run:

```powershell
$r = Get-Content content/readiness/readiness_status.json -Raw | ConvertFrom-Json
$r.subsystems | Group-Object status | Select-Object Name,Count
$r.templates | Group-Object status | Select-Object Name,Count
```

Expected before editing: no unexpected status values; any remaining non-ready row has an explicit owner decision to remove it from advertised scope.

- [ ] **Step 2: Update status docs**

Update all canonical docs so their claims match the machine-readable readiness JSON and the completed verification evidence.

- [ ] **Step 3: Verify truth**

Run:

```powershell
.\tools\ci\truth_reconciler.ps1
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

Expected: pass.

### Task P10-002: Run final local release gates

**Files:**
- Modify only if gates expose real bugs.
- Inspect: `docs/release/AAA_RELEASE_READINESS_REPORT.md`

- [ ] **Step 1: Run full local gates**

Run:

```powershell
pre-commit run --all-files
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1
```

Expected: all pass on the exact commit intended for release.

- [ ] **Step 2: Record gate evidence**

Update `docs/release/AAA_RELEASE_READINESS_REPORT.md` with exact commit SHA, commands, date, result, and any remote workflow URL.

- [ ] **Step 3: Run final status check**

Run:

```powershell
git status --short --branch
git rev-parse HEAD
git tag -l
```

Expected: clean tree after committing report updates; no release tag exists until the release owner approves tagging.

### Task P10-003: Create annotated release or prerelease tag

**Files:**
- No source file changes.
- Git action: annotated tag.

- [ ] **Step 1: Confirm release owner approval**

Record the release owner's final approval in `docs/release/AAA_RELEASE_READINESS_REPORT.md` before tagging.

- [ ] **Step 2: Create tag**

Run one of:

```powershell
git tag -a v0.1.0-rc.1 -m "URPG 0.1.0 release candidate 1"
git tag -a v0.1.0 -m "URPG 0.1.0"
```

Use the prerelease tag if any public-distribution confidence remains release-owner-waived rather than fully approved.

- [ ] **Step 3: Verify tag**

Run:

```powershell
git tag -l
git show --stat --oneline --decorate -1
```

Expected: tag points at the exact commit that passed final gates.

---

## Required Verification Summary

Run these commands before claiming the plan is complete:

```powershell
pre-commit run --all-files
ctest --preset dev-pr --output-on-failure
ctest --preset dev-snapshot --output-on-failure
ctest --preset dev-export --output-on-failure
ctest --preset dev-project-audit --output-on-failure
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1
.\tools\ci\truth_reconciler.ps1
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

Expected final state:

```text
All advertised subsystem rows: READY
All advertised template rows: READY
Project audit releaseBlockerCount: 0
Working tree: clean
Release-candidate gate: pass
Presentation gate: pass
Local gates: pass
Annotated release or prerelease tag: present
```

## Self-Review Notes

- Spec coverage: This plan covers every partial subsystem row found in `content/readiness/readiness_status.json`, the non-ready template rows, offline tooling backlog, asset promotion backlog, and final release tagging.
- Placeholder scan: The plan intentionally avoids open-ended placeholder steps; platform credentials and legal counsel are classified as release-owner/project-configuration boundaries when not engine-owned.
- Type consistency: Readiness status values are limited to the project's existing vocabulary: `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, `PLANNED`, `VERIFIED`, and release-owner waiver terms already present in canonical docs.
