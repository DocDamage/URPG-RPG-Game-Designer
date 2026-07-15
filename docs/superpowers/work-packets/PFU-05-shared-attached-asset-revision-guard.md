# PFU-05 Work Packet: shared attached-asset revision guard

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Move the attached-asset manifest revision check from the Map-only drop path
into the shared editor asset-drop contract. Apply it before durable mutation by
the native Map, Character Creator, Dialogue Graph voice picker, and Audio Mix
owners.

## Contract

1. An attached payload used while a project is open must carry the source
   SHA-256 from its project attachment manifest.
2. The shared guard reloads that manifest and rejects a missing, malformed,
   mismatched-asset, empty-revision, or stale-revision payload before the
   receiving owner changes durable state.
3. Map tile/prop/event placement, Character Creator appearance assignment,
   Dialogue Graph voice selection, and Audio Mix encounter-preview assignment
   call the same guard after their existing type/provenance validation.
4. Programmatic callers without an open project context retain their existing
   legacy behavior. The guard does not create an attachment, change an asset,
   or grant a raw/promoted payload authority.

## Acceptance and verification

- A stale attached drag is rejected consistently by every covered owner without
  changing that owner's document or local history.
- A current attached drag remains subject to each owner's existing media-type,
  durable-project, dirty-state, and undo rules.

Verification is deferred by user instruction. The next focused coverage should
exercise current, missing, malformed, and stale manifest payloads for every
covered owner.

## Limits

This guard is not a cross-owner transaction or shared undo stack. Menu, quest,
database, extension, and future contextual owners must opt in only after their
authoritative mutation path supports the same durable safety contract.
