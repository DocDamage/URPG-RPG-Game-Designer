# Documentation Truth Reconciliation Design

Date: 2026-04-19
Status: Approved design for repo-wide misleading-document cleanup
Scope: Update every document that could mislead current execution, including stale ADR and archive material, by editing in place to match current truth

## Problem

The repository's canonical runtime and program-status docs were updated during Phase 2 closure, but the broader documentation set still contains older present-tense claims, stale open-work markers, and archive or ADR wording that can misdirect current engineering work. The goal of this pass is not to rewrite the entire documentation corpus or flatten historical context. The goal is to remove execution-risking contradictions so that any engineer reading the repo gets an accurate picture of current state.

## Goals

- Make the repository documentation safe to execute against today.
- Correct stale present-tense claims, open gaps, and completion markers that no longer match current truth.
- Preserve useful historical context while eliminating wording that implies obsolete current state.
- Keep canonical docs, operational docs, ADRs, archives, and reference material aligned around the same current-status narrative.

## Non-Goals

- Rewriting every document into a single template or style.
- Removing historical decision records just because the implementation has evolved.
- Performing unrelated code changes to make docs true.
- Expanding scope beyond documents that could reasonably influence active execution.

## Recommended Approach

Use a canonical-first, in-place correction pass.

This approach updates the most authoritative current-state documents first, then works outward into operational docs, ADRs, archives, and remaining references. Every misleading document is edited in place. Historical intent and decision context are preserved where they matter, but stale wording is rewritten when it implies current truth that is no longer accurate.

This is preferred over annotation-heavy preservation because leaving obsolete body text in place still forces readers to reconcile contradictions manually. It is also preferred over a full repo rewrite because the task is truth reconciliation, not style normalization.

## Document Classes

### 1. Canonical Docs

These are the documents that define current repo status and should be treated as the top-level source of truth. This class includes materials such as:

- `docs/archive/planning/PROGRAM_COMPLETION_STATUS.md`
- `docs/status/PROGRAM_COMPLETION_STATUS.md`
- `WORKLOG.md`
- any other current-state status hub discovered during the audit

Work in this class focuses on:

- phase and completion accuracy
- current remediation status
- current runtime/editor/diagnostics state
- cross-links to the correct authoritative sources

### 2. Operational Docs

These are active documents an engineer might use to decide what to build next, how a subsystem currently behaves, or what an accepted plan says. This includes:

- active specs under `docs/superpowers/specs/`
- active plans under `docs/superpowers/plans/`
- workflow docs outside archives that still guide present work

Work in this class focuses on:

- removing stale statements about what remains open
- aligning current-state notes with the canonical docs
- correcting implementation/status assumptions that no longer hold

### 3. ADR and Archive Docs

These documents preserve decision history and older planning material, but they can still mislead execution if they contain present-tense claims, unresolved placeholders, or outdated framing that now conflicts with current reality.

For this class:

- preserve the historical decision or historical plan context
- edit stale status framing in place
- rewrite obsolete "still needed", "not implemented", or `[TBD]` wording when that wording is no longer true
- avoid broad stylistic modernization unless required to remove confusion

The pass does not treat archive status as immunity from correction. If a reader could plausibly follow the document and make a wrong execution decision today, it must be corrected.

### 4. Reference Sweep

After the first three classes are reconciled, run a repo-wide sweep for residual misleading language patterns such as:

- `[TBD]`
- `TODO` and `FIXME` markers in docs
- stale phase labels
- obsolete "partial" or "not started" status claims
- forward-looking statements that have already been completed or superseded

The sweep is intended to catch stragglers, not to force edits to clearly historical, non-misleading prose.

## Editing Rules

Each candidate document should be evaluated with the following rules:

1. If the document states or implies current truth, it must match current truth.
2. If the document is historical but includes execution-relevant current-state claims, those claims must be corrected.
3. If the historical intent is still useful, preserve it while updating stale framing around it.
4. If a statement is ambiguous between historical and current meaning, rewrite it to be explicit.
5. If a document is no longer authoritative, that should be clear from its wording and cross-references without leaving contradictory present-tense claims behind.

## Execution Strategy

The implementation should proceed in bounded passes:

1. Re-scan the doc corpus and build a concrete target list of potentially misleading documents.
2. Reconcile canonical docs first so downstream edits have a stable reference point.
3. Update operational docs that engineers are most likely to use during active development.
4. Update ADR and archive docs that contain stale present-tense claims or unresolved placeholders that no longer reflect current truth.
5. Run a final reference sweep for residual contradictions, stale markers, and superseded status language.
6. Verify consistency across the canonical docs and record the documentation pass in `WORKLOG.md`.

## Risks and Controls

### Risk: Overwriting meaningful history

Control:
Preserve the decision or planning record itself. Only rewrite the parts that incorrectly describe current truth or imply current actionability.

### Risk: Turning the pass into an unbounded rewrite

Control:
Restrict edits to documents that could mislead execution, status interpretation, or engineering prioritization. Do not normalize style or restructure docs unless needed for clarity.

### Risk: Divergence between canonical and non-canonical docs after edits

Control:
Use the canonical docs as the reference baseline, then perform a final consistency sweep after all targeted edits are complete.

## Verification

The work is complete when:

- canonical docs agree on current phase and current program status
- active specs and plans no longer state obsolete open work as if it is still current
- ADR and archive docs no longer contain misleading present-tense claims about current implementation state
- the repo-wide document sweep does not find unresolved misleading markers that affect current execution
- `WORKLOG.md` records the reconciliation pass

## Acceptance Criteria

- No document likely to influence active engineering work contradicts the current codebase or current program status.
- Archive and ADR documents that previously implied obsolete current truth have been corrected in place.
- Remaining historical material is still readable as history and no longer misdirects present execution.
- The documentation set gives a coherent current-state narrative without requiring readers to resolve contradictions manually.
