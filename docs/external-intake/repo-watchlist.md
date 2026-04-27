# URPG External Repository Watchlist

> Governed intake program for external repositories.
> See [URPG_repo_intake_plan.md](../archive/planning/external-intake__URPG_repo_intake_plan.md) and [TECHNICAL_DEBT_REMEDIATION_PLAN.md](../archive/planning/TECHNICAL_DEBT_REMEDIATION_PLAN.md) (P3-02, Phase 4 / Workstream 4.1) for program context.

---

## Status Legend

| Disposition | Meaning |
|-------------|---------|
| `reference_only` | Read/mine for ideas; no code or fixture ingestion |
| `fixture_only` | Ingested as test data, sample projects, or compat corpus only |
| `production_candidate` | Eligible for wrapped integration behind URPG-owned facades |
| `asset_reference` | Usable as attributed asset/reference pack in separated lanes |

---

## Tier A — High Value / Directly Actionable

| # | Repo | Source URL | Intended Use | Current Disposition | Owner | Next Action |
|---|------|------------|--------------|---------------------|-------|-------------|
| 1 | triacontane/RPGMakerMV | `https://github.com/triacontane/RPGMakerMV` | Compat fixture corpus; plugin behavior discovery | `fixture_only` | Compat/runtime | Index top 50 plugins; build metadata crawler |
| 2 | EasyRPG/Tools | `https://github.com/EasyRPG/Tools` | Import/conversion ideas; preview/inspection tooling | `reference_only` | Import/migration | Catalog tools and map 3 workflows to URPG features |
| 3 | erri120/rpgmpacker | `https://github.com/erri120/rpgmpacker` | Export/build automation reference; deployment validation | `production_candidate` | Export/CI | Audit CLI UX and unused-file exclusion logic |
| 4 | EasyRPG/liblcf | `https://github.com/EasyRPG/liblcf` | Legacy format parsing; data import foundation | `production_candidate` | Import/migration | Spike wrapped parser behind `LegacyProjectImporter` facade |

## Tier B — Important but Narrower / Conditional

| # | Repo | Source URL | Intended Use | Current Disposition | Owner | Next Action |
|---|------|------------|--------------|---------------------|-------|-------------|
| 5 | davide97l/rpgmaker-mv-translator | `https://github.com/davide97l/rpgmaker-mv-translator` | Text extraction / localization bootstrap | `reference_only` | Localization | Mine JSON coverage map; exclude online translator dependency |
| 6 | EasyRPG/RTP | `https://github.com/EasyRPG/RTP` | Retro-compatible reference assets; pipeline test material | `asset_reference` | Content/pipeline | Inventory categories; define attribution manifest |
| 7 | EasyRPG/TestGame | `https://github.com/EasyRPG/TestGame` | Regression corpus; import/behavior validation | `fixture_only` | Import/migration | Ingest as legacy fixture project; plug into regression suite |
| 8 | mjshi/RPGMakerRepo | `https://github.com/mjshi/RPGMakerRepo` | Compat fixture corpus; plugin behavior discovery | `fixture_only` | Compat/runtime | Sample and tag categories; select high-divergence fixtures |
| 9 | biud436/MV | `https://github.com/biud436/MV` | Compat fixture corpus; UI/battle/plugin stress cases | `fixture_only` | Compat/runtime | Sample and tag categories; select high-divergence fixtures |
| 10 | DrillUp/drill_plugins | `https://github.com/DrillUp/drill_plugins` | Compat fixture corpus; large plugin edge-case coverage | `fixture_only` | Compat/runtime | Sample and tag categories; select high-divergence fixtures |

## Tier C — Reference / Inspiration / Quarantined Use

| # | Repo | Source URL | Intended Use | Current Disposition | Owner | Next Action |
|---|------|------------|--------------|---------------------|-------|-------------|
| 11 | SnowSzn/rgss-script-editor | `https://github.com/SnowSzn/rgss-script-editor` | Editor workflow reference; script extraction UX ideas | `reference_only` | Editor | Document 3+ UX ideas as backlog tickets; no code reuse |
| 12 | AsPJT/AsLib | `https://github.com/AsPJT/AsLib` | Map-authoring/editor UX reference; procedural terrain inspiration | `reference_only` | Editor/spatial | Document 2+ concrete editor backlog items; no code reuse |

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial watchlist created from `docs/external-intake/URPG_repo_intake_plan.md` |
