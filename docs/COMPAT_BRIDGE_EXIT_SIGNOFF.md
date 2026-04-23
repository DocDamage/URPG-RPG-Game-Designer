# Compat Bridge Exit — Closure Sign-off

> **Status:** `PARTIAL`  
> **Purpose:** Evidence-gathering artifact for compat bridge exit review.  
> **Date:** 2026-04-23  
> **Rule:** This document does **not** promote `readiness_status.json` status. Human review is required for promotion to `READY`.

---

## 1. Runtime Scope

Compat bridge exit is intentionally scoped as an import, validation, and migration bridge rather than a production JavaScript runtime:

| Surface | Scope | Evidence |
|---------|-------|----------|
| `quickjs_runtime` | Fixture-backed compat-contract harness for import/verification workflows | `runtimes/compat_js/quickjs_runtime.h`, `tests/unit/test_quickjs_runtime.cpp` |
| Compat managers (`audio`, `battle`, `data`, `window`, `plugin`, `input`) | Deterministic harness-backed behavior with explicit `PARTIAL`/`STUB` truth surfaces where live engine/backend parity does not exist | `runtimes/compat_js/`, focused unit tests under `tests/unit/` |
| Migration/report tooling | Structured JSONL/report/export surfaces for compat diagnostics and migration planning | `tests/compat/test_compat_plugin_failure_diagnostics.cpp`, `editor/diagnostics/diagnostics_workspace.cpp` |

The authoritative claim is bounded: compat is trustworthy enough to support migration and verification without overstating runtime parity.
This is also the accepted TD-03 strategy in-tree: the compat harness is the only supported scripting/plugin bridge, and a live JavaScript runtime is future feature scope rather than unfinished current debt.

---

## 2. Evidence Anchors

Current evidence for the compat bridge exit lane includes:

| Area | Evidence |
|------|----------|
| Curated compat corpus | `tests/compat/test_compat_plugin_fixtures.cpp` covers the active 10-profile executable fixture corpus and keeps the two corpus-health fixtures separate from executable-surface claims |
| Failure-path parity | `tests/compat/test_compat_plugin_failure_diagnostics.cpp` proves JSONL export, report ingestion/export, and panel projection parity |
| Truthful status labels | `tests/unit/test_audio_manager.cpp`, `tests/unit/test_battlemgr.cpp`, `tests/unit/test_data_manager.cpp`, `tests/unit/test_window_compat.cpp`, `tests/unit/test_input_manager.cpp`, `tests/unit/test_plugin_manager.cpp` |
| Weekly maintenance lane | `tools/ci/run_compat_weekly_regression.ps1` plus the `weekly` CTest lane |
| Canonical exit criteria | `docs/COMPAT_EXIT_CHECKLIST.md` |

---

## 3. Diagnostics and Reporting

Compat bridge exit evidence is exposed through governed diagnostics surfaces:

- structured PluginManager failure diagnostics export as JSONL
- compat report model ingestion/export parity
- diagnostics workspace export parity with the rendered panels
- status/docs alignment through the canonical readiness and remediation stack

This keeps new compat failure modes from becoming silent or checklist-only evidence.

---

## 4. Remaining Residual Gaps (Honest Scope Limits)

1. **Human exit sign-off**: This artifact exists to support review, but `compat_bridge_exit` still requires explicit maintainer/release-owner/doc-truth signoff before any future promotion beyond `PARTIAL`.
2. **Curated corpus maintenance**: The active corpus is evidence-backed today, but continued fixture maintenance and weekly regression depth are ongoing obligations rather than one-time closure.
3. **Truth upkeep**: Fixture-backed or placeholder-backed compat paths must remain conservatively labeled as the implementation evolves.
4. **Live runtime/backend parity**: Out of scope for this lane. Compat remains a bounded bridge, not a finished JS runtime or live backend surface.

---

## 5. Review Cadence

Compat bridge exit is a maintenance-mode lane that requires ongoing review rather than a one-time closure:

| Review Trigger | Required Action |
|----------------|----------------|
| New compat fixture added to curated corpus | Confirm it is covered by `test_compat_plugin_fixtures.cpp` and the weekly regression lane |
| Plugin manifest change affecting a curated profile | Update the corresponding fixture and re-run `ctest -L weekly` |
| Compat status label changed from `PARTIAL` to anything else | Human maintainer must verify the new label against `COMPAT_EXIT_CHECKLIST.md` before commit |
| Monthly scheduled review | Run `tools/ci/run_compat_weekly_regression.ps1` and record outcome in a reviewer note below |
| Any promotion consideration | Reviewer must sign the checklist in `docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md` and file a deliberate edit to `readiness_status.json` |

---

## 6. Related Artifacts

- `docs/COMPAT_EXIT_CHECKLIST.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- `docs/RELEASE_READINESS_MATRIX.md`
- `content/readiness/readiness_status.json`

---

*Sign-off prepared by governance agent. Promotion to `READY` requires human review of the residual gaps above.*
