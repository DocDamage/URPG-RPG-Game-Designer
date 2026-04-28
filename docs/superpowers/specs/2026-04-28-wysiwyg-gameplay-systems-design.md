# WYSIWYG Gameplay Systems Design

## Goal

Add twelve creator-facing RPG/gameplay systems and make each one usable through URPG's WYSIWYG completion contract: visual authoring surface, live preview, saved project data, runtime execution, diagnostics, and tests.

## Systems

The batch covers:

1. Status Effect / Buff-Debuff Designer
2. Enemy AI Behavior Tree Editor
3. Boss Phase / Encounter Script Editor
4. Equipment Set Bonus System
5. Dungeon / Room Flow Designer
6. Companion / Party Banter System
7. Quest Choice Consequence Simulator
8. Shop / Economy Simulation Lab
9. Puzzle Mechanic Builder
10. World State Timeline / Calendar Events
11. Tactical Battle Terrain Effects
12. Procedural Content Rule Editor

## Architecture

Implement a shared deterministic authoring kernel under `engine/core/gameplay/` for these game-facing systems. Each authored document has a feature type, visual layers, deterministic rules, triggers, conditions, runtime state changes, diagnostics, JSON round-trip support, and a preview/execution API. This keeps the first full WYSIWYG pass consistent while allowing later systems to split into deeper specialist modules when product depth demands it.

Editor support lives under `editor/gameplay/` as a shared panel model that loads a document, edits preview context, renders a deterministic snapshot, saves project JSON, and executes the same runtime logic the shipped game uses. The editor registry exposes each of the twelve systems as a release top-level WYSIWYG surface so compiled panels are also reachable by navigation/smoke coverage.

## Data Contract

Each fixture uses:

- `schema_version`: `urpg.gameplay_wysiwyg.v1`
- `feature_type`: one of the twelve canonical feature ids
- `id` and `display_name`
- `visual_layers`: visual authoring lanes shown by the panel
- `rules`: triggerable deterministic rules with conditions, effects, values, durations, flag grants, and variable/resource writes

## Runtime Behavior

`preview()` reports which rules would fire for a trigger and state. `execute()` applies the same selected rules to mutable runtime state, including flags, variables, resources, and emitted runtime events. Diagnostics report missing ids, duplicate rules, missing visual layers, invalid numeric bounds, empty triggers/effects, and unsatisfied or unknown dependencies where provided.

## WYSIWYG Evidence

Every system must have:

- a registered editor panel id
- a fixture and schema
- live preview snapshot
- saved project JSON
- runtime execution through the shared kernel
- diagnostics
- focused Catch2 coverage

