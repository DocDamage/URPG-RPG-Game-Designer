#!/usr/bin/env python3
"""Emit SAM/SAM2-compatible segmentation manifests for offline asset prep."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def output_row(
    source_image: Path, output_root: Path, asset_id: str, manual_override: bool
) -> dict:
    mask_path = output_root / f"{asset_id}_mask.png"
    cutout_path = output_root / f"{asset_id}_cutout.png"
    existed = mask_path.exists() or cutout_path.exists()

    if manual_override and existed:
        state = "skipped"
    elif existed:
        state = "reused"
    else:
        output_root.mkdir(parents=True, exist_ok=True)
        mask_path.write_text(
            f"placeholder mask for {source_image.as_posix()}\n", encoding="utf-8"
        )
        cutout_path.write_text(
            f"placeholder cutout for {source_image.as_posix()}\n",
            encoding="utf-8",
        )
        state = "generated"

    return {
        "asset_id": asset_id,
        "mask_path": mask_path.as_posix(),
        "cutout_path": cutout_path.as_posix(),
        "manual_override": manual_override,
        "reviewed": False,
        "rerun_protection": (
            "preserve_manual_override" if manual_override else "reuse_existing_outputs"
        ),
        "state": state,
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Create an offline segmentation manifest."
    )
    parser.add_argument("--source-image", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--output-root", required=True)
    parser.add_argument("--asset-id", action="append", default=[])
    parser.add_argument("--manual-override", action="store_true")
    args = parser.parse_args()

    source_image = Path(args.source_image)
    output_root = Path(args.output_root)
    asset_ids = args.asset_id or [source_image.stem]
    manifest = {
        "schema": "content/schemas/segmentation_manifest.schema.json",
        "tool_id": "sam2_compatible_segmenter",
        "source_image": source_image.as_posix(),
        "outputs": [
            output_row(source_image, output_root, asset_id, args.manual_override)
            for asset_id in asset_ids
        ],
    }

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote segmentation manifest: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
