# URPG External Feature Upgrade Tickets

Date: 2026-04-15  
Source basis: `docs/EXTERNAL_FEATURE_UPGRADE_SHORTLIST_2026-04-15.md` + external repo scan batches  
Goal: concrete, implementation-ready tickets with scope, file ownership, and test gates

This is a planning ticket set. Test commands below should be run against the active local debug build directory or matching CTest preset, not assumed to require one specific generated tree.

## Ticket Summary

| Ticket | Priority | Lane | Effort | Status |
| --- | --- | --- | --- | --- |
| `URPG-EXT-001` Deterministic Pathfinding Runtime | P0 | 3.3 | M | Planned |
| `URPG-EXT-002` Dialogue Script Compiler + Roundtrip Export | P0 | 2 | M | Planned |
| `URPG-EXT-003` Behavior Tree Runtime for NPC/Battle AI | P1 | 3.1 | L | Planned |
| `URPG-EXT-004` Deterministic Export Packaging CLI | P0 | 4 | M | Planned |
| `URPG-EXT-005` Localization Extraction/Writeback + Lint Gate | P1 | 2 + 4 | M | Planned |
| `URPG-EXT-006` Structured Telemetry and Crash Envelope Pipeline | P1 | 1 + 4 | M | Planned |

## URPG-EXT-001: Deterministic Pathfinding Runtime

### Objective

Ship deterministic pathfinding for map/event movement and tactical positioning with inspector-visible debug overlays.

### Scope

- Add tile-graph pathfinding kernel with stable tie-break behavior.
- Add movement routing adapter for map scene and event runtime.
- Add editor diagnostics projection for path cost and blocked-cell reason.

### Out Of Scope

- Full navmesh streaming and crowd simulation (future follow-up).
- Non-deterministic heuristic tuning modes.

### Ownership Targets

- Runtime:
  - `engine/core/level/pathfinding_graph.h` (new)
  - `engine/core/level/pathfinding_graph.cpp` (new)
  - `engine/core/level/path_request_router.h` (new)
  - `engine/core/level/path_request_router.cpp` (new)
  - `engine/core/scene/map_scene.cpp`
  - `engine/core/events/event_runtime.cpp`
- Editor:
  - `editor/diagnostics/diagnostics_workspace.cpp`
  - `editor/diagnostics/pathfinding_debug_panel.h` (new)
  - `editor/diagnostics/pathfinding_debug_panel.cpp` (new)
- Build:
  - `CMakeLists.txt`

### Test Gates

- Unit:
  - `tests/unit/test_pathfinding_graph.cpp` (new)
  - `tests/unit/test_path_request_router.cpp` (new)
- Integration:
  - `tests/integration/test_integration_pathfinding_scene.cpp` (new)
- Gate command:
  - `ctest --test-dir <active-debug-build-dir> -R "pathfinding|path_request|integration_pathfinding" --output-on-failure`

### Acceptance Criteria

- Same input map/start/goal always produces identical path output and cost.
- Blocked path returns deterministic failure reason and no partial movement side-effects.
- Diagnostics panel shows route nodes, blocked reason, and final cost for last query.

## URPG-EXT-002: Dialogue Script Compiler + Roundtrip Export

### Objective

Enable text-first narrative authoring by compiling script text into native message/event structures and exporting back for roundtrip edits.

### Scope

- Add compiler from text script format to `dialogue_sequences` + event-command IR.
- Add reverse exporter from IR to canonical script text.
- Wire import/export hooks in message inspector.

### Out Of Scope

- Full third-party scripting language compatibility.
- LLM translation/autowriter integration.

### Ownership Targets

- Runtime/compiler:
  - `engine/core/message/dialogue_script_compiler.h` (new)
  - `engine/core/message/dialogue_script_compiler.cpp` (new)
  - `engine/core/message/dialogue_script_exporter.h` (new)
  - `engine/core/message/dialogue_script_exporter.cpp` (new)
  - `engine/core/message/message_core.cpp`
- Editor:
  - `editor/message/message_inspector_model.cpp`
  - `editor/message/message_inspector_panel.cpp`
- Schemas:
  - `content/schemas/dialogue_script.schema.json` (new)
- Build/tooling:
  - `CMakeLists.txt`
  - `tools/migrate/migration_op.json` (if script migration mapping is needed)

### Test Gates

- Unit:
  - `tests/unit/test_dialogue_script_compiler.cpp` (new)
  - `tests/unit/test_dialogue_script_exporter.cpp` (new)
  - Extend `tests/unit/test_message_schema_contracts.cpp`
- Snapshot:
  - `tests/snapshot/test_snapshot_dialogue_roundtrip.cpp` (new)
- Gate command:
  - `ctest --test-dir <active-debug-build-dir> -R "dialogue_script|message_schema|snapshot_dialogue" --output-on-failure`

### Acceptance Criteria

- Source script -> IR -> export script roundtrip is stable (canonicalized diff only).
- Compiler rejects malformed script with structured diagnostic rows.
- Inspector can import script into active message asset and export back out.

## URPG-EXT-003: Behavior Tree Runtime for NPC/Battle AI

### Objective

Add a deterministic behavior-tree execution lane for reactive AI in map and battle contexts.

### Scope

- Implement core BT runtime (`Selector`, `Sequence`, `Condition`, `Action`, `Cooldown`).
- Add blackboard for scoped AI dataflow.
- Integrate battle decision hook for enemy action selection.

### Out Of Scope

- Visual BT editor (deferred to follow-up ticket).
- Learning-based AI policies.

### Ownership Targets

- Runtime:
  - `engine/core/ai/behavior_tree_node.h` (new)
  - `engine/core/ai/behavior_tree_runtime.h` (new)
  - `engine/core/ai/behavior_tree_runtime.cpp` (new)
  - `engine/core/ai/behavior_blackboard.h` (new)
  - `engine/core/ai/behavior_blackboard.cpp` (new)
  - `engine/core/battle/battle_core.cpp`
  - `engine/core/events/event_runtime.cpp`
- Build:
  - `CMakeLists.txt`

### Test Gates

- Unit:
  - `tests/unit/test_behavior_tree_runtime.cpp` (new)
  - `tests/unit/test_behavior_blackboard.cpp` (new)
- Battle integration:
  - `tests/unit/test_battle_ai_behavior_tree.cpp` (new)
- Gate command:
  - `ctest --test-dir <active-debug-build-dir> -R "behavior_tree|behavior_blackboard|battle_ai" --output-on-failure`

### Acceptance Criteria

- BT tick order and status transitions are deterministic.
- Cooldown/action nodes are replay-safe under deterministic mode.
- Battle enemy action selection can be switched from legacy branch logic to BT lane behind feature flag.

## URPG-EXT-004: Deterministic Export Packaging CLI

### Objective

Provide reproducible export packaging for desktop/web targets with optional encryption and artifact pruning.

### Scope

- Implement packager runtime body for current `export_packager` API.
- Add CLI entrypoint for export packaging.
- Add CI/local gate smoke for packaging command.

### Out Of Scope

- Store/publisher upload automation.
- Platform-native installer creation.

### Ownership Targets

- Runtime/tools:
  - `engine/core/tools/export_packager.h` (existing contract)
  - `engine/core/tools/export_packager.cpp` (new)
  - `tools/pack/pack_cli.cpp` (new)
- CI:
  - `tools/ci/run_local_gates.ps1`
  - `tools/ci/known_break_waivers.json` (if temporary waivers are needed)
- Build:
  - `CMakeLists.txt` (new executable target, e.g. `urpg_pack`)

### Test Gates

- Unit:
  - `tests/unit/test_export_packager.cpp` (new)
- Integration:
  - `tests/integration/test_integration_export_packager.cpp` (new)
- Gate command:
  - `ctest --test-dir <active-debug-build-dir> -R "export_packager|integration_export_packager" --output-on-failure`

### Acceptance Criteria

- Same input project hash produces same package manifest hash.
- Packaging CLI supports target selection and output directory controls.
- CI/local gate can run a packaging smoke step without mutating unrelated artifacts.

## URPG-EXT-005: Localization Extraction/Writeback + Lint Gate

### Objective

Ship first-class localization operations: extract, edit, writeback, and validate narrative text across supported data lanes.

### Scope

- Add localization catalog model and canonical file format.
- Add extraction/writeback tooling for message/event text.
- Add localization lint checks into CI.

### Out Of Scope

- Machine translation provider integrations.
- Live collaborative translation server.

### Ownership Targets

- Runtime/data:
  - `engine/core/message/localization_catalog.h` (new)
  - `engine/core/message/localization_catalog.cpp` (new)
  - `engine/core/message/message_core.cpp`
  - `engine/core/message/message_migration.cpp`
- Schema:
  - `content/schemas/localization_catalog.schema.json` (new)
- Tools/CI:
  - `tools/localization/extract_localization.cpp` (new)
  - `tools/localization/writeback_localization.cpp` (new)
  - `tools/ci/check_localization_consistency.ps1` (new)
- Build:
  - `CMakeLists.txt`

### Test Gates

- Unit:
  - `tests/unit/test_localization_catalog.cpp` (new)
  - Extend `tests/unit/test_message_text_core.cpp`
- CI:
  - add lint validation step in local gates
- Gate command:
  - `ctest --test-dir <active-debug-build-dir> -R "localization_catalog|message_text_core" --output-on-failure`

### Acceptance Criteria

- Extraction produces stable canonical catalog ordering.
- Writeback updates only localization fields and preserves non-localized metadata.
- CI check fails deterministically on missing keys, duplicate keys, or invalid locale tags.

## URPG-EXT-006: Structured Telemetry and Crash Envelope Pipeline

### Objective

Unify runtime/editor crash and failure diagnostics into structured telemetry envelopes for faster triage.

### Scope

- Add telemetry event schema + writer API.
- Add crash envelope serialization with subsystem/version metadata.
- Bridge compat/plugin failure diagnostics into telemetry sink.

### Out Of Scope

- Mandatory external telemetry backend.
- PII-rich logging.

### Ownership Targets

- Runtime:
  - `engine/core/telemetry/telemetry_event.h` (new)
  - `engine/core/telemetry/telemetry_event.cpp` (new)
  - `engine/core/telemetry/crash_envelope.h` (new)
  - `engine/core/telemetry/crash_envelope.cpp` (new)
  - `runtimes/compat_js/plugin_manager.cpp`
  - `engine/core/debug/debug_session.cpp`
- Editor:
  - `editor/diagnostics/diagnostics_facade.cpp`
  - `editor/compat/compat_report_panel.cpp`
- Build:
  - `CMakeLists.txt`

### Test Gates

- Unit:
  - `tests/unit/test_telemetry_event.cpp` (new)
  - `tests/unit/test_crash_envelope.cpp` (new)
- Compat:
  - extend `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
- Gate command:
  - `ctest --test-dir <active-debug-build-dir> -R "telemetry|crash_envelope|compat_plugin_failure_diagnostics" --output-on-failure`

### Acceptance Criteria

- Every crash/failure event written through diagnostics facade includes deterministic envelope fields.
- Compat severity mapping (`WARN`/`SOFT_FAIL`/`HARD_FAIL`/`CRASH_PREVENTED`) survives telemetry projection.
- Telemetry output can be fully disabled via config switch without changing core execution semantics.

## Suggested Execution Order

1. `URPG-EXT-001` Pathfinding runtime
2. `URPG-EXT-002` Dialogue compiler + roundtrip export
3. `URPG-EXT-004` Export packaging CLI
4. `URPG-EXT-005` Localization operations
5. `URPG-EXT-006` Telemetry pipeline
6. `URPG-EXT-003` Behavior tree runtime

## Planning Notes

- All tickets follow existing policy: architecture patterns may be absorbed from external sources; direct third-party code import is not assumed.
- Each ticket is structured to fit the current `ctest` gate model and existing `urpg_core` + `urpg_tests` build wiring.
