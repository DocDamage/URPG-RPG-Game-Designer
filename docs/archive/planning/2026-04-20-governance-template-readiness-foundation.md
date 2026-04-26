# Governance and Template Readiness Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish the first governance foundation slice for subsystem readiness, template readiness, project audit truth, and schema/version drift control without overstating any proposed feature as already landed.

**Architecture:** Start with canonical records and rules, then wire focused CI/doc checks, then add the project-audit contract and diagnostics surface stub. Keep the first slice narrow and governance-first: define the readiness sources of truth before building richer editor/product layers on top of them.

**Tech Stack:** Markdown governance docs, JSON schema, PowerShell CI/doc checks, C++20 audit/diagnostics scaffolding, existing canonical status stack

---

### Task 1: Create the canonical readiness and label-rule docs

**Files:**
- Create: `docs/RELEASE_READINESS_MATRIX.md`
- Create: `docs/TEMPLATE_READINESS_MATRIX.md`
- Create: `docs/TRUTH_ALIGNMENT_RULES.md`
- Create: `docs/TEMPLATE_LABEL_RULES.md`
- Create: `docs/SUBSYSTEM_STATUS_RULES.md`
- Modify: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`

- [ ] Define the subsystem readiness fields, allowed states, and evidence requirements.
- [ ] Define the template readiness fields, safe-scope wording, and required-subsystem linkage.
- [ ] Define subsystem/template label-promotion rules that prevent prose-only upgrades.
- [ ] Cross-link the new docs from the canonical roadmap/status stack without claiming implementation beyond the new governance artifacts.
- [ ] Verify wording with:

```powershell
rg -n "READY|PARTIAL|EXPERIMENTAL|release-readiness|template readiness|truth" docs/RELEASE_READINESS_MATRIX.md docs/TEMPLATE_READINESS_MATRIX.md docs/TRUTH_ALIGNMENT_RULES.md docs/TEMPLATE_LABEL_RULES.md docs/SUBSYSTEM_STATUS_RULES.md docs/NATIVE_FEATURE_ABSORPTION_PLAN.md docs/PROGRAM_COMPLETION_STATUS.md
```

### Task 2: Add the machine-readable readiness schema and focused CI/doc checks

**Files:**
- Create: `content/schemas/readiness_status.schema.json`
- Create: `tools/ci/check_release_readiness.ps1`
- Create: `tools/docs/check_truth_alignment.ps1`
- Create: `tools/docs/check_template_claims.ps1`
- Create: `tools/docs/check_subsystem_badges.ps1`
- Create: `tools/ci/check_schema_changelog.ps1`
- Create: `docs/SCHEMA_CHANGELOG.md`

- [ ] Define a compact readiness schema that can represent subsystem/template status plus evidence references.
- [ ] Add focused PowerShell checks that fail when:
  - required readiness rows are missing
  - template labels exceed subsystem evidence
  - subsystem badges exceed documented evidence
  - schema changes lack changelog linkage
- [ ] Keep v1 checks conservative and truth-oriented rather than trying to solve every downstream feature gap immediately.
- [ ] Verify the scripts are at least invocation-safe with focused local calls.

### Task 3: Define the project-audit contract and diagnostics entry point

**Files:**
- Create: `tools/audit/urpg_project_audit.cpp`
- Create: `docs/PROJECT_AUDIT.md`
- Create: `editor/diagnostics/project_audit_panel.h`
- Create: `editor/diagnostics/project_audit_panel.cpp`
- Modify: `CMakeLists.txt`

- [ ] Add the initial audit contract for reporting readiness/export blockers, missing bindings, version mismatches, and overclaims.
- [ ] Keep the first implementation narrow: audit data model plus diagnostics surface scaffolding is enough if clearly labeled.
- [ ] Register any new build targets or sources truthfully; do not create header-only "landed" illusions.
- [ ] Add focused tests if the audit model is substantial enough to justify them in the first pass.

### Task 4: Add a first cross-cutting minimum-bar matrix

**Files:**
- Modify: `docs/RELEASE_READINESS_MATRIX.md`
- Modify: `docs/TEMPLATE_READINESS_MATRIX.md`
- Modify: `docs/TRUTH_ALIGNMENT_RULES.md`

- [ ] Add explicit minimum-bar columns or rule fields for:
  - accessibility
  - audio governance
  - input remapping/controller governance
  - localization completeness
  - schema versioning
  - performance budgets
- [ ] Keep these as governed bars first, not inflated implementation claims.
- [ ] Verify the canonical docs still distinguish "rule exists" from "subsystem fully delivered."

### Task 5: Reconcile the remaining canonical docs and record the pass

**Files:**
- Modify: `WORKLOG.md`
- Modify: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- Modify: `README.md` if needed for top-level status wording

- [ ] Update the worklog and canonical remediation hub to reflect the new governance-foundation slice.
- [ ] Re-run focused drift checks:

```powershell
rg -n "release-readiness matrix by subsystem|template readiness matrix|project audit|truth reconciler|schema versioning|breaking change" docs README.md WORKLOG.md
git status --short
```

- [ ] Record any intentionally deferred items for the next slice, especially Create-a-Character runtime and template-specific expansion specs.
