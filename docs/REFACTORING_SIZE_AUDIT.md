# Refactoring Audit — Files Requiring Size Reduction

**Generated:** 2026-04-22
**Scope:** `engine/`, `editor/`, `runtimes/`, `tests/`, `tools/`
**Thresholds:** Critical >2000 lines | High 1000–2000 lines | Medium 300–999 lines

---

## Summary

| Priority | Count | Description |
|----------|------:|-------------|
| Critical | 7 | Immediate split required — monolithic files causing maintainability and review burden |
| High     | 5 | Should be split before next major feature work |
| Medium   | 28 | Candidates for incremental extraction during normal feature work |

---

## Critical — Over 2,000 Lines

| File | Lines | Size (KB) | Suggested Action |
|------|------:|----------:|------------------|
| `tests/compat/test_compat_plugin_failure_diagnostics.cpp` | 4,033 | 164.3 | Split by failure category: load errors, permission errors, conflict errors, diagnostics output |
| `tests/compat/test_compat_plugin_fixtures.cpp` | 2,859 | 153.7 | Extract fixture factory helpers into a shared `compat_test_helpers` library; split by plugin domain |
| `tests/unit/test_diagnostics_workspace.cpp` | 2,376 | 141.6 | Split into `test_diagnostics_workspace_model.cpp`, `test_diagnostics_workspace_serialization.cpp`, `test_diagnostics_workspace_panels.cpp` |
| `runtimes/compat_js/plugin_manager.cpp` | 2,350 | 94.6 | Extract: plugin loading, security validation, dependency resolution, lifecycle hooks into separate translation units |
| `runtimes/compat_js/window_compat.cpp` | 2,369 | 91.6 | Split into `window_compat_core.cpp`, `window_compat_drawing.cpp`, `window_compat_text.cpp`, `window_compat_input.cpp` |
| `editor/diagnostics/diagnostics_workspace.cpp` | 2,198 | 90.9 | Extract panel rendering, model serialization, and filter logic into separate files under `editor/diagnostics/` |
| `tools/audit/urpg_project_audit.cpp` | 2,183 | 91.8 | Split CLI entrypoint from audit logic; extract reporters and checkers into `audit/checks/` subdirectory |

---

## High Priority — 1,000–2,000 Lines

| File | Lines | Size (KB) | Suggested Action |
|------|------:|----------:|------------------|
| `runtimes/compat_js/battle_manager.cpp` | 1,921 | 79.5 | Extract skill resolution, party AI, and turn-order logic into separate modules |
| `runtimes/compat_js/data_manager.cpp` | 1,744 | 65.8 | Split database loading, actor data, item data, and map data into domain-specific files |
| `tests/unit/test_plugin_manager.cpp` | 1,660 | 65.0 | Split into `test_plugin_manager_load.cpp`, `test_plugin_manager_security.cpp`, `test_plugin_manager_lifecycle.cpp` |
| `tests/unit/test_window_compat.cpp` | 1,655 | 62.4 | Mirror source split: `test_window_compat_drawing.cpp`, `test_window_compat_text.cpp`, etc. |
| `tests/unit/test_migration_wizard.cpp` | 1,383 | 62.7 | Split by migration stage: inference, decoration, upgrade; extract fixture setup |

---

## Medium — 300–999 Lines

These files exceed comfortable single-responsibility size and should be addressed incrementally.

### Runtime / Engine Sources

| File | Lines | Size (KB) | Notes |
|------|------:|----------:|-------|
| `engine/core/scene/battle_scene.cpp` | 950 | 38.1 | Scene setup, update loop, and rendering logic should be separated |
| `runtimes/compat_js/audio_manager.cpp` | 789 | 32.9 | BGM, BGS, SE, and ME handling can each be their own module |
| `runtimes/compat_js/input_manager.cpp` | 764 | 29.7 | Input binding, gamepad, and keyboard handlers should be split |
| `runtimes/compat_js/quickjs_runtime.cpp` | 651 | 21.4 | Separate JS value marshalling from runtime lifecycle |
| `engine/core/message/message_core.cpp` | 731 | 24.6 | Message parsing, command dispatch, and rendering are distinct concerns |
| `engine/core/battle/battle_migration.h` | 574 | 28.4 | Header is too large; split schema types from migration functions |
| `engine/core/save/save_catalog.cpp` | 404 | 16.2 | Separate catalog indexing from slot serialization |
| `engine/core/message/message_migration.cpp` | 342 | 15.9 | Extract per-command migration handlers |
| `engine/core/save/save_migration.cpp` | 301 | 15.5 | Extract per-version upgrade steps |

### Editor Sources

| File | Lines | Size (KB) | Notes |
|------|------:|----------:|-------|
| `editor/compat/compat_report_panel.cpp` | 715 | 25.1 | Separate panel rendering from report data model |
| `editor/diagnostics/project_audit_panel.cpp` | 509 | 22.6 | Extract filter UI from the audit result display |
| `editor/diagnostics/migration_wizard_model.h` | 498 | 21.2 | Header carrying too much implementation; move to `.cpp` where possible |
| `editor/ui/menu_inspector_model.cpp` | 490 | 20.4 | Inspector state and serialization should be separated |

### Runtime Headers

| File | Lines | Size (KB) | Notes |
|------|------:|----------:|-------|
| `runtimes/compat_js/window_compat.h` | 657 | 22.0 | Public interface is too wide; split into sub-headers by feature area |
| `runtimes/compat_js/battle_manager.h` | 450 | 17.0 | Move private implementation details to `battle_manager_impl.h` or `.cpp` |
| `runtimes/compat_js/data_manager.h` | 496 | 16.1 | Split by data domain (actors, items, maps, system) |

### Test Files

| File | Lines | Size (KB) | Notes |
|------|------:|----------:|-------|
| `tests/unit/test_compat_report_panel.cpp` | 723 | 29.9 | Group by panel section; extract shared fixture construction |
| `tests/unit/test_menu_core.cpp` | 689 | 28.6 | Split by menu subsystem (main, item, skill, status) |
| `tests/unit/test_project_audit_cli.cpp` | 947 | 48.9 | Split CLI option tests from report output tests |
| `tests/unit/test_battlemgr.cpp` | 930 | 38.7 | Split by combat phase: setup, turn, resolution, end |
| `tests/unit/test_battle_migration.cpp` | 828 | 34.0 | Split by migration version bands |
| `tests/unit/test_data_manager.cpp` | 467 | 20.3 | Split by data category |
| `tests/unit/test_quickjs_runtime.cpp` | 450 | 18.1 | Separate lifecycle tests from value-marshalling tests |
| `tests/unit/test_audio_manager.cpp` | 394 | 16.9 | Split by audio channel type |
| `tests/unit/test_presentation_runtime.cpp` | 410 | 19.0 | Split by presentation layer concern |
| `tests/unit/test_project_audit_panel.cpp` | 480 | 24.3 | Split by panel section / audit category |
| `tests/unit/test_menu_inspector_model.cpp` | 363 | 16.6 | Split by inspector pane |
| `tests/unit/test_spatial_editor.cpp` | 346 | 16.0 | Split by spatial operation type |

---

## Refactoring Guidelines

1. **One concern per file.** A source file should implement a single class, subsystem, or closely-related cluster of functions.
2. **Test file parity.** When a source file is split, split the corresponding test file to match.
3. **Header hygiene.** Headers exceeding ~200 lines should be reviewed; move non-interface implementation to `.cpp` or `_impl.h`.
4. **No behavior changes.** All refactoring splits must keep the build green and all tests passing at the `pr` gate before merging.
5. **One split per PR.** Keep refactoring PRs small and reviewable; do not bundle multiple splits or unrelated changes.

---

## Tracking

Progress on Critical and High items should be tracked as individual issues. Medium items may be resolved opportunistically during normal feature work touching those files.
