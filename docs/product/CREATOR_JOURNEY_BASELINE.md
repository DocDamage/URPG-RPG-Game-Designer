# Creator Journey Baseline (2026-07-13)

This is the rolling M0 evidence checkpoint. It records the current integrated
behavior rather than a release claim. The deterministic smoke report is
produced by the `creator journey baseline` integration test and checked by
`tools/ci/check_creator_journey.ps1`. The fixture retains the original
baseline classifications so historical reports remain comparable; this table
describes the newer implementation layered on top of that baseline.

| Step | Current status | Evidence / friction | Planned closure |
| --- | --- | --- | --- |
| Launch editor | Implemented; graphical review open | The editor falls back to a persistent creator shell without a valid project, validates supplied/recent projects through one session owner, and preserves the last valid project on open failure. | M10 graphical review |
| Create project | Implemented; playtest closure open | The nested guided wizard atomically materializes a runtime-valid starter project, opens it through the session flow, and highlights the next Map action. Immediate editor-owned playtest remains M6. | M6, M10 |
| Discover external assets | Implemented; live-library qualification open | The native Assets workspace loads metadata-only sharded catalogs, paged search/filter/sort state, refresh/source actions, and a tested 100,000-row fixture without copying payloads. | M10 performance on reference machine |
| Attach sprite | Implemented for governed workflow; consumer breadth open | External assets can be reviewed, licensed/classified, converted or sliced, promoted, attached, and represented by typed drag payloads. Raw external durable drops are rejected. Map Tiles and Props consume attached drops; other contextual consumers remain open. | M3.5, M8 |
| Paint map | Implemented; manual equivalence open | Both release routes deep-link into one Map workspace with shared project/map context, selection, history routing, diagnostics, readiness guidance, persisted layout, and rollback-capable paired document save. | M4 manual review, M10 |
| Add event | Deferred | Event authoring is not entered from a selected map object as a complete creator flow. | M8 |
| Choose spawn | Partial | Starter projects and Level Builder expose spawn contracts, but the current-map playtest target/session flow remains M6. | M6 |
| Playtest | Deferred | No editor-owned current-map playtest session or unsaved overlay exists. | M6 |
| Return to editor | Deferred | There is no playtest-session return path that restores map context. | M6 |
| Save project | Implemented for the Grid Parts-triggered Map save surface; dirty breadth open | The shared dirty-state registry provides Save, Save All, navigation decisions, failure focus, and atomic paired Map publication. Its app-level dirty trigger currently follows Grid Parts; Perspective 2D-only changes and durable non-Map editors still need shared aggregation/registration. | M1.4 breadth, M8 |
| Validate project | Implemented for current Map/project seams | The unified Map surface aggregates validation and package-readiness diagnostics with focus/next-action guidance. Full creator-slice validation remains dependent on M8-M9 content. | M8, M9 |
| Package project | Partial | Export tooling exists; it does not yet package the exact creator-reviewed project through the journey shell. | M9 |

The current checkpoint still does not satisfy the complete creator-product
promise because Add Event, editor-owned Playtest/Return, recovery, contextual
deep authoring, and the governed vertical slice are open. The fixture remains
the source of truth for stable step IDs, timing budgets, and original deferred
reasons. No M10 manual graphical review has been recorded; headless and model
tests must not be presented as that review.
