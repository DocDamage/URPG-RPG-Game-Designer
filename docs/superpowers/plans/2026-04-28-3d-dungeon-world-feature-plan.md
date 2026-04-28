# 3D Dungeon World Feature Plan

## Goal

Implement the missing RPG Maker-style 3D world feature as a concrete URPG WYSIWYG slice.

## Complete Scope

- Saved 3D dungeon world document.
- Conversion from authored 2D grid cells into a faux-3D raycast runtime.
- 2D/3D mode switching.
- Wall/floor/ceiling material customization.
- Minimap and auto-mapping preview.
- Event/dark-zone/stair metadata on cells.
- Runtime preview commands.
- Diagnostics for invalid maps, cameras, cells, and materials.
- Editor panel snapshot for live WYSIWYG preview.
- Release top-level editor registry entry.
- Focused tests.

## Explicit Boundary

This completes a RPG Maker-style 3D dungeon/world feature. It is not an arbitrary 3D mesh engine with model import, skeletal animation, physics, and freeform terrain.
