#!/usr/bin/env python3
"""Convert a local Unity-Duelyst-Animations checkout into URPG sprite atlas manifests.

The tool does not clone or vendor assets. Point it at a local checkout of
https://github.com/DocDamage/Unity-Duelyst-Animations and write generated
metadata into imports/normalized/duelyst_animations/ or another staging folder.
"""

from __future__ import annotations

import argparse
import json
import plistlib
import re
import shutil
import struct
from pathlib import Path


SOURCE_ID = "SRC-006"
SOURCE_REPO = "DocDamage/Unity-Duelyst-Animations"
SOURCE_URL = "https://github.com/DocDamage/Unity-Duelyst-Animations"
LICENSE = "CC0-1.0"
FRAME_DURATION_SECONDS = 0.05


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as handle:
        header = handle.read(24)
    if len(header) < 24 or header[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path} is not a readable PNG")
    return struct.unpack(">II", header[16:24])


def parse_pair(value: str) -> tuple[int, int]:
    numbers = [int(item) for item in re.findall(r"-?\d+", value)]
    if len(numbers) != 2:
        raise ValueError(f"Expected two numbers in {value!r}")
    return numbers[0], numbers[1]


def parse_rect(value: str) -> tuple[int, int, int, int]:
    numbers = [int(item) for item in re.findall(r"-?\d+", value)]
    if len(numbers) != 4:
        raise ValueError(f"Expected four numbers in {value!r}")
    return numbers[0], numbers[1], numbers[2], numbers[3]


def split_frame_name(unit_id: str, frame_name: str) -> tuple[str, int]:
    stem = Path(frame_name).stem
    prefix = f"{unit_id}_"
    if stem.startswith(prefix):
        stem = stem[len(prefix) :]
    match = re.match(r"(?P<animation>.+)_(?P<frame>\d+)$", stem)
    if match is None:
        raise ValueError(f"Frame name {frame_name!r} does not end with a numeric frame suffix")
    return match.group("animation"), int(match.group("frame"))


def convert_unit(plist_path: Path, png_path: Path, texture_path: str) -> dict:
    unit_id = plist_path.stem
    width, height = png_size(png_path)
    with plist_path.open("rb") as handle:
        document = plistlib.load(handle)
    frames = document.get("frames")
    if not isinstance(frames, dict):
        raise ValueError(f"{plist_path} does not contain a plist frames dictionary")

    sprites_by_animation: dict[str, list[tuple[int, dict]]] = {}
    sprite_entries: list[dict] = []
    for raw_name, frame_data in sorted(frames.items()):
        if not isinstance(frame_data, dict):
            raise ValueError(f"{plist_path}:{raw_name} frame entry is not a dictionary")
        animation_id, frame_index = split_frame_name(unit_id, raw_name)
        frame_x, frame_y, frame_w, frame_h = parse_rect(str(frame_data["frame"]))
        offset_x, offset_y = parse_pair(str(frame_data.get("offset", "{0,0}")))
        sprite_id = f"{animation_id}_{frame_index:03d}"
        sprite = {
            "id": sprite_id,
            "x": frame_x,
            "y": frame_y,
            "w": frame_w,
            "h": frame_h,
            "pivot": [frame_w // 2 + offset_x, frame_h // 2 + offset_y],
        }
        sprite_entries.append(sprite)
        sprites_by_animation.setdefault(animation_id, []).append((frame_index, sprite))

    animations = []
    for animation_id in sorted(sprites_by_animation):
        ordered = [sprite["id"] for _, sprite in sorted(sprites_by_animation[animation_id], key=lambda item: item[0])]
        animations.append(
            {
                "id": animation_id,
                "frames": ordered,
                "frameDuration": FRAME_DURATION_SECONDS,
                "loop": animation_id not in {"death"},
            }
        )

    default_animation = "idle" if "idle" in sprites_by_animation else animations[0]["id"] if animations else ""
    ordered_sprite_ids = [sprite["id"] for sprite in sprite_entries]
    return {
        "atlasName": unit_id,
        "texturePath": texture_path,
        "size": [width, height],
        "sprites": sprite_entries,
        "animations": animations,
        "preview": {
            "defaultAnimation": default_animation,
            "frameCount": len(sprites_by_animation.get(default_animation, [])),
            "orderedSpriteIds": ordered_sprite_ids,
        },
        "provenance": {
            "sourceId": SOURCE_ID,
            "sourceRepo": SOURCE_REPO,
            "sourceUrl": SOURCE_URL,
            "license": LICENSE,
            "sourcePlist": str(plist_path.as_posix()),
            "sourceTexture": str(png_path.as_posix()),
            "promotionStatus": "normalized_not_promoted",
            "exportEligible": False,
        },
    }


def discover_units(source_root: Path, limit: int | None) -> list[tuple[Path, Path]]:
    xml_root = source_root / "Assets" / "Duelyst-Sprites" / "Scripts" / "XMLS"
    png_root = source_root / "Assets" / "Duelyst-Sprites" / "Spritesheets" / "Units"
    if not xml_root.is_dir():
        raise FileNotFoundError(f"Missing Duelyst plist directory: {xml_root}")
    if not png_root.is_dir():
        raise FileNotFoundError(f"Missing Duelyst spritesheet directory: {png_root}")
    units: list[tuple[Path, Path]] = []
    for plist_path in sorted(xml_root.glob("*.plist")):
        png_path = png_root / f"{plist_path.stem}.png"
        if png_path.exists():
            units.append((plist_path, png_path))
        if limit is not None and len(units) >= limit:
            break
    return units


def write_outputs(source_root: Path, output_root: Path, limit: int | None, copy_textures: bool) -> dict:
    atlases_root = output_root / "atlases"
    textures_root = output_root / "textures"
    atlases_root.mkdir(parents=True, exist_ok=True)
    if copy_textures:
        textures_root.mkdir(parents=True, exist_ok=True)

    units = discover_units(source_root, limit)
    generated = []
    for plist_path, png_path in units:
        unit_id = plist_path.stem
        texture_path = f"textures/{png_path.name}" if copy_textures else str(png_path.relative_to(source_root).as_posix())
        atlas = convert_unit(plist_path, png_path, texture_path)
        atlas_path = atlases_root / f"{unit_id}.sprite_atlas.json"
        atlas_path.write_text(json.dumps(atlas, indent=2) + "\n", encoding="utf-8")
        if copy_textures:
            shutil.copy2(png_path, textures_root / png_path.name)
        generated.append(
            {
                "unit_id": unit_id,
                "atlas_path": str(atlas_path.as_posix()),
                "texture_path": texture_path,
                "animation_count": len(atlas["animations"]),
                "sprite_count": len(atlas["sprites"]),
                "default_animation": atlas["preview"]["defaultAnimation"],
                "export_eligible": False,
            }
        )

    report = {
        "schema": "urpg.duelyst_animation_intake_report.v1",
        "source_id": SOURCE_ID,
        "source_repo": SOURCE_REPO,
        "source_url": SOURCE_URL,
        "license": LICENSE,
        "source_root": str(source_root.as_posix()),
        "output_root": str(output_root.as_posix()),
        "unit_count": len(generated),
        "copied_textures": copy_textures,
        "promotion_status": "normalized_not_promoted",
        "export_eligible": False,
        "generated_atlases": generated,
        "notes": [
            "Generated metadata is normalized intake output only.",
            "Do not ship Duelyst-derived textures until release attribution and promotion manifests are approved.",
        ],
    }
    report_path = output_root / "duelyst_animation_intake_report.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", required=True, type=Path, help="Local Unity-Duelyst-Animations checkout")
    parser.add_argument("--output-root", required=True, type=Path, help="URPG normalized output directory")
    parser.add_argument("--limit", type=int, default=None, help="Optional maximum number of units to convert")
    parser.add_argument("--copy-textures", action="store_true", help="Copy PNG spritesheets beside generated metadata")
    args = parser.parse_args()

    report = write_outputs(args.source_root.resolve(), args.output_root.resolve(), args.limit, args.copy_textures)
    print(json.dumps({"unit_count": report["unit_count"], "report": str((args.output_root / "duelyst_animation_intake_report.json").as_posix())}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
