# Creator Journey Baseline (2026-07-13)

This is the M0 evidence baseline. It records current behavior rather than a
release claim. The deterministic smoke report is produced by the `creator
journey baseline` integration test and checked by
`tools/ci/check_creator_journey.ps1`.

| Step | Current status | Evidence / friction | Planned closure |
| --- | --- | --- | --- |
| Launch editor | Partial | The native headless editor shell can start, but the app still requires a valid CLI project root and has no startup project-session flow. | M1 |
| Create project | Partial | `ProjectTemplateGenerator` creates a valid in-memory project contract; the wizard does not yet atomically materialize and open a project directory. | M1, M5 |
| Discover external assets | Deferred | Asset indexing tooling exists, but there is no native virtual-catalog query workflow. | M2 |
| Attach sprite | Partial | Governed promotion/attachment services exist, but they are not one resumable creator workflow. | M3 |
| Paint map | Partial | The grid-part document can place parts, while level-builder and spatial routes remain separate editor destinations. | M4 |
| Add event | Deferred | Event authoring is not entered from a selected map object as a complete creator flow. | M8 |
| Choose spawn | Partial | Template maps include a spawn contract, but there is no shared Map workspace spawn selection flow. | M4, M6 |
| Playtest | Deferred | No editor-owned current-map playtest session or unsaved overlay exists. | M6 |
| Return to editor | Deferred | There is no playtest-session return path that restores map context. | M6 |
| Save project | Partial | Individual authoring services persist data, but there is no shared Save All and dirty-state registry. | M1 |
| Validate project | Partial | Project and export validation seams exist, but they are not unified as focused Map/package diagnostics. | M4 |
| Package project | Partial | Export tooling exists; it does not yet package the exact creator-reviewed project through the journey shell. | M9 |

The baseline does not satisfy the creator-product promise. The fixture is the
source of truth for baseline step IDs, timing budgets, and explicit deferred
reasons. No manual graphical review has been recorded yet; that review is
required at M10 and must not be inferred from headless output.
