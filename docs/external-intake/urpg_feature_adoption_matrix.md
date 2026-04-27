# URPG External Repository Feature Adoption Matrix

> Mapping of external repos to URPG features/compat areas.
> See [URPG_repo_intake_plan.md](../archive/planning/external-intake__URPG_repo_intake_plan.md) and [TECHNICAL_DEBT_REMEDIATION_PLAN.md](../archive/planning/TECHNICAL_DEBT_REMEDIATION_PLAN.md) (P3-02, Phase 4 / Workstream 4.1).

---

## Adoption Legend

| Symbol | Meaning |
|--------|---------|
| `A` | Adopt (through wrapper, adapter, or direct integration) |
| `R` | Reference only (mine ideas, do not ship code) |
| `F` | Fixture only (ingest as test data or compat corpus) |
| `D` | Defer |
| `-` | Not applicable |

---

## Matrix

| Repo | Compat Layer | Importer / Migration | Localization | Exporter / Packager | Asset Browser | Editor Workflow |
|------|--------------|----------------------|--------------|---------------------|---------------|-----------------|
| triacontane/RPGMakerMV | **F** | - | - | - | - | - |
| EasyRPG/Tools | - | **A** | - | - | - | **R** |
| erri120/rpgmpacker | - | - | - | **A** | - | - |
| EasyRPG/liblcf | - | **A** | - | - | - | - |
| davide97l/rpgmaker-mv-translator | - | - | **A** | - | - | - |
| EasyRPG/RTP | - | - | - | - | **F** | - |
| EasyRPG/TestGame | - | **F** | - | - | - | - |
| mjshi/RPGMakerRepo | **F** | - | - | - | - | - |
| biud436/MV | **F** | - | - | - | - | - |
| DrillUp/drill_plugins | **F** | - | - | - | - | - |
| SnowSzn/rgss-script-editor | - | - | - | - | - | **R** |
| AsPJT/AsLib | - | - | - | - | - | **R** |

---

## Detailed Adoption Cards

### triacontane/RPGMakerMV
- **Adopted idea:** Plugin fixture corpus and compat stress cases
- **Source repo:** triacontane/RPGMakerMV
- **Why it matters:** Highest-value compat corpus covering UI, battle, message, menu, data, audio, input, save, and rendering extensions
- **Implementation complexity:** Medium (metadata crawler + fixture harness)
- **Runtime risk:** Low (fixtures do not ship in production runtime)
- **License risk:** Medium (verify per-file MIT claims)
- **Integration mode:** Fixture ingestion

### EasyRPG/liblcf
- **Adopted idea:** Legacy RM2K/RM2K3 project import backend
- **Source repo:** EasyRPG/liblcf
- **Why it matters:** Strongest code-level import foundation for legacy formats
- **Implementation complexity:** Medium–High (wrapper facade + conversion layer)
- **Runtime risk:** Low–Medium (wrapped behind read-only import boundary)
- **License risk:** Low (MIT)
- **Integration mode:** Wrapped dependency behind `LegacyProjectImporter`

### erri120/rpgmpacker
- **Adopted idea:** Export packager features (unused-file exclusion, platform packaging, hardlink optimization)
- **Source repo:** erri120/rpgmpacker
- **Why it matters:** Cleanest exporter/deployer reference in the intake set
- **Implementation complexity:** Medium
- **Runtime risk:** Low (build-time/export-time tool path)
- **License risk:** Low (MIT-friendly)
- **Integration mode:** Reference + selective feature adoption / possible wrapper

### davide97l/rpgmaker-mv-translator
- **Adopted idea:** Text extraction coverage map and dialogue segmentation heuristics
- **Source repo:** davide97l/rpgmaker-mv-translator
- **Why it matters:** Accelerates localization bootstrap by mapping JSON text-bearing fields
- **Implementation complexity:** Low–Medium
- **Runtime risk:** Low (tooling path only)
- **License risk:** Medium (verify repo license and exclude online translator dependency)
- **Integration mode:** Clean-room reimplementation of extraction logic

### EasyRPG/RTP
- **Adopted idea:** Retro-compatible reference asset pack for testing and internal previews
- **Source repo:** EasyRPG/RTP
- **Why it matters:** Real assets for pipeline testing and regression visual validation
- **Implementation complexity:** Low
- **Runtime risk:** Low (separated asset/reference lane)
- **License risk:** Low (CC-BY, attribution required)
- **Integration mode:** Asset pack with attribution manifests

### SnowSzn/rgss-script-editor & AsPJT/AsLib
- **Adopted idea:** Editor UX workflow concepts (script extraction, load-order visualization, map authoring affordances)
- **Source repos:** SnowSzn/rgss-script-editor, AsPJT/AsLib
- **Why it matters:** Improves editor backlog with concrete, proven workflow ideas
- **Implementation complexity:** N/A (reference only)
- **Runtime risk:** None
- **License risk:** Low–Medium (GPL expectation for SnowSzn; read-only mitigates risk)
- **Integration mode:** Reference notes → URPG-owned backlog tickets

---

## Acceptance Criteria

- [ ] No repo remains in a vague “interesting” state without an `A`, `R`, `F`, or `D` decision.
- [ ] Every `A` row has an assigned URPG lane owner and a target ticket or design doc.
- [ ] Every `F` row has a fixture ingestion plan and manifest schema.
- [ ] Every `R` row has at least one captured reference note or backlog ticket.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial matrix and adoption cards created from `docs/external-intake/URPG_repo_intake_plan.md` |
