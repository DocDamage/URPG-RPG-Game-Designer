# Compat Exit Checklist

Status Date: 2026-04-20

This checklist is the canonical pass/fail artifact for deciding whether the compat lane is trustworthy enough to serve as an import, validation, and migration bridge without overstating runtime parity.

Cross-cutting source of truth: [TECHNICAL_DEBT_REMEDIATION_PLAN.md](./TECHNICAL_DEBT_REMEDIATION_PLAN.md)

Phase 2 runtime closure completed on 2026-04-19. The unchecked items below are post-closure import/migration confidence gates and ongoing truth-maintenance requirements, not evidence that the baseline Phase 2 runtime closure is still open.

## Purpose

Use this document to gate compat-lane completion against explicit evidence instead of optimistic status labels.

It is intentionally narrower than full native-feature completion:
- Compat must be trustworthy.
- Compat does not need to be a production JavaScript runtime.
- Remaining fixture-backed behavior must be documented honestly.

## Import Confidence

- [x] Curated compat corpus covers the active plugin/import profiles that matter for migration planning.
- [x] New failure operations are locked to JSONL export, report ingestion, and panel projection parity.
- [x] QuickJS is still documented as a fixture-backed compat harness everywhere it is referenced.
- [x] Runtime status labels remain aligned with actual bridge behavior; fixture-backed or placeholder-backed paths are not labeled `FULL` without evidence.
- [x] Focused compat suites pass in the active local build lane.

Evidence anchors:
- [TECHNICAL_DEBT_REMEDIATION_PLAN.md](./TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md)
- [test_compat_plugin_fixtures.cpp](../tests/compat/test_compat_plugin_fixtures.cpp)
- [test_compat_plugin_failure_diagnostics.cpp](../tests/compat/test_compat_plugin_failure_diagnostics.cpp)

## Migration Confidence

- [ ] Wave 1 migration targets have schema/import/export anchors or explicit waivers.
- [ ] Diagnostics/export surfaces reflect what the editor can actually render today.
- [ ] Compat subsystems with strong planning weight have regression anchors for their current claimed scope.
- [ ] Audio, battle, data, and window compat lanes no longer rely on silently stale status metadata.
- [ ] Remaining partial behavior is documented as deterministic harness behavior rather than live backend parity.

Evidence anchors:
- [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md](./WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)
- [test_audio_manager.cpp](../tests/unit/test_audio_manager.cpp)
- [test_battlemgr.cpp](../tests/unit/test_battlemgr.cpp)
- [test_data_manager.cpp](../tests/unit/test_data_manager.cpp)
- [test_window_compat.cpp](../tests/unit/test_window_compat.cpp)

## Current Notes

- Phase 2 runtime closure completed on 2026-04-19:
  - battle reward/event cadence and switch coverage are closed in the compat lane
  - `DataManager::loadDatabase()` seeded-container behavior is explicitly covered
  - `Window_Base::contents()` lifecycle truthfulness is explicitly covered
  - AudioManager remains honestly `PARTIAL` because the surface still models deterministic harness behavior rather than a live mixer/backend
- AudioManager compat closure advanced on 2026-04-18:
  - deterministic playback position now advances during `update()`
  - deterministic duck/unduck ramps are covered by focused tests
  - master/bus volume changes now affect active compat playback
  - the QuickJS `AudioManager` object now routes live compat state for BGM/BGS/ME/SE, volume, and ducking helpers
- Active local compat snapshot refreshed on 2026-04-20:
  - `.\build\Debug\urpg_compat_tests.exe --reporter compact` => 3375 assertions / 43 test cases passed
- Failure-path parity reconciled on 2026-04-20:
  - `test_compat_plugin_failure_diagnostics.cpp` already proves mixed-chain failure operations survive JSONL export, report-model ingestion/export, and compat report panel projection
  - parity anchors include `execute_command_dependency_missing`, `execute_command_quickjs_call`, `execute_command_quickjs_context_missing`, `execute_command_by_name_parse`, `load_plugins_directory_scan`, and `load_plugins_directory_scan_entry`
- Weekly compat runner stabilized on 2026-04-20:
  - `tools/ci/run_compat_weekly_regression.ps1` now configures/builds `urpg_compat_tests` and runs the CTest `weekly` lane as a dedicated maintenance command
- Curated compat corpus reconciled on 2026-04-20:
  - the active fixture corpus already covers 10 curated plugin/import profiles through `fixtureSpecs()` plus all-profile orchestration and reload-survival scenarios in `test_compat_plugin_fixtures.cpp`
  - the same corpus is exercised in the compat weekly lane and missing-command/failure diagnostics coverage
- Update this checklist whenever a compat surface changes status, link location, or exit evidence.

## Signed Off By

- [ ] Compat/runtime maintainer
- [ ] Tech lead or release owner
- [ ] Documentation truthfulness pass completed

## Related Documents

- [PROGRAM_COMPLETION_STATUS.md](./PROGRAM_COMPLETION_STATUS.md)
- [TECHNICAL_DEBT_REMEDIATION_PLAN.md](./TECHNICAL_DEBT_REMEDIATION_PLAN.md)
- [DEVELOPMENT_KICKOFF.md](./DEVELOPMENT_KICKOFF.md)
- [WORKLOG.md](../WORKLOG.md)
