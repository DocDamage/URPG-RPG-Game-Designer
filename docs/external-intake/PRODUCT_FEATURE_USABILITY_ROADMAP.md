# Product Feature and Usability Roadmap

> **Historical source input — do not execute as a URPG implementation plan.**
> This document was authored for a non-Git RPG Maker MZ asset/vendor bundle and
> a standalone Node Creator Hub prototype. Its Node/NW.js, HTML sidecar,
> `plugins.js`, bundle-count, and M0-M13 assumptions do not describe the native
> URPG product. The corrected executable mapping is
> `docs/superpowers/plans/2026-07-14-product-feature-usability-native-absorption-plan.md`.

**Status:** Superseded external-intake proposal; retained for provenance
**Last updated:** 2026-07-14  
**Primary user:** Historical target — a creator building an RPG Maker MZ game with the old bundle
**Planning horizon:** Multi-release; no calendar commitment is implied  
**Replacement plan:** `docs/superpowers/plans/2026-07-14-product-feature-usability-native-absorption-plan.md`

## 1. Purpose

This roadmap turns the current asset and vendor bundle into a coherent, safe, and approachable creator product. It covers the original ten feature additions and ten improvements plus direct asset placement, an embedded tool-using AI helper, hybrid RAG, frontier authoring, bring-your-own assets, unified modern controller support, a selective SpriteForge Asset Studio integration, and a complete audio/voice/caption workflow. Its final no-regret pass also makes project recovery, storage/offline operation, save compatibility, signed updates, safe mode, privacy, and executable supply-chain trust explicit rather than implied.

The delivery strategy is feature-first:

1. establish only the thin product spine needed for safe parallel feature work: canonical source boundary, incremental development runner, feature flags, shared contracts, and a minimal fixture;
2. fix development-blocking safety defects first: boot hangs, destructive writes, unrecoverable configuration corruption, and unsafe trust boundaries;
3. build and continuously merge all planned features and improvements in dependency order behind feature flags;
4. during feature implementation, run only affected static/contract checks and one representative development-mode workflow; do not run clean production builds, packaging, full regressions, full catalog traversals, platform matrices, or soak suites;
5. author fixtures and acceptance automation with each feature, but defer broad execution until every planned feature reaches the Feature Complete gate;
6. after feature freeze, create one canonical immutable production artifact, run the full integration and release-qualification suites against that same artifact, batch fixes, and rebuild only when candidate inputs change;
7. promote the exact tested artifact when no code or packaged-content change has invalidated it.

Feature-first does not mean keeping feature branches isolated until the end. Code, schemas, routes, and shared-service integrations merge continuously so integration drift is discovered cheaply without repeatedly producing release builds.

## 2. Current Baseline and Constraints

The current checkout is an asset and vendor bundle rather than a root application repository. At the top level it contains `third_party/`, `AGENTS.md`, and the existing technical-debt plan, but no canonical first-party RPG Maker MZ project or repo-wide build/test entrypoint.

Grounded inventory:

- the curated Steam DLC payload contains 7 plugins and passes the existing structural validator with 0 errors and 0 warnings;
- the raw 30-file Steam drop-in tree reports 48 errors and 12 warnings caused by duplicate stems, plugin keys, and copies;
- the itch inventory describes 72 unique archives, 15,264 extracted pack files, and 396 loose files;
- the semantic inventory spans 12 top-level categories, including 7,146 icon files and 1,737 files currently classified as `misc`;
- the CGMZ manifest lists 44 feature plugins, with additional support-file requirements in the payload;
- `TECH_DEBT_REMEDIATION_PLAN.md` still identifies active Synrec preloader and Menu Builder runtime risks;
- no first-party embedded LLM implementation is present in this checkout: there is no model client, prompt/tool registry, provider configuration, embeddings pipeline, vector store, RAG index, or agent orchestration code to audit;
- the separately supplied SpriteForge Studio project is a substantial Python/Flask asset-authoring application with useful image, sprite, pixel-art, QA, revision, and export services. Its current assistant is deliberately read-only/navigation-only, its retrieval is lexical rather than embedding-based, its Gamepad API support is partial and hard-coded, and its RPG Maker export preset is not a direct MZ import/placement implementation;
- the SpriteForge source is MIT-licensed, but its local vendor, model, LPC, CuteSCKR, input, output, project, virtual-environment, and generated payloads cannot be assumed redistributable or suitable for inclusion. Integration must select first-party services and audit every separately licensed payload.

The existing structural validation is useful, but it does not replace boot, runtime, editor, input, memory, upgrade, or packaging tests.

## 3. Product Principles

All work in this roadmap follows these principles:

1. **Safe by default:** preview changes before mutation; never silently overwrite user work.
2. **Reversible:** installs, transforms, configuration changes, and upgrades have undo or rollback paths.
3. **Portable:** checked-in and packaged content uses project-relative paths and works on a clean machine.
4. **Deterministic:** identical inputs produce the same catalog IDs, dependency plan, configuration, and release manifest.
5. **Guided:** common tasks use validated forms, presets, and actionable diagnostics instead of manual file editing.
6. **Vendor-preserving:** source assets and vendor plugins remain unchanged; derived artifacts retain provenance.
7. **Accessible:** keyboard operation, visible focus, readable layouts, scalable UI, and localization are designed in rather than added at the end.
8. **Observable:** failures identify the affected project, file, plugin, menu, field, and recommended remediation.
9. **Performance-budgeted:** indexing, previews, preloading, and runtime features have explicit memory and response-time targets.
10. **Release-gated:** a clean metadata report is never treated as proof that the runtime is release-ready.

## 4. Scope and Traceability

### 4.1 Feature Additions

| ID | Feature | Outcome |
|---|---|---|
| F01 | Creator Hub | One entry point for projects, assets, plugins, menus, playtesting, validation, and export. |
| F02 | Visual Asset Library | Search, preview, filter, favorite, collect, and use the complete asset catalog. |
| F03 | Asset Transformation Studio | Non-destructive spritesheet, icon, palette, scale, and tileset workflows. |
| F04 | Dependency-aware Plugin Manager | Safe plugin discovery, ordering, dependency resolution, configuration, locking, and rollback. |
| F05 | WYSIWYG Menu Studio | Visual menu creation with templates, responsive layouts, live preview, and input-flow preview. |
| F06 | Gameplay Recipe Gallery | Tested, guided combinations of existing gameplay plugins with usable defaults. |
| F07 | Visual Quest and Dialogue Builder | Node-based authoring for dialogue, quests, conditions, actions, rewards, and cutscenes. |
| F08 | Live Playtest Lab | Hot reload, map travel, state inspection, diagnostics, profiling, and crash reporting. |
| F09 | Accessibility and Localization Center | Input, display, text, translation, and layout validation tools for creators and players. |
| F10 | Release Assistant | Readiness checks, credits, license gates, packaging, manifests, and supported-platform export. |
| F11 | Direct Placement and Smart Prefabs (Plop) | Drag or choose an asset and place it directly into a map, menu, event, database slot, dialogue, quest, or audio target as one reversible transaction. |
| F12 | Embedded Agentic Creator Assistant | A provider-neutral LLM helper that uses the same typed project tools as the UI, with permissions, previews, approvals, rollback, and audit trails. |
| F13 | Hybrid Project RAG and Knowledge Fabric | Structured, lexical, semantic, and optional multimodal retrieval over project knowledge with source citations and incremental indexing. |
| F14 | Intent-to-Project Compiler | Convert a prompt, sketch, screenshot, or design brief into reviewable structured operations for maps, events, menus, quests, and asset placement. |
| F15 | Multimodal Asset and Style Intelligence | Search and compare assets by text, sketch, screenshot, palette, mood, visual similarity, and project style consistency. |
| F16 | Autonomous Playtest Agents | Input-driven agents explore, pursue goals, fuzz interfaces, detect softlocks, and emit reproducible traces and state snapshots. |
| F17 | Constraint-based Procedural Authoring | Deterministically generate and selectively regenerate map dressing, encounters, treasure, NPC schedules, and quest variants. |
| F18 | Balance and Progression Simulator | Simulate combat, economy, drops, crafting, leveling, and quest progression across configurable player personas. |
| F19 | Project World Graph and Impact Explorer | A live graph of maps, events, assets, database records, menus, quests, plugins, saves, and references for impact analysis. |
| F20 | Branchable Collaborative Editing | Structured operation history, snapshots, branches, conflict detection, selective merge, and replayable changes across human and AI edits. |
| F21 | Bring Your Own Assets and Personal Asset Vault | Add files, folders, archives, or clipboard content to a managed `My Assets` library, then validate, organize, transform, search, and plop them without manual project-folder work. |
| F22 | Unified Input and Controller Platform | One semantic action system for the Creator Hub, editors, previews, playtests, and exported games with modern devices, remapping, glyphs, accessibility, diagnostics, and replay. |
| F23 | SpriteForge Asset Studio Integration | Adapt SpriteForge's high-value asset-generation and editing services behind URPG commands, then import or plop MZ-ready results without absorbing its development payload or unsafe extension boundaries. |
| F24 | Audio, Voice, and Caption Studio | Import, edit, loop, normalize, convert, organize, localize, caption, validate, and assign game audio through non-destructive RPG Maker-aware workflows. |

### 4.2 Improvements

| ID | Improvement | Outcome |
|---|---|---|
| I01 | Preloader resource lifetime | Bounded retention, normal eviction, explicit pinned assets, and stable long-session memory. |
| I02 | Preloader correctness and recovery | Accurate queue completion/progress and fail-open behavior for missing or corrupt preload data. |
| I03 | Menu Builder portability | A complete creator distribution or an explicit runtime-only mode with no dead editor entry point. |
| I04 | Safe menu persistence | Versioned configuration, explicit saves, atomic replacement, backup rotation, and automatic recovery. |
| I05 | Configuration validation | Aggregated, actionable diagnostics and graceful handling of recoverable configuration errors. |
| I06 | Actor preview correctness | Independent row state and correct X/Y placement in actor selection previews. |
| I07 | Typed configuration and restricted scripting | Ordinary fields no longer rely on broad `eval()` execution. |
| I08 | Existing editor usability | Responsive, accessible editor behavior with drafts, dirty state, undo/redo, and controlled persistence. |
| I09 | Asset and plugin catalog integrity | Portable payloads, complete provenance, multi-tag taxonomy, support-file metadata, and licensing status. |
| I10 | First-party release engineering | Canonical source layout, onboarding, deterministic commands, manifests, logging, tests, and release policy. |

## 5. Decisions Required Before Implementation

Phase 0 must record these decisions in an architecture decision log.

| Decision | Recommended direction |
|---|---|
| Product boundary | Keep first-party application/tooling separate from `third_party/`; treat vendor/sample projects as inputs and fixtures only. |
| Supported runtime | Select and document the supported RPG Maker MZ and NW.js versions; reject or warn on unsupported combinations. |
| Menu Builder distribution | Ship the complete sidecar in the creator distribution and omit it from runtime-only game exports. If that cannot be licensed or supported, gate editor launch and document runtime-only behavior. |
| Menu source of truth | Store menus in a versioned, schema-backed first-party configuration artifact; generate RPG Maker integration data from it. |
| License policy | Unknown or restricted redistribution status blocks release by default. Any waiver is explicit, scoped, named, and recorded. |
| Creator Hub architecture | Perform a short spike comparing an NW.js-integrated tool, a local desktop sidecar, and a browser/local-service design. Choose based on filesystem safety, portability, preview fidelity, and maintainability. |
| Mutation model | Use a shared transaction layer for imports, transforms, plugin changes, editor saves, upgrades, and rollback. |
| Configuration scripting | Use typed fields and named actions for standard behavior. Advanced scripts are isolated, clearly labeled, error-contained, and excluded from untrusted inputs. |
| LLM provider strategy | Use a provider-neutral adapter with capability discovery, model/version locks, streaming, cancellation, timeouts, retries, cost limits, and an offline/no-AI mode. Do not embed provider secrets in exported games. |
| Embedded model default | Ship a pinned, checksummed, locally executed **Qwen3.5-4B** 4-bit profile as the capability-first default. Prefer Q5 on systems meeting the recommended memory profile and Q4 on lower-memory supported systems; lazy-load vision. Offer a smaller explicitly limited fallback, but never present a sub-4B fallback as equally capable for autonomous multi-step work. |
| Local inference runtime | Bundle a pinned `llama.cpp` sidecar bound to loopback with a per-session token, hardware discovery, bounded context profiles, cancellation, idle unload, and no unsolicited network access. Keep Transformers.js/ONNX as an optional WebGPU adapter rather than the sole desktop runtime. |
| Tool-call transport | Use an application-owned, schema-constrained action envelope and complete multi-turn tool loop. Validate tool names and arguments, permissions, source versions, step limits, and results in application code. Native provider tool-call parsing may be enabled only for a pinned model/runtime pair that passes the same evaluation corpus. |
| AI execution boundary | The assistant never receives raw filesystem or shell access. It invokes the same typed domain commands as the UI through a capability broker and transaction layer. |
| RAG strategy | Use structured project queries first, exact/lexical search second, semantic retrieval where it adds value, and multimodal retrieval for visual assets. Live mutable state always comes from tools, not retrieved text. |
| Local embedding default | Use a separate pinned **IBM Granite Embedding 97M Multilingual R2** model for local semantic indexing; do not use the chat model as the embedder. Keep lexical identifiers alongside vectors and access retrieval through read-only tools. |
| AI trust policy | Retrieved or imported content cannot grant permissions, alter system policy, or bypass user confirmation. Treat it as untrusted data and preserve citations, hashes, and versions. |
| AI data policy | Record provider/model disclosure, redaction, retention, telemetry, generated-content provenance, and per-action token/cost budgets. Provide an open export path and provider replacement path. |
| Personal asset custody | Copy approved assets into a managed content-addressed vault by default. External links are advanced, visibly non-portable, monitored by hash, and must be resolved or explicitly waived before release. |
| Controller architecture | Feature code consumes semantic input actions, not raw button indices. Use shared contexts, focus navigation, profiles, calibration, glyphs, optional haptics, and deterministic input recording across creator and runtime surfaces. |
| SpriteForge adoption | Integrate selected MIT-licensed service algorithms through a versioned loopback adapter and URPG transactions. Do not copy its `.venv`, vendor/model payloads, generated data, global-script frontend, unrestricted Python plugin loader, launcher auto-install behavior, or separate project database into the product. |
| Project lifecycle and recovery | Transaction undo is not disaster recovery. Provide checksummed whole-project snapshots, automatic pre-risk snapshots, restore-to-new-location, portable project archives, retention/free-space policy, and a bootstrap safe mode that works before optional plugins/AI/services load. |
| Storage and offline policy | Let users choose and relocate project, vault, model, cache, backup, and release roots. Distinguish authored/referenced data from regenerable cache, preflight disk needs, and guarantee a documented no-network core workflow after selected packs are installed. |
| Save compatibility and updates | Version player saves independently from project/config schemas. Require declared compatibility, ordered idempotent migrations, pre-migration backups, signed update metadata, staged install, health check, rollback, anti-replay/downgrade, and an offline installer/update path. |
| Executable trust boundary | Catalog inspection never executes RPG Maker plugins. Runtime plugins, Hub extensions/importers, local services, and external connectors have separate trust models; trust is hash/capability scoped and invalidates when either changes. |
| Secrets, privacy, and data lifecycle | Store credentials in the OS vault and persist only opaque references. Default outbound telemetry/crash upload to off, preview exact payloads, enforce per-project RAG/cloud/retention policy, propagate deletion to derived indexes/caches, and provide project export/forget. |
| External tool interoperability | Optional CLI/MCP/provider adapters reuse the canonical command API and capability broker. They are disabled until explicitly installed/enabled, pin origin/schema/version, require re-consent on capability change, and cannot be installed or re-permissioned by the model. |
| Hosted accounts and cloud sync | Do not require an account or hosted service for GA. Portable archives and F20 operation-history export/import are the supported collaboration path; preserve a provider-neutral encrypted sync adapter boundary, but defer hosted sync until identity, conflict, retention, recovery, legal, and operational ownership are explicitly funded and qualified. |
| Platform/install contract | Windows is the initial GA desktop target and Web remains separately gated. Normal installation works as a standard user without system Python; user data survives repair/update/uninstall unless explicitly selected for deletion. Other platforms are unsupported until qualified. |

## 6. Roadmap Overview

| Milestone | Target | Primary scope | Gate |
|---|---|---|---|
| M0 | Product baseline | I10 foundation and decisions required by all work | G0 Product Baseline |
| M1 | Development safety floor | I01-I07 implementation plus targeted proofs for boot/data/trust boundaries | G1 Development Safety Floor |
| M2 | Portable contracts and representative data | I03, I09, I10, and F10 preflight contracts | G2 Platform Contracts Ready |
| M3 | Creator platform and project-lifecycle foundation | F01 plus shared catalog, transaction, validation, recovery, storage, project, import-job, and input-action services | G3 Platform Foundation |
| M4 | Asset and audio workflow | F02, F03, F21, and F24 | G4 Safe Asset and Audio Workflow |
| M5 | Plugin ecosystem | F04 and F06 | G5 Safe Plugin Workflow |
| M6 | Menu authoring | F05 and I08, building on I03-I07 | G6 Menu Studio Beta |
| M7 | Advanced authoring and playtest | F07 and F08 | G7 Creator Beta |
| M8 | Complete core product scope | F09, F22, and full F10 implementation | G8 Core Scope Feature Complete |
| M9 | Core-scope integration rehearsal | Cross-feature development-mode rehearsals and verification mapping; no clean production build | G9 Core Scope Integrated |
| M10 | Command platform, direct placement, and asset-generation bridge | F11 and F23 plus shared tool/capability, stable-ID, project-graph, and concurrency foundations | G10 Tool, Plop, and Asset Studio Complete |
| M11 | Embedded AI and project knowledge | F12 and F13 | G11 Agent Foundation Complete |
| M12 | Frontier authoring implementation | F14-F20 behind feature flags and final F21-F24 cross-feature closure | G12 All Feature Scope Complete |
| M13 | Batched build, integration, qualification, and release | One immutable candidate plus all deferred test matrices | G13 Frontier General Availability |

M0-M2 establish the safety floor while feature work proceeds in parallel after G0. M3-M8 implement the core creator experience, including project recovery, storage/offline management, personal assets, audio/voice/captions, unified input, save compatibility, safe updates, and trust/privacy controls. M9 rehearses integration without a clean production build. M10-M12 implement direct placement, SpriteForge, AI/RAG, and frontier scope. M13 performs the first clean production build and broad qualification after all feature scope is frozen.

G0-G8 and G10-G12 use the persistent incremental development assembly and bounded verification lanes in Section 9. The first clean production build for a delivery wave and its first broad regression run occur only after all features in that wave are implemented and frozen.

## 7. Milestone Plans

### M0 — Establish the Product Baseline

**Scope:** I10 foundation; prerequisites for F01-F24.

**Deliverables:**

- identify the canonical first-party RPG Maker MZ project or define a deterministic assembly process for it;
- separate first-party source, generated output, vendor inputs, fixtures, reports, and release artifacts;
- establish a persistent incremental/watch development assembly that does not perform a clean production build;
- establish feature flags and route placeholders so incomplete features can integrate continuously;
- document supported RPG Maker MZ/NW.js versions and platform targets;
- document supported app/project/save/plugin/model/runtime/platform combinations, standard-user install locations, secure credential storage, data-retention defaults, network behavior, and the unsupported-platform/storage policy;
- create a minimal first-party fixture project for automated tests instead of relying on bundled sample projects;
- define configuration, catalog, plugin manifest, transaction, diagnostics, and release-manifest schemas;
- define asset-source/import-plan/job schemas plus input-action, context, controller-profile, device-capability, glyph, haptics, and replay schemas;
- define project snapshot/archive, storage-root/budget, audio/voice/caption, save-version/migration, update-channel/manifest, executable inventory, trust grant, privacy/data-lifecycle, support-bundle, and safe-mode schemas;
- define trust zones and a threat/data-flow model for projects, runtime plugins, Hub extensions/importers, local services, models, external connectors, credentials, telemetry, and release/update infrastructure;
- define severity levels: information, warning, blocker, and internal error;
- define guarded logging and release-mode logging behavior;
- define the proposed single command interface for validate, test, catalog, assemble, and release operations;
- create the architecture decision log for the decisions in Section 5.

**Exit criteria — G0 Product Baseline:**

- a clean checkout clearly identifies or reproducibly assembles the runnable first-party product;
- no undocumented sample-project workflow is required;
- root documentation distinguishes source files from generated and vendor content;
- supported versions and platforms are explicit;
- a minimal fixture boots independently of vendor sample projects;
- the persistent incremental development assembly loads all registered feature modules and exposes flags/routes without production packaging;
- initial catalog, configuration, transaction, command/tool, capability, diagnostics, history, and plugin contracts are versioned;
- initial personal-asset import and semantic-input contracts are versioned so individual feature screens cannot invent incompatible paths or raw button mappings;
- initial recovery/archive, storage/offline, audio, save migration, updater, executable-trust, privacy, and supportability contracts are versioned;
- Windows GA/Web-gated platform tiers and the standard-user install/update/uninstall data-preservation contract are explicit;
- no development or test command implicitly performs a clean production build or release package;
- every later milestone has a named schema owner and migration policy;
- the exact checked-in commands are documented only after they exist and have been verified.

### M1 — Establish the Development Safety Floor

**Scope:** I01-I07 and the runtime portion of I10.

The detailed acceptance criteria below define final M13 qualification. G1 requires implementation plus targeted L0/L1 proof only for boot-hang, data-loss, unsafe-write, secret, and trust-boundary protections; it does not run the full matrices or soak suites.

#### I01 — Preloader Resource Lifetime

Implementation tasks:

- audit all ImageManager, AudioManager, bitmap, audio buffer, video, cache, clear, and destroy overrides;
- remove the no-op `ImageManager.clear()` behavior;
- preserve generic engine `destroy()` semantics;
- separate boot-time warming from permanent retention;
- introduce explicit preload groups and a small pinned allowlist;
- stop adding arbitrary later runtime loads to the preload registry;
- expose cache counts, pinned counts, memory estimates, load failures, and evictions to diagnostics;
- release video textures and audio buffers through normal lifecycle paths.

Acceptance criteria:

- ordinary assets are evictable and explicit pinned assets remain available;
- generic bitmap and audio destruction is preserved;
- after warm-up and 30 repeated map/menu cycles, memory is not monotonically increasing and finishes within 15% of the agreed post-warm-up baseline after collection;
- image, audio, and video soak fixtures complete without retained-resource growth outside the approved budget.

#### I02 — Preloader Correctness and Recovery

Implementation tasks:

- replace the shared cumulative/shrinking-queue calculation with explicit queued, active, succeeded, failed, and completed counters;
- complete only after the relevant queues are empty and all reservations reach a terminal state;
- make progress monotonic and bounded from 0 to 100%;
- make missing or corrupt stored preload data fall back to a safe empty/default manifest;
- bound retries and report terminal missing-asset failures;
- cover empty, image-only, audio-only, mixed, and large manifests.

Acceptance criteria:

- every manifest entry is requested exactly once or ends in a reported terminal failure;
- progress reaches 100% only after queued and reserved work settles;
- corrupt JSON, missing storage, and missing assets cannot leave boot waiting indefinitely;
- automated cases cover 0, 1, 4, 5, 6, and 100 image/audio entries, unequal audio/image counts, retry exhaustion, and empty title-video configuration.

#### I03 — Menu Builder Portability

Implementation tasks:

- inventory the HTML, JavaScript, CSS, and image sidecar dependencies;
- add an explicit capability check before editor launch;
- package the complete sidecar in creator installations, subject to the recorded distribution decision;
- exclude development-only editor assets from runtime-only game exports;
- show one actionable playtest message when the sidecar is incomplete or unavailable;
- extend curation validation beyond plugin JavaScript presence.

Acceptance criteria:

- a clean creator install contains and launches every required sidecar file, or editor launch is deliberately disabled;
- complete, partial, and absent sidecars are detected before `window.open`;
- no blank window, missing-page launch, or silent failure is possible;
- all sidecar-relative paths validate on a clean machine.

#### I04 — Safe Menu Persistence

Implementation tasks:

- introduce a versioned menu configuration artifact independent of `plugins.js`;
- keep edits in memory until an explicit Apply/Save action;
- validate and serialize to a temporary file before replacement;
- use atomic replace where supported and a safe fallback where it is not;
- rotate backups and automatically restore on failed replacement;
- retain unsaved drafts when a save fails;
- make generator output reproducible and boot-testable;
- use the shared transaction journal so recovery can explain what happened.

Acceptance criteria:

- successful saves produce valid configuration and bootable generated integration data;
- failed or interrupted saves leave the previous file byte-for-byte intact or restore it automatically;
- permission denial, invalid output, replace failure, and simulated interruption tests pass;
- 100 sequential saves do not corrupt data or lose the recovery chain;
- 200 field input events cause zero filesystem writes before Apply and at most one transaction afterward.

#### I05 — Configuration Validation and Fallbacks

Implementation tasks:

- define versioned schemas and semantic validators for menus, windows, database references, dimensions, actions, and conditions;
- run the same validator in the editor, project checks, runtime development mode, and Release Assistant;
- report all detected issues together with menu ID, field path, severity, and remediation;
- skip invalid optional entries and preserve unaffected scene behavior;
- provide controlled developer-facing failures for missing required structures;
- support strict validation for release and soft validation for iterative editing;
- add schema migrations and round-trip fixtures for existing configurations.

Acceptance criteria:

- partial and malformed configs cannot crash unrelated menu sections;
- invalid item, weapon, armor, event, switch, and variable references identify the exact offending field;
- missing required structures produce one controlled, actionable failure;
- minimal, partial, malformed, unknown-field, empty-array, and fuzzed-scalar fixtures pass in strict and soft modes;
- existing supported menu data migrates and round-trips without semantic loss.

#### I06 — Actor Preview Correctness

Implementation tasks:

- move actor preview state from the shared window field to the row/sprite that owns it;
- use the configured Character Y value for vertical placement;
- review battler previews for the same shared-state pattern;
- verify refresh, scroll, visibility, selection, and scene reload behavior;
- ensure default scale values remain visible.

Acceptance criteria:

- four or more actor rows retain independent character and battler state;
- X and Y offsets work independently;
- direction, scale, visibility, scrolling, refresh, and scene reload do not cross-contaminate rows;
- state assertions and visual regression fixtures pass.

#### I07 — Typed Configuration and Restricted Scripting

Implementation tasks:

- inventory and classify every `eval()` call by boolean, number, color, ID, condition, action, or advanced script;
- replace ordinary scalar parsing with typed parsers and clear defaults;
- replace common condition/action scripts with named, validated operations;
- isolate remaining advanced hooks behind an explicit advanced-script field and an error boundary;
- prevent untrusted catalog, asset, plugin metadata, or translated text from becoming executable code;
- remove or bound per-frame script execution where possible;
- record migration behavior when legacy expression semantics cannot be preserved exactly.

Acceptance criteria:

- boolean, number, color, enum, and database-ID fields never invoke `eval()`;
- ordinary menus can be exercised with an instrumented `eval` that throws and still function;
- advanced hook failure identifies its menu and field without crashing unrelated UI;
- edge cases cover `false`, `0`, empty text, `NaN`, `Infinity`, malformed expressions, and thrown hooks;
- the approved advanced-script surface is documented and test-covered.

#### Cross-cutting trust and bootstrap safety floor

Implementation tasks:

- ensure catalog/header/dependency inspection treats JavaScript, Python, model files, archives, and connector descriptions as data and executes none of them;
- separate RPG Maker runtime plugins from Hub extensions/importers/generators; unknown or changed executable hashes require a new hash- and capability-scoped trust decision;
- define out-of-process extension/importer capability tokens, approved filesystem roots, egress-denied default, minimal child environment, resource/time/output limits, and termination behavior;
- establish an OS-credential-vault adapter so project/config files store only opaque secret references and child processes receive allowlisted scoped credentials only when required;
- establish a bootstrap safe-mode path and crash-loop marker that can reach read-only project health before plugins, AI, SpriteForge, external connectors, or derived indexes load;
- establish local-only/default-off outbound telemetry and crash reporting, with a byte-for-byte payload preview required before any later upload;
- define the loopback-service supervisor contract: random port/nonce, token transfer outside command-line/environment/logs, strict Host/Origin/CSRF checks, PID/version handshake, bounded requests/concurrency, minimal environment, and orphan cleanup.

Acceptance criteria:

- indexing a malicious executable-plugin fixture executes zero plugin code;
- a denied importer/extension cannot read or write outside granted roots, use ambient credentials, or make network requests;
- safe mode opens Recovery Center after a seeded crash loop without loading the failing optional subsystem;
- a seeded credential canary appears in none of the project, prompt, transcript, history, log, crash, backup, or diagnostic artifacts inspected by the targeted proof;
- a clean installation makes zero unsolicited outbound requests during the representative local workflow.

**Exit criteria — G1 Development Safety Floor:**

- corrupt or absent preload data cannot hang the representative fixture;
- writes use staging, source-version verification, backup, and rollback in the representative failure proof;
- missing editor sidecars fail with one actionable message;
- untrusted metadata cannot enter executable scripting or elevate permissions;
- each I01-I07 implementation is integrated behind its intended flag/contract and has mapped deferred acceptance IDs;
- the executable-inspection, extension-capability, secret-reference, safe-mode, loopback-supervisor, and default-off egress contracts have targeted proofs and named owners;
- the 30-cycle soak, 100-save sequence, complete failure matrix, and release-log audit remain deferred to M13.

### M2 — Make Platform Contracts and Representative Data Usable

**Scope:** I09, remaining I10 foundation, and an F10 preflight MVP.

#### I09 — Catalog and Distribution Integrity

Implementation tasks:

- replace absolute or broken junction-based distribution paths with portable project-relative content or a deterministic materialization step;
- reconcile the catalog to all 72 archives, 15,264 pack files, and 396 loose files;
- assign stable IDs and retain source-relative path, hash, pack, author, source URL, and provenance;
- make `unknown` an explicit value rather than guessing missing metadata;
- add dimensions, alpha, probable animation/frame grid, tile size, audio duration, type, style/theme tags, and RPG Maker compatibility where derivable;
- replace substring-only single categories with token-aware multi-tags, confidence, and manual overrides;
- add plugin manifests for dependencies, ordering, versions, supported MZ versions, conflicts, commands, parameters, and support files;
- record license, redistribution, modification, attribution, and release-inclusion status;
- create a used-content ledger and generated credits/NOTICE/BOM output.

Acceptance criteria:

- deterministic catalog rebuilds reconcile exactly to the canonical report counts and produce stable IDs;
- every catalog path resolves on a clean machine or has an explicit unavailable state;
- all 396 loose files participate in the same search, license, and release workflow;
- taxonomy regression tests prevent matches such as `ui` inside `ruin`, permit multiple tags, and expose all `misc` packs for review;
- all 44 CGMZ manifest plugins and their unlisted support files are modeled;
- all declared `CGMZ_Core` dependencies and support-file requirements are captured;
- every record has an explicit verified, restricted, unknown, or waived license status;
- unknown/restricted shipping content blocks release unless an auditable waiver exists.

#### F10 MVP — Read-only Release Preflight

Implementation tasks:

- scan missing and case-mismatched resources;
- validate plugin dependencies, order, support files, versions, and hashes;
- validate menu schemas and referenced database IDs;
- validate player-save format/version declarations, plugin save-impact metadata, supported compatibility horizon, and required migration registration;
- validate license and attribution readiness;
- inventory every executable/native/JavaScript/Python/plugin/model/updater payload and reject unmanifested or disallowed executable formats;
- validate update-manifest/channel/signature metadata schemas, anti-downgrade version ordering, platform/install location policy, and whole-product software/model/asset BOM inputs without performing an update;
- report source hash drift, unused assets, oversized content, and unresolved warnings;
- emit human-readable and machine-readable results without modifying the project;
- define automation-friendly ready, warning, and blocker exit states.

Acceptance criteria:

- a clean fixture reports zero blockers;
- fault-injection detects every seeded missing asset, case mismatch, broken config, missing dependency, wrong order, missing support file, unknown shipping license, hash drift, missing save migration, unmanifested executable, and malformed update manifest;
- preflight makes no project changes;
- unchanged inputs produce the same file list and per-file hashes;
- `release ready` is impossible while blockers remain.

**Exit criteria — G2 Platform Contracts Ready:**

- catalog, plugin graph, provenance/license, used-content ledger, save-compatibility, executable-inventory/BOM, update-manifest, platform/install, and preflight schemas are versioned and usable by feature teams;
- representative assets, loose files, plugins, dependencies, support files, license states, and broken references exercise every contract;
- portable-path materialization and deterministic regeneration are implemented;
- feature work may proceed after G0; incomplete G1/G2 items block only dependent unsafe runtime, mutation, catalog, or release paths;
- full 15,660-record reconciliation, clean-machine portability, complete license review, and exhaustive fault injection remain deferred to M13.

#### Common Feature Implementation Gate for M3-M12

A feature implementation gate proves scope completion, not release qualification. It requires:

- the feature is reachable in the persistent incremental assembly behind its intended flag;
- its primary journey works with a representative fixture;
- persisted data uses a versioned schema and has migration/rollback behavior;
- mutations use the shared transaction and capability layers;
- diagnostics identify failures without silent data loss;
- no placeholder controls, mock-only production paths, dead routes, or required unfinished implementations remain;
- every acceptance criterion maps to a fixture, verification ID, expected evidence, and execution lane;
- only the affected-check, feature-proof, and cluster-rehearsal lanes have run.

Passing an M3-M12 gate does not trigger a clean build, packaging run, full regression, complete inventory traversal, platform matrix, visual/accessibility matrix, AI/security evaluation suite, or soak.

### M3 — Build the Creator Platform Foundation

**Scope:** F01 and the shared services/contracts used by F02-F24. The stable-ID, reference, command, capability, history, external-change, recovery, storage/offline, import-job, and semantic-input foundations begin here so they are not retrofitted after feature work.

#### F01 — Creator Hub

Deliverables:

- project discover/import/create/open/select/clone/move/rename and recent-project management with compatibility/migration preview;
- a resumable/skippable first-run wizard covering project discovery or a disposable sample sandbox, hardware/disk/network capabilities, storage roots, offline/privacy/model choices, controller setup, and a guided first-task checklist;
- a project health dashboard showing runtime version, catalog status, plugins, menus, warnings, and release readiness;
- navigation shells for Assets, Audio/Voice, Plugins, Menus, Quests, Playtest, Accessibility, Localization, AI Assistant, Jobs/Downloads, Storage, Privacy/Trust, Recovery, History, and Release;
- universal search/command palette, recent actions, contextual help, and visible background-job/download status across the Hub;
- stable IDs and the initial cross-domain reference graph;
- a canonical typed domain-command API used by both UI actions and future tool callers;
- shared project detection, catalog, validation, diagnostics, transaction, undo, history, capability, and notification services;
- shared background import/job services and the first `InputActionRegistry`, input-context stack, device adapters, binding resolver, spatial-navigation service, profile store, glyph service, and synthetic input adapter;
- source-version/external-change detection before every write;
- read-only mode when the project cannot be safely mutated;
- dry-run views and explicit confirmation for mutating operations;
- Recovery Center with checksummed crash-consistent whole-project snapshots, content-addressed incremental storage, retention/free-space policy, automatic pre-risk snapshots, integrity state, restore-to-new-location default, and interrupted-action/update/migration visibility;
- portable project archives containing the canonical project, used managed assets, locks, schemas, provenance, and explicit unresolved external-link gaps while excluding caches, models, temporary output, and secrets;
- a Storage and Offline Pack Manager showing per-category disk usage for projects, vault, derived output, thumbnails, embeddings, model/content packs, SpriteForge, jobs, backups, and releases; support budgets, pinning, low-disk preflight, dry-run cleanup, atomic relocation, and resumable/checksummed offline pack install/remove;
- bootstrap safe mode with crash-loop detection, read-only health, optional-subsystem disable controls, and derived cache/index rebuild that cannot modify authored content;
- a product-wide no-network mode and disclosure surface: after selected packs are installed, core create/edit/import/plop/playtest/preflight/package/help workflows make no outbound requests and hosted-only features fail clearly;
- keyboard navigation, visible focus, zoom support, and scalable layout from the first usable build.

Acceptance criteria:

- a creator can select a valid project and see an accurate health summary without changing it;
- invalid or unsupported projects receive actionable diagnostics;
- all write operations use the shared domain commands, capability checks, and transaction layer and produce a preview, journal, reference update, and rollback point;
- renames/deletes can inspect inbound references through stable object IDs;
- stale external state rejects or rebases a planned write rather than silently overwriting it;
- interrupted operations are detected on next launch;
- an automatic pre-risk snapshot and a manual portable archive restore to a different clean root open with equivalent canonical project state;
- corrupt or interrupted snapshots never appear restorable, and pruning cannot remove the last known-good snapshot or data referenced by a retained snapshot;
- cleanup never treats authored/referenced content, linked sources, model licenses, or the sole recovery point as regenerable cache;
- storage relocation interruption leaves the prior location valid, and inadequate disk space is reported before mutation;
- no-network mode completes the representative core workflow with outbound traffic blocked and no deferred telemetry/update upload;
- repeated startup failure reaches safe mode without loading the seeded failing plugin/AI/SpriteForge/index subsystem;
- no screen requires manual browsing of vendor/sample directories;
- core workflows remain usable at 1024x600 and 200% zoom with keyboard-only navigation.

**Exit criteria — G3 Platform Foundation:**

- F02-F24 can consume the same stable-ID, reference, command, catalog, project, validation, capability, transaction, recovery, storage/offline, history, diagnostics, import-job, and input-action APIs;
- no feature team needs to implement its own filesystem mutation or backup system;
- the UI exercises the same typed command contracts that F12 will expose as model tools;
- a clean project can be opened, assessed, snapshotted, archived, safely relocated, diagnosed in safe mode, and restored through the Hub.

### M4 — Deliver the Asset and Audio Workflow

**Scope:** F02, F03, F21, and F24.

#### F02 — Visual Asset Library

Deliverables:

- lazy thumbnail grid with transparent/checkerboard and zoom views;
- sprite animation and spritesheet previews;
- audio audition controls;
- full-text search and filters for type, pack, tag, dimensions, license, compatibility, and project usage;
- favorites, collections, comparison, contact sheets, and saved project palettes;
- an explicit preview failure reason for unsupported or corrupt files;
- `Use this asset` actions that map supported assets to appropriate RPG Maker destinations and object types;
- dry-run collision handling with add, rename, replace, and skip choices;
- transactional import, post-import validation, install manifests, and undo.

Acceptance criteria:

- all 15,660 pack-plus-loose records are discoverable or have an explicit unavailable state;
- every decodable PNG/audio item previews successfully and every failure is explained;
- search/filter p95 is under 250 ms after index load on the current catalog;
- browsing remains responsive with the largest icon pack;
- no import silently overwrites an existing file;
- failed imports leave the destination unchanged or automatically roll back;
- undo restores a fixture project byte-for-byte.

#### F03 — Asset Transformation Studio

Deliverables:

- non-destructive crop and frame-grid slicing;
- grid detection and animation preview;
- nearest-neighbor resizing and palette swaps;
- icon-atlas generation;
- RPG Maker sprite and tileset layout validation/conversion;
- derived-asset provenance recording input hash, operation, parameters, output hash, and destination;
- transform presets and one-click undo.

Acceptance criteria:

- source/vendor files are never overwritten;
- identical input and parameters produce pixel-identical or byte-identical output as defined per format;
- nearest-neighbor transforms introduce no blended colors and preserve alpha on golden fixtures;
- invalid dimensions, partial grids, bounds errors, destination collisions, and oversized atlases are caught before import;
- transformed outputs remain traceable to their original sources and license obligations.

#### F21 — Bring Your Own Assets and Personal Asset Vault

Deliverables:

- a prominent `Add Assets` action and controller/keyboard-accessible drop zone on every asset surface;
- import from files, folders, supported archives, clipboard images, SpriteForge results, and an optional watched inbox;
- separate `My Assets`, `Project Assets`, generated results, and vendor-catalog collections backed by the same catalog contract;
- copy into a managed content-addressed vault by default; offer external linking only as an advanced choice with missing-source and portability warnings;
- a staging review showing previews, detected type/layout, proposed role and destination, required conversion, duplicate/collision state, license declaration, expected writes, and rollback plan;
- deterministic type/layout detection with manual correction, reusable import presets, and batch tag/rename/classify/license actions;
- content-hash duplicate detection plus optional visually similar suggestions that never auto-delete or auto-merge;
- background, cancellable, resumable import jobs that preserve immutable sources and commit project changes only after the plan is valid;
- reference-aware replace, relink, and remove operations with complete inbound-impact previews;
- allowlisted, resource-bounded decoders and sandboxed/versioned importer adapters; user asset archives cannot load plugins or executable code;
- immediate interoperability with F03 transforms, F11 plopping, F13 indexing, F15 similarity search, F23 generation, and F10 provenance/credits.

Initial typed commands:

- `asset.source.inspect`, `asset.import.plan`, `asset.import.commit`, `asset.import.cancel`, and `asset.import.resume`;
- `asset.classify`, `asset.license.set`, `asset.watch.configure`, and `asset.validate`;
- `asset.replace`, `asset.relink`, `asset.remove`, and `asset.plop`.

Every `AssetImportPlan` records source fingerprints, detected types, proposed managed IDs/destinations, conversions, duplicate and collision decisions, license state, expected writes, reference changes, warnings, resource limits, and rollback ID.

Acceptance criteria:

- a first-time user can add one valid local asset and plop it using no more than three required decisions;
- after commit, a custom asset is searchable, previewable, transformable, ploppable, and represented in release provenance;
- default copied assets remain usable when the original external source is moved or deleted;
- unresolved linked assets clearly show their state and block a portable release unless resolved or explicitly waived;
- identical files cannot create accidental duplicates, and same-name/different-content collisions require an explicit add, rename, replace, or skip decision;
- replace previews every inbound reference and updates the asset plus references atomically;
- cancel, decoder failure, interruption, or import validation failure leaves the project unchanged; undo restores it byte-for-byte;
- archive traversal, absolute paths, out-of-root links, executable payloads, decompression bombs, malformed media, and configured size/count limits are blocked or quarantined with actionable diagnostics;
- source files are never modified, license rights are never inferred merely from possession, and the full workflow works with AI disabled.

#### F24 — Audio, Voice, and Caption Studio

Deliverables:

- F21-backed import and role detection for music, ambience, musical effects, sound effects, and dialogue voice, with correction before commit;
- waveform and spectrogram preview with trim, split, silence removal, fades/crossfades, gain, peak/loudness analysis, channel/sample-rate conversion, and runtime-target codec/quality presets;
- loop-in/loop-out markers, click/seam diagnostics, crossfade assistance, repeated-loop audition, and preservation/export of supported loop metadata;
- non-destructive source/revision handling, deterministic transform manifests, batch presets, size/quality comparison, collision planning, provenance, license, and one-step undo;
- searchable audio collections, audition queues, favorites, tags, usage/reference lookup, and direct assignment/plopping to maps, events, menus, database records, dialogue, and quests;
- stable voice-line IDs tied to dialogue/localization keys, with character/cast, language, take, approval, pronunciation, replacement, and coverage status;
- caption tracks with speaker identity, dialogue text, non-speech/SFX labels, optional timing, visual-sound-indicator metadata, and locale coverage;
- optional speech-to-text/text-to-speech/provider adapters only through the normal capability/data/consent system; no generated voice is required for the core workflow, and every generated/converted take records model/provider, voice-rights declaration, consent, parameters, and provenance;
- audio QA for decode failure, corruption, clipping, unintended silence, channel mismatch, loop discontinuity, missing runtime variant, case/path collision, missing required voice/caption, and unknown redistribution status;
- controller/keyboard-accessible editing and audition with non-audio visual state for transport, selection, meters, loop markers, and warnings.

Initial typed commands:

- `audio.inspect`, `audio.import.plan`, `audio.transform.plan`, `audio.transform.commit`, `audio.loop.preview`, and `audio.validate`;
- `audio.assign`, `audio.replace`, `audio.batch`, and `audio.undo`;
- `voice.line.list`, `voice.take.add`, `voice.take.approve`, `voice.coverage`, `caption.edit`, and `caption.validate`.

Acceptance criteria:

- a first-time user can import one music/ambience track, one sound effect, and one voiced/captioned dialogue line, assign them, playtest them, and undo without manual audio-folder or event-command surgery;
- source audio remains byte-identical, every derived file has a deterministic transform/provenance record, and cancel/failure leaves the project unchanged;
- repeated loop audition has no blocker-level discontinuity under the approved measurement/listening rule, and clipping/invalid silence/missing target variants are detected before release;
- voice/caption status follows stable dialogue/localization IDs across text edits and reports missing/stale locale coverage;
- captioned/visual alternatives remain usable when audio is muted or unavailable;
- generated or externally processed voice cannot be committed without explicit rights/consent/provenance fields, and the complete workflow works without AI or network access.

**Exit criteria — G4 Safe Asset and Audio Workflow:**

- a new user can find or add, preview/audition, verify, transform, import, assign/plop, and undo a visual or audio asset without manual project-folder work;
- voice/caption coverage is connected to dialogue/localization IDs rather than filenames alone;
- all imported and derived visual/audio content is represented in project and release manifests.

### M5 — Deliver the Plugin Ecosystem

**Scope:** F04 and F06.

#### F04 — Dependency-aware Plugin Manager

Deliverables:

- searchable plugin catalog with description, provenance, version, target MZ versions, commands, and configuration summary;
- static/header/source inspection that never evaluates or loads plugin code during cataloging, preview, dependency extraction, or RAG indexing;
- explicit separation of RPG Maker runtime plugins from Hub extensions/importers, including executable type, origin, immutable hash, declared capabilities, trust state, and trust rationale;
- dependency and ordering graph;
- enable, disable, configure, install, update, and remove plans;
- support-file bundling and validation;
- conflict and compatibility diagnostics;
- hash- and capability-scoped trust prompts; changed code, origin, or requested capability invalidates prior trust and requires a new decision;
- save-impact fingerprints and compatibility declarations tied to the plugin lock, with required `SaveMigrationRegistry` entries for supported save-affecting changes;
- lossless `plugins.js` read/write behavior using the shared transaction layer;
- a project lockfile containing selected source hashes, order, versions, support files, and acknowledged warnings;
- rollback and repeatable installation from the lockfile.

Acceptance criteria:

- all 44 CGMZ manifest plugins and all required support files are represented;
- `CGMZ_Core` is placed above every dependent CGMZ plugin;
- selecting CGMZ PixiFilters includes its support JavaScript and displacement image requirements;
- MZ version mismatches are visible before installation;
- catalog and RAG inspection execute zero plugin JavaScript, and a malicious plugin fixture cannot affect the host before an explicitly trusted playtest/runtime launch;
- untrusted or changed executable hashes cannot be enabled silently, and Hub extensions cannot inherit project-wide filesystem, network, environment, or credential access;
- unknown existing plugin entries and parameters survive a read/write cycle losslessly;
- repeated application is idempotent and creates no duplicates;
- install, enable, disable, update, and undo are transactional;
- a save-impacting plugin plan without a declared compatible migration or explicit supported-save break is a release blocker;
- the lockfile and release executable inventory identify every enabled plugin by exact hash and trust state;
- the fixture project boots to title after every supported plan.

#### F06 — Gameplay Recipe Gallery

Initial recipes:

1. Crafting + Professions + Toasts;
2. Achievements + Extra Stats;
3. Encyclopedia/Bestiary + Drop Tables;
4. Fast Travel;
5. Title and Save polish;
6. Screenshots + Changelog + Itch launch experience.

Deliverables:

- plain-language explanation and visible player outcome;
- required plugins, support files, ordering, versions, conflicts, generated data, and save implications;
- safe defaults, guided parameter forms, sample data/events, and a preview/test checklist;
- install, customize, validate, remove, and rollback operations;
- recipe versions locked to tested plugin hashes.

Acceptance criteria:

- each initial recipe installs from a clean fixture with no more than five required user decisions;
- every recipe is fully removable or reversible;
- enum, number, file, and database-reference fields validate before writing;
- reapplying a recipe is idempotent;
- automated smoke tests open the recipe's primary command/scene without unguarded errors;
- recipe documentation identifies save-data and upgrade consequences.

**Exit criteria — G5 Safe Plugin Workflow:**

- a creator can understand, install, configure, test, lock, reproduce, and remove a plugin or recipe without hand-editing `plugins.js`;
- the lockfile recreates the same plugin hashes and order on a clean fixture.

### M6 — Deliver Menu Studio

**Scope:** F05 and I08. Depends on G1-G3.

#### I08 — Modernize Existing Editor Behavior

Implementation tasks:

- repair invalid/incomplete HTML metadata and event attributes;
- replace global styling with semantic components and consistent design tokens;
- add labels, descriptions, units, validation messages, searchable fields, and contextual help;
- keep changes in an in-memory draft until Apply/Save;
- add dirty-state indicators, close/reset confirmation, autosaved drafts, and recoverable sessions;
- debounce preview recompilation;
- add undo/redo and named revisions;
- support full keyboard operation, visible focus, accessible names, and touch targets;
- remain usable at 1024x600 and 200% zoom and meet WCAG AA contrast for the creator UI.

#### F05 — WYSIWYG Menu Studio

Deliverables:

- drag, resize, duplicate, align, distribute, zoom, grid, guides, snapping, and layer controls;
- reusable buttons, actor cards, gauges, labels, images, videos, windows, and navigation components;
- starter templates and team-shareable component/menu presets;
- live preview against a safe fixture/project data snapshot;
- responsive anchors, percentages, constraints, safe areas, and target-resolution presets;
- keyboard, controller, touch, and focus-order preview;
- visual condition/action builder with an advanced-script escape hatch;
- import/export of versioned menu configurations;
- diff, revision history, migration preview, and restore;
- validation that blocks unsafe saves while allowing recoverable drafts.

Acceptance criteria:

- a creator can build and preview a representative menu without hand-editing JSON or `plugins.js`;
- undo/redo covers creation, deletion, movement, resize, property edits, and condition/action changes;
- invalid states identify the exact component and field before Apply;
- responsive previews cover 16:9, 16:10, ultrawide, and a compact/handheld profile;
- controller and keyboard focus paths are visible and testable;
- menu configuration import/export round-trips without semantic loss;
- save failure retains the complete draft and restores the prior applied version.

**Exit criteria — G6 Menu Studio Beta:**

- representative new users can create, preview, validate, save, export, reopen, revise, and recover a menu without manual file surgery;
- affected Menu Builder checks and one representative menu journey pass; full regression, visual, and accessibility matrices remain deferred to M13.

### M7 — Add Advanced Authoring and Live Playtest

**Scope:** F07 and F08.

#### F07 — Visual Quest and Dialogue Builder

MVP deliverables:

- versioned graph schema for dialogue, quest state, conditions, actions, rewards, and scene transitions;
- nodes for text, choices, branches, switches, variables, inventory, database references, common events, rewards, and quest state;
- graph validation for unreachable nodes, missing targets, invalid references, dead ends, and cycles requiring explicit intent;
- searchable variable/switch/database pickers;
- localization keys instead of duplicated hard-coded translated text;
- stable F24 voice-line/caption IDs, take/locale coverage state, speaker/SFX labels, and preview links for every voiced or captioned node;
- import/export and migration support;
- deterministic generation of the selected RPG Maker representation;
- a deliberately limited and clearly marked advanced-script node.

Acceptance criteria:

- creators can build, validate, preview, export, reopen, and revise a branching quest/dialogue fixture;
- invalid references and unreachable required nodes are caught before export;
- generated output is stable for unchanged graph input;
- graph changes integrate with undo, transactions, diagnostics, localization, and release manifests;
- editing dialogue text preserves the stable voice/caption identity and marks affected takes/locales stale rather than silently detaching them;
- advanced script failures are contained and attributed to the exact node.

#### F08 — Live Playtest Lab

Deliverables:

- development-only overlay and separate Hub view;
- map travel and safe test-start presets;
- switch, variable, inventory, actor, quest, and menu-state inspection;
- safe hot reload for schemas/configurations explicitly marked reloadable;
- scene load-time, frame-time, memory, cache, preload hit/miss, missing asset, and plugin failure diagnostics;
- snapshot and restore of disposable playtest state;
- Recovery Center integration for crash-loop detection, safe launch before optional subsystems, selective plugin/AI/SpriteForge/connector disable, changed-plugin bisect assistance, and rebuild of regenerable caches/indexes;
- stable error IDs and an offline support-bundle builder with exact payload preview, build/artifact hash, platform profile, plugin/model locks, relevant journal entries, and bounded recent logs;
- local-only/default-off crash telemetry with separate consent from general diagnostics; exclude project content, prompts, assets, raw device IDs, credentials, and memory dumps unless individually selected;
- repeatable smoke scenarios for title, save/load, menus, recipes, quest graphs, and release-critical scenes.

Acceptance criteria:

- the Playtest Lab cannot be enabled accidentally in a production package;
- unsafe state mutations require confirmation and remain limited to the active disposable test session;
- hot reload refuses incompatible schema/runtime changes and explains why;
- support bundles identify versions, artifact hash, plugin/model locks, scene, stable errors, timings, and memory without leaking secrets, unrelated paths, project content, or raw identifiers;
- the bytes previewed for an optional upload exactly equal the bytes sent; withdrawing consent stops upload and clears any bounded local queue;
- a seeded startup crash reaches read-only Recovery Center and can disable or bisect the failing optional subsystem without changing the project;
- deleting/rebuilding derived caches and indexes cannot affect authored data;
- a repeatable scenario can reproduce a defect from a clean fixture state.

**Exit criteria — G7 Creator Beta:**

- a creator can author a representative voiced/captioned quest, inspect it during playtest, reproduce failures, recover through safe mode, and export a previewed redacted support bundle;
- release packages prove that all development-only controls are absent or disabled.

### M8 — Complete Accessibility, Localization, and Release Automation

**Scope:** F09, F22, and full F10. Accessibility and controller-parity requirements apply to every earlier UI even though the center and exported-runtime integration complete here.

#### F09 — Accessibility and Localization Center

Deliverables:

- keyboard/controller remapping and conflict detection;
- UI scale, text scale, contrast themes, color-blind-safe palettes, reduced motion, text speed, subtitle/caption, and focus-indicator settings;
- native accessibility-tree name/role/state/value/live-region semantics and structured alternatives for custom map, graph, waveform, timeline, and other canvas surfaces;
- forced-colors/system-high-contrast support, non-color status cues, photosensitivity controls for flash/shake/parallax, adjustable/disable timeouts, and no precision-only gesture requirement;
- hearing support through F24 speaker/SFX captions, visual sound indicators, mono output, independent volume channels, and complete muted-audio alternatives;
- translatable string extraction with stable keys;
- locale import/export and missing/stale translation detection;
- font fallback and glyph coverage checks;
- right-to-left/bidirectional layout, IME composition, locale-aware plural/date/number formatting, and mixed-script text fixtures;
- pseudo-localization and text expansion testing;
- layout overflow, clipping, focus order, target size, and contrast checks;
- Menu Studio and game-preview integration for accessibility/input/locale profiles.
- optional simplified language/instruction mode and consistent step/progress/error summaries for cognitively demanding workflows.

Acceptance criteria:

- core creator workflows are completable with keyboard only;
- the core creator workflow is completable with a supported Windows screen reader, and every essential custom canvas operation has an equivalent structured control/list/form path;
- supported game menus are testable with keyboard, controller, and touch profiles;
- creator UI meets WCAG AA contrast and remains usable at 200% zoom;
- pseudo-localized and long-string fixtures expose no unresolved blocker-level clipping in release-critical screens;
- missing glyphs, missing translations, duplicate keys, and stale translations are visible before release;
- RTL/IME/plural/number/date fixtures preserve input and meaning without blocker-level layout or cursor-order defects;
- reduced-motion/photosensitivity modes disable or substitute every animation/effect classified as nonessential or unsafe;
- muted/mono audio profiles retain required dialogue, speaker, SFX, objective, and hazard information through captions or visual alternatives.

#### F22 — Unified Input and Controller Platform

F22 begins as shared infrastructure in M0/M3 and closes here only after every core creator surface and the exported-game bridge consume it.

The baseline adapter follows the [W3C Gamepad specification](https://www.w3.org/TR/gamepad/). [Steam Input](https://partner.steamgames.com/doc/features/steam_controller) is an optional distribution adapter layered over the same semantic actions, not the product's universal input API.

Architecture:

- `InputActionRegistry`: versioned semantic actions; feature code never assumes physical button indices;
- `InputContextStack`: explicit Hub, modal, asset grid, canvas, menu preview, playtest, generated-game, and text-entry contexts with deterministic priority;
- `DeviceAdapter`: W3C Gamepad baseline plus keyboard/mouse/touch, synthetic replay, and optional Steam Input/native adapters;
- `InputRouter` and `BindingResolver`: axes, chords, thresholds, repeat, context priority, conflict detection, and duplicate-event suppression;
- `SpatialNavigationService`: deterministic focus graphs with explicit overrides, focus restoration, and navigation linting;
- `ControllerProfileStore`: versioned bindings, calibration, device-family overrides, confirm/cancel convention, recovery profile, and migrations;
- `GlyphService`: Xbox, PlayStation, Nintendo-position, generic, keyboard, and touch prompt families with automatic and manual selection;
- `HapticsService`: capability-gated effects and strength/off controls that safely no-op when unavailable;
- `InputRecorder/Replayer`: deterministic semantic-action traces shared by F08 and F16 without storing raw device telemetry by default;
- `ControllerDiagnostics`: live controls, axes, dead zones, active context, resolved bindings, focus target, and recent semantic events.

Creator experience:

- controller onboarding, calibration, test, remapping, conflict repair, reset, and an always-available recovery chord/profile;
- hot-plug, disconnect/reconnect, last-used-device detection, active-controller ownership, and multiple connected-device selection without restart;
- controller-native grids, trees, tabs, dialogs, menus, sliders, context menus, import staging, asset preview, and release preflight;
- grab/move/drop canvas editing, snapping, coarse/fine movement and resize modifiers, context actions, confirm/cancel, and undo without pointer emulation;
- stable glyph switching with drift/noise hysteresis, persistent action hints, haptic controls, dead-zone/curve/trigger/repeat settings, and hold/toggle accessibility behavior;
- an OS/on-screen keyboard handoff for controller text entry, while all non-text core operations remain controller-completable.

Generated-game integration:

- export a versioned semantic-input manifest and small RPG Maker runtime bridge while preserving expected `ok`, `cancel`, directions, menu, and page actions;
- let plugins register additional semantic actions through a declared contract rather than raw key/button hooks;
- provide player remapping, glyph prompts, last-used-device switching, disconnect handling, calibration, haptics settings, persistence, and profile migration in title/pause/options flows;
- let the Release Assistant detect unbound required actions, conflicts, unreachable focus targets, missing glyph assets, duplicate Steam/direct events, and unsupported capability assumptions;
- generate an optional Steam Input action manifest only for an enabled Steam distribution target; Steam Input is not a Creator Hub or non-Steam dependency.

Assistant-facing tools are limited to `input.listDevices`, `input.inspectProfile`, `input.planRebind`, `input.applyProfile`, `input.resetProfile`, `input.validateNavigation`, `input.inspectFocusGraph`, and `input.runReplay`. They do not grant raw device capture or system-wide remapping.

Acceptance criteria:

- the core creator journey works controller-only: open a recent project, add an asset through the in-app picker, preview/transform/plop it, undo, install a recipe, inspect a menu, start playtest, and run preflight;
- every core interactive control is focusable/reachable, and canvas placement, movement, resizing, confirmation, cancellation, and undo work without mouse emulation;
- connect/disconnect cannot commit, cancel, or corrupt a transaction, and supported devices work without restarting;
- remapping cannot leave navigation, confirm, cancel, or recovery unbound; profiles survive restart and supported schema migrations;
- calibrated noise cannot navigate, held-direction repeat is predictable, glyph selection does not oscillate, and supported UI input is visible by the next rendered frame;
- haptics-off is always honored and unsupported haptics safely no-op;
- generated-game title, save/load, options, menus, dialogue, recipes, and accessibility settings are controller-completable;
- identical initial state plus an input trace reproduces the same semantic actions and focus path;
- full raw device identifiers and traces never enter telemetry without explicit diagnostics consent;
- M13 Windows qualification covers Xbox-style, DualShock 4/DualSense, Switch Pro, and a standards-compliant generic controller; Steam Deck/Steam Input and Web have explicit gates rather than implied support.

#### F10 — Full Release Assistant

Deliverables:

- all M2 preflight checks;
- reference and case-sensitivity scan;
- plugin lock, ordering, compatibility, config, save impact, registered save migrations, compatibility horizon, and support-file checks;
- a `SaveMigrationRegistry` with independent save-format/version stamps, plugin-lock fingerprints, ordered/idempotent forward migrations, pre-migration backup, unsupported-newer-save rejection, and declared supported-version matrix;
- used-content license/credits/NOTICE/BOM generation;
- whole-product executable SBOM plus model and asset BOMs, immutable source/dependency/runtime/model/plugin locks, build-provenance attestation, platform code-signing inputs, vulnerability/license/secret gates, and key-rotation/revocation metadata;
- unused and oversized asset reporting;
- source and derived asset provenance validation;
- F24 codec/variant, loop, clipping, voice-rights, caption/locale coverage, and muted-audio-alternative validation;
- staging directory creation without mutating source projects;
- supported-platform packaging, initially Windows and then Web after a separate readiness gate;
- launch smoke tests against staged output;
- SHA-256 release manifest, acknowledged-warning list, waivers, and release notes inputs;
- signed Creator Hub update lifecycle with stable/beta channels, offline full installer/update bundle, expiring metadata, offline root and rotating online keys, anti-replay/downgrade, staged side-by-side install, pre-update recovery snapshot, first-start health check, automatic app rollback, pause, and repair;
- standard-user Windows install with no system-Python dependency, explicit user-data/cache/model/project locations, relocation/repair semantics, secure credential store, graceful CPU/GPU fallback, and uninstall that preserves user content unless separately selected;
- distribution adapter manifests for future storefront/depot/content-pack layouts without embedding publishing credentials in projects or artifacts;
- reproducible reruns and automation-friendly exit states.

Acceptance criteria:

- preflight detects 100% of seeded release blockers;
- every save from the declared compatibility horizon loads/migrates/re-saves with representative state preserved; migration failure leaves the original byte-identical, unsupported newer saves are not modified, and an incompatible save-affecting plugin change blocks release;
- release generation writes only to the staging/output area;
- two runs from unchanged inputs produce the same file list and per-file hashes, excluding explicitly documented nondeterministic metadata;
- the packaging/staging path is implemented and exercised against the minimal development fixture without producing a clean production candidate; the staged Windows launch qualification is deferred to M13;
- tampered, expired, replayed, revoked, or unauthorized-downgrade update metadata is rejected before install; interrupted install or failed first start restores the prior app version without modifying projects, vault data, model packs, or player saves;
- manifest, SBOM/BOMs, attestation, update metadata, and platform signature bind to the same immutable candidate hash, with zero unmanifested executable payloads;
- no critical vulnerability or unknown executable/license payload can ship without a named, scoped, expiring waiver, and signing keys are absent from source and ordinary build environments;
- clean offline Windows install/repair/update works for a standard user, and uninstall preserves projects, personal assets, model packs, backups, settings, and saves unless each category is explicitly selected;
- Web packaging remains disabled until its own compatibility, path-case, input, audio, persistence, and performance suite passes;
- credits and notices contain every required attribution exactly once;
- `release ready` requires zero blockers, and warnings/waivers are embedded in the final manifest.

**Exit criteria — G8 Core Scope Feature Complete:**

- a representative user can set up a project, select/import assets, enable a recipe, build a menu, author a quest, playtest, address diagnostics, and produce a staged package without manual file surgery;
- F01-F10, F21, F22, F24, and I01-I10 are implemented rather than represented by placeholders;
- each core-scope feature has a representative development-mode proof and mapped deferred verification IDs;
- clean-install, previous-version upgrade, full accessibility/localization matrices, and production staging qualification remain deferred to M13.

### M9 — Rehearse Core-scope Integration

No clean production build or broad regression matrix runs in M9. This milestone confirms that the core feature clusters share contracts and can continue into the frontier work without integration drift.

Tasks:

- run one persistent-development cross-feature path through project selection, asset import/transform, plugin recipe, menu/quest authoring, playtest, preflight, and development staging;
- verify that features use shared catalog, transaction, configuration, diagnostics, history, and permission contracts rather than local substitutes;
- register clean-install, upgrade, rollback, interruption, platform, memory, visual, accessibility, localization, license, and release cases for deferred M13 execution;
- identify contract defects that directly block F11-F20/F23 or final F21-F24 cross-feature closure, and fix only those blockers before proceeding;
- preserve feature flags and keep production packaging disabled by default.

**Exit criteria — G9 Core Scope Integrated:**

- the core cross-feature journey works in the persistent incremental assembly;
- no core feature needs an isolated or incompatible filesystem/configuration path;
- all core acceptance criteria have fixture IDs, expected evidence, and a verification lane;
- the remaining core-scope work is deferred qualification, documentation, performance tuning, or non-blocking defects;
- no clean production candidate or broad regression suite has run.

### M10 — Build the Command Platform, Direct Asset Placement, and SpriteForge Bridge

**Scope:** F11, F23, and the no-regret foundations required by AI, automation, procedural generation, collaboration, and safe future editing.

#### No-regret foundations

These foundations begin in M0/M3 and are completed here. They are mandatory before the embedded assistant or frontier generators may mutate projects:

1. **Stable object IDs:** maps, events, pages, database records, assets, menus, quests, plugins, generated outputs, and placed instances have durable IDs independent of filenames and display names.
2. **Complete reference graph:** every known reference can be queried in both directions for safe rename/delete, impact analysis, RAG context, and release pruning.
3. **Canonical command/tool API:** the UI, CLI, tests, automations, extensions, and LLM call the same typed, schema-validated domain operations.
4. **Semantic operation log:** mutations record intent, inputs, affected IDs, before/after state, dependencies, hashes, provenance, inverse operation, and author/provider.
5. **Capability broker:** tools declare read/write/destructive/external effects, allowed project roots, required confirmation, cost, timeout, cancellation, and rollback support.
6. **Concurrent-editor protection:** every write verifies the source version/hash inspected when the plan was created; external RPG Maker changes trigger conflict handling rather than overwrite.
7. **Deterministic seeds, clocks, and replay:** procedural work, simulation, agent tests, and bug reproduction can replay the same result.
8. **Extension SDK:** importers, exporters, tools, generators, and adapters use versioned contracts, origin/schema/hash pins, declared capabilities, compatibility metadata, out-of-process isolation, minimal environments, resource limits, and capability tokens. A capability or hash change disables the extension pending re-consent.
9. **Secret and network isolation:** provider keys and external-service credentials live in the OS credential vault; projects/configs store opaque IDs only. Child environments are allowlisted, scopes can be rotated/revoked/deleted, and tools do not inherit unrestricted filesystem/network access.
10. **Open exit path:** schemas, operation logs, generated outputs, and project knowledge can be exported in documented human-readable and machine-readable formats.
11. **Hardened local-service supervisor:** AI/SpriteForge/extension services use random loopback ports/nonces, handle/pipe-delivered tokens, strict Host/Origin/CSRF/DNS-rebinding defenses, PID/version handshake, request/body/concurrency limits, filesystem/network sandboxing, minimal environment, and orphan shutdown.
12. **Optional external-tool gateway:** CLI/MCP/connector adapters reuse the same domain tools and are disabled by default. User-initiated installation pins origin/schema/version, OAuth uses PKCE and the OS vault where applicable, private-network/SSRF access is denied by default, capability drift forces re-consent, results are bounded/untrusted, and the model cannot install, enable, or re-permission a connector.

#### Shared command/tool domains

The initial domain API includes:

- `project`: inspect, health, current selection, snapshot, archive, diff, restore-to-new-location, storage relocation, and external-change detection;
- `catalog`: exact/filtered/semantic search, metadata, provenance, license, preview, and usage;
- `asset`: dry-run import, transform, place, move, replace, remove, batch-place, and undo;
- `audio` and `voice`: inspect, import, transform, loop, validate, assign, caption, report coverage, replace, and undo;
- `map`: inspect layers/regions/events, validate placement, create/update event, and apply a reviewed patch;
- `database`: inspect/create/update supported actor, enemy, item, weapon, armor, skill, state, and tileset entries;
- `plugin`: inspect graph, plan, configure, enable/disable, install recipe, validate, lock, and rollback;
- `menu`, `quest`, and `dialogue`: inspect, validate, create/update through versioned schemas, preview, and restore;
- `playtest`: start a disposable session, execute an approved scenario, inspect state, and collect diagnostics;
- `release`: preflight, save-compatibility/migration planning, executable/BOM inspection, update planning, and staging; release/update writes remain separately permissioned;
- `recovery`, `storage`, `privacy`, and `connector`: inspect safe-mode state, snapshot integrity, disk/budgets, pack/cache plans, network/data policy, trust grants, and enabled external capabilities; all changes require explicit typed plans;
- `history`: inspect operations, branch, compare, revert, replay, and merge.

Every command has strict input/output schemas, stable error codes, idempotency behavior, cancellation, a dry-run form for mutations, and an explicit permission classification.

#### F11 — Direct Placement and Smart Prefabs (Plop)

Plopping means more than copying a file. It is a context-aware placement workflow:

1. the creator drags an asset from the Asset Library, Explorer, generated results, or a collection, or uses an equivalent keyboard `Place in...` action;
2. the target exposes valid drop zones and shows a ghost preview, footprint, snapping, layer, inferred role, and rejected-target explanation;
3. the system plans required import, conversion, naming, dependencies, database/event creation, references, provenance, and license changes;
4. collision, dimension, compatibility, shared-dependency, and license issues are shown before mutation;
5. one transaction imports/transforms the asset, creates or updates the target object, records usage/credits, and refreshes the preview;
6. one undo removes the placed instance and only those introduced dependencies that are no longer used elsewhere.

Supported placement targets are delivered in this order:

1. Hub-owned Menu Studio, Quest/Dialogue Builder, database, and audio assignment surfaces;
2. a Hub-owned map canvas with tile, character, event, parallax, region, animation, and prefab placement;
3. conflict-aware writes to RPG Maker map/database JSON when the editor is closed or the inspected source hash still matches;
4. batch brush, scatter, fill, replace, variant, rule-based placement, and reusable smart prefabs.

Smart prefabs can bundle related content, such as a chest sprite, event template, sound, animation, loot rule, switch behavior, and attribution requirements.

Acceptance criteria:

- a valid simple placement needs at most one confirmation;
- cancel leaves the project byte-identical;
- invalid targets explain the rejection and offer valid alternatives;
- no placement can silently overwrite a file, database record, event, or concurrent external edit;
- undo removes the instance and only unshared introduced dependencies;
- keyboard users can complete every placement workflow without drag-and-drop;
- provenance, project usage, license, credits, reference graph, and operation history update atomically;
- repeating an idempotent placement plan cannot create accidental duplicates;
- map placement detects stale `MapXXX.json`/database sources and requires a rebase, retry, or explicit three-way conflict resolution.

#### F23 — SpriteForge Asset Studio Integration

SpriteForge is worth integrating for its image/sprite pipeline, Pixel Studio concepts, QA, revisions, deterministic export patterns, and no-GPU demo. It is not merged wholesale. URPG remains the authority for projects, assets, jobs, commands, permissions, history, and release state.

Integration sequence:

1. freeze a reviewed SpriteForge source revision and inventory only first-party MIT-licensed files; segregate every vendor, model, LPC, CuteSCKR, generated, and unknown-license payload;
2. extract or adapt pure image/metadata algorithms into a versioned `asset_authoring` package with injected paths/configuration and no Flask globals or implicit application-root writes;
3. supervise any retained heavy-generation process on loopback with session authentication, health/version/capability negotiation, cancellation, resource budgets, and no broad filesystem access;
4. wrap every capability in URPG's typed commands, durable jobs, transactions, stable asset/revision IDs, provenance, and undo; do not retain SpriteForge's parallel `projects/`, `output/pixel_assets`, `library.json`, flat job-history, or independent asset authority;
5. rebuild the useful generation, Pixel Studio, preview, frame editor, review, QA, and export flows with Creator Hub components and F22 actions; do not iframe or transplant the global-script frontend;
6. add real RPG Maker MZ adapters for character sheets, faces, enemies, icons, pictures, parallax, animations, and supported tileset roles, including validation/conversion, correct project destinations, database/map registration, attribution, and transactional plopping;
7. expose only the same reviewed commands to F12 after deterministic UI workflows and permissions are complete.

Initial command surface:

- `spriteforge.capabilities`, `spriteforge.health`, `spriteforge.modelStatus`, and `spriteforge.estimate`;
- `spriteforge.generate`, `spriteforge.normalize`, `spriteforge.cleanup`, `spriteforge.directions`, `spriteforge.tileset`, `spriteforge.animate`, `spriteforge.edit`, and `spriteforge.qa`;
- `spriteforge.jobStatus`, `spriteforge.cancel`, `spriteforge.preview`, `spriteforge.exportRmmz`, and `spriteforge.importResult`.

Generation routes declare `production`, `experimental`, `mock`, or `unavailable` capability states. SpriteForge's current procedural/local-mock or provider-capable-but-unwired paths cannot be labeled as production model generation. Its unrestricted Python plugin loader and command-palette metadata are not exposed to users, imported packs, or the assistant.

Packaging boundary:

- include only reviewed first-party source/runtime files, exact dependency locks, license/NOTICE/SBOM records, and adapters needed for enabled capabilities;
- exclude `.venv`, caches, `vendor`, model weights, inputs, outputs, projects, logs, releases, scratch/state, nested repositories, local LPC/CuteSCKR content, and development artifacts;
- distribute optional models as separately versioned, checksummed packs with their own terms and disk requirements;
- never auto-install Python packages, download models, or start network-exposed services during a normal Creator Hub launch without an explicit setup action.

Acceptance criteria:

- one representative user can generate or edit an asset, review its QA/provenance, convert it to an MZ-compatible role, and import/plop it through one previewed transaction;
- normal F23 operations cannot write outside approved cache, model, and selected-project roots;
- cancellation, failure, restart, or unavailable provider cannot leave a partial URPG asset/revision, stale job lock, or falsely successful result;
- every result records source revision, provider/model/workflow, seed and parameters where applicable, input/output hashes, generated-content disclosure, license state, and operation/undo IDs;
- no mock/scaffold route passes as real generation, and capability states match the actual installed/wired backend;
- F21 can ingest SpriteForge output without a second catalog or split-brain asset history, and F11 can plop it without manual export-folder browsing;
- F22 provides controller parity for the integrated user flows;
- the assistant can call only approved F23 commands and cannot reach the Python plugin loader, raw routes, process launcher, shell, or arbitrary paths;
- a release-size/content report proves excluded development/vendor/model/user-data payloads are absent, and every included dependency/payload has reviewed redistribution evidence.

**Exit criteria — G10 Tool, Plop, and Asset Studio Complete:**

- the no-regret foundations and command domains are implemented, not stubs;
- UI operations use the same domain tools that will be exposed to the assistant;
- representative asset, map, database, menu, dialogue, audio, and smart-prefab placements work in the incremental assembly;
- representative SpriteForge author/edit/QA/export/import flows work through URPG commands with no parallel asset or job authority;
- local services use the hardened supervisor contract, and hostile-origin/stale-token/port-squat/version-mismatch/oversized-request fixtures are registered for deferred adversarial qualification;
- extensions and optional connectors cannot exceed approved project/network/credential/resource scopes, and capability/schema/hash drift disables them pending re-consent;
- every mutation produces a plan, capability decision, transaction record, reference update, and undo path;
- all G10 acceptance criteria map to fixtures and deferred qualification IDs.

### M11 — Add the Embedded AI Helper and Hybrid RAG

**Scope:** F12 and F13. The current checkout contains no first-party LLM implementation, so this milestone is a new architecture and implementation workstream rather than a wiring cleanup.

#### Locked local model and runtime direction

The capability-first embedded default is **Qwen3.5-4B**, distributed as an optional signed/checksummed 4-bit local model pack under its Apache-2.0 license. It is selected because the 4B model combines strong small-model reasoning, explicit agent/tool-call training, vision input for sprites/screenshots/maps, multilingual support, and a permissive redistribution posture. Its advertised maximum context is not the product default: local profiles start at 16K and may use 32K when measured hardware budgets allow it.

- use Q5 for the recommended-memory profile and Q4 for the lower-memory supported profile; lazy-load the vision projector only for a visual request;
- run a pinned `llama.cpp` sidecar on loopback with a per-session secret, exact runtime/model/template hashes, hardware discovery, CPU/GPU offload selection, streaming, cancellation, idle unload, and memory/context ceilings;
- provide resumable model-pack setup and an offline installer path; never download, convert, or update a model during an ordinary application build or without explicit user action;
- retain Qwen3.5-2B as an explicitly reduced-capability fallback for constrained hardware. Limit it to narrower routed tool sets and shorter workflows; do not claim parity with the 4B assistant;
- retain deterministic no-model behavior for all core editing, import, plop, validation, controller, and release workflows;
- keep the model/runtime adapter replaceable and record model, quantization, runtime, chat template, prompt, and tool-schema versions in every evaluation and audited action.

The local execution protocol uses an application-owned schema-constrained envelope with `tool_call`, `clarification`, `plan`, `final`, and `refusal` variants. A tool-call variant contains only registered tool names and JSON arguments. The host validates it, applies permission and source-version checks, executes through the transaction layer, returns a bounded structured result, and continues the multi-turn loop. Native runtime/provider tool parsing is an optimization, not a trust boundary, and is enabled only for a pinned combination that passes the same corpus.

Primary references for the locked direction are the [official Qwen3.5-4B model card](https://huggingface.co/Qwen/Qwen3.5-4B), the [`llama.cpp` server documentation](https://github.com/ggml-org/llama.cpp/blob/master/tools/server/README.md), and the [official Granite Embedding 97M Multilingual R2 model card](https://huggingface.co/ibm-granite/granite-embedding-97m-multilingual-r2). Exact artifacts are recorded by hash rather than following mutable `latest` references.

Before the default pack is frozen, run a bounded L1 bake-off of Qwen3.5-4B Q5/Q4 against the deployment-stability alternative on real project traces: asset search/import/plop, stale-write rejection, rollback, plugin configuration, cited retrieval, ambiguity, denied actions, visual asset inspection, malformed arguments, and loop termination. This is a model qualification exercise in the incremental assembly, not a production build or broad regression. Qwen3.5-4B remains the default unless it fails a release-blocking reliability or hardware target that the alternative passes.

#### F12 — Embedded Agentic Creator Assistant

Architecture:

- use a provider-neutral model adapter with capability discovery rather than hard-coding one provider or model;
- support streaming, cancellation, timeouts, bounded retries, rate-limit handling, token/cost budgets, model/version locks, and provider failover where configured;
- implement the complete multi-turn tool loop: submit available tools, receive calls, validate arguments, request permission, execute application-side code, return structured results, and continue until completion or a configured step limit;
- route each turn to the smallest relevant tool subset, normally no more than 10–15 tools, rather than placing the entire registry in every prompt;
- define tools with strict JSON schemas and domain namespaces; never parse free-form text into a write operation when a typed command exists;
- let the model propose plans and tool calls, while the application remains the authority for permissions, execution, transactions, state versions, and rollback;
- expose read tools broadly within the selected project and mutation tools through capability policies and preview/approval rules;
- forbid unrestricted shell, arbitrary filesystem, arbitrary network, secret access, and raw SQL/database tools by default;
- enforce a single per-project AI/data policy for included/excluded content, classification, local RAG, allowed provider destinations, transcript/audit/index/cache/temp retention, export, deletion, and project forgetting;
- enforce no-network mode below the model/tool layer so prompts, embeddings, telemetry, update checks, connectors, and deferred queues cannot create outbound traffic;
- resolve credentials through scoped OS-vault references immediately before an approved provider/tool call; never place raw secrets in tool schemas, model context, project state, child environments, or persisted transcripts;
- treat connector schemas/descriptions/results as untrusted data, require user-installed origin/schema/version pins and scopes, and prevent the model from installing, enabling, authenticating, or changing connector policy;
- check project/object versions immediately before execution so stale calls cannot overwrite newer human or external changes;
- preserve a user-visible transcript of tool name, arguments summary, permission, result, cost, provider/model, affected objects, and undo action;
- support offline/no-AI operation; the core editor, plopping, validation, and release workflows cannot depend on model availability.

Minimum assistant capabilities:

- find, compare, explain, preview, import, transform, and plop assets;
- inspect the active project, scene, selection, references, diagnostics, and history;
- configure plugins and recipes through the Plugin Manager;
- create or revise menus, quests, dialogue, events, database entries, localization, and accessibility profiles through typed schemas;
- start disposable playtests, inspect failures, and summarize diagnostics;
- plan release fixes and staging, while keeping final packaging and destructive actions separately permissioned;
- explain every result with links to the project objects, source documents, and tool evidence used.

Permissions:

| Class | Examples | Default behavior |
|---|---|---|
| Read | inspect project, search catalog, preview asset, read diagnostics | Allowed inside the selected project and approved knowledge sources. |
| Reversible write | create draft, place asset, edit menu/quest, configure plugin | Show summarized plan; allow policy-based approval; always journal and support undo. |
| Destructive/high-impact | delete shared content, migrate saves, bulk replace, release write | Require explicit confirmation with affected-object and rollback summary. |
| External/cost-bearing | provider call, download, publish, network integration | Require configured provider/budget policy and disclose destination/cost class. |
| Forbidden by default | unrestricted shell/filesystem/network, credential reads, policy changes | Not exposed as model-callable tools. |

Acceptance criteria:

- the assistant can complete a representative read-plan-write-validate-undo workflow using only registered tools;
- malformed arguments, unknown tools, duplicate calls, provider failure, timeout, cancellation, stale state, and denied permission fail safely;
- no model response can bypass tool validation or directly edit project files;
- every write is attributable, previewable, reversible, and visible in history;
- tool output uses stable structured errors rather than prompt-only error interpretation;
- provider/model can be replaced without rewriting domain tools;
- prompts, retrieved content, plugin help, and asset metadata cannot elevate privileges;
- secrets never enter prompts, logs, project files, diagnostics bundles, or exported games;
- a seeded secret canary appears in none of transcripts, history, indexes, caches, backups, crash/support artifacts, or child environments, and a revoked credential cannot be reused;
- excluded/deleted content is neither retrievable nor sent to a provider; `forget project` propagates to configured transcripts, embeddings, captions, thumbnails, caches, and temporary artifacts while preserving separately required audit/legal records according to visible policy;
- network capture proves zero egress in no-network mode, including queued telemetry, update, connector, and provider work;
- a malicious connector/tool description cannot expand scope, alter permission policy, reach private-network targets, or access ambient credentials;
- an evaluation set covers tool selection, parameter accuracy, permission behavior, rollback, prompt injection, cost, latency, and task completion.

#### F13 — Hybrid Project RAG and Knowledge Fabric

RAG is recommended, but it is a knowledge layer rather than the mutation or permission system.

The default local semantic embedder is **IBM Granite Embedding 97M Multilingual R2** under Apache-2.0, using its shipped ONNX path. It is separate from the Qwen chat model and is chosen for its small 97M footprint, 384-dimensional vectors, multilingual and programming-code retrieval, and long-document support. Exact IDs, filenames, plugin keys, command names, and error codes remain in lexical/structured indexes alongside vectors.

Retrieval order:

1. typed tools and structured queries for current project state, IDs, dependencies, licenses, and exact facts;
2. exact and lexical search for filenames, plugin keys, commands, parameters, database IDs, and error codes;
3. semantic text retrieval for documentation, recipes, troubleshooting, lore, design notes, quest/dialogue summaries, and natural-language asset descriptions;
4. optional multimodal/image embeddings for visual similarity, palette, composition, mood, and matching asset discovery;
5. reranking and project/selection-aware filtering before context is sent to the model.

Knowledge sources:

- first-party schemas, decisions, tutorials, help, recipes, diagnostics, and migration guides;
- plugin headers/help, commands, parameters, dependency manifests, compatibility, and known issues;
- asset metadata, tags, dimensions, palettes, provenance, licenses, project usage, and derived-asset history;
- map, event, menu, quest, dialogue, localization, and world-graph summaries generated from canonical structured state;
- user-approved project lore, design documents, conventions, and prior accepted decisions.

Index requirements:

- index per project/workspace, with explicit global vendor-document collections;
- stable chunk/document IDs, source path/object ID, hash, schema/version, provider, license, and timestamp;
- incremental re-index after committed transactions rather than full rebuilds;
- metadata/attribute filters for project, content type, plugin/version, license, language, map/quest, and trust class;
- citations back to the exact source object/file and version;
- deletion/retention propagation and a complete `rebuild index` path;
- prompt-injection scanning and trust labels; retrieved text never changes tool permissions or system policy;
- configurable local index/embedding mode and hosted mode, with clear data-transfer disclosure;
- do not index secrets, API keys, private saves, or user content outside the configured retention policy.

For the first multimodal release, Qwen3.5 vision produces reviewable import-time captions/tags/palette descriptions that enter the text index. A dedicated image-embedding index is added only after F15's visual-similarity evaluation proves a material quality gain worth its model and storage cost.

RAG must not be used as the authority for live switches/variables, current files, permissions, transaction status, or release readiness. The assistant must call tools for those facts.

Acceptance criteria:

- exact identifiers resolve through structured/lexical retrieval without semantic ambiguity;
- semantic queries return useful cited sources even when wording differs;
- every answer based on project knowledge cites object/path plus version/hash where available;
- after a committed edit, affected index entries update incrementally and stale entries are removed;
- project and license filters prevent cross-project or disallowed-content leakage;
- a seeded prompt-injection corpus cannot change permissions, call forbidden tools, or suppress required confirmation;
- RAG-disabled mode still supports all deterministic editor operations;
- retrieval quality, latency, context size, citation accuracy, and cost are measured by a versioned evaluation set.

Official OpenAI reference points, if OpenAI is selected as one provider:

- function tools are JSON-schema-defined application actions and the application executes the tool-call loop: <https://developers.openai.com/api/docs/guides/function-calling>;
- file search combines semantic and keyword search over vector stores: <https://developers.openai.com/api/docs/guides/tools-file-search>;
- retrieval supports semantic search plus attribute filtering: <https://developers.openai.com/api/docs/guides/retrieval>.

These references inform one adapter; they do not replace the provider-neutral domain API.

**Exit criteria — G11 Agent Foundation Complete:**

- F12 and F13 are implemented behind explicit AI and retrieval flags;
- deterministic UI tools work with AI disabled;
- the assistant can retrieve cited context, call permitted tools, handle multiple tool rounds, and undo writes;
- permissions, state-version checks, privacy, cost, provenance, prompt-injection defenses, and audit history are implemented;
- the representative assistant journeys run in the incremental assembly without a clean production build;
- every acceptance criterion maps to an evaluation or deferred qualification ID.

### M12 — Implement Frontier Authoring Features

**Scope:** F14-F20 plus final cross-feature closure for F21-F24. All frontier/provider-dependent features remain pluggable and behind flags until M13 qualification.

#### F14 — Intent-to-Project Compiler

- accept text, structured briefs, sketches, screenshots, and reference layouts;
- compile intent into editable maps, events, menus, quests, dialogue, database changes, and F11 placements using typed commands;
- produce a reviewable operation graph, preview, diff, cost estimate, provenance, and rollback plan before mutation;
- never emit an opaque generated project blob or bypass schema validation;
- allow creators to lock authored areas/components and regenerate only selected operations.

#### F15 — Multimodal Asset and Style Intelligence

- search by text, sketch, screenshot, crop, palette, mood, composition, or visual similarity;
- recommend companion portraits, tiles, icons, effects, UI, and audio from verified-license content;
- detect style/palette/resolution inconsistencies and missing companion assets;
- explain recommendations using catalog attributes and cited source assets;
- never claim visual/legal compatibility solely from embedding similarity.

#### F16 — Autonomous Playtest Agents

- drive actual supported input paths rather than mutating game state to fake coverage;
- support goals such as reach location, finish quest, exercise menu/input profile, find softlock, fuzz choices, or compare balance personas;
- emit deterministic seed, input trace, start-state snapshot, screenshots/video where enabled, diagnostics, and minimal reproduction;
- support multiple isolated agents in parallel subject to resource budgets;
- keep screen-observing/general-game agents experimental and provider-pluggable.

#### F17 — Constraint-based Procedural Authoring

- provide a visual graph for map dressing, encounter tables, loot, NPC schedules, and quest variants;
- use deterministic seeds, explicit constraints, locked regions, collision/reference validation, and selective regeneration;
- output ordinary editable project objects through the same transaction/tool API;
- show diffs before Apply and allow per-operation rejection;
- record generator/version/seed/inputs for exact reproduction.

#### F18 — Balance and Progression Simulator

- model combat, economy, drops, crafting, leveling, travel, resources, and quest progression from canonical game data;
- support configurable player personas, strategies, randomness seeds, and batch simulation;
- surface difficulty cliffs, grind spikes, dominant strategies, resource deadlocks, unreachable progression, and sensitivity to parameter changes;
- link findings to exact database/plugin/quest parameters;
- keep simulation assumptions visible and distinguish modeled results from actual playtest evidence.

#### F19 — Project World Graph and Impact Explorer

- graph maps, exits, events, pages, switches, variables, database entries, assets, menus, quests, dialogue, plugins, saves, generated output, and usage;
- support inbound/outbound references, orphan detection, reachability, change impact, safe rename/delete, and `why is this included?` explanations;
- feed structured context to the assistant and Release Assistant;
- update incrementally from committed transactions and verified external changes;
- make graph queries available through UI and typed tools.

#### F20 — Branchable Collaborative Editing

- create named snapshots/branches from semantic operation history;
- compare structured object changes rather than only line-based JSON;
- detect human/AI/external-editor conflicts and offer three-way object-level resolution;
- selectively merge/replay operations with dependency validation;
- show author, model/provider, tool, intent, inputs, outputs, and generated-content provenance;
- provide export/import so collaboration does not require a proprietary hosted service.

**Exit criteria — G12 All Feature Scope Complete:**

- F01-F24, I01-I10, and every subsequently approved roadmap feature are implemented; no promised feature is only a stub, disabled placeholder, hard-coded demo, or future-work note;
- every product surface is reachable through the Creator Hub or deliberately unavailable with an explained capability reason;
- each feature's representative journey works in the incremental development assembly;
- all schemas, migrations, tool contracts, transaction behavior, diagnostics, RAG indices, AI policies, and release-manifest inputs are implemented;
- every acceptance criterion maps to a verification ID, fixture, expected evidence, and execution lane;
- the open work list contains defects, qualification, documentation, or tuning only—no missing promised feature scope;
- no known boot-hang, data-loss, unsafe-write, secret-exposure, or trust-boundary defect remains;
- production build inputs are frozen before M13 begins.

### M13 — Build Once, Integrate, Qualify, and Release

**Scope:** one clean immutable candidate after G12, followed by all deferred build, regression, platform, catalog, plugin, AI, visual, accessibility, localization, performance, soak, security, and reproducibility work.

#### G13A — Integrated Candidate

- produce one clean, platform-signed production artifact from frozen inputs with source revision, dependency/model/plugin locks, executable/SBOM/model-BOM/asset-BOM manifests, provenance attestation, update metadata, and SHA-256 hashes;
- perform signing in the controlled release boundary; signing keys never enter source, ordinary development, or ordinary build/test environments;
- store it immutably and make all qualification jobs reference that exact artifact;
- run the cross-feature path: controller-driven project selection -> add/search/transform a personal asset -> SpriteForge edit/generation and MZ conversion -> plop -> plugin recipe -> menu/quest authoring -> Qwen-assisted edit with cited RAG -> playtest agent -> preflight -> staged output;
- tests may not rebuild or mutate the candidate;
- if candidate inputs change, create a new hash and invalidate the evidence identified by the change-impact graph plus the critical smoke.

#### G13B — Qualified Release Candidate

- execute all deferred acceptance criteria and matrices in parallel against G13A;
- run full vendor/personal/generated catalog reconciliation/preview, all supported plugin/recipe smokes, complete fault injection, model/tool and permissions/prompt-injection evaluations, retrieval evaluation, SpriteForge adapter/provider checks, physical-controller matrices, visual/accessibility/localization matrices, audio/voice/caption matrices, platform install/update/rollback, project backup/archive/restore, safe-mode/support/privacy, offline/no-network, supply-chain/tamper, historical save/config migration, and long memory/performance soaks;
- batch discovered fixes by subsystem before producing a replacement candidate;
- maintain zero open boot, corruption, security, permission, migration, licensing, privacy, or release blockers;
- rerun only invalidated evidence plus the critical release smoke for a replacement candidate.

#### G13C — Frontier General Availability

- clean install and the critical release journey pass against the qualified candidate;
- signed stable-channel update from the previous supported release, interrupted-update rollback, repaired install, offline install/update, uninstall-with-data-preservation, and application downgrade/rollback policy pass against the qualified candidate;
- manifests, credits, notices, AI/provider disclosures, privacy/retention settings, compatibility, recovery, migration, and support documentation are complete;
- the released artifact hash exactly equals the qualified artifact hash;
- any artifact difference requires repeating G13A-G13C for the replacement.
- the supported-version/deprecation matrix, update pause/rollback procedure, key-revocation procedure, and emergency security/data-loss maintenance runbook are published and rehearsed.

**Exit criteria — G13 Frontier General Availability:**

- G13A, G13B, and G13C pass against the same immutable artifact hash;
- all deferred F01-F24/I01-I10 acceptance evidence is recorded;
- the released artifact is the exact qualified artifact and remains reproducible from checked-in inputs and instructions.

## 8. Shared Technical Foundations

These foundations must be implemented once and reused.

### 8.1 Catalog Schema

The catalog covers assets, packs, plugins, support files, recipes, transforms, installed state, project usage, versions, hashes, compatibility, and license status. The CLI, Creator Hub, Plugin Manager, Asset Library, and Release Assistant consume the same versioned schema.

### 8.2 Transaction and Recovery Layer

Every mutation follows:

1. inspect and validate source/destination;
2. generate a dry-run plan;
3. obtain explicit confirmation when required;
4. back up affected destination state;
5. write into a temporary/staging location;
6. validate generated output;
7. atomically replace or safely copy into place;
8. record a journal and output hashes;
9. roll back automatically on failure;
10. expose undo and recovery status to the user.

### 8.3 Versioned Configuration and Migration

Menus, recipes, quests, accessibility/input/controller profiles, asset-source/import plans, localization catalogs, AI/model profiles, plugin locks, and release manifests use explicit schema versions. Each supported prior version has forward migration fixtures. Destructive downgrade is never implied; rollback restores a saved previous artifact.

### 8.4 Validation and Diagnostics

All validators return structured issues containing:

- code and severity;
- project/component identifier;
- file or logical field path;
- human-readable explanation;
- remediation guidance;
- whether automatic repair is available;
- related source/provenance information.

The UI and command-line outputs are different presentations of the same results.

### 8.5 Plugin Manifests and Lockfile

The plugin model records plugin key, filename, source/hash, version, target RPG Maker version, dependencies, required ordering, conflicts, support files, parameters, commands, save-data impact, and redistribution status. A lockfile records the exact selected plan.

### 8.6 Used-content Ledger

Imports, transforms, menu selections, recipes, generated events, and release scans update a ledger of assets/plugins actually used by the project. Release credits and license checks derive from this ledger and then verify it against a complete reference scan.

### 8.7 Personal Asset Vault and Durable Jobs

Personal, generated, and vendor-derived assets share stable IDs, content hashes, revisions, provenance, license state, references, and one catalog authority. Background ingest/generation jobs use durable state, idempotency keys, cancellation, restart recovery, bounded resources, and an atomic final commit. Temporary decode/generation output is never treated as a project asset until validation and commit succeed.

### 8.8 Semantic Input Platform

Creator and runtime features bind semantic actions through the shared registry/context/router/profile services. Raw Gamepad API indices, keyboard codes, Steam Input handles, and touch gestures remain adapter details. Focus graphs, required actions, glyph families, calibration, replay traces, privacy controls, and profile migrations are versioned product contracts.

### 8.9 Local Model Packs and AI Runtime

Chat, embedding, and optional visual model packs are independent, signed/checksummed artifacts with model-card source, license/NOTICE, quantization, runtime/template compatibility, memory/disk estimate, and version lock. Application builds consume a manifest and never train, convert, download, or silently update weights. The local inference process binds only to loopback, authenticates each app session, and exposes no raw project filesystem.

### 8.10 Asset Authoring Adapter Boundary

SpriteForge and future authoring engines implement a versioned capability/health/job/result adapter. Results cross the boundary as staged files plus a manifest; URPG validates them and creates canonical asset revisions through ordinary F21/F11 transactions. External routes, stores, plugin loaders, processes, and provider credentials never become direct model tools or alternate project authorities.

### 8.11 Save Compatibility and Update Lifecycle

Player saves have their own format/version stamps, plugin-lock fingerprint, compatibility horizon, and ordered idempotent forward-migration registry. Migrations always back up the original, never modify unsupported newer saves, and never imply destructive reverse migration. Creator Hub/app updates use signed, expiring, anti-replay metadata, staged side-by-side installation, health-check promotion, automatic rollback, offline bundles, key rotation/revocation, and a strict boundary that cannot mutate projects, vault data, model packs, backups, or player saves.

### 8.12 Storage, Offline Operation, and Disaster Recovery

The product classifies every byte as authored/referenced source, managed derived revision, pinned optional pack, regenerable cache, temporary job output, backup, or release artifact. Users can inspect usage/budgets, preflight free space, relocate roots transactionally, and dry-run safe cleanup. Whole-project snapshots and portable archives are checksummed and restorable on a clean machine. No-network mode is a transport-enforced product contract, not only an AI preference.

### 8.13 Trust, Privacy, and Supply Chain

Executable inspection is non-executing; trust grants bind origin, immutable hash, schema/version, capabilities, roots, network scope, and resource limits. Secrets use OS-vault references and scoped injection. Per-project data policy governs RAG/provider inclusion, retention, export, deletion, and network use. Every shipped executable, dependency, plugin, model, updater, and asset payload is represented by the appropriate SBOM/model BOM/asset BOM and immutable provenance, and substituted/revoked/disallowed artifacts are rejected before parsing or loading.

### 8.14 Supportability and Platform Contract

Bootstrap safe mode, crash-loop detection, stable error IDs, optional-subsystem disable/bisect, regenerable-cache rebuild, and previewable redacted support bundles work offline. Windows GA installation runs as a standard user without system Python and preserves user data through repair/update/uninstall. Web and every additional OS/storage target remain explicitly gated; unsupported cloud-sync, network-share, removable, case, symlink/junction, long-path, or permission conditions fail with actionable diagnostics before partial writes.

## 9. Feature-first Build and Verification Strategy

Acceptance tests are specified and authored with their features, but expensive execution is deferred until all approved feature scope is implemented. Continuous integration protects shared contracts through small checks instead of repeated clean/release builds.

Feature-first does **not** mean zero verification until the end. A total verification freeze would allow basic syntax, schema, and integration errors to compound into an undiagnosable big-bang failure. Before G12, only the bounded L0-L2 lanes run; the expensive L3-L5 lanes begin in M13.

### 9.1 Verification Lanes

| Lane | Trigger | Scope | Build policy | Target budget |
|---|---|---|---|---|
| L0 Affected check | Each merged change | Parsing/type/static checks plus changed schema and service contracts | No package build | 60 seconds |
| L1 Feature proof | Feature implementation gate | One representative happy path; include rollback when the feature writes | Existing incremental development assembly | 10 minutes |
| L2 Cluster rehearsal | Asset, plugin, menu, authoring, AI, or placement cluster completion | One cross-feature path in the persistent development environment | No clean or production build | 20 minutes |
| L3 Candidate build | Once after G12 | Clean production build from frozen inputs; emit hashes and manifest | Build once and store immutably | Measured at M0 |
| L4 Qualification | After L3 | Full acceptance, integration, E2E, inventory, plugin, AI, fault, visual, accessibility, localization, platform, performance, and soak suites | Consume L3; tests may not rebuild it | Parallelized |
| L5 Release confirmation | After blockers close | Clean install, critical-path smoke, manifest, credits, reproducibility, and disclosure confirmation | Promote the same artifact if its hash is unchanged | Bounded release gate |

A check exceeding its lane budget is optimized, split, or moved to L4 unless it protects against an immediate boot hang, data loss, unsafe write, secret exposure, or security boundary failure.

### 9.2 Build Rules

- Development uses one persistent incremental/watch assembly.
- Clean builds and production packaging are prohibited by default before G12.
- No test command may hide an implicit build or packaging step.
- Catalogs, thumbnails, transforms, embeddings, project graphs, controller glyph caches, and plugin plans are content-addressed and regenerate only when inputs change.
- Model downloads/conversion and SpriteForge dependency/model setup are explicit setup operations and never implicit build or test steps.
- Content/model/plugin/extension/update downloads and signing are never implicit build/test steps; ordinary build workers have no release/update signing credentials.
- Feature flags allow incomplete surfaces to merge without pretending they are complete.
- Tests consume an explicit artifact hash and never invoke a build implicitly.
- One L3 candidate is reused across every L4 qualification shard.
- Documentation-only changes do not invalidate the product artifact.
- Candidate defects are batched by subsystem; a replacement candidate is built after the batch rather than after every fix.
- Any code or packaged-content change creates a new artifact hash and invalidates the evidence identified by the impact graph plus the critical release smoke.
- A final artifact can ship only when its hash equals the qualified candidate.

### 9.3 Integration Safeguards Before the Full Build

- all features merge continuously behind flags rather than remaining on long-lived isolated branches;
- shared contracts have executable compatibility fixtures;
- every merge proves that the application shell can load all registered feature modules;
- each feature uses the real transaction, catalog, project graph, history, diagnostics, configuration, permission, and command interfaces;
- L2 rehearsals cover personal/vendor asset -> transform/plop, audio/voice/caption -> dialogue/playtest, SpriteForge result -> MZ import, controller action -> focus/canvas operation, Plugin Manager -> recipe/save-impact, Menu/Quest -> playtest, assistant -> tool/undo, project snapshot/archive -> restore, and project -> preflight/update-plan handoffs;
- every persisted schema has a migration/rollback path before the feature is called implemented;
- every feature gate maps acceptance criteria to verification IDs, fixtures, expected evidence, and execution lanes;
- no placeholder controls, mock-only production paths, dead routes, or required unfinished implementations may pass G12.

### 9.4 Development Safety Floor

These targeted proofs may run before G12 even when they exceed normal feature work because they protect the project itself:

- corrupt or absent preload data cannot hang boot;
- project writes stage, back up, verify source versions, and roll back;
- missing editor sidecars fail with an actionable message;
- untrusted metadata/retrieved content cannot enter executable scripting or elevate permissions;
- secrets cannot enter project files, prompts/logs where prohibited, diagnostics exports, or game packages;
- the persistent development assembly can load every registered feature module.

The 30-cycle memory soak, 100-save sequence, full failure matrices, complete catalog traversal, every plugin/recipe smoke, and release logging audit remain deferred to L4.

### 9.5 Deferred L4 Qualification Suites

The following run only after G12 against the immutable L3 candidate:

#### Unit and Contract Coverage

- typed parsing, schemas, tool inputs/outputs, and structured errors;
- preload queue state and progress;
- dependency resolution and ordering;
- catalog IDs, tags, hashes, provenance, references, and stable object IDs;
- asset-source inspection, archive containment/resource limits, import plans, durable job restart, and managed-vault deduplication;
- audio decode/transforms, loudness/peak, loop metadata/seam analysis, voice/caption IDs/coverage, and deterministic derived manifests;
- semantic actions, context priority, bindings, calibration/dead zones, focus graphs, glyph resolution, profile migration, haptic capability fallback, and trace replay;
- transaction planning, stale-state rejection, rollback, branch/replay, and merge;
- transforms and derived provenance;
- configuration/save/index migration;
- player-save versioning, plugin-lock fingerprints, ordered/idempotent migration, backup preservation, unsupported-newer-save handling, and compatibility horizon;
- snapshot/archive integrity, retention/reachability, storage classification/budgets/relocation, cache safety, and no-network policy enforcement;
- update metadata signatures/expiry/version ordering/revocation, staged promotion/rollback, executable inventory, SBOM/BOM/attestation binding, and trust-grant invalidation;
- license and release severity rules;
- provider adapters, tool loops, permissions, budgets, cancellation, and retry behavior;
- model-pack manifests/hashes/licenses, hardware-profile selection, context/memory ceilings, structured action-envelope parsing, and local-runtime session authentication;
- SpriteForge capability truthfulness, adapter schemas, MZ conversion manifests, store consolidation, and disallowed plugin/process/path boundaries;
- non-executing plugin inspection, out-of-process extension capability enforcement, loopback supervisor validation, OS-vault references, support-bundle redaction, connector origin/schema pinning, and data-deletion propagation;
- structured, lexical, semantic, and multimodal retrieval quality and filtering;
- deterministic procedural and simulation seeds.

#### Integration Coverage

- clean minimal MZ fixture boot;
- complete, partial, and absent Menu Builder sidecar;
- safe menu save and full failure injection;
- personal/vendor/generated asset import/transform/plop/undo, archive isolation, deduplication, relink/replace, and collision handling;
- audio import/edit/loop/convert/assign/caption/playtest/undo and missing/stale voice-locale coverage;
- SpriteForge edit/generate/QA -> MZ export -> canonical import/plop, including cancellation, unavailable providers, and restart recovery;
- controller hot-plug, context/focus transitions, remapping/recovery, glyph switching, and generated-game input-manifest integration;
- external RPG Maker change and three-way conflict handling;
- plugin install/enable/disable/recipe/rollback matrices;
- menu and quest migration/preview;
- assistant read-plan-write-validate-undo and provider failure;
- RAG indexing, citations, deletion, cross-project isolation, and prompt injection;
- playtest-agent trace replay;
- staged release launch.
- whole-project automatic/manual snapshot, portable archive, clean-root restore, corrupt/interrupted backup rejection, retention pruning, and storage-root relocation;
- safe-mode crash-loop recovery, plugin/optional-subsystem disable/bisect, cache/index rebuild, support-bundle preview, and consent withdrawal;
- signed online/offline app update, tamper/replay/downgrade/revocation rejection, interrupted install, failed-first-start rollback, repair, and uninstall data preservation;
- hostile webpage/DNS rebinding, port squat, stale token, version/PID mismatch, oversized request, path traversal, malicious extension/connector, credential revocation, and private-network/SSRF rejection;

#### End-to-End Coverage

- first-run onboarding through release preflight;
- keyboard-only creator workflow and keyboard-equivalent plopping;
- supported-screen-reader creator workflow plus structured alternatives for map/graph/waveform/timeline canvases;
- first-time music/SFX/voiced-and-captioned-dialogue import -> transform/loop -> assign -> playtest -> preflight;
- controller-only Creator Hub, editor canvas, playtest, preflight, and generated-game navigation;
- previous-supported-version upgrade and rollback;
- interrupted import/save/plugin/AI/index/release operations;
- prompt/sketch -> reviewed operation graph -> plop -> playtest -> undo;
- production-package assertion that development and AI-authoring controls are absent or correctly configured.
- clean offline standard-user Windows install and network-blocked create/edit/import/plop/playtest/preflight/package/local-help journey;
- previous-supported-app/project/save/plugin-lock update -> migration -> health check -> rollback journey.

#### Full Data and Plugin Coverage

- complete 15,660-record catalog reconciliation and preview traversal;
- malicious/corrupt/oversized personal archive corpus plus large managed personal-library reconciliation;
- every supported plugin dependency plan and gameplay recipe primary-scene smoke;
- full license/credits/NOTICE/BOM and used-content reconciliation;
- complete executable/plugin/extension/model/updater manifest, SBOM/model-BOM/asset-BOM, provenance, signature, vulnerability/waiver, and revocation reconciliation;
- source hash, derived provenance, support-file, and project-reference checks.

#### Performance and Soak Coverage

- Asset Library search/filter p95 under 250 ms after index load on the current catalog;
- responsive lazy browsing of the largest asset pack;
- 30 repeated map/menu transitions after warm-up;
- extended playtest with images, audio, video, saves, recipes, custom menus, AI-disabled operation, and AI-enabled authoring;
- Creator Hub behavior while indexing, embedding, previewing, simulating, and switching large collections;
- local Qwen3.5-4B text/vision latency, cancellation, idle unload, memory ceilings, long tool loops, and concurrent indexing on each supported hardware profile;
- personal-asset ingestion and SpriteForge job throughput, disk/cache budgets, interrupted cleanup, and large-batch cancellation;
- audio batch conversion/preview/loop analysis throughput plus storage relocation, snapshot, archive, restore, update staging, and low-disk budgets;
- parallel playtest-agent and procedural-generation resource budgets.

#### Visual, Accessibility, and Localization Coverage

- golden screenshots for representative menu, plop, AI plan/review, and creator screens;
- 1024x600, 16:9, 16:10, ultrawide, compact/handheld, and 200% zoom profiles;
- keyboard focus and controller-navigation snapshots, calibration/glyph screens, and canvas grab/move/drop states;
- automated accessibility scan plus keyboard-only and supported-screen-reader walkthroughs;
- accessibility-tree/live-region/name-role-state checks and structured equivalents for custom canvas surfaces;
- contrast, forced-colors, non-color cue, touch-target, timeout, reduced-motion/photosensitivity, mono/muted-audio alternative, glyph, pseudo-localization, RTL/bidi, IME, plural/date/number, and overflow matrices.

#### Controller Hardware and Platform Matrix

- Xbox-style, DualShock 4/DualSense, Switch Pro, and standards-compliant generic devices on Windows;
- hot-plug, disconnect/reconnect, two connected devices, remap recovery, drift/dead-zone, glyph convention, and haptics capability/off behavior;
- optional Steam Input/Steam Deck adapter with duplicate-event suppression and generated action manifest;
- Web controller support only after its separate browser/device compatibility gate.

#### Install, Storage, Recovery, and Update Matrix

- standard-user Windows install/repair/update/uninstall with no system-Python dependency and graceful CPU-only/GPU-unavailable behavior;
- projects, vaults, caches, models, backups, and releases under paths containing spaces, Unicode, long names, case collisions, read-only components, symlinks/junctions, and OneDrive-style external changes;
- explicitly supported local/removable locations plus actionable rejection or read-only behavior for unsupported cloud-sync/network-share configurations;
- interrupted/corrupt snapshot, archive, relocation, pack download, update, migration, low-disk, and failed-first-start cases with recovery to the last known-good state;
- offline installer/update/pack verification uses the same signature/hash/revocation policy as connected installation.

#### AI and Security Evaluation

- tool selection, parameter accuracy, multi-step completion, rollback, latency, cost, and citation accuracy;
- malformed tool calls, duplicates, timeouts, cancellations, stale state, denied permissions, provider outage, and budget exhaustion;
- Qwen3.5-4B Q5/Q4 tool-selection and visual-inspection corpus plus reduced-capability fallback-policy enforcement;
- direct and retrieved prompt injection, malicious plugin help/asset metadata, data exfiltration attempts, and cross-project leakage;
- malicious plugin/extension/connector schemas and results, capability drift, scope escalation, SSRF/private-network attempts, hostile loopback origins, ambient credential access, secret canaries, and deletion/forget propagation;
- forbidden tool/path/network/secret access remains impossible;
- offline/no-AI mode completes every deterministic editing and release workflow, and enforced no-network mode produces zero outbound traffic or deferred upload.

## 10. Success Metrics

Targets are confirmed or adjusted after M0 baseline measurement.

| Metric | Target |
|---|---|
| Clean project setup | No manual vendor-folder copying or `plugins.js` editing. |
| Asset discovery | Every catalog record is searchable or explicitly unavailable; search/filter p95 under 250 ms after index load. |
| Import safety | Zero silent overwrites; failed imports automatically roll back; undo restores fixture state byte-for-byte. |
| Personal asset onboarding | A first-time user can add and plop a valid custom asset with no more than three required decisions; copied assets remain portable and fully traceable. |
| Audio/voice/caption workflow | A first-time user can import, transform/loop, assign, playtest, caption, preflight, and undo representative music, SFX, and voice without manual folder/event surgery. |
| Direct placement | A valid simple plop requires at most one confirmation, creates no dangling references, updates provenance atomically, and is fully undoable. |
| Controller coverage | The complete core creator and generated-game journeys are controller-completable with no mouse-only action; supported hot-plug/remap/glyph/recovery cases pass. |
| Onboarding and daily use | First run can resume/skip, reaches a safe sample or real project, explains capabilities/storage/privacy, and exposes universal search plus visible jobs/downloads. |
| Project recovery and portability | Automatic/manual snapshots and portable archives verify, restore to a different clean root, preserve canonical state, and never lose the last known-good recovery point. |
| Storage safety | Cleanup/relocation never deletes referenced or non-regenerable content; disk preflight prevents partial import/generation/update/release work. |
| Offline operation | The complete defined local core journey succeeds with outbound network blocked and creates no deferred telemetry/update upload. |
| Save/update safety | All saves in the compatibility horizon migrate or load safely; tampered/interrupted/failed app updates reject or roll back without modifying user content. |
| Trust and privacy | Zero plugin execution during inspection, zero unapproved scope/secret access, zero unsolicited egress, exact support payload preview, and deletion/forget propagation pass. |
| Supply-chain integrity | Every executable/plugin/model/updater/payload is manifested and bound to the signed candidate hash; substituted, revoked, disallowed, or unwaived critical-risk artifacts are rejected. |
| SpriteForge integration | Representative author/edit/QA output becomes a canonical MZ-ready asset and can be plopped without manual file movement, duplicate stores, false capability claims, or unlicensed payload inclusion. |
| Plugin setup | A recipe installs in no more than five required decisions and can be reproduced from a lockfile. |
| Menu authoring | A representative menu can be created, previewed, saved, reopened, and recovered without hand editing. |
| AI execution safety | 100% of assistant mutations use registered typed tools, capability checks, version checks, transaction history, and undo; zero direct project-file writes by model output. |
| AI task completion | Representative asset, plugin, menu, quest, playtest, and release-planning tasks complete within configured step/cost limits and cite their tool/retrieval evidence. |
| Embedded AI profile | The pinned Qwen3.5-4B pack meets agreed task-completion, tool-argument, visual-inspection, latency, cancellation, and memory thresholds on the recommended hardware profile. |
| Retrieval quality | Exact identifiers resolve deterministically; semantic results meet the agreed evaluation threshold; every project-knowledge claim is traceable to a source/version. |
| Agent reproducibility | Every automated playtest failure includes a replayable seed, start-state snapshot, input trace, and linked diagnostics. |
| Validation quality | Seeded blockers are detected before release; recoverable configuration errors do not crash unrelated scenes. |
| Runtime memory | No monotonic preloader-driven growth; post-soak memory remains within the approved warm-baseline budget. |
| Accessibility | Core creator flow works with keyboard, controller, and a supported screen reader; structured canvas alternatives, contrast/zoom/input/photosensitivity/hearing/RTL checks pass. |
| Release traceability | Every shipped file has an origin/hash or documented generated provenance and applicable license status. |
| Reproducibility | Unchanged inputs reproduce the same plugin lock, staged file list, and per-file hashes within documented exceptions. |
| Build efficiency | Zero clean production builds before G12; one immutable L3 candidate is reused across L4; replacement candidates are created only for batched input changes. |

## 11. Risks and Mitigations

| Risk | Mitigation |
|---|---|
| The actual first-party app is absent from this checkout | Make G0 mandatory; do not infer workflows from vendor samples. |
| Vendor plugins change behavior during hardening | Preserve originals, patch curated copies with provenance, add fixtures before refactoring, and roll out behind compatibility flags. |
| Removing `eval()` breaks legacy expressions | Inventory semantics, migrate typed fields, isolate documented advanced hooks, and provide migration diagnostics. |
| Existing menu data is lost during schema conversion | Version artifacts, preserve originals, dry-run migrations, compare round trips, and keep automatic rollback. |
| Transaction rollback is mistaken for disaster recovery | Keep operation undo, transaction rollback, whole-project snapshots, portable archives, and tested clean-root restore as separate layers; never prune the last known-good recovery point. |
| Player saves break after an application, project, or plugin update | Stamp saves independently, record plugin-lock impact, publish a compatibility horizon, require ordered idempotent migrations, preserve pre-migration backups, and reject unsupported newer saves without mutation. |
| An application update is tampered with, replayed, interrupted, or fails on first start | Require signed expiring metadata, anti-replay/downgrade ordering, revocation, staged installation, health checks, automatic rollback, repair, and offline packages under the same verification policy. |
| An executable, model, plugin, extension, or updater payload is substituted or omitted from review | Produce a complete executable inventory plus SBOM/model-BOM/asset-BOM, bind it to the signed artifact hash, verify provenance and licenses, scan vulnerabilities/secrets, and reject unmanifested or revoked payloads. |
| Asset redistribution terms are incomplete | Block unknown/restricted release inclusion by default and require evidence or explicit auditable waiver. |
| Personal archives or media decoders expose the workstation | Stage in isolation, normalize paths, allowlist formats, impose file/count/decompression/time/memory limits, quarantine failures, and sandbox importer adapters. |
| External personal-asset links break portability | Copy by default; hash/monitor advanced links, provide relink, and block portable release while unresolved. |
| A user mistakes possession for redistribution rights | Use a guided license declaration, preserve source evidence, default to unknown, and enforce the normal release blocker/waiver policy. |
| A large personal library freezes the product | Use background incremental indexing, virtualized views, content-addressed caches, cancellation, resumable jobs, and explicit storage budgets. |
| Cleanup, relocation, repair, or uninstall removes user-authored or non-regenerable data | Classify storage by ownership and regenerability, show exact plans, preflight capacity, use checksummed copy-then-switch, protect referenced content, and preserve projects/vaults/backups by default. |
| Audio conversion or voice generation harms quality, rights, localization, or accessibility | Use non-destructive revisions, measured loudness/peak and loop QA, stable voice-line IDs, explicit performer/provider rights and consent, provenance, caption coverage, and human approval before assignment or release. |
| Plugin configuration scope overwhelms users | Prioritize recipes, searchable guided forms, progressive disclosure, and safe defaults. |
| Creator Hub becomes a second RPG Maker editor | Keep scope focused on asset/plugin/menu/quest/playtest/release workflows and enforce milestone exit criteria. |
| Cross-platform filesystem behavior differs | Use relative paths, case-sensitivity checks, transactional staging, and separate platform gates. |
| Large asset packs make the UI slow | Precompute metadata/thumbnails, use lazy rendering, virtualized lists, cache budgets, and performance tests. |
| Debug tooling leaks into production | Build-time/release assertions and staged-package tests verify development controls are absent or disabled. |
| Direct map/database placement overwrites concurrent RPG Maker edits | Verify inspected source hashes immediately before write and require rebase, retry, or three-way conflict resolution. |
| A game plugin or Creator Hub extension executes arbitrary code during discovery or gains ambient authority | Make inspection non-executing, distinguish runtime plugins from Hub extensions, run extensions/importers out of process, grant narrow hash-bound capabilities, impose resource limits, and invalidate trust when code changes. |
| A malicious webpage or stale local process reaches an embedded-model or helper loopback service | Use an application-owned supervisor, authenticated per-session IPC, strict origin/host/PID/version checks, request and path limits, port-squat/DNS-rebinding defenses, idle shutdown, and fail-closed startup. |
| Credentials or private content leak through projects, prompts, logs, support bundles, generated games, or connectors | Store secrets in the OS credential vault, pass references rather than values, redact by policy, show the exact support/export payload, use secret canaries, and propagate revoke/delete/forget operations. |
| Telemetry, updates, providers, or queued work violate offline or no-network expectations | Make telemetry opt-in/default-off, declare every network capability, enforce no-network at the transport broker, prevent deferred uploads, and prove the full local workflow with outbound traffic blocked. |
| Optional MCP or external-tool connectors drift capabilities, inject content, or reach private resources | Require explicit installation, origin/schema/hash pins, PKCE or scoped credentials, capability review, result-as-untrusted-data handling, SSRF/private-network controls, revocation, and prohibit the model from installing or expanding connectors. |
| Controller mappings differ by browser, OS, and device | Use semantic actions, adapter capability discovery, calibration, per-device profiles, synthetic replays, and a physical certification matrix. |
| Steam Input and direct Gamepad input double-fire | Select an explicit active adapter, suppress duplicate events, and expose mixed-input diagnostics. |
| Remapping strands the user or confirm/cancel conventions confuse them | Protect essential actions, reserve a recovery chord/profile, separate semantic positions from glyph labels, and offer convention override/reset. |
| New UI silently regresses controller focus | Require shared focus primitives, focus-graph linting, replay fixtures, and controller parity in each feature gate. |
| Model output bypasses application safety | Never execute model prose; expose only strict typed domain tools through the capability broker and transaction layer. |
| Small local model or runtime parser emits malformed/unsafe tool calls | Use schema-constrained application envelopes, narrow routing, double validation, one bounded repair, pinned versions/checksums, step limits, and project-specific evaluation traces. |
| Local model memory/latency is unacceptable on some hardware | Use Q5/Q4 hardware profiles, bounded 16K/32K contexts, lazy vision, idle unload/offload, an explicitly limited 2B fallback, and complete no-AI workflows. |
| Retrieved/plugin/asset content contains prompt injection | Treat all retrieved content as untrusted data, separate policy from context, label trust, filter tools by application policy, and red-team retrieval. |
| RAG returns stale or cross-project facts | Use object IDs/hashes/versions, incremental invalidation, project/tenant filters, citations, and tools for live state. |
| AI costs, latency, privacy, or provider terms become unacceptable | Provider abstraction, budgets, caching, local/offline options, explicit retention controls, and deterministic non-AI workflows. |
| Frontier research capability is unreliable | Keep providers pluggable, outputs structured/reviewable, features flagged, and deterministic authored fallbacks available. |
| SpriteForge integration imports 100+ GB of models/vendor/user data | Manifest only approved first-party runtime files; exclude development and user payloads; distribute optional model packs separately; verify packaged contents and size. |
| SpriteForge/LPC/CuteSCKR licensing leaks into release | Pin a clean revision, inventory nested/ignored content, segregate GPL/mixed/unknown assets, require per-payload evidence, and fail packaging on unmanifested files. |
| SpriteForge creates split-brain assets, jobs, or provenance | Consolidate into URPG's canonical store and durable jobs; treat sidecar output as uncommitted staging until an ordinary transaction succeeds. |
| SpriteForge scaffold/mock provider paths look production-ready | Publish truthful capability states, require a real-backend acceptance fixture, and prevent mock/experimental routes from satisfying G12 production criteria. |
| Deferring broad tests creates late integration risk | Merge continuously, enforce L0-L2 budgets, use real shared contracts, rehearse clusters, and freeze all feature scope before the single candidate build. |

## 12. Critical Paths and Parallel Work

Critical runtime path:

`M0 -> I01/I02 -> I03/I04/I05/I06/I07 -> G1`

Critical creator-platform path:

`M0 -> I09 catalog -> M3 F01 shared/recovery/storage services -> F02/F21/F24 -> F04 -> F05 -> F07/F08`

Critical direct-placement and AI path:

`M0 -> stable IDs/reference graph/command API/transactions -> F21 asset intake -> F11 plop -> F23 SpriteForge bridge -> capability broker -> F12 -> F13 -> F14-F20`

Critical release path:

`M0 -> F01-F10/F21-F22/F24/I01-I10 -> G9 -> F11-F20/F23 -> G12 F01-F24 freeze -> one L3 build -> L4 qualification -> G13C`

Work that can run in parallel after M0:

- preloader fixes and Menu Builder persistence/validation;
- catalog/provenance schema and minimal first-party test fixture;
- Creator Hub architecture, resumable onboarding, snapshots/portable archives, storage/offline manager, safe-mode recovery, and accessibility design standards;
- personal-asset staging/vault/import-job design and malicious archive fixture corpus;
- audio/voice/caption schema, non-destructive transform pipeline, loop/loudness QA, and RPG Maker assignment adapter;
- semantic input/action/profile/focus architecture plus synthetic controller adapter;
- plugin manifest extraction, player-save impact/migration, signed updater, complete executable inventory, and release preflight rule design;
- extension isolation, loopback-service hardening, OS-vault secret references, privacy/no-network enforcement, and optional connector gateway threat fixtures;
- stable-ID/reference-graph and domain-command contract design;
- pinned Qwen3.5 model/runtime pack, provider-neutral assistant, capability, privacy, and cost-policy design;
- structured/lexical/Granite-embedding RAG prototype and multimodal retrieval evaluation design;
- SpriteForge source/license inventory, pure-service extraction spike, capability manifest, and real MZ adapter contract.

F01 recovery, storage, offline, onboarding, and support services protect every mutating feature and release operation. F03 should reuse F02/F21 import transactions. F24 should reuse F21 intake/provenance, F07 voice-line/localization IDs, F09 accessibility semantics, and F10 release validation. F06 should reuse F04 dependency plans. F05 depends on I03-I08 and the versioned menu schema. F10 must consume, not duplicate, the catalog, plugin graph, validators, transaction journal, used-content ledger, diagnostics, save-migration, updater, and trust services. F11 must accept F21/F23 asset IDs and use the same import/transform/transaction/reference services. F22 actions and focus primitives are required across every feature surface. F23 must adapt algorithms behind URPG commands and must not introduce a parallel asset/job authority. F12 must call the same domain commands as the UI. F13 enriches context but never replaces tools for live state. F14-F20 must emit ordinary typed operations through that platform.

## 13. Change Control

- Each milestone begins with a written scope and ends with its named gate review.
- Feature implementation can begin after G0; unfinished G1/G2 work blocks only dependent unsafe runtime, mutation, catalog, or release paths.
- Vendor inputs are not edited in place.
- Schema and lockfile changes require a migration and rollback story.
- Release blockers cannot be downgraded without a written rationale and named approval.
- Feature flags protect incomplete post-release features from the stable runtime.
- A feature milestone cannot request a clean production build or broad regression run.
- Acceptance automation is authored and registered with its feature, then broadly executed in L4.
- Only boot-hang, data-loss, unsafe-write, secret-exposure, or security-boundary checks may exceed the pre-G12 verification limits.
- Qualification failures are batched by subsystem before candidate rebuilds.
- No test command may hide an implicit build or packaging step.
- M13 accepts fixes, qualification, documentation, and performance work only; no missing feature scope can enter after G12.
- Planned post-release feature waves use the same scope, freeze, immutable-candidate, impact-invalidation, qualification, and promotion discipline, with a published supported-version and deprecation matrix.
- After general availability, a narrow emergency lane may patch only an actively exploitable security issue, confirmed data loss/corruption, a broken update/rollback path, or urgent signing-key/credential revocation on a supported release branch. Each hotfix freezes its own minimal inputs, creates a new immutable candidate, reruns all impact-invalidated evidence plus the critical release smoke, and contains no unrelated feature work.
- Update rollout can be paused, revoked, or rolled back without shipping unrelated changes; signing-key rotation and revocation are rehearsed operations.

## 14. Immediate Next Actions

1. Locate the actual first-party application repository and separately audit any embedded LLM implementation there; this checkout contains no AI integration to validate.
2. Complete the thin G0 spine: canonical source boundary, persistent incremental development runner, feature flags, minimal fixture, and initial versioned contracts.
3. Define stable object IDs, the reference graph, canonical domain-command API, transaction/history layer, capability broker, and concurrent-editor policy before implementing plopping or AI writes.
4. Fix only the development safety floor that blocks feature work: preload boot hangs, destructive persistence, missing-sidecar failure, untrusted execution, and secret isolation.
5. Begin F01-F10, F21, F22, F24, and I01-I10 vertical slices after G0 while remaining G1/G2 implementation proceeds in parallel; use only L0-L2 checks.
6. Complete F01's resumable onboarding, whole-project snapshots and portable archives, Storage & Offline Pack Manager, no-network mode, Recovery Center, bootstrap safe mode, and support-bundle preview before broad mutation workflows depend on them.
7. Implement F21 personal-asset intake and the F22 semantic input stack before feature screens create one-off import or controller paths.
8. Implement F24 audio/voice/caption intake, non-destructive transforms, loop/loudness QA, stable dialogue/localization links, assignment/plopping, and release checks on the shared asset/job/transaction platform.
9. Implement F11 plopping across Hub-owned targets first, then conflict-aware map/database placement and smart prefabs.
10. Pin a clean SpriteForge source revision, complete its payload/license inventory, adapt only approved services, consolidate storage/jobs, and implement F23's real MZ bridge.
11. Implement F12 on the pinned Qwen3.5-4B local profile with the application-owned tool loop; do not give the model raw filesystem, shell, network, process, plugin-loader, connector installation, or credential access.
12. Implement F13 in stages: structured/exact queries, lexical retrieval, Granite 97M semantic retrieval, then optional dedicated visual embeddings only after evaluation.
13. Close player-save compatibility, the signed Creator Hub updater, executable/SBOM/BOM provenance, extension isolation, hardened loopback IPC, OS-vault secrets, privacy/deletion, offline, and optional external-connector contracts before feature freeze.
14. Implement F14-F20 behind flags and continuously merge them into the persistent development assembly.
15. Reach G12 with F01-F24/I01-I10 implemented and every acceptance criterion mapped; do not create a clean production build or run broad qualification before this freeze.
16. Produce one immutable L3 candidate in M13, run all L4 suites in parallel against it, batch fixes, and rebuild only when candidate inputs change.
17. Release only the exact qualified artifact hash after L5 confirmation.

## 15. Definition of Done for the Roadmap

This roadmap is complete only when:

- F01-F24 and I01-I10 meet their acceptance criteria;
- G0-G13 have recorded pass evidence;
- the existing Synrec technical-debt acceptance criteria are closed;
- every shipped asset and plugin is portable, traceable, and legally reviewed;
- whole-project snapshots, portable archives, clean-root restore, storage relocation/cleanup, offline operation, bootstrap safe mode, Recovery Center, and exact support-bundle preview protect user work under interruption and corruption fixtures;
- a representative creator completes the primary workflow, including easy custom visual/audio asset intake, audio/voice/caption authoring and assignment, direct placement, controller operation, and an optional AI-assisted edit, without manual file surgery;
- the LLM helper uses only registered tools with permissions, source-version checks, transactions, audit history, privacy/cost controls, and undo;
- RAG results are incremental, project-isolated, cited, and never authoritative for live mutable state;
- all deterministic creator and release workflows remain usable with AI disabled and the defined local core remains usable with outbound network blocked;
- player saves meet the published compatibility horizon and the signed application updater rejects or recovers from tamper, replay, downgrade, interruption, revocation, and failed-first-start cases without harming user content;
- the executable inventory, SBOM/model-BOM/asset-BOM, signatures, provenance, licenses, privacy/deletion rules, secret isolation, extension/connector capabilities, and loopback-service boundaries reconcile to the exact candidate;
- clean-install, upgrade, rollback, recovery, storage, offline, long-session, audio, input, screen-reader/accessibility, localization, AI/security, supply-chain, and release matrices pass;
- one immutable artifact is qualified and released without being rebuilt or changed;
- the release is reproducible from a clean environment using checked-in documentation.
