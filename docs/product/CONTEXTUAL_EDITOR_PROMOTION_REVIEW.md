# Contextual Editor Promotion Review

This record applies the WYSIWYG done rule to each route promoted from
`Deferred` by the M8 creator-cohesion work. A route stays deferred unless it
has all listed evidence and a Map-context entry point.

| Route | Owner / Map entry | Saved contract and runtime consumer | Preview and diagnostics | Tests / evidence | Decision |
|---|---|---|---|---|---|
| `event_authoring` | `editor/events`, Map **Event** dock | `ContextualCreatorProject` event document; native event runtime | EventAuthoringPanel counts, dependency diagnostics, debugger | event authoring integration/unit coverage | Existing Nested route retained |
| `message_inspector` | `editor/dialogue`, Map **Dialogue** dock | `ContextualCreatorProject` dialogue graph; event message command references | DialogueGraphPanel route/choice/ending preview | dialogue graph and contextual-project round-trip tests | Existing Nested route retained |
| `character_creator` | `editor/character`, Map **Character** dock | `CharacterIdentity` in `ContextualCreatorProject`; deterministic character spawner/save contract | CharacterCreatorPanel appearance/runtime preview and validation snapshot | character creator panel/model plus contextual-project tests | Promoted to Nested on 2026-07-13 |
| `database` | `editor/database`, Map **Database** dock | `RpgDatabase` in `ContextualCreatorProject`; vertical-slice actor/item references | DatabasePanel count/validation snapshot | database panel suite plus contextual-project tests | Promoted to Nested for the governed actor/item slice on 2026-07-13 |
| `vendor` | `editor/shop`, Map **Vendor** dock | `VendorCatalog` in `ContextualCreatorProject`; inventory/item contract | VendorPanel visible-stock and reference diagnostics | vendor catalog plus contextual-project round-trip tests | Promoted to Nested on 2026-07-13 |
| `battle_preview` | `editor/battle`, Map **Battle** dock | Current Map encounter/event selection plus `BattleFlowController`; no duplicate battle model | BattlePreviewPanel deterministic damage, phase, escape, and victory-result preview | battle preview panel suite and contextual action/factory tests | Promoted to Nested on 2026-07-13 |
| `audio_mix` | `editor/audio`, Map **Audio** dock | `ContextualCreatorProject` persists the preset bank, selected preset, and active Map; native `AudioCore` consumes the selected mix | Mix selection applies to live native preview buses and reports disabled state | audio mix panel/preset, contextual-project round-trip, and contextual routing tests | Promoted to Nested on 2026-07-13 |
| `accessibility` | `editor/accessibility`, Map **Accessibility** dock | `ContextualCreatorProject` records reviewed Map/object scope and audit count; native `AccessibilityAuditor` regenerates current diagnostics | Audit issue count and first-issue focus action | accessibility panel/auditor, contextual-project round-trip, and contextual routing tests | Promoted to Nested on 2026-07-13 |
| `input_remap` | `editor/input`, Map **Input** dock | `ContextualCreatorProject` persists the validated input profile; runtime/editor input consumers reuse it | Binding validation and bound/empty preview state | input remap core, contextual-project round-trip, and contextual routing tests | Promoted to Nested on 2026-07-13 |
| `export_diagnostics` | `editor/export`, Map **Export Diagnostics** dock | Active project root supplies a bounded dev-bootstrap preflight configuration; the owned release/export workflow remains the only package emitter | Preflight readiness, existing-output validation, and first actionable issue | export diagnostics panel suite and contextual routing tests | Existing Nested route now has a Map-context dock |

The listed promoted routes are nested, not top-level. Their Map dock preserves
the active selection and returns to the unified Map workspace. A missing,
empty, invalid, or unsaved contextual project emits a visible status/diagnostic
instead of falling back to ad-hoc JSON editing.
