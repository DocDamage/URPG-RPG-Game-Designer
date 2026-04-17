# Presentation Core - Risk Register (RISKS.md)

Derived from `urpg_first_class_presentation_architecture_plan_v2.md` Section 26.

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

---

## Detailed Risk Tracking

### Risk 1 — Backend becomes the real source of truth again
- **Mitigation**: Contract-first `PresentationFrameIntent`; explicit review rule against semantic leakage.
- **Trigger**: Presentation logic found in renderer execution code.
- **Owner**: Presentation Architect
