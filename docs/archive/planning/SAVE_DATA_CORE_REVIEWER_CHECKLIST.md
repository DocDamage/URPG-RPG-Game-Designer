# Save/Data Core — Reviewer Checklist

> **Status:** `PARTIAL`
> **Purpose:** Human-reviewer checklist for Wave 1 `save_data_core` promotion consideration.
> **Date:** 2026-04-23
> **Rule:** This document does **not** grant or imply release approval. A designated human reviewer must complete every item below and record findings before any promotion to `READY` may occur.

---

## How to Use This Checklist

1. Read the evidence packet in [`docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md`](SAVE_DATA_CORE_CLOSURE_SIGNOFF.md) first.
2. Build and run the verification commands listed under each section.
3. For each checkbox, either mark **VERIFIED** (evidence matches expectation) or **DEFERRED** (accepted scope limit) with a note.
4. Record any new gaps found during review in the *Reviewer Notes* section at the bottom.
5. Sign at the bottom only when all items are resolved or explicitly deferred.

---

## Section 1 — Runtime Correctness

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 1.1 | `SaveJournal::WriteAtomically` does not leave a partial write on simulated failure | `engine/core/save/save_journal.h`, `tests/unit/test_save_journal.cpp` | ☐ |
| 1.2 | `SaveCatalog` sequence numbers are stable across separate load/save cycles | `engine/core/save/save_catalog.h`, `tests/unit/test_save_catalog.cpp` | ☐ |
| 1.3 | `SaveRecoveryManager` falls through all three tiers in the correct priority order | `engine/core/save/save_recovery.h`, `tests/unit/test_save_recovery.cpp` | ☐ |
| 1.4 | `RuntimeSaveLoader` exposes synchronous API and raises on schema violation | `engine/core/save/save_runtime.h`, `tests/unit/test_save_runtime.cpp` | ☐ |

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[save]" --reporter compact
```

---

## Section 2 — Migration and Compat Coverage

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 2.1 | `UpgradeCompatSaveMetadataDocument()` produces native-format output for a known legacy fixture | `engine/core/save/save_migration.h`, `tests/unit/test_save_migration.cpp` | ☐ |
| 2.2 | `ImportCompatSaveDocument()` emits severity-graded diagnostics when fields are missing | `engine/core/save/save_migration.h`, `tests/unit/test_save_migration.cpp` | ☐ |
| 2.3 | `MigrationRunner` `rename`/`move` operations preserve unrelated blobs (including `_battle_state`) | `engine/core/migrate/migration_runner.h`, `tests/integration/test_wave1_closure_integration.cpp` | ☐ |
| 2.4 | Canonical save policy fixture validates against `save_policies.schema.json` | `content/fixtures/save_policies.json`, `content/schemas/save_policies.schema.json` | ☐ |

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[save][migration]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_save_policy_governance.ps1
```

---

## Section 3 — Schema and Governance

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 3.1 | All four save schemas (`save_metadata`, `save_migrations`, `save_policies`, `save_slots`) are present and registered in `SCHEMA_CHANGELOG.md` | `content/schemas/`, `docs/SCHEMA_CHANGELOG.md` | ☐ |
| 3.2 | `test_save_schema_contracts.cpp` covers all four schemas against canonical fixtures | `tests/unit/test_save_schema_contracts.cpp` | ☐ |
| 3.3 | `check_release_readiness.ps1` exits 0 (excluding build dependency) | `tools/ci/check_release_readiness.ps1` | ☐ |
| 3.4 | `truth_reconciler.ps1` exits 0 | `tools/ci/truth_reconciler.ps1` | ☐ |

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[save][schema]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

## Section 4 — Integration and Cross-Subsystem

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 4.1 | Battle metadata survives `RuntimeSaveLoader` round-trip | `tests/integration/test_battle_save_integration.cpp` | ☐ |
| 4.2 | Menu and message runtime states survive save/load boundaries | `tests/integration/test_wave1_closure_integration.cpp` | ☐ |
| 4.3 | `SaveInspectorPanel` render snapshot is deterministic for a known catalog state | `tests/unit/test_save_inspector_panel.cpp` | ☐ |

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_integration_tests.exe "[battle][save]" --reporter compact
.\build\dev-ninja-debug\urpg_integration_tests.exe "[integration][wave1][closure]" --reporter compact
```

---

## Section 5 — Unresolved Gap Log

The following gaps are **open** as of this review preparation date. The reviewer must determine whether each is acceptable for Wave 1 production use or whether it constitutes a promotion blocker.

| Gap | Status at Prep Date | Reviewer Determination |
|-----|---------------------|----------------------|
| Transactional multi-slot save operations (atomically commit >1 slot at once) are not supported | Open — bounded scope | ☐ Accept  ☐ Block |
| Platform cloud-save integration is not in-tree | Open — out of tree by design | ☐ Accept  ☐ Block |
| File-system test coverage on Windows-specific path behavior is partial | Open — CI covers Linux path | ☐ Accept  ☐ Block |
| Save slot count scalability above ~50 slots is untested | Open — bounded scope | ☐ Accept  ☐ Block |

---

## Reviewer Sign-off

> **Completing this section constitutes a human determination that the evidence above is sufficient for promotion consideration. It does not automatically update `readiness_status.json` status. A deliberate edit to the readiness record is required.**

| Field | Value |
|-------|-------|
| Reviewer name | ___________ |
| Review date | ___________ |
| Promotion recommendation | ☐ READY  ☐ Requires further work |
| Reviewer notes | |

---

## Reviewer Notes

_(Record any new gaps, concerns, or conditions found during review here.)_

### Non-Human Review Preparation - 2026-04-24

Automation has completed all evidence-refresh work it can truthfully perform for `save_data_core`. The reviewer still must make the accept/reject decision, but no unchecked machine-validation command remains in this checklist.

| Checklist Area | Command | Automated Result |
|---|---|---|
| Runtime correctness | `.\build\dev-ninja-debug\urpg_tests.exe "[save]" --reporter compact` | Passed: 509 assertions / 62 test cases |
| Migration and compat coverage | `.\build\dev-ninja-debug\urpg_tests.exe "[save][migration]" --reporter compact` | Passed: 49 assertions / 5 test cases |
| Migration and compat coverage | `powershell -ExecutionPolicy Bypass -File tools\ci\check_save_policy_governance.ps1` | Passed |
| Schema and governance | `.\build\dev-ninja-debug\urpg_tests.exe "[save][schema]" --reporter compact` | Passed: 87 assertions / 6 test cases |
| Schema and governance | `powershell -ExecutionPolicy Bypass -File tools\ci\check_release_readiness.ps1` | Passed |
| Schema and governance | `powershell -ExecutionPolicy Bypass -File tools\ci\truth_reconciler.ps1` | Passed |
| Integration and cross-subsystem | `.\build\dev-ninja-debug\urpg_integration_tests.exe "[battle][save]" --reporter compact` | Passed: 44 assertions / 4 test cases |
| Integration and cross-subsystem | `.\build\dev-ninja-debug\urpg_integration_tests.exe "[integration][wave1][closure]" --reporter compact` | Passed: 39 assertions / 4 test cases |
| Release gate shape | `.\build\dev-ninja-debug\urpg_project_audit.exe --json` | Passed command execution; still reports `releaseBlockerCount: 2` because `battle_core` and `save_data_core` are intentionally blocked on explicit human review |

Machine conclusion: `save_data_core` has current machine evidence prepared for human review. Automation cannot mark unresolved gaps accepted, cannot select `READY`, and cannot update `readiness_status.json` without a human reviewer decision.

### Automated Evidence Refresh - 2026-04-24

Automation refreshed the save/data evidence without granting human approval:

- `.\build\dev-ninja-debug\urpg_tests.exe "[save]" --reporter compact` passed: 509 assertions / 62 test cases.
- `.\build\dev-ninja-debug\urpg_integration_tests.exe "[battle][save]" --reporter compact` passed: 44 assertions / 4 test cases.
- `.\build\dev-ninja-debug\urpg_integration_tests.exe "[integration][wave1][closure]" --reporter compact` passed: 39 assertions / 4 test cases.
- `powershell -ExecutionPolicy Bypass -File tools\ci\check_save_policy_governance.ps1` passed.
- `powershell -ExecutionPolicy Bypass -File tools\ci\check_release_readiness.ps1` passed.
- `powershell -ExecutionPolicy Bypass -File tools\ci\truth_reconciler.ps1` passed.
- `.\build\dev-ninja-debug\urpg_project_audit.exe --json` still reports `releaseBlockerCount: 2`, with `save_data_core` intentionally blocked on explicit human review.

Human reviewer decision remains pending.
