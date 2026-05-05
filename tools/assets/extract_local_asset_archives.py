#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import shutil
import subprocess
from pathlib import Path


ARCHIVE_EXTS = {".zip", ".rar", ".7z"}


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()


def slug(value: str, fallback: str = "archive") -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-") or fallback


def rel(path: Path, root: Path) -> str:
    return str(path.resolve().relative_to(root)).replace("\\", "/")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def iter_archives(root: Path):
    for current, dirs, files in os.walk(root, topdown=True, followlinks=False):
        dirs[:] = [d for d in dirs if d not in {".git", "__MACOSX", "__pycache__"}]
        base = Path(current)
        for name in files:
            path = base / name
            if path.suffix.lower() in ARCHIVE_EXTS:
                yield path


def count_tree(root: Path) -> tuple[int, int]:
    file_count = 0
    byte_count = 0
    if not root.exists():
        return file_count, byte_count
    for path in root.rglob("*"):
        if path.is_file():
            file_count += 1
            byte_count += path.stat().st_size
    return file_count, byte_count


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract local asset archives into ignored raw quarantine and write an intake report."
    )
    parser.add_argument("--repo-root", default=".", help="Repository root.")
    parser.add_argument("--source-id", required=True, help="Source id such as SRC-014.")
    parser.add_argument("--source-root", required=True, help="Archive source root.")
    parser.add_argument("--extract-root", required=True, help="Ignored raw extraction root.")
    parser.add_argument("--report", required=True, help="JSON extraction report path.")
    parser.add_argument(
        "--extractor",
        default=None,
        help="7-Zip executable. Defaults to 7z.exe/7z/7zz discovered on PATH.",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-extract archives even when the extraction root already contains files.",
    )
    return parser.parse_args()


def resolve_extractor(configured: str | None) -> str | None:
    candidates = [configured] if configured else ["7z.exe", "7z", "7zz"]
    for candidate in candidates:
        if not candidate:
            continue
        resolved = shutil.which(candidate)
        if resolved:
            return resolved
        path = Path(candidate)
        if path.is_file():
            return str(path)
    return None


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    source_root = (repo_root / args.source_root).resolve()
    extract_root = (repo_root / args.extract_root).resolve()
    report_path = (repo_root / args.report).resolve()

    if not source_root.is_dir():
        raise SystemExit(f"source root not found: {source_root}")
    if not str(extract_root).lower().startswith(str(repo_root).lower()):
        raise SystemExit(f"extract root must stay under repo root: {extract_root}")

    extractor = resolve_extractor(args.extractor)
    if not extractor:
        raise SystemExit("7-Zip extractor not found. Install 7-Zip or pass --extractor.")

    extract_archive_root = extract_root / "__archive_extracted"
    extract_archive_root.mkdir(parents=True, exist_ok=True)

    records: list[dict] = []
    summary = {
        "archive_count": 0,
        "extracted_archive_count": 0,
        "already_extracted_count": 0,
        "failed_archive_count": 0,
        "total_archive_bytes": 0,
        "total_extracted_files": 0,
        "total_extracted_bytes": 0,
    }

    for archive in sorted(iter_archives(source_root), key=lambda p: rel(p, repo_root).lower()):
        digest = sha256_file(archive)
        short_hash = digest[:12]
        destination = extract_archive_root / f"{slug(archive.stem)}-{short_hash}"
        existing_files, existing_bytes = count_tree(destination)
        record = {
            "source_id": args.source_id,
            "source_archive": rel(archive, repo_root),
            "archive_kind": archive.suffix.lower().lstrip("."),
            "size_bytes": archive.stat().st_size,
            "sha256": digest,
            "extraction_root": rel(destination, repo_root),
            "extracted": False,
            "skipped_reason": None,
            "error": None,
            "extracted_file_count": 0,
            "extracted_size_bytes": 0,
        }
        summary["archive_count"] += 1
        summary["total_archive_bytes"] += record["size_bytes"]

        if existing_files > 0 and not args.force:
            record["extracted"] = True
            record["skipped_reason"] = "already_extracted"
            record["extracted_file_count"] = existing_files
            record["extracted_size_bytes"] = existing_bytes
            summary["already_extracted_count"] += 1
            summary["total_extracted_files"] += existing_files
            summary["total_extracted_bytes"] += existing_bytes
            records.append(record)
            continue

        destination.mkdir(parents=True, exist_ok=True)
        command = [
            extractor,
            "x",
            "-y",
            "-bd",
            "-bb0",
            f"-o{destination}",
            str(archive),
        ]
        completed = subprocess.run(
            command,
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if completed.returncode != 0:
            record["error"] = completed.stdout[-4000:]
            summary["failed_archive_count"] += 1
            records.append(record)
            continue

        extracted_files, extracted_bytes = count_tree(destination)
        record["extracted"] = True
        record["extracted_file_count"] = extracted_files
        record["extracted_size_bytes"] = extracted_bytes
        summary["extracted_archive_count"] += 1
        summary["total_extracted_files"] += extracted_files
        summary["total_extracted_bytes"] += extracted_bytes
        records.append(record)

    report = {
        "schema": "urpg/local_asset_archive_extraction_report/v1",
        "generated_at": now_utc(),
        "source_id": args.source_id,
        "source_root": rel(source_root, repo_root),
        "extract_root": rel(extract_root, repo_root),
        "extractor": extractor,
        "summary": summary,
        "archives": records,
    }
    write_json(report_path, report)

    print(f"Archive extraction report written: {rel(report_path, repo_root)}")
    print(
        "Archives: {archive_count}, extracted: {extracted_archive_count}, "
        "already present: {already_extracted_count}, failed: {failed_archive_count}, "
        "files: {total_extracted_files}".format(**summary)
    )
    return 0 if summary["failed_archive_count"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
