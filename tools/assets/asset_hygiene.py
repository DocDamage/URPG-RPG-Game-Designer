#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import datetime as dt
import hashlib
import json
import os
import shutil
from pathlib import Path

DEFAULT_ROOTS = ["third_party", "imports", "itch"]
EXCLUDED_DIRS = {".git", ".venv", "__pycache__", "node_modules", ".cache", "build", "Testing"}
JUNK_FILE_NAMES = {".DS_Store", "Thumbs.db", "Desktop.ini"}
JUNK_DIR_NAMES = {"__MACOSX"}
JUNK_PREFIXES = ("._",)
LOWER_JUNK_FILE_NAMES = {n.lower() for n in JUNK_FILE_NAMES}


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def preference_score(path_rel: str) -> tuple[int, int, str]:
    checks = [
        "third_party/rpgmaker-mz/steam-dlc/packs/",
        "third_party/huggingface/",
        "third_party/itch-assets/packs/",
        "third_party/rpgmaker-mz/",
        "third_party/",
        "imports/",
        "itch/",
    ]
    for idx, prefix in enumerate(checks):
        if path_rel.startswith(prefix):
            return (idx, path_rel.count("/"), path_rel)
    return (len(checks), path_rel.count("/"), path_rel)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Asset hygiene scan: junk, duplicates, oversize files.")
    p.add_argument("--repo-root", default=".", help="Repository root.")
    p.add_argument("--roots", nargs="*", default=DEFAULT_ROOTS, help="Roots to scan (relative to repo root).")
    p.add_argument("--oversize-mb", type=int, default=100, help="Oversize threshold in MB.")
    p.add_argument(
        "--max-hash-mb",
        type=int,
        default=512,
        help="Skip hashing files larger than this MB for duplicate detection.",
    )
    p.add_argument("--report-dir", default="imports/reports", help="Directory for report output.")
    p.add_argument("--write-reports", action="store_true", help="Write JSON/CSV reports.")
    p.add_argument("--prune-junk", action="store_true", help="Delete junk files/dirs after scan.")
    return p.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    oversize_bytes = int(args.oversize_mb) * 1024 * 1024
    max_hash_bytes = int(args.max_hash_mb) * 1024 * 1024

    scan_roots: list[Path] = []
    for r in args.roots:
        rp = (repo_root / r).resolve()
        if rp.exists():
            scan_roots.append(rp)

    if not scan_roots:
        raise SystemExit("No scan roots exist.")

    file_rows: list[dict] = []
    junk_files: list[dict] = []
    junk_dirs: list[dict] = []
    oversize_rows: list[dict] = []
    hash_skips: list[dict] = []
    hash_groups: dict[tuple[int, str], list[dict]] = {}
    seen_paths: set[str] = set()

    for root in scan_roots:
        for current, dirs, files in os.walk(root, topdown=True):
            dirs[:] = [d for d in dirs if d not in EXCLUDED_DIRS]
            base = Path(current)

            for d in list(dirs):
                if d in JUNK_DIR_NAMES:
                    junk_dirs.append({"path_rel": rel(base / d, repo_root), "reason": "junk-dir"})

            for name in files:
                p = base / name
                p_rel = rel(p, repo_root)
                if p_rel in seen_paths:
                    continue
                seen_paths.add(p_rel)
                lower_name = name.lower()
                is_junk = (
                    name in JUNK_FILE_NAMES
                    or lower_name in LOWER_JUNK_FILE_NAMES
                    or any(name.startswith(prefix) for prefix in JUNK_PREFIXES)
                )
                if is_junk:
                    junk_files.append({"path_rel": p_rel, "reason": "junk-file"})

                try:
                    stat = p.stat()
                except OSError:
                    continue

                row = {
                    "path_rel": p_rel,
                    "size_bytes": int(stat.st_size),
                    "mtime_ns": int(getattr(stat, "st_mtime_ns", int(stat.st_mtime * 1_000_000_000))),
                    "ext": p.suffix.lower().lstrip("."),
                }
                file_rows.append(row)

                if row["size_bytes"] >= oversize_bytes:
                    oversize_rows.append(row)

                if row["size_bytes"] == 0:
                    continue
                if row["size_bytes"] > max_hash_bytes:
                    hash_skips.append(
                        {
                            "path_rel": p_rel,
                            "size_bytes": row["size_bytes"],
                            "reason": f"exceeds max hash size ({args.max_hash_mb} MB)",
                        }
                    )
                    continue
                try:
                    digest = sha256_file(p)
                except OSError:
                    continue
                key = (row["size_bytes"], digest)
                hash_groups.setdefault(key, []).append(row)

    duplicate_rows: list[dict] = []
    duplicate_groups = 0
    duplicate_files = 0
    duplicate_waste = 0

    for (size_bytes, digest), rows in hash_groups.items():
        if len(rows) < 2:
            continue
        duplicate_groups += 1
        duplicate_files += len(rows)
        keep = sorted(rows, key=lambda r: preference_score(r["path_rel"]))[0]["path_rel"]
        for r in rows:
            duplicate_rows.append(
                {
                    "sha256": digest,
                    "size_bytes": size_bytes,
                    "path_rel": r["path_rel"],
                    "recommended_keep": keep,
                    "recommended_remove": "yes" if r["path_rel"] != keep else "no",
                }
            )
        duplicate_waste += size_bytes * (len(rows) - 1)

    pruned_paths: list[str] = []
    if args.prune_junk:
        for item in junk_files:
            target = repo_root / item["path_rel"]
            if target.exists():
                target.unlink()
                pruned_paths.append(item["path_rel"])
        for item in junk_dirs:
            target = repo_root / item["path_rel"]
            if target.exists():
                shutil.rmtree(target)
                pruned_paths.append(item["path_rel"])

    summary = {
        "generated_at": iso_now(),
        "repo_root": str(repo_root),
        "scan_roots": [rel(r, repo_root) for r in scan_roots],
        "file_count": len(file_rows),
        "junk_file_count": len(junk_files),
        "junk_dir_count": len(junk_dirs),
        "pruned_count": len(pruned_paths),
        "oversize_threshold_mb": args.oversize_mb,
        "oversize_count": len(oversize_rows),
        "duplicate_groups": duplicate_groups,
        "duplicate_file_count": duplicate_files,
        "duplicate_waste_bytes": duplicate_waste,
        "hash_skip_count": len(hash_skips),
    }

    report_dir = (repo_root / args.report_dir).resolve()
    if args.write_reports:
        ensure_dir(report_dir)
        (report_dir / "asset_hygiene_summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")

        with (report_dir / "asset_hygiene_junk.csv").open("w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=["path_rel", "reason"])
            w.writeheader()
            for row in junk_files + junk_dirs:
                w.writerow(row)

        with (report_dir / "asset_hygiene_oversize.csv").open("w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=["path_rel", "size_bytes", "mtime_ns", "ext"])
            w.writeheader()
            for row in sorted(oversize_rows, key=lambda r: (-r["size_bytes"], r["path_rel"])):
                w.writerow(row)

        with (report_dir / "asset_hygiene_duplicates.csv").open("w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(
                f,
                fieldnames=["sha256", "size_bytes", "path_rel", "recommended_keep", "recommended_remove"],
            )
            w.writeheader()
            for row in sorted(duplicate_rows, key=lambda r: (r["sha256"], r["path_rel"])):
                w.writerow(row)

        with (report_dir / "asset_hygiene_hash_skips.csv").open("w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=["path_rel", "size_bytes", "reason"])
            w.writeheader()
            for row in sorted(hash_skips, key=lambda r: (-r["size_bytes"], r["path_rel"])):
                w.writerow(row)

    print(f"SCAN_ROOTS\t{','.join(summary['scan_roots'])}")
    print(f"FILE_COUNT\t{summary['file_count']}")
    print(f"JUNK_FILES\t{summary['junk_file_count']}")
    print(f"JUNK_DIRS\t{summary['junk_dir_count']}")
    print(f"PRUNED\t{summary['pruned_count']}")
    print(f"OVERSIZE_COUNT\t{summary['oversize_count']}")
    print(f"DUPLICATE_GROUPS\t{summary['duplicate_groups']}")
    print(f"DUPLICATE_FILES\t{summary['duplicate_file_count']}")
    print(f"DUPLICATE_WASTE_BYTES\t{summary['duplicate_waste_bytes']}")
    print(f"HASH_SKIPS\t{summary['hash_skip_count']}")
    if args.write_reports:
        print(f"REPORT_DIR\t{report_dir}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
