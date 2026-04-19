# URPG External Repository License Matrix

> Legal intake gate for the twelve external repositories in the URPG intake program.  
> See `URPG_repo_intake_plan.md` and `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md` (P3-02).

---

## Rules

- No code is copied, vendored, or mined into URPG product lanes before this matrix records a legal disposition.
- Any repo whose upstream terms are not yet re-verified in a source-capture pass stays blocked from direct adoption even if a provisional disposition is recorded here.
- Custom/credit-based or unverified terms default to `fixture_only` or `reference_only` until clarified.
- Code and assets are tracked separately if licenses differ within a repo.
- Any `production_candidate` path must enter URPG through URPG-owned wrappers and facades; direct architecture absorption is not an allowed intake outcome.
- Wrapper/facade review is part of the legal and technical disposition, not a later optional cleanup step.

---

## Matrix

| Repo | Owner / Org | Default License Record | Permissive? | Reciprocal? | Redistribution OK? | Commercial OK? | Attribution Required? | Code≠Asset Licenses? | URPG Policy | Status |
|------|-------------|-----------------|-------------|-------------|--------------------|----------------|-----------------------|----------------------|-------------|--------|
| triacontane/RPGMakerMV | triacontane | MIT noted for author-created plugins; per-file verification still required | Yes (provisional) | No (provisional) | Yes for verified MIT files only | Yes for verified MIT files only | Yes | Possible | `fixture_only`; copy only clearly licensed pieces | Disposition recorded; per-file verification still required |
| EasyRPG/Tools | EasyRPG | Upstream terms not re-verified in canonical record | Unknown | Unknown | Blocked until verified | Blocked until verified | Unknown | No | `reference_only`; prefer idea extraction | Disposition recorded; blocked from direct adoption |
| erri120/rpgmpacker | erri120 | MIT-friendly candidate recorded in intake notes | Yes (provisional) | No (provisional) | Yes if MIT confirmation holds | Yes if MIT confirmation holds | Yes | No | `production_candidate`; evaluate partial direct use behind URPG-owned wrappers | Disposition recorded; wrapper-only evaluation |
| EasyRPG/liblcf | EasyRPG | MIT recorded in intake notes | Yes (provisional) | No (provisional) | Yes if upstream record remains unchanged | Yes if upstream record remains unchanged | Yes | No | `production_candidate`; wrapped integration only | Disposition recorded; wrapper-only evaluation |
| davide97l/rpgmaker-mv-translator | davide97l | Upstream terms not re-verified in canonical record | Unknown | Unknown | Blocked until verified | Blocked until verified | Unknown | No | `reference_only`; only mine extraction ideas and offline workflow gaps | Disposition recorded; blocked from direct adoption |
| EasyRPG/RTP | EasyRPG | CC-BY recorded for asset lane; attribution manifest required | Yes (provisional) | No (provisional) | Yes with attribution | Yes with attribution | Yes | Yes (assets only) | `asset_reference`; separate attribution manifests required | Disposition recorded; attribution-gated asset reference |
| EasyRPG/TestGame | EasyRPG | Upstream terms not re-verified in canonical record | Unknown | Unknown | Blocked outside governed fixture use | Blocked outside governed fixture use | Unknown | No | `fixture_only`; quarantine as regression corpus only | Disposition recorded; fixture quarantine only |
| mjshi/RPGMakerRepo | mjshi | Credit-based custom terms recorded in intake notes | No | No | Conditional | Conditional | Yes | Possible | `fixture_only`; mine edge cases without direct product absorption | Disposition recorded; no production adoption |
| biud436/MV | biud436 | Upstream terms not re-verified in canonical record | Unknown | Unknown | Blocked until verified | Blocked until verified | Unknown | No | `fixture_only`; use only for quarantined compat coverage if cleared per file | Disposition recorded; blocked from direct adoption |
| DrillUp/drill_plugins | DrillUp | Per-plugin terms expected; canonical record remains unverified | Unknown | Unknown | Blocked until verified | Blocked until verified | Unknown | Possible | `fixture_only`; use only for quarantined compat coverage if cleared per plugin | Disposition recorded; blocked from direct adoption |
| SnowSzn/rgss-script-editor | SnowSzn | GPL-family expectation recorded in intake notes | No (provisional) | Yes (provisional) | Conditional | Conditional | Yes | No | `reference_only`; no code absorption | Disposition recorded; reference-only |
| AsPJT/AsLib | AsPJT | Upstream terms not re-verified in canonical record | Unknown | Unknown | Blocked until verified | Blocked until verified | Unknown | No | `reference_only`; extract editor/spatial ideas only | Disposition recorded; blocked from direct adoption |

---

## Acceptance Criteria

- [x] Every repo has a recorded legal disposition.
- [ ] Every repo is explicitly tagged `reference_only`, `fixture_only`, `production_candidate`, or `asset_reference`.
- [ ] No code is copied into URPG before this matrix is approved.

---

## Change Log

| Date | Change |
|------|--------|
| 2026-04-17 | Initial template created from `URPG_repo_intake_plan.md` |
| 2026-04-19 | Replaced placeholder audit rows with explicit recorded dispositions, blocking rules for unverified upstream terms, and Phase 4 closure-ready policy language. |
