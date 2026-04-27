# S16 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the accessibility auditor to release-grade evidence with a live editor surface ingest adapter, a CI gate script, and reconciled governance.

**Sprint theme:** accessibility auditor live ingestion

**Primary outcomes for this sprint:**
- A real ingest adapter converts live `MenuInspectorModel` rows into `UiElementSnapshot` elements for the auditor.
- A focused CI gate script validates the accessibility artifact contract.
- ProjectAudit governance and readiness docs reflect the live-ingestion evidence.

**Non-goals for this sprint:**
- No new audit rules beyond the existing MissingLabel/FocusOrder/Contrast/Navigation set.
- No unrelated subsystem work.
- No claim that the adapter covers all editor surfaces (menu is the bounded first slice).

---

## Canonical Sources

Read these before starting `S16-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`

Add sprint-specific sources below before starting implementation:

- `engine/core/accessibility/accessibility_auditor.h`
- `engine/core/accessibility/accessibility_auditor.cpp`
- `editor/accessibility/accessibility_panel.h`
- `editor/accessibility/accessibility_panel.cpp`
- `editor/ui/menu_inspector_model.h`
- `editor/ui/menu_inspector_panel.h`
- `editor/diagnostics/diagnostics_workspace.cpp`
- `tests/unit/test_accessibility_auditor.cpp`
- `tests/unit/test_accessibility_panel.cpp`
- `tools/audit/urpg_project_audit.cpp`

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

### S16-T01: Live Editor Surface Ingest Adapter

**Goal:** Build a real ingest adapter from `MenuInspectorModel` into `AccessibilityAuditor`.

**Primary files:**
- `editor/accessibility/accessibility_menu_adapter.h` (new)
- `editor/accessibility/accessibility_menu_adapter.cpp` (new)
- `tests/unit/test_accessibility_panel.cpp`
- `CMakeLists.txt`

**Implementation targets:**
- Create `AccessibilityMenuAdapter` with a static method `ingest(const MenuInspectorModel& model)` returning `std::vector<UiElementSnapshot>`.
- Mapping rules:
  - `id` → `command_id`
  - `label` → `command_label`
  - `hasFocus` → `row_navigable && command_visible && command_enabled`
  - `focusOrder` → `priority` (or `command_index + 1` if priority is 0)
  - `contrastRatio` → `0.0f` (not available from menu model; auditor skips contrast when 0)
- Add focused test in `test_accessibility_panel.cpp` (or `test_accessibility_auditor.cpp`) that:
  1. Creates a `MenuSceneGraph` with panes and commands (some missing labels, some with duplicate priorities).
  2. Loads it into `MenuInspectorModel`.
  3. Uses the adapter to ingest into `AccessibilityAuditor`.
  4. Asserts the auditor finds `MissingLabel` and `FocusOrder` issues derived from the live menu state.
- Register the new `.cpp` in `CMakeLists.txt` under `urpg_core`.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[accessibility]" --reporter compact
```

**Done when:**
- The adapter compiles and the new test passes.
- Existing accessibility tests remain green.

---

### S16-T02: CI Gate Script for Canonical Accessibility Report Fixture

**Goal:** Add a focused executable gate for the accessibility artifact contract.

**Primary files:**
- `tools/ci/check_accessibility_governance.ps1` (new)
- `tests/unit/test_accessibility_auditor.cpp`

**Implementation targets:**
- Create `tools/ci/check_accessibility_governance.ps1` that validates:
  - `content/schemas/accessibility_report.schema.json` exists and is valid JSON.
  - `engine/core/accessibility/accessibility_auditor.h` and `.cpp` exist.
  - `editor/accessibility/accessibility_panel.h` and `.cpp` exist.
  - `editor/accessibility/accessibility_menu_adapter.h` and `.cpp` exist.
  - A canonical accessibility report fixture exists at `content/fixtures/accessibility_report_fixture.json` (create a minimal valid fixture if missing).
- The script should emit a JSON result shape matching the validator pattern: `{"passed": true/false, "errors": [...]}`.
- Add a PowerShell-invocation test in `test_accessibility_auditor.cpp` (or a new file) that invokes the script and asserts it passes.
- Wire the script into `tools/ci/run_local_gates.ps1` after the build step (add an invocation comment or actual call if the gate structure is obvious).

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_accessibility_governance.ps1
.\build\dev-ninja-debug\urpg_tests.exe "[accessibility]" --reporter compact
```

**Done when:**
- The script passes locally.
- The script is referenced in the local gate runner.

---

### S16-T03: Thread Evidence into ProjectAudit/Governance

**Goal:** Reconcile ProjectAudit governance and readiness records with the live-ingestion evidence.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`

**Implementation targets:**
- Update `addAccessibilityArtifactGovernance` in `urpg_project_audit.cpp` to also check for:
  - `editor/accessibility/accessibility_menu_adapter.h`
  - `editor/accessibility/accessibility_menu_adapter.cpp`
  - `tools/ci/check_accessibility_governance.ps1`
  - `content/fixtures/accessibility_report_fixture.json`
- Update `readiness_status.json` for `accessibility_auditor`:
  - Set `diagnostics` to `true`.
  - Update `mainGaps` to reflect the bounded menu-adapter scope and remaining surface coverage.
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
