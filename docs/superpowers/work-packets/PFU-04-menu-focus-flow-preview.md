# PFU-04 Work Packet: native Menu Studio focus-flow preview

**Status:** Implemented; automated verification audited on 2026-07-16; see the active PFU evidence

**Date:** 2026-07-15

## Scope

Render authored pane traversal directly on the native Menu Studio canvas. Each
visible pane with commands receives a numbered focus badge, and guides connect
the traversal in the exact explicit-focus-then-legacy-insertion ordering used
by the native scene graph.

## Contract

1. Explicit non-negative focus orders precede legacy negative orders; ties and
   legacy entries retain pane insertion order.
2. The overlay is authoring feedback only. It does not mutate the graph or
   claim that runtime visibility/enabled rules will make every pane navigable.
3. Moving/resizing a pane updates only overlay geometry while the interaction
   is in progress; the underlying project mutation still occurs on release.

## Acceptance and verification

- Canvas badges and guides match the graph's authored focus ordering.
- A pane with no commands is excluded, matching baseline navigability.
- The overlay does not change serialization, dirty state, or local history.

Packet-local verification commands (current pass not implied):

`./build/dev-ninja-debug/urpg_tests.exe "[ui][menu][focus]" --reporter compact`

## Rollback and limits

Removing the overlay leaves the runtime focus behavior unchanged. Dynamic
visibility/enabled evaluation, controller-device simulation, keyboard-only
editor operation, and manual input evidence remain separate PFU-06 work.
