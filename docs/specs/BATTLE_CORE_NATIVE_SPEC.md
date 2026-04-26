# Battle Core Native-First Spec

Date: 2026-04-14
Status: closure evidence and broader Wave 1 release proof recorded on 2026-04-20; Battle Wave 1 scope is complete
Scope: runtime ownership, editor ownership, schema, migration, diagnostics, and test anchors for native Battle Core absorption

## Last landed progress (2026-04-15)

- Native battle runtime ownership slices are active in:
  - `engine/core/battle/battle_core.*`
  - deterministic flow controller, action queue ordering, and rule resolver baselines
- Battle editor/preview slices are active in:
  - `editor/battle/battle_inspector_model.*`
  - `editor/battle/battle_preview_panel.*`
  - `editor/battle/battle_inspector_panel.*`
  - diagnostics workspace `battle` tab wiring
  - live scene diagnostics binding parity via `flowController()`, `nativeActionQueue()`, and `buildDiagnosticsPreview()` preview payload consumption
- Compat battle confidence anchors are active:
  - tactical routed battle fixture coverage across plugin reload
  - deterministic escape lifecycle parity in compat tests
- Validation anchors are active in:
  - `tests/unit/test_battle_core.cpp`
  - `tests/unit/test_battle_inspector_model.cpp`
  - `tests/unit/test_battle_preview_panel.cpp`
  - `tests/unit/test_battle_inspector_panel.cpp`
  - `tests/unit/test_battlemgr.cpp`
- **Native AI Battle Tactics Bridge landed (2026-04-16):**
  - `BattleKnowledgeBridge` (HP, MP, Weakness serialization for LLMs)
  - Real-time tactical advice pipeline via `ChatbotComponent`

## Closure evidence bundle

### Runtime owner files

- `engine/core/battle/battle_core.h`
- `engine/core/battle/battle_core.cpp`
- `engine/core/scene/battle_scene.h`
- `engine/core/scene/battle_scene.cpp`
- `engine/core/battle/battle_migration.h`

### Editor owner files

- `editor/battle/battle_inspector_model.h`
- `editor/battle/battle_inspector_model.cpp`
- `editor/battle/battle_preview_panel.h`
- `editor/battle/battle_preview_panel.cpp`
- `editor/battle/battle_inspector_panel.h`
- `editor/battle/battle_inspector_panel.cpp`
- `editor/diagnostics/diagnostics_workspace.cpp`

### Schema and migration files

- `content/schemas/battle_troops.schema.json`
- `content/schemas/battle_actions.schema.json`
- `engine/core/battle/battle_migration.h`

### Latest deterministic test outputs

- `.\build\Debug\urpg_tests.exe "[battle][scene][diagnostics],[battle][editor][panel],[editor][diagnostics][integration][battle_preview]" --reporter compact`
  - `67 assertions / 5 test cases passed`
- `.\build\Debug\urpg_tests.exe "[presentation][bridge],[presentation][runtime]" --reporter compact`
  - `40 assertions / 5 test cases passed`
- `.\build\Debug\urpg_tests.exe "[editor][diagnostics][wizard],[battle][migration]" --reporter compact`
  - `533 assertions / 41 test cases passed`
- `ctest --test-dir build -C Debug --output-on-failure -R "PresentationBridge derives battle frame from active BattleScene|PresentationBridge builds frame for active scene using runtime|BattleScene builds diagnostics preview from the next ordered queued action|Battle inspector panel binds live scene diagnostics preview payload|DiagnosticsWorkspace - Battle tab exports live scene diagnostics preview payload|BattleMigration:|MigrationWizardModel: battle migration warnings propagate from unsupported troop phase/page data|MigrationWizardModel: Batch Orchestration"`
  - `11/11 tests passed`

## Purpose

Battle Core becomes the native owner for battle flow, action sequencing, targeting, result handling, and battle-state hooks that currently appear across BattleManager compat tests and routed presentation anchors.

The live BattleScene path now routes phase progression, ordered action draining, and damage resolution through Battle Core instead of maintaining a parallel scene-local authority.

Battle diagnostics now also support a live-scene binding seam: the inspector/workspace can consume `flowController()`, `nativeActionQueue()`, and an optional `buildDiagnosticsPreview()` payload so preview/export state stays aligned with the runtime battle scene when a usable queued action exists, while the existing flow+queue-only binding remains valid.

This seam is now covered by focused native tests for scene preview construction, inspector binding, and diagnostics workspace export parity against a real `BattleScene`.

The presentation bridge now also projects battle presentation state from an active `BattleScene`, so battle HUD/presentation clients can consume native battle participants through the existing battle translator path instead of relying on manually seeded `PresentationContext::battleState`.

Battle migration now emits explicit warnings for unsupported troop page/phase scripts and unsupported action scope/effect payloads while still returning schema-shaped fallback native JSON, and those warning counts propagate through the migration wizard reporting path.

The subsystem should absorb the deterministic behavior already proven in compat tests while keeping HUD and overlay presentation as clients of battle state instead of turning battle logic into a plugin-command-driven surface.

## Source evidence

Primary evidence currently comes from:

- battle-flow reload routed fixture
- `BattleManager` setup, phase, action queue, and escape tests
- routed presentation and menu-presentation anchors that show battle HUD state as a presentation client
- character motion and battle HUD fixture surfaces used by compat routing

These anchors show that the product needs first-class ownership of:

- deterministic battle phase and result progression
- action queue and subject processing contracts
- targeting, damage, and healing rules
- hookable but bounded battle lifecycle extension points
- clean separation between battle logic and HUD/overlay presentation

## Ownership boundary

Battle Core owns:

- battle setup, phases, turn progression, and result state
- action queue, subject dispatch, and targeting resolution
- damage, healing, resource spend, and escape logic
- battle lifecycle hooks and deterministic extension points
- battle-state adapters consumed by HUD and presentation systems

Battle Core does not own:

- HUD and menu composition chrome
- save serialization policy
- narrative dialogue formatting
- project-wide event graph ownership outside battle callbacks

It consumes those systems through typed interfaces.

## Runtime model

### Core runtime objects

- `BattleFlowController`
  - authoritative phase and result progression for battle start, input, action, and termination states
- `BattleActionQueue`
  - deterministic queue of actions, subjects, target bindings, and resolution order
- `BattleRuleResolver`
  - applies targeting, damage, healing, status, and escape calculations
- `BattleHookRegistry`
  - bounded extension points for lifecycle hooks and project-specific rule injection
- `BattlePresentationBridge`
  - read-only adapter exposing battle state to HUD, motion, and overlay clients

### Runtime contracts

- battle phase progression is deterministic and testable
- hook execution is bounded and registry-owned, not hidden side effects
- presentation clients consume battle state but do not own battle outcomes
- escape and action-resolution rules remain reproducible for the same setup
- battle result and subject state remain serializable through typed contracts when required

## Editor surfaces

Battle Core should ship with these editor owners:

- `Battle Flow Inspector`
  - setup defaults, phase rules, result hooks, and turn policy
- `Action and Targeting Inspector`
  - action definitions, targeting rules, and resolution priorities
- `Battle Rules Panel`
  - damage, healing, escape, and hook policy configuration
- `Battle Preview Panel`
  - deterministic simulation preview for turn and action outcomes
- `HUD Integration Inspector`
  - binds battle-state outputs to HUD and overlay clients without moving ownership out of Battle Core

## Schema and data contracts

### Required schemas

- `battle_flows.json`
  - setup defaults, phase transitions, and result hooks
- `battle_actions.json`
  - action definitions, target rules, and execution metadata
- `battle_rules.json`
  - damage, healing, resource, and escape policy settings
- `battle_hooks.json`
  - extension-hook registry, project callbacks, and validation rules

### Schema rules

- battle action IDs and flow IDs are stable and migration-safe
- phase transitions are explicit typed records
- battle presentation bindings are adapters, not owners of battle logic
- imported compat metadata is preserved as mapping notes, not treated as native source of truth
- unsupported rule extensions emit diagnostics instead of silently mutating outcomes

## Migration and import rules

### Import goals

- translate compatible battle rule and HUD-driven behavior into native battle-flow and action records where possible
- preserve deterministic action and escape intent even when plugin command names are discarded
- separate battle-state ownership from HUD and motion presentation concerns during import

### Mapping strategy

- current `BattleManager` compat behavior maps into native battle-flow, rule, and action queue contracts
- `MOG_BattleHud_MZ` maps into presentation bindings under the HUD bridge rather than Battle Core ownership
- character motion surfaces map into presentation adapters that consume battle state but do not redefine it
- imported battle plugins should map core rules and hooks into native schemas, leaving only unsupported edge behavior behind compat shims

### Failure handling

- unsupported battle rule extensions are recorded as import diagnostics with preserved mapping notes
- conflicts between battle rule providers produce upgrade diagnostics instead of silent precedence
- projects can still preview canonical battle flow when custom presentation clients are disabled

## Diagnostics and safety

Battle Core diagnostics should include:

- invalid phase transitions
- unreachable or cyclic hook flows
- malformed action definitions or targeting rules
- inconsistent escape configuration
- presentation clients requesting missing battle-state bindings
- imported mappings that could not be normalized cleanly

Safe-mode expectations:

- preserve deterministic battle resolution without optional HUD/presentation clients
- fall back to minimal battle-state presentation when overlays fail
- surface hook or rule diagnostics without corrupting battle result state

## Extension points

Allowed extension points:

- lifecycle hook providers
- custom action-rule providers
- battle preview decorators
- HUD binding adapters

Disallowed extension patterns:

- presentation clients mutating authoritative battle state directly
- hidden rule mutations that bypass the hook registry
- plugin-owned phase transitions outside schema-defined flow

## Test anchors

The subsystem should inherit and later replace these evidence paths:

- `tests/unit/test_battlemgr.cpp`
  - setup and flow
  - escape gates and deterministic escape outcomes
  - hook registration
  - action queue lifecycle
  - damage and healing
- `tests/compat/test_compat_plugin_fixtures.cpp`
  - battle-flow reload routed anchor
  - presentation and menu-presentation reload anchors involving battle HUD state
- future native tests should add:
  - battle schema migration tests
  - deterministic flow simulation tests
  - hook validation diagnostics tests
  - HUD bridge contract tests against native battle presentation

## First implementation slice

Phase 1 of Battle Core absorption should deliver:

- native battle flow controller
- native action queue and rule resolver
- deterministic escape and result policies
- battle flow inspector and preview panel
- HUD bridge contracts for current routed battle presentation anchors

<!-- WAVE1_CHECKLIST_START -->
## Wave 1 Closure Checklist (Canonical)

_Managed by `tools/docs/sync-wave1-spec-checklist.ps1`. Do not edit manually._
_Canonical source: [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md](../archive/planning/WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)_

### Universal closure gates

- [ ] Runtime ownership is authoritative and compat behavior for this subsystem is bridge-only.
- [ ] Editor productization is complete (inspect/edit/preview/validate) with diagnostics surfaced.
- [ ] Schema contracts and migration/import paths are explicit, versioned, and test-backed.
- [ ] Deterministic validation exists (unit + integration + snapshot where layout/presentation applies).
- [ ] Failure-path diagnostics and safe-mode/bounded fallback behavior are explicitly documented and tested.
- [ ] Release evidence is published in status docs and gate snapshots are recorded.

### Battle Core specific closure gates

- [ ] Battle flow/action/rule ownership is authoritative; HUD and overlays consume battle state only.
- [ ] Deterministic turn/escape/action outcomes are preserved under runtime and preview validation anchors.
- [ ] Battle schema + migration + diagnostics closure is complete and release-evidence-backed.

### Closure sign-off artifact checklist

- [ ] Runtime owner files listed (header + source).
- [ ] Editor owner files listed.
- [ ] Schema and migration files listed.
- [ ] Latest deterministic test outputs recorded.
- [ ] README.md, docs/status/PROGRAM_COMPLETION_STATUS.md, and docs/archive/blueprints/URPG_Blueprint_v3_1_Integrated.md updated.
<!-- WAVE1_CHECKLIST_END -->

## Non-goals for this slice

- recreating every third-party battle plugin one-for-one
- letting HUD plugins define authoritative battle rules
- shipping cinematic battle presentation before the rule and phase model is authoritative
- collapsing battle-state ownership into menu or message systems

