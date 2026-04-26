# S22 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the bounded input/controller governance gap around the already-landed `InputRemapStore` contract with a focused CI governance contract and truthful ProjectAudit/doc parity.

**Sprint theme:** input remap governance closure

**Primary outcomes for this sprint:**
- A focused CI gate script validates the canonical input-remap governance artifacts.
- A canonical input bindings fixture exists for the governance script and PowerShell invocation tests.
- ProjectAudit governance points at the real input-remap/schema/governance artifacts instead of historical controller-binding placeholders.
- Canonical status/docs reflect the bounded input governance evidence without overclaiming controller authoring UI.

**Non-goals for this sprint:**
- No gamepad/device polling backend.
- No platform-specific controller detection or hot-plug support.
- No new controller authoring panel or full project-schema/controller-binding migration beyond the bounded artifact contract.

---

## Canonical Sources

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `engine/core/input/input_remap_store.h`
- `engine/core/input/input_remap_store.cpp`
- `engine/core/input/input_core.h`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_input_remap_store.cpp`

---

## Ticket Order

### S22-T01: Canonical Input Artifact Contract Reconciliation

- Reconcile the Sprint 22 bounded contract around the existing `InputRemapStore` runtime and schema.
- Remove historical controller-binding artifact assumptions from ProjectAudit/docs where the active tree uses input-remap artifacts instead.

### S22-T02: CI Gate Script for Canonical Input Governance Fixture

- Add a focused input governance script.
- Add a canonical input bindings fixture.
- Add a PowerShell-invocation test and wire the script into `tools/ci/run_local_gates.ps1`.

### S22-T03: Thread Evidence into ProjectAudit/Governance

- Extend `inputArtifacts` governance expectations with the bounded Sprint 22 contract.
- Reconcile ProjectAudit tests/docs and canonical sprint/status artifacts.
- Close Sprint 22 with focused validation evidence.

---

## Sprint-End Validation

```powershell
cmake --build --preset dev-debug
.\build\dev-ninja-debug\urpg_tests.exe "[input]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_input_governance.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
