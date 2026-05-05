# AAA Release-Readiness Report - URPG Engine

**Report date:** 2026-05-05
**Repository audited:** `C:\dev\URPG Maker`
**Active branch:** `codex/public-release-record` docs-only release record for `main`
**Verification base commit:** `60f61bd07179cc85a665695ddc2d543ff766eba0` on `main`
**Purpose:** Authoritative release-readiness audit for the current runtime, editor, packaging, release governance, and asset-hydration gates.
**Verdict:** **PUBLIC RELEASE READY; V0.1.0 TAG AUTHORIZED**

**Current documentation note (2026-05-05):** The production release execution-plan remediation has been merged to `main`. Runtime chat text entry, project-root-aware save/load, AI animation application, release-mode missing-asset diagnostics, release legal/contact metadata, hidden cloud-sync policy, silent audio scope, bounded starter-visual scope, public security reporting policy, install/package docs, and release evidence fixes are implemented and covered by local and remote gates. This report records the release-owner decision to create the public `v0.1.0` release tag after the final docs-only release record lands and the final gate passes on the tag target commit.

URPG is no longer blocked by the original app-entry, editor-navigation, install/package, metadata, release-required asset hydration, legal owner acceptance/waiver, remote workflow, local release-candidate validation, security-reporting, or package-documentation issues recorded in the release audits. Those items now have direct implementation and gate evidence or explicit release-owner waiver. For the bounded `v0.1.0` package scope, the remaining action is mechanical publication: create the annotated release tag and GitHub release from the verified package artifacts.

- The release owner approved public `v0.1.0` tag creation for the verified bounded package scope.

The current app-level source of truth is [docs/APP_RELEASE_READINESS_MATRIX.md](../APP_RELEASE_READINESS_MATRIX.md). That matrix maps each release-facing workflow to owner files, task IDs, evidence commands, and remaining release gates.

## Current Release Decision

| Decision Area | Status | Evidence | Release Decision |
| --- | --- | --- | --- |
| Runtime boot, title, startup diagnostics, and save/continue flow | `VERIFIED` | P1-001, P1-002, P1-003, P3-001, P3-002, P4-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Editor navigation and smoke coverage | `VERIFIED` | P1-004, P1-005, P2-001, P2-002, P3-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Settings, analytics consent, persistence, and save integrity | `VERIFIED` | P4-001, P4-002, P4-003 tests and local gates | No longer a release blocker for the claimed scope. |
| Install/package/app metadata | `VERIFIED` | P5-001, P5-003, P5-004, P6-002 local gates and release-candidate gate | No longer a release blocker for local artifacts. |
| Release-candidate gate script | `VERIFIED` | `.\tools\ci\run_release_candidate_gate.ps1` passed on `main`; GitHub Actions run `25351794008` passed at `60f61bd07179cc85a665695ddc2d543ff766eba0` | Local gate passes through fresh-clone asset verification, configure, build, PR tests, presentation validation, install smoke, package smoke, and CPack package generation; remote manual workflow passed on `main`: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25351794008`. |
| Release-required asset hydration | `VERIFIED` | `resources/icons/*.png` were demoted from LFS to normal Git blobs; fresh clone from GitHub passed the RC asset check | No longer a release-package blocker. Repository-wide vendor/source LFS hydration remains blocked by GitHub budget/access and is not required by current package/install rules. |
| Legal/privacy/distribution review | `VERIFIED` | Required docs exist and install/package; release owner recorded `WAIVED_BY_RELEASE_OWNER` in `docs/release/LEGAL_REVIEW_SIGNOFF.md`; P5-02 evidence reflects bundled `BND-001`, deferred `BND-002`, opt-in local analytics behavior, and release-owner certification that shipped paid/licensed assets are usable in distributed games | Does not block public release. This is an owner waiver, not qualified legal counsel approval. |
| Release tag | `APPROVED_TO_TAG` | `git tag --list 'v0.1.0'` returned no existing tag before publication; local and remote release-candidate gates passed on `main` at `60f61bd07179cc85a665695ddc2d543ff766eba0`; release owner approved public release | Create annotated `v0.1.0` tag and GitHub release from the final `main` release commit after this docs-only release record passes the final gate. |

## 2026-05-05 Public Release Verification Results

The following commands were run in `C:\dev\URPG Maker` after PR #8 merged to `main`:

```powershell
.\tools\ci\run_release_candidate_gate.ps1
gh workflow run "URPG CI Gates" --ref main -f run_release_candidate_gate=true -f lfs_waiver_reference=""
gh run view 25351794008 --json status,conclusion,headSha,url,jobs
git tag --list 'v0.1.0'
gh release view v0.1.0
```

Results:

- `.\tools\ci\run_release_candidate_gate.ps1`: passed on `main` at `60f61bd07179cc85a665695ddc2d543ff766eba0`. The gate hydrated release-required assets from a fresh clone, configured and built the release preset, ran PR-level tests, ran the presentation gate, passed install smoke, passed package smoke, and generated the component ZIPs.
- GitHub Actions run `25351794008`: passed on `main` at `60f61bd07179cc85a665695ddc2d543ff766eba0`; `release-candidate` and `gate1-pr` completed successfully, while nightly and weekly jobs were skipped by design for the manually triggered release run.
- `git tag --list 'v0.1.0'`: returned no existing tag before publication.
- `gh release view v0.1.0`: returned no existing GitHub release before publication.

Verified package artifacts:

| Artifact | SHA-256 |
| --- | --- |
| `build/release-candidate-package/URPG-0.1.0-Windows-AMD64-Docs.zip` | `3C147F40B1363D1217C1215AB08430DB7D7EFC215DDEDB0DACC1A7A42A5789FF` |
| `build/release-candidate-package/URPG-0.1.0-Windows-AMD64-Runtime.zip` | `296FA6D108B1EDB3611322B5A06965BD8210E884EA47EC5A0AF4876B53C3DE5A` |
| `build/release-candidate-package/URPG-0.1.0-Windows-AMD64-RuntimeData.zip` | `0F66A4236E8EF2FEC95EE1CBBB123011EACB9CCB3813A71A49A818D15524AD38` |

## P6-03 Local Verification Results

The following commands were run in `C:\dev\URPG Maker` on 2026-05-04 during P6-03:

```powershell
ctest --preset dev-snapshot --output-on-failure
.\tools\ci\check_release_required_assets.ps1
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1
.\build\dev-ninja-debug\urpg_project_audit.exe --json
```

Results:

- `ctest --preset dev-snapshot --output-on-failure`: passed.
- `.\tools\ci\check_release_required_assets.ps1`: passed and rewrote `imports/reports/asset_intake/release_required_asset_report.json`.
- `.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug`: passed.
- `.\tools\ci\run_local_gates.ps1`: passed end to end after correcting the new save/load recovery regression.
- `.\tools\ci\run_presentation_gate.ps1`: passed; presentation unit lane, spatial editor lane, release validation, and visual regression were green.
- `.\tools\ci\run_release_candidate_gate.ps1`: passed without LFS waiver after adding release PR-test build targets and correcting the documented CTest regex. The gate fresh-cloned to `C:\Users\Doc\AppData\Local\Temp\urpg-rc-lfs-20260504-124558`, ran Git LFS fsck, validated release-required assets, configured and built `dev-ninja-release`, ran PR-level tests, presentation validation, install smoke, and package smoke.
- `.\build\dev-ninja-debug\urpg_project_audit.exe --json`: passed with `releaseBlockerCount: 0`, `exportBlockerCount: 0`, and summary `Selected template jrpg is READY. 0 release blockers and 0 export blockers were found.`

P6-03 package artifacts:

- `build/release-candidate-package/URPG-0.1.0-Windows-AMD64-Docs.zip`
- `build/release-candidate-package/URPG-0.1.0-Windows-AMD64-Runtime.zip`
- `build/release-candidate-package/URPG-0.1.0-Windows-AMD64-RuntimeData.zip`

P6-03 release-scope notes:

- Public legal/privacy distribution remains release-owner-waived, not qualified-counsel-approved.
- Current audio release scope is intentionally silent/muted unless a future bundled release-required audio asset is promoted.
- Current release visuals are bounded starter/proof assets, not final AAA art direction, and `releaseAssets.visualClaimScope` is now enforced by the release asset gate.
- Cloud/cross-device sync remains hidden from production release surfaces unless an out-of-tree reviewed provider reports visible remote support through `ICloudService::releaseVisibility()`.

## Phase 12 Local Verification Results

The following commands were run in `C:\dev\URPG Maker` on branch `codex/asset-release-ingestion` after promoting governed content bundles `BND-006`, `BND-007`, and `BND-008`:

```powershell
pre-commit run --all-files
.\tools\ci\run_local_gates.ps1
.\tools\ci\run_presentation_gate.ps1
.\tools\ci\run_release_candidate_gate.ps1
```

Results:

- `pre-commit run --all-files`: passed after hook formatting was applied.
- `.\tools\ci\run_local_gates.ps1`: passed end to end after correcting a case-sensitive documented CTest regex, aligning the 3D dungeon registry assertion with the current nested route contract, and extending `urpg_export_unit_lane` timeout to 900 seconds for the larger governed asset-manifest scan.
- `.\tools\ci\run_presentation_gate.ps1`: passed; presentation unit lane, spatial editor lane, presentation release validation, and visual regression gate were green.
- `.\tools\ci\run_release_candidate_gate.ps1`: passed without `-SkipLfsHydration`; the gate fresh-cloned to `C:\Users\Doc\AppData\Local\Temp\urpg-rc-lfs-20260503-145111`, ran Git LFS fsck, validated release-required assets, configured and built `dev-ninja-release`, ran install smoke, and produced CPack component ZIPs for `Runtime`, `RuntimeData`, and `Docs`.

Phase 12 asset evidence:

- `BND-006` promotes 12,430 unique app-usable SRC-010 CC0/public-domain payloads with attribution records, SHA-256 checksums, package destinations, and `release_eligible: true`.
- `BND-007` promotes 4,697 additional app-usable SRC-010 payloads unlocked by source-folder/archive license evidence with attribution records, SHA-256 checksums, package destinations, and `release_eligible: true`.
- `BND-008` promotes 394 valid `itch/loose` CC0 PNG payloads under `SRC-013` with attribution records, SHA-256 checksums, package destinations, and `release_eligible: true`.
- All three curated bulk bundles use `distribution: deferred` and `release_required: false`, so assets are available to the app/library and project selection without bloating default release packages.

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
- Remote manual GitHub Actions release-candidate workflow run `25025111713`: passed on `development` at commit `7439132f4fa2638730498781f617d78af7b16514`; `release-candidate` and `gate1-pr` jobs were green, while nightly and weekly jobs were skipped by design. Run URL: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25025111713`. This historical run is superseded for public release by the 2026-05-05 `main` run recorded above.
- `git tag -l`: returned no tags at that checkpoint. No prerelease tag was created during P6-002 because required release exits were not yet closed; this is superseded by the 2026-05-05 tag authorization recorded above.

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

### RB-1: Legal/Privacy/Distribution Review Is Owner-Waived

**Status:** `VERIFIED`

`THIRD_PARTY_NOTICES.md`, `EULA.md`, `PRIVACY_POLICY.md`, `CREDITS.md`, and `CHANGELOG.md` exist and are included in install/package smoke outputs. The release owner recorded `WAIVED_BY_RELEASE_OWNER` in `docs/release/LEGAL_REVIEW_SIGNOFF.md` on 2026-04-30, accepted public distribution responsibility without qualified legal counsel approval, and certified that shipped paid/licensed assets are usable in distributed games. The P5-02 evidence pass checked the notices against the current release-required asset manifest: `BND-001` is the bundled visual proof lane, while `BND-002` UI SFX WAV payloads are deferred/local-only. The privacy policy is aligned to the current analytics implementation: default disabled consent, explicit opt-in, and local JSONL export only.

Required release discipline: do not include raw/vendor/source asset packs unless source-specific license and attribution evidence is reviewed before inclusion.

### RB-2: Remote Manual Release-Candidate Workflow Is Verified

**Status:** `VERIFIED`

The manual GitHub Actions release-candidate workflow passed in run `25351794008` on `main` at commit `60f61bd07179cc85a665695ddc2d543ff766eba0`: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25351794008`.

This is no longer a release blocker for public `v0.1.0` validation.

### RB-3: Release Tag Approved

**Status:** `APPROVED_TO_TAG`

Legal review is formally waived by the release owner, local and remote release-candidate gates have passed for the `v0.1.0` target lineage, and the release owner approved public tag creation. The publication step is to create an annotated `v0.1.0` tag and GitHub release from the final `main` release commit after this docs-only release record passes the final gate.

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
- Legal/privacy/distribution review is complete for the intended distribution scope or explicitly waived by the release owner.
- A release owner records the release decision and creates an annotated prerelease or release tag.

For the bounded `v0.1.0` package scope, these criteria are satisfied or owner-waived. The authoritative verdict is **public-release ready; `v0.1.0` tag authorized**.
