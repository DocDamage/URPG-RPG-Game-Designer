#!/usr/bin/env python3
from __future__ import annotations

import argparse
import fnmatch
import json
import re
import time
from pathlib import Path

TEXT_EXTENSIONS = {
    ".c",
    ".cc",
    ".cmake",
    ".cpp",
    ".cs",
    ".css",
    ".h",
    ".hpp",
    ".html",
    ".ini",
    ".java",
    ".js",
    ".json",
    ".jsx",
    ".md",
    ".ps1",
    ".py",
    ".rs",
    ".sh",
    ".toml",
    ".ts",
    ".tsx",
    ".txt",
    ".xml",
    ".yaml",
    ".yml",
}

DEFAULT_EXCLUDES = [
    ".git/**",
    ".cache/**",
    ".urpg/**",
    "build*/**",
    "build-local/**",
    "imports/raw/**",
    "third_party/**",
    "vendor/**",
    "node_modules/**",
    "**/__pycache__/**",
]


def stable_id(path: str) -> str:
    value = re.sub(r"[^a-zA-Z0-9]+", "_", path).strip("_").lower()
    return value or "project_root"


def path_matches(path: str, patterns: list[str]) -> bool:
    normalized = path.replace("\\", "/")
    return any(fnmatch.fnmatch(normalized, pattern) for pattern in patterns)


def classify_kind(path: Path) -> str:
    ext = path.suffix.lower()
    if ext in {".md", ".txt"}:
        return "doc"
    if ext in {".json", ".yaml", ".yml", ".toml", ".ini", ".xml"}:
        return "data"
    if ext in {".cpp", ".h", ".hpp", ".c", ".cc", ".cs", ".py", ".ps1", ".js", ".ts", ".tsx", ".jsx", ".rs", ".java", ".sh", ".cmake"}:
        return "source"
    return "text"


def read_text(path: Path, max_file_bytes: int) -> tuple[str | None, str | None]:
    size = path.stat().st_size
    if size > max_file_bytes:
        return None, "file_too_large"
    data = path.read_bytes()
    if b"\x00" in data:
        return None, "binary_file"
    try:
        return data.decode("utf-8"), None
    except UnicodeDecodeError:
        return data.decode("utf-8", errors="replace"), "decode_replaced"


def build_record(root: Path, path: Path, content: str, max_age_days: int, now_epoch: int) -> dict:
    relative = path.relative_to(root).as_posix()
    stat = path.stat()
    modified = int(stat.st_mtime)
    age_days = max(0, int((now_epoch - modified) / 86400))
    title = path.name
    return {
        "id": stable_id(relative),
        "path": relative,
        "title": title,
        "kind": classify_kind(path),
        "content": content,
        "indexed_at_epoch": now_epoch,
        "modified_at_epoch": modified,
        "age_days": age_days,
        "max_age_days": max_age_days,
        "summary": f"Indexed {classify_kind(path)} file {relative} for project knowledge search.",
    }


def collect_documents(
    root: Path,
    include: list[str],
    exclude: list[str],
    max_files: int,
    max_bytes: int,
    max_file_bytes: int,
    max_age_days: int,
) -> dict:
    root = root.resolve()
    patterns = include or ["**/*"]
    excludes = DEFAULT_EXCLUDES + exclude
    records: list[dict] = []
    diagnostics: list[dict] = []
    skipped: dict[str, int] = {}
    total_bytes = 0
    now_epoch = int(time.time())

    def skip(path: str, reason: str) -> None:
        skipped[reason] = skipped.get(reason, 0) + 1
        diagnostics.append({"code": reason, "path": path})

    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root).as_posix()
        if path_matches(relative, excludes):
            skip(relative, "excluded")
            continue
        if not path_matches(relative, patterns):
            skip(relative, "not_included")
            continue
        if path.suffix.lower() not in TEXT_EXTENSIONS:
            skip(relative, "unsupported_extension")
            continue
        if len(records) >= max_files:
            skip(relative, "max_files_exceeded")
            continue
        size = path.stat().st_size
        if total_bytes + size > max_bytes:
            skip(relative, "max_bytes_exceeded")
            continue
        content, read_diagnostic = read_text(path, max_file_bytes)
        if content is None:
            skip(relative, read_diagnostic or "unreadable")
            continue
        if read_diagnostic:
            diagnostics.append({"code": read_diagnostic, "path": relative})
        total_bytes += size
        records.append(build_record(root, path, content, max_age_days, now_epoch))

    return {
        "schemaVersion": "1.0.0",
        "source": "tools/ai/collect_project_knowledge.py",
        "root": root.as_posix(),
        "summary": {
            "records": len(records),
            "totalBytes": total_bytes,
            "skipped": skipped,
        },
        "filesystem_documents": records,
        "diagnostics": diagnostics,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Collect bounded project filesystem documents for URPG AI knowledge indexing.")
    parser.add_argument("--root", default=".", help="Project root to scan.")
    parser.add_argument("--output", required=True, help="Output JSON path.")
    parser.add_argument("--include", action="append", default=[], help="Glob to include. May be repeated.")
    parser.add_argument("--exclude", action="append", default=[], help="Glob to exclude. May be repeated.")
    parser.add_argument("--max-files", type=int, default=500)
    parser.add_argument("--max-bytes", type=int, default=2 * 1024 * 1024)
    parser.add_argument("--max-file-bytes", type=int, default=64 * 1024)
    parser.add_argument("--max-age-days", type=int, default=30)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    result = collect_documents(
        Path(args.root),
        args.include,
        args.exclude,
        args.max_files,
        args.max_bytes,
        args.max_file_bytes,
        args.max_age_days,
    )
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps(result["summary"], indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
