# Release Packaging

Status Date: 2026-04-25

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

Release packaging also requires the curated RPG Maker plugin release manifest:

```text
third_party/rpgmaker-mz/steam-dlc/reports/plugin_dropins_release_manifest.json
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
