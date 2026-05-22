# Modern UI Asset Ingest Split Plan

Status date: 2026-05-22

The branch `codex/modern-ui-asset-ingest` contains useful editor, catalog, onboarding, and asset work, but it is too large to merge directly. Draft PR #22 exists only as a quarantine/reference PR for the full branch.

## Why Direct Merge Is Blocked

PR #22 changes roughly 94,904 files and adds more than 4 million lines. That size makes normal file review, changed-file listing, and CI diagnosis unreliable. It also risks moving raw/generated gameplay assets into `main` before package, LFS, release-required asset, and attribution behavior is proven.

## Split Order

1. `codex/modern-ui-asset-indexes` / PR #23
   - Metadata-only game-maker starter asset indexes.
   - No PNG payloads.

2. `codex/modern-ui-shell-core-split` / PR #24
   - Header/API surface for maker-shell main menu and grid-part catalog scope loading.
   - No implementation wiring or asset payloads.

3. Shell implementation split
   - Add `editor/project/main_menu_panel.cpp`.
   - Add `engine/core/map/grid_part_catalog_loader.cpp`.
   - Add focused tests such as `tests/unit/test_main_menu_panel.cpp`.
   - Update `CMakeLists.txt` in the same PR.
   - This requires normal git patch tooling because the root CMake file is large.

4. Curated starter payload split
   - Add only the tiny PNG subset directly referenced by starter indexes.
   - Update `.gitattributes` only for those explicitly approved files.
   - Run release-required asset and package smoke checks.

5. Bulk payload quarantine
   - Keep broad generated/raw gameplay assets out of `main` until each source has attribution, package policy, and release eligibility evidence.

## Merge Rule

Do not mark PR #22 ready for review and do not merge it as-is. Extract small, auditable PRs until no useful work remains in the bulk branch.

## Required Checks For Payload Splits

Any PR that adds binary gameplay assets must pass or explicitly document:

- `tools/ci/check_release_required_assets.ps1`
- install/package smoke
- LFS pointer review
- attribution/source-manifest review
- release scope statement explaining whether assets are default-shipped, opt-in, or local-only
