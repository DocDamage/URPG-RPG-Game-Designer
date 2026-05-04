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
| `release_starter_ready` | Bounded release starter coverage is promoted, attributed, checksumed, and package-gated; final art direction may remain backlog |
| `reviewed_deferred` | Category was reviewed for the 100% claim and explicitly left out of bundled release assets |

---

## Current Gap Map

| Category | Status | Coverage Notes | Target Source(s) | Priority |
|----------|--------|----------------|------------------|----------|
| UI sounds | `reviewed_deferred` | Reviewed for Phase 9 and intentionally kept as explicit silent/system fallback for the 100% claim. `BND-002` remains attribution history only; no WAV/LFS payload is release-required until an approved OGG bundle is promoted. | `SRC-003` | P0 |
| Prototype sprites | `release_starter_ready` | `BND-001` promotes the `SRC-002` actor SVG with checksum, attribution, release eligibility, and package destination for title/map/battle starter coverage. Broader character/item/grid coverage remains content backlog. | `SRC-002` | P0 |
| Release icons | `adequate` | `resources/icons/urpg_editor.png` and `resources/icons/urpg_runtime.png` are release-required repo resources validated for hydration by the release-candidate gate | repo resources | P0 |
| Runtime font fallback | `adequate` | Current release uses platform/system text rendering; no raw/vendor font pack is release-required, and this fallback is declared in the project release asset manifest | system fallback | P0 |
| Fantasy environment tilesets | `fixture_only` | Existing environment coverage remains test/reference-oriented; `SRC-001` is cataloged for curated capture and promotion work | `SRC-001` | P1 |
| Placeholder characters / monsters | `fixture_only` | Existing character coverage remains fixture/reference-oriented; `SRC-001` is cataloged for curated capture and promotion work | `SRC-001` | P1 |
| Placeholder UI frames / chrome | `release_starter_ready` | `BND-003` promotes a repo-generated UI frame/chrome SVG with attribution, checksum, release eligibility, and package destination for the bounded starter claim. | `SRC-009` | P1 |
| VFX / animation sheets | `release_starter_ready` | `BND-003` promotes a repo-generated single-frame battle VFX SVG for package coverage. Multi-frame/polished VFX identity remains backlog. | `SRC-009` | P2 |
| Environmental SFX / ambient audio | `reviewed_deferred` | Reviewed for Phase 9 and excluded from bundled release assets. Current release uses explicit muted/system fallback; future ambient audio requires OGG promotion and attribution. | Discovery backlog | P2 |
| Background music (BGM) | `reviewed_deferred` | Reviewed for Phase 9 and excluded from bundled release assets. No soundtrack is claimed for the 100% release scope. | Discovery backlog | P2 |
| Icon packs | `empty` | No direct source is captured; discovery indexes feed the active candidate-mining queue | `SRC-004`, `SRC-005` | P2 |
| Fantasy UI skin (cohesive) | `release_starter_ready` | `BND-003` promotes repo-generated starter skin metadata that binds the Phase 9 frame and VFX assets. Final unified visual identity remains future art direction. | `SRC-009` | P3 |
| Character portrait art (cohesive) | `empty` | No final-quality portrait set exists | Discovery backlog | P3 |
| Polished VFX identity | `empty` | No finalized VFX style guide or master sheet set | Discovery backlog | P3 |
| Final music identity | `empty` | No composed/curated signature soundtrack | Discovery backlog | P3 |
| High-end environment textures/materials | `empty` | Mine from `awesome-cc0` for presentation experiments | `SRC-004` | P3 |
| 3D materials / references | `empty` | Required for presentation/spatial experiments | `SRC-004` | P3 |

---

## Fast-Win Targets

## Character Appearance Import Governance

Creator-supplied character appearance import tooling is now governed for portrait, field, and battle appearance parts:

| Appearance Category | Governance Status | Release Claim Boundary |
|---------------------|-------------------|------------------------|
| Portrait parts | `governed_import_ready` | Import rows capture source metadata, normalized asset id, dimensions, attribution state, blocked reason, and promoted-library target before assignment. Final-quality cohesive portrait art remains content backlog. |
| Field sprite parts | `governed_import_ready` | RPG Maker `img/characters` sources map to `character/field` import rows and must promote into governed runtime locations before use. Raw intake paths remain quarantine-only. |
| Battle sprite parts | `governed_import_ready` | RPG Maker `img/sv_actors`, `img/sv_enemies`, and `img/enemies` sources map to `character/battle` import rows and must promote into governed runtime locations before use. Raw/source categories remain quarantine-only until reviewed and promoted. |

These are the governed promotion targets after the first TD Sprint 04 proof lanes:

1. **UI Sound Pass** — Deferred out of the current 100% release claim unless an approved OGG distribution path is promoted through a release-required bundle.
2. **Prototype Sprite Pass** — `BND-001` is package-gated for starter title/map/battle coverage; broader curated actor/item/grid expansion remains backlog.
3. **Release Starter Skin Pass** — `BND-003` is package-gated for UI frame/chrome, a bounded battle VFX proof, and cohesive starter skin metadata.
4. **Fantasy Environment Vertical Slice** — Remains future backlog: capture one curated tileset bundle and one character subset from `Intersect-Assets`, record attribution manifests, and promote them into a representative scene.

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
| 2026-05-01 | Added character appearance import governance for portrait, field, and battle parts while keeping raw/source assets quarantine-only and final-quality character art as content backlog. |
| 2026-05-02 | Phase 9 promoted `BND-003` repo-generated release starter skin assets and tightened the release asset gate for selected bundle categories, checksums, attribution records, package destinations, and release eligibility. |
| 2026-05-04 | Added explicit `releaseAssets.visualClaimScope` enforcement so prototype actor, starter UI skin, and VFX proof assets can only satisfy release coverage as bounded starter visuals, not final art direction. |
