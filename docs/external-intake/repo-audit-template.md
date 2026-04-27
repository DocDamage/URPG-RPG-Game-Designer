# URPG External Repository Audit Template

> Checklist for auditing an external repository before adoption into URPG.
> See [URPG_repo_intake_plan.md](../archive/planning/external-intake__URPG_repo_intake_plan.md) and [TECHNICAL_DEBT_REMEDIATION_PLAN.md](../archive/planning/TECHNICAL_DEBT_REMEDIATION_PLAN.md) (P3-02).

---

## Metadata

| Field | Value |
|-------|-------|
| Repo name | |
| Source URL | |
| Snapshot commit / tag | |
| Audit date | |
| Auditor | |
| Proposed disposition | `reference_only` / `fixture_only` / `production_candidate` / `asset_reference` |

---

## 1. Legal and License Review

- [ ] Default license identified and linked
- [ ] Code license confirmed (if different from default)
- [ ] Asset license confirmed (if different from code)
- [ ] Redistribution allowed for intended use case
- [ ] Commercial use allowed for intended use case
- [ ] Attribution requirements recorded
- [ ] Patent / trademark risks noted (if any)
- [ ] Legal disposition recorded in `license-matrix.md`

---

## 2. Quality and Maintenance Review

- [ ] Last commit date recorded
- [ ] Maintenance health assessed (active / stale / archived)
- [ ] Open issue trend noted
- [ ] Test coverage assessed (if visible)
- [ ] Build system complexity noted
- [ ] Code quality and style assessed
- [ ] Security surface reviewed (parsers, network calls, file I/O)

---

## 3. Determinism Review

- [ ] No non-deterministic runtime behavior that would affect URPG
- [ ] No global state or singleton assumptions that leak across boundaries
- [ ] No threading behavior that conflicts with URPG contracts
- [ ] RNG usage is explicit and controllable (if any)

---

## 4. Dependency Review

- [ ] Runtime dependencies listed
- [ ] Build dependencies listed
- [ ] Dependency licenses recorded
- [ ] No heavy or unmaintained transitive dependencies flagged
- [ ] No platform-locked dependencies that conflict with URPG targets

---

## 5. Portability Review

- [ ] Target platforms assessed (desktop / web / console / mobile)
- [ ] Build system portability noted
- [ ] URPG-relevant platforms explicitly checked

---

## 6. Integration Stance

- [ ] Reference-only mining path defined (if `reference_only`)
- [ ] Fixture ingestion plan defined (if `fixture_only`)
- [ ] Wrapped integration design sketched (if `production_candidate`)
- [ ] Adapter/facade boundary identified (if `production_candidate`)
- [ ] Clean-room reimplementation path considered as alternative
- [ ] Architecture contamination risks noted and mitigated

---

## 7. Asset / Content Review (if applicable)

- [ ] Asset categories inventoried
- [ ] Attribution manifest template prepared
- [ ] Asset hygiene report run (`tools/assets/asset_hygiene.py`)
- [ ] Deduplication against existing URPG assets checked
- [ ] Visual/audio identity collision with first-party assets noted

---

## 8. Decision and Next Steps

| Decision | Select one |
|----------|------------|
| Adopt | |
| Defer | |
| Ignore / Reject | |

### Rationale

*(Why this decision was made)*

### Next Action

*(Concrete next task and owner)*

### URPG Feature Mapping

*(If adopted, which URPG lane(s) will consume this repo?)*

---

## Sign-off

- [ ] Tech lead / release owner sign-off
- [ ] Content / pipeline lead sign-off (if asset or fixture related)
- [ ] Affected subsystem owner sign-off (if production candidate)
