#!/usr/bin/env python3
"""Generate Level Builder grid-part catalogs from promoted asset bundle manifests."""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageSequence


IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".gif"}


@dataclass(frozen=True)
class GenerationResult:
    generated_catalogs: int
    generated_parts: int
    skipped_assets: int
    catalog_paths: list[Path]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--bundle", type=Path, action="append", required=True)
    parser.add_argument("--output-root", type=Path, default=Path("content/assets/gameplay/promoted_legacy"))
    parser.add_argument(
        "--catalog-root",
        type=Path,
        default=Path("content/part_catalogs/generated/promoted_legacy"),
    )
    parser.add_argument("--max-thumbnail-size", type=int, default=64)
    parser.add_argument("--report-output", type=Path, default=None)
    return parser.parse_args()


def slug(value: str) -> str:
    value = value.strip().lower()
    value = re.sub(r"[^a-z0-9]+", "_", value)
    value = re.sub(r"_+", "_", value)
    return value.strip("_") or "asset"


def relative_posix(repo_root: Path, path: Path) -> str:
    return path.resolve().relative_to(repo_root.resolve()).as_posix()


def normalized_asset_path(repo_root: Path, promoted_relative_path: str) -> Path:
    promoted = Path(promoted_relative_path)
    if promoted.parts[:2] == ("imports", "normalized"):
        return repo_root / promoted
    return repo_root / "imports" / "normalized" / promoted


def category_mapping(source_category: str) -> tuple[str, str, str, bool, bool]:
    category = source_category.lower()
    if "tile" in category:
        return "Tile", "Terrain", "None", False, False
    if "sprite" in category or "character" in category or "portrait" in category:
        return "Npc", "Objects", "Interact", True, False
    if "vfx" in category:
        return "Hazard", "Effects", "Trigger", True, False
    if "ui" in category:
        return "Prop", "Decor", "None", True, False
    return "Prop", "Decor", "None", True, False


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


def iter_promoted_image_assets(bundle: dict) -> Iterable[dict]:
    for asset in bundle.get("assets", []):
        promoted_path = asset.get("promoted_relative_path", "")
        if not promoted_path:
            continue
        if Path(promoted_path).suffix.lower() not in IMAGE_EXTENSIONS:
            continue
        if asset.get("license_cleared") is False or asset.get("release_eligible") is False:
            continue
        yield asset


def build_part(
    *,
    repo_root: Path,
    bundle_id: str,
    bundle_name: str,
    asset: dict,
    index: int,
    source_path: Path,
    preview_path: Path,
    width: int,
    height: int,
) -> dict:
    source_category = str(asset.get("category", "prop"))
    category, layer, collision, allow_overlap, blocks_navigation = category_mapping(source_category)
    category_slug = slug(category)
    bundle_slug = slug(bundle_id)
    asset_stem = slug(Path(asset["promoted_relative_path"]).stem)
    part_id = f"promoted.{bundle_slug}.{category_slug}.{index:05d}"
    display_name = f"{bundle_name} {asset_stem}".replace("_", " ").strip()
    source_relative = relative_posix(repo_root, source_path)
    preview_relative = relative_posix(repo_root, preview_path)
    tags = sorted(
        {
            "generated",
            "promoted",
            "legacy",
            slug(bundle_id).replace("_", "-"),
            slug(source_category).replace("_", "-"),
            category.lower(),
        }
    )

    return {
        "partId": part_id,
        "displayName": display_name,
        "description": "Generated from a promoted legacy asset bundle for Level Builder placement.",
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
        "assetId": f"promoted.{bundle_slug}.{index:05d}",
        "tileId": 500000 + index,
        "previewPath": preview_relative,
        "sourceImagePath": source_relative,
        "atlasRect": {"x": 0, "y": 0, "width": width, "height": height},
        "tags": tags,
        "defaultProperties": {
            "sourceBundleId": bundle_id,
            "sourceCategory": source_category,
            "sourceImagePath": source_relative,
            "atlasRect": f"0,0,{width},{height}",
            "checksumSha256": str(asset.get("checksum_sha256", "")),
        },
    }


def generate_catalogs(
    *,
    repo_root: Path,
    bundle_paths: list[Path],
    output_root: Path,
    catalog_root: Path,
    max_thumbnail_size: int = 64,
) -> GenerationResult:
    repo_root = repo_root.resolve()
    output_root = (repo_root / output_root).resolve() if not output_root.is_absolute() else output_root.resolve()
    catalog_root = (repo_root / catalog_root).resolve() if not catalog_root.is_absolute() else catalog_root.resolve()
    catalog_root.mkdir(parents=True, exist_ok=True)
    catalog_paths: list[Path] = []
    generated_parts = 0
    skipped_assets = 0

    for bundle_path in bundle_paths:
        bundle_file = (repo_root / bundle_path).resolve() if not bundle_path.is_absolute() else bundle_path.resolve()
        bundle = json.loads(bundle_file.read_text(encoding="utf-8"))
        bundle_id = str(bundle.get("bundle_id", bundle_file.stem))
        bundle_name = str(bundle.get("bundle_name", bundle_id))
        bundle_slug = slug(bundle_id)
        parts = []

        for asset in iter_promoted_image_assets(bundle):
            source_path = normalized_asset_path(repo_root, asset["promoted_relative_path"])
            if not source_path.exists():
                skipped_assets += 1
                continue
            source_category = str(asset.get("category", "prop"))
            category, _, _, _, _ = category_mapping(source_category)
            preview_path = output_root / bundle_slug / slug(category) / f"{len(parts):05d}.png"
            try:
                width, height = save_thumbnail(source_path, preview_path, max_thumbnail_size)
            except (OSError, ValueError):
                skipped_assets += 1
                continue
            parts.append(
                build_part(
                    repo_root=repo_root,
                    bundle_id=bundle_id,
                    bundle_name=bundle_name,
                    asset=asset,
                    index=len(parts),
                    source_path=source_path,
                    preview_path=preview_path,
                    width=width,
                    height=height,
                )
            )

        if not parts:
            continue

        catalog = {
            "schemaVersion": 1,
            "catalogId": f"legacy.{bundle_slug}",
            "displayName": f"{bundle_name} Level Builder Parts",
            "description": "Generated Level Builder grid parts from promoted legacy asset bundle records.",
            "parts": parts,
        }
        catalog_path = catalog_root / f"{bundle_slug}_parts.json"
        catalog_path.write_text(json.dumps(catalog, indent=2) + "\n", encoding="utf-8")
        catalog_paths.append(catalog_path.relative_to(repo_root))
        generated_parts += len(parts)

    return GenerationResult(
        generated_catalogs=len(catalog_paths),
        generated_parts=generated_parts,
        skipped_assets=skipped_assets,
        catalog_paths=catalog_paths,
    )


def main() -> int:
    args = parse_args()
    result = generate_catalogs(
        repo_root=args.repo_root,
        bundle_paths=args.bundle,
        output_root=args.output_root,
        catalog_root=args.catalog_root,
        max_thumbnail_size=args.max_thumbnail_size,
    )
    if args.report_output is not None:
        report_path = (args.repo_root / args.report_output).resolve()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(
            json.dumps(
                {
                    "generated_catalogs": result.generated_catalogs,
                    "generated_parts": result.generated_parts,
                    "skipped_assets": result.skipped_assets,
                    "catalog_paths": [path.as_posix() for path in result.catalog_paths],
                },
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )
    print(
        f"Generated {result.generated_parts} promoted grid parts in "
        f"{result.generated_catalogs} catalogs; skipped {result.skipped_assets} assets."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
