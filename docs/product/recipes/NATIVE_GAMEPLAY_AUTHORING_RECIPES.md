# Native Gameplay Authoring Recipes

These recipes use only URPG's native project contracts. They are deliberately
small, deterministic, and safe to repeat in a creator project.

## NPC patrol and proximity

1. Place an NPC with a stable ID in the Map workspace.
2. Bind the active `GridPartDocument` and `PathfindingGraph` to
   `GameplayRuntimeFacade`.
3. Register `NpcRuntimeState` with that same stable NPC ID.
4. Request `moveNpc(npc_id, destination)` for each authored patrol point.
5. Use `observeProximity` / `displayThought` through `NpcRuntimePrimitives`
   for a bounded authored response; do not generate dialogue in the runtime
   loop.

Movement returns a structured failure when navigation cannot produce a route.
The event system, not the NPC primitive, owns dialogue and quest effects.

## Stateful door or container

1. Give the placed prop a stable Map instance ID.
2. Create `MapPropState` entries such as `closed` and `open`; each names only
   an attached asset, collision footprint, render layer, and optional event.
3. Validate the set against the project's attached asset IDs and event IDs.
4. Select the desired state and call `applyTo` on the placed prop.
5. Persist the prop through the normal Map document save and invoke the
   optional event through `GameplayRuntimeFacade` during playtest.

The state set never accepts raw paths. A missing asset or event is a blocking
diagnostic, not an implicit fallback.

## Perspective 2D placement and occlusion

1. Author ground, occluder, and prop layers by authored layer semantics.
2. Store placement and interaction positions at an entity's feet/ground anchor.
3. Feed render candidates to `orderPerspective2DRenderItems`.

The ordering contract is layer, feet Y, then stable object ID. Collision,
navigation destinations, interaction distance, and rendering all consume the
same ground anchor; callers must not convert repeatedly from sprite top-left
coordinates.

## Quest and inventory hooks

1. Put a stable event ID in the quest's objective condition or reward.
2. Use the event runtime to invoke the authored event.
3. Apply a named resource change with `GameplayRuntimeFacade::addResource`.
4. Have the quest/runtime owner consume that resource through its existing
   project-data contract.

The facade supplies a compact, testable handoff. It does not replace the quest,
inventory, save, or event owners.

## HUD visibility and save-safe IDs

- Use stable IDs for maps, events, NPCs, props, assets, and resources.
- Keep HUD visibility in the presentation/UI owner; use facade results only as
  input for truthful status and diagnostics.
- Save references by stable ID, never editor pointers, generated file paths,
  recovery snapshots, or playtest overlays.

## Bounded local-MCP posture

The local automation surface may inspect and preview facade operations. A
durable change must be structured, diagnostic-producing, and explicitly
applied through an existing authorized project workflow. It must not execute
arbitrary runtime code or mutate arbitrary filesystem locations.
