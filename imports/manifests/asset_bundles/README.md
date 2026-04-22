# asset_bundles

Governance directory for URPG asset and external repository intake.
See docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md (P3-02, P3-03) for program context.

`ExportPackager` may stage bundle manifests from this directory into `data.pck`, together with referenced promoted assets under `imports/normalized/`, but only when:
- the manifest is a `.json` bundle record other than `asset_bundle.schema.json`
- `bundle_state` is `promoted`
- referenced asset rows also have `status` set to `promoted`
- `promoted_relative_path` resolves to an existing repo-local file under `imports/normalized/`

Automatic project-wide asset discovery is still out of scope; this directory is a governed explicit allowlist, not a generic content scan root.
