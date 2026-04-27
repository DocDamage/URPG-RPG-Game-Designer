# S18 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the achievement registry diagnostics gap with a live validation runtime, a focused CI gate script, and reconciled governance.

**Sprint theme:** achievement registry validation pipeline

**Primary outcomes for this sprint:**
- A real `AchievementValidator` runtime checks achievement definitions for duplicate IDs, empty fields, unparseable conditions, and impossible targets.
- A focused CI gate script validates the achievement artifact contract.
- ProjectAudit governance and readiness docs reflect the validation evidence.

**Non-goals for this sprint:**
- No new achievement trigger types or event bus features.
- No unrelated subsystem work.
- No claim that the validator covers all possible content policy edge cases (bounded rule set only).

---

## Canonical Sources

Read these before starting `S18-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`

Add sprint-specific sources below before starting implementation:

- `engine/core/achievement/achievement_registry.h`
- `engine/core/achievement/achievement_registry.cpp`
- `engine/core/achievement/achievement_trigger.h`
- `editor/achievement/achievement_panel.h`
- `content/schemas/achievements.schema.json`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_achievement_registry.cpp`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation.
- Unsupported source behavior must become structured diagnostics or explicit waivers, never silent loss.
- Do not widen sprint scope by "cleaning up nearby code" unless the cleanup is necessary for the current ticket to compile or test.

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

### S18-T01: Achievement Validator Runtime + Tests

**Goal:** Build a real validation runtime that audits `AchievementRegistry` definitions and produces structured issues.

**Primary files:**
- `engine/core/achievement/achievement_validator.h` (new)
- `engine/core/achievement/achievement_validator.cpp` (new)
- `tests/unit/test_achievement_registry.cpp`
- `CMakeLists.txt`

**Implementation targets:**
- Create `AchievementValidator` with `validate(const AchievementRegistry& registry)` returning `std::vector<AchievementIssue>`.
- Validation rules:
  - **EmptyId** → Error if any registered achievement has an empty `id`.
  - **EmptyTitle** → Error if any achievement has an empty `title`.
  - **UnparseableUnlockCondition** → Error if `AchievementTrigger::parse()` fails to produce a valid trigger (target == 0 and condition non-empty indicates parse failure).
  - **ZeroTarget** → Warning if parsed target is 0 (achievement is impossible to unlock).
  - **DuplicateId** → Error if two registered achievements share the same `id`.
  - **EmptyIconId** → Warning if `iconId` is empty.
- Add focused test in `test_achievement_registry.cpp` that:
  1. Constructs an `AchievementRegistry` with intentionally bad definitions.
  2. Runs `AchievementValidator::validate()`.
  3. Asserts the expected issues are found.
- Register new `.cpp` in `CMakeLists.txt` under `urpg_core`.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[achievement]" --reporter compact
```

**Done when:**
- The validator compiles and the new test passes.
- Existing achievement tests remain green.

---

### S18-T02: CI Gate Script for Canonical Achievement Governance Fixture

**Goal:** Add a focused executable gate for the achievement artifact contract.

**Primary files:**
- `tools/ci/check_achievement_governance.ps1` (new)
- `content/fixtures/achievement_registry_fixture.json` (new)
- `tests/unit/test_achievement_registry.cpp`
- `tools/ci/run_local_gates.ps1`

**Implementation targets:**
- Create `tools/ci/check_achievement_governance.ps1` that validates:
  - `content/schemas/achievements.schema.json` exists and is valid JSON.
  - `engine/core/achievement/achievement_registry.h` and `.cpp` exist.
  - `engine/core/achievement/achievement_validator.h` and `.cpp` exist.
  - `editor/achievement/achievement_panel.h` and `.cpp` exist.
  - `tools/ci/check_achievement_governance.ps1` exists.
  - A canonical achievement fixture exists at `content/fixtures/achievement_registry_fixture.json` (create a minimal valid fixture if missing).
- The script should emit a JSON result shape matching the validator pattern: `{"passed": true/false, "errors": [...]}`.
- Add a PowerShell-invocation test in `test_achievement_registry.cpp` that invokes the script and asserts it passes.
- Wire the script into `tools/ci/run_local_gates.ps1` after the build step.

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_achievement_governance.ps1
.\build\dev-ninja-debug\urpg_tests.exe "[achievement]" --reporter compact
```

**Done when:**
- The script passes locally.
- The script is referenced in the local gate runner.

---

### S18-T03: Thread Evidence into ProjectAudit/Governance

**Goal:** Reconcile ProjectAudit governance and readiness records with the validation evidence.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`

**Implementation targets:**
- Update `addAchievementArtifactGovernance` in `urpg_project_audit.cpp` to also check for:
  - `engine/core/achievement/achievement_validator.h`
  - `engine/core/achievement/achievement_validator.cpp`
  - `tools/ci/check_achievement_governance.ps1`
  - `content/fixtures/achievement_registry_fixture.json`
- Update `readiness_status.json` for `achievement_registry`:
  - Set `diagnostics` to `true`.
  - Update `mainGaps` to reflect the bounded validator scope and remaining backend integration.
- Update `RELEASE_READINESS_MATRIX.md`, `PROGRAM_COMPLETION_STATUS.md`, and `WORKLOG.md`.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit_cli]" --reporter compact
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- ProjectAudit CLI tests pass.
- Readiness gates pass.
- Canonical docs are truthful.

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest --test-dir build/dev-ninja-debug -L pr --output-on-failure
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

If a command is too expensive or blocked, record the reason in the task board and final sprint note.

---

## Commit Rhythm

Preferred commit shape:

1. tests that expose the missing behavior
2. implementation
3. docs/readiness/worklog alignment

Suggested commit subject style:

- `test: add <ticket> regression coverage`
- `feat: implement <ticket behavior>`
- `docs: reconcile sprint status and readiness evidence`

---

## Resume Protocol

When handing off to a new LLM session:

1. Update the task board `Resume From Here` section.
2. Record the current ticket status and exact next command.
3. Keep the touched-files list current.
4. Leave blockers explicit, even if the blocker is "full suite too expensive right now".

Use the handoff template in:

- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

---

## Acceptance Summary

This sprint is complete only when all of the following are true:

- every ticket is `DONE` or explicitly moved out of sprint with rationale
- targeted verification passed or is explicitly waived with reason
- canonical docs and readiness records are aligned with the landed changes
- `WORKLOG.md` records the sprint slices
