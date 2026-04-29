# Feature Robustness Plan

Status date: 2026-04-29

URPG has enough native subsystems now that the next work should deepen existing surfaces instead of adding isolated demos. This plan tracks the feature-packed follow-through requested after the asset, AI, and plugin absorption passes.

## Priority Lanes

1. Battle feedback: chip damage/healing, configurable zero-damage presentation, custom buff caps, troop-position reuse, preview panel visibility, and migration/schema coverage.
2. State/message/picture: scoped/self/map/JS variable banks, nested text escapes, picture task bindings, and high-count picture management are started in native code; picture UI runtime preview remains.
3. Progression: level-up stat allocation pools, caps, class/actor rules, preview math, and editor snapshots are started in native code; save integration remains.
4. Asset browser/runtime library: tag/status/category filters, duplicate groups, source/license badges, previewable/runtime-ready counts, and used-by references are started in native code; promote/archive actions and richer thumbnail/waveform UI remain.
5. AI editor workflow: in-editor diff viewer, per-record approvals, apply history, undo stack visibility, change rationale, and validation results per proposed change.
6. Project knowledge indexing: project files, schemas, readiness reports, validation reports, asset catalogs, template specs, generated docs, and source summaries.
7. Concrete AI tools: subsystem validators/builders for event graphs, battle VFX, lighting/weather, ability sandboxing, export preview, asset promotion, and template instancing.
8. Export/release UX: export preview panel, platform checklist, missing asset report, packaging diagnostics, signing/notarization status, and playable smoke-test evidence.

## Implementation Rule

Each lane should land as native project data, schema coverage when data-driven, runtime behavior, editor state/preview, diagnostics, focused tests, and status documentation. Raw imported plugin or asset folders remain ignored intake evidence, not shipped runtime dependencies.
