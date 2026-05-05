# App Release Readiness Matrix

Status Date: 2026-05-05
Authority: canonical app-level release-readiness tracker for runtime, editor, packaging, legal, and asset-hydration gates.

This matrix maps release-facing application workflows to the execution-plan task that proves or blocks them. It complements `docs/release/RELEASE_READINESS_MATRIX.md`, which remains the subsystem status reference.

Latest audit-remediation checkpoint: Phase 0 through Phase 12 plus the 2026-05-05 public-release record pass are landed or approved for the bounded `v0.1.0` package scope. The mandatory completion-scope lock is active: curated asset/content promotion has governed bundle evidence, final local and remote release-candidate gates passed on `main` at `60f61bd07179cc85a665695ddc2d543ff766eba0`, and release-owner approval authorizes the annotated `v0.1.0` tag after this docs-only release record passes the final gate. Editor navigation is verified against app-shell factories, release authoring surfaces have persistence/input/error-state coverage, release-required assets are classified, selected Phase 9 release bundle categories are checksumed and package-gated, approved Phase 11 offline retrieval/vision/audio tooling stays artifact-producing under `tools/`, install/package smoke passes against `build/dev-ninja-release`, and `.\tools\ci\run_local_gates.ps1`, `.\tools\ci\run_presentation_gate.ps1`, and `.\tools\ci\run_release_candidate_gate.ps1` pass end to end.

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
| Editor navigation | `VERIFIED` | `apps/editor/main.cpp`; `apps/editor/editor_app_panels.cpp`; `editor/*/*panel.*`; `engine/core/editor/editor_panel_registry.cpp` | `ctest --test-dir build\dev-ninja-debug -R "editor app panels|Editor panel registry|urpg_editor_smoke|urpg_editor_list_panels" --output-on-failure`; `.\build\dev-ninja-debug\urpg_editor.exe --headless --open-panel level_builder --frames 2 --project-root .`; `.\build\dev-ninja-debug\urpg_editor.exe --safe-mode`; `.\build\dev-ninja-debug\urpg_editor.exe --probe-platform`; `ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure` | P1-004; P1-005 | App-shell wired top-level panels are registered and smoke-covered. `level_builder` is reachable from production navigation through the real native grid-part Level Builder workspace. `--safe-mode` provides a non-OpenGL diagnostic startup path for machines where visible platform-surface creation is unsafe; `--probe-platform` checks SDL video/controller state without creating an OpenGL window. |
| Editor incomplete/empty surfaces | `VERIFIED` | `editor/*/*panel.*`; `docs/release/EDITOR_CONTROL_INVENTORY.md` | `ctest --preset dev-all -R "editor|empty|loading|error" --output-on-failure` | P2-001; P2-002; P3-003 | High-risk panels have explicit disabled/empty/error states; graphical hover/manual behavior is release-owner accepted for the current internal branch, with any owner-observed regressions handled as follow-up bugs. |
| Analytics consent | `VERIFIED` | `engine/core/analytics/*`; `apps/editor/main.cpp` | `ctest --preset dev-all -R "analytics|privacy|consent" --output-on-failure` | P4-002 | Analytics remains opt-in with persisted consent and disable path. |
| Install layout | `VERIFIED` | `CMakeLists.txt`; `tools/ci/check_install_smoke.ps1`; `docs/packaging.md` | `./tools/ci/check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke` | P5-002 | Installed tree includes apps, runtime data, docs, metadata, and can launch runtime smoke. |
| Package layout | `VERIFIED` | `cmake/packaging.cmake`; `tools/ci/check_package_smoke.ps1`; `docs/release/RELEASE_PACKAGING.md` | `./tools/ci/check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke` | P5-002 | CPack emits component archives with expected runtime, data, docs, icon, and desktop metadata entries. |
| Legal docs | `VERIFIED` | `THIRD_PARTY_NOTICES.md`; `EULA.md`; `PRIVACY_POLICY.md`; `CREDITS.md`; `CHANGELOG.md`; `docs/release/LEGAL_REVIEW_SIGNOFF.md` | `rg -n "WAIVED_BY_RELEASE_OWNER|OWNER_WAIVED_FOR_PUBLIC_RELEASE|RECORDED_BY_RELEASE_OWNER" docs/release/LEGAL_REVIEW_SIGNOFF.md docs/APP_RELEASE_READINESS_MATRIX.md` | P5-002 | Required docs exist and install; release owner recorded an explicit public-release waiver on 2026-04-30 and certifies shipped paid/licensed assets are usable in distributed games. This is not qualified legal counsel approval. Raw/vendor/source asset packs remain excluded unless source-specific license and attribution evidence is reviewed before inclusion. |
| Release-required asset hydration | `VERIFIED` | `.gitattributes`; `resources/icons/*.png`; `imports/manifests/asset_bundles/BND-001.json`; `imports/manifests/asset_bundles/BND-003.json`; `tools/ci/check_release_required_assets.ps1`; fresh no-smudge clone check | `./tools/ci/check_release_required_assets.ps1`; `git lfs ls-files` | P5-005; P6-002; P9-001 | Fresh clone from GitHub verifies release-required assets without an LFS waiver; current branch has zero tracked LFS pointers. Raw/source asset packs remain outside the release package path. Phase 9 adds checksumed/package-gated starter coverage for prototype sprite, UI frame/chrome, VFX proof, and cohesive UI skin categories while keeping UI sounds, environmental SFX, and BGM outside bundled release claims unless future cleared OGG bundles are promoted. |
| Final release candidate gate | `VERIFIED` | `tools/ci/run_release_candidate_gate.ps1`; `.github/workflows/ci-gates.yml`; `docs/release/AAA_RELEASE_READINESS_REPORT.md` | `./tools/ci/run_release_candidate_gate.ps1`; GitHub Actions run `25351794008` | P6-001; P6-002; P6-03; Phase 12 | Local unwaived gate passes through fresh-clone asset hydration, configure, build, PR tests, presentation validation, install smoke, package smoke, and component package generation; it passed on `main` at `60f61bd07179cc85a665695ddc2d543ff766eba0` with package artifacts under `build/release-candidate-package`. Remote manual workflow run `25351794008` passed on `main` at the same commit: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25351794008`. |
| Full local gate | `VERIFIED` | `tools/ci/run_local_gates.ps1`; `tools/docs/*`; `tools/ci/*`; CTest labels `pr`, `nightly`, `weekly` | `.\tools\ci\run_local_gates.ps1` | P6-002 | Local validation passes waiver checks, doc/readiness/truth/governance scripts, CMake completeness, debug and warnings-as-errors builds, PR tests, nightly tests, weekly compat tests, install smoke, package smoke, grid-part integration, and presentation release validation. |

## Open Release Blocks And External Constraints

| Blocker | Status | Required Resolution |
| --- | --- | --- |
| Repository-wide source/vendor LFS budget/access | `RESOLVED_FOR_CURRENT_BRANCH` | Current branch no-smudge fresh clone reports zero tracked LFS pointers. Do not depend on old branch history for source/vendor payload recovery; keep broad raw/source intake local and ignored unless selected assets are promoted through governed manifests. |
| Legal review | `VERIFIED` | Public release legal/privacy/distribution review is explicitly waived by the release owner in `docs/release/LEGAL_REVIEW_SIGNOFF.md`. This records owner acceptance of risk and distribution responsibility; it is not qualified legal counsel approval. |
| Release tagging | `APPROVED_TO_TAG` | The release owner approved public `v0.1.0` publication for the bounded package scope. Create the annotated tag and GitHub release from the final `main` release commit after this docs-only release record passes the final gate. |

## Verification

Use this query to confirm this matrix and task references remain discoverable:

```powershell
rg -n "APP_RELEASE_READINESS_MATRIX|P[0-6]-[0-9]{3}" docs
```
