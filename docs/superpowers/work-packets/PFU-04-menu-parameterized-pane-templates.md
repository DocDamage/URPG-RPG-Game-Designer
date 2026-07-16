# PFU-04 Parameterized Native Menu Pane Templates

## Scope

Extend the existing Menu Studio pane-template command with bounded creator
parameters. This is native `menu_scene_graph` layout authoring, not an
imported HTML component system or a new UI ownership layer.

## Contract

- Compact List, Centered Dialog, Bottom Overlay, and Full Canvas templates now
  accept a non-negative margin plus optional preferred width and height.
  Zero preferred dimensions retain the selected template's native default.
- The owner clamps derived dimensions to the authored canvas after its effective
  margin is applied. Full Canvas continues to occupy the canvas by definition.
- A parameterized template is applied through the existing Menu Inspector model
  as one local history mutation, then follows the existing runtime/project save
  route. It retains the pane ID, commands, layer, focus order, and responsive
  anchor semantics.

## Limits

This does not add a persisted component library, template import/export,
parameter expressions, collaborative history, or target-size/manual
qualification. Automated verification was audited on 2026-07-16; see the active PFU evidence. Manual/package qualification remains outside this bounded packet.
