# Export Runtime Signature Enforcement Design

Status: implemented runtime/load-time contract for TD-AUD-03
Date: 2026-04-27
Scope: record the runtime/load-time security contract for `data.pck` and the remaining non-runtime export hardening boundaries.

## Current Landed Boundary

The current export pipeline and runtime startup path are stronger than the old placeholder bundle lane:

- `ExportPackager` emits a deterministic bounded `data.pck` container with an embedded manifest.
- Bundle entries carry keyed integrity metadata.
- The bundle manifest carries a keyed SHA-256 bundle signature.
- `ExportValidator` verifies the keyed SHA-256 bundle signature during post-export validation.
- `RuntimeBundleLoader` validates the same bundle contract at load time before exposing payloads.
- `RuntimeStartupServices` discovers exported `data.pck` candidates and emits a `RuntimeBundleLoader` startup error when validation fails.
- `ExportPackager` publishes bundles through a temp-file validation path and atomically replaces `data.pck` only after validation succeeds.
- Tests cover malformed manifests, missing signature metadata, corrupt payload bytes, manifest tampering, unsupported signature mode, failed atomic publish preservation, and runtime startup rejection.

This is bounded runtime/load-time protection. It is not OS/vendor code signing, notarization, platform-store packaging, anti-tamper, or DRM-grade asset protection.

## Runtime Contract

The runtime bundle loader:

1. Open `data.pck` through a runtime-owned loader API, not through ad hoc file reads.
2. Parse the bundle header and manifest with strict size bounds before reading payload data.
3. Reject wrong magic bytes, truncated headers, malformed manifest JSON, missing `integrityMode`, missing `signatureMode`, and missing `bundleSignature`.
4. Recompute the keyed SHA-256 bundle signature over the exact bytes and manifest fields covered by the packager/validator contract.
5. Reject any bundle whose signature does not match before exposing payload entries to game systems.
6. Recompute per-entry keyed integrity tags before decompressing or deobfuscating payloads.
7. Return structured diagnostics that distinguish missing bundle, malformed bundle, signature mismatch, entry integrity mismatch, unsupported signature mode, and unsupported integrity mode.
8. Share one canonical verification helper or byte-contract fixture with `ExportValidator` so runtime and post-export validation cannot drift.

## Test Coverage

Runtime-side implementation coverage includes:

- a valid packaged `data.pck` loading through the runtime loader;
- rejection of a tampered manifest;
- rejection of a tampered payload byte;
- rejection of a missing bundle signature;
- rejection of an unsupported signature mode;
- diagnostics surfaced through runtime startup;
- exported runtime smoke rejection of a tampered bundle.

These tests exercise the runtime/load-time path, not only `ExportValidator`.

## Atomic Bundle Write Decision

`writeBundleFile()` already fails closed on open, size-bound, stream, and close failures. Stronger partial-write protection should move to temp-file-plus-atomic-rename:

- write `data.pck.tmp` in the target export directory;
- flush and close the temp file;
- validate the temp file's manifest/signature before publishing;
- atomically replace `data.pck` only after validation succeeds;
- remove the temp file on failure and report a structured export error.

This is implemented as bounded local filesystem publication. Broader platform installer/updater atomicity remains separate release engineering work.

## Out Of Scope Until Implemented

The current tree must not claim:

- OS/vendor code signing;
- notarization;
- platform-store packaging;
- anti-tamper or DRM-grade asset protection.

Those are valid future features, but they are not landed by the current runtime bundle-signature lane.
