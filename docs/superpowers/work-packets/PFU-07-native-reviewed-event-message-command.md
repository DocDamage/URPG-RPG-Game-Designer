# PFU-07 Native Reviewed Event-Message Command

## Scope

Extend developer-only creator intent by one event domain: a single
confirm-interact `show_text` event on one explicit active Map layer. This is a
native Perspective 2D command, not generic event-logic authority.

## Contract

- A deterministic `place message event` request supplies one target, one
  selected event/object layer, one label, and one message of at most 1024
  characters.
- Review accepts only the exact message-event shape, captures the active Map
  revision, and binds the requested layer explicitly. All other logic kinds,
  mixed tile/prop/event plans, and missing bindings remain unavailable.
- Apply rechecks revision, layer visibility/lock/kind, bounds, operation-ID
  format, label/message limits, and the derived stable event-ID collision
  before mutation. The Map owner creates one event, one selected page, and one
  `show_text` command as one local undo/redo action.

## Limits

The command does not authorize provider-generated logic, choices, conditions,
transfers, shops, quests, battle, arbitrary event commands, or runtime/package
qualification. Provider transport stays dry-run only. Verification is deferred
under the user instruction not to run builds or tests during this phase.
