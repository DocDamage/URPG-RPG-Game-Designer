# Presentation Core - Risk Register (RISKS.md)

Derived from `docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md` Section 26.

## Risks Overview

| Risk ID | Title | Severity | Likelihood | Status |
|---|---|---|---|---|
| 1 | Backend becomes the real source of truth again | Critical | High | Active |
| 2 | Maps get all the love and everything else rots | High | High | Active |
| 3 | Editor support gets deferred indefinitely | High | High | Active |
| 4 | Migration becomes fake-complete | High | Medium | Active |
| 5 | Performance collapses after content scales up | High | Medium | Active |
| 6 | Asset requirements are unclear | Medium | High | Active |
| 7 | Tasks stay too large and churn | Medium | Medium | Active |
| 8 | Spatial mode functionally unsupported | Critical | Medium | Active |
| 9 | Lack of domain knowledge | High | Medium | Active |
| 10 | Platform GPU behavior divergence | High | Low | Active |
| 11 | Shader complexity maintenance | Medium | Medium | Active |
| 12 | Hot-reload introduces frame corruption | High | Low | Active |
| 13 | Reference corpora get mistaken for production-ready assets | High | High | Active |
| 14 | AI tooling boundaries blur into deterministic runtime or policy scope | High | Medium | Active |

---

## Detailed Risk Tracking

### Risk 1 — Backend becomes the real source of truth again
- **Mitigation**: Contract-first `PresentationFrameIntent`; explicit review rule against semantic leakage.
- **Trigger**: Presentation logic found in renderer execution code.
- **Owner**: Presentation Architect

### Risk 6 — Asset requirements are unclear
- **Mitigation**: Maintain an explicit asset-gap inventory in `PLAN.md` and keep reference corpora separated from production asset claims.
- **Trigger**: Fixture libraries are cited as if they close real content-production needs.
- **Owner**: Content / Pipeline Lead

### Risk 13 — Reference corpora get mistaken for production-ready assets
- **Mitigation**: Keep curated corpora tagged as tooling/reference inputs only; require license-cleared starter packs before claiming asset-readiness.
- **Trigger**: Planning or release docs start treating TMX/VNM/Godot/RPG Maker corpora as the actual art/audio solution.
- **Owner**: Content / Pipeline Lead

### Risk 14 — AI tooling boundaries blur into deterministic runtime or policy scope
- **Mitigation**: Write and enforce an ADR covering editor-only AI boundaries, offline-first expectations, review rules, and licensing constraints.
- **Trigger**: AI assist features begin affecting authoritative runtime logic, export paths, or unclear-license content flows.
- **Owner**: Presentation Architect / Tools Lead
