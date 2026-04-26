# URPG Missing Features, Governance Gaps, and Template Expansion Plan

Date: 2026-04-20
Version: 2.0 — revised and expanded
Status: proposed planning addendum
Scope: missing product features, template-governance gaps, game-type readiness, the Create-a-Character runtime lane, cross-cutting concerns, and newly identified subsystem gaps

This document extends the earlier gap checklist (v1.0) and folds in the major planning work developed around:

- template/game-type readiness,
- subsystem dependency matrices,
- the robust Create-a-Character runtime system for games exported by URPG,
- **cross-cutting concerns newly identified as missing from v1.0** (accessibility, audio, input, localization completeness, versioning, performance budgeting, mod support, achievements).

This document is **not** a replacement for:

- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- `docs/PROGRAM_COMPLETION_STATUS.md`
- `docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md`

It is an addendum focused on the things URPG still needs in order to become a **trustworthy multi-template engine product**.

---

## What changed in v2.0

The following sections are new or substantially expanded relative to v1.0:

- **P0-07** — Schema Versioning and Changelog Governance *(new)*
- **P0-08** — Breaking Change Detection in CI *(new)*
- **P1-06** — Localization Completeness Checker *(new)*
- **P1-07** — Performance Budget Profiling Subsystem *(new)*
- **P1-08** — Visual Regression Testing Suite *(new)*
- **P1-09** — Accessibility Subsystem Governance *(new)*
- **P2-06** — Audio Subsystem Governance *(new)*
- **P2-07** — Input Remapping and Controller Governance *(new)*
- **P2-08** — Achievement and Trophy System *(new)*
- **P2-09** — Platform-Specific Export Validation *(new)*
- **P2-10** — Monster Collector RPG Template Spec *(new; was in catalog only)*
- **P2-11** — Cozy / Life RPG Template Spec *(new; was in catalog only)*
- **P2-12** — Metroidvania-lite Template Spec *(new; was in catalog only)*
- **P2-13** — 2.5D RPG Template Spec *(new; was in catalog only)*
- **P2-14** — Mod and Player Extension Layer *(new)*
- **P2-15** — Analytics and Opt-In Telemetry Layer *(new)*
- **Part V** — Create-a-Character expanded with subsystems I–M *(new)*
- **Part VI** — Template Readiness Matrix expanded with Audio, Localization, Accessibility, Input columns *(new)*
- **Part VII** — Cross-Cutting Concerns chapter *(new)*
- **Phase 3** added to Execution Order *(new)*
- **Definition of Done** expanded with changelog, versioning, and accessibility gates *(expanded)*

---

## Why this document exists

URPG already has substantial real implementation across:

- native-first Wave 1 ownership lanes,
- Message/Text, Battle, UI/Menu, and Save/Data slices,
- migration and compat reporting,
- advanced Wave 2 capability seeding,
- template-facing claims such as ARPG, VN, and Tactics.

What is still easy to miss are the **governance, template, and productization layers** that make those capabilities safe to expose, safe to market, and safe for users to build on.

The current repo state suggests the main risk is no longer "URPG lacks serious capability."
The bigger risk is that **template support, subsystem labels, and editor surfaces can drift away from verified reality** unless they are governed explicitly.

A second class of risk — now explicitly named — is that the following **cross-cutting concerns have no subsystem owner and no CI gate**:

- accessibility
- audio state governance
- input remapping
- localization completeness
- schema versioning and changelogs
- performance budgets

These do not belong to any single template. They belong to every template. Without them, URPG cannot honestly claim to ship finished game projects.

---

## Priority Legend

| Level | Meaning |
| --- | --- |
| P0 | Missing feature/governance layer that directly affects truthful product claims or release confidence |
| P1 | Important operational capability that materially improves project survivability and user trust |
| P2 | Valuable productization layer that should follow once the higher-priority governance pieces are in place |

---

# Part I — Missing Features and Governance Gaps

## P0 — Must Add Soon

### P0-01 — Release-Readiness Matrix by Subsystem

#### Why this is needed
Without this, subsystem claims and public readiness language will keep drifting.

#### What it should answer
For every major subsystem:

- is the runtime owner complete,
- is the editor surface complete,
- is the schema finalized,
- is migration implemented,
- is diagnostics coverage present,
- are tests sufficient,
- is the subsystem safe to label `READY`.

#### Affected templates
All templates.

#### Suggested repo additions
- `docs/RELEASE_READINESS_MATRIX.md`
- `content/schemas/readiness_status.schema.json`
- `tools/ci/check_release_readiness.ps1`
- `editor/diagnostics/release_readiness_panel.h`
- `editor/diagnostics/release_readiness_panel.cpp`

---

### P0-02 — Template Readiness Matrix

#### Why this is needed
Template support is still governed more by implication than by a formal readiness contract.

#### What it should answer
For each template:

- status (`READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`)
- required subsystems
- optional subsystems
- known blockers
- safe project scope
- export confidence
- migration confidence where relevant

#### Suggested repo additions
- `docs/TEMPLATE_READINESS_MATRIX.md`
- `docs/TEMPLATES/README.md`
- `docs/TEMPLATES/JRPG.md`
- `docs/TEMPLATES/TURN_BASED_RPG.md`
- `docs/TEMPLATES/ARPG.md`
- `docs/TEMPLATES/VN.md`
- `docs/TEMPLATES/TACTICS.md`
- `editor/project/template_selector_panel.h`
- `editor/project/template_selector_panel.cpp`

---

### P0-03 — Project Audit Command

#### Why this is needed
URPG still needs a one-click answer to: "What is incomplete or unsafe in my actual project right now?"

#### What it should report
- missing assets
- broken references
- incomplete required subsystem setup
- template blockers
- unreachable dialogue/event branches
- stale import leftovers
- unsupported constructs still present
- export blockers
- release blockers
- overclaimed readiness labels
- **missing localization keys** *(v2 addition)*
- **schema version mismatches** *(v2 addition)*
- **audio event references without registered sounds** *(v2 addition)*
- **input action references without registered bindings** *(v2 addition)*
- **accessibility violations detectable at authoring time** *(v2 addition)*

#### Suggested repo additions
- `tools/audit/urpg_project_audit.cpp`
- `docs/PROJECT_AUDIT.md`
- `editor/diagnostics/project_audit_panel.h`
- `editor/diagnostics/project_audit_panel.cpp`

---

### P0-04 — Canonical Truth Reconciler

#### Why this is needed
URPG now has several competing truth surfaces:

- README summary claims
- program completion status
- technical debt plan
- template-level marketing language
- editor diagnostics
- future readiness panels

#### What it should do
- detect documentation overclaiming
- detect subsystem label drift
- detect template label drift
- fail CI if public docs exceed verified implementation state
- force evidence-gated upgrades for `READY` / `FULL` / `COMPLETE`
- **verify schema version tags match registered changelog entries** *(v2 addition)*

#### Suggested repo additions
- `docs/TRUTH_ALIGNMENT_RULES.md`
- `tools/docs/check_truth_alignment.ps1`
- `tools/docs/check_template_claims.ps1`
- `tools/docs/check_subsystem_badges.ps1`

---

### P0-05 — Template Claim Guardrails

#### Why this is needed
A template should not be called `READY` just because the engine can theoretically compose something like it.

#### Rules to enforce
A template may be labeled `READY` only if:

- all required subsystems are themselves `READY`,
- required editor surfaces exist,
- required tests exist,
- release-readiness row exists,
- public docs and status docs agree on the template status,
- **audio, input, and localization subsystems satisfy template minimums** *(v2 addition)*,
- **accessibility gate is satisfied for all exported UI** *(v2 addition)*.

#### Suggested repo additions
- `docs/TEMPLATE_LABEL_RULES.md`
- CI integration inside `tools/docs/check_template_claims.ps1`

---

### P0-06 — Subsystem Badge Guardrails

#### Why this is needed
A subsystem may not be labeled `FULL` / `COMPLETE` / `READY` unless:

- runtime owner exists,
- editor surface exists if promised,
- schema exists if promised,
- migration exists if promised,
- diagnostics exist if promised,
- tests exist,
- documentation matches the implementation state.

#### Suggested repo additions
- `docs/SUBSYSTEM_STATUS_RULES.md`
- CI integration inside `tools/docs/check_subsystem_badges.ps1`

---

### P0-07 — Schema Versioning and Changelog Governance *(new in v2)*

#### Why this is needed
URPG manages an expanding collection of data schemas (character, battle, dialogue, save, creator, etc.). As schemas evolve, existing projects silently break if no version contract is enforced. Currently there is no evidence of a versioned schema registry or a mandatory changelog gate.

#### What it should define
- every schema must carry a `schemaVersion` field,
- every schema file must have a corresponding entry in a central `SCHEMA_CHANGELOG.md`,
- CI must reject a schema change that lacks a changelog entry,
- the migration system must be linked to schema version deltas,
- subsystem badge upgrades require a schema version bump.

#### What it should catch
- schema changes without a corresponding migration,
- version fields missing from new schemas,
- changelog entries that don't match the actual diff,
- projects referencing a schema version the engine no longer supports.

#### Suggested repo additions
- `docs/SCHEMA_VERSIONING_POLICY.md`
- `docs/SCHEMA_CHANGELOG.md`
- `content/schemas/schema_registry.json`
- `tools/ci/check_schema_versions.ps1`
- `tools/ci/check_schema_changelog.ps1`

---

### P0-08 — Breaking Change Detection in CI *(new in v2)*

#### Why this is needed
Schema changes, API renames, and subsystem restructures that silently break existing projects are one of the highest trust risks for any engine product. URPG currently has no explicit CI gate for this.

#### What it should do
- compare current schema against last tagged release,
- compare public runtime API surface against last tagged release,
- flag removals, renames, and type changes as breaking,
- fail CI on unacknowledged breaking changes,
- require a `BREAKING_CHANGES.md` entry and major version bump before a breaking change merges.

#### What counts as breaking
- removing a schema field that had no `deprecated` flag,
- renaming a runtime owner class or method,
- changing the expected type or range of a schema field,
- removing a template feature without replacement.

#### Suggested repo additions
- `docs/BREAKING_CHANGE_POLICY.md`
- `docs/BREAKING_CHANGES.md`
- `tools/ci/detect_breaking_changes.ps1`
- `tools/ci/schema_diff_report.ps1`

---

## P1 — Important Operational Features

### P1-01 — Narrative Graph / Event Graph Linting

#### What it should catch
- unreachable branches
- dead-end choices
- unresolved speaker IDs
- broken token usage
- missing localization keys
- invalid condition references
- orphaned dialogue sequences
- loops without safe exit
- **missing voice line references when voice is enabled** *(v2 addition)*
- **narrative tags referenced in conditions that are never produced** *(v2 addition)*

#### Suggested repo additions
- `engine/core/message/message_graph_linter.h`
- `engine/core/message/message_graph_linter.cpp`
- `editor/message/message_lint_panel.h`
- `editor/message/message_lint_panel.cpp`
- `docs/NARRATIVE_LINTING.md`

---

### P1-02 — Asset Dependency / "Where Used" Browser

#### What it should answer
- where is this asset used
- what breaks if I remove it
- what maps/scenes/battles/dialogue chains reference it
- which imported assets still lack native replacement
- **which audio events reference this asset** *(v2 addition)*
- **which character creator parts depend on this art asset** *(v2 addition)*

#### Suggested repo additions
- `engine/core/assets/asset_dependency_graph.h`
- `engine/core/assets/asset_dependency_graph.cpp`
- `editor/assets/asset_usage_panel.h`
- `editor/assets/asset_usage_panel.cpp`
- `docs/ASSET_DEPENDENCY_GRAPH.md`

---

### P1-03 — Runtime Record / Replay System

#### What it should support
- battle replay
- event replay
- scene replay
- bug-report playback
- deterministic comparison between revisions
- **input sequence replay for regression testing input remapping** *(v2 addition)*
- **accessibility audit replay (confirm UI was reachable at each step)** *(v2 addition)*

#### Suggested repo additions
- `engine/core/debug/runtime_replay.h`
- `engine/core/debug/runtime_replay.cpp`
- `editor/diagnostics/replay_panel.h`
- `editor/diagnostics/replay_panel.cpp`
- `docs/REPLAY_AND_REGRESSION.md`

---

### P1-04 — Template-Aware Onboarding / Guided Setup

#### What it should include
- first VN scene
- first JRPG battle
- first tactics encounter
- first ARPG room
- first message/localization path
- first save/export test
- **first audio event binding walkthrough** *(v2 addition)*
- **first input action mapping setup** *(v2 addition)*
- **first character creator configuration** *(v2 addition)*

#### Suggested repo additions
- `docs/TUTORIALS/`
- `editor/project/guided_setup_wizard.h`
- `editor/project/guided_setup_wizard.cpp`
- `content/templates/tutorials/`

---

### P1-05 — Content Coverage Analysis

#### What it should analyze
- every enemy has rewards/AI/death handling
- every class has progression and legal equipment
- every dialogue branch terminates or resolves
- every template has required starting scenes
- every battle troop has valid action data
- every imported unsupported construct is surfaced
- **every audio event ID used in scripts has a registered sound** *(v2 addition)*
- **every input action used in UI has a registered binding** *(v2 addition)*
- **every localization key used in templates has a string entry in all enabled locales** *(v2 addition)*

#### Suggested repo additions
- `engine/core/validation/content_coverage_analyzer.*`
- `editor/diagnostics/content_coverage_panel.*`
- `docs/CONTENT_COVERAGE_ANALYSIS.md`

---

### P1-06 — Localization Completeness Checker *(new in v2)*

#### Why this is needed
Localization is referenced throughout the document (message/text, VN, narrative) but there is currently no subsystem that owns completeness. A shipped game with missing translations silently falls back or crashes.

#### What it should validate
- every string key used in UI, dialogue, menus, and tooltips has a registered entry in every enabled locale,
- no locale file is missing keys that exist in the primary locale,
- no key is registered but unused,
- all locales pass character encoding checks,
- fonts support all characters required by enabled locales,
- locale-specific asset overrides (fonts, images, audio) are present when declared.

#### Suggested repo additions
- `engine/core/localization/localization_completeness_checker.h`
- `engine/core/localization/localization_completeness_checker.cpp`
- `editor/localization/localization_audit_panel.h`
- `editor/localization/localization_audit_panel.cpp`
- `tools/ci/check_localization_completeness.ps1`
- `docs/LOCALIZATION_COMPLETENESS.md`

---

### P1-07 — Performance Budget Profiling Subsystem *(new in v2)*

#### Why this is needed
Different game templates have very different performance profiles. An ARPG running heavy collision and sprite compositing has different frame-time budgets than a VN. Without a formal budget system, released templates can ship with silent performance regressions.

#### What it should define and enforce
- per-template draw call budgets,
- per-scene memory upper bounds,
- frame-time budget targets (16ms / 33ms thresholds),
- sprite layer composition budget per frame,
- audio voice channel limits,
- CI profile runs that fail if a template sample project exceeds its declared budget.

#### Suggested repo additions
- `engine/core/profiling/performance_budget.h`
- `engine/core/profiling/performance_budget.cpp`
- `editor/diagnostics/performance_budget_panel.h`
- `editor/diagnostics/performance_budget_panel.cpp`
- `tools/ci/run_performance_budgets.ps1`
- `docs/PERFORMANCE_BUDGETS.md`
- `content/templates/budgets/` (per-template budget config files)

---

### P1-08 — Visual Regression Testing Suite *(new in v2)*

#### Why this is needed
UI, portrait composition, battle scenes, and sprite layer stacking are all highly sensitive to silent visual regressions caused by rendering order changes, palette changes, or rig anchor shifts. Currently there is no visual regression gate in CI.

#### What it should cover
- per-screen UI screenshots compared against approved baselines,
- layered portrait rendering compared against approved baselines,
- battle scene layout compared against approved baselines,
- character creator preview rendering compared against approved baselines,
- automatic diff flagging on pixel-significant changes,
- approved baseline management workflow (who can approve a visual change).

#### Suggested repo additions
- `tools/ci/visual_regression_suite/`
- `tools/ci/run_visual_regression.ps1`
- `tests/visual_baselines/` (per-screen and per-template baseline images)
- `docs/VISUAL_REGRESSION_TESTING.md`

---

### P1-09 — Accessibility Subsystem Governance *(new in v2)*

#### Why this is needed
Accessibility is absent from the current plan entirely. This is increasingly a legal, ethical, and commercial requirement for shipped products. No single template can claim to be "ready" if its UI surfaces are inaccessible.

#### What it should cover
- **Input**: full keyboard/controller navigation for all UI screens; no mouse-only surfaces,
- **Visual**: colorblind-safe palette options; minimum contrast ratios on UI text; scalable font sizes,
- **Audio**: subtitles/captions on all voiced content; visual indicators for audio-only cues,
- **Cognitive**: skip options for long cutscenes and tutorials; pause-anywhere support; readable font defaults,
- **Authoring**: accessibility checklist built into the Project Audit Command and export validation,
- **CI**: accessibility linting on UI schema definitions.

#### Accessibility labels for templates
Templates should carry an accessibility readiness label:
- `ACCESSIBLE`: meets minimum standards
- `PARTIAL`: some gaps remain
- `NOT_AUDITED`: not yet reviewed

#### Suggested repo additions
- `docs/ACCESSIBILITY_POLICY.md`
- `docs/ACCESSIBILITY_CHECKLIST.md`
- `engine/ui/accessibility/accessibility_manager.h`
- `engine/ui/accessibility/accessibility_manager.cpp`
- `editor/diagnostics/accessibility_audit_panel.h`
- `editor/diagnostics/accessibility_audit_panel.cpp`
- `tools/ci/check_accessibility_compliance.ps1`

---

## P2 — Productization and Template-Specific Gaps

### P2-01 — Template Productization Docs
Each template needs:
- intended genre/use case
- required subsystems
- optional subsystems
- known blockers
- recommended project size
- import notes
- current readiness label
- **accessibility readiness label** *(v2 addition)*
- **localization readiness label** *(v2 addition)*
- **audio readiness label** *(v2 addition)*

Suggested repo additions:
- `docs/TEMPLATES/*.md`

---

### P2-02 — ARPG Runtime/Editor Closure Visibility
Still needed:
- explicit ARPG template spec
- hitbox/hurtbox debugging surfaces
- real-time combat timing/state views
- clearer authoring workflow docs
- **audio state sync debugging surface** *(v2 addition)*
- **performance budget panel integration for action-heavy scenes** *(v2 addition)*

Suggested repo additions:
- `docs/ARPG_TEMPLATE_SPEC.md`
- `editor/action/`
- `editor/debug/combat_debug_overlay.*`

---

### P2-03 — Tactics Scenario Authoring Layer
Still needed:
- tactical objective editor
- reinforcement/spawn scripting
- battle-map coupling tools
- tactics scenario validation
- **procedural encounter seed debugging** *(v2 addition)*

Suggested repo additions:
- `docs/TACTICS_TEMPLATE_SPEC.md`
- `editor/tactics/scenario_editor.*`

---

### P2-04 — Dungeon / Roguelite Template Packaging
Still needed:
- run-loop structure
- encounter/scenario generator packaging
- seed-debug UI
- dungeon-specific onboarding
- release-readiness rules for procedural templates
- **run persistence / meta-progression save layer** *(v2 addition)*
- **relic/item run economy spec** *(v2 addition)*

Suggested repo additions:
- `docs/DUNGEON_TEMPLATE_SPEC.md`
- `docs/ROGUELITE_TEMPLATE_SPEC.md`
- `editor/procedural/run_seed_panel.*`

---

### P2-05 — Character Identity / Create-a-Character Template Lane
Still needed:
- character-creator runtime module
- template rules for custom protagonists
- appearance/portrait/battle-actor recipe system
- narrative/gameplay tag binding for created characters

Suggested repo additions:
- `docs/CHARACTER_CREATOR_RUNTIME_SPEC.md`
- `docs/TEMPLATES/CHARACTER_CREATION_SUPPORT.md`
- `engine/core/character_creator/`
- `editor/character_creator/`

---

### P2-06 — Audio Subsystem Governance *(new in v2)*

#### Why this is needed
Audio is referenced in passing (audio state sync for ARPG) but has no formal subsystem ownership, no readiness label, and no CI coverage. Every shipped template will have audio. This must be owned.

#### What it should define
- audio event registry (every sound registered and versioned),
- audio state machine spec (what states exist, what transitions trigger sounds),
- music/ambience layer spec (layers, crossfade rules, priority rules),
- voice line pipeline spec (filename conventions, locale variants, missing-file behavior),
- audio budget per template (max concurrent voices, max memory for audio assets),
- audio editor surface (event browser, preview player, state machine editor),
- CI validation (no unregistered event IDs in scripts; no missing locale voice files).

#### Suggested repo additions
- `docs/AUDIO_SUBSYSTEM_SPEC.md`
- `engine/core/audio/audio_event_registry.h`
- `engine/core/audio/audio_event_registry.cpp`
- `engine/core/audio/audio_state_machine.h`
- `engine/core/audio/audio_state_machine.cpp`
- `editor/audio/audio_event_browser.*`
- `editor/audio/audio_state_editor.*`
- `tools/ci/check_audio_coverage.ps1`

---

### P2-07 — Input Remapping and Controller Governance *(new in v2)*

#### Why this is needed
No mention of input governance exists in the current plan. Every template ships UI and gameplay that requires input bindings. Without a formal input subsystem, templates independently invent input handling and drift apart, making it impossible to guarantee controller support across templates.

#### What it should define
- input action registry (every action named and versioned),
- default binding maps per platform (keyboard/mouse, gamepad, touch),
- runtime remapping API (player can rebind any action),
- conflicting binding detection and resolution,
- UI navigation input contract (all menus must be navigable without a mouse),
- editor surface for authoring and previewing input action sets,
- CI validation (no unregistered input action IDs used in scripts or UI).

#### Suggested repo additions
- `docs/INPUT_SUBSYSTEM_SPEC.md`
- `engine/core/input/input_action_registry.h`
- `engine/core/input/input_action_registry.cpp`
- `engine/core/input/input_remapper.h`
- `engine/core/input/input_remapper.cpp`
- `editor/input/input_binding_editor.*`
- `tools/ci/check_input_coverage.ps1`

---

### P2-08 — Achievement and Trophy System *(new in v2)*

#### Why this is needed
Achievements are a near-universal expectation in RPGs across all platforms. No mention of an achievement layer exists in the plan. Without it, every game built on URPG must invent its own achievement wiring, making a consistent release story impossible.

#### What it should define
- achievement definition schema (id, name, description, unlock condition, icon),
- platform backend adapters (Steam Achievements, console trophy systems, generic),
- runtime achievement tracker (condition evaluation engine, progress tracking),
- save/data integration (achievement unlock state is part of the save record),
- editor surface (achievement list editor, condition builder, test-unlock panel),
- CI validation (no referenced achievement IDs without definitions).

#### Suggested repo additions
- `docs/ACHIEVEMENT_SYSTEM_SPEC.md`
- `content/schemas/achievement_definition.schema.json`
- `engine/core/achievements/achievement_tracker.h`
- `engine/core/achievements/achievement_tracker.cpp`
- `engine/core/achievements/platform_achievement_adapter.h`
- `editor/achievements/achievement_editor.*`
- `tools/ci/check_achievement_coverage.ps1`

---

### P2-09 — Platform-Specific Export Validation *(new in v2)*

#### Why this is needed
Export is described generically. URPG games will ship to PC, potentially console, and potentially mobile. Each platform has different constraints. Without platform-aware export validation, a passing export on one platform may fail silently on another.

#### What it should validate per platform
- **PC**: path length limits, dependency packaging, executable naming, icon sizes
- **Console** (future): TRC/TCR compliance stubs, save data size limits, controller-only navigation required, age rating metadata
- **Mobile** (future): asset resolution tiers, battery-aware audio, touch input mapping, orientation support
- **All**: localization coverage per target market, achievement platform backend selected

#### Suggested repo additions
- `docs/PLATFORM_EXPORT_VALIDATION.md`
- `tools/export/platform_export_validator.cpp`
- `content/schemas/platform_export_config.schema.json`
- `editor/export/platform_export_panel.*`
- `tools/ci/run_platform_export_checks.ps1`

---

### P2-10 — Monster Collector RPG Template Spec *(new in v2)*

#### Why this is needed
Monster Collector RPG is listed in the game-type catalog as EXPERIMENTAL but has no template spec, no dependency notes, and no authoring guidance. It is one of the most requested RPG subgenres commercially and requires distinct subsystems.

#### Unique subsystem requirements
- monster roster/party management (separate from standard party),
- capture/encounter mechanic integration into battle,
- monster evolution/transformation pipeline,
- monster-specific stat and move-learning schema,
- monster breeding/fusion layer (optional),
- PC storage box system (extended monster collection management),
- wild encounter rate/zone tables.

#### Suggested repo additions
- `docs/TEMPLATES/MONSTER_COLLECTOR.md`
- `docs/MONSTER_COLLECTOR_TEMPLATE_SPEC.md`
- `content/schemas/monster_definition.schema.json`
- `content/schemas/capture_rule.schema.json`
- `engine/core/monster_collector/`
- `editor/monster_collector/`

---

### P2-11 — Cozy / Life RPG Template Spec *(new in v2)*

#### Why this is needed
Cozy / Life RPG is listed as EXPERIMENTAL with no spec. The subsystem profile for this genre is substantially different from combat-focused templates, with unique systems that currently have no owner.

#### Unique subsystem requirements
- relationship/affinity progression system (NPC friendship/romance),
- daily time cycle and schedule system,
- crafting/farming/gathering loop,
- home/space customization layer,
- low-stress difficulty options (no game over, optional combat),
- event calendar (seasonal festivals, NPC birthday events),
- player home persistence and decoration schema.

#### Suggested repo additions
- `docs/TEMPLATES/COZY_LIFE_RPG.md`
- `docs/COZY_LIFE_TEMPLATE_SPEC.md`
- `content/schemas/npc_relationship.schema.json`
- `content/schemas/time_cycle.schema.json`
- `engine/core/life_sim/`
- `editor/life_sim/`

---

### P2-12 — Metroidvania-lite Template Spec *(new in v2)*

#### Why this is needed
Metroidvania-lite is listed as EXPERIMENTAL with no spec. This genre requires specific spatial and ability-gating subsystems not covered by any current template.

#### Unique subsystem requirements
- ability-gated map progression (rooms locked until ability acquired),
- persistent map state (rooms remember their cleared/explored state),
- backtracking and fast travel layer,
- upgrade acquisition and ability unlock pipeline,
- boss gate validation (all required abilities reachable before each boss),
- room transition and screen edge system.

#### Suggested repo additions
- `docs/TEMPLATES/METROIDVANIA_LITE.md`
- `docs/METROIDVANIA_TEMPLATE_SPEC.md`
- `content/schemas/room_gate.schema.json`
- `engine/core/metroidvania/ability_gate_system.h`
- `engine/core/metroidvania/map_persistence.h`
- `editor/metroidvania/gate_editor.*`

---

### P2-13 — 2.5D RPG Template Spec *(new in v2)*

#### Why this is needed
2.5D RPG is listed as EXPERIMENTAL and partially supported in the readiness matrix, but has no spec or dependency notes. The rendering pipeline assumptions and authoring workflow for 2.5D are unique and need explicit documentation.

#### Unique subsystem requirements
- depth-sorting layer management (sprites in 3D space sorted correctly),
- parallax background system,
- perspective-aware lighting layer,
- 3D camera rig for 2D-sprite actors,
- billboard vs. flat-plane actor modes,
- authoring-time depth preview in the editor.

#### Suggested repo additions
- `docs/TEMPLATES/2_5D_RPG.md`
- `docs/2_5D_TEMPLATE_SPEC.md`
- `engine/rendering/depth_sort_manager.h`
- `engine/rendering/parallax_background.h`
- `editor/rendering/depth_preview_panel.*`

---

### P2-14 — Mod and Player Extension Layer *(new in v2)*

#### Why this is needed
RPGs in particular have strong modding communities. A formal mod layer protects the engine's stability while allowing third-party content. Without it, modders directly hack shipped data, which makes engine upgrades impossible.

#### What it should define
- safe data extension points (which schemas can be extended by mods),
- mod manifest format (id, version, dependencies, load order),
- mod conflict detection and load order resolution,
- sandboxed script execution scope for mod scripts,
- mod validation at game load (missing dependencies fail gracefully),
- official mod SDK documentation,
- mod-aware save versioning (saves record which mods were active).

#### Suggested repo additions
- `docs/MOD_SDK.md`
- `content/schemas/mod_manifest.schema.json`
- `engine/core/mods/mod_loader.h`
- `engine/core/mods/mod_conflict_resolver.h`
- `editor/mods/mod_manager_panel.*`
- `tools/ci/validate_mod_manifests.ps1`

---

### P2-15 — Analytics and Opt-In Telemetry Layer *(new in v2)*

#### Why this is needed
Developers shipping games built on URPG have no built-in way to understand player behavior, funnel drop-off, or session length. Without opt-in telemetry tooling, every developer must wire this up themselves in incompatible ways.

#### What it should define
- opt-in consent screen contract (players must consent before any data is sent),
- event taxonomy (session start/end, scene enter/exit, battle result, dialogue choice, death, export),
- privacy-safe data format (no PII; all data anonymized at collection),
- backend adapter interface (developers plug in their own endpoint),
- local telemetry preview mode for playtesting without a live backend,
- GDPR/CCPA compliance checklist embedded in export validation.

#### Suggested repo additions
- `docs/ANALYTICS_TELEMETRY_SPEC.md`
- `docs/PRIVACY_COMPLIANCE_CHECKLIST.md`
- `content/schemas/telemetry_event.schema.json`
- `engine/core/telemetry/telemetry_manager.h`
- `engine/core/telemetry/telemetry_consent_gate.h`
- `editor/telemetry/telemetry_preview_panel.*`

---

# Part II — Recommended Game-Type Catalog

## Primary game types
These should be first-class in the product:

- JRPG
- Turn-Based RPG
- ARPG
- Visual Novel
- Tactics RPG

## Secondary supported types
These should be presented as supported but not headline promises:

- Adventure RPG
- Narrative Adventure
- Action-Adventure
- Tactical VN

## Experimental templates
These should carry explicit experimental badges:

- Dungeon Crawler
- Roguelite / Roguelike
- 2.5D RPG
- Monster Collector RPG
- Cozy / Life RPG
- Metroidvania-lite

## Future investigation templates *(new in v2)*
These have been requested or observed in the market and should be tracked but make no current claim:

- Deck-Builder RPG (card-based battle layer)
- Survival RPG (resource management + degradation)
- Rhythm RPG (beat-based input layer)
- Social Sim RPG (heavy NPC schedule + relationship systems)
- City Builder RPG (map + economy layer)

---

# Part III — Template Readiness Matrix (Proposed)

## Readiness values
- **READY**: safe to market now
- **PARTIAL**: meaningful support exists, but important closure is still in progress
- **EXPERIMENTAL**: foundational support exists, but not enough to promise broadly
- **BLOCKED**: should not be exposed yet
- **N/A**: not applicable to this game type

## Subsystem columns *(v2 adds Audio, Localization, Accessibility, Input)*

| Game type | Menu | Message/Text | Battle | Save/Data | Abilities | Pattern/Tactics | Level Assembly | Sprite Pipeline | Procedural | Timeline/Anim | 2.5D | Audio | Localization | Accessibility | Input | Template Readiness |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| JRPG | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | N/A | PARTIAL | PARTIAL | N/A | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| Turn-Based RPG | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | N/A | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | N/A | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| ARPG | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | N/A | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| Visual Novel | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | N/A | N/A | N/A | OPTIONAL | N/A | PARTIAL | N/A | PARTIAL | PARTIAL | PARTIAL | PARTIAL | READY/PARTIAL |
| Tactics RPG | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | N/A | PARTIAL | PARTIAL | PARTIAL | PARTIAL | READY/PARTIAL |
| Adventure RPG | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | N/A | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| Narrative Adventure | PARTIAL | PARTIAL | N/A | PARTIAL | N/A | N/A | N/A | OPTIONAL | N/A | PARTIAL | N/A | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| Dungeon Crawler | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | OPTIONAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | EXPERIMENTAL |
| Roguelite / Roguelike | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | EXPERIMENTAL |
| Action-Adventure | PARTIAL | OPTIONAL | OPTIONAL | PARTIAL | PARTIAL | N/A | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| 2.5D RPG | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | EXPERIMENTAL |
| Monster Collector RPG | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | N/A | PARTIAL | PARTIAL | PARTIAL | PARTIAL | EXPERIMENTAL |
| Cozy / Life RPG | PARTIAL | PARTIAL | N/A | PARTIAL | N/A | N/A | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | EXPERIMENTAL |
| Tactical VN | PARTIAL | PARTIAL | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | OPTIONAL | N/A | PARTIAL | N/A | PARTIAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL |
| Metroidvania-lite | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | N/A | PARTIAL | PARTIAL | OPTIONAL | PARTIAL | OPTIONAL | PARTIAL | PARTIAL | PARTIAL | PARTIAL | EXPERIMENTAL |

---

# Part IV — Template Dependency Notes

## JRPG
Core:
- UI/Menu Core
- Message/Text Core
- Battle Core
- Save/Data Core

Important:
- timeline/animation orchestration
- sprite pipeline toolkit
- migration/import trust
- release-readiness matrix
- battle preview/editor productization
- **audio subsystem (BGM layers, battle stings, voice lines)** *(v2)*
- **localization completeness (JRPG is typically localized into multiple languages)** *(v2)*

Key blocker:
- Wave 1 closure still incomplete across menu/message/battle/save-data

---

## Turn-Based RPG
Core:
- Battle Core
- UI/Menu Core
- Save/Data Core
- Message/Text Core

Important:
- gameplay ability framework
- battle migration/schema coverage
- battle preview/inspector
- release validation

Key blocker:
- battle readiness truth alignment still needs cleanup

---

## ARPG
Core:
- action-capable project template
- UI/Menu Core
- Save/Data Core
- gameplay ability framework
- timeline/animation orchestration

Important:
- sprite pipeline toolkit
- modular level assembly
- transient FX/events
- spatial presentation
- audio state sync
- **input remapping (ARPG is the most input-sensitive template)** *(v2)*
- **performance budgets (ARPG has the highest per-frame cost)** *(v2)*

Key blocker:
- ARPG is named, but its closure story is not documented as clearly as Menu/Message/Battle/Save

---

## Visual Novel
Core:
- Message/Text Core
- UI/Menu Core
- Save/Data Core
- scene flow / dialogue choices / narrative routing

Important:
- text rendering closure
- message inspector/preview
- localization/hot reload
- portrait/sprite pipeline
- timeline orchestration
- **voice line pipeline (VN is the template most likely to be voiced)** *(v2)*
- **accessibility (subtitles, text scaling, contrast — VN must be accessible by default)** *(v2)*

Key blocker:
- backend text-command consumption and message-scene handoff

---

## Tactics RPG
Core:
- Battle Core
- UI/Menu Core
- Save/Data Core
- pattern field editor
- gameplay ability framework

Important:
- modular level assembly
- timeline/animation orchestration
- battle preview/editor tooling
- deterministic placement/validation
- scenario generators

Key blocker:
- tactics-specific tooling is still not packaged as a first-class template surface

---

## Monster Collector RPG *(new in v2)*
Core:
- Battle Core
- UI/Menu Core
- Save/Data Core
- Message/Text Core

Important:
- capture mechanic integration
- monster roster schema
- PC box / storage system
- wild encounter zone tables
- evolution pipeline

Key blocker:
- no template spec or subsystem ownership exists; must start from scratch

---

## Cozy / Life RPG *(new in v2)*
Core:
- UI/Menu Core
- Save/Data Core
- Message/Text Core

Important:
- NPC relationship system
- time cycle and schedule system
- crafting/gathering loop
- event calendar
- home customization

Key blocker:
- no time-cycle or relationship subsystem exists; must be built new

---

## Metroidvania-lite *(new in v2)*
Core:
- UI/Menu Core
- Save/Data Core
- Level Assembly

Important:
- ability gate system
- persistent map state
- room transition system
- backtracking and fast travel

Key blocker:
- ability-gated map progression has no current subsystem owner

---

# Part V — Create-a-Character Runtime Module

## Goal
URPG must be able to export games that include a **real in-game Create-a-Character system**.
Not just an editor-side importer.
Not just a cosmetic picker.
A runtime module that ships inside the finished game.

## Core rule
This system must exist in two layers:

### 1. URPG authoring side
Used by the game creator to define:
- what the player can customize
- what races/classes/backgrounds exist
- which art parts are legal together
- how stats are assigned
- which dialogue and story tags are produced
- which templates use custom protagonists

### 2. Exported game runtime side
Used by the player to:
- create their protagonist
- preview visuals
- preview portrait
- choose class/background/origin
- allocate stats
- finalize character
- save/load that character

---

## System goals
The exported game should support:

- full protagonist creation
- modular appearance assembly
- race/species/body model support
- class/job selection
- background/origin selection
- stat/archetype/perk logic
- portrait generation from the same parts
- field sprite generation
- battle sprite generation
- equipment visibility updates
- dialogue and narrative tag integration
- cutscene-safe protagonist substitution
- save/load persistence
- optional recruit generation later
- **voice type selection** *(v2)*
- **multi-protagonist support** *(v2)*
- **mid-game barbershop / partial respec** *(v2)*
- **companion/recruit character creation** *(v2)*
- **NPC affinity seeds produced from creation choices** *(v2)*

---

## Runtime modules required

### A. Character Creation Rules Engine
Controls what is allowed:
- valid species/body combinations
- valid appearance options per species/body
- valid class restrictions
- starting stat rules
- origin/background effects
- perk/flaw rules
- narrative tags produced by selections

### B. Appearance Assembly System
Slot-based layered composition:
- base body
- head
- skin overlay
- eyes
- brows
- mouth
- hair back/front
- ears/horns
- torso outfit
- leg outfit
- gloves
- boots
- cloak
- accessories
- weapon
- shield
- aura/fx overlays

Each part must track:
- slot id
- compatible rig/body types
- animation offsets
- draw order
- palette masks
- compatibility tags

### C. Animation Compatibility Layer
Every visual part must obey a rig contract:
- same frame naming
- same anchor system
- same animation states
- same pose assumptions

### D. Character Identity Builder
Builds the final `PlayerCharacterDefinition` from user selections.

### E. Portrait Composer
Builds:
- layered portraits
- compatible preset portraits
- expression variants
- palette-synced portrait outputs

### F. Gameplay Binding Layer
Creation choices must affect:
- stats
- class
- skills
- starting gear
- passives
- progression seeds
- affinity/faction hooks

### G. Narrative Binding Layer
Creation choices must produce:
- player name/pronoun tokens
- dialogue condition tags
- origin/class/species/background reactions
- intro variant flags

### H. Persistence Layer
Saves must store:
- raw creation choices
- derived character object
- visual recipes
- current equipment overrides
- generated narrative tags

### I. Voice Selection System *(new in v2)*
For games with voiced protagonists:
- voice type options presented at creation (e.g., voice_type_01 through voice_type_N),
- voice type written into `PlayerCharacterDefinition`,
- dialogue system uses voice type to resolve voiced line variants,
- voice type must be changeable in barbershop if voice is also a barbershop option,
- export validator confirms all selected voice types have audio assets for all enabled locales.

### J. NPC Affinity Seed System *(new in v2)*
Creation choices produce initial NPC relationship modifiers:
- species/race may start with positive or negative affinity with specific factions,
- background/origin produces affinity seeds with specific NPCs or factions,
- class may alter guild or faction starting relations,
- affinity seeds are written to the save record at creation and consumed by the relationship system.

### K. Barbershop / Mid-Game Respec Interface *(new in v2)*
After initial character creation, players may access limited respec:
- authoring-side config governs which slots are rebarbable (appearance, name, pronouns, voice type),
- authoring-side config governs which slots are locked after creation (class, species, background),
- barbershop is a runtime screen separate from the creation flow,
- barbershop changes must re-validate portrait, field, and battle recipes,
- respec (stat re-allocation) follows a separate configurable ruleset (cost, frequency limit),
- all barbershop changes are persisted to save.

### L. Companion / Recruit Character Creation *(new in v2)*
Some games allow creating recruitable party members using a simplified creation flow:
- authoring-side config defines a "recruit creation depth" (may be narrower than protagonist depth),
- recruit creation uses the same rules engine and appearance system as protagonist creation,
- recruit characters receive their own `PlayerCharacterDefinition`-style records,
- multiple recruits can be created per session,
- recruit records are stored in the party roster alongside the protagonist record.

### M. Multi-Protagonist Support *(new in v2)*
Some games (e.g., party-of-3 JRPGs) allow creating multiple playable protagonists:
- authoring-side config defines how many protagonists can be created,
- each protagonist gets a full creation flow or a subset,
- created protagonists are stored as an ordered list in the save record,
- the active/lead protagonist may be switchable during play,
- narrative tags from all created protagonists are available to the dialogue system.

---

## Runtime flow
The shipped game should run:

1. identity
2. species/body
3. appearance
4. voice type *(if enabled)*
5. class
6. background/origin
7. stats
8. traits/flaws
9. portrait
10. confirm
11. finalize character object
12. write NPC affinity seeds *(v2)*
13. write new save
14. start game

---

## Core data assets

Required asset families:

- `CharacterCreatorTemplate`
- `SpeciesDefinition`
- `BodyFrameDefinition`
- `CharacterPartDefinition`
- `AnimationRigDefinition`
- `PortraitPartDefinition`
- `ClassDefinition`
- `BackgroundDefinition`
- `TraitDefinition`
- `FlawDefinition`
- `PlayerCharacterDefinition`
- `CharacterVisualRecipe`
- `VoiceTypeDefinition` *(v2)*
- `AffinitySeedDefinition` *(v2)*
- `RecruitCreatorTemplate` *(v2)*

---

## Proposed folder structure

```text
urpg/
  engine/
    character_creator/
      editor/
      runtime/
      rendering/
      validation/
      data/
      export/
      barbershop/        ← new in v2
      recruit/           ← new in v2
      voice/             ← new in v2
```

```text
game_root/
  data/
    character_creator/
  content/
    character_creator/
  runtime/
    character_creator/
```

---

## Example asset: CharacterCreatorTemplate

```json
{
  "id": "main_protagonist_creator_v1",
  "schemaVersion": "1.2.0",
  "enabled": true,
  "protagonistMode": "custom",
  "creationDepth": "full",
  "enabledScreens": [
    "identity",
    "species",
    "appearance",
    "voice",
    "class",
    "background",
    "stats",
    "traits",
    "portrait",
    "confirm"
  ],
  "allowedSpecies": ["human", "elf", "lykari"],
  "allowedClasses": ["fighter", "spellblade", "warden"],
  "allowedBackgrounds": ["streetborn", "noble", "mercenary"],
  "allowedVoiceTypes": ["voice_01", "voice_02", "voice_03"],
  "traitPoints": 2,
  "flawPointsRequired": 1,
  "statMode": "point_buy",
  "statBudget": 20,
  "portraitMode": "layered",
  "fieldVisualMode": "layered_with_cache",
  "battleVisualMode": "layered_with_cache",
  "equipmentVisualsEnabled": true,
  "barbershopEnabled": true,
  "barbershopRebarbableSlots": ["appearance", "name", "pronouns", "voice"],
  "respecEnabled": false,
  "narrativeMode": "semi_authored",
  "multiProtagonistCount": 1,
  "recruitCreationEnabled": false
}
```

---

## Example asset: PlayerCharacterDefinition

```json
{
  "id": "pc_001",
  "schemaVersion": "1.2.0",
  "name": "Doc",
  "pronounSet": "he_him",
  "voiceType": "voice_02",
  "species": "human",
  "bodyFrame": "medium",
  "class": "spellblade",
  "background": "mercenary",
  "traits": ["quick_reflexes"],
  "flaws": ["burnout_prone"],
  "stats": {
    "str": 7,
    "dex": 8,
    "vit": 6,
    "int": 6,
    "spi": 3,
    "cha": 2
  },
  "appearanceSelections": {
    "skin_tone": "skin_03",
    "eyes": "eyes_02",
    "mouth": "mouth_01",
    "hair_front": "hair_front_07",
    "hair_back": "hair_back_07",
    "torso_outfit": "starter_spellblade_torso_01",
    "legs_outfit": "starter_spellblade_legs_01"
  },
  "paletteSelections": {
    "hair_primary": "black_01",
    "cloth_primary": "blue_03",
    "cloth_secondary": "white_01"
  },
  "portraitRecipeId": "portrait_pc_001",
  "fieldRecipeId": "field_pc_001",
  "battleRecipeId": "battle_pc_001",
  "dialogueTags": [
    "species_human",
    "class_spellblade",
    "background_mercenary",
    "trait_quick_reflexes",
    "flaw_burnout_prone"
  ],
  "affinitySeeds": {
    "merchant_guild": 10,
    "warrior_order": 5,
    "npc_elara": -5
  }
}
```

---

## Editor tooling required

To make this usable in URPG, add:

- Character Creator Template Editor
- Species/Class/Background Editor
- Part Library Manager
- Rig Compatibility Editor
- Portrait Composer Editor
- Narrative Tag Editor
- Preview Simulator
- Export Validator
- **Voice Type Assignment Panel** *(v2)*
- **Affinity Seed Editor** *(v2)*
- **Barbershop Config Panel** *(v2)*
- **Recruit Creator Config Panel** *(v2)*
- **Multi-Protagonist Roster Editor** *(v2)*

---

## Export pipeline
When a game with Create-a-Character is exported:

1. validate creator template
2. validate slot coverage
3. validate rig compatibility
4. validate portraits
5. validate class/background data
6. validate narrative tags
7. **validate voice type audio asset coverage per locale** *(v2)*
8. **validate affinity seed target IDs exist in NPC database** *(v2)*
9. **validate barbershop config against allowed slots** *(v2)*
10. write manifests
11. optionally pre-bake caches
12. package runtime creator module into exported game

---

## Acceptance criteria
This subsystem is only "real" when:

- the shipped game can create a protagonist at runtime,
- the protagonist renders correctly in portrait, field, and battle,
- save/load reconstructs the same character,
- dialogue can react to creator tags,
- equipment can update visuals if enabled,
- **NPC affinities are seeded correctly from creation choices**, *(v2)*
- **voice lines resolve correctly to the selected voice type**, *(v2)*
- **barbershop correctly updates only rebarbable slots**, *(v2)*
- export fails cleanly when creator config is incomplete.

---

# Part VI — Cross-Cutting Concerns *(new in v2)*

Cross-cutting concerns are subsystems that do not belong to any single template but must be satisfied across all of them. These are the concerns most likely to be skipped because no template "owns" them. Every template readiness claim must satisfy these minimum bars.

## VI-A — Accessibility Minimum Bar
Every template claiming `PARTIAL` or higher must:

- have all menus navigable by keyboard or controller alone,
- have minimum 4.5:1 contrast ratio on primary UI text,
- have subtitle/caption support in the Message/Text subsystem,
- have font size scaling as a settings option.

Every template claiming `READY` must additionally:

- have colorblind palette options validated,
- have all audio-only cues matched with visual indicators,
- pass the accessibility audit in the Project Audit Command.

## VI-B — Audio Minimum Bar
Every template claiming `PARTIAL` or higher must:

- have audio event IDs validated at export (no unregistered IDs),
- have BGM layer spec documented,
- have missing-audio fallback behavior defined.

## VI-C — Input Minimum Bar
Every template claiming `PARTIAL` or higher must:

- have all input actions registered in the input action registry,
- have keyboard/controller default bindings defined,
- have no UI surfaces that require mouse input exclusively.

## VI-D — Localization Minimum Bar
Every template claiming `PARTIAL` or higher must:

- have all string keys validated at export,
- have primary locale fully populated,
- have localization audit passing for any secondary locales declared.

## VI-E — Schema Versioning Minimum Bar
Every schema used in a template must:

- carry a `schemaVersion` field,
- have a matching entry in `SCHEMA_CHANGELOG.md`,
- have a corresponding migration path from the previous version.

## VI-F — Performance Budget Minimum Bar
Every template claiming `READY` must:

- have a declared performance budget in `content/templates/budgets/`,
- pass CI performance budget runs without exceeding declared limits.

---

# Part VII — Recommended Execution Order

## Phase 0 — Governance Foundation
1. Release-Readiness Matrix
2. Template Readiness Matrix
3. Canonical Truth Reconciler
4. Template Claim Guardrails
5. Subsystem Badge Guardrails
6. **Schema Versioning and Changelog Governance** *(v2)*
7. **Breaking Change Detection in CI** *(v2)*

## Phase 1 — Operational Capability
1. Project Audit Command *(expanded with audio/input/localization/accessibility checks)*
2. Narrative Graph / Event Graph Linting *(expanded with voice line and tag validation)*
3. Asset Dependency / "Where Used" Browser
4. Content Coverage Analysis *(expanded with audio/input/localization checks)*
5. **Localization Completeness Checker** *(v2)*
6. **Accessibility Subsystem Governance** *(v2)*
7. **Input Remapping and Controller Governance** *(v2)*
8. **Audio Subsystem Governance** *(v2)*

## Phase 2 — Productization and Major Features
1. Runtime Record / Replay System
2. Template-Aware Onboarding
3. ARPG Template Productization
4. Tactics Scenario Authoring
5. Dungeon / Roguelite Packaging
6. Create-a-Character Runtime Module *(expanded with subsystems I–M)*
7. **Visual Regression Testing Suite** *(v2)*
8. **Performance Budget Profiling** *(v2)*
9. **Achievement and Trophy System** *(v2)*
10. **Platform-Specific Export Validation** *(v2)*

## Phase 3 — Experimental Template Specs and Extended Features *(new in v2)*
1. Monster Collector RPG Template Spec
2. Cozy / Life RPG Template Spec
3. Metroidvania-lite Template Spec
4. 2.5D RPG Template Spec
5. Mod and Player Extension Layer
6. Analytics and Opt-In Telemetry Layer
7. Future Investigation Template Cataloging (Deck-Builder, Survival, Rhythm, Social Sim, City Builder)

---

# Definition of Done for this Addendum

A feature/gap listed here is only considered closed when:

- code exists,
- docs exist,
- editor or CLI surface exists if promised,
- tests or validation anchors exist,
- public-facing readiness labels were updated in the same change,
- README, completion docs, and debt docs stay aligned,
- **schema version was bumped and a changelog entry was written** *(v2)*,
- **breaking changes (if any) were acknowledged in `BREAKING_CHANGES.md`** *(v2)*,
- **accessibility checklist was reviewed for any UI surfaces introduced** *(v2)*,
- **audio, input, and localization coverage checks pass in CI** *(v2)*.

---

# Short Conclusion

The biggest things URPG is most likely to forget are **not more flashy engine lanes**.

They are the systems that make the growing engine:

- truthful,
- governable,
- template-safe,
- and trustworthy for real project creation.

The most important missing layers from v1.0 were:

- release-readiness matrix,
- template readiness matrix,
- project audit command,
- truth reconciler,
- narrative linting,
- asset dependency graph,
- content coverage analysis,
- and the formal Create-a-Character runtime lane.

The most important missing layers **identified in v2.0** are:

- schema versioning and breaking-change governance,
- accessibility subsystem (with minimum bars per template tier),
- audio subsystem ownership,
- input remapping and controller governance,
- localization completeness checking,
- performance budget profiling,
- visual regression testing,
- achievement and trophy system,
- platform-specific export validation,
- formal specs for the four experimental template types listed in the catalog but never specced (Monster Collector, Cozy/Life, Metroidvania-lite, 2.5D),
- and the extended Create-a-Character subsystems (voice selection, affinity seeds, barbershop, companion/recruit creation, multi-protagonist support).

Without the v1.0 layers, URPG risks becoming powerful but fuzzy.

Without the v2.0 additions, URPG risks shipping templates that are internally consistent but not safe to put in front of real players.

With all of them, URPG becomes a credible, governable, accessible, and shippable multi-template engine product.
