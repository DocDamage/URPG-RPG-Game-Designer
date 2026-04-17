# Presentation Core - Release Checklist (RELEASE_CHECKLIST.md)

Derived from `urpg_first_class_presentation_architecture_plan_v2.md` Section 32.

Cross-cutting readiness waivers, truthfulness corrections, and intake-governance gates are tracked in `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

## Pre-Release Items

- [ ] Runtime path is stable and has been exercised in all scene families
- [ ] Focused presentation validation gate is green:
  - `ctest -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure`
  - or `pwsh -File .\tools\ci\run_presentation_gate.ps1`
- [ ] All scene families are contract-covered (per Section 10)
- [ ] Schema is versioned, validated, and documented
- [ ] Editor authoring exists for all required modules (per Section 13)
- [ ] Migration exists and is not fake-complete
- [ ] Downgrade path exists for all supported constructs
- [ ] Diagnostics exist for all defined diagnostic classes
- [ ] Unit, integration, and snapshot test coverage meets Phase 8 exit criteria
- [ ] Performance budgets are defined, measured, and met on all platform tiers
- [ ] CI enforces all required gates on all platform targets
- [ ] ADRs are finalized (not just opened)
- [ ] Schema changelog is complete
- [ ] Onboarding documentation is complete and reviewed by a new team member
- [ ] Unsupported cases are explicitly documented
- [ ] Reference/import corpora are clearly separated from shippable production assets
- [ ] License-cleared starter asset coverage exists for tiles, portraits, UI, VFX, and audio, or those gaps are explicitly waived
- [ ] AI tooling boundary policy is documented before editor-side AI assist features are treated as release-ready
- [ ] Presentation validation commands and ownership are documented in `docs/presentation/VALIDATION.md`
