# URPG Maker

URPG Maker is a native-first RPG engine and editor for deterministic, data-driven RPGs. It combines a C++20 runtime, an ImGui editor, a release-wired Level Builder, OpenGL and headless validation paths, bounded RPG Maker MZ compatibility tooling, governed asset intake, native package validation, and review-gated AI-assisted editing.

The project is built for creators who want RPG Maker-style production speed without giving up native runtime ownership, source-control-friendly project data, diagnostics, packaging governance, and explicit release gates.

## Current Status

Status date: 2026-05-04

The current `codex/release-surface-p0` branch has completed the release-surface audit remediation plan through Phase 6 Task P6-002. The full local gate now passes end to end:

```powershell
.\tools\ci\run_local_gates.ps1
```

This means the current branch has verified editor release navigation, Level Builder exposure, release authoring persistence, input and pause behavior, release-required asset checks, install/package smoke, PR tests, nightly tests, weekly compat tests, and documentation/readiness guard scripts.

Public release is release-owner-waived for legal/privacy review, not qualified-counsel-approved. `EULA.md`,
`PRIVACY_POLICY.md`, and `docs/release/LEGAL_REVIEW_SIGNOFF.md` now state the same distribution scope. Final public
distribution still requires a release tag and any platform-specific signing/notarization credentials required for final
distribution.

## Verified Release Surface

- Release top-level editor panels are intentionally limited to `diagnostics`, `assets`, `ability`, `patterns`, `mod`, `analytics`, and `level_builder`.
- Every release top-level panel has an app-shell factory and is covered by registry/app panel regression tests.
- `developer_debug_overlay` and other debug/dev surfaces are `DevOnly`, excluded from release navigation, and tested as such.
- Deferred editor panels remain compiled for direct tests, snapshots, or roadmap work, but are not advertised as release navigation.
- The native `level_builder` panel is wired into the editor shell and uses the real grid-part Level Builder workspace.
- The release inventory, app readiness matrix, and editor panel registry are now cross-checked by tests instead of maintained only by convention.

## What Is Release-Ready In This Branch

Within the bounded internal/private release-candidate scope:

- Native runtime startup, title/menu input, scene stack pause/resume, settings, save/load, audio startup diagnostics, localization startup diagnostics, and runtime asset preflight have focused coverage.
- Editor release navigation starts headlessly, lists panels, and opens `level_builder`.
- Ability draft save/load/apply, ability project-content save, pattern editing, Level Builder save/load/export/playtest/package, analytics consent/local JSONL export, and app settings persistence are covered by deterministic tests.
- Release-required assets are validated by `tools/ci/check_release_required_assets.ps1`; raw/vendor intake paths are not eligible release payloads.
- Current release visuals are bounded starter/proof assets, not final AAA art direction. The release asset gate requires this scope to be declared before prototype actor, starter UI skin/chrome, or VFX proof rows can satisfy release coverage.
- Cloud sync is not a production-visible release feature in the shipped tree. `LocalInMemoryCloudService` is process-local
  test/dev storage only, and release UI must keep cloud/cross-device sync hidden unless an out-of-tree provider reports a
  reviewed remote transport through `ICloudService::releaseVisibility()`.
- Native release builds expose version metadata through `urpg_runtime.exe --version` and `urpg_editor.exe --version`.
- Install and package smoke checks verify app binaries, runtime data, docs/legal files, icons, desktop entries, and component ZIP archives.
- Compat diagnostics distinguish successful execution diagnostics from failure diagnostics in both model and live panel refresh paths.

## Still Blocked For Public Release

- Release-owner legal/privacy waiver is recorded in `docs/release/LEGAL_REVIEW_SIGNOFF.md`; qualified public legal review has not been performed.
- Public distribution approval and release tagging.
- Platform signing/notarization credentials for final release artifacts.
- Repository-wide source/vendor LFS budget/access if future release work depends on full vendor/source asset hydration. Current release-required app assets are normal Git blobs and are checked separately.

## Product Pillars

### Native Level Builder

The Level Builder is the shippable map authoring surface. It is registered as the `level_builder` editor workspace and owns normal grid-part map building.

Implemented capabilities include:

- Build, Validate, Playtest, Package, and Supporting Spatial workflow modes.
- `GridPartDocument` source-of-truth editing.
- Palette-driven part selection and deterministic grid placement.
- Inspector selection, property editing, command history, undo, redo, and diagnostic focus.
- Save draft, load draft, export current level, playtest from start, and return-to-editor flows.
- Spawn/objective authoring commands and package-readiness evidence commands.
- Deterministic JSON serialization, malformed-load rejection, map-id mismatch rejection, and runtime compile validation.
- Supporting spatial handoff for elevation, props, ability binding, and composite spatial tools without making legacy spatial authoring the primary editor.

Key files:

- `editor/spatial/level_builder_workspace.*`
- `editor/spatial/grid_part_*_panel.*`
- `engine/core/map/grid_part_*`
- `content/schemas/grid_part_*.schema.json`
- `tests/unit/test_grid_part_editor.cpp`

### Native Runtime Core

URPG Maker includes deterministic native systems for scene flow, map, battle, menu, save/load, settings, abilities, events, dialogue, quests, relationships, crafting, encounters, loot, NPCs, character creation, achievements, mods, audio, accessibility, analytics consent, input, localization, and template data models.

Runtime diagnostics cover project health, assets, plugins, map rendering, battle state, ability state, package readiness, and export validation.

### Editor And Diagnostics

Editor behavior is expected to be visible, testable, and diagnostic-rich. Release panels expose disabled, empty, error, loading, and ready states through deterministic snapshots or JSON exports. Dev-only and deferred surfaces remain available for development or direct tests but are not release navigation.

### RPG Maker MZ Compatibility

The compatibility layer is bounded and diagnostic-first:

- QuickJS harness under `runtimes/compat_js/`.
- Plugin command registration, execution, reload, dependency gates, manifest parsing, compatibility scoring, and diagnostics.
- RPG Maker MZ import/migration support where native contracts exist.

Compatibility is an import, validation, migration, and diagnostic harness. It is not claimed as full live RPG Maker JavaScript runtime parity.

### Asset Intake

Asset handling is governed:

- Raw intake: `imports/raw/`
- Normalized assets: `imports/normalized/`
- Source manifests: `imports/manifests/asset_sources/`
- Bundle manifests: `imports/manifests/asset_bundles/`
- Reports: `imports/reports/asset_intake/`
- Local ignored asset DB: `.urpg/asset-index/`

Raw assets are quarantine/catalog inputs. They are not release-export eligible until curated subsets receive attribution, bundle manifests, and promotion approval.

### Export, Packaging, And Governance

Release packaging makes blockers explicit:

- CMake install layout for runtime/editor/data/docs.
- CPack component archives for `Runtime`, `RuntimeData`, and `Docs`.
- Version metadata shared by executables and package names.
- Install and package smoke scripts under `tools/ci/`.
- Release-required asset gate that rejects raw/vendor paths, unresolved LFS pointers, and uncleared required metadata.
- Release mode signing/notarization checks that fail instead of fabricating credentials.

### AI-Assisted Editing

AI-assisted editing is review-gated:

- `IChatService` abstraction and deterministic local/mock transport paths.
- OpenAI-compatible transport profiles for hosted and local providers.
- Reviewable plans before mutation.
- Persisted `_ai_change_history` with forward and reverse JSON patches.
- Shared apply/revert controls across AI assistant and chatbot snapshots.
- Knowledge indexing for project files, docs, schemas, readiness reports, validation reports, asset catalogs, and template specs.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `apps/` | Runtime and editor entry points. |
| `engine/core/` | Native runtime systems and data models. |
| `engine/api/` | Public runtime API entry points. |
| `editor/` | ImGui panels, editor models, diagnostics, and workspaces. |
| `runtimes/compat_js/` | QuickJS and RPG Maker MZ compatibility surfaces. |
| `content/` | Schemas, fixtures, templates, abilities, catalogs, and readiness data. |
| `resources/` | Release-required app resources. |
| `imports/raw/` | Quarantined raw local, third-party, itch, RPG Maker, and tool intake. |
| `imports/normalized/` | Promoted or metadata-normalized assets. |
| `imports/manifests/` | Source and bundle manifests. |
| `imports/reports/` | Intake, attribution, validation, and audit reports. |
| `tests/` | Unit, integration, snapshot, compat, and engine tests. |
| `tools/` | CI, docs, assets, packaging, migration, and workflow scripts. |
| `docs/` | Architecture, release, governance, signoff, ADRs, status, and templates. |
| `.urpg/` | Ignored local cache/archive/state. |

The old root-level `third_party/` and `itch/` folders are retired. Ingested content belongs under `imports/raw/third_party_assets/` and `imports/raw/itch_assets/`.

## Build

Recommended Windows/Ninja debug build:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
```

Release build used by install/package smoke:

```powershell
cmake --preset dev-ninja-release
cmake --build --preset dev-release
```

## Test And Validate

Full local gate:

```powershell
.\tools\ci\run_local_gates.ps1
```

PR, nightly, and weekly lanes:

```powershell
ctest --preset dev-all -L pr --output-on-failure
ctest --preset dev-all -L nightly --output-on-failure
ctest --preset dev-all -L weekly --output-on-failure
```

Release-surface regression lane:

```powershell
ctest --test-dir build\dev-ninja-debug -R "Editor panel registry|editor app panels|urpg_editor_smoke|urpg_editor_list_panels|curated save-data lifecycle" --output-on-failure
```

Level Builder / grid-part lane:

```powershell
ctest --test-dir build\dev-ninja-debug -L grid_part --output-on-failure
```

Release asset, install, and package gates:

```powershell
.\tools\ci\check_release_required_assets.ps1
.\tools\ci\check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

Version metadata:

```powershell
.\build\dev-ninja-release\urpg_runtime.exe --version
.\build\dev-ninja-release\urpg_editor.exe --version
```

## Documentation Map

- [Agent Knowledge Index](./docs/agent/INDEX.md)
- [Architecture Map](./docs/agent/ARCHITECTURE_MAP.md)
- [Quality Gates](./docs/agent/QUALITY_GATES.md)
- [Known Debt](./docs/agent/KNOWN_DEBT.md)
- [Program Completion Status](./docs/PROGRAM_COMPLETION_STATUS.md)
- [App Release Readiness Matrix](./docs/APP_RELEASE_READINESS_MATRIX.md)
- [Release Readiness Matrix](./docs/release/RELEASE_READINESS_MATRIX.md)
- [Editor Control Inventory](./docs/release/EDITOR_CONTROL_INVENTORY.md)
- [Release Packaging](./docs/release/RELEASE_PACKAGING.md)
- [Native Packaging](./docs/packaging.md)
- [AI Copilot Guide](./docs/integrations/AI_COPILOT_GUIDE.md)
- [Asset Intake Registry](./docs/asset_intake/ASSET_SOURCE_REGISTRY.md)
- [Asset Promotion Guide](./docs/asset_intake/ASSET_PROMOTION_GUIDE.md)
- [Template Specs](./docs/templates/)

## Development Rules

- Keep release claims truthful and evidence-backed.
- Prefer existing subsystem patterns over new abstractions.
- Update tests and docs when behavior changes.
- Update schema changelogs when schemas change.
- Keep raw intake separate from promoted release assets.
- Treat `imports/raw/` as quarantine/catalog input, not automatic shipping content.
- Use focused verification for narrow changes and broader gates for release-facing changes.
- Do not mark public release readiness until legal/privacy/distribution exits are closed or formally waived.
