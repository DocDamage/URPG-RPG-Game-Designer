# URPG Product Feature and Usability Native Absorption Plan

> **Status:** Active corrective child plan; planning and traceability only until a task is implemented and evidenced.
>
> **Repository:** `G:\URPG Maker-development`
>
> **Parent execution plan:** `docs/superpowers/plans/2026-07-12-creator-product-cohesion-plan.md`
>
> **Source intake:** `docs/external-intake/PRODUCT_FEATURE_USABILITY_ROADMAP.md`
>
> **Last audited:** 2026-07-14

## Goal

Absorb the useful product outcomes from the RPG Maker MZ/Node Creator Hub prototype into the existing native URPG editor and runtime without importing its application architecture, creating a second project authority, or overstating the readiness of bounded native subsystems.

The active creator-product cohesion plan remains first in execution order. This plan preserves the source feature IDs `F01`-`F24` and improvement IDs `I01`-`I10` for traceability, but uses `PFU-*` task IDs so it does not collide with the parent's M0-M11 sequence.

## 1. Correction Record

### 1.1 What went wrong

The previous implementation started in `F:\stuff from desktop\URPG - RPG Game Maker`, a non-Git asset/vendor bundle whose own instructions said that no root application existed. The proposed Creator Hub architecture was then mistaken for the URPG product architecture, and a standalone Node application was built under `first_party/creator-hub`.

That was the wrong implementation target. The real product repository already contains a native C++20/ImGui editor, deterministic native runtime, CMake/Ninja build, Catch2 tests, governed asset services, project/session ownership, map authoring, QuickJS compatibility, diagnostics, replay, recovery, and packaging.

| Mistake | Consequence | Required correction |
| --- | --- | --- |
| Repository identity was not verified before editing. | Work was created outside the Git repository and could not be committed with URPG. | Require a repository-root and remote check before every implementation run. |
| A proposed “Creator Hub” was treated as an absent application to build. | A parallel Node/HTML product shell duplicated existing native ownership. | Extend `urpg_editor`, `EditorProjectSession`, and native document owners only. |
| RPG Maker MZ JSON and `plugins.js` were treated as primary product storage. | Prototype services bypassed URPG schemas, dirty state, history, runtime consumers, and packaging rules. | Keep MZ mutation/import under the optional compat/migration boundary. |
| Prototype tests were treated as product progress. | Passing tests proved only self-contained JavaScript behavior, not native reachability or runtime execution. | Translate selected assertions into native Catch2/Python fixtures and actual app-shell evidence. |
| The imported M0-M13 sequence was allowed to compete with the active native M0-M11 plan. | Status and execution authority became ambiguous. | Keep the imported roadmap as provenance and make this a subordinate `PFU-*` plan. |

### 1.2 What exists from the mistaken implementation

A forensic audit found the local prototype intact at `F:\stuff from desktop\URPG - RPG Game Maker\first_party\creator-hub`:

| Evidence | Audited value |
| --- | ---: |
| Total files | 226 |
| Source files | 157 |
| Test files | 63 |
| Total size | 1,162,219 bytes |
| Test cases on 2026-07-14 | 246 passed, 0 failed |
| Runtime | Node.js 22+, ES modules, browser/local development shell |
| Git history in that workspace | None |

The work is not physically lost, but its passing tests are not evidence that URPG implements those features. Its durable value is the behavior vocabulary and fault scenarios: plan/preview/confirm/apply, source-version checks, provenance, deterministic fixtures, rollback, explicit unavailable states, bounded jobs, and release-blocking diagnostics.

The raw prototype must not be copied into `apps/`, `editor/`, `engine/`, or `runtimes/`. It remains a local source for salvage review until its behavior ledger is complete; deletion or archival requires a separate explicit decision.

### 1.3 Repository-lock preflight

Before changing product code, the implementer must verify all of the following from the intended working directory:

```powershell
git rev-parse --show-toplevel
git remote get-url origin
git status -sb
Test-Path AGENTS.md
Test-Path CMakeLists.txt
Test-Path apps/editor
Test-Path engine/core
```

Expected current root: `G:\URPG Maker-development`. Expected origin: `https://github.com/DocDamage/URPG-RPG-Game-Designer.git`. Stop if either identity is different; do not create a replacement application around a missing path.

## 2. Planning Authority and Status Language

When documents disagree, use this order for this work:

| Purpose | Authority |
| --- | --- |
| Machine-readable subsystem claims | `content/readiness/readiness_status.json` |
| Release-facing subsystem evidence | `docs/release/RELEASE_READINESS_MATRIX.md` |
| App-level release evidence | `docs/APP_RELEASE_READINESS_MATRIX.md` |
| Current program status | `docs/PROGRAM_COMPLETION_STATUS.md` |
| Current creator execution | `docs/superpowers/plans/2026-07-12-creator-product-cohesion-plan.md` |
| Broad native capability direction | `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` |
| This feature absorption work | This document |
| Original MZ/Node proposal | `docs/external-intake/PRODUCT_FEATURE_USABILITY_ROADMAP.md` (reference only) |

This plan uses the following conservative states:

- **Integrate:** substantial native owners exist, but creator-path breadth or release evidence remains open.
- **Extend:** a bounded seed exists, while much of the requested feature still needs native implementation.
- **Defer:** no approved shipping contract exists, or prerequisites/evidence are missing.
- **Compat-only:** the behavior belongs to optional MZ import/execution and is not a native product requirement.
- **Not applicable:** a source defect has no native reproduction and must not become speculative work.

`READY` on a bounded subsystem such as `ui_menu_core`, `accessibility_auditor`, or `export_validator` does not mean the corresponding broad feature F05, F09, or F10 is complete.

## 3. Native Architecture Decisions

1. **One application shell.** `urpg_editor` is the creator product. There is no separate Creator Hub server, website, or Node runtime.
2. **One project authority.** `EditorProjectSession`, native project services, and authoritative document models own open/switch/save/dirty/recovery behavior.
3. **One runtime authority.** Native runtime systems consume native project data. QuickJS is an explicit compatibility/extension lane, not the core gameplay architecture.
4. **One governed asset path.** Discovery, promotion, attachment, relinking, transformation revisions, and packaging extend `engine/core/assets`, `editor/assets`, and `tools/assets`.
5. **Domain-owned mutation.** Preview/apply/inverse behavior must call the responsible native document owner. Generic JSON copies are not authoritative project state.
6. **Shared safety semantics.** Durable changes carry stable IDs, source revisions, validation, creator-readable diagnostics, dirty-state registration, undo/recovery behavior, and atomic persistence.
7. **Contextual editor integration.** Deep tools open from the creator's project/map/object context unless repeated workflow evidence justifies a top-level workspace.
8. **Review-gated intelligence.** AI and procedural suggestions remain optional, bounded, previewable, explicitly approved, reversible, and unable to use raw shell/filesystem authority.
9. **Isolated external tooling.** Heavy ML, media, or generation tooling stays under `tools/` or a supervised adapter and produces reviewed manifests/artifacts for normal native intake.
10. **Truth before promotion.** Compiled models and headless snapshots do not establish WYSIWYG usability, platform qualification, or release readiness.

## 4. Prototype Salvage Disposition

| Prototype family | Portable behavior | Native destination | Disposition |
| --- | --- | --- | --- |
| Hub/project lifecycle, storage, recovery, privacy | Explicit project inspection, safe mode, snapshots, portable archive rules, redacted support previews | `editor/project/*`, `engine/core/project/*`, diagnostics and packaging owners | Translate missing acceptance cases; discard Node shell and stores. |
| Contracts, transaction, command platform, acceptance map | Stable diagnostics, source-version recheck, preview/confirm/apply/undo, evidence-to-requirement mapping | Existing document commands, dirty-state/history, context actions, project audit | Translate the pattern; do not add a competing generic database or transaction journal. |
| Asset intake, transforms, collections, rights, plop | Content hashes, provenance, archive containment, non-destructive revisions, stale-source conflicts | `engine/core/assets/*`, `editor/assets/*`, `tools/assets/*`, map/document commands | Translate behavior and fixtures into native services. |
| Menu, dialogue, quest, localization, captions | Versioned typed data, aggregated validation, preview-before-save, stable localization/voice IDs | Native menu/message/dialogue/quest/localization/audio owners | Translate feature gaps; MZ exporters remain optional compat outputs. |
| Plugin locks, recipes, curated Synrec tests | Non-executing inspection, hashes, dependency diagnostics, typed parameters, rollback | `editor/plugin`, `engine/core/plugin`, `engine/core/mod`, `runtimes/compat_js` | Split native mods from MZ plugins; keep sidecar/`plugins.js` behavior compat-only. |
| Playtest, semantic input, replay, balance, world graph | Bounded state, deterministic traces, reproducible seeds, visible assumptions | Native playtest controller, input, replay, balance, dependency graphs | Translate only after authoritative runtime seams are used. |
| Assistant, RAG, intent, frontier history | Citations, permissions, approval, diffs, reverse patches, bounded plans | `engine/core/ai`, `editor/ai`, `tools/ai`, `tools/retrieval`, native commands | Extend existing native seeds; discard browser/provider/storage duplication. |
| SpriteForge adapter | Admission, loopback isolation, staged results, normal asset intake | Future governed external-tool adapter feeding F21/F11 | Requirements only until source/license/capability review approves a bounded integration. |
| Browser pages/controllers and `node:test` | Accessible labels and explicit disabled states | Native ImGui widgets, panels, snapshots, and manual review | Re-express behavior; never port page/controller code. |

PFU-00 must produce a salvage ledger containing every prototype test name, source feature ID, disposition (`native-test`, `compat-test`, `already-covered`, `bundle-only`, or `discard`), native owner, target fixture/test, and result. A passing JavaScript assertion is not checked off until its selected native equivalent passes through an actual URPG owner.

## 5. Feature Reconciliation

### 5.1 Core features F01-F10

| ID | Native state | Existing owners/evidence | Residual native outcome |
| --- | --- | --- | --- |
| F01 Creator Hub | Integrate | `apps/editor/main.cpp`, `editor/project/*`, `engine/core/project/*`, creator-journey tests | Extend the existing shell with all-surface dirty ownership, remaining lifecycle/archive/storage/recovery breadth, truthful health/search/jobs, and safe migration flows. No second Hub. |
| F02 Visual Asset Library | Integrate | `editor/assets/*`, `engine/core/assets/*`, `tools/assets/*`, 100,000-row catalog fixture | Add collections/favorites/comparison, richer facets and preview coverage, governed “use here” actions, and exact undo/history integration. |
| F03 Asset Transformation Studio | Extend | Asset conversion handoff, spritesheet metadata/preview, `tools/sprite_pipeline`, import tooling | Add one native transform-plan/revision path for crop/slice/scale/palette/atlas/tileset outputs with hashes, provenance, preview, validation, and inverse/removal behavior. |
| F04 Dependency-aware Plugin Manager | Extend | Native plugin manifests/inspection, mod registry/manager, QuickJS plugin manager and tests | Keep native mods and MZ plugins separate; add non-executing discovery, dependency/ordering plans, locks, typed config, trust, support files, save impact, and rollback where each lane owns them. |
| F05 WYSIWYG Menu Studio | Extend | Native menu scene graph, serializer/migration, inspector and preview, bounded core signoff | Add real canvas layout editing, constraints, guides, templates/components, focus-flow preview, drafts/history, failure-safe persistence, runtime round trip, and manual graphical evidence. |
| F06 Gameplay Recipe Gallery | Extend | Native gameplay recipes document, templates and runtime profiles | Redefine as native schema/runtime compositions with guided parameters, preview, validation, apply/undo, and versioning. MZ plugin recipes remain compat-only. |
| F07 Visual Quest and Dialogue Builder | Extend | Native dialogue/quest graphs, schemas, panels, contextual creator project, runtime tests | Complete visual node editing, stable reference pickers, reachability/dead-end validation, localization/voice IDs, undo, migrations, and authored runtime proof. |
| F08 Live Playtest Lab | Integrate | `editor/playtest/playtest_session_controller.*`, runtime diagnostics, recovery, vertical-slice tests | Add bounded state inspectors/mutations, map travel, profiling, reload negotiation, scenario replay, disposable state restore, crash isolation, and exact redacted support bundles. |
| F09 Accessibility and Localization Center | Integrate | Native auditor/adapters, localization models/panel, input remap, bounded signoff | Complete keyboard/controller focus, canvas alternatives, user profiles, pseudo-localization, stale keys, font/glyph/RTL/IME/plural/layout validation, and manual assistive review. |
| F10 Release Assistant | Integrate | Native export validator/packager, preview/diagnostics panels, project audit and release matrices | Aggregate project references, credits/notices/BOMs, plugin/save/update impact, secret/security checks, exact staging, platform qualification, install/repair/update/rollback evidence. |

### 5.2 Advanced features F11-F24

| ID | Native state | Existing owners/evidence | Residual native outcome |
| --- | --- | --- | --- |
| F11 Direct Placement and Smart Prefabs | Extend | Typed asset drag payloads, Map workspace, grid-part commands, attachment service | Move from Tiles/Props palette enrollment to immediate atomic instance placement, other contextual targets, stale-source checks, shared history, keyboard parity, and versioned smart prefabs. |
| F12 Embedded Agentic Creator Assistant | Extend; developer-only | AI knowledge/tool registry, approval/diff/revert UI, creator planner, OpenAI-compatible service | Bind tools to authoritative native commands, complete bounded multi-turn tool handling, capability/source-version/secret/budget policy, and optional provider/model packaging qualification. |
| F13 Hybrid Project RAG | Extend; offline foundation | Native lexical knowledge plus `tools/ai` and `tools/retrieval` artifacts | Add per-project cited hybrid retrieval, incremental invalidation, object/path/hash/version identity, deletion propagation, access/license filters, and injection-safe untrusted-data handling. |
| F14 Intent-to-Project Compiler | Extend; early seed | Creator command planner/panel for bounded tile/prop/logic plans | Expand only through approved native commands to maps/events/menus/quests; add locked regions, provenance/cost, preview, validation, rollback, and separately evaluated image/sketch inputs. |
| F15 Multimodal Asset and Style Intelligence | Defer beyond tooling seed | Metadata catalog and offline vision/segmentation tooling | Establish an evaluation corpus before visual embeddings, crop/sketch search, palette/style scoring, explanations, or license-aware recommendations become product work. |
| F16 Autonomous Playtest Agents | Defer beyond strong replay seed | Playtest controller, semantic input foundations, replay recorder/player/gallery, headless tests | Add isolated budgeted explorers/fuzzers/personas that drive real semantic input and emit reproducible state/input/visual evidence. Do not call static graph analysis an agent. |
| F17 Constraint-based Procedural Authoring | Extend | Seeded map generator, procedural toolkit/panel, terrain brush | Add visual constraints, locked regions, selective regeneration, NPC/loot/quest rules, authoritative commands, per-operation review, provenance/versioning, and deterministic runtime proof. |
| F18 Balance and Progression Simulator | Extend | Economy simulator, encounter/balance panels, combat/crafting/progression models | Build canonical-data scenarios, personas/strategies, batches/sensitivity, cliff/dominant-strategy/deadlock diagnostics, and navigation back to source parameters. |
| F19 Project World Graph and Impact Explorer | Extend | Event and grid-part dependency graphs, asset `used_by`, world travel graph | Add one incremental stable-ID project reference view spanning document types, why-included/orphan/impact queries, safe rename/delete plans, external-change handling, and graph UI. |
| F20 Branchable Collaborative Editing | Defer beyond early primitives | Domain undo, snapshots/recovery, local review bundles, AI patches | First establish semantic operation history. Then add named branches, object compare, three-way conflict resolution, dependency-aware selective replay/merge, provenance, and portable exchange. |
| F21 Personal Asset Vault | Integrate | Import session, global library, promotion, attachment, archive, relink, cleanup, asset UI/tooling | Add clipboard/watched intake, collections, durable cancel/resume jobs, explicit custody/portability policy, full transaction/undo integration, and broader format/archive evidence. |
| F22 Unified Input and Controller Platform | Extend | Input core, remap profiles/store, controller binding runtime/panels | Add device polling/hot-plug/ownership, context routing, axes/chords/deadzones/repeat, spatial focus/lint, calibration/recovery, glyphs/haptics, replay integration, and controller-only editor proof. |
| F23 SpriteForge Integration | Defer | No SpriteForge implementation; native sprite pipeline/preview are possible attachment points | Complete source/license/capability admission first. If approved, expose a supervised typed job adapter whose outputs enter F21 and F11; never import its environment, project DB, plugin loader, launcher, or frontend. |
| F24 Audio, Voice, and Caption Studio | Extend | Audio core/mixer/presets/validator, inspector/mix panels, offline audio tools | Add waveform/spectrogram editing, non-destructive trim/fade/gain revisions, loudness/peak/loop QA, take/locale/rights/caption models, assignment, locale/muted-alternative checks, and undo/release integration. |

### 5.3 Improvements I01-I10

| ID | Disposition in native URPG |
| --- | --- |
| I01 Preloader resource lifetime | Adapt the invariant to native cache/loader/thumbnail budgets and M10 performance evidence. Synrec code is not a native implementation target. |
| I02 Preloader correctness/recovery | Audit native priority loader, asset loader, startup services, and runtime preflight for queue/cancellation/terminal-failure gaps; add native tests only for reproduced gaps. |
| I03 Menu Builder portability | Retire the HTML sidecar requirement. Prove native `menu_builder` route/package reachability; unsupported MZ sidecars receive explicit compat diagnostics. |
| I04 Safe menu persistence | Merge into native serializer, shared dirty/save ownership, atomic publication, recovery, and interruption tests. |
| I05 Configuration validation | Merge into menu schemas, inspector/runtime diagnostics, migration, and release preflight. |
| I06 Actor preview correctness | Mark not applicable unless reproduced in the native character/appearance/battle preview; never port a speculative Synrec row/X/Y fix. |
| I07 Typed configuration/restricted scripting | Keep as an invariant. Ordinary native configuration remains typed; advanced scripts stay in the governed QuickJS/extension boundary. |
| I08 Existing editor usability | Merge into parent M1/M4/M7/M10 dirty state, history, recovery, theme, accessibility, performance, and manual UX work. |
| I09 Catalog integrity | Merge into F02/F04/F21 and parent M2/M3/M11 provenance, support-file, license, promotion, attachment, and release gates. Remove bundle-specific record counts. |
| I10 First-party release engineering | The native foundation exists. Residual work is documentation authority, clean-clone/package/platform evidence, and continuing deterministic gate maintenance. |

## 6. Dependency Order

```text
PFU-00 provenance/salvage
  -> PFU-01 active creator-plan closure
  -> PFU-02 native mutation/reference/history spine
  -> PFU-03 governed asset/transform/audio depth
  -> PFU-04 native extensions/recipes/menu/narrative depth
  -> PFU-05 placement/impact/local-history depth
  -> PFU-06 playtest/input/accessibility/release integration
  -> PFU-07 optional assistant/RAG/intent integration
  -> PFU-08 evaluated advanced authoring and simulation
  -> PFU-09 optional SpriteForge admission/integration
  -> PFU-10 governed vertical-slice and release qualification
```

PFU-06 requirements such as focus, input, diagnostics, and release metadata apply continuously to earlier work even though their final qualification occurs later. PFU-07-PFU-09 must not block the no-AI native creator path or the current parent-plan release decision.

## 7. Executable Task Plan

### PFU-00 - Lock provenance and salvage behavior

**Source IDs:** all; especially I10

**Status:** Now
**Owners:** `docs/external-intake`, `docs/superpowers/plans`, `docs/agent`

Implementation:

1. Keep the imported roadmap as a clearly labeled historical source input.
2. Create `docs/external-intake/CREATOR_HUB_PROTOTYPE_SALVAGE_LEDGER.md` before deleting or archiving the local prototype.
3. Record a content hash and one row for every one of the 246 prototype test cases.
4. Classify MZ-only assertions as compat tests, portable safety assertions as native test candidates, and browser-only assertions as discarded implementation with retained UX intent.
5. Link every accepted assertion to an existing or planned native fixture/test and named owner.
6. Prohibit `.mjs`, Node packages, browser pages, prototype data stores, and direct `plugins.js` writers from native product directories.

Acceptance:

- Every prototype assertion has a disposition and native owner or explicit rejection reason.
- No prototype code is counted as native feature progress.
- The only working Git repository used for implementation is the verified URPG root.
- Documentation authority no longer points at missing or archived files as current truth.

Verification:

```powershell
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
.\tools\ci\check_release_readiness.ps1
.\tools\ci\truth_reconciler.ps1
git diff --check
```

### PFU-01 - Finish the active creator-product cohesion plan

**Source IDs:** F01, F02, F08, F10, F21, I08-I10

**Status:** Current product priority
**Parent tasks:** M1, M3-M5, M9-M11

Implementation:

1. Register Perspective 2D-only changes and durable non-Map editors with the shared dirty-state owner.
2. Extend governed asset drops beyond Map Tiles/Props and ensure real owner-aware undo history.
3. Complete remaining unified Map shortcuts and record graphical route/layout equivalence.
4. Make new-project-to-immediate-playtest satisfy the original M5 creator outcome.
5. Author and replay the M9 vertical slice through creator controls; any required manual JSON edit is a product blocker.
6. Complete M10 graphical, keyboard, accessibility, DPI, and reference-hardware review.
7. Complete M11 non-sparse clean-clone, exact asset hydration, target packaging/platform evidence, and release-owner decision.

Acceptance:

- The creator journey works through the native shell without editing JSON.
- M6-M9 bounded contracts are requalified from the target build and are not substituted for manual evidence.
- No PFU expansion claim hides an open parent-plan blocker.

Verification:

```powershell
ctest --test-dir build/dev-ninja-debug -R "creator journey" --output-on-failure
.\tools\ci\check_creator_journey.ps1 -BuildDirectory build/dev-ninja-debug
ctest --preset dev-spatial --output-on-failure
.\tools\ci\check_creator_vertical_slice.ps1 -BuildDirectory build/dev-ninja-debug
```

Manual evidence: startup, wizard, Assets, both Map routes, contextual editors, playtest return, recovery, package preview, keyboard-only use, supported scales/resolutions, and target package playthrough.

### PFU-02 - Complete the native mutation, reference, and history spine

**Source IDs:** F01, F11-F14, F19-F20
**Status:** Next; begin only from existing owners

Primary owners:

- `editor/project/editor_project_session.*`
- `editor/project/editor_dirty_state_registry.*`
- `editor/ui/editor_context_action.*`
- native document command/history implementations
- `engine/core/events/event_dependency_graph.*`
- `engine/core/map/grid_part_dependency_graph.*`
- `engine/core/project/project_snapshot_store.*`

Implementation:

1. Inventory stable object IDs, source revisions, dirty owners, history owners, reference edges, and atomic save behavior by native document type.
2. Define a small shared operation envelope only for fields that at least two authoritative domains require: operation ID, project/document/object ID, source revision, capability, preview, diagnostics, affected documents, inverse/rollback description, and provenance.
3. Keep application logic in domain commands. The envelope delegates; it never mutates an independent JSON mirror.
4. Aggregate reference edges through adapters over existing native graphs and asset metadata before inventing a new store.
5. Persist portable semantic history only after Map placement and one non-Map domain prove deterministic apply/inverse/replay.
6. Route AI, procedural, placement, and collaboration callers through the same approved domain commands used by native UI controls.

Acceptance:

- A stale operation is rejected without partial mutation.
- A multi-document operation declares every dirty owner and either commits atomically or leaves the prior valid state.
- Preview and apply target the same source revision.
- Undo/recovery behavior is explicit and tested for Map placement plus one non-Map domain.
- No second project database, generic JSON authority, or global history bypass is created.

Verification: add focused Catch2 coverage for every participating domain, then run `ctest --preset dev-pr --output-on-failure` and the affected persistence/Map gates from `docs/agent/QUALITY_GATES.md`.

### PFU-03 - Deepen governed assets, transforms, personal custody, and audio

**Source IDs:** F02, F03, F21, F24, I01-I02, I09
**Status:** Next after PFU-01; PFU-02 required for durable mutation

Implementation:

1. Add project/user collections, favorites, comparison, richer facets, and complete bounded preview states to the native asset library.
2. Define a versioned native transform plan and derived-revision manifest. Implement image crop/slice/scale/palette/atlas/tileset operations incrementally through current asset and sprite tooling.
3. Make all derived outputs content-hashed, provenance-linked, collision-validated, previewable, and removable/revertible without altering the source revision.
4. Clarify global-library custody, external-link portability, relocation, capacity, cancellation/resume, and cleanup policy. Add clipboard/watched intake only after the same review/promotion path is preserved.
5. Add audio revision and QA contracts for trim/fade/gain, codec, loudness/peak, loop seams, waveform/spectrogram metadata, voice take/locale/rights, captions, and muted alternatives.
6. Feed only promoted/attached revisions into Map, character, menu, dialogue, event, database, and release consumers.

Acceptance:

- Discovery never implies promotion, attachment, licensing, or release eligibility.
- Transform/audio jobs are deterministic, cancel-safe, source-version checked, and do not overwrite source media.
- A selected visual or audio asset can travel from intake through review, revision, attachment, contextual assignment, runtime preview, and package evidence.
- Cache/resource budgets remain bounded under long scrolling and repeated preview.

Verification:

```powershell
python -m unittest tools.assets.tests.test_asset_db tools.assets.tests.test_catalog_interchange -v
python -m unittest tools.assets.tests.test_global_asset_import -v
.\build\dev-ninja-debug\urpg_tests.exe "[assets][local_catalog]" --reporter compact
.\build\dev-ninja-debug\urpg_tests.exe "[assets][thumbnail],[assets][archive],[assets][drag_drop],[assets][promotion],[assets][attachment]" --reporter compact
```

Run focused audio/tool tests for each touched operation and record manual image, animation, archive, waveform, loop, caption, and assignment review.

### PFU-04 - Deepen extensions, recipes, menu, quest, and dialogue authoring

**Source IDs:** F04-F07, I03-I08
**Status:** Next; independent sublanes share PFU-02 safety semantics

Implementation:

1. Separate the native mod/extension lifecycle from MZ plugin compatibility in UI, manifests, trust, diagnostics, and packaging.
2. For MZ compatibility, add non-executing discovery/metadata/dependency/lock review before optional execution; never inspect a catalog by evaluating plugin code.
3. Create a native gameplay recipe artifact that composes supported schemas/templates/runtime primitives, validates prerequisites, previews changes, applies through domain commands, records a version, and can be undone.
4. Complete native Menu Studio canvas/layout/focus authoring over `menu_scene_graph` rather than exporting a Synrec sidecar as the product source of truth.
5. Complete dialogue and quest graph mutation, stable-reference pickers, reachability and softlock diagnostics, localization/voice/caption links, undo, migrations, and runtime execution.
6. Keep advanced scripts explicit, sandboxed, diagnostic-rich, and outside ordinary typed configuration.

Acceptance:

- Native features do not depend on the curated CGMZ/Synrec inventory.
- A recipe can be applied twice without duplication and reverted without corrupting unrelated content.
- Menu, dialogue, and quest changes are visually authored, previewed, saved, executed by the native runtime, diagnosed, and recovered.
- Unsupported MZ behavior is preserved or diagnosed; it is not silently rewritten as native support.

Verification:

```powershell
ctest -L weekly --output-on-failure
ctest --preset dev-all -R "WYSIWYG|readiness_status" --output-on-failure
ctest --preset dev-pr --output-on-failure
```

Manual evidence: menu layout/focus across target sizes and input modes; dialogue/quest graph editing and playthrough; native-mod versus MZ-plugin labeling and trust flow.

### PFU-05 - Complete direct placement, impact analysis, and local operation history

**Source IDs:** F11, F19, F20
**Status:** After PFU-02 and governed asset revisions

Implementation:

1. Extend typed drops from palette enrollment to immediate Map tile/prop/event placement with preview, conflict policy, source revision, stable instance ID, inverse, and owner-aware history.
2. Add contextual placement adapters for character/database slots, menus, dialogue/quest nodes, events, and audio only when their authoritative owner supports the shared safety fields.
3. Define versioned smart prefabs as native operation groups with declared dependencies, parameters, conflicts, stable IDs, and per-operation acceptance/rejection.
4. Aggregate reference edges for maps, events, assets, database records, menus, quests, dialogue, extensions, and saves. Expose why-included, inbound/outbound, orphan, and impact queries.
5. Implement safe rename/delete/replace plans through domain owners.
6. Promote portable operation history and local branches only after deterministic replay and three-way conflict fixtures prove dependency-closed selective merge. Keep local review useful without hosted accounts.

Acceptance:

- One governed asset can be placed immediately, undone as one creator action, replayed, and rejected on stale/conflicting state without partial writes.
- Deleting or renaming a referenced object always previews affected native objects and cannot bypass their validators.
- Branch/merge never becomes a raw file-copy or generic JSON overwrite path.
- Hosted sync is not required for local review or portable exchange.

Verification: use the M3 asset gate, `ctest --preset dev-spatial --output-on-failure`, affected dependency-graph tests, and new operation-history/merge fixtures before any F20 exposure change.

### PFU-06 - Integrate playtest, semantic input, accessibility, localization, and release evidence

**Source IDs:** F08-F10, F22, I08-I10
**Status:** Cross-cutting; final closure after PFU-03-PFU-05

Implementation:

1. Expand the playtest controller through explicit native adapters for map travel, state inspection, reviewed disposable mutation, profiling, reload compatibility, scenario replay, state restore, crash isolation, and support bundles.
2. Route editor and runtime controls through semantic actions with device/context ownership, axes/chords/deadzones/repeat, hot-plug, calibration, recovery chord, focus graph, glyph, haptic, and replay contracts.
3. Complete keyboard/controller-only creator workflows and structured alternatives for custom canvases.
4. Complete localization extraction/import/stale-key/pseudo-localization/font/glyph/RTL/IME/plural/format/layout checks through native project data.
5. Aggregate release preflight from authoritative project/reference/asset/extension/save/localization/accessibility/input/export owners. Produce exact credits, notices, BOMs, manifests, blockers, and target evidence.
6. Keep signing, notarization, store credentials, and legal decisions explicit external requirements.

Acceptance:

- A replay uses semantic actions and records deterministic state hashes without storing unnecessary raw device telemetry.
- All required creator surfaces are usable with keyboard alone and have truthful controller/focus status.
- Playtest diagnostics focus the responsible native object and do not expose secrets.
- The exact reviewed package input is the exact validated input; raw external/local/recovery/playtest data is rejected.

Verification:

```powershell
ctest --preset dev-all -R "startup|settings|input" --output-on-failure
ctest --preset dev-export --output-on-failure
ctest --preset dev-snapshot --output-on-failure
.\tools\ci\check_accessibility_governance.ps1
.\tools\ci\check_input_governance.ps1
.\tools\ci\check_localization_consistency.ps1
```

Manual evidence includes controller-only and keyboard-only editor/runtime passes, assistive review, target-device hot-plug/calibration, playtest crash/recovery, and exact package inspection.

### PFU-07 - Rebind assistant, RAG, and intent to authoritative native commands

**Source IDs:** F12-F14
**Status:** Optional, developer-only until all acceptance evidence passes

Implementation:

1. Preserve the current approve/reject/approve-all/apply/revert UI, result diffs, reverse patches, and explicit unavailable states.
2. Replace generic project-JSON mutation tools with adapters over PFU-02 domain commands and source revisions.
3. Complete the bounded multi-turn tool loop with schemas, capability grants, step/time/cost limits, cancellation, secret isolation, provider disclosure, and offline/no-AI behavior.
4. Join native structured knowledge with offline lexical/semantic artifacts through cited object/path/hash/version records. Live mutable state always comes from tools.
5. Invalidate retrieval on committed project changes and propagate deletion/forget to derived indexes.
6. Extend intent planning one native domain at a time. Image/sketch input is a separate evaluated adapter, not an automatic expansion of text authority.
7. Do not adopt the old roadmap's specific Qwen, Granite, or `llama.cpp` packaging choices without a current ADR, licensing, hardware, quality, security, size, and update evaluation.

Acceptance:

- The no-AI creator workflow remains complete.
- The assistant cannot invoke raw filesystem, shell, process, credential, plugin-loader, or unrestricted network tools.
- Every mutation is reviewable, source-version checked, validated by the native owner, and reversible.
- Retrieved content is visibly cited and untrusted; it cannot grant permissions or replace live tool state.

Verification:

```powershell
ctest --preset dev-all -R "AI (knowledge|task|tool|assistant)|Chatbot component" --output-on-failure
python -m unittest discover -s tools/retrieval/tests -p "test_*.py" -v
```

Provider/model work also requires manual privacy, permission, cancellation, failure, offline, secret-redaction, and reference-hardware evidence before release exposure changes.

### PFU-08 - Promote advanced authoring only through evaluated native slices

**Source IDs:** F15-F18, residual F20
**Status:** Later; each feature is independently gated

Implementation slices:

1. **F15:** Create a license-safe visual-search/style evaluation corpus and baseline. Add embeddings or multimodal models only if measured relevance materially beats metadata/palette search within storage/performance budgets.
2. **F16:** Build a deterministic non-AI explorer first using semantic input, real runtime state, isolated seeds/budgets, replay, screenshots, and softlock/oracle diagnostics. Optional model-guided policies remain separate.
3. **F17:** Route seeded generation and selective regeneration through PFU-02 commands with visual constraints, locks, per-operation review, provenance, and runtime validation.
4. **F18:** Run canonical combat/economy/crafting/progression/quest data through declared personas, batches, sensitivity comparisons, and source-linked diagnostics.
5. **F20:** Expand local semantic history into branch/merge only after dependency-closed replay and conflict resolution pass deterministic fixtures.

Acceptance:

- Each feature has a benchmark/fixture that distinguishes real behavior from a static preview or hard-coded demo.
- Deterministic seeds, assumptions, budgets, failures, and source versions are visible.
- Generated or simulated results never silently mutate authored data.
- A failed experiment can be disabled or removed without affecting the core creator/runtime path.

Verification: add focused native tests and tool-level evaluation tests per slice, run `ctest --preset dev-pr --output-on-failure`, and keep each panel deferred/developer-only until its complete WYSIWYG and manual gate passes.

### PFU-09 - Decide SpriteForge admission before integration

**Source IDs:** F23
**Status:** Deferred pending external evidence

Implementation:

1. Identify the exact source repository/revision, license, model/data dependencies, service routes, generated payloads, and redistribution constraints.
2. Record keep/reimplement/reject decisions per capability. A documented rejection is a valid outcome.
3. If approved, implement a versioned supervised job adapter with explicit health/capabilities, bounded inputs/outputs, cancellation, timeouts, path allowlists, no unsolicited network access, and no arbitrary plugin/process authority.
4. Import staged results through F21 promotion/revision rules and place them through F11. Do not add a SpriteForge project database, catalog, history, or frontend to URPG.
5. Exclude development environments, caches, models, source payloads, and service credentials from game packages.

Acceptance:

- No code or payload is adopted before provenance/license/security disposition.
- Approved output is indistinguishable from other governed asset revisions after intake.
- The adapter can be absent, disabled, cancelled, or removed without breaking URPG.

Verification: provenance/license checks, adapter contract tests, containment/path tests, cancellation tests, asset promotion/attachment tests, and exact package exclusion evidence.

### PFU-10 - Qualify one governed product increment

**Source IDs:** all promoted IDs
**Status:** Final gate for each selected release scope

Implementation:

1. Select a bounded release feature set; do not require every deferred frontier feature for the native core product.
2. Update the creator vertical slice to exercise only features claimed for that increment.
3. Author the slice through actual creator controls and package the exact reviewed project-selected assets.
4. Run focused gates during implementation, milestone gates before checking tasks complete, and the full local/release-candidate gates only against the intended candidate.
5. Update readiness/status/docs only after code, reachability, tests, manual evidence, migration/rollback, and release-owner review support the exact claim.

Acceptance:

- Every claimed feature meets the definition of done below.
- Deferred, developer-only, optional-provider, platform, legal, credential, and asset-hydration limitations remain explicit.
- Clean-clone and package evidence binds to the same commit/candidate and exact project-selected payload.

Final verification:

```powershell
.\tools\ci\run_local_gates.ps1
.\tools\ci\check_release_required_assets.ps1
.\tools\ci\check_promoted_asset_library.ps1
.\tools\ci\check_lfs_release_scope.ps1
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
```

## 8. Required Evidence Record

Each implemented PFU task must record:

| Field | Requirement |
| --- | --- |
| Source | F/I IDs and prototype test/requirement references accepted or rejected |
| Ownership | Native runtime owner, editor entry point, schema, persistence owner, package consumer |
| Change | Files and behavior actually changed; no completion credit for untouched seeds |
| Automated evidence | Exact command, fixture, expected artifact, result, commit |
| Manual evidence | Scenario, target build, resolution/DPI/input/device/hardware as applicable, reviewer/date |
| Safety | Source revision, validation, diagnostics, atomicity, inverse/recovery, migration behavior |
| Reachability | Registry exposure and actual app-shell/context route |
| Release truth | Readiness rows/docs changed or intentionally unchanged, with reason |

Do not use feature completion percentages unless the denominator and evidence fields are versioned and reproducible.

## 9. Definition of Done

A feature is done for a named release scope only when:

1. it is reachable through the native creator shell or an explicitly documented contextual route;
2. it has a named authoritative runtime/document owner and no parallel project or asset authority;
3. visual authoring, live preview, saved project data, runtime execution, diagnostics, and tests satisfy the WYSIWYG done rule where applicable;
4. durable mutations are source-version checked, validated, atomic, dirty/history aware, reversible or recoverable, and migration-safe;
5. empty, loading, disabled, warning, error, conflict, cancellation, and recovery states are truthful;
6. focused automated gates pass and required graphical/accessibility/device/performance review is recorded;
7. exact governed assets and external dependencies are present, licensed, attributed, and package eligible;
8. canonical status and readiness documents match the bounded evidence; and
9. no Node/browser prototype code, hidden online dependency, raw external path, recovery data, or development-only payload enters the shipping product.

## 10. Non-goals

- Porting the Node Creator Hub, its HTML pages, or its stores into URPG.
- Making RPG Maker MZ plugin management or sidecars the native product architecture.
- Preserving bundle-specific asset/plugin counts as product acceptance criteria.
- Replacing the native runtime with browser, npm, cloud, SpriteForge, or AI authorities.
- Shipping every F01-F24 feature in one release or blocking the core no-AI product on deferred frontier work.
- Promoting a panel because it compiles, renders a snapshot, or passes a prototype test.

## 11. Immediate Next Actions

1. Validate this documentation rewrite and reconcile current-status authority links.
2. Build the PFU-00 salvage ledger from the intact 246-test prototype before any cleanup decision.
3. Re-run the parent creator-plan focused gates from the current native branch and update only evidence that actually changed.
4. Finish PFU-01/M10-M11 creator and release qualification before broad feature expansion.
5. Start PFU-02 with an inventory of existing native command/history/reference owners; do not begin by adding a universal framework.
