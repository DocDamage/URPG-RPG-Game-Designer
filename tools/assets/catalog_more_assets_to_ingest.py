#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import struct
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path


SOURCE_ID = "SRC-010"
SOURCE_ROOT = "imports/raw/more_assets_to_ingest"
RAW_CATALOG = "imports/reports/more_assets_to_ingest/more_assets_to_ingest_catalog.json"
RAW_SHARD_ROOT = "imports/reports/more_assets_to_ingest/more_assets_to_ingest_catalog"
OUTPUT_CATALOG = (
    "imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog.json"
)
OUTPUT_SUMMARY = (
    "imports/reports/asset_intake/more_assets_to_ingest_promotion_summary.json"
)
OUTPUT_SHARD_ROOT = (
    "imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog"
)

CATEGORY_MAP = {
    "audio": "audio",
    "audio_music_or_ambient": "audio/music",
    "background": "backgrounds",
    "binary_or_tool": "tooling/source",
    "font": "fonts",
    "image_uncategorized": "props",
    "license_or_readme": "documentation",
    "map_or_tileset_data": "tilesets",
    "metadata": "documentation",
    "model_3d": "models",
    "nested_archive": "archives",
    "portrait": "characters/portraits",
    "source_art": "tooling/source",
    "sprite_or_character": "characters",
    "tileset": "tilesets",
    "ui": "ui",
    "vfx": "vfx",
}

KIND_MAP = {
    "binary": "tool",
    "map_or_tileset_data": "map",
    "source_art": "source",
    "text_or_metadata": "document",
}


def slug(value: str) -> str:
    normalized = re.sub(r"[^A-Za-z0-9]+", "-", value.lower()).strip("-")
    return normalized[:72] or "asset"


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def png_size(path: Path) -> tuple[int, int] | None:
    try:
        with path.open("rb") as stream:
            header = stream.read(24)
    except OSError:
        return None
    if header.startswith(b"\x89PNG\r\n\x1a\n") and header[12:16] == b"IHDR":
        return struct.unpack(">II", header[16:24])
    return None


def gif_size(path: Path) -> tuple[int, int] | None:
    try:
        with path.open("rb") as stream:
            header = stream.read(10)
    except OSError:
        return None
    if header[:6] in (b"GIF87a", b"GIF89a"):
        return struct.unpack("<HH", header[6:10])
    return None


def image_size(path: Path, ext: str) -> tuple[int, int] | None:
    if ext == "png":
        return png_size(path)
    if ext == "gif":
        return gif_size(path)
    return None


def duplicate_lookup(raw_catalog: dict) -> dict[str, str]:
    duplicates: dict[str, str] = {}
    for group in raw_catalog.get("duplicate_groups", []):
        paths = group.get("paths") or []
        if len(paths) < 2:
            continue
        canonical = paths[0]
        for path in paths[1:]:
            duplicates[path] = canonical
    return duplicates


def preview_kind(media_kind: str) -> str:
    return {
        "archive": "archive",
        "audio": "audio",
        "document": "document",
        "font": "font",
        "image": "image",
        "map": "document",
        "model": "model",
        "source": "document",
        "tool": "tool",
    }.get(media_kind, "")


def asset_tags(
    pack: str,
    filename: str,
    raw_category: str,
    category: str,
    media_kind: str,
    ext: str,
) -> list[str]:
    words: list[str] = []
    for part in [pack, Path(filename).stem, raw_category, category]:
        words.extend(re.findall(r"[A-Za-z0-9]+", part.lower()))
    tags = {
        f"category:{category}",
        f"ext:{ext or 'none'}",
        f"kind:{media_kind}",
        f"pack:{slug(pack)}" if pack else "pack:unknown",
        "local-use",
        "not-release-cleared",
        "src:src-010",
    }
    tags.update(word for word in words if len(word) >= 3)
    return sorted(tags)


def build_catalog(args: argparse.Namespace) -> dict:
    raw_catalog_path = Path(args.raw_catalog)
    raw_shard_root = Path(args.raw_shard_root)
    output_shard_root = Path(args.output_shard_root)
    output_shard_root.mkdir(parents=True, exist_ok=True)

    raw_catalog = read_json(raw_catalog_path)
    duplicate_of = duplicate_lookup(raw_catalog)
    by_category: dict[str, list[dict]] = defaultdict(list)
    kind_counts: Counter[str] = Counter()
    category_counts: Counter[str] = Counter()
    extension_counts: Counter[str] = Counter()
    pack_counts: Counter[str] = Counter()
    records_seen = 0

    for shard_path in sorted(raw_shard_root.glob("*.json")):
        shard = read_json(shard_path)
        raw_category = shard.get("category", shard_path.stem)
        category = CATEGORY_MAP.get(raw_category, raw_category)
        for record in shard.get("records", []):
            source_path = record["source_path"].replace("\\", "/")
            source_file = Path(source_path)
            ext = (record.get("extension") or "").lstrip(".").lower()
            filename = record.get("filename") or source_file.name
            pack = record.get("pack") or ""
            media_kind = KIND_MAP.get(
                record.get("media_kind") or "", record.get("media_kind") or "file"
            )
            sha256 = record.get("sha256") or ""
            short_hash = sha256[:12] if sha256 else f"{records_seen:012d}"
            normalized_path = (
                f"asset://src-010/{category.replace('/', '-')}/"
                f"{slug(Path(filename).stem)}-{short_hash}{('.' + ext) if ext else ''}"
            )
            kind = preview_kind(media_kind)
            preview_path = source_path if kind in {"audio", "image"} else ""
            width_height = (
                image_size(source_file, ext) if media_kind == "image" else None
            )

            asset = {
                "id": f"src-010:{short_hash}",
                "source_id": SOURCE_ID,
                "source_path": source_path,
                "normalized_path": normalized_path,
                "preview_path": preview_path,
                "preview_kind": kind,
                "media_kind": media_kind,
                "category": category,
                "pack": pack,
                "filename": filename,
                "ext": ext,
                "size_bytes": int(record.get("size_bytes") or 0),
                "sha256": sha256,
                "tags": asset_tags(
                    pack, filename, raw_category, category, media_kind, ext
                ),
                "status": "duplicate" if source_path in duplicate_of else "cataloged",
                "export_eligible": False,
                "license": "local_dev_use_only_pending_per_pack_attribution_review",
                "license_status": record.get(
                    "license_status", "pending_per_pack_attribution_review"
                ),
                "promotion_status": "cataloged_local_available",
                "local_use_allowed": True,
                "release_use_allowed": False,
                "notes": (
                    "Local editor/library catalog record for SRC-010; raw file remains "
                    "quarantined and is not release payload."
                ),
            }
            if width_height:
                asset["width"], asset["height"] = width_height
                asset["preview_width"], asset["preview_height"] = width_height
            if source_path in duplicate_of:
                asset["duplicate_of"] = duplicate_of[source_path]

            by_category[category].append(asset)
            kind_counts[media_kind] += 1
            category_counts[category] += 1
            extension_counts[ext or "(none)"] += 1
            pack_counts[pack] += 1
            records_seen += 1

    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    shards = []
    for category, assets in sorted(by_category.items()):
        shard_name = category.replace("/", "-") + ".json"
        shard_path = Path(args.output_shard_root) / shard_name
        shard_rel = f"{args.output_shard_root}/{shard_name}".replace("\\", "/")
        write_json(
            shard_path,
            {
                "schema": "urpg/promoted_asset_catalog_shard/v1",
                "generated_at": generated_at,
                "source_id": SOURCE_ID,
                "source_root": SOURCE_ROOT,
                "category": category,
                "promotion_status": "cataloged_local_available",
                "export_eligible": False,
                "asset_count": len(assets),
                "assets": assets,
            },
        )
        shards.append(
            {"category": category, "path": shard_rel, "asset_count": len(assets)}
        )

    summary = {
        "schema": "urpg/asset_promotion_summary/v1",
        "generated_at": generated_at,
        "source_id": SOURCE_ID,
        "source_root": SOURCE_ROOT,
        "promotion_status": "cataloged_local_available",
        "export_eligible": False,
        "asset_count": records_seen,
        "canonical_asset_count": records_seen - len(duplicate_of),
        "duplicate_group_count": raw_catalog.get("duplicate_group_count", 0),
        "duplicate_asset_count": raw_catalog.get("duplicate_file_count", 0),
        "unsupported_count": 0,
        "kind_counts": dict(sorted(kind_counts.items())),
        "category_counts": dict(sorted(category_counts.items())),
        "extension_counts": dict(sorted(extension_counts.items())),
        "pack_count": len(pack_counts),
        "notes": [
            "All SRC-010 records are catalog-normalized for local editor/library use.",
            "Raw binaries remain in ignored quarantine under imports/raw/more_assets_to_ingest.",
            "normalized_path values are stable virtual asset ids, not copied release payload paths.",
            "local_use_allowed is true for creator/editor browsing; release_use_allowed and export_eligible remain false until curated bundle promotion.",
        ],
    }
    main = {
        "schema": "urpg/promoted_asset_catalog/v1",
        "generated_at": generated_at,
        "source_id": SOURCE_ID,
        "source_root": SOURCE_ROOT,
        "promotion_status": "cataloged_local_available",
        "export_eligible": False,
        "local_use_allowed": True,
        "release_use_allowed": False,
        "summary": summary,
        "shards": shards,
        "duplicate_groups": raw_catalog.get("duplicate_groups", []),
        "unsupported_files": [],
    }
    write_json(Path(args.output_summary), summary)
    write_json(Path(args.output_catalog), main)
    return {
        "asset_count": records_seen,
        "shard_count": len(shards),
        "output": args.output_catalog,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Catalog-normalize SRC-010 for local editor/library use."
    )
    parser.add_argument("--raw-catalog", default=RAW_CATALOG)
    parser.add_argument("--raw-shard-root", default=RAW_SHARD_ROOT)
    parser.add_argument("--output-catalog", default=OUTPUT_CATALOG)
    parser.add_argument("--output-summary", default=OUTPUT_SUMMARY)
    parser.add_argument("--output-shard-root", default=OUTPUT_SHARD_ROOT)
    return parser.parse_args()


def main() -> int:
    result = build_catalog(parse_args())
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
