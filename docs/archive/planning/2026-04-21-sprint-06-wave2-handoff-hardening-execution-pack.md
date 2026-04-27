# S06 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Stand up a reusable active-sprint control surface for multi-session LLM execution while landing the first bounded Wave 2 continuation slice already in progress on this branch.

**Sprint theme:** active sprint ownership plus Wave 2 ability hardening

**Primary outcomes for this sprint:**
- activate a reusable sprint control surface for future LLM sessions
- land the current Wave 2 gameplay ability runtime-hardening slice with focused verification
- leave canonical sprint artifacts truthful and resumable for the next session

**Non-goals for this sprint:**
- no broad roadmap rewrite
- no reopening closed Wave 1 or compat sprints
- no READY/status promotion without evidence

---

## Canonical Sources

Read these before starting `S06-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Sprint-specific sources for this sprint:

- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `tools/workflow/README.md`
- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`
- `tools/workflow/SPRINT_EXECUTION_PACK_TEMPLATE.md`
- `tools/workflow/SPRINT_TASK_BOARD_TEMPLATE.md`
- `tools/workflow/new-sprint-pack.ps1`
- `tests/unit/test_wave3_gaf.cpp`
- `engine/core/ability/ability_system_component.h`
- `WORKLOG.md`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation.
- Unsupported source behavior must become structured diagnostics or explicit waivers, never silent loss.
- Do not widen sprint scope by “cleaning up nearby code” unless the cleanup is necessary for the current ticket to compile or test.

---

## Session Start Checklist

- [x] Read the canonical sources above.
- [x] Open the paired sprint task board.
- [x] Mark exactly one ticket as `IN PROGRESS`.
- [x] Reconfirm the active local build profile.
- [x] Run the sprint preflight command block.

Suggested preflight:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### S06-T01: Sprint Workflow Activation And Handoff Pack Hardening

**Goal:** land a stable active-sprint entrypoint plus reusable sprint-pack scaffolding so any LLM session can resume bounded sprint work from one canonical pointer file.

**Primary files:**
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `tools/workflow/README.md`
- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`
- `tools/workflow/SPRINT_EXECUTION_PACK_TEMPLATE.md`
- `tools/workflow/SPRINT_TASK_BOARD_TEMPLATE.md`
- `tools/workflow/new-sprint-pack.ps1`
- `WORKLOG.md`

**Implementation targets:**
- add reusable execution-pack and task-board templates instead of copying an old sprint by hand
- provide a scriptable sprint-pack generator that can also update the active sprint pointer
- make `ACTIVE_SPRINT.md` the stable first file for any new LLM session
- verify the generator with a real temp-directory smoke test so handoff is operational, not just documented

**Verification:**

```powershell
& '.\tools\workflow\new-sprint-pack.ps1' -SprintId S99 -Slug 'sprint-99-smoke' -Goal 'Smoke test the sprint scaffolding flow.' -Theme 'generator verification' -PrimaryOutcomes @('create pack files','write active pointer') -NonGoals @('no repo doc edits from smoke test') -PlansRoot (Join-Path $env:TEMP 'urpg-sprint-pack-smoke3\plans') -ActiveSprintFile (Join-Path $env:TEMP 'urpg-sprint-pack-smoke3\ACTIVE_SPRINT.md') -Activate
```

**Done when:**
- a new sprint pack can be generated without editing old sprint files manually
- `ACTIVE_SPRINT.md` points to the active pack and explains the start order
- the smoke test proves the generated active pointer, execution pack, and task board all render correctly

---

### S06-T02: Wave 2 Ability Runtime Ownership Hardening

**Goal:** replace the remaining stub-style Wave 2 gameplay-ability assumptions with explicit runtime assertions so the sprint opens on concrete ownership behavior, not placeholder commentary.

**Primary files:**
- `engine/core/ability/ability_system_component.h`
- `tests/unit/test_wave3_gaf.cpp`
- `WORKLOG.md`

**Implementation targets:**
- expose only the minimal runtime state needed for focused effect-refresh verification
- replace “stub” comments in the Wave 3 gameplay-ability tests with real MP-cost, cooldown, and effect-refresh assertions
- keep the change bounded to focused ability runtime behavior and sprint truthfulness

**Verification:**

```powershell
& '.\build\dev-ninja-debug\urpg_tests.exe' "[ability][asc]"
& '.\build\dev-ninja-debug\urpg_tests.exe' "Wave 3: Ability Cooldown and Cost"
& '.\build\dev-ninja-debug\urpg_tests.exe' "Wave 3: Gameplay Effect Stacking - Refresh and Stack"
```

**Done when:**
- the Wave 3 gameplay-ability tests no longer describe current runtime behavior as stubbed
- focused ability/ASC verification passes locally
- `WORKLOG.md` records the slice as a Wave 2 continuation item

---

### S06-T03: Sprint Truth Maintenance And Closeout

**Goal:** leave the sprint artifacts, worklog, and active pointer aligned so a later LLM session can continue or close the sprint without rediscovering the branch state.

**Primary files:**
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s06-task-board.md`
- `docs/superpowers/plans/2026-04-21-sprint-06-wave2-handoff-hardening-execution-pack.md`
- `WORKLOG.md`

**Implementation targets:**
- keep exactly one active ticket in progress at a time
- record the last green command, first failing command, and touched files truthfully
- close or hand off the sprint without leaving placeholder text in the control files

**Verification:**

```powershell
Get-Content docs/superpowers/plans/ACTIVE_SPRINT.md
Get-Content docs/superpowers/plans/2026-04-21-s06-task-board.md
```

**Done when:**
- the task board resume note is current
- the execution pack matches the actual sprint scope
- the active sprint pointer remains the correct first stop for the next session

---

## Sprint-End Validation

Run this before declaring the sprint complete:

```powershell
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
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
4. Leave blockers explicit, even if the blocker is “full suite too expensive right now”.

Use the handoff template in:

- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

---

## Acceptance Summary

This sprint is complete only when all of the following are true:

- every ticket is `DONE` or explicitly moved out of sprint with rationale
- targeted verification passed or is explicitly waived with reason
- canonical docs and readiness records are aligned with the landed changes
- `WORKLOG.md` records the sprint slices

## Status

Sprint 06 is closed on the current tree. Reusable sprint-pack scaffolding and the active-sprint pointer are landed, the bounded Wave 2 ability hardening slice is verified, and PR/governance validation completed with green local results.
