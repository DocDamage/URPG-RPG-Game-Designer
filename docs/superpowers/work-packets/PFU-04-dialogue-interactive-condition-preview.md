# PFU-04 Dialogue Interactive Condition Preview

## Scope

The native Dialogue Graph now has a bounded, non-persistent interactive preview for authored choices, conditions, and effects.

## Behavior

- A preview starts only at the graph's valid authored start node.
- Choice conditions support `=`, `==`, `!=`, `>`, `>=`, `<`, and `<=` against session-local integer values; unsupported operators are explicitly disabled and diagnosed.
- Choosing an enabled option applies its typed integer effects to the session-local values and advances to the authored target node.
- The Map Dialogue Authoring control offers only those supported operators for new conditions; persisted unsupported operators remain loadable and visibly disabled for diagnosis.
- The project-owned Map Dialogue Authoring surface and the panel snapshot expose the current node, choices, enabled state, diagnostics, value state, and a 64-step trace cap.
- Preview never changes the saved graph, project state, runtime state, history, or recovery data.

## Limits

This is authoring preview evidence, not a claim that Dialogue Graph is the final dialogue runtime executor. Localization/audio presentation, runtime handoff, playtest integration, migration breadth, and manual graph-playthrough evidence remain PFU-04 work. Verification is deferred by the user's instruction not to build or test during this implementation phase.
