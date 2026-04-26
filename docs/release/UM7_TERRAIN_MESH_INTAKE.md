# UM7 Terrain Mesh Intake Decision

Status Date: 2026-04-26

## Decision

`imports/staging/plugin_intake/UM7_TerrainMesh/UM7_TerrainMesh.js` remains a quarantined backlog reference. It is not a release blocker, not a compat fixture, and not approved for native implementation.

The safe implementation direction is native backlog requirements capture, not direct plugin promotion. URPG already has `SpatialMapOverlay`, `ElevationGrid`, and spatial editor panels, so the plugin should inform future terrain-elevation requirements only after provenance and license review are complete.

## Provenance And License

- Plugin header identifies the author as `Paradajz`.
- Plugin header declares `@base UltraMode7`.
- Plugin header does not include a source URL.
- Plugin header does not include license or redistribution terms.
- Web searches performed on 2026-04-26 did not identify an authoritative public source:
  - `"UM7_TerrainMesh" Paradajz`
  - `"The Continuous Mesh" "Paradajz" "UltraMode7"`
  - `"$getUM7PixelHeight"`

## Compatibility Impact

The plugin patches RPG Maker MZ and UltraMode7 globals directly:

- `PluginManager`
- `$dataMap`
- `$gameMap`
- `$gamePlayer`
- `Game_Player`
- `Game_Vehicle`
- `Sprite_Character`
- `Tilemap.Layer`
- `UltraMode7`

Because `UltraMode7` itself is not present as a verified, licensed compatibility target in this repository, a fixture for `UM7_TerrainMesh` would create a false support claim. Do not add `tests/compat/fixtures/plugins/UM7_TerrainMesh.json` until UltraMode7 provenance, license, and intended compatibility scope are approved.

## Native Backlog Requirements

If the release owner later promotes this as native terrain behavior, implementation must start with tests for:

- Terrain-tag-to-elevation mapping for tags 1 through 5.
- Region-driven passability override behavior.
- Region-driven elevation override behavior, including `hillregion` and `RegionHeightN` map metadata equivalents.
- Actor and vehicle elevation offsets, especially airship behavior.
- Forest/visual-only height treatment versus collision height.
- Presentation mapping through `SpatialMapOverlay::elevation`.
- Editor preview and authoring affordances in the spatial workspace.

## Required Tests Before Code

- Unit tests for deterministic tile and region elevation resolution.
- Unit tests for passability override precedence.
- Spatial presentation tests proving actor/prop Y positions resolve from the authored elevation grid.
- Editor snapshot tests proving terrain/region elevation choices are visible before runtime.
- Compat dependency tests only after UltraMode7 is approved as a fixture target.

## Current Verification

- `Get-Content imports/staging/plugin_intake/UM7_TerrainMesh/manifest.json`
- `rg -n "UM7_TerrainMesh|UltraMode7|um7_terrain_mesh" imports docs tests runtimes engine editor`

Promotion remains unverified and blocked until source URL, license, redistribution terms, and UltraMode7 dependency scope are approved.
