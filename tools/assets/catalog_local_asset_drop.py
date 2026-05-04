#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import struct
from collections import Counter, defaultdict
from pathlib import Path

IMAGE_EXTS = {
    "png",
    "gif",
    "jpg",
    "jpeg",
    "bmp",
    "webp",
    "ase",
    "aseprite",
    "psd",
    "svg",
}
AUDIO_EXTS = {"ogg", "wav", "mp3", "flac", "m4a", "opus"}
ARCHIVE_EXTS = {"zip", "rar", "7z", "tar", "gz", "bz2", "xz"}
FONT_EXTS = {"ttf", "otf"}
MODEL_EXTS = {"obj", "fbx", "glb", "gltf", "blend", "vox", "mtl", "mat", "ply"}
MAP_EXTS = {"tmx", "tsx", "tiled-project", "world"}
DOC_EXTS = {"txt", "md", "pdf", "rtf", "csv", "license", "json", "xml", "yaml", "yml"}
SOURCE_EXTS = {"py", "js", "ts", "gd", "lua", "sh", "bat", "cmd", "cpp", "h", "hpp"}
SUPPORTED_EXTS = (
    IMAGE_EXTS
    | AUDIO_EXTS
    | ARCHIVE_EXTS
    | FONT_EXTS
    | MODEL_EXTS
    | MAP_EXTS
    | DOC_EXTS
    | SOURCE_EXTS
)
EXCLUDED_DIRS = {".git", "__MACOSX", "__pycache__", "node_modules", ".cache", ".venv"}
JUNK_NAMES = {".DS_Store", "Thumbs.db", "Desktop.ini"}


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()


def slug(value: str, fallback: str = "asset") -> str:
    value = re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")
    return value[:96] or fallback


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def image_size(path: Path, ext: str) -> tuple[int, int] | None:
    try:
        with path.open("rb") as stream:
            if ext == "png":
                data = stream.read(24)
                if data.startswith(b"\x89PNG\r\n\x1a\n") and data[12:16] == b"IHDR":
                    return struct.unpack(">II", data[16:24])
            if ext == "gif":
                data = stream.read(10)
                if data[:6] in (b"GIF87a", b"GIF89a"):
                    return struct.unpack("<HH", data[6:10])
            if ext == "bmp":
                data = stream.read(26)
                if len(data) >= 26 and data[:2] == b"BM":
                    width, height = struct.unpack("<ii", data[18:26])
                    return abs(width), abs(height)
    except OSError:
        return None
    return None


def iter_files(root: Path):
    for current, dirs, files in os.walk(root, topdown=True, followlinks=False):
        dirs[:] = [d for d in dirs if d not in EXCLUDED_DIRS]
        base = Path(current)
        for name in files:
            if name in JUNK_NAMES or name.startswith("._"):
                continue
            yield base / name


def media_kind(ext: str) -> str:
    if ext in IMAGE_EXTS:
        return "image"
    if ext in AUDIO_EXTS:
        return "audio"
    if ext in ARCHIVE_EXTS:
        return "archive"
    if ext in FONT_EXTS:
        return "font"
    if ext in MODEL_EXTS:
        return "model"
    if ext in MAP_EXTS:
        return "map"
    if ext in DOC_EXTS:
        return "document"
    if ext in SOURCE_EXTS:
        return "source"
    return "file"


def infer_category(path_rel: str, kind: str) -> str:
    lower = path_rel.lower()
    name = Path(path_rel).name.lower()
    if kind == "audio":
        if any(term in lower for term in ["music", "bgm", "ambient", "loop"]):
            return "audio/music"
        return "audio"
    if kind == "archive":
        return "archives"
    if kind == "font":
        return "fonts"
    if kind == "model":
        return "models"
    if kind == "map":
        return "tilesets"
    if kind in {"document", "source"}:
        if any(
            term in name for term in ["license", "readme", "credits", "attribution"]
        ):
            return "documentation"
        return "tooling/source" if kind == "source" else "documentation"
    if any(
        term in lower
        for term in ["tileset", "tile", "terrain", "dungeon", "outside_", "inside_"]
    ):
        return "tilesets"
    if any(
        term in lower
        for term in ["character", "sprite", "hero", "npc", "enemy", "battler", "actor"]
    ):
        return "characters"
    if any(term in lower for term in ["portrait", "face", "bust"]):
        return "characters/portraits"
    if any(
        term in lower
        for term in ["ui", "interface", "icon", "icons", "button", "panel", "window"]
    ):
        return "ui"
    if any(
        term in lower
        for term in [
            "vfx",
            "fx",
            "effect",
            "animation",
            "slash",
            "impact",
            "fire",
            "lightning",
        ]
    ):
        return "vfx"
    if any(
        term in lower for term in ["background", "parallax", "forest", "sky", "cave"]
    ):
        return "backgrounds"
    return "props"


def pack_name(path: Path, source_root: Path) -> str:
    rel_parts = path.relative_to(source_root).parts
    if not rel_parts:
        return source_root.name
    if rel_parts[0] == "__archive_extracted" and len(rel_parts) > 1:
        return rel_parts[1]
    if len(rel_parts) > 1:
        return rel_parts[0]
    return source_root.name


def tags_for(
    source_id: str, category: str, kind: str, ext: str, pack: str, filename: str
) -> list[str]:
    tags = {
        f"category:{category}",
        f"ext:{ext or 'none'}",
        f"kind:{kind}",
        f"pack:{slug(pack, 'unknown')}",
        f"src:{source_id.lower()}",
        "local-use",
        "not-release-cleared",
    }
    words: list[str] = []
    for value in [category, pack, Path(filename).stem]:
        words.extend(re.findall(r"[a-z0-9]+", value.lower()))
    tags.update(word for word in words if len(word) >= 3)
    return sorted(tags)


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def chunked_assets(records: list[dict], max_assets: int) -> list[list[dict]]:
    if max_assets <= 0 or len(records) <= max_assets:
        return [records]
    return [
        records[index : index + max_assets]
        for index in range(0, len(records), max_assets)
    ]


def shard_name(category: str, part: int, total_parts: int) -> str:
    base = category.replace("/", "-")
    if total_parts <= 1:
        return f"{base}.json"
    return f"{base}-part-{part:03d}.json"


def build_catalog(args: argparse.Namespace) -> dict:
    repo_root = Path(args.repo_root).resolve()
    source_root = (repo_root / args.source_root).resolve()
    if not source_root.is_dir():
        raise SystemExit(f"source root not found: {source_root}")

    source_root_rel = rel(source_root, repo_root)
    generated_at = now_utc()
    records: list[dict] = []
    unsupported: list[str] = []
    by_hash: dict[str, list[int]] = defaultdict(list)

    for path in iter_files(source_root):
        path_rel = rel(path, repo_root)
        ext = path.suffix.lower().lstrip(".")
        if ext not in SUPPORTED_EXTS:
            unsupported.append(path_rel)
            continue
        digest = sha256_file(path)
        stat = path.stat()
        kind = media_kind(ext)
        category = infer_category(path_rel, kind)
        pack = pack_name(path, source_root)
        short_hash = digest[:12]
        normalized_path = (
            f"asset://{args.source_id.lower()}/{category.replace('/', '-')}/"
            f"{slug(path.stem)}-{short_hash}{('.' + ext) if ext else ''}"
        )
        preview_kind = (
            kind
            if kind in {"audio", "image", "font", "model", "archive"}
            else "document"
        )
        preview_path = path_rel if kind in {"audio", "image"} else ""
        record = {
            "id": f"{args.source_id.lower()}:{short_hash}:{len(by_hash[digest])}",
            "source_id": args.source_id,
            "source_path": path_rel,
            "normalized_path": normalized_path,
            "preview_path": preview_path,
            "preview_kind": preview_kind,
            "media_kind": kind,
            "category": category,
            "pack": pack,
            "filename": path.name,
            "ext": ext,
            "size_bytes": stat.st_size,
            "sha256": digest,
            "tags": tags_for(args.source_id, category, kind, ext, pack, path.name),
            "status": "cataloged",
            "export_eligible": False,
            "local_use_allowed": True,
            "release_use_allowed": False,
            "license": "local_dev_use_only_pending_per_pack_attribution_review",
            "license_status": "pending_per_pack_attribution_review",
            "promotion_status": "cataloged_local_available",
            "notes": (
                f"Local editor/library catalog record for {args.source_id}; raw file "
                "remains quarantined and is not release payload."
            ),
        }
        size = image_size(path, ext) if kind == "image" else None
        if size:
            record["width"], record["height"] = size
            record["preview_width"], record["preview_height"] = size
        by_hash[digest].append(len(records))
        records.append(record)

    duplicate_groups: list[dict] = []
    duplicate_asset_count = 0
    for digest, indexes in sorted(by_hash.items()):
        if len(indexes) < 2:
            continue
        canonical = records[indexes[0]]["source_path"]
        paths = [records[index]["source_path"] for index in indexes]
        duplicate_groups.append(
            {
                "sha256": digest,
                "canonical_path": canonical,
                "paths": paths,
                "duplicate_count": len(paths) - 1,
            }
        )
        duplicate_asset_count += len(paths) - 1
        for index in indexes[1:]:
            records[index]["status"] = "duplicate"
            records[index]["duplicate_of"] = canonical

    by_category: dict[str, list[dict]] = defaultdict(list)
    kind_counts: Counter[str] = Counter()
    category_counts: Counter[str] = Counter()
    extension_counts: Counter[str] = Counter()
    pack_counts: Counter[str] = Counter()
    for record in records:
        by_category[record["category"]].append(record)
        kind_counts[record["media_kind"]] += 1
        category_counts[record["category"]] += 1
        extension_counts[record["ext"] or "(none)"] += 1
        pack_counts[record["pack"]] += 1

    output_shard_root = repo_root / args.output_shard_root
    if output_shard_root.exists():
        for old_file in output_shard_root.glob("*.json"):
            old_file.unlink()
    shards: list[dict] = []
    for category, assets in sorted(by_category.items()):
        parts = chunked_assets(assets, args.max_shard_assets)
        for part_index, part_assets in enumerate(parts, start=1):
            filename = shard_name(category, part_index, len(parts))
            shard_path = output_shard_root / filename
            shard_rel = f"{args.output_shard_root}/{filename}".replace("\\", "/")
            part_label = None if len(parts) <= 1 else f"{part_index}/{len(parts)}"
            write_json(
                shard_path,
                {
                    "schema": "urpg/promoted_asset_catalog_shard/v1",
                    "generated_at": generated_at,
                    "source_id": args.source_id,
                    "source_root": source_root_rel,
                    "category": category,
                    "promotion_status": "cataloged_local_available",
                    "export_eligible": False,
                    "asset_count": len(part_assets),
                    "category_asset_count": len(assets),
                    "part": part_label,
                    "assets": part_assets,
                },
            )
            shards.append(
                {
                    "category": category,
                    "path": shard_rel,
                    "asset_count": len(part_assets),
                    "category_asset_count": len(assets),
                    "part": part_label,
                }
            )

    summary = {
        "schema": "urpg/asset_promotion_summary/v1",
        "generated_at": generated_at,
        "source_id": args.source_id,
        "source_root": source_root_rel,
        "promotion_status": "cataloged_local_available",
        "export_eligible": False,
        "asset_count": len(records),
        "canonical_asset_count": len(records) - duplicate_asset_count,
        "duplicate_group_count": len(duplicate_groups),
        "duplicate_asset_count": duplicate_asset_count,
        "unsupported_count": len(unsupported),
        "kind_counts": dict(sorted(kind_counts.items())),
        "category_counts": dict(sorted(category_counts.items())),
        "extension_counts": dict(sorted(extension_counts.items())),
        "pack_count": len(pack_counts),
        "notes": [
            f"All supported {args.source_id} records are catalog-normalized for local editor/library use.",
            f"Raw binaries remain in ignored quarantine under {source_root_rel}.",
            "normalized_path values are stable virtual asset ids, not copied release payload paths.",
            "local_use_allowed is true for creator/editor browsing; release_use_allowed and export_eligible remain false until curated bundle promotion.",
        ],
    }
    catalog = {
        "schema": "urpg/promoted_asset_catalog/v1",
        "generated_at": generated_at,
        "source_id": args.source_id,
        "source_root": source_root_rel,
        "promotion_status": "cataloged_local_available",
        "export_eligible": False,
        "local_use_allowed": True,
        "release_use_allowed": False,
        "summary": summary,
        "shards": shards,
        "duplicate_groups": duplicate_groups,
        "unsupported_files": unsupported,
    }
    write_json(repo_root / args.output_summary, summary)
    write_json(repo_root / args.output_catalog, catalog)
    return {
        "source_id": args.source_id,
        "asset_count": len(records),
        "canonical_asset_count": summary["canonical_asset_count"],
        "duplicate_group_count": len(duplicate_groups),
        "duplicate_asset_count": duplicate_asset_count,
        "unsupported_count": len(unsupported),
        "shard_count": len(shards),
        "output": args.output_catalog,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Catalog-normalize a local raw asset drop for editor/library use."
    )
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--source-id", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--output-catalog", required=True)
    parser.add_argument("--output-summary", required=True)
    parser.add_argument("--output-shard-root", required=True)
    parser.add_argument(
        "--max-shard-assets",
        type=int,
        default=5000,
        help="Maximum asset records per category shard file. Use 0 to disable chunking.",
    )
    return parser.parse_args()


def main() -> int:
    print(json.dumps(build_catalog(parse_args()), indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
