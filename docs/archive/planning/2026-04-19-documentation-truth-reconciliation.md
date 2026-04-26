# Documentation Truth Reconciliation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reconcile every execution-relevant document in the repo so current-state, roadmap, remediation, and archive/ADR materials no longer misstate present truth.

**Architecture:** Work in bounded document classes. First lock the canonical docs that define current truth, then reconcile active operational docs, then correct misleading archive/ADR material in place, and finish with a repo-wide contradiction sweep plus verification. Preserve historical intent, but rewrite stale present-tense status language anywhere it could misdirect current execution.

**Tech Stack:** Markdown documentation, PowerShell, `rg`, git, focused text verification commands.

---

## File Structure

### Canonical current-state docs

- `README.md`
  Public repo overview and top-level phase/status framing.
- `PLAN.md`
  Active phase/task tracker used to orient current execution.
- `WORKLOG.md`
  Narrative record of recent work; must align with canonical program status.
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
  Canonical remediation hub and cross-cutting truthfulness source.
- `docs/PROGRAM_COMPLETION_STATUS.md`
  Canonical latest-status snapshot.
- `docs/COMPAT_EXIT_CHECKLIST.md`
  Exit-criteria tracker for compat hardening and truthful residual partials.
- `docs/DEVELOPMENT_KICKOFF.md`
  Current kickoff/status framing that still influences execution assumptions.

### Operational specs and plans

- `docs/superpowers/specs/2026-04-19-phase-2-runtime-closure-design.md`
  Historical design doc that must read as completed work, not open work.
- `docs/superpowers/plans/2026-04-19-phase-2-runtime-closure.md`
  Historical implementation plan that must be clearly read as completed and non-authoritative for current execution.
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
  Canonical product roadmap; terminology and authority wording must stay aligned.
- `docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md`
  Active subsystem plan whose wording must match the current remediation/program narrative.
- `docs/presentation/README.md`
  Active presentation-lane framing that engineers may use during current work.
- `docs/presentation/test_matrix/README.md`
  Current presentation verification expectations.

### ADR and archive material

- `docs/archive/README.md`
  Entry point that explains how archive docs should be interpreted.
- `docs/adr/ADR-010.md`
  Contains stale placeholder wording that can mislead current execution.
- `docs/adr/ADR-010-presentation-completion.md`
  Completion framing must match current incubating/canonical truth.
- `docs/adr/ADR-011-presentation-spatial-status.md`
  Spatial/presentation status framing must match current truth.
- `docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`
  Large planning input that must not present itself as active execution authority.
- `docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md`
  Historical roadmap input that must be framed as reference rather than active truth.
- `docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md`
  Historical support plan whose open-work language must not override canonical execution docs.
- `docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md`
  Older presentation plan with stale TBDs and present-tense execution wording.

### Supporting governance/reference docs to inspect during edits

- `docs/external-intake/*.md`
- `docs/asset_intake/*.md`
- `docs/BATTLE_CORE_NATIVE_SPEC.md`
- `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
- `docs/presentation/schema_changelog.md`

These may not all need edits, but they must be checked if canonical wording changes ripple into them.

---

### Task 1: Build the Misleading-Docs Inventory and Reconcile Canonical Docs

**Files:**
- Modify: `README.md`
- Modify: `PLAN.md`
- Modify: `WORKLOG.md`
- Modify: `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/COMPAT_EXIT_CHECKLIST.md`
- Modify: `docs/DEVELOPMENT_KICKOFF.md`

- [ ] **Step 1: Capture the current misleading-doc inventory**

Run:

```powershell
rg -n "\[TBD|TODO|FIXME|Status Date|Document status|In Progress|Partially Remediated|still needed|not implemented|not started|remaining work|open work|Phase [0-9]|phase [0-9]" README.md PLAN.md WORKLOG.md docs
```

Expected: a concrete list of candidate docs and stale status phrases to reconcile against the current codebase/program state.

- [ ] **Step 2: Re-read the canonical docs as the current-truth baseline**

Run:

```powershell
Get-Content 'README.md'
Get-Content 'PLAN.md'
Get-Content 'WORKLOG.md'
Get-Content 'docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md'
Get-Content 'docs/PROGRAM_COMPLETION_STATUS.md'
Get-Content 'docs/COMPAT_EXIT_CHECKLIST.md'
Get-Content 'docs/DEVELOPMENT_KICKOFF.md'
```

Expected: confirm which claims are stale, which remain accurate, and which terms must be normalized across the canonical set.

- [ ] **Step 3: Edit canonical docs to current truth**

Apply the following content rules while editing:

```text
- README.md must no longer describe Phase 2 as in progress if the canonical docs now treat Phase 2 closure as complete.
- PLAN.md must describe the actual current phase/task and remove stale “next” framing that was already completed.
- WORKLOG.md must record this documentation reconciliation pass and keep prior closure entries chronological and accurate.
- docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md must update stale status dates/revision labels and any finding/phase statuses that no longer match the current branch truth.
- docs/PROGRAM_COMPLETION_STATUS.md must keep canonical planning authority, current phase framing, and latest validation snapshots aligned with the remediation hub and README.
- docs/COMPAT_EXIT_CHECKLIST.md and docs/DEVELOPMENT_KICKOFF.md must stop overstating remaining compat/runtime debt where closure already landed, while preserving honestly partial residuals.
```

- [ ] **Step 4: Run a focused canonical-doc contradiction check**

Run:

```powershell
rg -n "Phase 2|In Progress|Partially Remediated|Document status|Status Date|remaining work|open work" README.md PLAN.md WORKLOG.md docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md docs/PROGRAM_COMPLETION_STATUS.md docs/COMPAT_EXIT_CHECKLIST.md docs/DEVELOPMENT_KICKOFF.md
```

Expected: remaining hits are either intentionally current or clearly historical/contextual rather than contradictory.

- [ ] **Step 5: Commit the canonical-doc pass**

```bash
git add README.md PLAN.md WORKLOG.md docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md docs/PROGRAM_COMPLETION_STATUS.md docs/COMPAT_EXIT_CHECKLIST.md docs/DEVELOPMENT_KICKOFF.md
git commit -m "docs: reconcile canonical status and remediation truth"
```

### Task 2: Reconcile Active Operational Docs and Current Working Plans

**Files:**
- Modify: `docs/superpowers/specs/2026-04-19-phase-2-runtime-closure-design.md`
- Modify: `docs/superpowers/plans/2026-04-19-phase-2-runtime-closure.md`
- Modify: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- Modify: `docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md`
- Modify: `docs/presentation/README.md`
- Modify: `docs/presentation/test_matrix/README.md`

- [ ] **Step 1: Read the active operational docs that engineers may still use**

Run:

```powershell
Get-Content 'docs/superpowers/specs/2026-04-19-phase-2-runtime-closure-design.md'
Get-Content 'docs/superpowers/plans/2026-04-19-phase-2-runtime-closure.md'
Get-Content 'docs/NATIVE_FEATURE_ABSORPTION_PLAN.md'
Get-Content 'docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md'
Get-Content 'docs/presentation/README.md'
Get-Content 'docs/presentation/test_matrix/README.md'
```

Expected: identify any open-work or authority wording that could conflict with the updated canonical docs.

- [ ] **Step 2: Edit operational docs so they no longer imply obsolete current work**

Apply the following content rules while editing:

```text
- Historical Phase 2 spec/plan docs must read as completed reference material, not open execution authority.
- docs/NATIVE_FEATURE_ABSORPTION_PLAN.md must remain the canonical product roadmap without contradicting the current program/remediation phase vocabulary.
- docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md must reflect that Phase 2 audio closure landed while any further work belongs to later roadmap/remediation lanes.
- docs/presentation/README.md and docs/presentation/test_matrix/README.md must preserve incubating status where appropriate without overstating build graph/product readiness.
```

- [ ] **Step 3: Run the operational-doc wording check**

Run:

```powershell
rg -n "current task|next|In Progress|open work|remaining work|Phase 2|authoritative|canonical|not implemented|TBD" docs/superpowers/specs/2026-04-19-phase-2-runtime-closure-design.md docs/superpowers/plans/2026-04-19-phase-2-runtime-closure.md docs/NATIVE_FEATURE_ABSORPTION_PLAN.md docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md docs/presentation/README.md docs/presentation/test_matrix/README.md
```

Expected: wording is either accurate current truth or explicitly historical/reference-only.

- [ ] **Step 4: Commit the operational-doc pass**

```bash
git add docs/superpowers/specs/2026-04-19-phase-2-runtime-closure-design.md docs/superpowers/plans/2026-04-19-phase-2-runtime-closure.md docs/NATIVE_FEATURE_ABSORPTION_PLAN.md docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md docs/presentation/README.md docs/presentation/test_matrix/README.md
git commit -m "docs: reconcile operational plans and specs"
```

### Task 3: Correct Misleading ADR and Archive Docs In Place

**Files:**
- Modify: `docs/archive/README.md`
- Modify: `docs/adr/ADR-010.md`
- Modify: `docs/adr/ADR-010-presentation-completion.md`
- Modify: `docs/adr/ADR-011-presentation-spatial-status.md`
- Modify: `docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md`
- Modify: `docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md`
- Modify: `docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md`
- Modify: `docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md`

- [ ] **Step 1: Read the archive/ADR entry points and known misleading docs**

Run:

```powershell
Get-Content 'docs/archive/README.md'
Get-Content 'docs/adr/ADR-010.md'
Get-Content 'docs/adr/ADR-010-presentation-completion.md'
Get-Content 'docs/adr/ADR-011-presentation-spatial-status.md'
Get-Content 'docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md'
Get-Content 'docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md'
Get-Content 'docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md'
Get-Content 'docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md'
```

Expected: identify where archive/ADR docs still present themselves as active execution truth or contain obsolete unresolved placeholders.

- [ ] **Step 2: Edit archive/ADR docs in place to remove execution-risking misdirection**

Apply the following content rules while editing:

```text
- docs/archive/README.md must clearly steer readers to the canonical docs for current execution truth.
- ADR docs must preserve the decision/history but remove stale present-tense status claims and stale placeholder wording.
- Archive planning docs must be readable as historical/reference inputs, not active execution authority.
- Older roadmap/architecture docs must replace obsolete “[TBD]”, “must begin”, or “current phase” framing when those statements are no longer true in the present branch state.
```

- [ ] **Step 3: Run the archive/ADR sweep for residual stale markers**

Run:

```powershell
rg -n "\[TBD|TODO|FIXME|must begin|current phase|not implemented|still needed|remaining work|open work|authoritative|canonical" docs/archive docs/adr
```

Expected: residual hits, if any, are either historically explicit or intentionally point readers back to canonical current-state docs.

- [ ] **Step 4: Commit the archive/ADR pass**

```bash
git add docs/archive/README.md docs/adr/ADR-010.md docs/adr/ADR-010-presentation-completion.md docs/adr/ADR-011-presentation-spatial-status.md docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md
git commit -m "docs: reconcile archive and adr truthfulness"
```

### Task 4: Run the Final Reference Sweep, Verify Consistency, and Record the Pass

**Files:**
- Modify: `WORKLOG.md`
- Inspect: `docs/external-intake/*.md`
- Inspect: `docs/asset_intake/*.md`
- Inspect: `docs/BATTLE_CORE_NATIVE_SPEC.md`
- Inspect: `docs/SAVE_DATA_CORE_NATIVE_SPEC.md`
- Inspect: `docs/presentation/schema_changelog.md`

- [ ] **Step 1: Run the repo-wide documentation sweep**

Run:

```powershell
rg -n "\[TBD|TODO|FIXME|In Progress|Partially Remediated|remaining work|open work|not implemented|not started|current phase|must begin|authoritative|canonical" README.md PLAN.md WORKLOG.md docs
```

Expected: only intentional, accurate, or explicitly historical usages remain.

- [ ] **Step 2: Inspect any residual hits and fix the ones that could still mislead execution**

Apply this decision rule:

```text
If a residual hit could cause an engineer to choose the wrong execution authority, misunderstand current status, or believe obsolete open work is still active, edit it now. If it is clearly historical and non-misleading as written, leave it.
```

- [ ] **Step 3: Add the final worklog entry for this docs pass**

Append an entry shaped like:

```markdown
### 2026-04-19 — Documentation Truth Reconciliation
- **Action**: Reconciled canonical status docs, active plans/specs, and misleading archive/ADR materials so current execution truth no longer conflicts across the repo.
- **Result**: Canonical current-state docs, operational docs, and archive/reference materials now point to the same present program truth; residual historical docs are explicitly non-authoritative.
```

- [ ] **Step 4: Run the final verification commands**

Run:

```powershell
git diff --stat
rg -n "\[TBD|TODO|FIXME|current phase|must begin|remaining work|open work|not implemented|not started" README.md PLAN.md WORKLOG.md docs
git diff -- README.md PLAN.md WORKLOG.md docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md docs/PROGRAM_COMPLETION_STATUS.md docs/COMPAT_EXIT_CHECKLIST.md docs/DEVELOPMENT_KICKOFF.md docs/superpowers/specs/2026-04-19-phase-2-runtime-closure-design.md docs/superpowers/plans/2026-04-19-phase-2-runtime-closure.md docs/NATIVE_FEATURE_ABSORPTION_PLAN.md docs/WAVE2_AUDIO_STATE_SYNC_PLAN.md docs/presentation/README.md docs/presentation/test_matrix/README.md docs/archive/README.md docs/adr/ADR-010.md docs/adr/ADR-010-presentation-completion.md docs/adr/ADR-011-presentation-spatial-status.md docs/archive/planning/URPG_MASTER_NATIVE_ABSORPTION_AND_PGMMV_ROADMAP_2026-04-18.md docs/archive/planning/URPG_NATIVE_ABSORPTION_ROADMAP_2026-04-18.md docs/archive/planning/URPG_PGMMV_SUPPORT_PLAN.md docs/archive/planning/urpg_first_class_presentation_architecture_plan_v2.md
```

Expected: diffs show only truth-reconciliation changes, and residual wording hits are either accurate or explicitly historical.

- [ ] **Step 5: Commit the final sweep**

```bash
git add WORKLOG.md README.md PLAN.md docs
git commit -m "docs: reconcile repo documentation to current truth"
```
