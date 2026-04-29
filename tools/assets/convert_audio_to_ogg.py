#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import subprocess
from pathlib import Path

AUDIO_EXTENSIONS = {".wav", ".mp3", ".flac", ".aiff", ".aif", ".m4a", ".aac", ".wma"}
EXCLUDED_DIRS = {
    ".git",
    ".cache",
    ".venv",
    "__pycache__",
    "build",
    "build-local",
    "node_modules",
    "Testing",
    "third_party",
    "__MACOSX",
}
DEFAULT_ROOTS = [
    "content",
    "data",
    "imports",
    "itch",
    "resources",
    "urpg stuff",
]


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Convert repo audio assets to OGG/Vorbis.")
    p.add_argument("--repo-root", default=".", help="Repository root.")
    p.add_argument(
        "--roots",
        nargs="*",
        default=DEFAULT_ROOTS,
        help="Roots to scan relative to repo root.",
    )
    p.add_argument(
        "--report",
        default="imports/reports/audio_conversion/audio_to_ogg_manifest.json",
        help="Manifest path relative to repo root.",
    )
    p.add_argument(
        "--delete-source",
        action="store_true",
        help="Delete source audio after a matching OGG output exists.",
    )
    p.add_argument(
        "--dry-run",
        action="store_true",
        help="Report planned conversions without invoking ffmpeg or deleting files.",
    )
    p.add_argument(
        "--quality",
        type=int,
        default=5,
        help="Vorbis quality passed as ffmpeg -q:a. Default is 5.",
    )
    return p.parse_args()


def discover_audio(repo_root: Path, roots: list[str]) -> list[Path]:
    files: list[Path] = []
    for root_name in roots:
        root = (repo_root / root_name).resolve()
        if not root.exists():
            continue
        for current, dirs, names in os.walk(root, topdown=True):
            dirs[:] = [d for d in dirs if d not in EXCLUDED_DIRS]
            base = Path(current)
            for name in names:
                path = base / name
                if path.suffix.lower() in AUDIO_EXTENSIONS:
                    files.append(path)
    return sorted(files, key=lambda p: rel(p, repo_root).lower())


def convert_one(source: Path, output: Path, quality: int) -> subprocess.CompletedProcess[str]:
    output.parent.mkdir(parents=True, exist_ok=True)
    command = [
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "error",
        "-y",
        "-i",
        str(source),
        "-vn",
        "-map_metadata",
        "0",
        "-strict",
        "-2",
        "-ac",
        "2",
        "-c:a",
        "vorbis",
        "-q:a",
        str(quality),
        str(output),
    ]
    return subprocess.run(command, text=True, capture_output=True, check=False)


def is_valid_ogg(path: Path) -> bool:
    if not path.exists() or path.stat().st_size == 0:
        return False
    result = subprocess.run(
        [
            "ffprobe",
            "-v",
            "error",
            "-show_entries",
            "format=duration",
            "-of",
            "default=noprint_wrappers=1:nokey=1",
            str(path),
        ],
        text=True,
        capture_output=True,
        check=False,
    )
    return result.returncode == 0 and bool(result.stdout.strip())


def is_lfs_pointer(path: Path) -> bool:
    try:
        with path.open("rb") as f:
            return f.read(64).startswith(b"version https://git-lfs.github.com/spec/")
    except OSError:
        return False


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    report_path = (repo_root / args.report).resolve()
    sources = discover_audio(repo_root, args.roots)

    converted: list[dict] = []
    existing_outputs: list[dict] = []
    deleted_sources: list[str] = []
    skipped_lfs_pointers: list[str] = []
    failed: list[dict] = []

    for source in sources:
        output = source.with_suffix(".ogg")
        row = {
            "source": rel(source, repo_root),
            "output": rel(output, repo_root),
        }

        if is_lfs_pointer(source):
            skipped_lfs_pointers.append(row["source"])
            continue

        if output.exists() and is_valid_ogg(output):
            existing_outputs.append(row)
        elif output.exists():
            output.unlink()
            result = convert_one(source, output, args.quality)
            if result.returncode == 0 and is_valid_ogg(output):
                converted.append(row)
            else:
                failed.append(
                    {
                        **row,
                        "returncode": result.returncode,
                        "stderr": result.stderr.strip()[-2000:],
                    }
                )
                continue
        elif args.dry_run:
            converted.append({**row, "dry_run": True})
        else:
            result = convert_one(source, output, args.quality)
            if result.returncode == 0 and is_valid_ogg(output):
                converted.append(row)
            else:
                failed.append(
                    {
                        **row,
                        "returncode": result.returncode,
                        "stderr": result.stderr.strip()[-2000:],
                    }
                )
                continue

        if args.delete_source and not args.dry_run and output.exists():
            source.unlink()
            deleted_sources.append(row["source"])

    manifest = {
        "schema": "urpg/audio_to_ogg_manifest/v1",
        "generated_at": iso_now(),
        "repo_root": str(repo_root),
        "roots": args.roots,
        "dry_run": bool(args.dry_run),
        "delete_source": bool(args.delete_source),
        "quality": args.quality,
        "summary": {
            "source_files": len(sources),
            "converted": len(converted),
            "existing_outputs": len(existing_outputs),
            "deleted_sources": len(deleted_sources),
            "skipped_lfs_pointers": len(skipped_lfs_pointers),
            "failed": len(failed),
        },
        "converted": converted,
        "existing_outputs": existing_outputs,
        "deleted_sources": deleted_sources,
        "skipped_lfs_pointers": skipped_lfs_pointers,
        "failed": failed,
    }

    if not args.dry_run:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    if failed:
        print(json.dumps(manifest["summary"], indent=2))
        return 1

    print(json.dumps(manifest["summary"], indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
