# Battle Core — Reviewer Checklist

> **Status:** `PARTIAL`  
> **Purpose:** Human-reviewer checklist for Wave 1 `battle_core` promotion consideration.  
> **Date:** 2026-04-23  
> **Rule:** This document does **not** grant or imply release approval. A designated human reviewer must complete every item below and record findings before any promotion to `READY` may occur.

---

## How to Use This Checklist

1. Read the evidence packet in [`docs/BATTLE_CORE_CLOSURE_SIGNOFF.md`](BATTLE_CORE_CLOSURE_SIGNOFF.md) first.
2. Build and run the verification commands listed under each section.
3. For each checkbox, either mark **VERIFIED** (evidence matches expectation) or **DEFERRED** (accepted scope limit) with a note.
4. Record any new gaps found during review in the *Reviewer Notes* section at the bottom.
5. Sign at the bottom only when all items are resolved or explicitly deferred.

---

## Section 1 — Runtime Correctness

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 1.1 | `BattleFlowController` transitions through all phases without locking | `engine/core/battle/battle_core.h`, `tests/unit/test_battle_core.cpp` | ☐ |
| 1.2 | `BattleActionQueue` produces stable ordering for equal speed/priority | `engine/core/battle/battle_core.h`, `tests/unit/test_battle_core.cpp` | ☐ |
| 1.3 | `BattleRuleResolver` damage outputs are deterministic for the same inputs | `engine/core/battle/battle_core.h`, `tests/unit/test_battle_core.cpp` | ☐ |
| 1.4 | Escape ratio calculation matches formula spec | `engine/core/battle/battle_core.h`, `tests/unit/test_battle_core.cpp` | ☐ |

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -R "test_battle_core" --output-on-failure
```

---

## Section 2 — Migration and Compat Coverage

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 2.1 | `BattleMigration::migrateTroop()` handles all documented troop event commands | `engine/core/battle/battle_migration.h`, `tests/unit/test_battle_migration.cpp` | ☐ |
| 2.2 | Unrecognised event commands emit explicit structured fallback warnings, not silent drops | `engine/core/battle/battle_migration.h`, `tests/unit/test_battle_migration.cpp` | ☐ |
| 2.3 | All 12 RPG Maker scope codes are covered in `migrateAction()` | `engine/core/battle/battle_migration.h`, `tests/unit/test_battle_migration.cpp` | ☐ |
| 2.4 | Grouped boolean `and`/`or` phase condition trees are preserved in `_compat_condition_fallbacks` | `engine/core/battle/battle_migration.h`, `tests/unit/test_battle_migration.cpp` | ☐ |

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -R "test_battle_migration" --output-on-failure
```

---

## Section 3 — Schema and Governance

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 3.1 | `battle_troops.schema.json` passes JSON Schema draft-07 validation on canonical fixture | `content/schemas/battle_troops.schema.json` | ☐ |
| 3.2 | `battle_actions.schema.json` passes validation on canonical fixture | `content/schemas/battle_actions.schema.json` | ☐ |
| 3.3 | `check_release_readiness.ps1` exits 0 (excluding build dependency) | `tools/ci/check_release_readiness.ps1` | ☐ |
| 3.4 | `truth_reconciler.ps1` exits 0 | `tools/ci/truth_reconciler.ps1` | ☐ |

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

---

## Section 4 — Integration and Cross-Subsystem

| # | Check | Expected Evidence File | Result |
|---|-------|----------------------|--------|
| 4.1 | Battle metadata survives `RuntimeSaveLoader` round-trip | `tests/integration/test_battle_save_integration.cpp` | ☐ |
| 4.2 | `MigrationRunner` does not drop `_battle_state` during unrelated field renames | `tests/integration/test_wave1_closure_integration.cpp` | ☐ |
| 4.3 | Inspector panel renders deterministic snapshot for a live flow controller | `tests/unit/test_battle_inspector_panel.cpp` | ☐ |

**Verification:**

```powershell
ctest --test-dir build/dev-ninja-debug -R "test_battle" --output-on-failure
ctest --test-dir build/dev-ninja-debug -R "test_wave1_closure" --output-on-failure
```

---

## Section 5 — Residual Gap Acceptance

The following residual gaps are **pre-accepted** for Wave 1. The reviewer must confirm each is understood and that no undiscovered dependencies have materialized since the signoff doc was written.

| Gap | Pre-Accepted? | Reviewer Confirmation |
|-----|--------------|----------------------|
| Compat import/migration completion: niche event commands and control-flow constructs still require fallback handling | Yes — bounded in-tree surface | ☐ |
| Plugin authority validation: deep behavioral sandbox tests for every battle plugin variant are not at 100% | Yes — manifest-level validation is sufficient for Wave 1 | ☐ |
| Networked / async battle: not in Wave 1 scope | Yes — out of tree | ☐ |
| Bounded in-tree compat surface: broader formula semantics, transition-type routing, deeper troop-page scripting | Yes — documented scope limit | ☐ |

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
