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
ctest --test-dir build/dev-ninja-debug -R "test_save" --output-on-failure
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
ctest --test-dir build/dev-ninja-debug -R "test_save_migration" --output-on-failure
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
ctest --test-dir build/dev-ninja-debug -R "test_save_schema" --output-on-failure
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
ctest --test-dir build/dev-ninja-debug -R "test_battle_save|test_wave1_closure" --output-on-failure
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
