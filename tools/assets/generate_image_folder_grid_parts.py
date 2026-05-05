#!/usr/bin/env python3
"""Generate Level Builder catalog entries from individual image files."""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageSequence


IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".gif"}


@dataclass(frozen=True)
class GenerationResult:
    generated_parts: int
    skipped_images: int
    catalog_path: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--catalog-output", type=Path, required=True)
    parser.add_argument("--catalog-id", required=True)
    parser.add_argument("--display-name", required=True)
    parser.add_argument("--part-prefix", required=True)
    parser.add_argument("--category", default="Npc")
    parser.add_argument("--source-bundle-id", required=True)
    parser.add_argument("--max-thumbnail-size", type=int, default=96)
    parser.add_argument("--report-output", type=Path, default=None)
    return parser.parse_args()


def slug(value: str) -> str:
    value = value.strip().lower()
    value = re.sub(r"[^a-z0-9]+", "_", value)
    value = re.sub(r"_+", "_", value)
    return value.strip("_") or "asset"


def relative_posix(repo_root: Path, path: Path) -> str:
    return path.resolve().relative_to(repo_root.resolve()).as_posix()


def open_preview_image(path: Path) -> Image.Image:
    image = Image.open(path)
    if path.suffix.lower() == ".gif":
        image = next(ImageSequence.Iterator(image))
    return image.convert("RGBA")


def save_thumbnail(source: Path, destination: Path, max_thumbnail_size: int) -> tuple[int, int]:
    destination.parent.mkdir(parents=True, exist_ok=True)
    with open_preview_image(source) as image:
        width, height = image.size
        thumbnail = image.copy()
        thumbnail.thumbnail((max_thumbnail_size, max_thumbnail_size), Image.Resampling.NEAREST)
        thumbnail.save(destination)
        return width, height


def iter_images(source_root: Path) -> list[Path]:
    return sorted(
        (path for path in source_root.rglob("*") if path.is_file() and path.suffix.lower() in IMAGE_EXTENSIONS),
        key=lambda path: path.relative_to(source_root).as_posix().lower(),
    )


def category_defaults(category: str) -> tuple[str, str, bool, bool]:
    if category == "Npc":
        return "Actor", "Interact", True, False
    if category == "Prop":
        return "Decor", "None", True, False
    return "Object", "None", True, False


def generate_catalog(
    *,
    repo_root: Path,
    source_root: Path,
    output_root: Path,
    catalog_output: Path,
    catalog_id: str,
    display_name: str,
    part_prefix: str,
    category: str,
    source_bundle_id: str,
    max_thumbnail_size: int = 96,
) -> GenerationResult:
    repo_root = repo_root.resolve()
    source_root = (repo_root / source_root).resolve() if not source_root.is_absolute() else source_root.resolve()
    output_root = (repo_root / output_root).resolve() if not output_root.is_absolute() else output_root.resolve()
    catalog_output = (repo_root / catalog_output).resolve() if not catalog_output.is_absolute() else catalog_output.resolve()
    layer, collision, allow_overlap, blocks_navigation = category_defaults(category)

    parts = []
    skipped_images = 0
    for image_path in iter_images(source_root):
        preview_path = output_root / f"{len(parts):05d}_{slug(image_path.stem)}.png"
        try:
            width, height = save_thumbnail(image_path, preview_path, max_thumbnail_size)
        except (OSError, ValueError):
            skipped_images += 1
            continue
        source_relative = relative_posix(repo_root, image_path)
        parts.append(
            {
                "partId": f"{part_prefix}.{len(parts):05d}",
                "displayName": image_path.stem.replace("_", " ").replace("-", " "),
                "description": "Generated from an individual image asset for game-maker use.",
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
                "assetId": f"{part_prefix}.{len(parts):05d}",
                "tileId": 900000 + len(parts),
                "previewPath": relative_posix(repo_root, preview_path),
                "sourceImagePath": source_relative,
                "atlasRect": {"x": 0, "y": 0, "width": width, "height": height},
                "tags": sorted({"generated", "gameplay", "portrait", source_bundle_id.lower(), category.lower()}),
                "defaultProperties": {
                    "sourceBundleId": source_bundle_id,
                    "sourceImagePath": source_relative,
                    "atlasRect": f"0,0,{width},{height}",
                    "sourceLicense": "CC0",
                },
            }
        )

    catalog_output.parent.mkdir(parents=True, exist_ok=True)
    catalog_output.write_text(
        json.dumps(
            {
                "schemaVersion": 1,
                "catalogId": catalog_id,
                "displayName": display_name,
                "description": "Generated Level Builder grid parts from individual image assets.",
                "parts": parts,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return GenerationResult(len(parts), skipped_images, catalog_output.relative_to(repo_root))


def main() -> int:
    args = parse_args()
    result = generate_catalog(
        repo_root=args.repo_root,
        source_root=args.source_root,
        output_root=args.output_root,
        catalog_output=args.catalog_output,
        catalog_id=args.catalog_id,
        display_name=args.display_name,
        part_prefix=args.part_prefix,
        category=args.category,
        source_bundle_id=args.source_bundle_id,
        max_thumbnail_size=args.max_thumbnail_size,
    )
    if args.report_output is not None:
        report_path = (args.repo_root / args.report_output).resolve()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(result.__dict__, indent=2, default=str) + "\n", encoding="utf-8")
    print(f"Generated {result.generated_parts} image parts; skipped {result.skipped_images} images.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
