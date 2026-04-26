# Export Runtime Signature Enforcement Design

Status: design note for TD-AUD-03
Date: 2026-04-24
Scope: define the missing runtime/load-time security contract for `data.pck` before any export lane can be promoted to `READY`.

## Current Landed Boundary

The current export pipeline is stronger than the old placeholder bundle lane:

- `ExportPackager` emits a deterministic bounded `data.pck` container with an embedded manifest.
- Bundle entries carry keyed integrity metadata.
- The bundle manifest carries a keyed SHA-256 bundle signature.
- `ExportValidator` verifies the keyed SHA-256 bundle signature during post-export validation.
- Tests cover malformed manifests, missing signature metadata, corrupt payload bytes, and manifest tampering through the validator path.

This is validator-time protection only. It proves emitted artifacts can be checked after export; it does not prove an exported runtime refuses to load a tampered bundle.

## Required Runtime Contract Before `READY`

The export lane must remain `PARTIAL` until the runtime loader owns the same rejection behavior that `ExportValidator` currently proves offline.

A release-grade runtime bundle loader must:

1. Open `data.pck` through a runtime-owned loader API, not through ad hoc file reads.
2. Parse the bundle header and manifest with strict size bounds before reading payload data.
3. Reject wrong magic bytes, truncated headers, malformed manifest JSON, missing `integrityMode`, missing `signatureMode`, and missing `bundleSignature`.
4. Recompute the keyed SHA-256 bundle signature over the exact bytes and manifest fields covered by the packager/validator contract.
5. Reject any bundle whose signature does not match before exposing payload entries to game systems.
6. Recompute per-entry keyed integrity tags before decompressing or deobfuscating payloads.
7. Return structured diagnostics that distinguish missing bundle, malformed bundle, signature mismatch, entry integrity mismatch, unsupported signature mode, and unsupported integrity mode.
8. Share one canonical verification helper or byte-contract fixture with `ExportValidator` so runtime and post-export validation cannot drift.

## Test Requirements

Runtime-side implementation is not considered landed until tests cover:

- a valid packaged `data.pck` loading through the runtime loader;
- rejection of a tampered manifest;
- rejection of a tampered payload byte;
- rejection of a missing bundle signature;
- rejection of an unsupported signature mode;
- diagnostics surfaced to the export/runtime report path.

These tests must exercise the runtime/load-time path, not only `ExportValidator`.

## Atomic Bundle Write Decision

`writeBundleFile()` already fails closed on open, size-bound, stream, and close failures. Stronger partial-write protection should move to temp-file-plus-atomic-rename:

- write `data.pck.tmp` in the target export directory;
- flush and close the temp file;
- validate the temp file's manifest/signature before publishing;
- atomically replace `data.pck` only after validation succeeds;
- remove the temp file on failure and report a structured export error.

This is backlog hardening rather than the old bug where failed writes could still be reported as successful, but it should be completed before release-grade packaging claims.

## Out Of Scope Until Implemented

The current tree must not claim:

- OS/vendor code signing;
- notarization;
- platform-store packaging;
- anti-tamper or DRM-grade asset protection;
- runtime-side signature enforcement.

Those are valid future features, but they are not landed by the current validator-time SHA-256 bundle-signature lane.
