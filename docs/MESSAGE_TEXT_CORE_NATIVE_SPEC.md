# Message / Text Core Native-First Spec

Date: 2026-04-14
Status: closure-complete as of 2026-04-19 (runtime renderer handoff, editor productization, schema/migration fields, and diagnostics integration all landed and validated)
Scope: runtime ownership, editor ownership, schema, migration, diagnostics, and test anchors for native Message/Text Core absorption

## Last landed progress (2026-04-15)

- Native message/text runtime ownership slices are active in:
  - `engine/core/message/message_core.*` (`MessageFlowRunner`, `RichTextLayoutEngine`, presentation variants, choice state)
- Compat renderer bridge slices are active in:
  - `runtimes/compat_js/window_compat.*`
  - `Window_Base.drawText` emits backend-facing `RenderLayer::TextCommand` payloads
  - `Window_Message.drawMessageBody` supports deterministic dialogue-body alignment parity (`left`/`center`/`right`)
- Deterministic wrapped centered/right text placement snapshots are active in:
  - `tests/unit/test_window_compat.cpp` (`[compat][window][snapshot]`)
- Message editor and schema/migration slices are active in:
  - `editor/message/message_inspector_*`
  - `content/schemas/message_styles.schema.json` and related message schemas
- `engine/core/message/message_migration.*`
- **Native AI Chatbot Infrastructure landed (2026-04-16):**
  - `ChatbotComponent` (`IChatService`-backed streaming and tool-calling, with deterministic mock and bounded OpenAI-compatible in-tree providers)
  - `WorldKnowledgeBridge` (narrative and world state context serialization)
  - `PersonalityRegistry` (NPC behavioral prompt templates)
  - `ChatWindow` (native word-wrapped streaming chat UI)
## Next steps

- ~~Complete native MessageScene renderer ownership handoff~~ (DONE 2026-04-19): `MapScene::onUpdate()` now submits `TextCommand` and `RectCommand` to `RenderLayer`; `OpenGLRenderer::processCommands()` handles `RectCommand`.
- ~~Complete backend text-command consumption across renderer tiers~~ (DONE 2026-04-19): TIER_BASIC placeholder logging covers both `TextCommand` and `RectCommand`.
- ~~Finalize editor/schema/migration productization~~ (DONE 2026-04-19): `MessageInspectorModel` supports editing, `DiagnosticsWorkspace` exposes mutation/export round-trips, migration maps `defaultChoiceIndex`/`command`/window/audio style fields, schema updated.
- Continue narrowing compat message behavior to import/verification bridge-only ownership.

## Purpose

Message / Text Core becomes the native owner for dialogue flow, rich text layout, portraits/faces, namebox chrome, and message presentation timing that currently appear as compat Window surfaces and routed fixture evidence.

The subsystem should absorb the behavior proven by routed message-text fixtures and Window parity tests without turning plugin escape sheets or ad hoc text commands into the long-term authoring model.

## Source evidence

Primary evidence currently comes from:

- message-text reload routed fixture
- `Window_Base.drawTextEx` escape handling
- `Window_Base.textWidth` and `Window_Base.textSize`
- `Window_Base.drawActorFace`
- `Window_Base.drawText` renderer command bridge (`RenderLayer::TextCommand`)
- `Window_Message.drawMessageBody` centered/right alignment parity behavior

These anchors show that the product needs first-class ownership of:

- dialogue-mode routing between speaker, narration, and system presentations
- deterministic rich-text token layout
- portrait/face placement and clipping
- namebox and message chrome rules
- reload-safe message presentation state
- backend text command emission with deterministic placement semantics

## Ownership boundary

Message / Text Core owns:

- dialogue line flow and page progression
- rich-text escape parsing and layout tokens
- portraits, faces, and namebox presentation rules
- message presentation variants such as narration, speaker, and system notices
- choice prompt presentation and message-adjacent interaction chrome
- preview-safe reload of message layouts, localized text, and presentation settings

Message / Text Core does not own:

- event graph sequencing outside message waits and callbacks
- localization asset storage format
- save serialization internals
- battle logic or menu scene ownership

It consumes those systems through typed interfaces.

## Runtime model

### Core runtime objects

- `MessageFlowRunner`
  - authoritative runner for pages, waits, advance rules, pauses, and callbacks into event runtime
- `RichTextLayoutEngine`
  - tokenizes escape codes, measures lines, resolves icons/variables/currency, and produces layout boxes
- `DialoguePresentationModel`
  - data-only description of chrome, namebox, portrait placement, and message variant styling
- `PortraitBindingRegistry`
  - resolves actor face, portrait source, fallback art, and variant swaps for message presentation
- `ChoicePromptState`
  - owns active choices, selection state, disabled-state reasons, and navigation contracts while a message session is open

### Runtime contracts

- message flow state is deterministic and serializable when the event/runtime layer requests it
- escape processing is schema-owned and testable, not plugin-command-string-owned
- text measurement is stable for preview and runtime under the same layout rules
- narration, speaker, and system routes are explicit presentation modes rather than hidden plugin flags
- portrait and face layout rules remain independent of menu or battle ownership

## Editor surfaces

Message / Text Core should ship with these editor owners:

- `Dialogue Inspector`
  - author lines, speaker bindings, pauses, waits, and branching hooks
- `Rich Text Preview`
  - live preview for escape codes, icons, variables, currency, and multiline wrapping
- `Portrait and Namebox Layout Inspector`
  - face source, portrait placement, namebox style, margins, and safe-area preview
- `Choice Prompt Editor`
  - choice labels, enable rules, default selection, and preview flow
- `Localization Validation Panel`
  - overflow checks, missing-token checks, and locale-sensitive layout warnings

## Schema and data contracts

### Required schemas

- `message_styles.json`
  - chrome variants, namebox rules, portrait docking, spacing, and safe-area settings
- `dialogue_sequences.json`
  - dialogue pages, speaker bindings, timing rules, waits, and callbacks into event runtime
- `rich_text_tokens.json`
  - supported escape families, icon mappings, variable bindings, and fallback handling
- `choice_prompts.json`
  - prompt definitions, options, enable rules, default selections, and continuation hooks

### Schema rules

- dialogue sequence IDs are stable and migration-safe
- presentation mode is explicit and typed
- layout rules are previewable without running gameplay logic
- imported plugin metadata is preserved as mapping notes, not treated as the native source of truth
- unsupported escape families fail with diagnostics instead of silently changing meaning

## Migration and import rules

### Import goals

- translate message plugin behaviors into native dialogue and layout records where possible
- preserve escape intent, portraits/faces, and timing semantics even when plugin command names are discarded
- separate text layout concerns from event sequencing concerns during import

### Mapping strategy

- current compat `Window_Base` parity maps into native rich-text token and text-layout contracts
- routed message-text fixture modes map into native presentation variants for speaker, narration, and system dialogue
- imported message plugins should map namebox, portrait, and timing rules into native message-style schema instead of opaque parameter blocks
- unsupported custom control flows remain behind compat shims only until a native message-flow contract exists

### Failure handling

- unsupported escape codes are recorded as import diagnostics with preserved raw token text
- portrait/namebox conflicts produce upgrade diagnostics instead of hidden precedence rules
- imported dialogue content can remain readable in safe mode even when custom chrome is dropped

## Diagnostics and safety

Message / Text Core diagnostics should include:

- unsupported or malformed escape tokens
- overflow and truncation risk for localized variants
- missing portrait or face bindings
- invalid namebox layout references
- choice prompts with unreachable continuation targets
- imported mappings that could not be normalized cleanly

Safe-mode expectations:

- preserve readable text output even if advanced chrome is disabled
- fall back from portrait layouts to text-only presentation when assets or layout contracts are invalid
- surface import warnings without blocking basic narrative progression

## Extension points

Allowed extension points:

- custom rich-text token providers
- portrait source providers
- message-style skins
- choice prompt decorators

Disallowed extension patterns:

- direct mutation of authoritative message flow state outside the runner
- hidden token semantics that bypass schema registration
- plugin-owned layout mutation that changes runtime meaning without diagnostics

## Test anchors

The subsystem should inherit and later replace these evidence paths:

- `tests/compat/test_compat_plugin_fixtures.cpp`
  - message-text reload routed anchor
- `tests/unit/test_window_compat.cpp`
  - `drawTextEx`
  - `textWidth`
  - `textSize`
  - `drawActorFace`
  - `drawText` backend command emission
  - `Window_Message` centered/right alignment parity
  - wrapped centered/right `drawTextEx` snapshot-style placement assertions
- future native tests should add:
  - dialogue schema migration tests
  - localized overflow snapshot tests
  - choice prompt state tests
  - message reload-safe state preservation tests
  - import upgrade tests from compat message mappings into native dialogue schema

## First implementation slice

Phase 1 of Message / Text Core absorption should deliver:

- [x] native rich-text layout engine
- [x] native dialogue presentation variants for speaker, narration, and system text
- [x] portrait/namebox layout model
- [x] dialogue inspector and preview panel
- [x] import mapping for current Window parity and routed message-text anchor behavior
- [x] compat renderer bridge from `Window_Base.drawText` to backend-facing text commands
- [x] compat `Window_Message` parity slice for centered/right dialogue body alignment
- [x] wrapped centered/right multiline placement snapshot coverage in WindowCompat tests
- [x] native MessageScene UI renderer ownership replacing compat-window bridge as the authoritative runtime path
- [x] renderer-tier implementation closure for backend text command consumption where still placeholder

<!-- WAVE1_CHECKLIST_START -->
## Wave 1 Closure Checklist (Canonical)

_Managed by `tools/docs/sync-wave1-spec-checklist.ps1`. Do not edit manually._
_Canonical source: [WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md](WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md)_

### Universal closure gates

- [ ] Runtime ownership is authoritative and compat behavior for this subsystem is bridge-only.
- [ ] Editor productization is complete (inspect/edit/preview/validate) with diagnostics surfaced.
- [ ] Schema contracts and migration/import paths are explicit, versioned, and test-backed.
- [ ] Deterministic validation exists (unit + integration + snapshot where layout/presentation applies).
- [ ] Failure-path diagnostics and safe-mode/bounded fallback behavior are explicitly documented and tested.
- [ ] Release evidence is published in status docs and gate snapshots are recorded.

### Message / Text Core specific closure gates

- [ ] MessageScene-native renderer ownership is authoritative (compat window bridge no longer primary).
- [ ] Text layout + alignment behavior is deterministic across runtime and preview, including wrapped snapshot anchors.
- [ ] Escape/token/schema migration and diagnostics remain explicit, typed, and test-backed.

### Closure sign-off artifact checklist

- [ ] Runtime owner files listed (header + source).
- [ ] Editor owner files listed.
- [ ] Schema and migration files listed.
- [ ] Latest deterministic test outputs recorded.
- [ ] README.md, docs/PROGRAM_COMPLETION_STATUS.md, and docs/archive/blueprints/URPG_Blueprint_v3_1_Integrated.md updated.
<!-- WAVE1_CHECKLIST_END -->

## Non-goals for this slice

- recreating every commercial message plugin one-for-one
- folding event graph ownership into the message renderer
- making escape strings the primary authoring UX forever
- shipping advanced cinematic text effects before the layout and flow model is authoritative
