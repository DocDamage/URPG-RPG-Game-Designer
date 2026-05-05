# Gameplay Asset Slices

This folder contains editor-ready gameplay thumbnails generated from promoted, license-cleared sources.

Current vertical slice:

- `src012/grunge_tileset/`: 48 generated `16x16` tile previews from `imports/normalized/src012_cc0_tiles_vfx/tilesets/grunge_tileset.png`.
- `src012/caverns_tiles/`: 64 generated `16x16` tile previews from `imports/normalized/src012_cc0_tiles_vfx/tilesets/caverns_tiles.png`.
- `cutesckr/medieval_fantasy_dungeon_1/`: 96 generated `48x48` previews from the extracted CuteSCKR medieval dungeon sheet `1.png`.
- `cutesckr/medieval_fantasy_town_1/`: 96 generated `48x48` previews from the extracted CuteSCKR medieval town sheet `1.png`.
- `cutesckr/farm_1/`: 78 generated `48x48` previews from the extracted CuteSCKR farm sheet `1.png`.
- `cutesckr/forest_wilderness_1/`: 96 generated `48x48` previews from the extracted CuteSCKR forest wilderness sheet `1.png`.
- `cutesckr/medieval_castle_1/`: 96 generated `48x48` previews from the extracted CuteSCKR medieval castle sheet `1.png`.
- `cutesckr/cyberpunk_city_1/`: 96 generated `48x48` previews from the extracted CuteSCKR cyberpunk city sheet `1.png`.
- `cutesckr/medieval_fantasy_town_props_2/`: 80 generated `Prop` previews from the extracted CuteSCKR medieval town sheet `2.png`.
- `cutesckr/medieval_fantasy_dungeon_walls_2/`: 80 generated `Wall` previews from the extracted CuteSCKR medieval dungeon sheet `2.png`.
- `cutesckr/medieval_castle_walls_2/`: 80 generated `Wall` previews from the extracted CuteSCKR medieval castle sheet `2.png`.
- `cutesckr/cyberpunk_city_props_2/`: 80 generated `Prop` previews from the extracted CuteSCKR cyberpunk city sheet `2.png`.
- `promoted_legacy/`: 17,297 generated thumbnails from older promoted bundle manifests `BND-006`, `BND-007`, and `BND-008`.
- `cutesckr_all/`: 71,404 generated `48x48` gameplay slices from all 747 PNG sheets extracted from the 90 CuteSCKR archives.
- `human_rpg_portraits/`: 648 generated portrait thumbnails from the Human RPG Portrait Pack archives.
- `modernui_portrait_generator/`: 903 generated character/portrait thumbnails from promoted ModernUI portrait-generator PNG assets.

The default Level Builder startup catalog is `content/part_catalogs/base_jrpg_parts.json`, which keeps editor startup fast with the curated starter slice. The full generated game-maker catalog is `content/part_catalogs/game_maker_all_parts.json`; it includes the older promoted library, full CuteSCKR import, Human portraits, and ModernUI portrait-generator material.

The initial CuteSCKR subset is extracted under `imports/raw/cutesckr` with the source license copied alongside it. The full CuteSCKR archive extraction is under `imports/raw/cutesckr_all`, with extraction and grid-part generation reports under `imports/reports/asset_intake/`.

ModernUI app chrome/control-skin integration is intentionally not part of this gameplay asset catalog. Only the ModernUI portrait-generator PNGs are exposed here as game-maker character/portrait material.
