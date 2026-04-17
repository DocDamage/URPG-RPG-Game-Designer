# URPG External Repository License Matrix

> Legal intake gate for the twelve external repositories in the URPG intake program.  
> See `URPG_repo_intake_plan.md` and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` (P3-02).

---

## Rules

- No code is copied, vendored, or mined into URPG product lanes before this matrix records a legal disposition.
- Custom/credit-based licenses default to `fixture_only` or `reference_only` until clarified.
- Code and assets are tracked separately if licenses differ within a repo.

---

## Matrix

| Repo | Owner / Org | Default License | Permissive? | Reciprocal? | Redistribution OK? | Commercial OK? | Attribution Required? | Code≠Asset Licenses? | URPG Policy | Status |
|------|-------------|-----------------|-------------|-------------|--------------------|----------------|-----------------------|----------------------|-------------|--------|
| triacontane/RPGMakerMV | triacontane | MIT (author-created plugins) | Yes | No | Yes | Yes | Yes | Possible (verify per file) | `fixture_only`; copy only clearly licensed pieces | Pending audit |
| EasyRPG/Tools | EasyRPG | GPL-like / repo license TBD | TBD | TBD | TBD | TBD | TBD | No | `reference_only`; prefer idea extraction | Pending audit |
| erri120/rpgmpacker | erri120 | MIT-friendly candidate | Yes | No | Yes | Yes | Yes | No | `production_candidate`; evaluate partial direct use | Pending audit |
| EasyRPG/liblcf | EasyRPG | MIT | Yes | No | Yes | Yes | Yes | No | `production_candidate`; wrapped integration only | Pending audit |
| davide97l/rpgmaker-mv-translator | davide97l | TBD | TBD | TBD | TBD | TBD | TBD | No | `reference_only` unless legal/offline path is acceptable | Pending audit |
| EasyRPG/RTP | EasyRPG | CC-BY | Yes | No | Yes | Yes | Yes | Yes (assets only) | `asset_reference`; separate attribution manifests required | Pending audit |
| EasyRPG/TestGame | EasyRPG | TBD | TBD | TBD | TBD | TBD | TBD | No | `fixture_only` | Pending audit |
| mjshi/RPGMakerRepo | mjshi | Credit-based custom terms | No | No | Conditional | TBD | Yes | Possible | `fixture_only` / `reference_only` until exact terms documented | Pending audit |
| biud436/MV | biud436 | TBD | TBD | TBD | TBD | TBD | TBD | No | `fixture_only` / `reference_only` until cleared | Pending audit |
| DrillUp/drill_plugins | DrillUp | TBD; per-plugin terms possible | TBD | TBD | TBD | TBD | TBD | Possible | `fixture_only` / `reference_only` until cleared | Pending audit |
| SnowSzn/rgss-script-editor | SnowSzn | GPL-family (expected) | No | Yes | Conditional | Conditional | Yes | No | `reference_only`; no code absorption | Pending audit |
| AsPJT/AsLib | AsPJT | TBD | TBD | TBD | TBD | TBD | TBD | No | `reference_only` until cleared | Pending audit |

---

## Acceptance Criteria

- [ ] Every repo has a recorded legal disposition.
- [ ] Every repo is explicitly tagged `reference_only`, `fixture_only`, `production_candidate`, or `asset_reference`.
- [ ] No code is copied into URPG before this matrix is approved.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial template created from `URPG_repo_intake_plan.md` |
