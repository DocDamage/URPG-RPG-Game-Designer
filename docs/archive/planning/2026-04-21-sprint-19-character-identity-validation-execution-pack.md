# S19 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the character identity diagnostics gap with a shared validation runtime, a focused CI gate script, and reconciled governance.

**Sprint theme:** character identity validation pipeline

**Primary outcomes for this sprint:**
- A real `CharacterIdentityValidator` runtime checks character identity data against the bounded native catalog contract.
- A focused CI gate script validates the character identity artifact contract.
- ProjectAudit governance and readiness docs reflect the validation evidence.

**Non-goals for this sprint:**
- No full runtime Create-a-Character UI.
- No appearance-preview renderer pipeline.
- No unrelated subsystem work.

---

## Canonical Sources

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `engine/core/character/character_identity.h`
- `editor/character/character_creator_model.cpp`
- `editor/character/character_creator_panel.cpp`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_character_identity.cpp`

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open the paired sprint task board.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [ ] Run the sprint preflight command block.

Suggested preflight:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### S19-T01: Character Identity Validator Runtime + Shared Editor Use

- Build `CharacterIdentityValidator`.
- Route `CharacterCreatorModel` validation snapshots through the shared validator.
- Add focused validation tests for bounded catalog failures.

### S19-T02: CI Gate Script for Canonical Character Governance Fixture

- Add `tools/ci/check_character_governance.ps1`.
- Add `content/fixtures/character_identity_fixture.json`.
- Add a PowerShell-invocation test and wire the script into `tools/ci/run_local_gates.ps1`.

### S19-T03: Thread Evidence into ProjectAudit/Governance

- Add `characterArtifacts` + `characterArtifactIssueCount` to `urpg_project_audit`.
- Update ProjectAudit contract tests and panel parsing.
- Reconcile readiness/docs/worklog status.

---

## Sprint-End Validation

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[character]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_character_governance.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
```
