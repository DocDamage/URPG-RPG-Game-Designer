# Presentation Runtime — Release Closure Sign-off

> **Status:** `PARTIAL`
> **Purpose:** Evidence-gathering artifact for Presentation Runtime release closure review.
> **Date:** 2026-04-23
> **Rule:** This document does **not** promote `readiness_status.json` status. Human review is required for promotion to `READY`. Passing tests or gate scripts never imply approval.

---

## 1. Runtime Ownership

The Presentation Core provides deterministic command-stream building, profile blending, and hot-reload lifecycle management across Map, Battle, Menu, Status-HUD, and Dialogue scene types.

| Component | Responsibility | Evidence |
|-----------|---------------|----------|
| `PresentationRuntime` | Builds `PresentationFrameIntent` from `PresentationContext` + `PresentationAuthoringData`; owns fog/PostFX blend resolution and actor world-position anchoring | `engine/core/presentation/presentation_runtime.h/.cpp` |
| `PresentationBridge` | Integrates `SceneManager` with the runtime; dispatches per-scene-type translation | `engine/core/presentation/presentation_bridge.h/.cpp` |
| `ProfileArenaHotReloader` | Concrete hot-reload lifecycle: queues change paths, resets arena between cycles, clears registry, invokes callback, exposes reload count for cache invalidation | `engine/core/presentation/presentation_hotreload.h` |
| `MapSceneTranslatorImpl` | Translates map actor anchoring, elevation grids, fog, PostFX, lights, and props into commands | `engine/core/presentation/map_scene_translator.h` |
| `BattleSceneTranslatorImpl` | Translates battle participants, anchors, formation layout, and effect cues | `engine/core/presentation/battle_scene_translator.h` |
| `MenuSceneTranslator` | Emits background proxy and blur PostFX for menu scenes | `engine/core/presentation/menu_scene_translator.h` |
| `StatusHUDTranslator` | Emits turn-indicator, status-icon, and minimap overlay commands | `engine/core/presentation/menu_scene_translator.h` |
| `DialogueTranslator` | Emits desaturation PostFX and contrast background for active dialogue | `engine/core/presentation/dialogue_translator.h` |

---

## 2. Schema and Migration Evidence

| Artifact | Description |
|----------|-------------|
| `content/schemas/presentation_schema.json` | JSON Schema v7 for `SpatialMapOverlay` and all embedded profile types (elevation grid, fog, PostFX, props, lights) |
| `content/fixtures/presentation_migration_fixture.json` | Canonical migration proof fixture: 8×8 map with elevation, fog, PostFX, 2 props, 2 lights |
| `engine/core/presentation/presentation_migrate.h` | `PresentationMigrationTool::MigrateMap` + `ToJson` covering legacy-tile → SpatialMapOverlay upgrade |

---

## 3. Hot-Reload Lifecycle (S23-T02)

`ProfileArenaHotReloader` provides:
- **Arena reset on every reload**: the bump allocator offset resets to 0 between cycles; no resource leak.
- **Registry cleared before callback**: stale profile data cannot survive across reload boundaries.
- **Path deduplication**: multiple `OnAssetChanged` calls for the same path in a single tick collapse to one callback invocation.
- **Reload count**: downstream caches can invalidate on `GetReloadCount()` change.

---

## 4. Scene Render-Command Coverage (S23-T04)

All five claimed scene render-command categories now have test coverage:

| Scene Type | Translator | Test Location |
|------------|-----------|--------------|
| Map | `MapSceneTranslatorImpl` | `test_spatial_editor.cpp` |
| Battle | `BattleSceneTranslatorImpl` | `test_presentation_runtime.cpp`, `test_spatial_editor.cpp` |
| Menu / UI | `MenuSceneTranslator` | `test_spatial_editor.cpp` |
| Status / HUD | `StatusHUDTranslator` | `test_spatial_editor.cpp` |
| Chat / Dialogue | `DialogueTranslator` | `test_spatial_editor.cpp`, `test_presentation_runtime.cpp` |

---

## 5. Command-Stream Replay Determinism (S23-T06)

Tests in `test_presentation_runtime.cpp` verify:
- Building the same frame twice from the same input yields identical command count, command types, actor IDs, and positions.
- After a "hot-reload" profile update, the command stream reflects the new profile data (fog density difference asserted).
- `ProfileArenaHotReloader` reload count advances deterministically per unique change event.

---

## 6. Diagnostics Integration

- `PresentationRuntime` exposes `EmitDiagnostic`, `GetDiagnostics`, and `ClearDiagnostics` for structured warning/error surfacing.
- `release_validation.cpp` (presentation release validation executable) exercises end-to-end intent building.
- `docs/presentation/VALIDATION.md` documents the gate commands.

---

## 7. Residual Gaps (Intentional — Not Blocking Current Evidence)

| Gap | Owner | Notes |
|-----|-------|-------|
| Cross-backend renderer breadth | Adjacent `visual_regression_harness` lane | The current claimed production path now has bounded real renderer-backed proof, but broader backend parity still belongs to the adjacent visual-regression/readiness lane rather than this signoff artifact |
| Human review and promotion decision | Maintainer / release owner | Promotion from `PARTIAL` to `READY` remains a deliberate human decision even with the current green evidence stack |

These gaps are explicit and scoped. They do not invalidate the evidence above.

---

## 8. Promotion Condition

Promotion from `PARTIAL` to `READY` requires:
1. A human reviewer to sign off on the residual gaps above.
2. Any remaining cross-backend renderer residual must be explicitly accepted as bounded scope rather than implied parity.
3. The reviewer must make a deliberate status edit in `readiness_status.json`; passing gates alone do not promote the subsystem.

**No automated gate completes promotion. A human reviewer must update `readiness_status.json`.**

---

## Verification Commands

```powershell
# Build and run presentation-scoped tests
powershell -ExecutionPolicy Bypass -File tools/ci/run_presentation_gate.ps1

# Check release-readiness alignment
powershell -ExecutionPolicy Bypass -File tools/ci/check_release_readiness.ps1

# Run truth reconciler
powershell -ExecutionPolicy Bypass -File tools/ci/truth_reconciler.ps1
```
