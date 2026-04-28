# Save/Data Core — Wave 1 Closure Sign-off

> **Status:** `PARTIAL`
> **Purpose:** Evidence-gathering artifact for Wave 1 closure review.
> **Date:** 2026-04-20
> **Rule:** Release-owner review approved the bounded Wave 1 save/data scope on 2026-04-28. This document does not grant public release approval or imply scope beyond the evidence below.

---

## 1. Runtime Ownership

Save/Data Core provides atomic persistence, tiered recovery, and catalog management:

| Component | Responsibility | Evidence |
|-----------|---------------|----------|
| `SaveCatalog` | In-memory index of save slot entries with recovery tier tracking and sequence numbers | `engine/core/save/save_catalog.h` |
| `SaveJournal` | Atomic write-through (`WriteAtomically`) with fsync semantics to prevent torn writes | `engine/core/save/save_journal.h` |
| `RuntimeSaveLoader` (`SaveRuntime`) | Synchronous load/save API with three-tier recovery fallback | `engine/core/save/save_runtime.h` |
| `SaveRecoveryManager` (`SaveRecovery`) | Level 1 (autosave) → Level 2 (metadata + variables) → Level 3 (safe skeleton) recovery | `engine/core/save/save_recovery.h` |

---

## 2. Editor Surfaces

Two editor panels provide save inspection and policy editing:

| Surface | Key Capability | Header |
|---------|---------------|--------|
| `SaveInspectorModel` | Loads from catalog + coordinator; produces summary rows, recovery diagnostics, schema summary, and policy draft | `editor/save/save_inspector_model.h` |
| `SaveInspectorPanel` | Binds runtime catalog/coordinator; renders inspector UI and applies policy changes | `editor/save/save_inspector_panel.h` |

---

## 3. Schema Contracts

Native JSON schemas are enforced under `content/schemas/`:

- **`save_metadata.schema.json`** — Typed contract for slot summaries, flags, party snapshots, and custom metadata.
- **`save_migrations.schema.json`** — Versioned migration contract with `from`/`to` ranges and operation arrays.
- **`save_policies.schema.json`** — Contract for autosave settings, retention limits, and pruning behavior.
- **`save_slots.schema.json`** — Contract for stable slot descriptors, categories, and reservation flags.

---

## 4. Migration Mapping

Two migration paths are available:

- **`SaveMigration`** (`engine/core/save/save_migration.h`) — `UpgradeCompatSaveMetadataDocument()` upgrades legacy compat save metadata to native format, and `ImportCompatSaveDocument()` imports full compat save payloads into native runtime-owned save JSON plus migrated metadata, with severity-graded diagnostics and safe-fallback support.
- **`MigrationRunner`** (`engine/core/migrate/migration_runner.h`) — Generic JSON Patch-like runner (`rename`, `move`, etc.) used for save payload upgrades. Battle metadata and other subsystem blobs are preserved during unrelated ops.
- **Save Policy Governance**: Canonical save policy fixture at `content/fixtures/save_policies.json` validates against `save_policies.schema.json` via `tools/ci/check_save_policy_governance.ps1`, enforced in Gate 1 CI.

---

## 5. Diagnostics Integration

- **Recovery diagnostics**: `SaveInspectorModel` computes `SaveRecoveryDiagnosticsSummary` (autosave recovery, metadata+variables recovery, safe-mode slots, corrupted slots) from live catalog entries.
- **Policy editing**: `SaveInspectorPanel` exposes live policy draft editing with validation (`SavePolicyValidationSummary`) before applying to the runtime coordinator.
- **Schema contract validation**: `test_save_schema_contracts.cpp` enforces that all save schemas parse and validate against canonical fixtures.

---

## 6. Test Coverage

| Layer | Count | Sources |
|-------|-------|---------|
| Unit | 10 | `test_save_catalog`, `test_save_journal`, `test_save_runtime`, `test_save_recovery`, `test_save_migration`, `test_save_serialization`, `test_save_state_sync`, `test_save_schema_contracts`, `test_save_inspector_model`, `test_save_inspector_panel` |
| Integration | 2 + closure suite | `test_integration_runtime_recovery`, `test_battle_save_integration`, `test_wave1_closure_integration` |

Key cross-subsystem assertions include:
- Migration output can be loaded through the runtime recovery path.
- Missing primary saves fall back to Level 2 metadata + variables.
- Battle metadata is not dropped during save payload migration.
- Real RPG Maker `.rpgsave` payloads can be read, imported into native payload + metadata, and loaded through the normal runtime path.
- Menu and message runtime states survive save/load boundaries.

---

## 7. Remaining Residual Gaps (Honest Scope Limits)

1. **Compat import/migration completion**: `SaveMigration`, `RPGMakerSaveFileReader`, and `MigrationRunner` now cover end-to-end compat save import through native payload + metadata generation, including a focused `.rpgsave` round-trip. Remaining work is limited to deeper plugin-specific save semantics and any future compat payload fields that still require explicit fallback handling. This specific compat import/migration item is complete; only unrelated residual gaps remain in `readiness_status.json`.
2. **Automated CI policy gates**: ~~Save policy validation runs in-editor and in-unit tests, but an automated CI gate that rejects builds on policy drift is not yet wired.~~ **RESOLVED**: `tools/ci/check_save_policy_governance.ps1` now validates the canonical fixture against the schema and enforces retention/autosave invariants in Gate 1 CI.
3. **Cloud / cross-device sync**: Not in Wave 1 scope.

---

*Sign-off prepared by governance agent. Promotion to `READY` requires human review of the residual gaps above.*
