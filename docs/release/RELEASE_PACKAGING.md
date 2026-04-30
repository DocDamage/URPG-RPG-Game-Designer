# Release Packaging

Status Date: 2026-04-30

This document records the native release packaging contract for URPG exports. It does not claim that production signing credentials are present in the repository.

## Packaging Command

Use the release packaging wrapper after building `urpg_pack_cli`:

```powershell
.\tools\ci\package_release_artifacts.ps1 -Mode DevUnsigned -BuildDirectory build/dev-ninja-debug
```

For release packaging:

```powershell
.\tools\ci\package_release_artifacts.ps1 -Mode Release -BuildDirectory build/ci -ReportPath build/release-packages/release_package_report.json
```

## Modes

`DevUnsigned` is the only mode allowed to emit unsigned artifacts. The generated report must show `unsignedArtifactsAllowed: true` and each artifact must carry an `unsignedReason`.

Export-packager smoke outputs that synthesize runtime launchers are explicitly marked as `DevBootstrap`. They include a `DevBootstrap/export_mode.json` sidecar with `productionPlayable: false` and `releaseEligible: false`. These artifacts are valid only for local export smoke tests and must not be promoted into native app release packages.

Release packaging also requires the curated RPG Maker plugin release manifest:

```text
imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_release_manifest.json
```

That manifest records the one packaged source per plugin key and the raw DLC candidates excluded as duplicate copies, localized sample copies, or conflicting implementations.

`Release` mode fails before export packaging if required signing or notarization inputs are missing. This prevents a release package from silently shipping unsigned native artifacts.

## Required Signing Inputs

| Target | Required inputs | Hook |
| --- | --- | --- |
| `Windows_x64` | `URPG_WINDOWS_SIGN_CERT_PATH`, `URPG_WINDOWS_SIGN_CERT_PASSWORD` | `signtool` signing hook |
| `Linux_x64` | `URPG_LINUX_SIGNING_KEY_PATH` | detached/package signature hook |
| `macOS_Universal` | `URPG_MACOS_DEVELOPER_ID_APPLICATION`, `URPG_MACOS_NOTARY_PROFILE` | `codesign` and `notarytool` hook |
| `Web_WASM` | none | bundle signature and manifest integrity only |

The current wrapper records the signing/notarization plan and enforces credential presence for release mode. It does not fabricate signatures when the platform tools or credentials are absent.

## Report

The JSON report includes:

- packaging mode
- dry-run status
- required signing inputs
- missing signing inputs
- export matrix result when not in dry run
- per-target artifact signing/notarization status
- explicit unsigned reason for development packages

## Verification

Development dry run:

```powershell
.\tools\ci\package_release_artifacts.ps1 -Mode DevUnsigned -DryRun -Json
```

Release failure path without credentials:

```powershell
.\tools\ci\package_release_artifacts.ps1 -Mode Release -DryRun -Json
```

The release failure path must exit non-zero and report the missing signing inputs.

## Installed App Layout

`cmake --install` is the native app-layout smoke path used by CPack. It installs only release-facing runtime/editor files and governed manifests; raw intake folders, duplicate source archives, and unpromoted third-party asset dumps are not installed.

Expected install tree:

```text
<prefix>/
  bin/
    urpg_runtime[.exe]
    urpg_editor[.exe]
    urpg_audio_smoke[.exe]
    SDL2.dll and MinGW runtime DLLs where required on Windows
  share/urpg/
    content/
      level_libraries/
      readiness/
      schemas/
      templates/
    imports/manifests/
      asset_bundles/
      asset_sources/
  share/doc/urpg/
    README.md
    LICENSE
    CHANGELOG.md
    PRIVACY_POLICY.md
    THIRD_PARTY_NOTICES.md
    EULA.md
    CREDITS.md
    release/
    templates/
  share/icons/hicolor/256x256/apps/
    urpg_runtime.png
    urpg_editor.png
  share/applications/
    urpg-runtime.desktop
    urpg-editor.desktop
```

Root legal documents are required package contents. `README.md`, `LICENSE`, `CHANGELOG.md`, `PRIVACY_POLICY.md`, `THIRD_PARTY_NOTICES.md`, `EULA.md`, and `CREDITS.md` are installed with the `Docs` component and checked by install/package smoke. Their presence does not clear public-release legal status; `docs/APP_RELEASE_READINESS_MATRIX.md` and `docs/release/RELEASE_READINESS_MATRIX.md` remain authoritative for `PARTIAL` or `BLOCKED` legal-review gates.

## Version Metadata

Release configure writes the native version header from `cmake/urpg_version.h.in` to the build tree and CPack reads the same `PROJECT_VERSION` values for archive names and package metadata. On Windows, `resources/windows/urpg_runtime.rc.in` and `resources/windows/urpg_editor.rc.in` are the resource-version templates when resource compilation is enabled for a path-safe toolchain layout.

Current release-build CLI evidence:

```powershell
.\build\dev-ninja-release\urpg_runtime.exe --version
.\build\dev-ninja-release\urpg_editor.exe --version
```

Both commands report `0.1.0` for this checkpoint, matching `build/dev-ninja-release/generated/urpg_version.h` and CPack archive names such as `URPG-0.1.0-Windows-AMD64-Runtime.zip`.

## Install Smoke

After building the release preset:

```powershell
cmake --build --preset dev-release
cmake --install build/dev-ninja-release --prefix build/install-smoke --component Runtime
cmake --install build/dev-ninja-release --prefix build/install-smoke --component RuntimeData
cmake --install build/dev-ninja-release --prefix build/install-smoke --component Docs
.\build\install-smoke\bin\urpg_runtime.exe --headless --frames 1 --project-root .\build\install-smoke\share\urpg
```

The CI wrapper performs the same install layout checks and runtime launch:

```powershell
.\tools\ci\check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke
```

Status on 2026-04-30: passed after the release runtime target was fixed to include the editor panel registry metadata required by AI knowledge/chatbot coverage.

## Native App Packages

Native URPG app packages are produced with CPack from the install components above. See `docs/packaging.md` for the canonical command.

Package smoke:

```powershell
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

The package smoke gate rejects any archive containing `DevBootstrap` marker paths or bootstrap-only marker content, so release package archives cannot silently include export-packager smoke launchers.

Status on 2026-04-30: passed and produced component ZIP archives for `Runtime`, `RuntimeData`, and `Docs`.

## Release Asset Gate

Release-required assets are checked before package promotion:

```powershell
.\tools\ci\check_release_required_assets.ps1
```

The gate validates the release asset block in `content/fixtures/project_governance_fixture.json`, promoted bundle rows under `imports/manifests/asset_bundles/`, app icons under `resources/icons/`, and repo-local promoted payloads under `imports/normalized/`. Required assets must resolve inside the repo, must not point at raw/vendor intake trees, must not be unresolved LFS pointers, and must carry license-cleared release metadata.

Individual promoted asset readiness can also be represented by `content/schemas/asset_promotion_manifest.schema.json`. Those records expose the source path, promoted path, license id, preview metadata, runtime package inclusion, release-required state, and diagnostics to the asset library/editor rows. Packaging semantics remain fail-closed: runtime-ready records require a promoted path, included runtime assets require license evidence, release-required assets must be included in runtime packages, and archived assets are not packageable.

The gate also writes an audit report:

```text
imports/reports/asset_intake/release_required_asset_report.json
```

That report classifies connected release assets and non-connected bundle rows. Current UI/audio release surfaces use explicit system fallback entries, while `BND-002` UI SFX remains `deferred` until an approved bundled audio asset is promoted.

## Full Local Gate Status

The release-surface remediation branch was verified with the full local gate:

```powershell
.\tools\ci\run_local_gates.ps1
```

Status on 2026-04-30: passed end to end. The gate covers waiver validation, doc/readiness/truth/governance checks, CMake completeness, debug and warnings-as-errors builds, PR tests, nightly tests, weekly compat tests, install smoke, package smoke, grid-part integration, and presentation release validation.
