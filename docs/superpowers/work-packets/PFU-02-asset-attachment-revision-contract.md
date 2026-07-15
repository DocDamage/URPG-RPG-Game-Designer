# PFU-02 Work Packet: governed asset-attachment revision contract

**Status:** Implemented bounded revision/idempotency, creator review/confirm, staged rollback publication, and conservative project-local interruption recovery

**Date:** 2026-07-15

## Scope

Add source-revision checking and idempotent operation identity to the existing native `ProjectAssetAttachmentService`. This is a bounded PFU-02 foundation slice; it does not add a generic transaction store, transform studio, Map placement changes, or cross-domain undo.

## Traceability and owners

- **Source outcomes:** F02 Visual Asset Library; F21 Personal Asset Vault; I10 provenance and safe workflow.
- **ADR:** `docs/adr/ADR-013-native-operation-revision-and-atomicity.md`.
- **Authoritative mutation owner:** `engine/core/assets/project_asset_attachment_service.*`.
- **Editor entry owners:** `editor/assets/asset_library_model.*` and `editor/assets/asset_library_panel.*`.
- **Persistence owner:** the attachment service writes the project-local attachment manifest and operation receipt; the existing project dirty/save path remains authoritative for document state.
- **Runtime/package consumer:** `content/assets/imported/*` and `content/assets/manifests/*` remain the only attachment payload/manifest paths.

## Bounded behavior

1. The service exposes an inspect/plan result with a stable source revision derived from the reviewed promoted payload and destination attachment state.
2. Apply receives an operation ID and expected source revision. It rejects a stale source before copying or replacing any project payload.
3. A completed operation ID with the same immutable request returns its recorded result; reusing it for a different request fails closed.
4. Payload and project manifest publication stay within the existing governed attachment paths. Both artifacts are staged before either is published; replacement retains backups until both same-directory renames succeed, then restores the prior pair on an in-process publish failure. A project-local transaction journal records state transitions; the next attachment operation validates the journal's project-relative paths, conservatively restores an interrupted prior pair, and clears recovery artifacts. The new operation receipt is project-local metadata and never package eligible.
5. Existing direct callers retain a compatibility overload during this service-foundation packet. The native Assets panel now presents a review step with destination paths, source revision, and operation ID before confirmation; a stale confirmation remains visible and can be refreshed without mutation.

## Safety contract

- **Stable IDs:** `assetId` identifies the attachment; `operationId` identifies one durable apply attempt.
- **Source revision:** SHA-256 over canonical reviewed source identity plus the current project attachment state for the target asset.
- **Preview lifetime:** this packet exposes no durable preview token. A plan is invalid immediately after its source revision changes.
- **Atomicity:** publish failures report a stable diagnostic and preserve the prior manifest/payload whenever in-process replacement cannot complete. Staging and backup files are cleaned after success or rollback. Receipt writes occur only after a successful attachment.
- **Inverse/recovery:** explicit attachment removal/undo remains excluded. Interrupted attachment publication is conservatively restored on the next attachment operation; this packet does not provide a standalone background recovery service or promise completion of an interrupted operation.
- **External edits:** changed/missing attachment manifests or payloads change the source revision and require a fresh plan.

## Acceptance and verification

- Existing attachment, conflict-policy, and path-containment behavior remains green.
- Focused tests cover successful checked apply, stale revision rejection without mutation, same-operation idempotent replay, operation-ID payload mismatch rejection, and external destination edit conflict.
- Run:

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[assets][asset_library][asset_attachment]" --reporter compact
python -m unittest tools.assets.tests.test_global_asset_import -v
```

## Manual evidence

In the native Assets workspace, attach a reviewed promoted image to a new project, change the promoted payload before confirmation, verify the stale-source diagnostic and unchanged attachment, then refresh and attach successfully. Record build, input path class, asset ID, operation ID, and the displayed diagnostic.

## Risks and rollback

The main risk is breaking existing asset-library callers while adding checked requests. Keep the compatibility overload until every native caller supplies a plan revision. Reverting this packet removes the checked-request/receipt code without touching promoted payloads or project attachment manifests created by existing workflows.

## Expected artifacts

- Native checked attachment request/plan/result types and focused Catch2 coverage.
- Project-local ignored operation receipts under `.urpg/`.
- No Node/browser storage, raw external durable path, or `plugins.js` mutation.

## Implementation evidence

2026-07-15: `ProjectAssetAttachmentService` now emits a plan-derived revision, accepts checked requests with safe operation IDs, rejects stale source revisions, and stores project-local receipts for idempotent replay. `AssetLibraryModel`, `AssetLibraryPanel`, and the native Assets workspace route attachment through a creator-visible review/confirm step and bind confirmation to that exact revision and operation ID. Attachment stages payload and manifest, publishes with same-directory rename, and rolls back retained backups if in-process publication fails. A project-local journal now conservatively restores the prior attachment pair after a process interruption, rejects absolute/traversal journal paths, and clears successful recovery artifacts on the next attachment operation. The focused native lane passed **169 assertions in 9 test cases**. This is not a cross-file crash-atomicity or background-recovery claim.
