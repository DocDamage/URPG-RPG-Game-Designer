# PFU-04 Work Packet: Map Event Dialogue Runtime Binding

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Allow a Perspective 2D map event to start one saved native Dialogue Graph
without adding a general script or arbitrary file-loading command.

## Contract

- The native Map event command picker exposes `start_dialogue`. Its argument is
  the saved Dialogue Graph ID under `content/dialogues/<id>.json`. A visible,
  in-bounds event page projects that command into the native MapScene tile
  interaction path for its authored trigger. At input time, the final matching
  page wins using the same ordered switch, integer-variable, and event-local
  self-switch comparisons as the authoring preview.
- `MapScene` accepts only non-empty IDs containing letters, digits, `_`, or
  `-`, reads only that fixed project-local location, parses the native graph
  schema, and starts it using the existing authored-dialogue runtime owner.
- Missing project root/file, malformed JSON/schema, invalid graph admission,
  and invalid IDs are observable through bounded runtime diagnostics. A
  rejected command makes the event runtime result fail and does not persist
  its local Perspective 2D preview state.
- Supported map-event switch and integer-variable changes that precede
  `start_dialogue` project as typed native state writes. MapScene preflights
  the saved graph before applying them through `GlobalStateHub`, then evaluates
  supported dialogue choice conditions against that same authority. Missing or
  invalid graphs therefore cannot partially apply projected writes.
- MapScene rejects duplicate trigger/tile projections as a whole. The Map
  workspace clears a rejected runtime batch rather than leaving stale event
  interactions active.
- The event document remains the command/persistence/history owner; `MapScene`
  owns graph loading and native message execution. No browser, script engine,
  or external runtime is introduced.

## Limits

This supports visible switch/integer-variable/event-local-self-switch
conditional map-event pages and the existing native `confirm_interact` path;
other event commands, interaction animation/collision, dialogue-choice-effect
synchronization back into the Perspective 2D
preview-state document, and playthrough/package/
release qualification remain open. Builds and test execution remain deferred
under the user instruction for this phase.
