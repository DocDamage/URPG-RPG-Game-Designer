# Release Protection And MZ Parity Evidence Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish the in-scope release protection lane and turn high-end RPG Maker MZ compatibility from report-only scaffolding into evidence-backed parity workbench coverage, while explicitly deferring cloud sync/live services/store publishing to future updates.

**Architecture:** Keep release protection honest and deterministic: add a real script transform pipeline, upgrade bundle protection metadata to a release-grade authenticated contract with tamper/replay evidence, and keep runtime validation fail-closed. For MZ parity, add repo-owned reference capture descriptors, backend comparison evidence, expanded legal corpus descriptors, and aggregate those into the migration workbench without claiming proprietary RPG Maker runtime parity.

**Tech Stack:** C++20, nlohmann/json, Catch2, CMake/Ninja, existing `engine/core/export`, `engine/core/security`, `engine/core/compat`, `editor/compat`, PowerShell CI gates, and repo-owned fixture JSON.

---

## File Structure

- Create `engine/core/security/script_transform.h`: deterministic script transform API.
- Create `engine/core/security/script_transform.cpp`: whitespace/comment-minifying script transform with manifest hashes.
- Modify `engine/core/tools/export_packager_payload_builder.cpp`: generate transformed script payloads when `obfuscateScripts=true` instead of failing closed.
- Modify `engine/core/tools/export_packager_bundle_writer.cpp`: emit upgraded protection metadata and log wording.
- Modify `engine/core/export/export_bundle_contract.h`: expose upgraded protection constants.
- Modify `engine/core/export/export_bundle_contract.cpp`: validate upgraded bundle protection metadata and preserve existing signed fixture compatibility.
- Modify `tests/unit/test_export_packager.cpp`: replace unsupported script obfuscation expectation with transformed script payload expectation.
- Modify `tests/unit/test_export_packager_bootstrap_cli.cpp`: update protection wording/metadata tests.
- Create `content/compat/mz_reference_captures.schema.json`: schema for repo-owned MZ-style reference captures.
- Create `imports/fixtures/compat/mz_projects/two_map_event_project.json`: expanded legal corpus descriptor.
- Create `imports/fixtures/compat/mz_references/minimal_jrpg_title_capture.json`: repo-owned reference capture descriptor.
- Create `imports/fixtures/compat/mz_references/two_map_event_capture.json`: second repo-owned reference capture descriptor.
- Create `engine/core/compat/mz_parity_evidence.h`: parity evidence aggregation structures.
- Create `engine/core/compat/mz_parity_evidence.cpp`: aggregate visual/backend/corpus evidence into deterministic JSON.
- Modify `editor/compat/mz_migration_workbench_model.h`: expose parity evidence fields.
- Modify `editor/compat/mz_migration_workbench_model.cpp`: include parity evidence in workbench snapshot.
- Create `tests/unit/test_mz_parity_evidence.cpp`: cover reference captures and backend comparison aggregation.
- Modify `tests/unit/test_mz_migration_workbench_model.cpp`: assert workbench renders parity evidence.
- Modify `tools/ci/check_mz_project_corpus.ps1`: validate expanded corpus and reference capture descriptors.
- Modify `docs/PROGRAM_COMPLETION_STATUS.md`: mark cloud/live/store scope future update, record protection/MZ evidence outcome.
- Modify `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`: update blocker rows after implementation.

## Task 1: Deterministic Script Transform

**Files:**
- Create: `engine/core/security/script_transform.h`
- Create: `engine/core/security/script_transform.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/test_export_packager.cpp`

- [x] **Step 1: Write failing script transform export test**

Replace the old unsupported `obfuscateScripts=true` test with a test that runs export with script transform enabled and asserts the bundle contains transformed script metadata.

Run: `.\build\dev-ninja-debug\urpg_export_unit_tests.exe "ExportPackager transforms script payloads when obfuscateScripts is enabled" --reporter compact`

Expected: FAIL because script transform still fails closed.

- [x] **Step 2: Implement script transform API**

Add `ScriptTransformResult TransformScriptForRelease(std::string_view source, std::string_view script_id)` that removes line comments, collapses whitespace outside strings, preserves string literals, and reports `transform_id=urpg_script_minify_v1`.

- [x] **Step 3: Wire transformed script payload**

In `buildBundlePayloads()`, when `obfuscateScripts=true`, add `runtime/scripts/bootstrap.urpg.js` and `runtime/script_transform_manifest.json` payloads instead of returning an error.

- [x] **Step 4: Verify script transform**

Run: `cmake --build --preset dev-debug --target urpg_export_unit_tests; .\build\dev-ninja-debug\urpg_export_unit_tests.exe "ExportPackager transforms script payloads when obfuscateScripts is enabled" --reporter compact`

Expected: PASS.

## Task 2: Release Bundle Protection Contract

**Files:**
- Modify: `engine/core/export/export_bundle_contract.h`
- Modify: `engine/core/export/export_bundle_contract.cpp`
- Modify: `engine/core/tools/export_packager_bundle_writer.cpp`
- Test: `tests/unit/test_export_packager_bootstrap_cli.cpp`
- Test: `tests/unit/test_runtime_bundle_loader.cpp`

- [x] **Step 1: Write failing upgraded protection metadata test**

Assert generated bundles include `protectionMode=authenticated_release_bundle_v1`, `signatureMode=hmac_sha256_bundle_v2`, and `bundleSignatureScope=manifest_payload_target_v2`.

Run: `.\build\dev-ninja-debug\urpg_export_unit_tests.exe "[export][security]" --reporter compact`

Expected: FAIL because the bundle still reports lightweight protection metadata.

- [x] **Step 2: Upgrade bundle constants and validation**

Add `kSignatureModeV2`, accept legacy v1 only for minimal fixtures that omit `protectionMode`, and require V2 bundles to include target-scoped signature metadata.

- [x] **Step 3: Upgrade writer metadata and logs**

Write V2 metadata from the packager, retain existing HMAC-SHA256 implementation, and remove “lightweight” wording from release bundle logs where the V2 contract is active.

- [x] **Step 4: Verify bundle protection**

Run: `cmake --build --preset dev-debug --target urpg_export_unit_tests; .\build\dev-ninja-debug\urpg_export_unit_tests.exe "[runtime_bundle],[export][security]" --reporter compact`

Expected: PASS.

## Task 3: Legal MZ Reference Capture Evidence

**Files:**
- Create: `content/compat/mz_reference_captures.schema.json`
- Create: `imports/fixtures/compat/mz_references/minimal_jrpg_title_capture.json`
- Create: `imports/fixtures/compat/mz_references/two_map_event_capture.json`
- Modify: `imports/fixtures/compat/mz_projects/two_map_event_project.json`
- Modify: `tools/ci/check_mz_project_corpus.ps1`
- Test: `tests/unit/test_mz_project_corpus_schema.cpp`

- [x] **Step 1: Write failing reference schema test**

Extend corpus tests to load `content/compat/mz_reference_captures.schema.json` and both reference descriptors.

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_corpus]" --reporter compact`

Expected: FAIL because the schema/descriptors are missing.

- [x] **Step 2: Add schema and descriptors**

Reference descriptors must require `captureId`, `projectId`, `sceneId`, `backend`, `frameHash`, `dimensions`, `legalUse`, `source`, and `releaseAuthoritative=false`.

- [x] **Step 3: Expand corpus checker**

Validate that reference descriptors are repo-owned/permissive, do not include copyrighted RPG Maker payloads, and keep `releaseAuthoritative=false`.

- [x] **Step 4: Verify corpus evidence**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_corpus]" --reporter compact; .\tools\ci\check_mz_project_corpus.ps1`

Expected: PASS.

## Task 4: MZ Backend Parity Evidence Aggregator

**Files:**
- Create: `engine/core/compat/mz_parity_evidence.h`
- Create: `engine/core/compat/mz_parity_evidence.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/test_mz_parity_evidence.cpp`

- [x] **Step 1: Write failing parity evidence test**

Assert two reference captures plus URPG backend observations produce visual/backend status rows, failed deltas, and `release_authoritative=false`.

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_parity_evidence]" --reporter compact`

Expected: FAIL because the aggregator does not exist.

- [x] **Step 2: Implement aggregator**

Add deterministic structs for reference captures, backend observations, comparison rows, and `BuildMzParityEvidenceReport()`.

- [x] **Step 3: Verify parity evidence**

Run: `cmake --build --preset dev-debug --target urpg_tests; .\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_parity_evidence]" --reporter compact`

Expected: PASS.

## Task 5: Workbench Parity Integration

**Files:**
- Modify: `editor/compat/mz_migration_workbench_model.h`
- Modify: `editor/compat/mz_migration_workbench_model.cpp`
- Modify: `tests/unit/test_mz_migration_workbench_model.cpp`

- [x] **Step 1: Write failing workbench parity test**

Assert the workbench snapshot includes `parity_reference_count`, `parity_backend_count`, `parity_failed_comparison_count`, and still reports `release_authoritative=false`.

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[editor][compat][mz_workbench]" --reporter compact`

Expected: FAIL because the fields do not exist.

- [x] **Step 2: Add parity evidence to workbench input and snapshot**

Accept optional `MzParityEvidenceReport` and expose the parity counts in JSON.

- [x] **Step 3: Verify workbench parity**

Run: `cmake --build --preset dev-debug --target urpg_tests; .\build\dev-ninja-debug\urpg_tests.exe "[editor][compat][mz_workbench]" --reporter compact`

Expected: PASS.

## Task 6: Docs And Scope Closure

**Files:**
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Modify: `docs/agent/KNOWN_DEBT.md`
- Modify: `docs/agent/QUALITY_GATES.md`

- [x] **Step 1: Update scope language**

Record cloud sync, live services, and store publishing as future-update scope, not unfinished engine code for this completion pass.

- [x] **Step 2: Update protection/MZ status**

Mark release bundle/script protection as implemented for the in-tree authenticated bundle + deterministic script transform scope. Mark MZ parity as evidence-backed but still non-authoritative for proprietary runtime parity.

- [x] **Step 3: Verify truth docs**

Run: `.\tools\ci\truth_reconciler.ps1; .\tools\ci\check_compat_health.ps1`

Expected: PASS.

## Task 7: Final Verification

**Files:**
- No new files.

- [x] **Step 1: Run focused protection verification**

Run: `.\build\dev-ninja-debug\urpg_export_unit_tests.exe "[runtime_bundle],[export][security]" --reporter compact`

Expected: PASS.

- [x] **Step 2: Run focused MZ verification**

Run: `.\build\dev-ninja-debug\urpg_tests.exe "[compat][mz_project_corpus],[compat][mz_parity_evidence],[editor][compat][mz_workbench]" --reporter compact; .\tools\ci\check_mz_project_corpus.ps1`

Expected: PASS.

- [x] **Step 3: Run PR gate**

Run: `ctest --preset dev-pr --output-on-failure`

Expected: PASS.

---

## Self-Review

Spec coverage: Covers the requested release-grade protection lane, real MZ evidence lane, expanded legal corpus, and explicit future-update boundary for cloud/live/store features.

Truth boundary: The plan avoids claiming proprietary RPG Maker runtime parity or external service completion. Bundle protection is scoped to authenticated in-tree release bundles and deterministic script transform unless a future external signing/notarization lane is added.
