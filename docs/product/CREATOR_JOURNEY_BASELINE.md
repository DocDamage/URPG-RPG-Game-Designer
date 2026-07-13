# Creator Journey Baseline (2026-07-13)

This is the M0 evidence baseline. It records current behavior rather than a
release claim. The deterministic smoke report is produced by the `creator
journey baseline` integration test and checked by
`tools/ci/check_creator_journey.ps1`.

| Step | Current status | Evidence / friction | Planned closure |
| --- | --- | --- | --- |
| Launch editor | Partial | The editor falls back to a creator shell without a valid project and validates a supplied/recent project through the session owner; graphical startup ergonomics still need review. | M1 |
| Create project | Partial | The guided wizard atomically materializes a runtime-valid starter project and opens it through the session flow; template choice and visual review remain limited. | M1, M5 |
| Discover external assets | Partial | The native Assets workspace loads a metadata-only, paged virtual catalog without copying payloads. Index setup, filters, and source selection remain incomplete. | M2 |
| Attach sprite | Partial | Raw external drag payloads are rejected for durable documents with Attach To Project remediation; the full resumable review-to-attachment workflow remains incomplete. | M3 |
| Paint map | Partial | Both release routes now deep-link into one Map workspace over the existing documents, but the creator layout and shared persistence are incomplete. | M4 |
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
