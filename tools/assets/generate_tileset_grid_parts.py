#!/usr/bin/env python3
"""Generate Level Builder grid-part catalog entries from a tileset image."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--source-image", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--catalog-output", type=Path, required=True)
    parser.add_argument("--catalog-id", required=True)
    parser.add_argument("--part-prefix", required=True)
    parser.add_argument("--asset-prefix", required=True)
    parser.add_argument("--display-prefix", required=True)
    parser.add_argument("--category", default="Tile")
    parser.add_argument("--default-layer", default="Terrain")
    parser.add_argument("--collision-policy", default="None")
    parser.add_argument("--footprint-width", type=int, default=1)
    parser.add_argument("--footprint-height", type=int, default=1)
    parser.add_argument("--allow-overlap", action="store_true")
    parser.add_argument("--blocks-navigation", action="store_true")
    parser.add_argument("--tag", action="append", default=[])
    parser.add_argument("--tile-size", type=int, default=16)
    parser.add_argument("--tile-id-start", type=int, default=1000)
    parser.add_argument("--max-tiles", type=int, default=48)
    parser.add_argument("--source-license", default="CC0")
    parser.add_argument("--skip-rgb", default="", help="Optional R,G,B background color to skip when dominant.")
    parser.add_argument("--skip-threshold", type=float, default=0.95)
    return parser.parse_args()


def relative_posix(repo_root: Path, path: Path) -> str:
    return path.resolve().relative_to(repo_root.resolve()).as_posix()


def has_visible_pixels(tile: Image.Image) -> bool:
    if tile.mode != "RGBA":
        tile = tile.convert("RGBA")
    alpha = tile.getchannel("A")
    return alpha.getbbox() is not None


def parse_rgb(value: str) -> tuple[int, int, int] | None:
    if not value:
        return None
    parts = value.split(",")
    if len(parts) != 3:
        raise ValueError("--skip-rgb must be formatted as R,G,B")
    return tuple(max(0, min(255, int(part))) for part in parts)


def is_dominant_rgb(tile: Image.Image, rgb: tuple[int, int, int], threshold: float) -> bool:
    if tile.mode != "RGBA":
        tile = tile.convert("RGBA")
    pixels = tile.getdata()
    total = tile.width * tile.height
    if total <= 0:
        return True
    matches = 0
    for r, g, b, a in pixels:
        if a > 0 and abs(r - rgb[0]) <= 2 and abs(g - rgb[1]) <= 2 and abs(b - rgb[2]) <= 2:
            matches += 1
    return matches / total >= threshold


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    source_image = (repo_root / args.source_image).resolve()
    output_dir = (repo_root / args.output_dir).resolve()
    catalog_output = (repo_root / args.catalog_output).resolve()
    skip_rgb = parse_rgb(args.skip_rgb)

    output_dir.mkdir(parents=True, exist_ok=True)
    catalog_output.parent.mkdir(parents=True, exist_ok=True)

    parts = []
    with Image.open(source_image) as image:
        image = image.convert("RGBA")
        tile_index = 0
        for y in range(0, image.height - args.tile_size + 1, args.tile_size):
            for x in range(0, image.width - args.tile_size + 1, args.tile_size):
                tile = image.crop((x, y, x + args.tile_size, y + args.tile_size))
                if not has_visible_pixels(tile):
                    continue
                if skip_rgb is not None and is_dominant_rgb(tile, skip_rgb, args.skip_threshold):
                    continue

                filename = f"tile_{tile_index:03d}.png"
                tile_path = output_dir / filename
                tile.save(tile_path)

                part_id = f"{args.part_prefix}.{tile_index:03d}"
                asset_id = f"{args.asset_prefix}.{tile_index:03d}"
                parts.append(
                    {
                        "partId": part_id,
                        "displayName": f"{args.display_prefix} {tile_index:03d}",
                        "description": "Generated gameplay tile slice from a promoted tileset.",
                        "category": args.category,
                        "defaultLayer": args.default_layer,
                        "collisionPolicy": args.collision_policy,
                        "supportedRulesets": ["TopDownJRPG", "TownHub", "DungeonRoomBuilder"],
                        "footprint": {
                            "width": args.footprint_width,
                            "height": args.footprint_height,
                            "allowOverlap": args.allow_overlap,
                            "blocksNavigation": args.blocks_navigation,
                        },
                        "assetId": asset_id,
                        "tileId": args.tile_id_start + tile_index,
                        "previewPath": relative_posix(repo_root, tile_path),
                        "sourceImagePath": relative_posix(repo_root, source_image),
                        "atlasRect": {"x": x, "y": y, "width": args.tile_size, "height": args.tile_size},
                        "tags": sorted(set(["generated", "gameplay", args.category.lower(), args.source_license.lower(), *args.tag])),
                        "defaultProperties": {
                            "sourceLicense": args.source_license,
                            "sourceImagePath": relative_posix(repo_root, source_image),
                            "atlasRect": f"{x},{y},{args.tile_size},{args.tile_size}",
                        },
                    }
                )
                tile_index += 1
                if tile_index >= args.max_tiles:
                    break
            if tile_index >= args.max_tiles:
                break

    payload = {
        "schemaVersion": 1,
        "catalogId": args.catalog_id,
        "displayName": args.display_prefix,
        "description": "Generated Level Builder grid parts from promoted gameplay tileset slices.",
        "parts": parts,
    }
    catalog_output.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(f"Generated {len(parts)} parts into {catalog_output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
