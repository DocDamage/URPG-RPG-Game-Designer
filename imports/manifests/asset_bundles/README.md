# asset_bundles

Governance directory for URPG asset and external repository intake.
See docs/PROGRAM_COMPLETION_STATUS.md (P3-02, P3-03) for program context.

`ExportPackager` may stage bundle manifests from this directory into `data.pck`, together with referenced promoted assets under `imports/normalized/`, but only when:
- the manifest is a `.json` bundle record other than `asset_bundle.schema.json`
- `bundle_state` is `promoted`
- referenced asset rows also have `status` set to `promoted`
- `promoted_relative_path` resolves to an existing repo-local file under `imports/normalized/`
- release-required rows are marked with `release_required: true`, `release_surfaces`, `license_cleared: true`, and `distribution: "bundled"`

Automatic project-wide asset discovery is still out of scope; this directory is a governed explicit allowlist, not a generic content scan root.

Release-required runtime and distribution surfaces are summarized in `content/fixtures/project_governance_fixture.json` under `releaseAssets`. `tools/ci/check_release_required_assets.ps1` validates those entries and is called from the release-candidate gate.

TD Sprint 04 added the first promoted allowlist records:

- `BND-001.json` - `SRC-002` prototype visual proof lane; release-required for title, map, and battle placeholder surfaces
- `BND-002.json` - `SRC-003` UI SFX proof lane; retained as a governed local/deferred audio record while WAV payloads are ignored and excluded from release-required packaging
- `BND-003.json` - `SRC-009` repo-generated Phase 9 release starter skin lane; release-required for UI frame/chrome, VFX proof, and cohesive skin metadata
- `BND-004.json` - `SRC-010` bounded CC0 starter promotion; release-eligible but not release-required, with the broader raw drop still quarantined
- `BND-006.json` - `SRC-010` bulk CC0/public-domain app-usable promotion; release-eligible but not release-required or default-bundled, with duplicate, source/tool/archive, invalid, and non-reviewed-license payloads still quarantined
- `BND-007.json` - `SRC-010` newly license-covered app-usable promotion; release-eligible but not release-required or default-bundled
- `BND-008.json` - `SRC-013` itch loose CC0 PNG promotion; release-eligible but not release-required or default-bundled

Current UI and audio release surfaces are satisfied by explicit fallback entries in `releaseAssets` until a non-LFS
bundled audio asset is approved. The release-required asset gate enforces this policy: the audio surface must be covered
by either an explicit muted/silent `system_fallback` `audio_policy` row or a bundled release-required audio asset.
