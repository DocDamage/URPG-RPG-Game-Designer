# Architecture Map

URPG is organized around a native C++ runtime with bounded compatibility and editor layers.

## Main Ownership Areas

| Area | Path | Notes |
| --- | --- | --- |
| Runtime core | `engine/core/` | ECS-adjacent kernels, scenes, render/audio/save/battle/message/input systems. |
| High-level API | `engine/api/` | Engine API singleton and public runtime entry points. |
| Editor | `editor/` | ImGui panels and editor models. Release top-level panels are governed by `engine/core/editor/editor_panel_registry.*`. |
| Native Level Builder | `editor/spatial/level_builder_workspace.*`; `engine/core/map/grid_part_*` | Top-level shippable map editor. `GridPartDocument` is the canonical editable map document; legacy spatial tools are supporting modes. |
| Compat JS | `runtimes/compat_js/` | QuickJS harness plus RPG Maker MZ managers and Window/Battle/Data/Input/Audio surfaces. |
| Native/script bridge | `engine/runtimes/bridge/` | Value bridge for script interop. |
| Tests | `tests/` | Unit, integration, snapshot, compat, and engine-core lanes. |
| Tools | `tools/` | CI, docs, assets, packaging, migration, and workflow scripts. |

## Architectural Rules

- Presentation Core is the source of truth for visual interpretation; render backends consume frame intent or render commands and must not reach into game state.
- Level Builder is the primary native map authoring surface. Spatial Authoring remains supporting tooling for elevation, props, ability bindings, and direct subsystem tests.
- Runtime/editor/compat surfaces should expose truthful status. Do not label fixture-backed or partial behavior as release-ready without tests and docs.
- Subsystems are considered landed only when they build, are test-registered, and are reachable from a runtime/editor/tool entry point.
- Migration and compat fallbacks must preserve unsupported source data and emit diagnostics rather than silently dropping behavior.
- Release exports must distinguish bootstrap/dev artifacts from production-playable artifacts.

## Deeper References

- ADRs: `docs/adr/`
- Presentation validation: `docs/presentation/VALIDATION.md`
- Release execution plan: `docs/release/AAA_RELEASE_EXECUTION_PLAN.md`
- Program status: `docs/PROGRAM_COMPLETION_STATUS.md`
