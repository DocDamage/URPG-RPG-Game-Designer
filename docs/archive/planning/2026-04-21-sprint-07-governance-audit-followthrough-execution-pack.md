# S07 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Deepen governance follow-through by expanding ProjectAudit beyond the current conservative readiness-derived scanner, tightening release-signoff enforcement, and keeping diagnostics/export parity truthful.

**Sprint theme:** project audit depth and release-signoff enforcement

**Primary outcomes for this sprint:**
- add richer canonical artifact checks to ProjectAudit
- improve template-specific audit execution for one or more roadmap templates
- strengthen governance/check parity without reopening broad feature scope

**Non-goals for this sprint:**
- no broad Wave 2 feature expansion
- no template marketing/status promotion without evidence
- no unrelated refactors outside audit/governance surfaces

---

## Canonical Sources

Read these before starting `S07-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`

Sprint-specific sources for this sprint:

- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/PROJECT_AUDIT.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `docs/RELEASE_SIGNOFF_WORKFLOW.md`
- `tools/audit/urpg_project_audit.cpp`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `README.md`
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
- [ ] Run the sprint preflight command block.

Suggested preflight:

```powershell
cmake --preset dev-ninja-debug
cmake --build --preset dev-debug
ctest -L pr --output-on-failure
```

---

## Ticket Order

### S07-T01: ProjectAudit Canonical Artifact Checks

**Goal:** deepen `urpg_project_audit` from a conservative readiness-derived scanner into a richer governance audit that checks explicit canonical artifact and project-schema conditions already defined by the roadmap.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tests/unit/test_project_audit_panel.cpp`
- `tests/unit/test_diagnostics_workspace.cpp`
- `docs/PROJECT_AUDIT.md`

**Implementation targets:**
- add one or more real canonical artifact-path or project-schema checks that go beyond merely echoing readiness records
- keep the audit CLI/report model, panel snapshot, and diagnostics export path in parity
- keep wording truthful: richer audit coverage is allowed, but it must not overclaim full project scanning

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[project_audit]" --reporter compact
```

**Done when:**
- the audit CLI catches at least one newly implemented canonical artifact/project-schema governance case under test
- panel and diagnostics export stay in parity with the richer audit contract
- `docs/PROJECT_AUDIT.md` describes the new scope truthfully

---

### S07-T02: Template-Specific Audit Execution And Governance Enforcement

**Goal:** make at least one roadmap template materially exercisable through the audit path while tightening readiness/signoff enforcement around the same governance surfaces.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_project_audit_cli.cpp`
- `tools/ci/check_release_readiness.ps1`
- `tools/ci/truth_reconciler.ps1`
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `docs/templates/monster_collector_rpg_spec.md`
- `docs/templates/2_5d_rpg_spec.md`
- `content/readiness/readiness_status.json`

**Implementation targets:**
- exercise one or two explicit template blocker paths through the audit command
- harden governance scripts where touched so they enforce the richer audit/signoff/readiness contract
- keep template wording evidence-gated and avoid accidental status inflation

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_project_audit.exe --json --template monster_collector_rpg
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```

**Done when:**
- at least one template-specific audit path is covered by tests or direct command verification
- governance scripts remain green with the richer contract in place
- touched readiness/template docs stay truthful and aligned

---

### S07-T03: Governance Truth Maintenance And Sprint Closeout

**Goal:** reconcile the canonical status stack, README, sprint artifacts, and worklog with the actual governance/audit depth landed in this sprint.

**Primary files:**
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `README.md`
- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `docs/superpowers/plans/2026-04-21-s07-task-board.md`
- `WORKLOG.md`

**Implementation targets:**
- update only the touched governance/audit/readiness language
- leave a clear resume note if the sprint is not fully closed in one session
- preserve the active-sprint contract for the next LLM session

**Verification:**

```powershell
ctest -L pr --output-on-failure
```

**Done when:**
- the canonical docs describe the landed governance depth accurately
- the task board resume note is current
- the sprint can be resumed or closed without placeholder text or hidden assumptions

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
