# App Release Readiness Matrix

Status Date: 2026-04-27
Authority: canonical app-level release-readiness tracker for runtime, editor, packaging, legal, and asset-hydration gates.

This matrix maps release-facing application workflows to the execution-plan task that proves or blocks them. It complements `docs/release/RELEASE_READINESS_MATRIX.md`, which remains the subsystem status reference.

## Status Values

| Status | Meaning |
| --- | --- |
| `VERIFIED` | The app-facing workflow has direct implementation and verification evidence for the claimed scope. |
| `PARTIAL` | The workflow has landed evidence, but at least one release gate remains unverified or human-review-gated. |
| `BLOCKED` | The workflow cannot pass release verification until an external or unresolved blocker is cleared. |
| `PENDING` | The workflow is planned but not yet implemented or verified in this execution plan. |

## Current App Matrix

| Area | Status | Owner File | Evidence Command | Blocking Task ID | Release Gate |
| --- | --- | --- | --- | --- | --- |
| Boot flow | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/scene/runtime_title_scene.*` | `ctest --preset dev-all -R "RuntimeTitleScene|SceneManager: Basic" --output-on-failure` | P1-001 | Runtime reaches title/startup flow instead of hardcoded map boot. |
| Save/load continue | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/save/*`; `engine/core/scene/runtime_title_scene.*` | `ctest --preset dev-all -R "RuntimeTitleScene|save|runtime" --output-on-failure` | P1-002; P4-003 | Runtime continue path discovers save metadata and handles recovery/failure. |
| Settings persistence | `VERIFIED` | `engine/core/settings/app_settings.*`; `apps/runtime/main.cpp`; `apps/editor/main.cpp` | `ctest --preset dev-all -R "settings|persistence|ImGui" --output-on-failure` | P4-001 | Runtime/editor settings persist through configured app settings paths. |
| Audio startup | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/audio/*` | `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` | P1-003 | Startup initializes or explicitly diagnoses audio subsystem state. |
| Input startup | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/input/*` | `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` | P1-003 | Startup initializes or explicitly diagnoses input subsystem state. |
| Localization startup | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/localization/*` | `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` | P1-003 | Startup initializes or explicitly diagnoses localization catalog state. |
| Runtime asset validation | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/project/runtime_project_preflight.*` | `ctest --preset dev-all -R "startup|preflight|runtime" --output-on-failure` | P3-002 | Missing project/runtime data emits targeted preflight diagnostics. |
| Editor navigation | `VERIFIED` | `apps/editor/main.cpp`; `editor/*/*panel.*` | `ctest --preset dev-all -R "editor|panel|smoke" --output-on-failure` | P1-004; P1-005 | Intended top-level editor panels are registered and smoke-covered. |
| Editor incomplete/empty surfaces | `VERIFIED` | `editor/*/*panel.*`; `docs/release/EDITOR_CONTROL_INVENTORY.md` | `ctest --preset dev-all -R "editor|empty|loading|error" --output-on-failure` | P2-001; P2-002; P3-003 | High-risk panels have explicit disabled/empty/error states; graphical hover/manual behavior is release-owner accepted for the current internal branch, with any owner-observed regressions handled as follow-up bugs. |
| Analytics consent | `VERIFIED` | `engine/core/analytics/*`; `apps/editor/main.cpp` | `ctest --preset dev-all -R "analytics|privacy|consent" --output-on-failure` | P4-002 | Analytics remains opt-in with persisted consent and disable path. |
| Install layout | `VERIFIED` | `CMakeLists.txt`; `tools/ci/check_install_smoke.ps1`; `docs/packaging.md` | `./tools/ci/check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke` | P5-001; P5-004 | Installed tree includes apps, runtime data, docs, metadata, and can launch runtime smoke. |
| Package layout | `VERIFIED` | `cmake/packaging.cmake`; `tools/ci/check_package_smoke.ps1`; `docs/release/RELEASE_PACKAGING.md` | `./tools/ci/check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke` | P5-003; P5-004 | CPack emits component archives with expected runtime, data, docs, icon, and desktop metadata entries. |
| Legal docs | `PARTIAL` | `THIRD_PARTY_NOTICES.md`; `EULA.md`; `PRIVACY_POLICY.md`; `CREDITS.md`; `CHANGELOG.md`; `docs/release/LEGAL_REVIEW_SIGNOFF.md` | `rg -n "PARTIAL|qualified legal|public-release waiver|NOT RELEASE-READY" docs/release docs/APP_RELEASE_READINESS_MATRIX.md` | P5-002 | Required docs exist and install; release owner approved private/internal RC use only. P5-02 evidence reflects bundled `BND-001`, deferred local-only `BND-002` WAV payloads, and opt-in local analytics behavior. Public legal sufficiency remains unverified until qualified legal review or explicit public-release waiver. |
| Release-required asset hydration | `VERIFIED` | `.gitattributes`; `resources/icons/*.png`; `tools/ci/run_release_candidate_gate.ps1` | `./tools/ci/run_release_candidate_gate.ps1` | P5-005; P6-002 | Fresh clone from GitHub verifies the release-required icon assets without an LFS waiver; source/vendor LFS packs remain outside the release package path. |
| Final release candidate gate | `VERIFIED` | `tools/ci/run_release_candidate_gate.ps1`; `.github/workflows/ci-gates.yml` | `./tools/ci/run_release_candidate_gate.ps1`; GitHub Actions run `25025111713` | P6-001; P6-002 | Local unwaived gate passes through fresh-clone asset hydration, configure, build, PR tests, presentation validation, install smoke, and package smoke; remote manual workflow run `25025111713` passed on `development` at commit `7439132f4fa2638730498781f617d78af7b16514`: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25025111713`. |

## Open Release Blocks And External Constraints

| Blocker | Status | Required Resolution |
| --- | --- | --- |
| Repository-wide source/vendor LFS budget/access | `BLOCKED` | Restore GitHub LFS budget/access before relying on full vendor/source asset hydration. This is not currently a release-package blocker because release-required assets are now normal Git blobs and are verified by the RC gate. |
| Legal review | `PARTIAL` | Private/internal RC use is owner-approved in `docs/release/LEGAL_REVIEW_SIGNOFF.md`; no public-release waiver is recorded, so public release still requires qualified legal/privacy review or an explicit public-release waiver. |

## Verification

Use this query to confirm this matrix and task references remain discoverable:

```powershell
rg -n "APP_RELEASE_READINESS_MATRIX|P[0-6]-[0-9]{3}" docs
```
