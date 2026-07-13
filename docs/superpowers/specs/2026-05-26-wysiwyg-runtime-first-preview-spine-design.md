# WYSIWYG Runtime-First Preview Spine Design

## Goal

Vastly improve URPG WYSIWYG by adding a shared runtime-first preview spine. The first complete slice proves that editor previews are driven by saved project data, runtime execution traces, diagnostics, and confidence reports for three existing WYSIWYG surfaces: dialogue/message preview, event-command graph preview, and export preview.

This slice improves the product foundation without claiming that every showcase route has a bespoke adapter yet. Existing route wiring, readiness gates, and template showcase coverage remain the release truth for broad WYSIWYG reach.

## Product Principle

The editor must not silently lie about what will ship. A WYSIWYG preview is trustworthy only when it can explain:

- which saved authoring document or project data produced the preview
- which runtime command trace was produced
- which diagnostics were checked
- whether the preview is runtime-backed or editor-only
- whether export/package evidence supports an exact-ship claim

## Architecture

Add a small shared WYSIWYG preview spine under the native core/editor boundary. The spine centers on a `WysiwygPreviewSession` value that panels and tests can inspect without depending on ImGui rendering.

The session carries:

- route id and surface id
- source document id or project data id
- preview mode
- runtime command trace rows
- state delta or preview summary rows
- diagnostics
- evidence bars for saved data, live preview, runtime execution, diagnostics, and tests
- confidence report flags for runtime-backed preview, clean diagnostics, dirty data, export awareness, and exact-ship readiness

Preview adapters stay small. They translate existing surface-specific saved documents into the shared session shape and reuse existing runtime preview/execution functions wherever those already exist.

## First Adapters

The first implementation slice covers three surfaces that already have meaningful runtime/editor contracts:

1. Dialogue/message preview
   - Source data: speaker, portrait, text, choices, choice commands, variable writes, and routing.
   - Runtime trace: page display, speaker/portrait/text rows, choice selection/confirmation, command hook rows, variable writes, and next-page routing.

2. Event-command graph preview
   - Source data: nodes, edges, sequence and conditional traversal, switch writes, variable writes, and command rows.
   - Runtime trace: node visits, edge decisions, command execution rows, and state writes.

3. Export preview
   - Source data: target, mode, runtime binary, output directory, expected artifacts, export manifest, and validation diagnostics.
   - Runtime trace: preflight rows, export rows, post-validation rows, missing expected artifacts, and exact-ship confidence.

## Data Flow

Each adapter follows the same flow:

1. Read the panel or fixture's saved document model.
2. Run the existing runtime preview or execution projection.
3. Convert the result into `WysiwygPreviewSession`.
4. Attach diagnostics and confidence flags.
5. Expose the session to tests and editor panel snapshots.

The invariant is:

```text
panel snapshot -> saved document -> runtime adapter -> command trace + diagnostics + confidence report
```

If an adapter cannot produce runtime commands, the session must say the preview is editor-only and must block runtime-backed or exact-ship confidence.

## Diagnostics And Confidence

Diagnostics must be actionable and tied to source ids when possible. The first slice covers:

- missing or empty source document ids
- duplicate ids where the source model supports ids
- missing pages, nodes, commands, choices, edges, or expected artifacts
- invalid references between authored records
- runtime preview unavailable
- export manifest or expected artifact mismatch

The confidence report is intentionally conservative:

- `runtimeBacked` is true only when runtime traces were produced from saved data.
- `diagnosticsClean` is true only when no blocker diagnostics exist.
- `savedDataPresent` is true only when the source model has a stable document or project data id.
- `exportAware` is true for export preview sessions and false for non-export surfaces.
- `exactShipReady` is true only for export preview sessions with runtime-backed preview, clean diagnostics, and satisfied expected artifacts.

## Editor Integration

The first slice does not redesign the full editor shell. Existing panels keep their current UI and models, but expose a preview-session snapshot where appropriate. This lets the product gain a shared proof language before broader visual polish.

Later slices can use the same session shape to power:

- common preview badges
- confidence panels
- route-level showcase editing
- AI-assisted change review
- unified undo/dirty/export state

## Tests

Use test-first implementation for each new behavior. Focused tests must prove:

- the shared session model records route, source, traces, diagnostics, evidence, and confidence
- dialogue/message saved data produces runtime-backed preview confidence
- broken dialogue/message data blocks confidence with actionable diagnostics
- event graph saved data produces runtime-backed preview confidence
- broken event graph data blocks confidence with actionable diagnostics
- export preview data produces export-aware exact-ship confidence when artifacts match
- broken export preview data blocks exact-ship confidence
- editor-facing panel snapshots can expose the session without ImGui rendering

The focused verification command for this slice is:

```powershell
ctest --preset dev-all -R "WYSIWYG|PreviewSession|Message|Event Authoring|Export preview|editor app panels|Editor panel registry" --output-on-failure
```

Do not run the broader local gate unless requested.

## Done Definition

This slice is complete when:

- shared preview-session and confidence-report types exist
- the three adapters emit sessions from real saved data and runtime traces
- diagnostics block confidence claims for broken real inputs
- editor/test snapshots can inspect the session
- focused tests pass
- docs describe this as the first runtime-first WYSIWYG spine slice, not a global bespoke-adapter claim for all showcase routes
