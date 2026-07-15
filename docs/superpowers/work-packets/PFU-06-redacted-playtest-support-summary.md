# PFU-06 Work Packet: redacted playtest support summary

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Provide a bounded support artifact for one disposable native playtest session.

## Contract

1. A completed, running, or returned session can write
   `redacted_support_summary.json` into its private disposable session
   directory through `PlaytestSessionController`.
2. The artifact contains only schema/redaction declarations, session state,
   exit code, elapsed duration, map/spawn identifiers, diagnostic count, and
   each diagnostic's severity, subsystem, code, and map ID.
3. The artifact explicitly omits project/session paths, process stdout/stderr,
   diagnostic messages, diagnostic source paths, and runtime object IDs. The
   editor does not upload, package, or otherwise transmit it.
4. Failure to write leaves the session and project data unchanged. The
   existing session overlay remains the disposable runtime input.

## Limits

This is not a complete support bundle, crash dump, screenshot/video capture,
save export, project diff, secret scanner, or external submission workflow.

Verification is deferred by user instruction.
