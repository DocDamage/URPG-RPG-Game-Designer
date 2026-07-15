# Architecture Map

URPG is organized around a native C++ runtime with bounded compatibility and editor layers.

## Main Ownership Areas

| Area | Path | Notes |
| --- | --- | --- |
| Runtime core | `engine/core/` | ECS-adjacent kernels, scenes, render/audio/save/battle/message/input systems. |
| High-level API | `engine/api/` | Engine API singleton and public runtime entry points. |
| Editor | `editor/` | ImGui panels and editor models. Release top-level panels are governed by `engine/core/editor/editor_panel_registry.*`. |
| Creator project shell | `editor/project/editor_project_session.*`; `editor/project/main_menu_panel.*`; `editor/project/new_project_wizard_*`; `editor/project/editor_dirty_state_registry.*`; `editor/project/creator_checklist*` | Owns validated open/switch/close state, user-facing startup, atomic project creation entry, recents/preferences, Save All/navigation guards, and project-local onboarding metadata. |
| Native Level Builder | `editor/spatial/level_builder_workspace.*`; `engine/core/map/grid_part_*` | Top-level shippable grid-part map editor. `GridPartDocument` is the canonical editable map document; Perspective 2D handoff is first-class. |
| Unified Map coordination | `editor/spatial/map_authoring_context.*`; `editor/spatial/map_authoring_workspace.*`; `editor/spatial/map_authoring_persistence.*` | Routes both release map IDs into one creator-facing shell while preserving explicit Grid Parts and Perspective 2D document owners. Shares selection, owner-aware history, diagnostics/readiness, persisted layout, and rollback-capable paired save. |
| Virtual and governed asset authoring | `tools/assets/asset_db.py`; `tools/assets/catalog_interchange.py`; `engine/core/assets/local_asset_catalog.*`; `engine/core/assets/archive_catalog.*`; `editor/assets/editor_thumbnail_cache.*`; `editor/assets/editor_asset_drag_payload.*`; existing promotion/attachment services | Keeps external payloads outside projects during discovery, pages metadata into the Assets workspace, stages only selected archive entries, and permits durable references only after governed promotion/attachment. |
| Atomic project creation | `engine/core/project/project_creation_service.*`; `editor/project/new_project_wizard_model.*` | Validates before write, creates through a temporary sibling, runs preflight/audit, publishes atomically, and keeps an optional external catalog root in local settings rather than project data. |
| Compat JS | `runtimes/compat_js/` | QuickJS harness plus RPG Maker MZ managers and Window/Battle/Data/Input/Audio surfaces. |
| Native/script bridge | `engine/runtimes/bridge/` | Value bridge for script interop. |
| Tests | `tests/` | Unit, integration, snapshot, compat, and engine-core lanes. |
| Tools | `tools/` | CI, docs, assets, packaging, migration, and workflow scripts. |

## Architectural Rules

- Presentation Core is the source of truth for visual interpretation; render backends consume frame intent or render commands and must not reach into game state.
- Level Builder is the primary native grid-part authoring surface. Spatial Authoring is the first-class Perspective 2D map editor surface for elevation, props, parts, ability bindings, terrain/worldbuilding previews, and direct spatial preview workflows.
- `level_builder` and `spatial_authoring` are deep links into one `MapAuthoringWorkspace`; neither route may create an independent project, selection, lifecycle, or readiness island. Their underlying document formats remain explicit and are not silently converted.
- `EditorProjectSession` is the authority for the active project. A project route is committed only after validation, and a failed open/switch must preserve the last valid session.
- External asset catalog rows are discovery metadata, not project assets. Durable editor/runtime/package references require explicit governance, promotion, and project attachment; raw external and staging paths remain ineligible.
- Creator settings such as recents, catalog root, and Map layout are user-local. Creator-checklist dismissal/completion metadata may live under project-local `.urpg`, but neither belongs in runtime or release payloads.
- Runtime/editor/compat surfaces should expose truthful status. Do not label fixture-backed or partial behavior as release-ready without tests and docs.
- Subsystems are considered landed only when they build, are test-registered, and are reachable from a runtime/editor/tool entry point.
- Migration and compat fallbacks must preserve unsupported source data and emit diagnostics rather than silently dropping behavior.
- Release exports must distinguish bootstrap/dev artifacts from production-playable artifacts.

## Deeper References

- ADRs: `docs/adr/`
- Presentation validation: `docs/presentation/VALIDATION.md`
- Release execution plan: `docs/release/AAA_RELEASE_EXECUTION_PLAN.md`
- Program status: `docs/PROGRAM_COMPLETION_STATUS.md`
- Active creator-product plan: `docs/superpowers/plans/2026-07-14-product-feature-usability-native-absorption-plan.md`
- Creator journey contract/checkpoint: `docs/product/CREATOR_JOURNEY.md`; `docs/product/CREATOR_JOURNEY_BASELINE.md`
