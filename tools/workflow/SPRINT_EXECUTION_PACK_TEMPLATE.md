# {{SPRINT_ID}} Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** {{GOAL}}

**Sprint theme:** {{THEME}}

**Primary outcomes for this sprint:**
{{PRIMARY_OUTCOMES}}

**Non-goals for this sprint:**
{{NON_GOALS}}

---

## Canonical Sources

Read these before starting `{{SPRINT_ID}}-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Add sprint-specific sources below before starting implementation:

- `<fill me>`

---

## Operating Rules

- Work in listed order. Do not start a later ticket early unless the current ticket is blocked and the blocker is recorded in the task board.
- Use failing-test-first whenever practical.
- Update docs in the same change as implementation.
- Unsupported source behavior must become structured diagnostics or explicit waivers, never silent loss.
- Do not widen sprint scope by “cleaning up nearby code” unless the cleanup is necessary for the current ticket to compile or test.

---

## Session Start Checklist

- [ ] Read the canonical sources above.
- [ ] Open the paired sprint task board.
- [ ] Mark exactly one ticket as `IN PROGRESS`.
- [ ] Reconfirm the active local build profile.
- [ ] Run the sprint preflight command block.

Suggested preflight:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### {{SPRINT_ID}}-T01: <title>

**Goal:** <fill me>

**Primary files:**
- `<fill me>`

**Implementation targets:**
- `<fill me>`

**Verification:**

```powershell
<fill me>
```

**Done when:**
- `<fill me>`

---

### {{SPRINT_ID}}-T02: <title>

**Goal:** <fill me>

**Primary files:**
- `<fill me>`

**Implementation targets:**
- `<fill me>`

**Verification:**

```powershell
<fill me>
```

**Done when:**
- `<fill me>`

---

### {{SPRINT_ID}}-T03: <title>

**Goal:** <fill me>

**Primary files:**
- `<fill me>`

**Implementation targets:**
- `<fill me>`

**Verification:**

```powershell
<fill me>
```

**Done when:**
- `<fill me>`

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
