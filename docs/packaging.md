# URPG Native Packaging

Status Date: 2026-05-04

This document defines the canonical native app package path for URPG. It packages the URPG runtime/editor applications. It does not replace the game export packager documented in `docs/release/RELEASE_PACKAGING.md`.

## Canonical Path

URPG native app packages are produced with CPack from the release build tree:

```powershell
cmake -S . -B build/dev-ninja-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build --preset dev-release --target urpg_runtime urpg_editor urpg_audio_smoke
cpack --config build/dev-ninja-release/CPackConfig.cmake -G ZIP -B build/package-smoke
```

The package version comes from the root CMake project version and must match the executable `--version` output.

Package identity metadata is owned in `cmake/packaging.cmake`:

| Field | Current value |
| --- | --- |
| Vendor | `URPG Project` |
| Homepage | `https://github.com/DocDamage/URPG-RPG-Game-Designer` |
| Contact | `URPG Project support via GitHub Issues: https://github.com/DocDamage/URPG-RPG-Game-Designer/issues` |

## Components

CPack emits component archives:

| Component | Contents |
| --- | --- |
| `Runtime` | `urpg_runtime`, `urpg_editor`, `urpg_audio_smoke`, SDL2 on Windows shared builds, and MinGW runtime DLLs when required. |
| `RuntimeData` | `content/level_libraries`, `content/readiness`, `content/schemas`, `content/templates`, and governed `imports/manifests`. |
| `Docs` | Root release/legal docs plus `docs/release/` and `docs/templates/`. |

Raw intake folders, duplicate source archives, and unpromoted asset dumps are not native app package contents.

The `Runtime` component also includes app identity resources under `share/icons/hicolor/256x256/apps/` and Linux desktop entries under `share/applications/`. Windows builds embed version metadata and app icons into `urpg_runtime.exe` and `urpg_editor.exe`.

## Smoke Check

Use the package smoke wrapper after configuring the release build tree:

```powershell
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

The smoke check runs CPack, verifies component archives exist, and checks that the required executable, runtime-data, icon, desktop-entry, and legal-document paths are present in the produced archives. It also rejects DevBootstrap marker paths/content so local export-smoke launchers cannot slip into native app packages.

Status on 2026-05-04: package metadata has final vendor, homepage, and GitHub Issues support contact values. Package
smoke must be rerun after the next release configure/build to regenerate archive metadata.

## Release Limits

External distribution remains subject to:

- release-owner waiver or qualified legal/privacy/distribution approval remains recorded,
- `THIRD_PARTY_NOTICES.md` remains aligned with shipped package contents,
- public privacy/contact language remains current,
- platform signing/notarization requirements in `docs/release/RELEASE_PACKAGING.md` are satisfied for final release
  artifacts.
