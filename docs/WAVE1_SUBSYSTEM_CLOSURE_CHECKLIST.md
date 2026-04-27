# Wave 1 Subsystem Closure Checklist (Canonical)

Date: 2026-04-20
Status: canonical source for Wave 1 subsystem closure gates

This document is the single source of truth for the closure criteria that apply to all Wave 1 subsystem specs:

- `docs/UI_MENU_CORE_NATIVE_SPEC.md`
- `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
- `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
- `docs/BATTLE_CORE_NATIVE_SPEC.md`

The in-spec checklist sections are generated/synchronized from this canonical checklist through:

- `tools/docs/sync-wave1-spec-checklist.ps1`

## Universal closure gates

Each Wave 1 subsystem is considered closure-ready only when all gates below are complete:

1. Runtime ownership
   - Native runtime owner is authoritative.
   - Compat path for that domain is reduced to import/verification bridge behavior.
2. Editor productization
   - Authoring surface exists (inspect/edit/preview/validate).
   - Diagnostics surfaced in editor workflows.
3. Schema and migration
   - Data contracts are explicit and versioned.
   - Import/upgrader mapping is documented and tested.
4. Deterministic validation
   - Unit + integration anchors exist for authoritative behavior.
   - Snapshot assertions exist where layout/presentation determinism matters.
5. Diagnostics and safety
   - Failure paths are structured and inspectable.
   - Safe-mode or bounded fallback behavior is explicit.
6. Release evidence
   - Completion status docs updated.
   - Gate evidence is recorded for the subsystem closure milestone.

## Cross-doc sync policy

To prevent drift and avoid future manual cleanup:

1. Keep this file as the canonical closure criteria.
2. Run `tools/docs/sync-wave1-spec-checklist.ps1` after any Wave 1 checklist changes.
3. Run `tools/docs/sync-wave1-spec-checklist.ps1 -Check` in validation lanes.
4. Treat mismatches as documentation debt and block closure until synchronized.

## Closure sign-off evidence bundle

Before marking a subsystem as closure-complete:

- Runtime implementation references (headers + source files)
- Editor implementation references
- Schema files and migration pipeline references
- Deterministic tests (unit/integration/snapshot) and latest pass output
- Diagnostics and safe-mode behavior references
- Updated status docs:
  - `README.md`
  - `docs/PROGRAM_COMPLETION_STATUS.md`
  - `docs/archive/blueprints/URPG_Blueprint_v3_1_Integrated.md`
