# Presentation Core - Project Plan (PLAN.md)

## Status
**Current Phase:** Post-Phase-2 documentation and governance follow-through
**Current Task:** Task 4 — final repo-wide documentation truth-reconciliation sweep and close-out

**Completed Milestones Since Initialization**
- Phase 0 — Program Setup: Complete
- Phase 1 — Runtime Spine: Complete
- Phase 2 — Spatial Data Foundation: Complete
- Phase 3 — MapScene Integration: Complete
- Phase 4 — Battle & Audio State Integration: Complete
- Phase 5 bootstrap: dynamic lights, fog/Post-FX intent emission, and shadow proxies are now wired into the map presentation path with test coverage.

**Related Enablement**
- Curated Hugging Face fixture ingestion is now in place for TMX, Visual Novel Maker, and Godot reference corpora, with manifest-driven refresh tooling under `tools/assets/`.
- Cross-cutting debt, truthfulness, and intake-governance tracking lives in `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

**Current framing note**
- The historical presentation-core backlog below remains accurate as a planning reference.
- Active repo-wide execution has moved beyond the Phase 5 milestone and the closed Phase 2 runtime lane; current truth and remaining work are tracked canonically in `docs/PROGRAM_COMPLETION_STATUS.md` and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`.

---

## Phase 0 — Program Setup Backlog

| ID | Task | Status | Priority |
|---|---|---|---|
| 1 | Write and review all 10 ADR skeletons | Completed | High |
| 2 | Write and review all 4 scene-family contracts (Section 10) | Completed | High |
| 3 | Scaffold repo structure (empty directories + placeholder headers) | Completed | High |
| 4 | Initialize PLAN / WORKLOG / RISKS / ADR repo scaffolding | Completed | Medium |
| 5 | Initialize performance budget document with placeholder targets | Completed | Medium |

---

## Phase 1 — Runtime Spine Backlog

| ID | Task | Status | Dependency |
|---|---|---|---|
| 6 | Define `PresentationMode` enum and resolver | Completed | ADR-001, ADR-002 |
| 7 | Add `ProjectPresentationSettings` schema with version metadata | Completed | ADR-002 |
| 8 | Add `PresentationContext` core type | Completed | 6 |
| 9 | Add `PresentationAuthoringData` structure | Completed | 6, 7 |
| 10 | Add `PresentationRuntime` entry point | Completed | 8, 9 |
| 11 | Add `PresentationDiagnostics` channel with severity classification | Completed | 10 |
| 12 | Add capability tier enum and model | Completed | ADR-004 |
| 13 | Add `PresentationCapabilityMatrix` | Completed | 12 |
| 14 | Add `PresentationFallbackRouter` | Completed | 13 |
| 15 | Add `PresentationSceneTranslator` base contract | Completed | 10 |
| 16 | Add MapScene presentation adapter interface | Completed | 15 |
| 17 | Add BattleScene presentation adapter interface | Completed | 15 |
| 18 | Add MenuScene presentation adapter interface | Completed | 15 |
| 19 | Add overlay/UI composition adapter interface | Completed | 15 |
| 20 | Add camera profile schema and camera runtime skeleton | Completed | ADR-003 |
| 21 | Add arena allocator skeleton with budget tracking | Completed | ADR-010 |
| 22 | Profile arena cost on representative MapScene; commit baseline to performance budget doc | Completed | 21 |

---

## Phase 2 — Spatial Data Foundation Backlog

| ID | Task | Status | Dependency |
|---|---|---|---|
| 23 | Add `SpatialMapOverlay` schema (versioned) | Completed | ADR-003 |
| 24 | Add `ElevationGrid` schema and validator | Completed | ADR-003 |
| 25 | Add prop instance schema | Completed | ADR-003, ADR-008 |
| 26 | Add light / fog / post-FX / material response profile schemas | Completed | ADR-003 |
| 27 | Add `TierFallbackDeclaration` schema | Completed | 13 |
| 28 | Add `ActorPresentationProfile` schema | Completed | ADR-003 |
| 29 | Add schema round-trip tests for all Phase 2 schema families | Completed | 23–28 |
| 30 | Initialize schema changelog and commit first version entries | Completed | 29 |

---

## Phase 3 — MapScene Integration Backlog

| ID | Task | Status | Dependency |
|---|---|---|---|
| 31 | MapScene adapter (full implementation) | Completed | 16, 23 |
| 32 | Actor anchoring rules and depth policy | Completed | 31 |
| 33 | First spatial command emission | Completed | 31 |

---

## Phase 4 — Battle & Audio State Integration

| ID | Task | Status | Dependency |
|---|---|---|---|
| 34 | GlobalStateHub "Diff-First" implementation | Completed | WAVE2_AUDIO |
| 35 | StateDrivenAudioResolver (BGM/BGS rules) | Completed | 34 |
| 36 | BattleFormation layout logic (Staged/Linear/Surround) | Completed | 17 |
| 37 | ElevationBrush and PropPlacement editor tools | Completed | 23, 24 |
| 38 | SpatialMapOverlay JSON Serialization/Migration | Completed | 23, 30 |
| 39 | CameraProfile blending and command emission | Completed | 20 |
| 40 | SaveSerializationHub state sync integration | Completed | 34 |

---

## Phase 5 — Environment & Presentation Polish

| ID | Task | Status | Dependency |
|---|---|---|---|
| 41 | Dynamic Light placement and shadow proxies | Completed | 38, 39 |
| 42 | Fog and Post-FX profile blending | Completed | 38 |
| 43 | Screen-to-World coordinate projection for Prop Gizmos | Completed | 37 |

---

## Supporting Enablement

| ID | Task | Status | Notes |
|---|---|---|---|
| E1 | Curate and vendor permissive Hugging Face fixture samples | Completed | Vendored TMX, VNM, and Godot samples under `third_party/huggingface/`. |
| E2 | Add manifest-driven Hugging Face ingest workflow | Completed | `tools/assets/huggingface_curated_manifest.json` + `tools/assets/ingest_huggingface_curated.ps1`. |
| E3 | Integrate Hugging Face fixture roots into asset tooling | Completed | Asset catalog, hygiene, duplicate-prune rules, and unit coverage updated. |
| E4 | Evaluate local embeddings + reranker stack for repo, docs, and asset retrieval | Planned | Favor editor/pipeline retrieval over runtime inference. |
| E5 | Add asset captioning / tagging layer for imported corpora and future asset packs | Planned | Use for searchability, duplicate clustering, and asset browser enrichment. |
| E6 | Add Whisper-based speech ingest for dialogue notes and timeline prep | Planned | Tooling-side only; not authoritative runtime logic. |
| E7 | Add draft localization assist for locale bootstrap flows | Planned | Writer-assist / translation-memory support; human review required. |
| E8 | Add optional TTS preview for editor-side dialogue timing checks | Planned | Preview only; not final voice pipeline. |
| E9 | Write ADR for AI tooling boundaries | Planned | Lock deterministic-runtime boundaries, licensing, review, and offline-first policy. |

---

## Asset Reality Check

### Current Asset Reality

- The repository currently contains **reference fixtures and import/test corpora**, not a serious production-ready game asset library.
- The strongest committed asset-adjacent payloads are:
  - TMX map fixtures
  - Visual Novel Maker data fragments
  - Godot reference scripts / migration corpus
  - RPG Maker MV / XP data references, with some sources intentionally kept manifest-only due to license constraints
- Existing tooling indicates the repo is ready to **host, scan, dedupe, validate, and ingest** real assets, but that is not the same thing as already having a strong final asset base.

### What The Current Asset Set Is Good For

- Importer validation
- Scene translation experiments
- Schema and migration testing
- Dialogue/script import experiments
- Cross-engine analysis and tooling regression coverage

### What The Current Asset Set Is Not

- A clean commercial-ready art pack
- A real character portrait library
- A substantial tileset/environment kit
- A mature VFX sheet library
- A production BGM/SFX library
- A finalized UI skin/icon/frame library

### Missing Asset Categories That Matter Most

| ID | Category | Status | Why It Matters |
|---|---|---|---|
| A1 | Environment tilesets and map kits | Missing | Needed to move beyond fixture maps into real scene production. |
| A2 | Character portraits and dialogue art | Missing | Needed for narrative/UI identity and editor preview realism. |
| A3 | Character sprite sheets / overworld actors | Missing | Needed for actual content-authoring throughput. |
| A4 | Battle sprites / enemy presentation assets | Missing | Needed to validate the battle presentation path with real content. |
| A5 | UI kit (frames, icons, cursors, widgets) | Missing | Needed for polished editor and runtime presentation. |
| A6 | VFX sheets / particles / overlays | Missing | Needed to stress the presentation stack with real content instead of placeholders. |
| A7 | SFX library | Missing | Needed to validate audio state integration against real game events. |
| A8 | BGM library | Missing | Needed to test resolver quality, transitions, and presentation cohesion. |
| A9 | Material / prop texture set | Missing | Needed for environment polish, prop variety, and scene readability. |
| A10 | License-cleared commercial asset packs | Missing | Needed so the repo can graduate from reference-fixture mode to shippable-content mode. |

### Near-Term Asset Direction

1. Treat current fixture corpora as **tooling/reference inputs**, not as the answer to the actual content problem.
2. Build or ingest a **license-clean starter pack** across tiles, portraits, UI, VFX, and audio.
3. Run all new drops through the existing asset hygiene and validation toolchain immediately.
4. Expand the sprite pipeline from scaffold state into a real production import/pack workflow once real asset volume exists.
