# Presentation Core - Release Checklist (RELEASE_CHECKLIST.md)

Derived from `docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md` Section 32.

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

## Phase 5 Hardening Closure Evidence

- [x] Focused plugin callback audit lane passes:
  - `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PluginManager: Command execution|MapScene:|SceneManager:"`
- [x] Focused presentation hardening gate passes:
  - `ctest --test-dir build/dev-mingw-debug -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure`
- [x] Phase 4 intake-governance validation passes with wrapper/facade and provenance checks:
  - `powershell -ExecutionPolicy Bypass -File tools/ci/check_phase4_intake_governance.ps1`
- [x] Repo-wide regression pass is green for the closure snapshot:
  - `ctest --preset dev-all --output-on-failure`

## Battle Core Focused Evidence

- [x] Native battle runtime/editor diagnostics parity passes:
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[battle][scene][diagnostics],[battle][editor][panel],[editor][diagnostics][integration][battle_preview]" --reporter compact`
- [x] Battle presentation bridge consumes active `BattleScene` state:
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[presentation][bridge],[presentation][runtime]" --reporter compact`
- [x] Battle migration warnings and wizard reporting stay green:
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[editor][diagnostics][wizard],[battle][migration]" --reporter compact`
- [x] Focused Battle Core CTest subset is green:
  - `ctest --test-dir C:\dev\urpg-battle-build --output-on-failure -R "PresentationBridge derives battle frame from active BattleScene|PresentationBridge builds frame for active scene using runtime|BattleScene builds diagnostics preview from the next ordered queued action|Battle inspector panel binds live scene diagnostics preview payload|DiagnosticsWorkspace - Battle tab exports live scene diagnostics preview payload|BattleMigration:|MigrationWizardModel: battle migration warnings propagate from unsupported troop phase/page data|MigrationWizardModel: Batch Orchestration"`

## Save/Data Focused Evidence

- [x] Save schema contract files and catalog/runtime unit lanes pass:
  - `C:\dev\urpg-battle-build\urpg_tests.exe "[save][schema],[save][catalog],[save][runtime],[save][editor],[save][panel][integration],[save][metadata],[editor][diagnostics][integration][save_actions]" --reporter compact`
  - includes runtime-owned `recovery_diagnostics` and `serialization_schema` export coverage plus live save-policy draft/validation/apply coverage through the save inspector and diagnostics workspace
- [x] Save integration recovery lane passes:
  - `C:\dev\urpg-battle-build\urpg_integration_tests.exe "[integration][save]" --reporter compact`
