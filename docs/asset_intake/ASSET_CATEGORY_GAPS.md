# URPG Asset Category Gaps

> Current asset gaps by category. Used to drive acquisition backlog and promotion decisions.  
> See `URPG_private_asset_intake_plan.md` and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` (P3-03, Phase 4 / Workstream 4.2).

---

## Gap Legend

| Status | Meaning |
|--------|---------|
| `empty` | No usable assets in this category |
| `fixture_only` | Only synthetic/test fixtures exist |
| `placeholder` | Some temporary/prototype assets exist |
| `partial` | A subset exists but coverage is incomplete |
| `adequate` | Enough assets to support current product lanes |

---

## Current Gap Map

| Category | Status | Coverage Notes | Target Source(s) | Priority |
|----------|--------|----------------|------------------|----------|
| UI sounds | `placeholder` | `kenney-interface-sounds` staged; needs promotion and audio-layer wiring | `SRC-003` | P0 |
| Prototype sprites | `placeholder` | `GDQuest/game-sprites` staged; needs normalization and atlas testing | `SRC-002` | P0 |
| Fantasy environment tilesets | `fixture_only` | `Intersect-Assets` staged; needs curated subset promotion | `SRC-001` | P1 |
| Placeholder characters / monsters | `fixture_only` | `Intersect-Assets` staged; needs curated subset promotion | `SRC-001` | P1 |
| Placeholder UI frames / chrome | `fixture_only` | `Intersect-Assets` staged; promote only if visual fit is acceptable | `SRC-001` | P1 |
| VFX / animation sheets | `fixture_only` | `Intersect-Assets` staged; needs pipeline validation | `SRC-001` | P2 |
| Environmental SFX / ambient audio | `fixture_only` | `Intersect-Assets` staged; needs curation | `SRC-001` | P2 |
| Background music (BGM) | `fixture_only` | `Intersect-Assets` staged; evaluate for vertical-slice use | `SRC-001` | P2 |
| Icon packs | `empty` | Mine from `awesome-cc0` and discovery indexes | `SRC-004`, `SRC-005` | P2 |
| Fantasy UI skin (cohesive) | `empty` | No unified visual identity; requires future art direction | Discovery backlog | P3 |
| Character portrait art (cohesive) | `empty` | No final-quality portrait set exists | Discovery backlog | P3 |
| Polished VFX identity | `empty` | No finalized VFX style guide or master sheet set | Discovery backlog | P3 |
| Final music identity | `empty` | No composed/curated signature soundtrack | Discovery backlog | P3 |
| High-end environment textures/materials | `empty` | Mine from `awesome-cc0` for future presentation experiments | `SRC-004` | P3 |
| 3D materials / references | `empty` | For future presentation/spatial experiments | `SRC-004` | P3 |

---

## Fast-Win Targets

These are the immediate promotion targets to move URPG beyond fixture-only content:

1. **UI Sound Pass** — Promote `kenney-interface-sounds` → `imports/normalized/ui_sfx/` and wire into editor/runtime audio events.
2. **Prototype Sprite Pass** — Promote `GDQuest/game-sprites` → `imports/normalized/prototype_sprites/` and wire into map/battle test scenes.
3. **Fantasy Environment Vertical Slice** — Promote one curated tileset bundle and one character subset from `Intersect-Assets` into a representative scene.

---

## Discovery Backlog Targets

Categories to mine from `awesome-cc0` and `Game-Assets-And-Resources`:

- Environment textures/materials
- Icon packs
- Fantasy UI frames
- Battle VFX sheets
- Ambient/background audio
- 3D materials and references for future presentation experiments

---

## Notes

- This intake is about **coverage and acceleration**, not final visual canon.
- Even after the current direct-ingest plan executes, cohesive final-quality art and audio identities will remain gaps.
- Use this document to justify future acquisition sprints and to avoid over-promising content completeness.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial gap map created from `URPG_private_asset_intake_plan.md` |
