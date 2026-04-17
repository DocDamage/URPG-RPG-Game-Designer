## Summary

- What changed:
- Why it changed:

## Verification

- [ ] `.\tools\ci\run_local_gates.ps1`
- [ ] `pre-commit run --all-files`
- [ ] If this PR touches presentation runtime, spatial editor tooling, or presentation-facing rendering behavior:
  - [ ] `.\tools\ci\run_presentation_gate.ps1`
- [ ] If this PR touches RPG Maker MZ DLC content:
  - [ ] `.\tools\rpgmaker\validate-plugin-dropins.ps1`
- [ ] If this PR imports or reorganizes assets:
  - [ ] `python .\tools\assets\asset_hygiene.py --write-reports`

## Notes

- Risks or follow-ups:
- Docs updated:
