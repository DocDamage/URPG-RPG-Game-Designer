# Presentation Core - Performance Budgets

## Target Platform Definitions
- **Minimum Spec**: Intel UHD 620 / Mobile Baseline (Tier 0/1)
- **Target Spec**: GTX 1650 / Modern Console (Tier 2/3)

## Global Budgets

| Metric | Tier 0 (Legacy) | Tier 1 (Baseline) | Tier 2 (Standard) | Tier 3 (Ultra) |
|---|---|---|---|---|
| Frame Time (ms) | 16.6 (60fps target) | 16.6 | 16.6 | 16.6 |
| Arena Size (MB) | 1.0 | 2.0 | 4.0 | 8.0 |
| Draw Calls (Max) | 150 | 400 | 1200 | 4000 |
| VRAM Budget (MB) | 128 | 512 | 2048 | 4096 |
| Max Lights | 0 | 4 | 16 | 64 |
| Max Particles | 100 | 500 | 2000 | 10000 |

## Measured Baselines (2026-04-16)

### Phase 1 Spine Profiling
- **Representative MapScene (Tier 1)**:
  - Allocation Content: 1000 tiles, 100 sprites.
  - Measured Arena Use: 76,800 bytes (~0.07 MB).
  - Overhead: Minimal (Zero heap in hot path).
  - Status: **PASS** (Within 2.0 MB budget).

## Scene Family Budgets (Validated 2026-04-16)

### MapScene
- **Max Actors**: 1024 (Tier 0: 64)
- **Max Props**: 4096 (Tier 0: 256)
- **Streaming Latency**: < 8.0ms (Tier 0: 32.0ms synchronous)
- **LOD Factor**: Linear steps (0.25x density per Tier drop)

### MapScene Retained Render Contract
- Tile render commands are retained between frames and only rebuilt when tile or passability data changes.
- Unchanged `MapScene` frames must reuse the existing retained tile command objects instead of rebuilding equivalent commands.
- Focused evidence:
  - `tests/unit/test_scene_manager.cpp` proves unchanged frames keep pointer-stable retained tile commands.
  - `engine/core/presentation/release_validation.cpp` reports the current environment-command envelope alongside actor command counts for the Phase 5 presentation lane.

### BattleScene
- **Max Emitters**: 64 total (Tier 0: 4)
- **Post-FX Override**: Linear Desaturate (0.5ms fixed)
- **Readability Buffer**: 2.0ms (fixed)
- **Dynamic Lights**: Tier-filtered (See Global Budgets)

### MenuScene
- **Max Background Layers**: 8 (Tier 0: 2)
- **Total GPU Cost**: < 1.0ms (Baseline)
- **Overlay Composition**: 3D Render-to-Texture (Tier 2+) or Flat Alpha (Tier <2)

### DialogueScene
- **Background De-emphasis**: Tier 0: Hidden | Tier 1: Desaturate | Tier 2+: Gaussian Blur (3x3 - 11x11)
- **Character Portraits**: 2 active (Standard)
- **Response Count Limit**: 12 active intents per window
