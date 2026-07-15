# ADR-013: Native operation, revision, and atomicity contract

**Status:** Accepted

**Date:** 2026-07-15

**Decision owners:** Native runtime, editor document, project persistence, and recovery owners

## Context

Creator workflows increasingly span project documents, asset attachments, map objects, and runtime-facing data. A generic JSON transaction layer would create a second project authority and bypass existing domain validation, dirty state, history, recovery, and persistence owners. PFU-02 requires one shared safety contract while preserving domain ownership.

## Decision

Every durable creator operation is owned and applied by the authoritative native document or service for the data it changes. Shared orchestration coordinates those owners; it does not become a mutable document store.

### Identity and revisions

- Durable projects, documents, objects, attachments, and operations have stable, schema-defined IDs. IDs are never inferred from display names or filesystem ordering.
- Each owner exposes a monotonic source revision covering the state it validates and persists. A plan, preview, undo record, or apply request names the exact source revision it was derived from.
- Apply rejects a stale revision with a stable diagnostic and no partial mutation. The caller must refresh, re-plan, or explicitly resolve the conflict.
- Preview tokens are opaque, owner-issued, short-lived capabilities. They are valid only for their owner, source revision, requested operation, and the current editor session; they cannot be serialized as durable authority or reused after apply, cancel, reload, recovery, or project switch.

### Validation, preview, and apply

- Owners validate input before producing a preview and validate again immediately before durable apply.
- A preview is side-effect free outside its disposable owner-managed preview state. It must show intended changes, diagnostics, and affected stable IDs.
- The owner returns structured diagnostics with stable codes, severity, target identity, remediation, and enough context for editor focus. It never silently falls back to raw JSON, an external path, or an alternate document.
- Operations are idempotent by stable operation ID within the owner’s retained history: a repeated completed apply returns the original result rather than applying a second change. Reusing an operation ID with different payload or source revision is rejected.

### Persistence and recovery

- A single-document durable operation commits through that document’s existing atomic persistence path. Dirty state changes only after in-memory validation succeeds, and is cleared only after the owner confirms durable publication.
- A multi-file operation has a coordinator owned by the participating domain. It stages every output beside its target, records a recovery manifest before publication, validates the complete staged set, then atomically promotes the set using recoverable replace/rename operations. The coordinator either reports completion or leaves a recoverable manifest; it never reports success after a partial publish.
- Startup recovery detects incomplete manifests, validates staged/current revisions, and offers deterministic rollback or completion only when the recorded source revisions still match. Ambiguous, corrupt, or externally modified state is preserved and surfaced as a blocking diagnostic.
- Schema migrations run before owner validation and produce a versioned migration record. A migration is atomic with its document write, preserves stable IDs where possible, and fails closed when compatibility cannot be established.

### History, undo, and external edits

- Each owner writes undo/redo records for its own applied operations. Cross-domain orchestration records a composite operation whose inverse invokes owner-provided inverses in reverse order.
- An inverse is permitted only when its expected post-apply revision still matches. Otherwise undo reports a conflict and leaves durable data intact; recovery or an explicit user resolution path is required.
- Dirty-state registration and save/navigation prompts remain with the document owner. Coordinators aggregate visible state but do not clear, manufacture, or suppress an owner’s dirty flag.
- Before apply, save, recovery, or undo, owners compare the persisted source revision with their loaded revision. External changes invalidate previews and history entries that cannot be safely replayed, then present a reload/compare/recover diagnostic.

### Boundaries

- Native project documents and native runtime schemas remain authoritative. MZ JSON, `plugins.js`, and compatibility migration output are compat-boundary artifacts, not generic native transaction targets.
- Asset intake, transformations, attachments, map placement, project creation, and export continue using their existing native owners. New work must extend those owners rather than introducing a central JSON store, browser runtime, Node service, or raw filesystem mutation path.
- Operations may not grant arbitrary shell, filesystem, network, plugin, or provider authority. External tools produce reviewed, bounded artifacts that re-enter through their governed native owner.

## Consequences

PFU-02 work packets must name the authoritative owner, stable IDs, source revision, preview lifetime, atomic persistence boundary, recovery behavior, diagnostics, inverse behavior, and migration/external-edit policy. Tests must cover stale-source rejection, validation failure without mutation, successful apply, idempotent replay, inverse/undo, recovery of an interrupted multi-file commit, and external-edit conflict where relevant.

This decision intentionally adds coordination work for a cross-domain feature. That cost is preferable to an unowned transaction layer whose apparent atomicity cannot be reconciled with native history, recovery, or runtime consumers.

## Compliance evidence

- Existing document owners and their focused tests establish the baseline implementation patterns.
- Every new PFU-02-or-later work packet records its focused test command and manual recovery evidence in the required evidence record of the active creator-product plan.
- Deviations require a new ADR or an explicit amendment to this decision before implementation.
