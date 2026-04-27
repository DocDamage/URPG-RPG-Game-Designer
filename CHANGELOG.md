# Changelog

All notable changes to URPG are tracked here.

## Unreleased

- Ran the P6 release-readiness verification pass, including pre-commit, local gates, presentation/visual regression, and the release-candidate gate.
- Removed the small runtime/editor release icon PNGs from Git LFS so release package metadata no longer depends on the exhausted repository LFS budget.
- Updated the AAA release-readiness report to reflect closed app/runtime/editor/package/release-required asset blockers and the remaining legal, remote workflow, and tag blockers.
- Restored root-level machine-contract governance docs required by existing CI scripts after the docs organization pass.
- Fixed CI smoke checks for generator-expression source entries and Debug SDL runtime DLL names.
- Added CMake project version metadata and shared application version reporting.
- Added `--version` support for shipped application entry points.
- Added strict runtime/editor CLI parsing with `--help`, unknown-option errors, and missing-value diagnostics.
- Added componentized native install rules, an install-smoke CTest lane, and release packaging layout documentation.
- Added first-pass third-party notices, credits, and internal-only EULA placeholders with explicit legal-review requirements.
