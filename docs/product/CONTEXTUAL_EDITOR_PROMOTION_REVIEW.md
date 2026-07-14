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
| `battle_preview`, `vendor`, `audio_mix`, `accessibility`, `input_remap` | Respective owners | Missing Map-context persistence/return evidence | Existing component evidence is not enough by itself | M8 Wave B/C integration pending | Remain Deferred |

The two promoted routes are nested, not top-level. Their Map dock preserves
the active selection and returns to the unified Map workspace. A missing,
empty, invalid, or unsaved contextual project emits a visible status/diagnostic
instead of falling back to ad-hoc JSON editing.
