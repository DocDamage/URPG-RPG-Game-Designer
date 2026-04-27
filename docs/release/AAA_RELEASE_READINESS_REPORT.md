# AAA Release-Readiness Report - URPG Engine

**Report date:** 2026-04-27
**Repository audited:** `C:\dev\URPG Maker`
**Active branch:** `development`
**Verification base commit:** `7439132f4fa2638730498781f617d78af7b16514`
**Purpose:** Authoritative release-readiness audit for the current runtime, editor, packaging, release governance, and asset-hydration gates.
**Verdict:** **NOT RELEASE-READY**

URPG is no longer blocked by the original app-entry, editor-navigation, install/package, metadata, release-required asset hydration, private/internal RC legal owner acceptance, or local validation issues recorded in the first 2026-04-26 audit. Those items now have direct implementation and gate evidence. The project is still not public-release-ready because required public release exits remain unverified or externally blocked:

- Legal/privacy/distribution sufficiency for public release is not verified by a qualified reviewer, and no explicit public-release waiver is recorded.
- No release or prerelease tag exists.

The current app-level source of truth is [docs/APP_RELEASE_READINESS_MATRIX.md](../APP_RELEASE_READINESS_MATRIX.md). That matrix maps each release-facing workflow to owner files, task IDs, evidence commands, and remaining release gates.

## Current Release Decision

| Decision Area | Status | Evidence | Release Decision |
| --- | --- | --- | --- |
| Runtime boot, title, startup diagnostics, and save/continue flow | `VERIFIED` | P1-001, P1-002, P1-003, P3-001, P3-002, P4-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Editor navigation and smoke coverage | `VERIFIED` | P1-004, P1-005, P2-001, P2-002, P3-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Settings, analytics consent, persistence, and save integrity | `VERIFIED` | P4-001, P4-002, P4-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Install/package/app metadata | `VERIFIED` | P5-001, P5-003, P5-004, P6-002 local gates and release-candidate gate | No longer a release blocker for local artifacts. |
| Release-candidate gate script | `VERIFIED` | `./tools/ci/run_release_candidate_gate.ps1` passed without LFS waiver after commit `4fb53f721`; GitHub Actions run `25025111713` passed at commit `7439132f4fa2638730498781f617d78af7b16514` | Local gate passes through fresh-clone asset verification, configure, build, PR tests, presentation validation, install smoke, and package smoke; remote manual workflow passed on `development`: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25025111713`. |
| Release-required asset hydration | `VERIFIED` | `resources/icons/*.png` were demoted from LFS to normal Git blobs; fresh clone from GitHub passed the RC asset check | No longer a release-package blocker. Repository-wide vendor/source LFS hydration remains blocked by GitHub budget/access and is not required by current package/install rules. |
| Legal/privacy/distribution review | `PARTIAL` | Required docs exist and install/package; release owner approved private/internal RC use in `docs/release/LEGAL_REVIEW_SIGNOFF.md`; P5-02 evidence now reflects bundled `BND-001`, deferred `BND-002`, and opt-in local analytics behavior; qualified legal counsel has not approved public release and no public-release waiver is recorded | Blocks public release, not private/internal RC use. |
| Release tag | `PENDING` | `git tag -l` returned no tags during P6-002 | Do not tag until legal/public distribution review and final release decision pass or are formally waived. |

## P6-002 Verification Results

The following commands were run in `C:\dev\URPG Maker` during P6-002:

```powershell
pre-commit run --all-files
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1 -SkipLfsHydration -LfsWaiverReference docs/APP_RELEASE_READINESS_MATRIX.md#open-release-blocks
.\tools\ci\run_release_candidate_gate.ps1 -SkipConfigure -SkipBuild
.\tools\ci\run_release_candidate_gate.ps1
git tag -l
```

Results:

- `pre-commit run --all-files`: passed after the hook normalized missing final newlines in existing docs.
- `.\tools\ci\run_local_gates.ps1`: passed after fixing two gate-script issues found by the run.
- `.\tools\ci\run_presentation_gate.ps1`: passed; presentation unit lane, spatial editor lane, release validation, and visual regression all passed.
- Release-candidate gate with explicit LFS waiver: passed; configure, build, PR tests, presentation/visual regression, install smoke, and package smoke completed.
- Initial unwaived release-candidate gate with build/configure skipped: timed out during broad fresh-clone LFS hydration before release-required asset scope was narrowed.
- Follow-up unwaived release-candidate gate after commit `4fb53f721`: passed from a fresh GitHub clone; release-required assets hydrated from normal Git blobs, `git lfs fsck --pointers` passed, configure/build passed, 1383 PR-level tests passed, focused presentation validation passed, install smoke passed, and package smoke passed.
- Remote manual GitHub Actions release-candidate workflow run `25025111713`: passed on `development` at commit `7439132f4fa2638730498781f617d78af7b16514`; `release-candidate` and `gate1-pr` jobs were green, while nightly and weekly jobs were skipped by design. Run URL: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25025111713`.
- `git tag -l`: returned no tags. No prerelease tag was created because required release exits are still blocked.

Gate fixes made during P6-002:

- `tools/ci/check_cmake_completeness.ps1` now recognizes CMake generator-expression-wrapped source paths, so conditionally compiled OpenGL files are not falsely reported as orphaned.
- `tools/ci/check_install_smoke.ps1` now accepts either `SDL2.dll` or `SDL2d.dll` in Windows install smoke output, matching Release and Debug SDL runtime names.
- `tools/ci/run_release_candidate_gate.ps1` now verifies only release-required fresh-clone assets instead of attempting to hydrate every source/vendor LFS pointer.
- `resources/icons/*.png` are normal Git blobs so package metadata icons do not depend on GitHub LFS budget/access.
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

### RB-1: Legal/Privacy/Distribution Review Is Not Complete

**Status:** `PARTIAL`

`THIRD_PARTY_NOTICES.md`, `EULA.md`, `PRIVACY_POLICY.md`, `CREDITS.md`, and `CHANGELOG.md` exist and are included in install/package smoke outputs. The release owner has approved them for private/internal release-candidate use in `docs/release/LEGAL_REVIEW_SIGNOFF.md`. The P5-02 evidence pass checked the notices against the current release-required asset manifest: `BND-001` is the bundled visual proof lane, while `BND-002` UI SFX WAV payloads are deferred/local-only. The privacy policy is aligned to the current analytics implementation: default disabled consent, explicit opt-in, and local JSONL export only. Public-release legal sufficiency is still not verified by qualified counsel, and no explicit public-release waiver is recorded.

Required verification for public release: qualified legal/privacy review approves the documents and distribution terms, or the release owner records an explicit public-release waiver.

### RB-2: Remote Manual Release-Candidate Workflow Is Verified

**Status:** `VERIFIED`

The manual GitHub Actions release-candidate workflow passed in run `25025111713` on `development` at commit `7439132f4fa2638730498781f617d78af7b16514`: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25025111713`.

This is no longer a release blocker for private/internal RC validation.

### RB-3: No Release Or Prerelease Tag Exists

**Status:** `PENDING`

No release tag should be created until RB-1 is resolved or formally waived by the release owner with owner, scope, issue URL, and expiration.

### Non-Release Block: Repository-Wide Source/Vendor LFS Hydration

**Status:** `BLOCKED`

GitHub still reports the repository-wide LFS budget/access blocker when attempting broad source/vendor asset hydration. Current install/package rules do not ship those source/vendor packs. Release-required assets are verified separately by the RC gate and are no longer LFS pointers.

Required verification before depending on source/vendor LFS packs: restore GitHub LFS budget/access, then run broad `git lfs pull`/object verification from a fresh clone.

## Final Release Exit Criteria

The project may be marked release-ready only after all of the following are true:

- `pre-commit run --all-files` passes.
- `.\tools\ci\run_local_gates.ps1` passes.
- `.\tools\ci\run_presentation_gate.ps1` passes.
- `.\tools\ci\run_release_candidate_gate.ps1` passes without `-SkipLfsHydration`.
- Fresh-clone release-required asset verification passes without relying on local cache.
- Legal/privacy/distribution review is complete for the intended distribution scope.
- A release owner records the release decision and creates an annotated prerelease or release tag.

Until then, the authoritative verdict remains **NOT RELEASE-READY**.
