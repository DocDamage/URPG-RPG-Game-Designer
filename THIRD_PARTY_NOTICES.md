# Third-Party Notices

Status Date: 2026-04-27

This notice records third-party material known to the URPG repository and the current release install layout. It is not a legal opinion. External distribution requires review by a qualified legal reviewer before publication.

## Shipped By `cmake --install`

| Component | Source | License / Terms | Installed Path | Notes |
| --- | --- | --- | --- | --- |
| URPG engine/editor/runtime source | This repository | MIT, see `LICENSE` | `bin/`, `share/doc/urpg/` | Primary project license. |
| SDL2 | `https://github.com/libsdl-org/SDL.git`, tag `release-2.30.0` | zlib license, verify against upstream release text before external publication | `bin/SDL2.dll` on Windows shared builds | Fetched by CMake when system SDL2 is unavailable. |
| MinGW runtime DLLs | Active MinGW toolchain | GNU runtime libraries with applicable runtime-library exceptions; verify exact toolchain notices before external publication | `bin/libgcc_s_seh-1.dll`, `bin/libstdc++-6.dll`, `bin/libwinpthread-1.dll` on MinGW Windows builds | Installed only when the DLLs are present beside the active compiler. |
| nlohmann/json | `https://github.com/nlohmann/json.git` | MIT | Linked into URPG binaries | Fetched by CMake when not found as a package. |
| Dear ImGui | `https://github.com/ocornut/imgui.git`, tag `v1.92.6` | MIT | Linked into URPG binaries | Editor UI dependency. |
| stb | `https://github.com/nothings/stb.git`, commit `f58f558c120e9b32c217290b80bad1a0729fbb2c` | Public domain or MIT option; verify upstream text before external publication | Linked into URPG binaries | Image loading dependency. |
| QuickJS-NG | Fetched by CMake when `URPG_FETCH_QUICKJS_NG=ON` | Upstream license must be verified from the fetched source before external publication | Linked into URPG binaries | Compatibility scripting runtime. The app install intentionally does not install `qjs`/`qjsc` tools. |
| Catch2 | `https://github.com/catchorg/Catch2.git`, tag `v3.6.0` | BSL-1.0 | Not installed in runtime components | Test-only dependency when `URPG_BUILD_TESTS=ON`. |

## Shipped Data And Manifests

| Data | Source | License / Terms | Installed Path | Notes |
| --- | --- | --- | --- | --- |
| URPG schemas, readiness data, starter templates, and level library records | This repository | MIT, see `LICENSE` | `share/urpg/content/` | Runtime/editor release data. |
| Asset source and bundle manifests | This repository intake records | Manifest text is MIT; referenced assets retain their original upstream terms | `share/urpg/imports/manifests/` | Manifests are shipped for provenance. Raw source assets are not installed by P5-001 install rules. |
| `SRC-002` release-required promoted visual proof lane | GDQuest/game-sprites, captured snapshot `ea05c63eb1d88af928d2d9a7445879500c9ece3f` | Recorded as `cc0_candidate_recorded_for_private_use_intake`; legal review required before external publication | Bundled in `data.pck` through `BND-001`; promoted asset path recorded as `imports/normalized/prototype_sprites/gdquest_blue_actor.svg` | Release-required for title, map, and battle placeholder surfaces. See `imports/manifests/asset_sources/SRC-002.json`, `imports/manifests/asset_bundles/BND-001.json`, and `content/fixtures/project_governance_fixture.json`. |
| `SRC-003` deferred UI SFX proof lane | Calinou/kenney-interface-sounds, captured snapshot `4596a49eaf5a533948d49a47467f606bcdea70ff` | Recorded as `cc0_candidate_recorded_for_private_use_intake`; legal review required before external publication | Not release-required and not bundled; local WAV payloads under `imports/normalized/ui_sfx/` are ignored from GitHub | The release UI/audio surfaces use explicit silent/muted fallback policy entries until an approved non-LFS audio artifact exists. See `imports/manifests/asset_sources/SRC-003.json` and `imports/manifests/asset_bundles/BND-002.json`. |

## Repository-Only Intake And Reference Material

These paths are present in the working repository but are not installed by the native app layout added in P5-001:

- `third_party/itch-assets/`
- `third_party/rpgmaker-mz/`
- `third_party/aseprite/`
- `third_party/huggingface/`
- `third_party/github_assets/`
- `imports/raw/`
- `imports/staging/`
- `imports/normalized/`

Do not treat these folders as redistributable product content unless a source-specific license record, attribution record, and release approval exist. The current install layout ships manifests for provenance, not the raw asset packs.

## Unverified Legal Items

- Legal sufficiency of this notice is unverified.
- Exact upstream license texts for fetched dependencies must be bundled or referenced from reviewed release artifacts before public distribution.
- Toolchain runtime redistribution terms must be checked for the compiler used to build the final package.
- Any future asset promotion or public-release waiver must update this file, `CREDITS.md`, `docs/release/LEGAL_REVIEW_SIGNOFF.md`, and the relevant `imports/manifests` records before packaging.
