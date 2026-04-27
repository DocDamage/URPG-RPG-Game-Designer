# App Release Readiness Matrix

Status Date: 2026-04-26
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
| Boot flow | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/scene/runtime_title_scene.*` | `ctest --preset dev-all -R "RuntimeTitleScene|runtime title|SceneManager: Basic" --output-on-failure` | P1-001 | Runtime reaches title/startup flow instead of hardcoded map boot. |
| Save/load continue | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/save/*`; `engine/core/scene/runtime_title_scene.*` | `ctest --preset dev-all -R "Runtime startup save|RuntimeTitleScene|save|runtime" --output-on-failure` | P1-002; P4-003 | Runtime continue path discovers save metadata and handles recovery/failure. |
| Settings persistence | `VERIFIED` | `engine/core/settings/app_settings.*`; `apps/runtime/main.cpp`; `apps/editor/main.cpp` | `ctest --preset dev-all -R "settings|persistence|imgui" --output-on-failure` | P4-001 | Runtime/editor settings persist through configured app settings paths. |
| Audio startup | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/audio/*` | `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` | P1-003 | Startup initializes or explicitly diagnoses audio subsystem state. |
| Input startup | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/input/*` | `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` | P1-003 | Startup initializes or explicitly diagnoses input subsystem state. |
| Localization startup | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/localization/*` | `ctest --preset dev-all -R "runtime|startup|audio|asset|input|localization|profiler" --output-on-failure` | P1-003 | Startup initializes or explicitly diagnoses localization catalog state. |
| Runtime asset validation | `VERIFIED` | `apps/runtime/main.cpp`; `engine/core/project/runtime_project_preflight.*` | `ctest --preset dev-all -R "startup|preflight|runtime" --output-on-failure` | P3-002 | Missing project/runtime data emits targeted preflight diagnostics. |
| Editor navigation | `VERIFIED` | `apps/editor/main.cpp`; `editor/*/*panel.*` | `ctest --preset dev-all -R "editor|panel|smoke" --output-on-failure` | P1-004; P1-005 | Intended top-level editor panels are registered and smoke-covered. |
| Editor incomplete/empty surfaces | `PARTIAL` | `editor/*/*panel.*`; `docs/release/EDITOR_CONTROL_INVENTORY.md` | `ctest --preset dev-all -R "editor|empty|loading|error" --output-on-failure` | P2-001; P2-002; P3-003 | High-risk panels have explicit disabled/empty/error states; graphical hover/manual behavior remains unverified. |
| Analytics consent | `VERIFIED` | `engine/core/analytics/*`; `apps/editor/main.cpp` | `ctest --preset dev-all -R "analytics|privacy|consent" --output-on-failure` | P4-002 | Analytics remains opt-in with persisted consent and disable path. |
| Install layout | `VERIFIED` | `CMakeLists.txt`; `tools/ci/check_install_smoke.ps1`; `docs/packaging.md` | `./tools/ci/check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke` | P5-001; P5-004 | Installed tree includes apps, runtime data, docs, metadata, and can launch runtime smoke. |
| Package layout | `VERIFIED` | `cmake/packaging.cmake`; `tools/ci/check_package_smoke.ps1`; `docs/release/RELEASE_PACKAGING.md` | `./tools/ci/check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke` | P5-003; P5-004 | CPack emits component archives with expected runtime, data, docs, icon, and desktop metadata entries. |
| Legal docs | `PARTIAL` | `THIRD_PARTY_NOTICES.md`; `EULA.md`; `PRIVACY_POLICY.md`; `CREDITS.md`; `CHANGELOG.md` | `./tools/ci/check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke` | P5-002 | Required docs exist and install; legal sufficiency remains unverified until qualified legal review. |
| LFS hydration | `BLOCKED` | `.gitattributes`; `.gitignore`; `imports/`; `third_party/`; `more assets/` | `GIT_LFS_SKIP_SMUDGE=1 git clone --depth 1 --branch development --filter=blob:none <origin> <temp>; git lfs pull` | P5-005 | Fresh-clone hydration failed because GitHub reports the repository exceeded its LFS budget. |
| Final release candidate gate | `PENDING` | `tools/ci/run_release_candidate_gate.ps1`; `.github/workflows/ci-gates.yml` | `./tools/ci/run_release_candidate_gate.ps1` | P6-001; P6-002 | One command and manual CI workflow must prove final build, tests, smokes, docs, and LFS gate behavior. |

## Open Release Blocks

| Blocker | Status | Required Resolution |
| --- | --- | --- |
| GitHub LFS budget/access | `BLOCKED` | Restore GitHub LFS budget/access or move release-required LFS payload to an accessible artifact store, then rerun fresh-clone hydration. |
| Legal review | `PARTIAL` | Qualified legal/privacy review must approve EULA, privacy policy, third-party notices, credits, and public distribution terms. |
| Release candidate gate | `PENDING` | Implement and run the P6-001 release-candidate gate, then record final P6-002 verification results. |

## Verification

Use this query to confirm this matrix and task references remain discoverable:

```powershell
rg -n "APP_RELEASE_READINESS_MATRIX|P[0-6]-[0-9]{3}" docs
```
