# Save / Data Core Native-First Spec

Date: 2026-04-14
Status: active implementation baseline (core slices landed; editor/schema/migration/release closure pending)
Scope: runtime ownership, editor ownership, schema, migration, diagnostics, and test anchors for native Save/Data Core absorption

## Last landed progress (2026-04-15)

- Save/runtime ownership slices are active in:
  - `engine/core/save/save_runtime.*` (runtime loader + recovery integration)
  - `engine/core/save/save_recovery.*` (tiered recovery behavior)
  - `engine/core/save/save_catalog.*` and session coordination slices
- Compat save/data bridge confidence anchors are active:
  - routed save-data failure projection through JSONL/report/panel paths
  - imported metadata normalization through `MigrationRunner`
  - runtime hydration after migrated metadata
- Save descriptor projection into inspector slot labels is active.
- **AI-Ready Save Infrastructure landed (2026-04-16):**
  - `AISyncCoordinator` (cloud-synced AI history serialization)
  - `CompressionLevel::Optimal` (incremental space-optimized history saving)
- validation anchors are active in:
  - `tests/unit/test_data_manager.cpp`
  - `tests/unit/test_save_runtime.cpp`
  - `tests/unit/test_save_recovery.cpp`
  - `tests/compat/test_compat_plugin_fixtures.cpp`
  - `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
  - `tests/unit/test_save_serialization.cpp`
  - `tests/unit/test_save_state_sync.cpp`

## Next steps

- Complete native save catalog + serializer ownership closure beyond seeded slices.
- Finalize schema contracts and importer/upgrader mapping for compat save metadata into native typed records.
- Ship editor productization surfaces:
  - save slot inspector
  - save policy panel
  - recovery diagnostics view
  - serialization schema panel
- Add native integration anchors for autosave policy, recovery escalation, and routed save-panel projection parity.

## Purpose

Save / Data Core becomes the native owner for slot metadata, autosave policy, recovery behavior, save integrity, and migration-safe serialization that currently appear across DataManager compat APIs, runtime recovery tests, and routed save-data fixtures.

The subsystem should absorb the behavior proven by routed save-data anchors and recovery tests without turning plugin header-extension conventions or opaque save metadata blobs into the long-term product model.

## Source evidence

Primary evidence currently comes from:

- save-data reload routed fixture
- save-data lifecycle failure projection through JSONL, report ingestion, and panel refresh
- imported save metadata migration coverage through `MigrationRunner`
- runtime save loader hydration after imported metadata migration
- `DataManager` save, load, autosave, and header-extension tests
- runtime save loader and recovery tests

These anchors show that the product needs first-class ownership of:

- deterministic slot catalog and autosave routing
- schema-owned save metadata and extension fields
- recovery tier selection and safe-mode boot contracts
- diagnostics for routed save-surface failures
- reload-safe save presentation and state persistence

## Ownership boundary

Save / Data Core owns:

- save slot catalog and slot metadata index
- autosave policy, slot selection rules, and recovery orchestration
- serialization boundaries for authoritative game-state persistence
- save integrity diagnostics and recovery tier selection
- editor-facing save policy, metadata, and diagnostics surfaces

Save / Data Core does not own:

- menu composition or save-scene layout chrome
- event script sequencing beyond save/load callbacks
- localization content storage
- battle logic or narrative flow ownership

It consumes those systems through typed interfaces.

## Runtime model

### Core runtime objects

- `SaveCatalog`
  - authoritative index of slots, autosave state, timestamps, headers, and validity markers
- `SaveSerializer`
  - schema-owned persistence layer for runtime state, metadata, and versioned migration boundaries
- `SaveRecoveryOrchestrator`
  - selects recovery tiers, boot-safe-mode behavior, and fallback loading policy
- `SaveMetadataRegistry`
  - typed metadata fields for slot presentation, extension keys, migration notes, and project policies
- `SaveSessionCoordinator`
  - mediates save/load requests, autosave triggers, and callbacks into runtime/editor systems

### Runtime contracts

- slot addressing and autosave policy are deterministic and schema-owned
- metadata fields are typed and migration-safe, not plugin-string-owned
- recovery tiers produce explicit diagnostics and safe-mode decisions
- save/load failures remain inspectable without corrupting active runtime state
- routed save surfaces may fail independently of authoritative persisted data

## Editor surfaces

Save / Data Core should ship with these editor owners:

- `Save Slot Inspector`
  - slot categories, labels, ordering, and presentation metadata
- `Save Policy Panel`
  - autosave rules, recovery policy, retention limits, and safe-mode defaults
- `Save Metadata Inspector`
  - typed metadata keys, extension-field migration notes, and preview values
- `Recovery Diagnostics View`
  - recovery tier history, corruption warnings, and boot-safe-mode reasons
- `Serialization Schema Panel`
  - version boundaries, migration fields, and compatibility validation

## Schema and data contracts

### Required schemas

- `save_policies.json`
  - autosave rules, retention policy, recovery preferences, and safe-mode defaults
- `save_slots.json`
  - slot IDs, categories, presentation metadata, and reserved routes
- `save_metadata_schema.json`
  - typed metadata fields, extension mappings, migration notes, and validation rules
- `save_migrations.json`
  - version upgrade rules, recovery hints, and compatibility transforms

### Schema rules

- slot IDs are stable and migration-safe
- autosave is an explicit policy, not a hidden special case
- metadata fields are typed and previewable without executing gameplay logic
- imported compat metadata is preserved as mapping notes, not treated as the native source of truth
- unsupported extension fields emit diagnostics instead of silently disappearing

## Migration and import rules

### Import goals

- translate compat save metadata, autosave behavior, and recovery signals into native save schema records
- preserve visible slot behavior and high-value metadata even when plugin-specific save commands are discarded
- separate authoritative persisted state from UI presentation metadata during import

### Mapping strategy

- current `DataManager` save/header APIs map into native slot catalog and metadata schema contracts
- routed save-data fixture routes map into native slot and autosave presentation descriptors under Save/Data ownership
- runtime save loader recovery tiers map into native recovery-orchestration contracts
- imported save plugins should map metadata, autosave affordances, and recovery flags into native save schema instead of opaque plugin config blocks

### Failure handling

- invalid or unsupported save metadata fields are recorded as import diagnostics with preserved raw mapping notes
- recovery fallbacks produce explicit upgrade diagnostics instead of hidden silent repair
- imported projects can boot in safe mode with readable save diagnostics even when full metadata normalization fails

## Diagnostics and safety

Save / Data Core diagnostics should include:

- missing or corrupt slot payloads
- invalid metadata fields or schema mismatches
- autosave policy conflicts
- recovery tier escalations and safe-mode triggers
- routed save-surface failures that leave authoritative slot data intact
- imported mappings that could not be normalized cleanly

Safe-mode expectations:

- preserve the ability to inspect recovery state and slot metadata
- fall back to safe skeleton boot when authoritative payload recovery fails
- surface diagnostics without deleting recoverable slot information

## Extension points

Allowed extension points:

- custom save metadata providers
- project-specific save policy hooks
- recovery annotation providers
- optional sync/export adapters

Disallowed extension patterns:

- ad hoc mutation of authoritative slot schema without registry updates
- plugin-owned hidden metadata fields that bypass validation
- save-surface UI assuming ownership of authoritative persisted state

## Test anchors

The subsystem should inherit and later replace these evidence paths:

- `tests/compat/test_compat_plugin_fixtures.cpp`
  - save-data reload routed anchor
- `tests/compat/test_compat_plugin_failure_diagnostics.cpp`
  - save-data lifecycle failure projection
- `tests/unit/test_data_manager.cpp`
  - save/load, autosave, and header-extension coverage
- `tests/unit/test_migration_runner.cpp`
  - imported save metadata shape normalization into URPG runtime metadata contracts
- `tests/unit/test_save_runtime.cpp`
  - runtime loader, migrated metadata hydration, and recovery-tier contracts
- `tests/unit/test_save_recovery.cpp`
  - recovery-path and malformed-save behavior
- future native tests should add:
  - save schema migration tests
  - recovery diagnostics snapshot tests
  - autosave policy validation tests
  - routed save-surface failure projection tests against native save panels

## First implementation slice

Phase 1 of Save / Data Core absorption should deliver:

- native save catalog and metadata registry
- native autosave policy model
- recovery orchestrator with safe-mode signaling
- editor save-slot inspector and recovery diagnostics view
- import mapping for current DataManager and routed save-data anchor behavior

<!-- WAVE1_CHECKLIST_START -->
## Wave 1 Closure Checklist (Canonical)

_Managed by `tools/docs/sync-wave1-spec-checklist.ps1`. Do not edit manually._
_Canonical source: [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md](WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)_

### Universal closure gates

- [ ] Runtime ownership is authoritative and compat behavior for this subsystem is bridge-only.
- [ ] Editor productization is complete (inspect/edit/preview/validate) with diagnostics surfaced.
- [ ] Schema contracts and migration/import paths are explicit, versioned, and test-backed.
- [ ] Deterministic validation exists (unit + integration + snapshot where layout/presentation applies).
- [ ] Failure-path diagnostics and safe-mode/bounded fallback behavior are explicitly documented and tested.
- [ ] Release evidence is published in status docs and gate snapshots are recorded.

### Save / Data Core specific closure gates

- [ ] Save catalog/serializer/recovery ownership is authoritative and separate from UI presentation concerns.
- [ ] Autosave, recovery tier escalation, and safe-mode behavior are policy-owned and diagnostics-backed.
- [ ] Compat metadata/import upgrade mappings are typed, versioned, and test-backed.

### Closure sign-off artifact checklist

- [ ] Runtime owner files listed (header + source).
- [ ] Editor owner files listed.
- [ ] Schema and migration files listed.
- [ ] Latest deterministic test outputs recorded.
- [ ] README.md, docs/PROGRAM_COMPLETION_STATUS.md, and URPG_Blueprint_v3_1_Integrated.md updated.
<!-- WAVE1_CHECKLIST_END -->

## Non-goals for this slice

- recreating every RPG Maker save plugin one-for-one
- coupling save presentation layout to authoritative serialization logic
- making raw plugin header extensions the long-term authoring model
- shipping cloud sync before the local schema and recovery contracts are authoritative
