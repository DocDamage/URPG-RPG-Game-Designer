# URPG Product Completion, Nintendo-Caliber Quality, and ARDy Evaluation Plan

> **Status:** Proposed release-quality execution appendix.
>
> **Created:** 2026-07-15.
>
> **Repository:** G:\URPG Maker-development.
>
> **Planning authority:** This document sequences and sharpens the release work in
> docs/superpowers/plans/2026-07-14-product-feature-usability-native-absorption-plan.md.
> It does not replace that plan, the readiness JSON, release matrices, or canonical
> debt records.
>
> **Meaning of “Nintendo-caliber”:** A public, platform-neutral quality target:
> unusually clear, responsive, friendly, controller-native, forgiving, accessible,
> reliable, and cohesive. It does not mean copying Nintendo characters, trademarks,
> sounds, fonts, layouts, trade dress, or private platform requirements. URPG must
> keep its own visual and product identity.

## 1. Executive Decision

URPG is not missing a single spectacular headline feature. It is missing the
cross-cutting product systems that make its many existing features safe, discoverable,
fast, coherent, and trustworthy across a complete creator journey.

The release strategy is therefore:

1. Repair candidate integrity and stop accepting stale or deferred evidence.
2. Finish one golden creator loop before adding more broad capability.
3. Build shared transaction, reference, migration, input, and design-system foundations.
4. Close the highest-regret authoring gaps: world flow, event depth, asset lifecycle,
   data productivity, menu authoring, debugging, accessibility, and recovery.
5. Replace prototype presentation with a distinctive, polished URPG experience.
6. Prove the result through a creator-authored vertical slice, packaged build, fresh
   evidence, and external usability sessions.
7. Treat Nintendo platform work as a separate, access-controlled qualification lane.
8. Treat NVIDIA ARDy as a conditional offline research pilot, not a runtime dependency
   and not a release blocker.

### 1.1 Priority verdict

| Priority | Work | Why it matters |
| --- | --- | --- |
| P0 | Build integrity, golden loop, save safety, undo, recovery, truthful gates | A release cannot be trusted without these. |
| P0 | Project-wide references and safe rename/move/delete/replace | This is the costliest omission to retrofit after creators build large projects. |
| P0 | World navigation, event authoring, playtest/debug loop | These decide whether a creator can finish a game without editing files manually. |
| P0 | Controller, accessibility, localization, runtime presentation | These determine whether the result feels like a consumer product. |
| P1 | Map productivity, asset jobs, database bulk tools, menu system | These determine daily creator speed and perceived maturity. |
| P1 | Platform abstraction, packaging, performance, reliability | These prevent desktop-only assumptions from hardening into architecture. |
| P2 | Stable extension SDK and advanced collaboration | Valuable after the first-party workflow is safe and coherent. |
| Research | ARDy 3D motion generation | Useful only if a real skeletal 3D lane is approved. |
| Defer | More AI breadth, marketplace, cloud collaboration, arbitrary 3D engine work | These would distract from product closure today. |

## 2. Audited Starting Point

This plan was written against the repository state audited on 2026-07-15. Every
implementation session must re-run the repository preflight because this snapshot
will age.

| Area | Current truth | Consequence |
| --- | --- | --- |
| Candidate build | The strict PFU-I1 qualification attempt stopped in engine/core/ai/creator_command_planner.cpp near line 820 because planEventMessage has an unmatched initializer/brace. | The current source candidate is not build-qualified. Fix and rebuild before interpreting older test binaries. |
| Evidence freshness | Existing editor/test binaries predate relevant source changes. | Historical green results are useful regression baselines, not evidence for the current candidate. |
| Creator journey | Historical creator-journey tests pass, but the recorded baseline still marks attachment/package as partial and add-event/playtest/return as deferred. | The golden loop remains open until the current source is built and the entire wrapper is passed from the product UI. |
| M6 playtest | Initial native playtest work exists. | Blocker policy, checkpoints, diagnostic focus, bounded reload, and end-to-end proof remain. |
| M7 recovery | Recovery foundations exist. | Broader document coverage, asset relinking, creator walkthroughs, and evidence remain. |
| M8 contextual tools | Initial contextual entry points exist. | WYSIWYG placement, manual proof, audio/animation breadth, and route cohesion remain. |
| M9 creator proof | No current creator-authored passed report exists. | A person must build the vertical slice from the UI and complete the exact acceptance script. |
| M10 product review | Graphical, accessibility, input, and performance review remains open. | “Feels finished” is not yet supported by evidence. |
| M11 release | Fresh gates, clean-clone hydration, external platform checks, and a release decision remain. | The repository cannot yet make a current release claim. |
| Editor presentation | The observed binary presents stock ImGui density, weak hierarchy, small text, fixed-size surfaces, brittle path entry, and limited semantic accessibility exposure. | The editor needs a designed product shell, not only a new color palette. |
| Scale and layout | UI scale is persisted, but editor-wide application is incomplete; important windows still use fixed geometry. | Scaling and responsive workspace behavior must be implemented and tested. |
| Export/platforms | Public export targets cover Windows, Linux, macOS, and Web; no Nintendo platform backend is present. Web is not release-qualified, and one toolchain path disables joystick/haptics. | Do not imply console readiness. Build a platform seam before confidential platform work. |
| 3D animation | URPG has general scalar animation tracks, sprite/timeline systems, and a faux-3D raycast dungeon lane. It does not have a shipping skeletal mesh, skinning, retargeting, and 3D animation pipeline. | ARDy cannot be dropped into the current runtime and should not drive 1.0 scope. |

### 2.1 Product strengths to preserve

- Deterministic C++20 native core.
- Atomic persistence and existing dirty-state/recovery foundations.
- Native runtime with an explicit RPG Maker MZ compatibility boundary.
- Governed asset catalog, promotion, attachment, diagnostics, and packaging seeds.
- Headless and OpenGL rendering paths, snapshot tests, visual comparison, and replay.
- ImGui editor with broad domain panels already connected to native models.
- Strong local gate vocabulary and machine-readable readiness/status infrastructure.
- A useful 2D and 2.5D RPG foundation that should not be destabilized by an
  unapproved general-purpose 3D pivot.

## 3. Goals, Non-Goals, and Release Boundary

### 3.1 Goals

- A new creator can create, edit, playtest, debug, package, install, and replay a
  polished small RPG without editing project files or invoking developer scripts.
- Common destructive operations are previewable, reference-aware, undoable, and
  crash-safe.
- Every primary editor and runtime flow works with keyboard and controller where
  appropriate, at supported UI scales, with visible focus and accessible alternatives.
- The editor and generated games share a coherent URPG visual language and interaction
  grammar.
- Current release claims are backed by current binaries, fresh automated evidence,
  manual evidence, and a clean-machine package test.
- Platform-specific code is isolated behind a stable platform-services interface.
- Optional research tooling can produce governed, baked assets without entering the
  runtime dependency graph.

### 3.2 Non-goals for the first complete product

- Copying Nintendo intellectual property or reverse-engineering private standards.
- Claiming Nintendo Switch support before developer approval, hardware access, an
  authorized backend, and successful platform review.
- Turning the faux-3D dungeon feature into an arbitrary mesh/physics/skeletal engine.
- Shipping CUDA, Python, model weights, gated language models, or ARDy inference in a
  game package.
- Building a second editor shell, project database, transaction authority, or runtime.
- Expanding broad AI generation, cloud collaboration, a marketplace, or full MZ parity
  before the golden path is complete.
- Treating snapshots, headless compilation, or unit tests alone as usability evidence.

## 4. Planning and Status Authority

When records disagree, use this order:

1. content/readiness/readiness_status.json for machine-readable subsystem claims.
2. docs/release/RELEASE_READINESS_MATRIX.md for release-facing subsystem evidence.
3. docs/APP_RELEASE_READINESS_MATRIX.md for application-level evidence.
4. docs/PROGRAM_COMPLETION_STATUS.md for current program status.
5. docs/agent/KNOWN_DEBT.md for unresolved release truth.
6. docs/superpowers/plans/2026-07-14-product-feature-usability-native-absorption-plan.md
   for creator-product implementation authority.
7. This document for release-quality sequencing, acceptance, and the ARDy decision.

Do not report percentage complete. Report passed requirements, open blockers, deferred
scope, and evidence dates.

## 5. Product Quality Contract

### 5.1 Nintendo-caliber principles

| Principle | Editor behavior | Player-facing behavior |
| --- | --- | --- |
| Immediate legibility | One obvious primary action, plain labels, strong hierarchy, contextual help, useful empty states. | A player understands the next action, current selection, and consequences without reading a manual. |
| Responsive delight | Input acknowledges immediately; long work becomes a cancellable job; transitions reinforce state. | Controls respond on the current frame when possible; feedback uses motion, sound, particles, and haptics with restraint. |
| Controller-native | Focus order, shortcuts, glyphs, and escape routes work without a mouse for declared surfaces. | All required game flows work from supported controllers, including reconnect and remapping. |
| Forgiving | Preview, undo, autosave, recovery, validation, and friendly diagnostics protect creators. | Confirm destructive actions, preserve saves, make failure recoverable, and avoid dead-end menus. |
| Cohesive | Shared components, spacing, terminology, commands, and status behavior. | Menus, dialogue, battle HUD, pause, settings, saves, and transitions feel authored by one team. |
| Quiet confidence | No stale spinners, mystery modes, dead buttons, raw stack traces, or silent data loss. | Stable frame pacing, bounded loads, safe suspend/resume, and graceful device changes. |
| Inclusive by default | Scaling, keyboard paths, semantic alternatives, contrast, reduced motion, localization stress tests. | Remapping, captions, text scale, readable contrast, motion options, audio alternatives, and safe-area support. |
| Distinctly URPG | Warm, playful, craft-focused identity rooted in URPG tools and community. | Original art direction and sound language; no imitation of another company’s trade dress. |

### 5.2 Definition of done for every creator feature

A feature is not done until all applicable rows pass:

- [ ] Reachable from the native editor shell in the creator’s current context.
- [ ] Has clear empty, loading, success, warning, error, disabled, and cancellation states.
- [ ] Uses the authoritative native domain model; no side database or generic JSON owner.
- [ ] Persists atomically with schema versioning and migration coverage.
- [ ] Registers dirty state and participates in undo/redo or explains a safe boundary.
- [ ] Updates the project reference graph and packaging closure.
- [ ] Validates before mutation and emits actionable, object-linked diagnostics.
- [ ] Supports keyboard navigation; controller support is included where declared.
- [ ] Has an accessible alternative for visual canvases and meaningful focus labels.
- [ ] Uses design-system tokens and behaves across the supported scale range.
- [ ] Has unit/integration coverage and a reachable WYSIWYG/manual proof.
- [ ] Round-trips through save, close, reopen, playtest, package, and load where relevant.
- [ ] Documents recovery, cancellation, and failure behavior.
- [ ] Updates readiness/debt/release truth without overstating broader readiness.

### 5.3 Provisional quality budgets

These are internal targets to validate on representative desktop hardware. Platform
budgets must be replaced by approved target-hardware measurements when access exists.

| Metric | Initial target | Failure behavior |
| --- | --- | --- |
| Runtime presentation | Stable 60 Hz mode with p95 frame time within the chosen frame budget for the vertical slice. | Capture profiler trace and block release on reproducible gameplay hitching. |
| Direct input acknowledgement | Visual or audio acknowledgement within 100 ms under normal load. | Log the interaction and remove blocking work from the UI/runtime thread. |
| Editor interaction | No routine operation blocks the UI for more than 100 ms; longer work is a progress job. | Show progress, current stage, cancel, retry, and recovery. |
| Micro-motion | Usually 120–220 ms, interruptible, with reduced-motion behavior. | Disable decorative motion before compromising input or readability. |
| Project save | Atomic; no acknowledged successful save may lose committed data. | Keep prior snapshot, surface recovery, and emit a redacted diagnostic bundle. |
| Autosave/recovery | Recover to the last acknowledged operation boundary after simulated interruption. | Release blocker for any primary document owner that loses work silently. |
| Search and pickers | Return useful feedback immediately and avoid unbounded main-thread scans. | Show indexed/progress state and allow cancellation. |
| UI scale | All primary editor routes work across the project’s supported scale setting without clipping critical controls. | Capture route/scale snapshots and block affected route signoff. |
| Controller focus | No focus trap or unreachable required action in declared controller routes. | Fail controller-navigation audit. |
| Text/localization | Pseudo-localized text, long text, missing glyphs, and supported scripts do not obscure required actions. | Emit object-linked diagnostics; block affected locale/package. |
| Beta reliability | Establish a measurable crash-free-session baseline, then require no known P0 data-loss/crash defect. | Keep release candidate frozen until triaged and retested. |

## 6. Target Architecture

~~~mermaid
flowchart LR
    Shell["URPG editor shell and design system"]
    Commands["Typed domain commands"]
    Transactions["Project transaction, history, and recovery spine"]
    References["Stable-ID reference and impact graph"]
    Documents["Native project documents"]
    Assets["Governed asset pipeline and background jobs"]
    Runtime["Deterministic native runtime"]
    Platform["Platform services interface"]
    Package["Validator, packager, and installer"]
    Tools["Optional supervised offline tools"]
    Baked["Reviewed, versioned, baked artifacts"]

    Shell --> Commands
    Commands --> Transactions
    Commands --> References
    Transactions --> Documents
    References --> Documents
    Assets --> Documents
    Documents --> Runtime
    Runtime --> Platform
    Documents --> Package
    Platform --> Package
    Tools --> Baked
    Baked --> Assets
~~~

### 6.1 Architecture invariants

1. Editor mutations go through typed domain commands.
2. A project operation either commits all affected documents/references or none.
3. Stable IDs, not display names or raw paths, identify project objects.
4. Runtime and package consumers read normal native assets and documents.
5. External tools produce staged artifacts and manifests; they never own project state.
6. Platform services are capability-based and can report unavailable explicitly.
7. Private platform requirements and credentials do not enter public repository history.

## 7. Sequencing and Dependency Map

~~~mermaid
flowchart TD
    R0["R0: candidate integrity and scope freeze"]
    R1["R1: transaction, references, migrations"]
    R2["R2: editor shell and design system"]
    R3["R3: asset and map productivity"]
    R4["R4: events, data, menus, world flow"]
    R5["R5: playtest, debug, recovery"]
    R6["R6: runtime presentation and accessibility"]
    R7["R7: performance, packaging, platform seam"]
    R8["R8: creator-authored vertical slice"]
    R9["R9: external beta and release candidate"]
    NP["Nintendo qualification lane"]
    AR["ARDy research pilot"]

    R0 --> R1
    R0 --> R2
    R1 --> R3
    R1 --> R4
    R2 --> R3
    R2 --> R4
    R3 --> R5
    R4 --> R5
    R5 --> R6
    R6 --> R7
    R7 --> R8
    R8 --> R9
    R7 --> NP
    R1 --> AR
    AR -. "Only if 3D lane is approved" .-> R8
~~~

### 7.1 Effort vocabulary

Effort bands are planning aids, not calendar promises:

- **S:** one focused change, usually days.
- **M:** one subsystem slice, usually one to two focused engineering weeks.
- **L:** several connected owners and evidence lanes.
- **XL:** a program made of multiple independently releasable slices.

No phase exits because time elapsed. It exits because its acceptance evidence passes.

## 8. R0 — Candidate Integrity, Truth, and Feature Freeze

**Outcome:** Every later decision is based on a fresh, reproducible candidate.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-000 | S | Fix the unmatched initializer/brace in planEventMessage, add the smallest regression test for event-message planning, and confirm no adjacent plan serialization change was lost. | engine/core/ai/creator_command_planner.cpp; focused AI planner tests | Fresh Debug build and focused test pass. |
| PCQ-001 | S | Add build provenance to qualification output: commit, dirty state, compiler, preset, configure time, binary time, and gate start time. Reject stale binaries. | tools/ci; generated qualification report | A deliberately stale binary causes a clear gate failure. |
| PCQ-002 | M | Re-run PFU-I1 on current source and replace every “verification deferred by user instruction” status with passed evidence, a current blocker, or explicitly deferred scope. | Active PFU plan; known debt; creator journey reports | No release-critical requirement is represented by a deferred verification placeholder. |
| PCQ-003 | S | Freeze P0/P1 release scope and establish a change-admission rule. New work must fix P0/P1, remove debt, improve evidence, or be separately approved. | Active plan; issue labels; review checklist | Scope ledger names owner, priority, dependency, acceptance, and deferral reason. |
| PCQ-004 | M | Add a clean-clone bootstrap check that configures, hydrates required assets/dependencies, builds, and discovers tests without workstation-only state. | tools/ci; CMake presets; asset hydration | A disposable clone reaches the same discovered test set. |
| PCQ-005 | M | Build a release evidence index mapping requirement IDs to commands, reports, screenshots, recordings, hardware, commit, and date. | docs/release; build/reports | Missing/stale evidence is machine-detectable. |
| PCQ-006 | S | Define the P0 defect bar: data loss, crash, unfinishable golden path, inaccessible required action, packaging corruption, security/privacy leak, or false readiness claim. | Issue/release policy | Release candidate creation fails while an accepted P0 remains open. |

### R0 exit criteria

- [ ] Current source configures and builds in the canonical Debug tree.
- [ ] PFU-I1 reaches execution rather than failing on stale or uncompilable code.
- [ ] Creator-journey evidence records the current commit and binary provenance.
- [ ] Every release-critical deferral has an owner and an explicit disposition.
- [ ] Scope freeze and defect severity rules are accepted.

## 9. R1 — Project Transactions, References, History, and Migrations

**Outcome:** Large projects can evolve without broken references or lost work.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-100 | L | Inventory every authoritative document owner, dirty-state path, history stack, atomic save path, and recovery path. Publish gaps by document type. | editor/project; engine/core/project; domain models | Machine-readable ownership matrix and tests for each claimed owner. |
| PCQ-101 | XL | Introduce a project-operation coordinator that composes typed domain commands across documents, validates source revisions, commits atomically, and records inverse operations. Reuse existing history patterns. | project/session and existing domain command owners | Cross-document operation either fully commits or leaves no partial mutation. |
| PCQ-102 | XL | Create an incremental stable-ID reference index for maps, events, switches, variables, assets, dialogue, quests, menus, database records, recipes, plugins/mods, and package inclusion. | engine/core/project or dedicated native reference owner; editor impact UI | Reference graph rebuild and incremental update produce identical results. |
| PCQ-103 | L | Implement “find uses” and “why included” queries with navigation to source object and reference type. | editor context actions; reference index | Every golden-slice object type can be located from at least one inbound and outbound edge. |
| PCQ-104 | XL | Implement previewable rename, move, delete, replace, and relink plans. Show affected references, blocked updates, package impact, and inverse. | domain commands; reference index; diagnostics | Rename/delete/replace across maps, events, assets, and data survives save/reopen/playtest and undo. |
| PCQ-105 | L | Unify project-wide undo/redo boundaries and labels. Preserve document-local performance while grouping one user intent into one operation. | editor history; document histories | A multi-document operation is one understandable undo step and one redo step. |
| PCQ-106 | L | Add crash-safe operation journaling or snapshots at acknowledged commit boundaries, including interrupted save simulation. | persistence; recovery | Fault-injection tests restore the last acknowledged state with no half-applied reference update. |
| PCQ-107 | XL | Establish forward-only schema migration contracts: version discovery, preflight, backup, dry run, diagnostics, idempotence, rollback boundary, and fixture corpus. | schema registry; document serializers; tools/ci | Oldest supported fixtures migrate to current, re-open, re-save, and run without silent loss. |
| PCQ-108 | M | Detect external file changes and offer compare/reload/keep-local flows without overwriting unsaved work. | project session; file watchers; recovery | Conflict tests cover unchanged, dirty, renamed, deleted, and malformed external files. |

### R1 exit criteria

- [ ] All primary document types have explicit owner, persistence, history, migration,
  reference, and recovery status.
- [ ] Project-wide rename/move/delete/replace is safe for the vertical-slice types.
- [ ] Interrupted operations and external changes cannot silently discard acknowledged work.
- [ ] Package closure is explainable from the same reference graph.

## 10. R2 — URPG Design System and Editor Shell

**Outcome:** The editor feels intentionally designed, scales correctly, and teaches
creators how to proceed.

### 10.1 Visual direction

Adopt an original “playful workshop” direction: warm neutral work surfaces, vivid but
limited tool-category accents, rounded cards only where they clarify grouping, strong
typographic hierarchy, friendly original icons, generous focus rings, and restrained
motion. The content remains the hero. Avoid a red-and-white Nintendo imitation, console
home-menu mimicry, branded sounds, or recognizable proprietary iconography.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-200 | L | Define tokens for color roles, typography, spacing, corner radius, borders, elevation, icons, motion, sound, focus, density, and scale. Include light/dark/high-contrast themes. | editor UI foundation; settings | Token gallery and snapshot coverage; no primary route uses unexplained raw style constants. |
| PCQ-201 | L | Build shared widgets for primary/secondary/destructive buttons, segmented controls, cards, fields, pickers, trees, tabs, tables, toasts, banners, progress jobs, empty states, diagnostics, confirmations, and command previews. | editor UI components | State matrix demonstrates normal/hover/focus/pressed/disabled/loading/error at supported scales. |
| PCQ-202 | L | Apply persisted UI scale to the entire editor, fonts, icons, hit targets, popups, canvases, and fixed-size windows. Replace fixed geometry with min/preferred/resizable constraints. | apps/editor; editor panels; settings | Route-by-scale snapshot suite and manual review at minimum, default, 150%, 200%, and supported maximum. |
| PCQ-203 | L | Redesign startup/project selection into a guided shell: recent projects, create/open/import, recovery, health, templates, examples, and platform/tool availability. | apps/editor; main menu; project services | A first-time user creates and reopens a project without raw path entry. |
| PCQ-204 | M | Make native system pickers a supported capability with a clear embedded fallback. Remove dead “unavailable” affordances from qualified builds. | platform services; project/assets UI | Picker works on each claimed desktop platform; cancellation returns safely. |
| PCQ-205 | L | Establish global workspace layout: project navigator, context toolbar, central canvas, inspector, status/jobs, diagnostics, and playtest controls. Preserve user layouts with reset/recovery. | editor shell/docking | Primary routes share consistent regions; corrupted layout resets without losing project data. |
| PCQ-206 | M | Add command palette, global search, recent actions, shortcut discovery, and contextual help. | shell; reference index; command registry | Golden-loop actions are searchable and expose shortcuts/requirements. |
| PCQ-207 | L | Define focus order, keyboard semantics, controller navigation, canvas alternatives, accessible names, and announcements for shared widgets. | shared widgets; input; accessibility | Automated widget audit plus manual keyboard/controller/screen-reader review on supported OS. |
| PCQ-208 | M | Add onboarding that uses a sample project and dismissible contextual coaching, not modal tours. Persist completion and allow replay. | creator checklist; templates; help | New-user sessions complete first map/event/playtest without facilitator intervention. |
| PCQ-209 | M | Standardize errors: plain summary, affected object, consequence, suggested fix, “go to,” retry, copy details, and redacted support export. | diagnostics; shared widgets | Error corpus review contains no raw-only exception or dead-end message. |

### R2 exit criteria

- [ ] The stock ImGui look is replaced by an original, documented URPG system.
- [ ] Primary routes are responsive and usable throughout the supported scale range.
- [ ] All golden-loop actions are reachable without typing filesystem paths.
- [ ] Keyboard navigation and visible focus pass for the shared widget gallery.
- [ ] New-user onboarding has measured usability evidence.

## 11. R3A — Governed Asset Lifecycle

**Outcome:** Assets can be discovered, transformed, attached, replaced, repaired, and
packaged without blocking or breaking the project.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-300 | L | Create one durable background-job contract for scans, imports, hashing, thumbnails, transforms, audio analysis, and packaging. Include stage, progress, cancel, resume/retry, logs, and recovery. | engine/core/assets; tools/assets; editor jobs | Cancellation and restart tests at every stage; no UI freeze on large fixtures. |
| PCQ-301 | L | Add collections, favorites, saved searches, richer filters, comparison, usage count, provenance, license, and package status. | catalog database; asset browser | 100,000-row fixture remains responsive within a recorded regression budget. |
| PCQ-302 | L | Finish relink, detach, replace, deduplicate, rename, move, and delete using the project-operation/reference systems. | attachment; catalog; project commands | Impact preview, undo, save/reopen, playtest, and package tests. |
| PCQ-303 | L | Complete non-destructive image revisions: crop, scale, palette, slice, atlas, spritesheet metadata, tileset rules, and retained source provenance. | sprite pipeline; transform manifests | Pixel output and metadata are reproducible from source hash and revision manifest. |
| PCQ-304 | L | Complete audio QA: waveform/spectrogram preview, trim/fade/gain revisions, loudness/peak/loop diagnostics, locale/take/rights/caption metadata. | audio tools/core/editor | Golden audio corpus covers clipping, bad loops, silence, missing captions, and muted alternative. |
| PCQ-305 | M | Add “use here” actions for map tile/prop/event sprite, battle asset, menu image, portrait, animation, and audio targets. | contextual panels; typed drag payloads | Every golden-slice target accepts picker, drag/drop, keyboard, and undo paths. |
| PCQ-306 | M | Add archive safety and custody rules: traversal protection, size/count limits, unsupported format diagnostics, external-source policy, and portable project audit. | asset archive/import; release validator | Adversarial archive and missing-source fixtures pass. |
| PCQ-307 | M | Generate third-party notices/BOM rows from governed provenance and block packages with unresolved required rights metadata under configured policy. | asset metadata; release assistant | Package contains deterministic, reviewed notices report. |

### R3A exit criteria

- [ ] Long asset work is cancellable and recoverable.
- [ ] No primary asset operation requires manual project-file edits.
- [ ] Replace/relink/delete is reference-aware and undoable.
- [ ] Image/audio assets have creator-visible quality checks and package provenance.

## 12. R3B — Map and World Authoring

**Outcome:** Creators can build interconnected RPG spaces quickly and safely.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-350 | L | Make Map the canonical spatial workspace. Remove route duplication and preserve context when entering tools from project, object, asset, diagnostic, and playtest surfaces. | Map workspace; panel registry; routing | Route-equivalence tests and manual navigation recording. |
| PCQ-351 | XL | Add multi-select, box/lasso selection, copy/cut/paste, duplicate, move, fill, line, rectangle, replace, stamp, layer visibility/lock, and batch property edit. | map document commands; history | Each operation supports preview where destructive, undo/redo, save/reopen, and runtime round trip. |
| PCQ-352 | L | Add reusable stamps/prefabs with stable IDs, versioning, instance overrides, update preview, detach, and package closure. | grid-part/prefab owners; reference graph | Updating a prefab reports and applies affected instances without overwriting overrides. |
| PCQ-353 | L | Complete tile/terrain productivity: autotile diagnostics, collision overlay, passability paint, terrain tags, region IDs, navigation preview, and validation. | tiles/map/navigation | Bad seams, unreachable entrances, invalid collisions, and missing materials are object-linked. |
| PCQ-354 | XL | Build a project world graph of maps, entrances, exits, transfers, checkpoints, spawn points, conditional routes, and orphan maps. | world travel graph; events; project graph UI | Creator can author and playtest a two-map transfer loop entirely from the UI. |
| PCQ-355 | M | Add “play from here,” “teleport here,” “return to editor selection,” and diagnostic-to-cell focus. | playtest controller; runtime/editor bridge | Runtime and editor preserve map/object identity through focus transitions. |
| PCQ-356 | M | Add minimap/world preview and dependency/impact panels for transfer changes. | map/world panels | Deleting or renaming an entrance shows every affected path. |
| PCQ-357 | M | Stress test large maps, many layers, many events, undo memory, and spatial snapshots. | performance fixtures; spatial tests | Recorded budgets and graceful warnings replace uncontrolled degradation. |

### R3B exit criteria

- [ ] A two-map quest loop with transfers and checkpoints is authored without raw IDs.
- [ ] Common painting/editing operations are batch-capable and undoable.
- [ ] Navigation, collision, and orphan world routes are inspectable before playtest.
- [ ] Playtest can start from and return to the current spatial context.

## 13. R4A — Event, Narrative, and Quest Authoring

**Outcome:** The editor supports the control flow needed for a real small RPG and makes
logic understandable.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-400 | XL | Close native event-command depth for conditions, branches, loops, breaks, waits/timers, parallel work, switch/variable/self-switch operations, movement routes, camera, animation, audio, common events, transfer, battle/shop, and controlled script/extension calls. | native event schema/compiler/runtime/editor | Command matrix maps authoring UI to serialization, runtime effect, diagnostics, undo, and tests. |
| PCQ-401 | L | Add structured command insertion/search, favorites/recent, templates, copy/paste, indent visualization, collapse, reorder, multi-edit, and keyboard operations. | event editor | A long event remains navigable and edits do not corrupt control-flow nesting. |
| PCQ-402 | L | Add stable reference pickers for maps, entries, events, actors, items, skills, switches, variables, common events, audio, animations, quests, and dialogue. | reference graph; shared pickers | No golden-loop event command requires a memorized numeric ID. |
| PCQ-403 | L | Add static analysis for unreachable branches, missing references, infinite/tight loops, unsafe parallel mutation, transfer dead ends, localization gaps, and likely softlocks. | event analyzer; diagnostics | Curated bad-event corpus produces actionable diagnostics with low false-positive review. |
| PCQ-404 | XL | Complete visual dialogue and quest editing: node create/connect/reorder, conditions, choices, outcomes, objectives, rewards, voice/caption/localization IDs, reachability, dead ends, and migration. | dialogue/quest models and panels | A branching voiced quest round-trips through editor, runtime, save/load, and package. |
| PCQ-405 | M | Add reusable common-event and narrative templates with parameter validation and find-uses. | common events; template service | Template update and deletion show impact and preserve instance overrides. |
| PCQ-406 | M | Add runtime event trace: current command, stack, branches, waits, parallel lanes, switches/variables changed, and source navigation. | runtime diagnostics; editor bridge | A failing quest can be followed from symptom to source event. |
| PCQ-407 | M | Preserve MZ compatibility as a separate import/execution matrix with explicit unsupported fallbacks. Do not bend native schemas around plugin-specific behavior. | runtimes/compat_js; migration | Native and compat command coverage are separately reported. |

### R4A exit criteria

- [ ] The vertical slice requires no unsupported native event workaround.
- [ ] Complex control flow is authorable, inspectable, and traceable.
- [ ] Reference pickers eliminate raw-ID entry on the golden path.
- [ ] Dialogue/quest localization and voice/caption links survive package and load.

## 14. R4B — Database, Gameplay, and Balance Productivity

**Outcome:** Creators can manage and tune game data at project scale.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-450 | L | Add virtualized table views for actors, classes, skills, items, equipment, enemies, encounters, states, quests, vendors, recipes, switches, and variables. | database/editor models | Sort/filter/search/select/edit remains responsive on large fixtures. |
| PCQ-451 | L | Add multi-row edit, fill, formulas, curve tools, duplicate, import/export, validation preview, and undoable batch changes. | domain commands; table UI | Batch mutation is atomic and reports every invalid row before apply. |
| PCQ-452 | M | Add stable reference pickers, find-uses, impact preview, orphan detection, and safe delete/replace to every golden-slice record. | reference graph; domain editors | No dangling reference after tested rename/delete/replace operations. |
| PCQ-453 | L | Expand progression/economy simulation with scenarios, seeded batches, personas/strategies, sensitivity, level curves, reward cadence, dominant strategy, grind, deadlock, and source navigation. | balance/economy tools | Simulation records assumptions/seed/version and links findings to exact parameters. |
| PCQ-454 | M | Add comparison/history views for balance revisions and export review reports without creating a second data authority. | history; reports | Creator can compare two committed revisions and restore through normal commands. |
| PCQ-455 | L | Prove integration between database records and battle, inventory, equipment, quests, vendors, crafting, saves, localization, and package closure. | runtime subsystems; integration tests | Vertical slice exercises each selected integration in a fresh save and migrated save. |

### R4B exit criteria

- [ ] Large data sets are manageable without repetitive form-by-form editing.
- [ ] Bulk changes are validated, atomic, and undoable.
- [ ] Simulations are reproducible and navigate back to authoritative data.
- [ ] Runtime/save/package consumers use the same records proved in the editor.

## 15. R4C — Menu and Player-UI Authoring

**Outcome:** Generated games can have polished, consistent, adaptable interfaces without
hand-editing code.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-480 | XL | Turn the menu scene graph into a real canvas editor with hierarchy, selection, transform/size tools, anchors, constraints, guides, alignment, distribution, snapping, safe areas, and zoom. | menu model/editor/runtime | Canvas edits persist, migrate, undo, and match runtime output at target viewports. |
| PCQ-481 | L | Add reusable components, style tokens, variants, templates, slots, instance overrides, and controlled updates. | menu schema; component library | Component change previews affected instances and preserves overrides. |
| PCQ-482 | L | Add states and transitions: default/focus/pressed/disabled/selected/loading/error, entrance/exit, interruption, reduced motion, and audio feedback hooks. | menu runtime; animation/audio | State matrix passes keyboard/controller/touch-like navigation simulations where declared. |
| PCQ-483 | L | Add data binding with typed sources, fallback values, localization, formatting, validation, and preview fixtures. | menu bindings; runtime | Missing data/localization fails visibly in editor, not silently in package. |
| PCQ-484 | M | Add focus-flow visualization/linting, controller glyph preview, safe-area overlays, text-overflow stress modes, and target-resolution presets. | menu editor; input; localization | No focus trap or obscured required action in supported target matrix. |
| PCQ-485 | M | Ship original URPG starter templates for title, save/load, settings, pause, inventory, equipment, quest log, dialogue, shop, battle HUD, and results. | first-party templates; runtime | Every template is editable, package-safe, accessible, and visually reviewed. |

### R4C exit criteria

- [ ] A creator can build and customize the vertical-slice UI without code.
- [ ] Editor preview and runtime match for target resolutions and UI scales.
- [ ] Focus, localization, safe area, and reduced-motion diagnostics pass.
- [ ] First-party templates look cohesive but remain distinctly URPG.

## 16. R5 — Playtest, Debugging, Recovery, and Support

**Outcome:** The create-test-diagnose-fix loop is fast enough for daily use and safe
enough for nontechnical creators.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-500 | L | Finish playtest blocker policy, checkpoints, disposable state, selected-map/object launch, and return-to-editor context. | playtest controller; runtime startup | Exact creator journey passes on current candidate. |
| PCQ-501 | XL | Add bounded hot reload by resource class. Negotiate capability, validate changes, preserve or reset state explicitly, and fall back to restart safely. | editor/runtime bridge; document owners | Matrix states hot-reloadable, restart-required, rejected, and failure recovery behavior. |
| PCQ-502 | L | Add switch/variable/self-switch/watch inspector, entity/quest/inventory state views, safe temporary edits, and reset to checkpoint. | runtime diagnostics; editor panels | Debug mutations are visually marked, disposable by default, and absent from packaged data. |
| PCQ-503 | L | Add event breakpoints, conditional breaks, pause/continue, step, frame advance where deterministic, call stack, and source focus. | event runtime; replay; editor bridge | Debugging one failing event produces a deterministic trace and returns to its command. |
| PCQ-504 | L | Integrate replay/scenario capture with semantic input, seed, project revision, runtime version, checkpoints, and redaction. | replay; playtest; support | A reported failure replays in headless/interactive mode or clearly reports divergence. |
| PCQ-505 | M | Add performance overlay and capture for frame time, render, script/event, asset streaming, audio, memory, and spikes. | profiler; runtime/editor | Capture links a visible hitch to a subsystem and timestamp. |
| PCQ-506 | L | Expand recovery to every primary document, asset jobs, external-change conflicts, corrupted layouts/settings, failed migrations, and failed packages. | recovery; project session; jobs | Fault-injection walkthrough covers each recovery class. |
| PCQ-507 | M | Create redacted support bundles with explicit preview: logs, diagnostics, versions, platform capabilities, project manifest hashes, replay, and optional selected project data. | diagnostics/support | Secret/path/PII corpus verifies redaction; nothing is uploaded automatically. |

### R5 exit criteria

- [ ] Add event → play from here → inspect → fix → reload/restart → return passes.
- [ ] Debug and replay tools operate on actual runtime state.
- [ ] Hot reload has explicit safe boundaries and never implies impossible preservation.
- [ ] Recovery and support flows have human-executed evidence.

## 17. R6A — Player-Facing Presentation and “Nintendo Feel”

**Outcome:** The default shipped experience feels joyful, readable, responsive, and
finished while remaining visibly URPG.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-600 | M | Define the runtime presentation bible: original palette, typography, iconography, spacing, motion, particles, sound motifs, camera language, feedback hierarchy, and accessibility variants. | design docs; runtime UI/audio/render owners | Approved style sheet and in-engine component showcase. |
| PCQ-601 | L | Polish startup/title/profile/save/settings/pause/quit flows with clear focus, immediate acknowledgement, transitions, error recovery, and controller glyphs. | runtime scenes; input; save | Controller-only start-to-game and save/load/quit paths pass. |
| PCQ-602 | L | Polish exploration feedback: interaction prompts, pickup/reward, doors/transfers, quest updates, dialogue cadence, screen transitions, camera, and contextual audio. | map/runtime UI/audio/animation | Frame-by-frame capture and sensory review against the presentation bible. |
| PCQ-603 | L | Polish combat feedback: selection, targeting, turn state, damage/heal/status, anticipation/impact/recovery, victory/defeat, result clarity, and skip/fast-forward rules. | battle; VFX; animation; audio; UI | Representative encounter remains readable with audio off, reduced motion, and color filters. |
| PCQ-604 | M | Add a consistent feedback stack: focus animation, confirm/cancel/error sounds, optional haptics, particles, screen shake limits, hit stop limits, and priority rules. | input/audio/render/settings | Feedback never delays control, stacks unboundedly, or bypasses user settings. |
| PCQ-605 | M | Replace placeholder/proof art and audio in the release vertical slice with licensed final-quality assets and complete credits. | content; asset governance | Asset audit reports no placeholder, missing provenance, or unresolved required credit. |
| PCQ-606 | M | Add first-run calibration for text, audio, display/safe area, input device, and accessibility shortcuts; keep it skippable and replayable. | runtime settings/startup | Fresh-profile tests and return-to-settings route pass. |
| PCQ-607 | M | Conduct holistic pacing review: time to control, menu depth, tutorial interruption, repeated confirmations, reward cadence, load masking, and idle waits. | vertical slice; UX evidence | Annotated playthrough lists and resolves every avoidable friction point. |

### 17.1 Presentation review questions

For every primary interaction, reviewers must answer:

1. What changed?
2. Why did it change?
3. What can the player do next?
4. Did input receive immediate acknowledgement?
5. Is the most important signal visually and audibly dominant?
6. Does the interaction still work with sound off?
7. Does it still work with reduced motion and non-color cues?
8. Can the player back out or recover safely?
9. Is the motion interruptible and the wait bounded?
10. Does this look and sound like URPG rather than borrowed brand language?

### R6A exit criteria

- [ ] Runtime shell, exploration, dialogue, combat, and results share one presentation language.
- [ ] Controller glyphs and feedback reflect the active device.
- [ ] Final vertical-slice assets are governed and credited.
- [ ] Sensory settings actually change every relevant feedback system.
- [ ] Holistic playthrough has no unexplained placeholder or dead-end state.

## 18. R6B — Accessibility, Localization, and Input

**Outcome:** Accessibility is part of the product architecture, not a final audit.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-650 | XL | Finish unified semantic input: device polling, hot-plug, ownership, contexts, axes, chords, deadzones, repeat, rebinding conflict resolution, calibration, glyphs, and recovery. | engine/core/input; editor/runtime input panels | Device matrix covers disconnect/reconnect, active-device changes, invalid bindings, and controller-only flow. |
| PCQ-651 | L | Add text/UI scale profiles, high contrast, color filters, non-color cues, reduced motion, screen shake, flash limits, subtitle/caption options, audio channel control, and mono alternatives where relevant. | settings; UI; render; audio | Settings persist, preview, reset, migrate, and affect all declared systems. |
| PCQ-652 | L | Complete localization model for stable keys, plurals, gender/grammar variants as required, locale fallback, font/glyph coverage, line breaking, text direction, RTL, IME, number/date formatting, and stale-key detection. | localization; fonts; text rendering | Locale corpus and pseudo-locales cover expansion, RTL, missing glyphs, and IME entry. |
| PCQ-653 | M | Add caption/voice alignment, speaker identity, non-speech cues, missing-alternative diagnostics, and locale/take selection. | audio/dialogue/localization | Muted playthrough retains required narrative/gameplay information. |
| PCQ-654 | L | Provide semantic alternatives for graph/canvas editors: tree/list representation, ordered navigation, properties, connection creation, and diagnostics. | menu/dialogue/quest/map editors | Required operations are possible without precise pointer use. |
| PCQ-655 | M | Add automated focus, contrast, touch-target/hit-target, clipping, missing label, localization overflow, and unsafe motion audits. | accessibility auditor; snapshots | Known-bad fixtures fail with object-linked diagnostics. |
| PCQ-656 | M | Run manual reviews with keyboard-only, controller-only, screen reader on supported desktop OS, 200% scale, reduced motion, high contrast, sound off, and representative locales. | UX evidence | Signed evidence names hardware/software, route, build, findings, and retest. |

### R6B exit criteria

- [ ] Required editor and runtime flows have no focus trap.
- [ ] Controller disconnect/reconnect and remapping are recoverable.
- [ ] Pseudo-localization, RTL/IME where claimed, and glyph coverage pass.
- [ ] Critical information has visual/non-color and audio-off alternatives.
- [ ] Manual accessibility evidence is current for the release candidate.

## 19. R7A — Performance, Reliability, Security, and Privacy

**Outcome:** The product remains predictable under real projects and adverse conditions.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-700 | M | Define representative tiny/medium/large project fixtures and target hardware classes. Baseline startup, project open, save, search, map edit, playtest launch, frame pacing, package, and memory. | benchmarks; tools/ci | Versioned benchmark report with regression thresholds. |
| PCQ-701 | L | Remove main-thread scans/blocking I/O from primary routes; use bounded jobs and incremental indexes. | editor; assets; project graph | Interaction latency traces meet provisional budgets. |
| PCQ-702 | L | Run long-session and stress scenarios: repeated open/close/playtest/package, large undo, asset churn, device churn, save cycles, and suspended/minimized windows. | integration/stress tests | No unbounded growth, corruption, deadlock, or unrecovered job. |
| PCQ-703 | L | Add fault injection for disk full, permission loss, interrupted write, malformed document, missing asset, bad archive, failed migration, renderer/device loss, and child-process failure. | persistence; assets; runtime; tools | Each failure has bounded behavior, diagnostic, and recovery path. |
| PCQ-704 | M | Complete sanitizer coverage where toolchains allow; document current blockers; add fuzzing for schemas, archives, event streams, and compatibility boundaries. | CI; parsers | Fresh sanitizer/fuzzer evidence or explicit current toolchain blocker with alternative coverage. |
| PCQ-705 | M | Audit external process invocation, archive extraction, path traversal, plugin/mod trust, secrets, network defaults, logs, and support bundles. | security review; CI guards | Threat model and adversarial fixtures; no secret in package/report. |
| PCQ-706 | M | Define privacy defaults: offline-first, no automatic upload, explicit preview/consent, retention, deletion, and telemetry opt-in if telemetry is introduced. | settings; support; release docs | Network capture of default session shows no unexpected outbound request. |
| PCQ-707 | M | Add dependency/SBOM/license review for binaries, tools, packaged assets, optional providers, and research outputs. | build/package; legal notices | Deterministic SBOM/notices and policy gate. |

### R7A exit criteria

- [ ] Representative performance baselines exist and regressions fail visibly.
- [ ] Stress/fault tests prove bounded recovery rather than happy-path success only.
- [ ] No default hidden network or data upload behavior exists.
- [ ] Package, support, and optional-tool outputs pass security/licensing review.

## 20. R7B — Packaging, Updates, and Platform Architecture

**Outcome:** Desktop releases are installable and maintainable, while console work has a
clean authorized seam.

| ID | Effort | Implementation | Primary owners | Required evidence |
| --- | --- | --- | --- | --- |
| PCQ-750 | L | Define PlatformServices capabilities for lifecycle, users, storage, input devices, display modes, achievements, presence, network status, virtual keyboard, locale, clock, power/suspend, and error presentation. | engine/core/platform; runtime/editor adapters | Headless and desktop fakes test available/unavailable/degraded capabilities. |
| PCQ-751 | L | Remove direct desktop assumptions from runtime-required code and report unsupported platform capabilities explicitly. | runtime; CMake; platform adapters | Static/build review and platform capability report. |
| PCQ-752 | L | Finish desktop package layout, dependency closure, signing/notarization hooks as applicable, install/uninstall, portable mode policy, save location, repair, and clean-machine smoke. | export packager; tools/ci | Signed-off clean VM install/play/save/relaunch/uninstall evidence per claimed OS. |
| PCQ-753 | L | Define versioning and update compatibility: project schema, save data, runtime, plugins/mods, packages, rollback boundary, and downgrade messaging. | migrations; packaging; release docs | N-1 upgrade fixtures and failed-update recovery pass. |
| PCQ-754 | M | Make target support truthful: qualified, experimental, unsupported, or unavailable. Keep Web unsupported until its own evidence passes. | export UI; validators; docs | UI, CLI, docs, and readiness records agree. |
| PCQ-755 | M | Build deterministic package manifests including content hashes, references, licenses, notices, platform capabilities, version, and reproducibility inputs. | packager; project graph | Two equivalent builds explain any hash difference or produce matching governed content. |
| PCQ-756 | M | Add release-channel and crash/support version metadata without embedding secrets or workstation paths. | version metadata; support | Installed build reports exact release provenance. |

### R7B exit criteria

- [ ] Claimed desktop packages install and run on clean machines.
- [ ] Save/update/migration behavior is explicit and tested.
- [ ] Platform-specific behavior uses capabilities instead of scattered compile-time assumptions.
- [ ] UI and docs do not expose unsupported targets as ready.

## 21. Nintendo Platform and Certification Lane

Nintendo’s public process supports native C++ development, but Nintendo Switch access
requires registration, a separate application, agreements, and adherence to private
production standards. Exact platform requirements are not public and must not be
invented in this plan.

Official public references:

- [Nintendo Developer Portal registration](https://developer.nintendo.com/register)
- [Nintendo developer and publishing process](https://developer.nintendo.com/the-process)

| ID | Effort | Implementation | Boundary | Required evidence |
| --- | --- | --- | --- | --- |
| NIN-000 | External | Confirm business owner, publishing intent, regions, target audience, support model, and budget. Register through the official portal. | Requires authorized company action. | Registration/application status held by the authorized owner. |
| NIN-001 | External | Request Nintendo Switch development access and execute required agreements/NDA. | Do not place confidential material in a public repository. | Access decision and authorized contacts. |
| NIN-002 | M | After access, create a restricted compliance matrix mapping each private requirement to owner, implementation, test, hardware/firmware, evidence, and submission status. | Restricted storage; only sanitized status enters public docs. | Confidential matrix reviewed by authorized team. |
| NIN-003 | L | Implement an authorized Nintendo PlatformServices adapter, build configuration, SDK boundary, and package path without leaking proprietary headers or details. | Requires approved SDK/dev environment. | Clean authorized build and capability tests. |
| NIN-004 | L | Qualify controller styles, pairing/ownership, disconnect/reconnect, glyphs, remapping policy, virtual keyboard, and multiplayer/device edge cases required by the private matrix. | Requirements come only from current authorized docs. | Hardware test matrix and video/log evidence. |
| NIN-005 | L | Qualify lifecycle: launch, user selection, suspend/resume, focus, power, docked/handheld changes, display changes, sleep, and interruption behavior. | Target-hardware testing required. | Repeated lifecycle stress evidence on supported configurations. |
| NIN-006 | L | Qualify storage/save behavior: user association, capacity/full media, corruption/interruption, migration/update, error presentation, and recovery. | Target and private requirements govern details. | Fault and recovery evidence; no data-loss P0. |
| NIN-007 | L | Optimize and qualify CPU/GPU/memory/I/O/frame pacing/load behavior on target hardware, including worst-case vertical-slice scenes. | Desktop results are not substitutes. | Target-hardware profiler captures and approved budgets. |
| NIN-008 | L | Complete ratings, legal, privacy, credits, store metadata/assets, languages, support, and publishing checklists. | Publisher/region dependent. | Owner-approved submission bundle. |
| NIN-009 | XL | Run pre-submission audits, submit, triage findings, fix, regression-test, and repeat until approved. | Findings remain confidential where required. | Platform-holder approval, not internal confidence, is the exit condition. |

### Nintendo quality gate

URPG may say “Nintendo-inspired quality target” internally or “controller-first,
console-quality polish” publicly when substantiated. It must not say “Nintendo-ready,”
“Switch-ready,” “certified,” or “approved” until the corresponding official process is
complete.

## 22. NVIDIA ARDy Evaluation

### 22.1 Decision: conditional pilot, not product dependency

ARDy can help URPG if the project deliberately approves a maintained 3D skeletal
animation lane. It cannot materially improve the current 2D/2.5D creator loop by
itself.

ARDY is NVIDIA research for interactive human-motion generation from text and
long-horizon kinematic constraints such as root paths, waypoints, full-body keyframes,
and sparse joint positions/rotations. The official implementation is young,
workstation-oriented research code. The published model is a 326M-parameter,
27-joint, 20 FPS model with an eight-second maximum output/history window. The
repository documents Linux, recent PyTorch/CUDA, an NVIDIA GPU, a compiled extension,
optional TensorRT, and a gated Llama text encoder.

Official sources:

- [NVIDIA Research ARDy project page](https://research.nvidia.com/labs/sil/projects/ardy/)
- [Official NVIDIA ARDy repository](https://github.com/nv-tlabs/ardy)
- [Official ARDy model card](https://huggingface.co/nvidia/ARDY-Core-RP-20FPS-Horizon40)
- [NVIDIA Open Model Agreement](https://www.nvidia.com/en-us/agreements/enterprise-software/nvidia-open-model-agreement/)

### 22.2 Fit matrix

| Question | Assessment |
| --- | --- |
| Can it generate useful humanoid motion references? | Yes. Text plus spatial/kinematic constraints could speed locomotion, blocking, and cinematic prototyping. |
| Can it plug into the current AnimationClip? | No. The current general clip is scalar-property tracks; it is not a skeletal clip, skin, rig, retargeter, or mesh animation runtime. |
| Does faux-3D dungeon support make ARDy a natural fit? | No. That lane is intentionally raycast/faux-3D and explicitly excludes a full skeletal 3D engine. |
| Could it help 2D games? | Possibly by producing motion reference or pre-rendered sprite frames, but the conversion and cleanup cost may outweigh the benefit. |
| Should it run in the editor/runtime? | No for 1.0. Keep it in a supervised external helper environment and import baked outputs. |
| Should shipped games include it? | No. Ship only reviewed native animation/sprite artifacts and provenance. |
| Is it deterministic enough for the runtime? | Runtime determinism is preserved by baking and hashing accepted output; generation itself need not occur at runtime. |
| Is licensing trivial? | No. Code and model artifacts have different terms; attribution/NOTICE, model agreement, gated Llama access, dependencies, and generated-asset rights need review. |
| Is it a release blocker? | No. It is an optional post-foundation research lane. |

### 22.3 Admission gates

Do not begin implementation until all are true:

- [ ] Product leadership approves a real skeletal 3D animation use case, not only curiosity.
- [ ] A neutral native skeletal clip/skeleton profile is already needed by a non-ARDy
  workflow.
- [ ] URPG has or has approved a mesh/skin/skeleton import, preview, retarget, and baked
  runtime path.
- [ ] A suitable Linux/NVIDIA workstation and maintenance owner exist.
- [ ] Legal review accepts the code, checkpoint, Llama, dependency, attribution, and
  output-use terms for the intended distribution.
- [ ] The pilot has a measurable creator-time/quality hypothesis.
- [ ] ARDy remains optional and its absence does not disable normal authoring.

If these gates are not met, record ARDy as deferred and spend the effort on sprite,
timeline, event, and presentation workflows that directly support the current product.

### 22.4 Proposed isolated pipeline

~~~mermaid
flowchart LR
    Prompt["Prompt and kinematic constraints"]
    Sandbox["External ARDy workstation sandbox"]
    NPZ["ARDy NPZ output plus provenance"]
    Convert["URPG experimental converter"]
    Review["Retarget, preview, and human QA"]
    Artifact["Baked native/glTF motion artifact"]
    Catalog["Governed asset catalog"]
    Runtime["Runtime consumes baked artifact only"]

    Prompt --> Sandbox
    Sandbox --> NPZ
    NPZ --> Convert
    Convert --> Review
    Review --> Artifact
    Artifact --> Catalog
    Catalog --> Runtime
~~~

### 22.5 Research work packets

| ID | Effort | Implementation | Required evidence |
| --- | --- | --- | --- |
| ARDY-000 | S | Write a one-page use-case admission: target character, rig, required motions, target output, current manual time, expected value, and why existing animation tools are insufficient. | Approved use-case statement or explicit deferral. |
| ARDY-001 | M | Perform code/model/dependency legal review. Distinguish Apache-2.0 repository code from model checkpoint terms and gated Llama access. Define required NOTICE/provenance. | Signed review and redistribution boundary. |
| ARDY-002 | M | Create a separate reproducible helper environment under a research/tooling boundary or external workstation. Pin OS, Python, CUDA, PyTorch, dependencies, commit, checkpoints, and hashes. Store no tokens, weights, vendor SDKs, or workstation paths in Git. | Environment manifest and secret scan. |
| ARDY-003 | L | Define MotionClipDocument only if the general 3D lane is approved. Include schema version, skeleton profile, joint hierarchy, FPS, duration, root transforms, local/global rotations, positions, contacts/events, coordinate system, units, constraints, source hashes, model/checkpoint, prompt hash/text policy, seed, and license metadata. | Schema fixtures, validation, migration, and round-trip tests. |
| ARDY-004 | L | Implement a converter for the documented NPZ fields into the neutral document, then an explicit retarget/bake step. Never teach the runtime to read ARDy’s environment directly. | Golden NPZ fixtures and deterministic baked artifact hashes. |
| ARDY-005 | L | Add skeletal preview only inside the approved 3D lane: rig mapping, joint mismatch diagnostics, coordinate/unit conversion, root motion, foot contacts, looping, crop, and comparison to constraints. | Visual fixtures and reviewed retarget report. |
| ARDY-006 | M | Add human review: constraint adherence, foot sliding, penetration, joint limits, discontinuity, contact timing, style fit, loop seam, and content appropriateness. Reject rather than silently repair unsafe output. | Structured review report attached to promoted asset. |
| ARDY-007 | M | Pilot three bounded cases: waypoint locomotion, keyframed full-body transition, and sparse hand/foot target. Compare creator time and accepted quality against the existing manual workflow. | Blind comparison where practical, timings, cleanup effort, pass/reject reasons. |
| ARDY-008 | S | Decide adopt/defer/reject. Adoption requires at least 30% median creator-time reduction on the pilot, acceptable output after bounded cleanup, zero runtime inference dependency, reproducible provenance, and a maintenance owner. | Recorded decision and next bounded scope. |
| ARDY-009 | M | Optional 2D experiment: render one accepted motion to sprite reference/frames and measure cleanup/time against native sprite/timeline work. Do not productize without positive measured ROI. | Side-by-side sprite workflow report. |

### 22.6 ARDy package and security rules

- Do not package Python, CUDA, PyTorch, TensorRT, Llama, ARDy weights, Hugging Face
  tokens, or the research server with URPG games.
- Do not make editor startup, normal asset import, or project load depend on ARDy.
- Bind any local research service to loopback, require explicit launch, and expose a
  narrow typed job protocol.
- Treat prompts and source motion as project data; preview exactly what is sent to any
  external service.
- Record code commit, model/checkpoint hash, dependency lock, prompt/constraint
  provenance, seed, converter version, retarget profile, human reviewer, and output
  content hash.
- Promote only reviewed baked artifacts through the normal asset catalog.
- Re-run license review before distributing any tooling bundle or generated content
  under materially different terms.

## 23. R8 — Creator-Authored Vertical Slice

**Outcome:** One polished project proves the product, not just its subsystems.

Use the existing designated vertical slice, such as Lantern of Willow if it remains
the canonical fixture. Do not replace a difficult requirement with hand-authored files.

### 23.1 Required content

- Two interconnected maps with entrances, transfer, checkpoint, collision, passability,
  and a discoverable optional area.
- A branching quest using dialogue, conditions, switches/variables, item reward,
  battle or challenge, success/failure states, and save/load continuity.
- At least one vendor/inventory/equipment or crafting interaction.
- A representative combat encounter with readable feedback and balance evidence.
- Title, settings, save/load, pause, inventory, quest, dialogue, and battle/results UI.
- Final governed art, animation, sound, music, captions, localization keys, and credits.
- Keyboard and controller paths, remapping, disconnect recovery, text scaling, high
  contrast, reduced motion, audio-off information, and pseudo-localization.
- Project rename/replace/relink operation, undo, recovery, and migration fixture.

### 23.2 Exact creator journey

1. Start from a clean installed editor and fresh profile.
2. Create the project from a qualified template.
3. Import/promote/attach governed assets through visible UI.
4. Build/edit maps with batch tools and world transfers.
5. Create events, dialogue, quest, gameplay data, and UI through native editors.
6. Save, close, reopen, and verify the creator checklist.
7. Play from the selected map/object.
8. Trigger a deliberate logic defect, inspect it, navigate to source, fix it, and
   reload/restart safely.
9. Create and undo a cross-document rename or asset replace.
10. Simulate an interrupted save/job and recover.
11. Run accessibility/localization/performance/project audits.
12. Package the project using the release UI.
13. Install on a clean machine/profile, start with controller, play through, save,
    quit, relaunch, load, finish, and inspect credits.
14. Export the redacted evidence/support bundle and verify its preview.

### 23.3 Evidence bundle

| Evidence | Requirement |
| --- | --- |
| Build manifest | Commit, dirty state, toolchain, presets, binary hashes/times. |
| Creator recording | Uncut or clearly indexed journey with no hidden file/script edits. |
| Runtime recording | Controller-first full playthrough with active glyph changes. |
| Screenshots | Primary routes at default/200% scale, target resolutions, themes, pseudo-locale, high contrast. |
| Accessibility report | Keyboard/controller/screen-reader/scale/motion/audio-off findings and retest. |
| Performance report | Target hardware, scene, settings, p50/p95/p99 frame/load/interaction data and traces. |
| Recovery report | Fault injected, expected boundary, recovered data, diagnostic, retest. |
| Package report | Validation, content closure, notices/SBOM, install/save/relaunch/uninstall. |
| Defect ledger | All P0/P1 findings with disposition and regression evidence. |
| Truth update | Readiness JSON, release matrices, program status, known debt, and active plan agree. |

### R8 exit criteria

- [ ] A non-implementing creator completes the exact journey without source/file edits.
- [ ] Package installs and completes on a clean qualified desktop target.
- [ ] Every evidence item identifies the same commit/candidate.
- [ ] No P0 remains and every P1 has accepted disposition.

## 24. R9 — External Usability, Beta, and Release Candidate

**Outcome:** The product survives creators who did not build it.

| ID | Effort | Implementation | Required evidence |
| --- | --- | --- | --- |
| PCQ-900 | M | Recruit representative novice, intermediate, accessibility, and experienced RPG-tool users. Use consented sessions and non-leading tasks. | Participant profile and privacy-safe research plan. |
| PCQ-901 | M | Test create/open/import, asset use, map/event/quest/UI authoring, playtest/debug, recovery, package, and reopen. Measure completion, errors, assistance, time, abandonment, and confidence. | Session notes/recordings and severity-ranked findings. |
| PCQ-902 | L | Resolve P0/P1 usability findings, retest with new users, and add automated protection for reproducible failures. | Before/after evidence and regression tests. |
| PCQ-903 | M | Run a bounded beta with opt-in diagnostics/support, migration policy, backup instructions, known issues, support response owner, and rollback plan. | Beta cohort report and crash/data-loss ledger. |
| PCQ-904 | M | Freeze the release candidate; allow only approved blocker fixes; rerun focused and full gates after every change. | Candidate ledger with change/retest chain. |
| PCQ-905 | M | Perform final clean-clone, clean-machine, package, upgrade, save migration, accessibility, performance, security, license, and truth audits. | Final evidence index contains no stale/missing required row. |
| PCQ-906 | S | Hold an explicit release decision: ship, delay, or reduce claimed scope. Record risks, unsupported targets, known issues, and owners. | Signed decision and aligned public claims. |

### R9 exit criteria

- [ ] New users complete the golden journey at the accepted success threshold.
- [ ] No unresolved P0 data-loss, crash, packaging, accessibility, or unfinishable-flow defect.
- [ ] Release claims match the exact qualified targets and evidence.
- [ ] Support, migration, rollback, and known-issue plans are operational.

## 25. Verification Matrix

Run the narrowest command while implementing, then the complete applicable set for the
release candidate. Use exact commands from docs/agent/QUALITY_GATES.md when that file
changes.

| Surface | Required command/evidence |
| --- | --- |
| Configure/build | .\tools\ci\configure_dev_ninja_debug.ps1 then cmake --build --preset dev-debug |
| General PR | ctest --preset dev-pr --output-on-failure |
| Creator journey | ctest --test-dir build/dev-ninja-debug -R "creator journey" --output-on-failure then .\tools\ci\check_creator_journey.ps1 -BuildDirectory build/dev-ninja-debug |
| Creator shell/session | ctest --preset dev-all -R "ProjectCreationService|NewProjectWizard|project template generator|CreatorChecklist|Runtime project preflight|Startup|MainMenu|EditorProjectSession|EditorDirtyStateRegistry|settings|editor app panels" --output-on-failure |
| Assets | Python asset unit suites and focused native asset tags specified in QUALITY_GATES.md |
| Map/spatial | ctest --preset dev-spatial --output-on-failure plus focused [spatial][map_authoring] coverage and route-equivalence evidence |
| Runtime input/title/menu | ctest --preset dev-all -R "startup|settings|input|SceneManager|RuntimeTitleScene" --output-on-failure |
| Snapshot/presentation | ctest --preset dev-snapshot --output-on-failure then .\tools\ci\run_presentation_gate.ps1 |
| Export | ctest --preset dev-export --output-on-failure |
| Persistence/migration | ctest --preset dev-all -R "settings|persistence|save|load|grid_part|Ability" --output-on-failure plus the migration fixture corpus introduced by PCQ-107 |
| Release assets | .\tools\ci\check_release_required_assets.ps1; .\tools\ci\check_promoted_asset_library.ps1; .\tools\ci\check_lfs_release_scope.ps1 |
| Repository truth | .\tools\ci\check_no_generated_tracked_files.ps1; .\tools\ci\check_no_production_system_calls.ps1; .\tools\ci\check_lfs_release_scope.ps1 |
| Package smoke | .\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke |
| Full local gate | .\tools\ci\run_local_gates.ps1 |
| Release-plan knowledge health | .\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug |
| Manual product evidence | Creator journey, controller matrix, accessibility matrix, target-resolution/scale review, performance captures, recovery drills, clean install/play/save/relaunch |

### 25.1 Evidence rules

- Every report includes commit, dirty state, binary provenance, command, result, date,
  OS/toolchain, and hardware where relevant.
- A pass from an older binary does not qualify newer source.
- “Not run,” “deferred,” and “unavailable” are not passes.
- Manual evidence names the route and acceptance requirement; screenshots alone do not
  prove interaction.
- Platform-holder approval cannot be replaced by an internal advisory checker.
- ARDy demo success cannot qualify the native runtime or a shipping package.

## 26. Release Blockers

The product does not ship while any of these are true:

- Current source does not configure/build through the canonical path.
- Golden creator journey requires source edits, raw project-file edits, or developer
  scripts.
- A primary destructive operation can silently break references or lose acknowledged
  work.
- Package validation/install/save/load/relaunch fails on a claimed target.
- A required keyboard/controller route has an unreachable action or focus trap.
- Required information is unavailable with supported accessibility settings.
- A known P0 crash, data-loss, corruption, security/privacy leak, or unfinishable game
  path remains.
- Final vertical-slice content contains unresolved placeholder, provenance, license,
  credit, missing-glyph, or missing-caption failures.
- Readiness, docs, UI, and package claims disagree.
- Nintendo or another platform is claimed without official access and approval.

## 27. Deliberate Post-1.0 Deferrals

These items require separate admission after the golden product loop is proven:

- General-purpose arbitrary 3D mesh/terrain/physics/skeletal game authoring.
- Productized ARDy integration beyond the bounded approved pilot.
- Broad autonomous playtest agents.
- Multimodal asset/style intelligence.
- Branchable collaborative editing and cloud synchronization.
- Marketplace/economy/community publishing platform.
- Expanded AI providers or intent-to-project breadth that bypasses typed native commands.
- Broad MZ plugin parity beyond explicit compatibility priorities.
- Additional export targets without hardware, packaging, support, and qualification owners.

## 28. Recommended First Ten Work Packets

Execute in this order unless a newly discovered P0 changes the dependency:

1. PCQ-000 — repair the current compile blocker and add regression coverage.
2. PCQ-001 — reject stale binary evidence with build provenance.
3. PCQ-002 — run PFU-I1 and replace deferred verification with current truth.
4. PCQ-003 — freeze release scope and establish the P0/P1 defect bar.
5. PCQ-100 — inventory document/history/reference/recovery ownership.
6. PCQ-102 — establish the stable-ID reference index contract.
7. PCQ-101 — establish cross-document operation/transaction semantics.
8. PCQ-200 — define the URPG visual and interaction token system.
9. PCQ-202 — fix editor-wide scaling and responsive layout.
10. PCQ-500 — complete and prove the exact create/add-event/playtest/return loop.

ARDY-000 may run as a low-cost paper exercise after PCQ-003. No ARDy code integration
should start before all admission gates in Section 22.3 pass.

## 29. Final Release Definition

URPG is ready for its first complete-product claim when:

- The entire creator journey is achievable through the native UI by a representative
  external creator.
- The result is a coherent, polished, accessible game rather than a technical fixture.
- Project evolution is protected by stable references, preview, undo, atomic save,
  migration, and recovery.
- Editor and runtime meet recorded interaction, performance, controller, localization,
  accessibility, and presentation budgets.
- Qualified desktop packages install, save, update/migrate, reload, and uninstall on
  clean systems.
- Current automated and manual evidence identifies the exact shipped candidate.
- Public claims distinguish qualified, experimental, deferred, and unsupported scope.
- Nintendo platform language is used only after official authorization and approval.
- Optional ARDy-produced content, if any, is reviewed, licensed, baked, governed, and
  completely absent from the shipping runtime dependency graph.

That quality bar—not the count of panels, subsystems, or prototype tests—is what will
make URPG feel like a product a platform holder could confidently put in front of its
audience.
