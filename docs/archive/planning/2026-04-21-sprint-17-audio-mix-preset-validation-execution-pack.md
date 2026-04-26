# S17 Execution Pack

> **For agentic workers:** Work strictly ticket-by-ticket. Keep checkbox state current in this file and the paired task board.

**Goal:** Close the audio mix preset diagnostics gap with a live validation runtime, a focused CI gate script, and reconciled governance.

**Sprint theme:** audio mix preset validation pipeline

**Primary outcomes for this sprint:**
- A real `AudioMixValidator` runtime checks audio mix presets for conflicting duck rules, orphaned categories, volume overflows, and missing defaults.
- A focused CI gate script validates the audio mix artifact contract.
- ProjectAudit governance and readiness docs reflect the validation evidence.

**Non-goals for this sprint:**
- No new audio backend integration beyond the current compat-truth harness.
- No unrelated subsystem work.
- No claim that the validator covers all possible audio edge cases (bounded rule set only).

---

## Canonical Sources

Read these before starting `S17-T01`:

- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`

Add sprint-specific sources below before starting implementation:

- `engine/core/audio/audio_mix_presets.h`
- `engine/core/audio/audio_mix_presets.cpp`
- `editor/audio/audio_mix_panel.h`
- `editor/audio/audio_mix_panel.cpp`
- `content/schemas/audio_mix_presets.schema.json`
- `tools/audit/urpg_project_audit.cpp`
- `tests/unit/test_audio_mix_presets.cpp`

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

### S17-T01: Audio Mix Validator Runtime + Editor Adapter

**Goal:** Build a real validation runtime that audits `AudioMixPresetBank` entries and produces structured issues.

**Primary files:**
- `engine/core/audio/audio_mix_validator.h` (new)
- `engine/core/audio/audio_mix_validator.cpp` (new)
- `editor/audio/audio_mix_validator_adapter.h` (new)
- `editor/audio/audio_mix_validator_adapter.cpp` (new)
- `tests/unit/test_audio_mix_presets.cpp`
- `CMakeLists.txt`

**Implementation targets:**
- Create `AudioMixValidator` with `validate(const AudioMixPresetBank& bank)` returning `std::vector<AudioMixIssue>`.
- Validation rules:
  - **MissingDefaultPreset** → Error if no preset with `isDefault == true`.
  - **ConflictingDuckRules** → Warning if two presets declare ducking for the same target category with different gain values.
  - **OrphanedCategoryMapping** → Warning if a category volume mapping references a category not present in the bank's canonical category list.
  - **VolumeOutOfRange** → Error if any volume value is outside `[0.0f, 1.0f]`.
  - **DuplicatePresetId** → Error if two presets share the same `id`.
- Create `AudioMixValidatorAdapter` with a static method `ingest(const AudioMixPanel& panel)` returning the validator input shape.
- Add focused test in `test_audio_mix_presets.cpp` that:
  1. Constructs an `AudioMixPresetBank` with intentionally bad presets.
  2. Runs `AudioMixValidator::validate()`.
  3. Asserts the expected issues are found.
  4. Ingests via the adapter from a panel snapshot and re-validates.
- Register new `.cpp` files in `CMakeLists.txt` under `urpg_core`.

**Verification:**

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[audio][mix]" --reporter compact
```

**Done when:**
- The validator compiles and the new test passes.
- Existing audio mix tests remain green.

---

### S17-T02: CI Gate Script for Canonical Audio Governance Fixture

**Goal:** Add a focused executable gate for the audio mix artifact contract.

**Primary files:**
- `tools/ci/check_audio_governance.ps1` (new)
- `content/fixtures/audio_mix_presets_fixture.json` (new)
- `tests/unit/test_audio_mix_presets.cpp`
- `tools/ci/run_local_gates.ps1`

**Implementation targets:**
- Create `tools/ci/check_audio_governance.ps1` that validates:
  - `content/schemas/audio_mix_presets.schema.json` exists and is valid JSON.
  - `engine/core/audio/audio_mix_presets.h` and `.cpp` exist.
  - `engine/core/audio/audio_mix_validator.h` and `.cpp` exist.
  - `editor/audio/audio_mix_panel.h` and `.cpp` exist.
  - `editor/audio/audio_mix_validator_adapter.h` and `.cpp` exist.
  - A canonical audio mix fixture exists at `content/fixtures/audio_mix_presets_fixture.json` (create a minimal valid fixture if missing).
- The script should emit a JSON result shape matching the validator pattern: `{"passed": true/false, "errors": [...]}`.
- Add a PowerShell-invocation test in `test_audio_mix_presets.cpp` that invokes the script and asserts it passes.
- Wire the script into `tools/ci/run_local_gates.ps1` after the build step (add an invocation comment or actual call if the gate structure is obvious).

**Verification:**

```powershell
powershell -ExecutionPolicy Bypass -File tools/ci/check_audio_governance.ps1
.\build\dev-ninja-debug\urpg_tests.exe "[audio][mix]" --reporter compact
```

**Done when:**
- The script passes locally.
- The script is referenced in the local gate runner.

---

### S17-T03: Thread Evidence into ProjectAudit/Governance

**Goal:** Reconcile ProjectAudit governance and readiness records with the validation evidence.

**Primary files:**
- `tools/audit/urpg_project_audit.cpp`
- `content/readiness/readiness_status.json`
- `docs/RELEASE_READINESS_MATRIX.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`

**Implementation targets:**
- Update `addAudioArtifactGovernance` in `urpg_project_audit.cpp` to also check for:
  - `engine/core/audio/audio_mix_validator.h`
  - `engine/core/audio/audio_mix_validator.cpp`
  - `editor/audio/audio_mix_validator_adapter.h`
  - `editor/audio/audio_mix_validator_adapter.cpp`
  - `tools/ci/check_audio_governance.ps1`
  - `content/fixtures/audio_mix_presets_fixture.json`
- Update `readiness_status.json` for `audio_mix_presets`:
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
