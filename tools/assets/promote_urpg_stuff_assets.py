#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
from pathlib import Path

IMAGE_EXTS = {"png", "gif", "jpg", "jpeg", "bmp", "webp", "ase", "psd"}
AUDIO_EXTS = {"ogg"}
DOC_EXTS = {"txt", "md", "pdf", "json", "csv"}
SUPPORTED_EXTS = IMAGE_EXTS | AUDIO_EXTS | DOC_EXTS
EXCLUDED_DIRS = {
    ".git",
    ".cache",
    ".venv",
    "__pycache__",
    "build",
    "build-local",
    "node_modules",
    "Testing",
    "__MACOSX",
}
JUNK_NAMES = {".DS_Store", "Thumbs.db", "Desktop.ini"}


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def slugify(value: str) -> str:
    value = re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")
    return value or "asset"


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_image_size(path: Path) -> tuple[int, int] | None:
    ext = path.suffix.lower().lstrip(".")
    try:
        with path.open("rb") as handle:
            if ext == "png":
                data = handle.read(24)
                if len(data) >= 24 and data[:8] == b"\x89PNG\r\n\x1a\n" and data[12:16] == b"IHDR":
                    return struct.unpack(">II", data[16:24])
            if ext == "gif":
                data = handle.read(10)
                if len(data) >= 10 and (data.startswith(b"GIF87a") or data.startswith(b"GIF89a")):
                    return struct.unpack("<HH", data[6:10])
            if ext in {"jpg", "jpeg"}:
                if handle.read(2) != b"\xff\xd8":
                    return None
                while True:
                    marker_prefix = handle.read(1)
                    if not marker_prefix:
                        return None
                    if marker_prefix != b"\xff":
                        continue
                    marker = handle.read(1)
                    while marker == b"\xff":
                        marker = handle.read(1)
                    if not marker:
                        return None
                    code = marker[0]
                    if code in {0xD8, 0xD9}:
                        continue
                    segment_len_data = handle.read(2)
                    if len(segment_len_data) != 2:
                        return None
                    segment_len = struct.unpack(">H", segment_len_data)[0]
                    if code in {0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF}:
                        payload = handle.read(segment_len - 2)
                        if len(payload) >= 5:
                            return struct.unpack(">H", payload[3:5])[0], struct.unpack(">H", payload[1:3])[0]
                        return None
                    handle.seek(segment_len - 2, os.SEEK_CUR)
            if ext == "bmp":
                data = handle.read(26)
                if len(data) >= 26 and data[:2] == b"BM":
                    width, height = struct.unpack("<ii", data[18:26])
                    return abs(width), abs(height)
            if ext == "webp":
                data = handle.read(64)
                if len(data) >= 30 and data[:4] == b"RIFF" and data[8:12] == b"WEBP" and data[12:16] == b"VP8X":
                    width = 1 + int.from_bytes(data[24:27], "little")
                    height = 1 + int.from_bytes(data[27:30], "little")
                    return width, height
    except OSError:
        return None
    return None


def ffprobe_duration_ms(path: Path) -> int | None:
    ffprobe = shutil.which("ffprobe") or shutil.which("ffprobe.exe")
    if not ffprobe:
        return None
    try:
        out = subprocess.check_output(
            [
                ffprobe,
                "-v",
                "error",
                "-show_entries",
                "format=duration",
                "-of",
                "default=noprint_wrappers=1:nokey=1",
                str(path),
            ],
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
        return int(float(out) * 1000.0) if out else None
    except Exception:
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
    if ext in DOC_EXTS:
        return "document"
    return "unsupported"


def infer_pack(path_rel: str, source_root: str) -> str:
    parts = path_rel.split("/")
    root_parts = source_root.split("/")
    local_parts = parts[len(root_parts) :]
    if len(local_parts) >= 2:
        return local_parts[1] if local_parts[0] in {"audio", "side scroller stuff"} else local_parts[0]
    if local_parts:
        return local_parts[0]
    return "urpg_stuff"


def infer_category(path_rel: str, kind: str) -> str:
    lower = path_rel.lower()
    if kind == "audio":
        if "dialogue" in lower or "voice" in lower:
            return "audio/dialogue"
        if "/ui" in lower or "soundpack" in lower or "button" in lower:
            return "audio/ui"
        if "music" in lower or "bgm" in lower:
            return "audio/music"
        return "audio/sfx"
    if kind == "document":
        return "documentation"
    if "spells" in lower or "vfx" in lower or "effect" in lower or "magic" in lower:
        return "vfx"
    if "side scroller" in lower or any(token in lower for token in ("idle", "walk", "attack", "hurt", "dead", "jump", "run")):
        return "characters"
    if "tile" in lower or "tileset" in lower or "terrain" in lower or "environment" in lower:
        return "tilesets"
    if "/ui" in lower or "button" in lower or "icon" in lower or "hud" in lower:
        return "ui"
    if "background" in lower or "parallax" in lower:
        return "backgrounds"
    return "props"


def build_tags(path: Path, path_rel: str, kind: str, category: str, pack: str) -> list[str]:
    tags = {f"kind:{kind}", f"category:{category.replace('/', '-')}", f"pack:{slugify(pack)}", f"ext:{path.suffix.lower().lstrip('.')}"}
    for token in re.findall(r"[a-z0-9]{3,}", path_rel.lower()):
        if token not in {"png", "ogg", "assets", "asset", "stuff", "urpg", "raw", "imports"}:
            tags.add(token)
        if len(tags) >= 18:
            break
    return sorted(tags)


def canonical_score(asset: dict) -> tuple[int, int, str]:
    path = asset["source_path"]
    duplicate_penalty = 1 if "/mp3/" in path.lower() or "/wav/" in path.lower() else 0
    return duplicate_penalty, path.count("/"), path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Catalog-normalize the local SRC-007 URPG asset drop.")
    parser.add_argument("--repo-root", default=".", help="Repository root.")
    parser.add_argument("--source-root", default="imports/raw/urpg_stuff", help="Raw local source root.")
    parser.add_argument(
        "--catalog",
        default="imports/reports/asset_intake/urpg_stuff_promotion_catalog.json",
        help="Full promoted catalog report path.",
    )
    parser.add_argument(
        "--summary",
        default="imports/reports/asset_intake/urpg_stuff_promotion_summary.json",
        help="Small summary report path.",
    )
    parser.add_argument("--source-id", default="SRC-007")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    source_root = (repo_root / args.source_root).resolve()
    if not source_root.is_dir():
        raise SystemExit(f"source root not found: {source_root}")

    source_root_rel = rel(source_root, repo_root)
    assets: list[dict] = []
    extension_counts: dict[str, int] = {}
    kind_counts: dict[str, int] = {}
    category_counts: dict[str, int] = {}
    unsupported: list[str] = []

    for path in iter_files(source_root):
        ext = path.suffix.lower().lstrip(".")
        extension_counts[ext or "(none)"] = extension_counts.get(ext or "(none)", 0) + 1
        if ext not in SUPPORTED_EXTS:
            unsupported.append(rel(path, repo_root))
            continue

        path_rel = rel(path, repo_root)
        stat = path.stat()
        digest = sha256_file(path)
        kind = media_kind(ext)
        category = infer_category(path_rel, kind)
        pack = infer_pack(path_rel, source_root_rel)
        record = {
            "id": f"{args.source_id.lower()}:{digest[:16]}",
            "source_id": args.source_id,
            "source_path": path_rel,
            "normalized_path": f"asset://{args.source_id.lower()}/{category}/{slugify(path.stem)}-{digest[:12]}.{ext}",
            "preview_path": path_rel if kind in {"image", "audio"} else None,
            "preview_kind": kind if kind in {"image", "audio"} else "metadata",
            "media_kind": kind,
            "category": category,
            "pack": pack,
            "filename": path.name,
            "ext": ext,
            "size_bytes": stat.st_size,
            "sha256": digest,
            "tags": build_tags(path, path_rel, kind, category, pack),
            "status": "cataloged",
            "export_eligible": False,
            "license": "user_attested_free_for_game_use_pending_per_pack_attribution",
        }
        if kind == "image":
            size = read_image_size(path)
            if size:
                record["width"], record["height"] = size
        if kind == "audio":
            duration = ffprobe_duration_ms(path)
            if duration is not None:
                record["duration_ms"] = duration
        assets.append(record)
        kind_counts[kind] = kind_counts.get(kind, 0) + 1
        category_counts[category] = category_counts.get(category, 0) + 1

    by_hash: dict[str, list[dict]] = {}
    for asset in assets:
        by_hash.setdefault(asset["sha256"], []).append(asset)

    duplicate_groups = []
    duplicate_assets = 0
    for digest, group in sorted(by_hash.items()):
        if len(group) < 2:
            continue
        sorted_group = sorted(group, key=canonical_score)
        canonical = sorted_group[0]
        duplicate_groups.append(
            {
                "sha256": digest,
                "canonical_asset_id": canonical["id"],
                "canonical_source_path": canonical["source_path"],
                "copies": len(group),
                "duplicate_source_paths": [asset["source_path"] for asset in sorted_group[1:]],
            }
        )
        for duplicate in sorted_group[1:]:
            duplicate["status"] = "duplicate"
            duplicate["duplicate_of"] = canonical["id"]
            duplicate_assets += 1

    assets.sort(key=lambda asset: asset["source_path"])
    generated_at = iso_now()
    summary = {
        "schema": "urpg/asset_promotion_summary/v1",
        "generated_at": generated_at,
        "source_id": args.source_id,
        "source_root": source_root_rel,
        "promotion_status": "cataloged_local",
        "export_eligible": False,
        "asset_count": len(assets),
        "canonical_asset_count": len(assets) - duplicate_assets,
        "duplicate_group_count": len(duplicate_groups),
        "duplicate_asset_count": duplicate_assets,
        "unsupported_count": len(unsupported),
        "kind_counts": dict(sorted(kind_counts.items())),
        "category_counts": dict(sorted(category_counts.items())),
        "extension_counts": dict(sorted(extension_counts.items())),
        "notes": [
            "All supported SRC-007 files were catalog-normalized for local editor/library use.",
            "Binary payloads remain in the local raw intake root; normalized_path values are stable virtual asset ids.",
            "export_eligible remains false until curated subsets receive per-pack attribution and bundle promotion.",
        ],
    }
    catalog = {
        "schema": "urpg/promoted_asset_catalog/v1",
        "generated_at": generated_at,
        "source_id": args.source_id,
        "source_root": source_root_rel,
        "promotion_status": "cataloged_local",
        "export_eligible": False,
        "summary": summary,
        "assets": assets,
        "duplicate_groups": duplicate_groups,
        "unsupported_files": unsupported,
    }

    catalog_path = (repo_root / args.catalog).resolve()
    summary_path = (repo_root / args.summary).resolve()
    catalog_path.parent.mkdir(parents=True, exist_ok=True)
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    catalog_path.write_text(json.dumps(catalog, indent=2), encoding="utf-8")
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
