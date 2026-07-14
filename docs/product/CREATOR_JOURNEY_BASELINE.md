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
| Create project | Implemented; graphical review open | The nested guided wizard atomically materializes a runtime-valid starter project, opens it through the session flow, and highlights the next Map action. The bounded current-map playtest contract is covered separately by M6 tests. | M10, M11 |
| Discover external assets | Implemented; live-library qualification open | The native Assets workspace loads metadata-only sharded catalogs, paged search/filter/sort state, refresh/source actions, and a tested 100,000-row fixture without copying payloads. | M10 performance on reference machine |
| Attach sprite | Implemented for governed workflow; consumer breadth open | External assets can be reviewed, licensed/classified, converted or sliced, promoted, attached, and represented by typed drag payloads. Raw external durable drops are rejected. Map Tiles and Props consume attached drops; other contextual consumers remain open. | M3.5, M8 |
| Paint map | Implemented; manual equivalence open | Both release routes deep-link into one Map workspace with shared project/map context, selection, history routing, diagnostics, readiness guidance, persisted layout, and rollback-capable paired document save. | M4 manual review, M10 |
| Add event | Implemented bounded contextual contract; visual walkthrough open | The Map context route opens event authoring with a stable selection/return payload; the vertical-slice fixture and contextual tests cover saved data, runtime execution, diagnostics, and return routing. | M10, M11 |
| Choose spawn | Implemented bounded contract | Starter projects and Level Builder expose spawn contracts, and the playtest session resolves player start, selected object/tile, entrance, and checkpoint targets. | M10, M11 |
| Playtest | Implemented bounded contract; target-build walkthrough open | The editor owns launch, ignored unsaved overlays, structured diagnostics, hot-reload decisions, stop/restart, and visible session state. | M10, M11 |
| Return to editor | Implemented bounded contract; target-build walkthrough open | Return restores the session’s Map context and preserves unsaved editor state; recovery and failed-session paths are covered by focused tests. | M10, M11 |
| Save project | Implemented for the Grid Parts-triggered Map save surface; dirty breadth open | The shared dirty-state registry provides Save, Save All, navigation decisions, failure focus, and atomic paired Map publication. Its app-level dirty trigger currently follows Grid Parts; Perspective 2D-only changes and durable non-Map editors still need shared aggregation/registration. | M1.4 breadth, M8 |
| Validate project | Implemented bounded creator-slice contract | The unified Map surface aggregates validation and package-readiness diagnostics with focus/next-action guidance; the governed vertical-slice gate covers its required seams. | M10, M11 |
| Package project | Implemented deterministic fixture contract; release qualification open | The governed vertical slice produces a deterministic package inventory and rejects raw, external, local-state, recovery, and playtest-overlay inputs. Exact target-release packaging still requires M11 evidence. | M10, M11 |

The current checkpoint still does not satisfy the complete creator-product
release promise because the bounded M6-M9 contracts need fresh graphical,
keyboard/accessibility, reference-machine, package, and external-approval
evidence for the target build. The fixture remains the source of truth for
stable step IDs, timing budgets, and original deferred reasons. No M10 manual
graphical review has been recorded; headless and model tests must not be
presented as that review.
