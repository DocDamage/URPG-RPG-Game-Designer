# PFU-04 Work Packet: Native Dialogue Graph Runtime Handoff

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Execute a saved native Dialogue Graph through the existing `MapScene` and
`MessageFlowRunner`, rather than treating its ImGui preview as runtime proof.

## Contract

- The Map Dialogue Authoring control enables the runtime action only after the
  current graph is saved. It passes that graph to the bound native `MapScene`
  with a stable project-local conversation ID.
- `MapScene` validates structural and flow diagnostics before replacing the
  current runtime dialogue. The current runtime dialogue remains intact if
  admission fails.
- Valid graph nodes become native message nodes with their authored preview
  text, speaker metadata, and choice targets. The existing message runtime
  renders choice labels, selected state, and keyboard selection, then follows
  the selected native graph target.
- When the runtime owner receives a valid native `LocaleCatalog`, node and
  choice localization keys resolve through that catalog. Missing selected keys
  retain their authored preview/label fallback and emit an observable runtime
  diagnostic; no locale bundle is changed.
- An authored node caption key resolves through that same selected catalog and
  renders above the native message box, with the resolved node body as its
  fallback. An authored voice asset ID dispatches through the injected native
  `AudioCore` as an SE source when the node begins. Missing audio-core binding
  or rejected playback records a diagnostic without rejecting the dialogue.
- Choice conditions evaluate against the existing native `GlobalStateHub`
  integer-compatible values. Unmet or unsupported conditions render disabled
  choices rather than silently changing the branch.
- Choice effects add their declared integer delta through that same native
  state authority after the selected choice is confirmed, saturating at native
  `int32_t` limits. Missing effect keys fail admission before the active
  runtime dialogue changes.
- MapScene save slots now retain a versioned checkpoint for an active
  project-saved dialogue graph: stable project dialogue ID, conversation ID,
  and active node ID. Load restores global state first, then reopens that node
  only after the current project graph passes native loading and admission.
  Invalid or stale checkpoints leave normal global-state load successful but
  report a bounded restore diagnostic and no dialogue remains active.

## Limits

This first execution slice selects a valid project `localization.default_locale`
bundle automatically when the MapScene binds to a project; the creator can
still provide an explicit preview override. It does not yet support non-integer
dialogue state, preserve transient page-presentation/choice-cursor state across
save, or qualify playthrough/package/release
behavior. Saved-graph execution from an authored map-event command is covered by
`PFU-04-map-event-dialogue-runtime-binding.md`. Builds and test execution
remain deferred under the user instruction for this phase.
