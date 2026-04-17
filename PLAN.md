# Presentation Core - Project Plan (PLAN.md)

## Status
**Current Phase:** Phase 0 — Program Setup
**Current Task:** 1. Write and review all 10 ADR skeletons

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

## Phase 5 — Environment & Presentation Polish (Next)

| ID | Task | Status | Dependency |
|---|---|---|---|
| 41 | Dynamic Light placement and shadow proxies | Not-Started | 38, 39 |
| 42 | Fog and Post-FX profile blending | Not-Started | 38 |
| 43 | Screen-to-World coordinate projection for Prop Gizmos | Not-Started | 37 |
