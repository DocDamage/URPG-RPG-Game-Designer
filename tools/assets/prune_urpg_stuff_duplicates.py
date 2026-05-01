#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import json
from pathlib import Path


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Remove exact duplicate files from the ignored SRC-007 raw intake root "
            "using the promotion catalog's SHA-256 duplicate groups."
        )
    )
    parser.add_argument("--repo-root", default=".", help="Repository root.")
    parser.add_argument("--source-root", default="imports/raw/urpg_stuff")
    parser.add_argument(
        "--catalog",
        default="imports/reports/asset_intake/urpg_stuff_promotion_catalog.json",
    )
    parser.add_argument(
        "--report",
        default="imports/reports/asset_intake/urpg_stuff_duplicate_prune_report.json",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Delete duplicate files. Default is report-only.",
    )
    return parser.parse_args()


def is_under(path: Path, root: Path) -> bool:
    try:
        path.resolve().relative_to(root.resolve())
        return True
    except ValueError:
        return False


def prune_empty_dirs(start: Path, stop: Path) -> int:
    removed = 0
    current = start.parent
    stop = stop.resolve()
    while current.resolve() != stop:
        if not is_under(current, stop):
            break
        try:
            current.rmdir()
        except OSError:
            break
        removed += 1
        current = current.parent
    return removed


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    source_root = (repo_root / args.source_root).resolve()
    catalog_path = (repo_root / args.catalog).resolve()
    report_path = (repo_root / args.report).resolve()

    if not source_root.is_dir():
        raise SystemExit(f"source root not found: {source_root}")
    if not catalog_path.is_file():
        raise SystemExit(f"catalog not found: {catalog_path}")

    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    plan: list[dict[str, object]] = []
    removed: list[dict[str, object]] = []
    skipped: list[dict[str, object]] = []
    removed_bytes = 0
    removed_empty_dirs = 0

    for group in catalog.get("duplicate_groups", []):
        canonical_rel = group.get("canonical_source_path", "")
        canonical = (repo_root / canonical_rel).resolve()
        if not canonical.is_file():
            skipped.append(
                {
                    "sha256": group.get("sha256", ""),
                    "reason": "missing_canonical",
                    "canonical_source_path": canonical_rel,
                }
            )
            continue

        for duplicate_rel in group.get("duplicate_source_paths", []):
            duplicate = (repo_root / duplicate_rel).resolve()
            if duplicate == canonical:
                skipped.append(
                    {
                        "sha256": group.get("sha256", ""),
                        "reason": "duplicate_matches_canonical_path",
                        "duplicate_source_path": duplicate_rel,
                    }
                )
                continue
            if not is_under(duplicate, source_root):
                skipped.append(
                    {
                        "sha256": group.get("sha256", ""),
                        "reason": "outside_source_root",
                        "duplicate_source_path": duplicate_rel,
                    }
                )
                continue
            row = {
                "sha256": group.get("sha256", ""),
                "canonical_source_path": canonical_rel,
                "duplicate_source_path": duplicate_rel,
            }
            plan.append(row)
            if not args.apply:
                continue
            if not duplicate.is_file():
                skipped.append({**row, "reason": "duplicate_missing"})
                continue
            size = duplicate.stat().st_size
            duplicate.unlink()
            removed_bytes += size
            removed_empty_dirs += prune_empty_dirs(duplicate, source_root)
            removed.append({**row, "size_bytes": size, "removed_at": iso_now()})

    report = {
        "schema": "urpg/asset_duplicate_prune_report/v1",
        "generated_at": iso_now(),
        "source_root": rel(source_root, repo_root),
        "catalog": rel(catalog_path, repo_root),
        "apply_mode": bool(args.apply),
        "planned_removal_count": len(plan),
        "removed_count": len(removed),
        "removed_bytes": removed_bytes,
        "removed_empty_dirs": removed_empty_dirs,
        "skipped_count": len(skipped),
        "removed": removed,
        "skipped": skipped,
        "notes": [
            "Only exact SHA-256 duplicate_source_paths from the promotion catalog are eligible.",
            "Canonical files are preserved.",
            "The raw source root is ignored by Git; this report is the tracked audit artifact.",
        ],
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(
        json.dumps(
            {
                "report": rel(report_path, repo_root),
                "apply_mode": bool(args.apply),
                "planned_removal_count": len(plan),
                "removed_count": len(removed),
                "removed_bytes": removed_bytes,
                "skipped_count": len(skipped),
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
