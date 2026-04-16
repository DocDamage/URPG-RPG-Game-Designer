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
