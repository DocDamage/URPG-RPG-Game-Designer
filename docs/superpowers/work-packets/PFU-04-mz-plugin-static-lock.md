# PFU-04 Work Packet: project-owned MZ static plugin lock

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Persist a reproducible, non-executing compatibility record for the MZ plugin
source already inspected by the native Mod workspace.

## Contract

1. The lock is native project data at `content/compat/mz_plugin_lock.json`,
   with schema `urpg.mz_plugin_static_lock.v1`. It contains only plugin IDs,
   versions, enabled flags, project-relative source paths, SHA-256 source
   hashes, declared static dependencies, and the computed compatibility load
   order.
2. Creating or refreshing a lock uses the existing static MZ scanner and
   compatibility analysis. It does not load, evaluate, activate, import, or
   trust JavaScript. The UI labels the lock as compatibility metadata rather
   than a permission or execution decision.
3. Lock loading rejects malformed shapes, absolute or parent-traversing source
   paths, duplicate IDs, empty dependency IDs, and malformed load-order fields
   before it replaces the in-memory lock. The load order must list every
   inspected plugin exactly once and cannot name unknown plugins. Dependency
   cycles remain static inspection diagnostics, not an executable ordering
   claim. The source scanner never becomes a project owner.
4. The Mod workspace compares the saved lock on project bind and through an
   explicit Check action; it never rehashes plugin sources during every render
   frame. Refresh is an explicit dirty action; save is same-directory atomic,
   and the existing dirty-close and private recovery paths own the document.

## Acceptance and verification

- A creator can inspect MZ plugin source statically, refresh a project lock,
  see its dependency/order and source-hash record, save it, reopen it, and be
  told when static source has changed.
- The lock does not add an execution, migration, enablement, trust, or native
  feature-support claim.

Packet-local verification command (current pass not implied):

`./build/dev-ninja-debug/urpg_tests.exe "[plugin][compat][editor]" --reporter compact`

## Rollback and limits

Removing this owner leaves the read-only MZ source inspector intact. Typed MZ
configuration, permissions/trust approval, support-file capture, migration,
execution, save impact analysis, packaging effects, and rollback remain
separate compatibility work.
