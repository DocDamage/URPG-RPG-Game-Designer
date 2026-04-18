# Compat Exit Checklist

Status Date: 2026-04-18

This checklist is the canonical pass/fail artifact for deciding whether the compat lane is trustworthy enough to serve as an import, validation, and migration bridge without overstating runtime parity.

Cross-cutting source of truth: [TECHNICAL_DEBT_REMEDIATION_PLAN.md](./TECHNICAL_DEBT_REMEDIATION_PLAN.md)

## Purpose

Use this document to gate compat-lane completion against explicit evidence instead of optimistic status labels.

It is intentionally narrower than full native-feature completion:
- Compat must be trustworthy.
- Compat does not need to be a production JavaScript runtime.
- Remaining fixture-backed behavior must be documented honestly.

## Import Confidence

- [ ] Curated compat corpus covers the active plugin/import profiles that matter for migration planning.
- [ ] New failure operations are locked to JSONL export, report ingestion, and panel projection parity.
- [ ] QuickJS is still documented as a fixture-backed compat harness everywhere it is referenced.
- [ ] Runtime status labels remain aligned with actual bridge behavior; fixture-backed or placeholder-backed paths are not labeled `FULL` without evidence.
- [ ] Focused compat suites pass in the active local build lane.

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

- AudioManager compat closure advanced on 2026-04-18:
  - deterministic playback position now advances during `update()`
  - deterministic duck/unduck ramps are covered by focused tests
  - master/bus volume changes now affect active compat playback
  - the QuickJS `AudioManager` object now routes live compat state for BGM/BGS/ME/SE, volume, and ducking helpers
  - status labels remain `PARTIAL` because the surface still models deterministic harness behavior rather than a live mixer/backend
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
