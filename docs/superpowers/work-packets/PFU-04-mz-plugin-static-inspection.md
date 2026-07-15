# PFU-04 Work Packet: non-executing MZ plugin inspection

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Inspect RPG Maker MZ plugin source files as text only. The scanner reads
header tags and a bounded list of static API tokens from `js/plugins/*.js`,
then feeds the existing compatibility analyzer. It is presented alongside, but
separate from, native URPG mods in the existing Mod workspace.

## Traceability and owners

- **Source outcomes:** F04 Dependency-aware Plugin Manager; F06 boundary
  separation; I03-I08 compatibility diagnostics.
- **Analysis owner:** `engine/core/plugin/plugin_compatibility_score.*`.
- **Editor projection:** `editor/plugin/plugin_inspector_*` and the existing
  Mod workspace in `apps/editor/main.cpp`.
- **Schema impact:** none. Inspection data is ephemeral and read-only.

## Contract

1. Discovery enumerates only regular `.js` files directly inside the supplied
   MZ plugin directory. It never executes source, creates a QuickJS context,
   parses/evaluates `plugins.js`, or changes plugin/mod load state.
2. The scanner extracts only documented header tags (`@plugindesc`, `@author`,
   `@base`) and exact static API tokens. File stem remains the stable plugin
   ID; metadata is diagnostic evidence, not trusted native configuration.
3. Native mods and MZ plugins remain visibly separate. The MZ section is
   inspection-only and never exposes activate/reload/install actions.
4. Missing/unreadable/oversized plugin source produces diagnostic manifests;
   it does not silently become native support.

## Acceptance and verification

- A fixture MZ header is discovered without executing JavaScript.
- `@base` produces a declared compatibility dependency.
- Static known API tokens flow into the existing unsupported/shim analysis.
- The Mod workspace labels MZ data as read-only compatibility inspection.

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[plugin][compatibility]" --reporter compact
git diff --check
```

## Rollback and limits

Removing the scanner and read-only projection restores the previous native-mod
surface without changing any project files. Plugin locks, `plugins.js` load
order/status parsing, trust approvals, support-file impact, and optional
QuickJS execution remain separate work.

## Implementation evidence

2026-07-15: `InspectMzPluginScriptsFromDirectory()` enumerates only direct
regular `.js` files, enforces a 2 MiB source limit, parses documented comment
header tags for display name and `@base` dependencies, and scans five exact
known API tokens without evaluating source. It yields diagnostic manifests for
missing/unreadable/oversized scripts. `PluginInspectorModel` marks this lane as
inspection-only, and the existing Mod workspace renders it below native mods
with no activate/reload/install controls. No build, test, or formatting command
was run after this increment because the user explicitly directed
implementation to continue without further test activity.
