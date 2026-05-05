#!/usr/bin/env python3
"""Generate Level Builder catalogs from extracted CuteSCKR tileset sheets."""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


@dataclass(frozen=True)
class GenerationResult:
    generated_catalogs: int
    generated_sheets: int
    generated_parts: int
    skipped_sheets: int
    aggregate_catalog: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument(
        "--source-root",
        type=Path,
        default=Path("imports/raw/cutesckr_all/__archive_extracted"),
    )
    parser.add_argument("--output-root", type=Path, default=Path("content/assets/gameplay/cutesckr_all"))
    parser.add_argument("--catalog-root", type=Path, default=Path("content/part_catalogs/generated/cutesckr_all"))
    parser.add_argument(
        "--aggregate-catalog",
        type=Path,
        default=Path("content/part_catalogs/generated/cutesckr_all_parts.json"),
    )
    parser.add_argument("--tile-size", type=int, default=48)
    parser.add_argument("--max-tiles-per-sheet", type=int, default=96)
    parser.add_argument("--report-output", type=Path, default=None)
    return parser.parse_args()


def slug(value: str) -> str:
    value = value.strip().lower()
    value = re.sub(r"[^a-z0-9]+", "_", value)
    value = re.sub(r"_+", "_", value)
    return value.strip("_") or "asset"


def relative_posix(repo_root: Path, path: Path) -> str:
    return path.resolve().relative_to(repo_root.resolve()).as_posix()


def has_visible_pixels(tile: Image.Image) -> bool:
    if tile.mode != "RGBA":
        tile = tile.convert("RGBA")
    return tile.getchannel("A").getbbox() is not None


def sheet_category(sheet: Path) -> tuple[str, str, str, bool, bool]:
    name = sheet.stem.lower()
    if name in {"1", "01"} or "tile" in name or "level" in name or "ground" in name:
        return "Tile", "Terrain", "None", False, False
    if "wall" in name:
        return "Wall", "Collision", "Solid", False, True
    return "Prop", "Decor", "None", True, False


def sheet_sort_key(path: Path) -> tuple[str, int, str]:
    stem = path.stem
    return (path.parent.as_posix().lower(), int(stem) if stem.isdigit() else 999999, stem.lower())


def iter_sheets(source_root: Path) -> list[Path]:
    return sorted((path for path in source_root.rglob("*.png") if path.is_file()), key=sheet_sort_key)


def find_pack_root(source_root: Path, sheet: Path) -> Path:
    relative = sheet.relative_to(source_root)
    return source_root / relative.parts[0]


def generate_sheet_catalog(
    *,
    repo_root: Path,
    source_root: Path,
    sheet: Path,
    output_root: Path,
    catalog_root: Path,
    tile_size: int,
    max_tiles_per_sheet: int,
) -> tuple[Path, int] | None:
    pack_root = find_pack_root(source_root, sheet)
    pack_slug = slug(pack_root.name)
    sheet_slug = slug("_".join(sheet.relative_to(pack_root).with_suffix("").parts))
    category, layer, collision, allow_overlap, blocks_navigation = sheet_category(sheet)
    output_dir = output_root / pack_slug / sheet_slug
    catalog_path = catalog_root / f"{pack_slug}_{sheet_slug}_parts.json"
    parts = []

    with Image.open(sheet) as image:
        image = image.convert("RGBA")
        tile_index = 0
        for y in range(0, image.height - tile_size + 1, tile_size):
            for x in range(0, image.width - tile_size + 1, tile_size):
                tile = image.crop((x, y, x + tile_size, y + tile_size))
                if not has_visible_pixels(tile):
                    continue
                output_dir.mkdir(parents=True, exist_ok=True)
                preview_path = output_dir / f"tile_{tile_index:03d}.png"
                tile.save(preview_path)
                part_id = f"cutesckr.all.{pack_slug}.{sheet_slug}.{tile_index:03d}"
                source_relative = relative_posix(repo_root, sheet)
                parts.append(
                    {
                        "partId": part_id,
                        "displayName": f"{pack_root.name} {sheet_slug} {tile_index:03d}".replace("_", " "),
                        "description": "Generated gameplay slice from the full CuteSCKR tileset import.",
                        "category": category,
                        "defaultLayer": layer,
                        "collisionPolicy": collision,
                        "supportedRulesets": ["TopDownJRPG", "TownHub", "DungeonRoomBuilder"],
                        "footprint": {
                            "width": 1,
                            "height": 1,
                            "allowOverlap": allow_overlap,
                            "blocksNavigation": blocks_navigation,
                        },
                        "assetId": f"cutesckr.all.{pack_slug}.{sheet_slug}.{tile_index:03d}",
                        "tileId": 700000 + len(parts),
                        "previewPath": relative_posix(repo_root, preview_path),
                        "sourceImagePath": source_relative,
                        "atlasRect": {"x": x, "y": y, "width": tile_size, "height": tile_size},
                        "tags": sorted({"generated", "gameplay", "cutesckr", category.lower(), pack_slug}),
                        "defaultProperties": {
                            "sourceBundleId": pack_root.name,
                            "sourceImagePath": source_relative,
                            "atlasRect": f"{x},{y},{tile_size},{tile_size}",
                            "sourceLicense": "CC0",
                        },
                    }
                )
                tile_index += 1
                if tile_index >= max_tiles_per_sheet:
                    break
            if tile_index >= max_tiles_per_sheet:
                break

    if not parts:
        return None

    catalog_path.parent.mkdir(parents=True, exist_ok=True)
    catalog_path.write_text(
        json.dumps(
            {
                "schemaVersion": 1,
                "catalogId": f"cutesckr.all.{pack_slug}.{sheet_slug}",
                "displayName": f"{pack_root.name} {sheet_slug}".replace("_", " "),
                "description": "Generated Level Builder grid parts from the full CuteSCKR import.",
                "parts": parts,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return catalog_path, len(parts)


def generate_catalogs(
    *,
    repo_root: Path,
    source_root: Path,
    output_root: Path,
    catalog_root: Path,
    aggregate_catalog: Path,
    tile_size: int = 48,
    max_tiles_per_sheet: int = 96,
) -> GenerationResult:
    repo_root = repo_root.resolve()
    source_root = (repo_root / source_root).resolve() if not source_root.is_absolute() else source_root.resolve()
    output_root = (repo_root / output_root).resolve() if not output_root.is_absolute() else output_root.resolve()
    catalog_root = (repo_root / catalog_root).resolve() if not catalog_root.is_absolute() else catalog_root.resolve()
    aggregate_catalog = (
        (repo_root / aggregate_catalog).resolve() if not aggregate_catalog.is_absolute() else aggregate_catalog.resolve()
    )

    generated_catalogs: list[Path] = []
    generated_parts = 0
    skipped_sheets = 0
    for sheet in iter_sheets(source_root):
        generated = generate_sheet_catalog(
            repo_root=repo_root,
            source_root=source_root,
            sheet=sheet,
            output_root=output_root,
            catalog_root=catalog_root,
            tile_size=tile_size,
            max_tiles_per_sheet=max_tiles_per_sheet,
        )
        if generated is None:
            skipped_sheets += 1
            continue
        catalog_path, part_count = generated
        generated_catalogs.append(catalog_path)
        generated_parts += part_count

    aggregate_catalog.parent.mkdir(parents=True, exist_ok=True)
    includes = [relative_posix(aggregate_catalog.parent, path) for path in generated_catalogs]
    aggregate_catalog.write_text(
        json.dumps(
            {
                "schemaVersion": 1,
                "catalogId": "cutesckr_all_parts",
                "displayName": "CuteSCKR Full Import",
                "description": "Aggregate includes for generated CuteSCKR Level Builder grid-part catalogs.",
                "includes": includes,
                "parts": [],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return GenerationResult(
        generated_catalogs=len(generated_catalogs),
        generated_sheets=len(generated_catalogs),
        generated_parts=generated_parts,
        skipped_sheets=skipped_sheets,
        aggregate_catalog=aggregate_catalog.relative_to(repo_root),
    )


def main() -> int:
    args = parse_args()
    result = generate_catalogs(
        repo_root=args.repo_root,
        source_root=args.source_root,
        output_root=args.output_root,
        catalog_root=args.catalog_root,
        aggregate_catalog=args.aggregate_catalog,
        tile_size=args.tile_size,
        max_tiles_per_sheet=args.max_tiles_per_sheet,
    )
    if args.report_output is not None:
        report_path = (args.repo_root / args.report_output).resolve()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(result.__dict__, indent=2, default=str) + "\n", encoding="utf-8")
    print(
        f"Generated {result.generated_parts} CuteSCKR parts from {result.generated_sheets} sheets "
        f"in {result.generated_catalogs} catalogs; skipped {result.skipped_sheets} sheets."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
