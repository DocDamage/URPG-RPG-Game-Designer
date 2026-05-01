# URPG Asset Category Gaps

> Current asset gaps by category. Used to drive acquisition backlog and promotion decisions.
> See [URPG_private_asset_intake_plan.md](../archive/planning/asset_intake__URPG_private_asset_intake_plan.md) and [PROGRAM_COMPLETION_STATUS.md](../archive/planning/PROGRAM_COMPLETION_STATUS.md) (P3-03, Phase 4 / Workstream 4.2).

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
| UI sounds | `fixture_only` | First governed `SRC-003` UI SFX proof is retained as a local/deferred lane; WAV payloads are ignored and are not release-required until binary hosting or a non-LFS asset is approved | `SRC-003` | P0 |
| Prototype sprites | `partial` | First governed `SRC-002` prototype actor SVG is normalized and promoted; broader character/item/grid coverage is active implementation work | `SRC-002` | P0 |
| Release icons | `adequate` | `resources/icons/urpg_editor.png` and `resources/icons/urpg_runtime.png` are release-required repo resources validated for hydration by the release-candidate gate | repo resources | P0 |
| Runtime font fallback | `adequate` | Current release uses platform/system text rendering; no raw/vendor font pack is release-required, and this fallback is declared in the project release asset manifest | system fallback | P0 |
| Fantasy environment tilesets | `fixture_only` | Existing environment coverage remains test/reference-oriented; `SRC-001` is cataloged for curated capture and promotion work | `SRC-001` | P1 |
| Placeholder characters / monsters | `fixture_only` | Existing character coverage remains fixture/reference-oriented; `SRC-001` is cataloged for curated capture and promotion work | `SRC-001` | P1 |
| Placeholder UI frames / chrome | `fixture_only` | No promoted pack exists; `SRC-001` is only a cataloged future source candidate | `SRC-001` | P1 |
| VFX / animation sheets | `fixture_only` | Pipeline realism still depends on future source capture and curated promotion from `SRC-001` | `SRC-001` | P2 |
| Environmental SFX / ambient audio | `fixture_only` | No governed promoted library exists yet; only future source candidates are cataloged | `SRC-001` | P2 |
| Background music (BGM) | `fixture_only` | No governed promoted BGM library exists yet; any future use depends on capture plus attribution review | `SRC-001` | P2 |
| Icon packs | `empty` | No direct source is captured; discovery indexes feed the active candidate-mining queue | `SRC-004`, `SRC-005` | P2 |
| Fantasy UI skin (cohesive) | `empty` | No unified visual identity; requires future art direction | Discovery backlog | P3 |
| Character portrait art (cohesive) | `empty` | No final-quality portrait set exists | Discovery backlog | P3 |
| Polished VFX identity | `empty` | No finalized VFX style guide or master sheet set | Discovery backlog | P3 |
| Final music identity | `empty` | No composed/curated signature soundtrack | Discovery backlog | P3 |
| High-end environment textures/materials | `empty` | Mine from `awesome-cc0` for presentation experiments | `SRC-004` | P3 |
| 3D materials / references | `empty` | Required for presentation/spatial experiments | `SRC-004` | P3 |

---

## Fast-Win Targets

These are the governed promotion targets after the first TD Sprint 04 proof lanes:

1. **UI Sound Pass** — Restore an approved audio distribution path, then expand the `kenney-interface-sounds` proof beyond `kenney_click_001.wav` into a small confirm/cancel/open/close set and wire those cues into editor/runtime audio events.
2. **Prototype Sprite Pass** — Expand the first `GDQuest/game-sprites` proof beyond `gdquest_blue_actor.svg` into a curated actor/item/grid subset, then wire it into map/battle test scenes.
3. **Fantasy Environment Vertical Slice** — Capture one curated tileset bundle and one character subset from `Intersect-Assets`, record attribution manifests, and promote them into a representative scene.

---

## Discovery Backlog Targets

Categories to mine from `awesome-cc0` and `Game-Assets-And-Resources`:

- Environment textures/materials
- Icon packs
- Fantasy UI frames
- Battle VFX sheets
- Ambient/background audio
- 3D materials and references for presentation experiments

---

## Notes

- This intake is about **coverage and acceleration**, not final visual canon.
- Even after the current direct-ingest plan executes, cohesive final-quality art and audio identities will remain gaps.
- Use this document to justify active acquisition implementation sprints and to avoid over-promising content completeness.
- Release-required surfaces are declared under `releaseAssets` in `content/fixtures/project_governance_fixture.json` and enforced by `tools/ci/check_release_required_assets.ps1`.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial gap map created from `docs/asset_intake/URPG_private_asset_intake_plan.md` |
| 2026-04-19 | Replaced misleading staged-state wording with cataloged-not-mirrored reality and rewrote fast-win targets as governed capture/promote implementation work. |
| 2026-04-23 | TD Sprint 04 moved UI sounds and prototype sprites from fixture-only to partial with one promoted proof asset in each lane. |
| 2026-04-27 | Added explicit release-required asset manifest coverage for title, map, battle, UI, audio, icons, and font fallback surfaces. |
| 2026-04-27 | Deferred tracked WAV payloads from release scope; UI/audio release surfaces now use fallback policy entries until an approved bundled audio asset exists. |
