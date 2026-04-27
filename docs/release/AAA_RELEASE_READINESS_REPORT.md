# AAA Release-Readiness Report - URPG Engine

**Report date:** 2026-04-26
**Repository audited:** `C:\dev\URPG Maker`
**Active branch:** `development`
**Verification base commit:** `6b4c0a81a8fe3eacdb53857d8d1e6723a779c0a6`
**Purpose:** Authoritative release-readiness audit for the current runtime, editor, packaging, release governance, and asset-hydration gates.
**Verdict:** **NOT RELEASE-READY**

URPG is no longer blocked by the original app-entry, editor-navigation, install/package, metadata, or local validation issues recorded in the first 2026-04-26 audit. Those items now have direct implementation and gate evidence. The project is still not release-ready because two required release exits remain unverified or externally blocked:

- Fresh-clone Git LFS hydration cannot be verified while GitHub reports the repository LFS budget/access blocker.
- Legal/privacy/distribution sufficiency is not verified by a qualified reviewer.

The current app-level source of truth is [docs/APP_RELEASE_READINESS_MATRIX.md](../APP_RELEASE_READINESS_MATRIX.md). That matrix maps each release-facing workflow to owner files, task IDs, evidence commands, and remaining release gates.

## Current Release Decision

| Decision Area | Status | Evidence | Release Decision |
| --- | --- | --- | --- |
| Runtime boot, title, startup diagnostics, and save/continue flow | `VERIFIED` | P1-001, P1-002, P1-003, P3-001, P3-002, P4-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Editor navigation and smoke coverage | `VERIFIED` | P1-004, P1-005, P2-001, P2-002, P3-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Settings, analytics consent, persistence, and save integrity | `VERIFIED` | P4-001, P4-002, P4-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Install/package/app metadata | `VERIFIED` | P5-001, P5-003, P5-004, P6-002 local gates and release-candidate gate | No longer a release blocker for local artifacts. |
| Release-candidate gate script | `PARTIAL` | `./tools/ci/run_release_candidate_gate.ps1 -SkipLfsHydration -LfsWaiverReference docs/APP_RELEASE_READINESS_MATRIX.md#open-release-blocks` passed | Gate exists and passes with explicit LFS waiver; unwaived gate remains blocked. |
| GitHub LFS hydration | `BLOCKED` | P5-005 fresh-clone attempt failed with GitHub LFS budget/access error; P6-002 unwaived hydration attempt timed out before verification | Blocks release until fresh clone plus `git lfs pull` and `git lfs fsck` pass without waiver. |
| Legal/privacy/distribution review | `PARTIAL` | Required docs exist and install/package, but legal sufficiency was not reviewed by qualified counsel | Blocks public release. |
| Release tag | `PENDING` | `git tag -l` returned no tags during P6-002 | Do not tag until unwaived LFS hydration, legal review, and final release decision pass. |

## P6-002 Verification Results

The following commands were run in `C:\dev\URPG Maker` during P6-002:

```powershell
pre-commit run --all-files
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1 -SkipLfsHydration -LfsWaiverReference docs/APP_RELEASE_READINESS_MATRIX.md#open-release-blocks
.\tools\ci\run_release_candidate_gate.ps1 -SkipConfigure -SkipBuild
git tag -l
```

Results:

- `pre-commit run --all-files`: passed after the hook normalized missing final newlines in existing docs.
- `.\tools\ci\run_local_gates.ps1`: passed after fixing two gate-script issues found by the run.
- `.\tools\ci\run_presentation_gate.ps1`: passed; presentation unit lane, spatial editor lane, release validation, and visual regression all passed.
- Release-candidate gate with explicit LFS waiver: passed; configure, build, PR tests, presentation/visual regression, install smoke, and package smoke completed.
- Unwaived release-candidate gate with build/configure skipped: timed out during fresh-clone LFS hydration. This is **unverified** and remains blocked.
- `git tag -l`: returned no tags. No prerelease tag was created because required release exits are still blocked.

Gate fixes made during P6-002:

- `tools/ci/check_cmake_completeness.ps1` now recognizes CMake generator-expression-wrapped source paths, so conditionally compiled OpenGL files are not falsely reported as orphaned.
- `tools/ci/check_install_smoke.ps1` now accepts either `SDL2.dll` or `SDL2d.dll` in Windows install smoke output, matching Release and Debug SDL runtime names.
- Root-level machine-contract docs expected by existing governance scripts were restored from the organized docs/archive locations.

## Closed Original Findings

The original audit findings below are closed for the current claimed scope:

| Original Finding | Current Status | Closing Evidence |
| --- | --- | --- |
| C-1 No project version metadata | `CLOSED` | CMake project version, generated version reporting, app metadata, install/package smoke. |
| C-3 Runtime boots directly into hardcoded map | `CLOSED` | Runtime title/startup scene and title-flow tests. |
| C-4 Runtime entry point has no save/load flow | `CLOSED` | Runtime continue/save discovery and recovery tests. |
| C-5 Runtime entry point missing subsystem startup diagnostics | `CLOSED` | Audio/input/localization/profiler/startup diagnostics tests. |
| C-6 Editor registers too few panels | `CLOSED` | Registry and smoke coverage for intended top-level panels. |
| C-7 Editor smoke test too narrow | `CLOSED` | Expanded smoke/registry coverage. |
| C-8 Release validation uses `assert()` | `CLOSED` | Release validation now uses explicit runtime checks. |
| C-9 Install rules are minimal | `CLOSED` | Component install rules, install smoke, package smoke. |
| C-10 Required legal/release documents missing | `PARTIAL` | Docs exist and package; legal sufficiency remains unverified. |
| High/medium app wiring, persistence, packaging, and matrix gaps | `CLOSED OR PARTIAL AS MATRIXED` | See [App Release Readiness Matrix](../APP_RELEASE_READINESS_MATRIX.md). |

## Remaining Release Blockers

### RB-1: Fresh-Clone Git LFS Hydration Is Blocked

**Status:** `BLOCKED`

The repository cannot be declared release-ready until a fresh clone can hydrate all required LFS assets without relying on local cache.

Required verification:

```powershell
GIT_LFS_SKIP_SMUDGE=1 git clone --depth 1 --branch development --filter=blob:none <origin> <temp>
git -C <temp> lfs pull
git -C <temp> lfs fsck
```

Current evidence: P5-005 observed GitHub LFS budget/access failure. P6-002 attempted the unwaived release-candidate hydration path and it did not complete within the local timeout. This remains unverified.

### RB-2: Legal/Privacy/Distribution Review Is Not Complete

**Status:** `PARTIAL`

`THIRD_PARTY_NOTICES.md`, `EULA.md`, `PRIVACY_POLICY.md`, `CREDITS.md`, and `CHANGELOG.md` exist and are included in install/package smoke outputs. Their legal sufficiency is not verified.

Required verification: qualified legal/privacy review approves the documents and distribution terms.

### RB-3: Remote Manual Release-Candidate Workflow Is Not Verified

**Status:** `PARTIAL`

The manual GitHub Actions release-candidate job exists, but no remote workflow run is recorded in this report.

Required verification: run the `workflow_dispatch` release-candidate job on GitHub. If the LFS blocker is still active, provide the same explicit waiver reference and record the remote run URL.

### RB-4: No Release Or Prerelease Tag Exists

**Status:** `PENDING`

No release tag should be created until RB-1, RB-2, and RB-3 are resolved or formally waived by the release owner with owner, scope, issue URL, and expiration.

## Final Release Exit Criteria

The project may be marked release-ready only after all of the following are true:

- `pre-commit run --all-files` passes.
- `.\tools\ci\run_local_gates.ps1` passes.
- `.\tools\ci\run_presentation_gate.ps1` passes.
- `.\tools\ci\run_release_candidate_gate.ps1` passes without `-SkipLfsHydration`.
- Fresh-clone LFS hydration passes without relying on local cache.
- Legal/privacy/distribution review is complete.
- A release owner records the release decision and creates an annotated prerelease or release tag.

Until then, the authoritative verdict remains **NOT RELEASE-READY**.
