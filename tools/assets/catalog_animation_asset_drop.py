#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import struct
from pathlib import Path

IMAGE_EXTS = {".png", ".jpg", ".jpeg", ".webp", ".gif", ".bmp"}
ARCHIVE_EXTS = {".zip", ".7z", ".rar"}
DOC_EXTS = {".docx", ".pdf", ".txt", ".md", ".rtf"}
SOURCE_EXTS = {".lua", ".json", ".xml", ".csv"}
SKIP_DIRS = {".git", "__MACOSX", "__pycache__"}


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def slugify(value: str) -> str:
    value = re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")
    return value or "asset"


def sha256_file(path: Path, limit_bytes: int | None = None) -> str:
    digest = hashlib.sha256()
    remaining = limit_bytes
    with path.open("rb") as handle:
        while True:
            size = 1024 * 1024 if remaining is None else min(1024 * 1024, remaining)
            if size <= 0:
                break
            chunk = handle.read(size)
            if not chunk:
                break
            digest.update(chunk)
            if remaining is not None:
                remaining -= len(chunk)
    return digest.hexdigest()


def png_size(path: Path) -> tuple[int, int] | None:
    try:
        with path.open("rb") as handle:
            data = handle.read(24)
        if (
            len(data) >= 24
            and data[:8] == b"\x89PNG\r\n\x1a\n"
            and data[12:16] == b"IHDR"
        ):
            return struct.unpack(">II", data[16:24])
    except OSError:
        return None
    return None


def infer_category(path: Path) -> str:
    lower = str(path).replace("\\", "/").lower()
    if "background" in lower or "map" in lower:
        return "backgrounds"
    if "sideview" in lower or "side_view" in lower or "sideviewbattler" in lower:
        return "characters/sideview"
    if (
        "isometric" in lower
        or "packed_" in lower
        or "separated" in lower
        or "assembled" in lower
    ):
        return "characters/isometric"
    if (
        "tree" in lower
        or "drone" in lower
        or "fox" in lower
        or "beast" in lower
        or "mummy" in lower
    ):
        return "characters/isometric"
    if "doc" in lower or "read me" in lower or "reference" in lower:
        return "documentation"
    return "characters"


def iter_files(root: Path):
    for current, dirs, files in os.walk(root, topdown=True, followlinks=False):
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
        base = Path(current)
        for name in files:
            if name.startswith("._") or name in {
                "Thumbs.db",
                ".DS_Store",
                "Desktop.ini",
            }:
                continue
            yield base / name


def sequence_signature(files: list[Path]) -> str:
    sample = []
    for path in files[:3] + files[-3:]:
        try:
            stat = path.stat()
        except OSError:
            continue
        sample.append(f"{path.name}:{stat.st_size}")
    return "|".join([str(len(files)), *sample])


def make_record(
    source_id: str,
    source_root: Path,
    repo_root: Path,
    path: Path,
    kind: str,
    category: str,
    preview: Path | None,
    file_count: int,
    total_bytes: int,
    extra: dict,
) -> dict:
    path_rel = rel(path, repo_root)
    digest_basis = f"{source_id}:{path_rel}:{kind}:{file_count}:{total_bytes}:{extra.get('fingerprint', '')}"
    asset_id = f"{source_id.lower()}:{hashlib.sha256(digest_basis.encode('utf-8')).hexdigest()[:16]}"
    tags = {
        f"kind:{kind}",
        f"category:{category.replace('/', '-')}",
        f"pack:{slugify(path.relative_to(source_root).parts[0] if path != source_root else path.name)}",
    }
    for token in re.findall(r"[a-z0-9]{3,}", path_rel.lower()):
        if token not in {
            "png",
            "assets",
            "asset",
            "ingest",
            "imports",
            "raw",
            "urpg",
            "frames",
        }:
            tags.add(token)
        if len(tags) >= 20:
            break
    record = {
        "id": asset_id,
        "source_id": source_id,
        "source_path": path_rel,
        "normalized_path": f"asset://{source_id.lower()}/{category}/{slugify(path.name)}-{asset_id.split(':')[1]}",
        "preview_path": rel(preview, repo_root) if preview else None,
        "preview_kind": "image" if preview else "metadata",
        "media_kind": kind,
        "category": category,
        "pack": path.relative_to(source_root).parts[0]
        if path != source_root
        else path.name,
        "filename": path.name,
        "file_count": file_count,
        "size_bytes": total_bytes,
        "tags": sorted(tags),
        "status": "cataloged",
        "export_eligible": False,
        "license": "user_attested_free_for_game_use_pending_per_pack_attribution",
    }
    record.update(extra)
    return record


def build_catalog(
    repo_root: Path, source_root: Path, source_id: str
) -> tuple[dict, dict, dict[str, list[dict]]]:
    files_by_dir: dict[Path, list[Path]] = {}
    extension_counts: dict[str, int] = {}
    total_files = 0
    total_bytes = 0
    for path in iter_files(source_root):
        try:
            stat = path.stat()
        except OSError:
            continue
        total_files += 1
        total_bytes += stat.st_size
        extension_counts[path.suffix.lower() or "(none)"] = (
            extension_counts.get(path.suffix.lower() or "(none)", 0) + 1
        )
        files_by_dir.setdefault(path.parent, []).append(path)

    records: list[dict] = []
    image_groups: dict[tuple[Path, str], dict] = {}
    for directory, files in sorted(
        files_by_dir.items(), key=lambda item: rel(item[0], repo_root)
    ):
        image_files = sorted(
            [p for p in files if p.suffix.lower() in IMAGE_EXTS],
            key=lambda p: p.name.lower(),
        )
        archive_files = sorted(
            [p for p in files if p.suffix.lower() in ARCHIVE_EXTS],
            key=lambda p: p.name.lower(),
        )
        doc_files = sorted(
            [p for p in files if p.suffix.lower() in DOC_EXTS],
            key=lambda p: p.name.lower(),
        )
        source_files = sorted(
            [p for p in files if p.suffix.lower() in SOURCE_EXTS],
            key=lambda p: p.name.lower(),
        )

        if image_files:
            bytes_sum = sum(p.stat().st_size for p in image_files)
            preview = image_files[0]
            dimensions = png_size(preview) if preview.suffix.lower() == ".png" else None
            signature = sequence_signature(image_files)
            category = infer_category(directory)
            local_parts = directory.relative_to(source_root).parts
            pack_root = source_root / local_parts[0] if local_parts else directory
            group = image_groups.setdefault(
                (pack_root, category),
                {
                    "file_count": 0,
                    "total_bytes": 0,
                    "sequence_count": 0,
                    "preview": preview,
                    "preview_dimensions": dimensions,
                    "representative_sequences": [],
                    "fingerprint_parts": [],
                },
            )
            group["file_count"] += len(image_files)
            group["total_bytes"] += bytes_sum
            group["sequence_count"] += 1
            group["fingerprint_parts"].append(
                f"{rel(directory, repo_root)}:{signature}"
            )
            if len(group["representative_sequences"]) < 24:
                group["representative_sequences"].append(
                    {
                        "path": rel(directory, repo_root),
                        "frame_count": len(image_files),
                        "representative_files": [
                            rel(p, repo_root) for p in image_files[:3]
                        ],
                    }
                )
            if group["preview_dimensions"] is None and dimensions:
                group["preview"] = preview
                group["preview_dimensions"] = dimensions

        for path in archive_files:
            stat = path.stat()
            records.append(
                make_record(
                    source_id,
                    source_root,
                    repo_root,
                    path,
                    "archive",
                    "archives",
                    None,
                    1,
                    stat.st_size,
                    {"sha256": sha256_file(path)},
                )
            )
        for path in doc_files:
            stat = path.stat()
            records.append(
                make_record(
                    source_id,
                    source_root,
                    repo_root,
                    path,
                    "document",
                    "documentation",
                    None,
                    1,
                    stat.st_size,
                    {"sha256_head": sha256_file(path, 1024 * 1024)},
                )
            )
        if source_files:
            bytes_sum = sum(p.stat().st_size for p in source_files)
            records.append(
                make_record(
                    source_id,
                    source_root,
                    repo_root,
                    directory,
                    "animation_metadata",
                    "tooling/source",
                    None,
                    len(source_files),
                    bytes_sum,
                    {"metadata_files": [rel(p, repo_root) for p in source_files[:12]]},
                )
            )

    sequence_groups: dict[str, list[dict]] = {}
    for (pack_root, category), group in sorted(
        image_groups.items(), key=lambda item: (rel(item[0][0], repo_root), item[0][1])
    ):
        fingerprint = hashlib.sha256(
            "\n".join(sorted(group["fingerprint_parts"])).encode("utf-8")
        ).hexdigest()
        extra = {
            "sequence_count": group["sequence_count"],
            "frame_count": group["file_count"],
            "fingerprint": fingerprint,
            "representative_sequences": group["representative_sequences"],
        }
        dimensions = group["preview_dimensions"]
        if dimensions:
            extra["preview_width"], extra["preview_height"] = dimensions
        record = make_record(
            source_id,
            source_root,
            repo_root,
            pack_root,
            "image_sequence_collection",
            category,
            group["preview"],
            group["file_count"],
            group["total_bytes"],
            extra,
        )
        records.append(record)
        sequence_groups.setdefault(
            f"{category}:{group['total_bytes']}:{fingerprint}", []
        ).append(record)

    duplicate_groups = []
    duplicate_asset_count = 0
    for _key, group in sorted(sequence_groups.items()):
        if len(group) < 2:
            continue
        canonical = sorted(
            group, key=lambda item: (item["file_count"], item["source_path"])
        )[0]
        duplicate_groups.append(
            {
                "kind": "potential_sequence_duplicate",
                "canonical_asset_id": canonical["id"],
                "canonical_source_path": canonical["source_path"],
                "copies": len(group),
                "duplicate_source_paths": [
                    item["source_path"]
                    for item in group
                    if item["id"] != canonical["id"]
                ],
            }
        )
        duplicate_asset_count += len(group) - 1

    category_counts: dict[str, int] = {}
    kind_counts: dict[str, int] = {}
    for record in records:
        category_counts[record["category"]] = (
            category_counts.get(record["category"], 0) + 1
        )
        kind_counts[record["media_kind"]] = kind_counts.get(record["media_kind"], 0) + 1

    generated_at = iso_now()
    summary = {
        "schema": "urpg/aggregate_animation_asset_intake_summary/v1",
        "generated_at": generated_at,
        "source_id": source_id,
        "source_root": rel(source_root, repo_root),
        "promotion_status": "cataloged_local_aggregate",
        "export_eligible": False,
        "raw_file_count": total_files,
        "raw_size_bytes": total_bytes,
        "asset_record_count": len(records),
        "potential_duplicate_group_count": len(duplicate_groups),
        "potential_duplicate_asset_count": duplicate_asset_count,
        "kind_counts": dict(sorted(kind_counts.items())),
        "category_counts": dict(sorted(category_counts.items())),
        "extension_counts": dict(sorted(extension_counts.items())),
        "notes": [
            "This aggregate catalog represents large frame folders as animation/background sequence assets.",
            "Raw files remain local under ignored intake storage and are not committed.",
            "Potential duplicate groups are directory-sequence fingerprints, not full byte-for-byte duplicate proof.",
            "Release promotion still requires per-pack attribution and curated bundle manifests.",
        ],
    }
    catalog = {
        "schema": "urpg/promoted_asset_catalog/v1",
        "generated_at": generated_at,
        "source_id": source_id,
        "source_root": rel(source_root, repo_root),
        "promotion_status": "cataloged_local_aggregate",
        "export_eligible": False,
        "summary": summary,
        "shards": [],
        "duplicate_groups": duplicate_groups,
    }
    shards: dict[str, list[dict]] = {}
    for record in sorted(records, key=lambda item: item["source_path"]):
        shards.setdefault(record["category"], []).append(record)
    return catalog, summary, shards


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Aggregate-catalog a large animation/background asset drop."
    )
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--source-id", default="SRC-008")
    parser.add_argument("--catalog", required=True)
    parser.add_argument("--summary", required=True)
    parser.add_argument("--shard-dir", required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    source_root = (repo_root / args.source_root).resolve()
    if not source_root.is_dir():
        raise SystemExit(f"source root not found: {source_root}")

    catalog, summary, shards = build_catalog(repo_root, source_root, args.source_id)
    shard_root = (repo_root / args.shard_dir).resolve()
    shard_root.mkdir(parents=True, exist_ok=True)
    for old in shard_root.glob("*.json"):
        old.unlink()
    shard_rows = []
    for category, records in sorted(shards.items()):
        shard_path = shard_root / f"{slugify(category)}.json"
        shard = {
            "schema": "urpg/promoted_asset_catalog_shard/v1",
            "generated_at": catalog["generated_at"],
            "source_id": args.source_id,
            "source_root": catalog["source_root"],
            "category": category,
            "promotion_status": "cataloged_local_aggregate",
            "export_eligible": False,
            "asset_count": len(records),
            "assets": records,
        }
        shard_path.write_text(json.dumps(shard, indent=2), encoding="utf-8")
        shard_rows.append(
            {
                "category": category,
                "path": rel(shard_path, repo_root),
                "asset_count": len(records),
            }
        )
    catalog["shards"] = shard_rows
    (repo_root / args.catalog).write_text(
        json.dumps(catalog, indent=2), encoding="utf-8"
    )
    (repo_root / args.summary).write_text(
        json.dumps(summary, indent=2), encoding="utf-8"
    )
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
