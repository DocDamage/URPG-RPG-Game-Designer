# Docs Automation Tools

## Wave 1 subsystem checklist sync

Canonical source:

- `docs/WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md`

Managed targets:

- `docs/UI_MENU_CORE_NATIVE_SPEC.md`
- `docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md`
- `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
- `docs/BATTLE_CORE_NATIVE_SPEC.md`

Sync command:

```powershell
.\tools\docs\sync-wave1-spec-checklist.ps1
```

Drift check (non-zero on mismatch):

```powershell
.\tools\docs\sync-wave1-spec-checklist.ps1 -Check
```

CI/local gate wrapper:

```powershell
.\tools\ci\check_wave1_spec_checklists.ps1
```

Policy:

1. Edit the canonical checklist file first.
2. Run sync script.
3. Commit canonical + generated spec updates together.
4. Treat `-Check` failures as required fixes before merge.

## Presentation docs link validation

Scope:

- `docs/presentation/README.md`
- `docs/presentation/VALIDATION.md`
- `docs/presentation/test_matrix/README.md`

Validation command:

```powershell
.\tools\docs\check-presentation-doc-links.ps1
```

Where it runs:

- directly from the focused presentation gate: `.\tools\ci\run_presentation_gate.ps1`
- from the broader local gates: `.\tools\ci\run_local_gates.ps1`
- in CI inside `.github/workflows/ci-gates.yml`

Policy:

1. When adding or renaming presentation docs, update the presentation hubs first.
2. Run the presentation docs link checker before merge.
3. Treat missing-link failures as documentation drift that must be fixed with the same change.
