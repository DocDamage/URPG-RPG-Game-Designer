# Technical Debt Audit Sprint Checklist - 2026-04-24

Status: active follow-up plan  
Source: manual technical debt audit findings from the current `codex/refresh-technical-debt-audit-2026-04-22` worktree  
Scope: convert the five current audit findings into bounded sprint work with check boxes, owners, validation, and exit criteria

This file is an execution checklist, not a new source of truth. If a sprint changes readiness status, update the canonical status stack in the same change:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TECHNICAL_DEBT_ACTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `WORKLOG.md`

## Global Done Rules

- [ ] Code behavior is fixed, or the claim is narrowed to match reality.
- [ ] Positive and negative tests cover the changed behavior.
- [ ] Relevant CTest lane passes with anchored labels where applicable.
- [ ] `urpg_project_audit` output still reflects the correct release/export posture.
- [ ] Canonical docs and worklog evidence match the commands that were actually run.
- [ ] No sprint is marked complete while a known gate for that sprint is red.

## Sprint Map

| Sprint | Priority | Theme | Primary Finding |
| --- | --- | --- | --- |
| `TD-AUD-01` | P1 | Restore nightly visual gate | Finding 1 |
| `TD-AUD-02` | P2 | Replace export license audit stub | Finding 2 |
| `TD-AUD-03` | P3 | Harden bundle write failure handling | Finding 3 |
| `TD-AUD-04` | P3 | Make bundle offsets size-safe | Finding 4 |
| `TD-AUD-05` | P2 | Refresh stale audit/status docs | Finding 5 |
| `TD-AUD-06` | P2 | Worktree and release hygiene sweep | Cross-cutting audit risk |

## TD-AUD-01 - Restore Nightly Visual Gate

### Objective

Make `ctest --preset dev-all -L "^nightly$" --output-on-failure` green again by reconciling the `Window_Base` gauge rendering change with its committed renderer-backed golden.

### Checklist

- [ ] Inspect the visual difference for `RendererBackedCapture_window_status_gauge_crop.golden.json`.
- [ ] Decide whether the new segmented gauge output in `runtimes/compat_js/window_compat.cpp` is intended behavior.
- [ ] If intended, regenerate and commit the correct golden for `RendererBackedCapture/window_status_gauge_crop`.
- [ ] If unintended, adjust `Window_Base::drawGauge` so it matches the existing golden and RPG Maker compatibility expectations.
- [ ] Add or update a focused test assertion that documents the expected gauge-gradient contract.
- [ ] Re-run the single failing test:
  `ctest --preset dev-all --output-on-failure -R "Window_Base status and gauge surface"`
- [ ] Re-run the full anchored nightly lane:
  `ctest --preset dev-all -L "^nightly$" --output-on-failure`
- [ ] Record the final command output in `WORKLOG.md`.

### Exit Criteria

- [ ] Finding 1 is closed.
- [ ] Anchored nightly lane passes.
- [ ] The visual regression golden and implementation describe the same contract.

## TD-AUD-02 - Replace Export License Audit Stub

### Objective

Make `ExportPackager::runLicenseAudit` enforce real asset-license safety, or explicitly downgrade export-governance claims until enforcement exists.

### Checklist

- [ ] Trace the existing `AssetLicenseAuditor` API and test coverage.
- [ ] Define the export-time inputs needed by the auditor: promoted bundle manifests, normalized assets, auto-discovered asset roots, and any license metadata.
- [ ] Replace the unconditional `return true` in `engine/core/tools/export_packager.cpp`.
- [ ] Ensure export fails closed on missing, malformed, forbidden, or unknown license evidence.
- [ ] Surface license-audit failures through `ExportResult::errors` and pack CLI JSON.
- [ ] Add tests for allowed assets, missing license metadata, disallowed license, and malformed manifest cases.
- [ ] Update docs that currently imply asset governance is enforced.
- [ ] Re-run:
  `ctest --preset dev-all --output-on-failure -R "AssetLicenseAuditor|ExportPackager|urpg_pack_cli|Project audit CLI"`

### Exit Criteria

- [ ] Finding 2 is closed.
- [ ] Export cannot succeed when license governance fails.
- [ ] `urpg_project_audit` and export docs no longer overclaim asset-governance enforcement.

## TD-AUD-03 - Harden Bundle Write Failure Handling

### Objective

Make `ExportPackager::bundleAssets` report write failures deterministically instead of returning `data.pck` after partial or failed output.

### Checklist

- [ ] Check stream state after writing the magic, manifest length, manifest body, and each payload.
- [ ] Flush and close the output stream before reporting success.
- [ ] Return an empty bundle result or a structured error path when any write fails.
- [ ] Consider writing to a temporary file and atomically renaming after successful close.
- [ ] Add a test seam for simulated write failure or invalid output target.
- [ ] Add regression coverage proving `data.pck` is not reported after a failed write.
- [ ] Re-run:
  `ctest --preset dev-all --output-on-failure -R "ExportPackager|urpg_pack_cli"`

### Exit Criteria

- [ ] Finding 3 is closed.
- [ ] Partial bundle writes fail loudly and do not masquerade as valid exports.

## TD-AUD-04 - Make Bundle Offsets Size-Safe

### Objective

Prevent large bundles from wrapping 32-bit payload offsets or manifest sizes.

### Checklist

- [ ] Decide the bundle format contract: explicit 4 GiB max or 64-bit offsets.
- [ ] If keeping 32-bit offsets, add preflight checks for per-payload size, total payload size, and manifest size.
- [ ] If moving to 64-bit offsets, update manifest schema, writer, reader/validator, and compatibility docs together.
- [ ] Add tests for near-limit and over-limit payload collections.
- [ ] Ensure over-limit bundles fail with actionable error messages.
- [ ] Re-run:
  `ctest --preset dev-all --output-on-failure -R "ExportPackager|urpg_pack_cli"`

### Exit Criteria

- [ ] Finding 4 is closed.
- [ ] Large export behavior is deterministic, documented, and tested.

## TD-AUD-05 - Refresh Stale Audit And Status Docs

### Objective

Bring the audit/status documentation back in line with the current worktree, test discovery count, and actual gate results.

### Checklist

- [ ] Update `docs/TECHNICAL_DEBT_AUDIT_2026-04-23.md` or supersede it with a clearly dated current audit.
- [ ] Remove or correct claims that git status is clean.
- [ ] Update CTest discovery from `1,220` to the current discovered count.
- [ ] Record that renderer-backed snapshot execution was run.
- [ ] Record that anchored PR and weekly passed, and anchored nightly failed until `TD-AUD-01` closes.
- [ ] Update `WORKLOG.md` so validation claims use anchored labels and match actual commands.
- [ ] Reconcile any related status language in `docs/PROGRAM_COMPLETION_STATUS.md`, `docs/RELEASE_READINESS_MATRIX.md`, and `docs/TECHNICAL_DEBT_ACTION_PLAN.md`.
- [ ] Re-run truth/documentation checks:
  `.\tools\ci\truth_reconciler.ps1`

### Exit Criteria

- [ ] Finding 5 is closed.
- [ ] No current status document claims a green gate or clean tree that is not true.

## TD-AUD-06 - Worktree And Release Hygiene Sweep

### Objective

Resolve the cross-cutting release hygiene risk from a large dirty worktree with required-looking untracked files.

### Checklist

- [ ] Classify every untracked file as required source/test/asset/golden, generated artifact, temporary artifact, or accidental output.
- [ ] Stage required files together with the code/tests that reference them.
- [ ] Remove or ignore accidental local artifacts.
- [ ] Confirm new tests and CMake entries do not reference files that remain untracked.
- [ ] Confirm large visual golden files are intentional and governed by LFS or repo policy.
- [ ] Re-run:
  `git status --short --branch`
- [ ] Re-run:
  `.\tools\ci\check_cmake_completeness.ps1`
- [ ] Re-run CTest discovery:
  `ctest --preset dev-all -N`

### Exit Criteria

- [ ] The worktree contains only intentional changes.
- [ ] Required source, test, asset, and golden files are tracked.
- [ ] Release evidence can be reproduced from a fresh checkout.

## Recommended Execution Order

1. `TD-AUD-01` first, because nightly is currently red.
2. `TD-AUD-05` next, so docs stop lying about the current state.
3. `TD-AUD-02`, because the license-audit stub undermines export governance.
4. `TD-AUD-03` and `TD-AUD-04` together, because both touch bundle writing.
5. `TD-AUD-06` before staging or opening a PR, so the branch is reproducible.

