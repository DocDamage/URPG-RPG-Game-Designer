# Product Completion Batched Finish Plan

**Date:** 2026-07-16
**Branch baseline:** `agent/pfu-i1-creator-qualification` at `ac309be6e1`
**Scope:** all 117 PCQ and ARDY rows in the canonical product-completion ledger
**Status:** active execution overlay; it changes sequencing and verification cadence, not requirement scope

## Current execution snapshot

This snapshot is current through `45740b5b46` (2026-07-17). Classifications are implementation-evidence accounting,
not release approval; the authoritative per-row detail remains in `content/readiness/product_completion_wave_*_gap_matrix.json`.

| Wave | Proved | Partial | Missing | External |
| --- | ---: | ---: | ---: | ---: |
| A | 0 | 23 | 0 | 3 |
| B | 5 | 11 | 0 | 0 |
| C | 21 | 0 | 0 | 0 |
| D | 8 | 10 | 0 | 4 |
| E | 0 | 14 | 0 | 1 |
| F | 8 | 1 | 0 | 5 |
| G | 0 | 0 | 0 | 3 |

PCQ-506 is now proved within Wave D: the fault corpus invokes all six concrete recovery-class adapters, application settings
quarantine malformed input before defaults are saved, and marker-validated failed package staging is relocated to
project recovery with a receipt. Focused normal and warnings-as-errors coverage passes 204 assertions in nine cases.

## Authority and purpose

The requirement definitions and acceptance criteria remain authoritative in
`docs/superpowers/plans/2026-07-15-product-completion-nintendo-quality-and-ardy-plan.md`.
This plan replaces the expensive one-small-slice/one-full-build cadence with larger implementation waves.

The current evidence index contains all 117 required IDs, but every row is still `in_progress`. Existing code and
tests must therefore be audited and reused, not reimplemented, while unproven claims remain open.

## What 100% completion means

Completion requires all of the following at the same clean source commit:

1. Every canonical PCQ and ARDY acceptance criterion is satisfied or, where the canonical row is explicitly
   conditional, closed by an approved defer/reject decision with evidence.
2. All 117 release-evidence rows are current, indexed, non-stale, and marked `passed` only when their evidence is
   sufficient.
3. No accepted P0 defect, deferred release-critical verification placeholder, false readiness claim, or unresolved
   required-rights issue remains.
4. Debug and Release builds, discovered tests, package/install smoke, clean-clone, clean-machine, migration, security,
   accessibility, localization, performance, and truth audits pass at the final candidate.
5. Required manual reviews, hardware captures, user research, beta evidence, and release-owner signoff exist. Automated
   fixtures do not substitute for these.
6. Public support claims match the qualified platform, feature, accessibility, localization, and package evidence.

## New execution model

### Audit before implementation

At the start of each wave, classify every included requirement as:

- **proved:** current source and current evidence already satisfy the full acceptance criterion;
- **partial:** useful implementation exists but one or more behaviors or evidence classes are missing;
- **missing:** the required product behavior does not exist;
- **external:** completion needs hardware, licensing, participants, legal review, or release-owner action.

Only partial and missing behavior is coded. Proved behavior receives evidence closure without churn.

### Code the whole wave before building

During a coding wave:

- finish all related source, tests, fixtures, schemas, diagnostics, and documentation before compiling;
- use read-only inspection, `rg`, JSON parsing, schema review, and `git diff --check` while coding;
- do not run incremental full builds or test executables after each small slice;
- do not commit or push partial wave state;
- keep compatibility shims only where the canonical acceptance criteria require them;
- preserve honest `in_progress` status until the wave boundary proves the claim.

### One verification boundary per wave

At the end of an implementation wave:

1. Run one grouped Debug build:

   ```powershell
   cmake --build build/dev-ninja-debug --target urpg_tests urpg_runtime urpg_editor -j 8
   ```

2. Run the wave's focused suites sequentially. Do not run multiple `urpg_tests.exe` instances concurrently because
   several fixtures share temporary roots.
3. Repair all failures as one boundary. Use the narrowest failing target/test while diagnosing; rerun the grouped build
   once after the fixes settle.
4. Update canonical docs and the release evidence index with exact commands, counts, hardware, commit basis, and limits.
5. Commit once, push once, verify the remote tip, and start the next wave from a clean worktree.

Clean rebuilds are reserved for the clean-clone/bootstrap requirement and the final release candidate. Stale generated
objects may be cleaned only when dependency evidence proves they are stale.

## Planned build budget

| Boundary | Normal-path build allowance |
| --- | --- |
| Waves A-E | One grouped Debug build per wave: five total |
| Wave E package qualification | One Release/package build after its Debug boundary |
| Wave F research/beta findings | One grouped Debug build for the consolidated P0/P1 remediation batch, only if code changes result |
| Wave G final candidate | One clean-clone Debug build, one clean Release/package build, then full gates without source changes |

Documentation-only evidence updates do not trigger product rebuilds. A release-candidate source change invalidates the
affected final evidence and returns to the appropriate wave gate.

## Cross-wave evidence procurement

Start these early so implementation does not finish before its external evidence can be obtained:

- reserve each claimed desktop OS, target hardware class, input-device matrix, screen reader, and clean VM;
- decide and legally review the complex-script shaping dependency and shipping fonts for PCQ-652;
- recruit representative research and beta participants with consent/privacy materials;
- obtain asset, binary dependency, SBOM, notices, signing, and platform-support ownership;
- perform ARDY use-case and legal admission before any 3D research implementation;
- schedule release-owner reviews for scope freeze, P0 disposition, beta exit, and ship/delay decision.

## Wave A — Release truth, project integrity, and editor foundation

**Requirements:** PCQ-000-006, PCQ-100-108, PCQ-200-209 (26 rows)

### Coding scope

- Close build provenance, stale-binary rejection, scope admission, clean-clone bootstrap, evidence indexing, and P0 policy.
- Finish the authoritative document-owner matrix, atomic cross-document operation coordinator, stable reference index,
  impact operations, project-wide history boundaries, crash recovery, migrations, and external-change conflicts.
- Finish design tokens, shared widgets, persisted scaling, guided startup, native pickers/fallbacks, workspace layout,
  command discovery, focus/input semantics, onboarding, and actionable/redacted errors.
- Consolidate existing partially implemented reference, command-palette, dirty-state, recovery, and widget systems rather
  than creating parallel owners.

### Required evidence before the boundary

- Machine-readable ownership, reference, migration, widget-state, and scale matrices.
- Fault-injection fixtures for atomic operations, stale binaries, recovery, and external changes.
- Route-by-scale snapshots and current automated accessibility/widget audits.
- Disposable-clone configure/build/test-discovery equivalence report. This is the first permitted clean bootstrap run.

### Wave gate

One grouped Debug build, then sequential project/session/history/reference/migration/UI/widget/snapshot suites and the
agent-knowledge check. Commit and push only after all 26 rows have either passed evidence or an explicit remaining
external dependency carried into the final manual lane.

## Wave B — Assets and spatial creation

**Requirements:** PCQ-300-307, PCQ-350-357 (16 rows)

### Coding scope

- Finish the durable asset-job contract, 100k-row catalog behavior, safe relink/detach/replace/deduplicate/rename/move/
  delete, reproducible image/audio revisions, contextual “use here,” archive custody, and deterministic rights notices.
- Complete the canonical Map workspace, selection and batch operations, reusable prefab lifecycle, terrain/collision/
  navigation tooling, world graph, play-from-here/context return, impact/minimap views, and large-map warnings.
- Route default asset report/shard discovery and JSON parsing through bounded work; complete bounded project-graph
  discovery/parsing/mutation rather than stopping at indexed queries.

### Required evidence before the boundary

- Tiny/medium/100k asset fixtures, adversarial archives, transform golden hashes, audio QA corpus, and rights fixtures.
- Map operation matrix covering preview, undo/redo, save/reopen, runtime round trip, diagnostics, and keyboard routes.
- Two-map world-transfer loop and large spatial stress fixtures.

### Wave gate

One grouped Debug build, then sequential asset, attachment, archive, transform, audio, map, grid-part, navigation,
world-graph, spatial, package-rights, and performance-fixture suites. Record manual route equivalence only after the
candidate is stable.

## Wave C — Events, narrative, data, menus, and semantic canvas alternatives

**Requirements:** PCQ-400-407, PCQ-450-455, PCQ-480-485, PCQ-654 (21 rows)

### Coding scope

- Close the native event-command matrix, structured authoring, stable pickers, analysis, dialogue/quest depth,
  templates, runtime trace, and separately reported MZ compatibility.
- Finish virtualized database tables, atomic batch editing, safe reference operations, balance simulation/history, and
  vertical-slice data integration.
- Finish Menu canvas editing, reusable components, states/transitions, typed bindings, focus/overflow/target previews,
  and original package-safe starter templates.
- Complete semantic tree/list alternatives across menu, dialogue, quest, map, and graph surfaces: ordered keyboard and
  controller navigation, property editing, connection creation, every required canvas operation, and object-linked
  diagnostics without precise pointer use.

### Required evidence before the boundary

- Command/runtime/diagnostic matrix and curated bad-event corpus.
- Branching voiced quest save/load/runtime/package fixture.
- Large database and atomic-invalid-batch fixtures plus seeded balance reports.
- Menu viewport/state/focus/binding/template matrices.
- One cross-editor semantic-operation matrix proving canvas/tree/list equivalence.

### Wave gate

One grouped Debug build, then sequential event, dialogue, quest, database, economy, menu, compatibility, semantic-input,
accessibility, runtime, save/load, and package-closure suites.

## Wave D — Playtest, presentation, input, accessibility, and localization

**Requirements:** PCQ-500-507, PCQ-600-607, PCQ-650-653, PCQ-655-656 (22 rows)

### Coding scope

- Finish playtest blockers/checkpoints/context, bounded hot reload, runtime state inspection, breakpoints/stepping,
  replay, profiling, full recovery, and previewed redacted support bundles.
- Approve and apply the original runtime presentation bible across startup, exploration, combat, feedback, final
  vertical-slice assets, calibration, and pacing.
- Complete unified semantic input, accessibility settings, localization shaping/fallback/RTL/IME/formatting, and the
  existing caption/voice/take system.
- Expand automated accessibility audits for focus, contrast, hit targets, clipping, labels, overflow, and unsafe motion.

### Required evidence before the boundary

- Hot-reload capability matrix, deterministic debug/replay trace, recovery fault corpus, and redaction corpus.
- In-engine component showcase and representative exploration/combat captures under accessibility variants.
- Device/hot-plug/rebinding matrix, locale/pseudo-locale/RTL/IME/font corpus, and object-linked known-bad audit fixtures.
- Manual keyboard/controller/screen-reader/200%-scale/reduced-motion/high-contrast/sound-off/locale review begins here;
  final retest is repeated on the frozen candidate in Wave G.

### Wave gate

One grouped Debug build, then sequential playtest, runtime diagnostics, replay, recovery, support, presentation, input,
accessibility, localization, dialogue-media, audio, rendering, snapshot, and save-migration suites.

## Wave E — Performance, reliability, security, platform, and packaging

**Requirements:** PCQ-700-707, PCQ-750-756 (15 rows)

### Coding scope

- Complete real capture execution for the governed benchmark fixtures and all eight primary routes; finish project open,
  map edit, save, playtest launch, and package bounded routes and the remaining asset/project-graph I/O work.
- Finish long-session stress, fault injection, sanitizer/fuzzer alternatives, threat model/adversarial fixtures, privacy
  defaults, deterministic SBOM/license policy, and dependency review.
- Complete PlatformServices capabilities, remove runtime desktop assumptions, finish desktop package/install/repair/
  portable policy, version/update/migration truth, support classification, deterministic package manifests, and exact
  release provenance.

### Required evidence before the boundary

- At least 20 real samples per primary route on every target hardware class, approved budgets, and versioned reports.
- Long-session memory/resource trends, fault recovery results, sanitizer/fuzzer evidence or explicit toolchain blocker,
  threat model, default-session network capture, SBOM, licenses, and notices.
- Headless/desktop capability matrices, equivalent-build reproducibility result, N-1 project/save/package upgrade
  fixtures, and claimed-OS clean install/play/save/relaunch/uninstall evidence.

### Wave gate

One grouped Debug build and sequential performance/stress/fault/security/platform/update/package suites. Then perform one
Release/package build and clean-machine smoke without changing source. Provisional or single-sample performance results
cannot close PCQ-700/701.

## Wave F — ARDY decision, representative research, beta, and consolidated remediation

**Requirements:** ARDY-000-009, PCQ-900-903 (14 rows)

### ARDY lane

- Complete use-case and legal admission first.
- If rejected or deferred, record the decision and close conditional implementation rows with explicit non-adoption
  evidence; do not add runtime dependencies.
- If approved, keep the reproducible helper isolated, convert through a neutral versioned document, run the three
  bounded pilots, measure creator-time ROI, and make the adopt/defer/reject decision before any productization.
- Treat the optional 2D experiment as conditional on one accepted motion and positive measured value.

### User research and beta lane

- Run consented novice, intermediate, accessibility, and experienced-user sessions over the complete creator journey.
- Rank findings by the canonical P0/P1 bar. Consolidate all reproducible P0/P1 fixes into one remediation coding batch;
  do not build after each finding.
- After that batch, run one grouped Debug build and all affected focused suites once, then retest with new users.
- Run the bounded beta with backup/migration/known-issue/support/rollback policy and an explicit crash/data-loss ledger.

### Wave gate

Research evidence, privacy review, ARDY decision, resolved/retested P0/P1 findings, and beta exit report. No product build
is required if the wave produces no source changes.

## Wave G — Freeze, clean qualification, and release decision

**Requirements:** PCQ-904-906 (3 rows), plus final closure of any external evidence carried by earlier waves

### Freeze rules

- Freeze the candidate and admit only approved P0/P1 blocker fixes.
- Every admitted change names affected evidence and reruns the affected focused gate before the candidate is re-frozen.
- No new feature breadth, speculative cleanup, or ARDY productization enters the candidate.

### Final qualification sequence

1. Verify a clean, commit-matched worktree and frozen 117-row evidence ledger.
2. Bootstrap a disposable clean clone and compare configure/build/test discovery.
3. Run clean Debug and Release builds from the frozen commit.
4. Run the full local, PR, spatial, snapshot, export, project-audit, process, presentation, release-candidate, package,
   promoted-asset, LFS, provenance, security, license, and truth gates.
5. Run clean-machine install/play/save/relaunch/upgrade/uninstall and the final target-hardware performance captures.
6. Repeat the signed accessibility/localization/input/manual review on the frozen binaries.
7. Verify every evidence row references the frozen commit and no row is incomplete, stale, or falsely passed.
8. Hold the explicit ship/delay/reduce-scope decision and align public support claims, known issues, risks, and owners.

Any source change restarts the affected focused gate and final clean build. Documentation-only correction requires a new
commit and evidence-source alignment but no product rebuild when binary provenance remains demonstrably unchanged.

## Coverage accounting

| Wave | IDs | Count |
| --- | --- | ---: |
| A | PCQ-000-006, PCQ-100-108, PCQ-200-209 | 26 |
| B | PCQ-300-307, PCQ-350-357 | 16 |
| C | PCQ-400-407, PCQ-450-455, PCQ-480-485, PCQ-654 | 21 |
| D | PCQ-500-507, PCQ-600-607, PCQ-650-653, PCQ-655-656 | 22 |
| E | PCQ-700-707, PCQ-750-756 | 15 |
| F | ARDY-000-009, PCQ-900-903 | 14 |
| G | PCQ-904-906 | 3 |
| **Total** | **All canonical IDs exactly once** | **117** |

## Immediate next action

Begin Wave A with a machine-readable gap matrix derived from current source and current evidence. Do not code or rebuild
until every Wave A row is classified and its exact missing proof is named. Then implement all Wave A gaps before its
single grouped verification boundary.
