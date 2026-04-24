#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
from pathlib import Path

RAW_PREFIX = "imports/raw/more_assets/"
PROTECTED_NAME_TOKENS = ("license", "licence", "readme", "copying", "credits", "third-party")


def now_iso() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Plan exact duplicate cleanup inside imports/raw/more_assets while preserving "
            "archive boundaries and protected license/readme files. Defaults to report-only."
        )
    )
    parser.add_argument("--repo-root", default=".", help="Repository root")
    parser.add_argument("--duplicates-csv", default="imports/reports/asset_hygiene_duplicates.csv")
    parser.add_argument("--plan-csv", default="imports/reports/more_assets/more_assets_safe_dedupe_plan.csv")
    parser.add_argument("--applied-csv", default="imports/reports/more_assets/more_assets_safe_dedupe_applied.csv")
    parser.add_argument("--summary-json", default="imports/reports/more_assets/more_assets_safe_dedupe_summary.json")
    parser.add_argument("--apply", action="store_true", help="Delete planned duplicate files")
    parser.add_argument("--max-removals", type=int, default=0, help="Optional cap when --apply is used")
    return parser.parse_args()


def archive_root(path_rel: str) -> str | None:
    if not path_rel.startswith(RAW_PREFIX):
        return None
    remainder = path_rel[len(RAW_PREFIX) :]
    parts = remainder.split("/", 1)
    if not parts or not parts[0]:
        return None
    return RAW_PREFIX + parts[0] + "/"


def is_protected(path_rel: str) -> bool:
    name = Path(path_rel).name.lower()
    return any(token in name for token in PROTECTED_NAME_TOKENS)


def is_eligible(path_rel: str, keep_rel: str) -> tuple[bool, str]:
    remove_root = archive_root(path_rel)
    keep_root = archive_root(keep_rel)
    if remove_root is None:
        return False, "remove_not_more_assets_raw"
    if keep_root is None:
        return False, "keep_not_more_assets_raw"
    if remove_root != keep_root:
        return False, "cross_archive_duplicate_preserved"
    if path_rel == keep_rel:
        return False, "candidate_is_keep"
    if is_protected(path_rel):
        return False, "protected_metadata_file"
    return True, "same_archive_exact_duplicate"


def prune_empty_dirs(start_file: Path, stop_dir: Path) -> int:
    removed = 0
    current = start_file.parent
    while current != stop_dir and stop_dir in current.parents:
        try:
            next(current.iterdir())
            break
        except StopIteration:
            current.rmdir()
            removed += 1
            current = current.parent
        except OSError:
            break
    return removed


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    duplicates_csv = (repo_root / args.duplicates_csv).resolve()
    plan_csv = (repo_root / args.plan_csv).resolve()
    applied_csv = (repo_root / args.applied_csv).resolve()
    summary_json = (repo_root / args.summary_json).resolve()
    plan_csv.parent.mkdir(parents=True, exist_ok=True)

    if not duplicates_csv.exists():
        raise SystemExit(f"Missing duplicates CSV: {duplicates_csv}")

    plan_rows: list[dict[str, str]] = []
    skipped_reasons: dict[str, int] = {}

    with duplicates_csv.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            if row.get("recommended_remove", "").lower() != "yes":
                continue
            path_rel = row.get("path_rel", "")
            keep_rel = row.get("recommended_keep", "")
            eligible, reason = is_eligible(path_rel, keep_rel)
            if not eligible:
                skipped_reasons[reason] = skipped_reasons.get(reason, 0) + 1
                continue
            plan_rows.append(
                {
                    "path_rel": path_rel,
                    "recommended_keep": keep_rel,
                    "archive_root": archive_root(path_rel) or "",
                    "sha256": row.get("sha256", ""),
                    "size_bytes": row.get("size_bytes", ""),
                    "eligible_reason": reason,
                }
            )

    plan_rows.sort(key=lambda item: (item["archive_root"], item["path_rel"]))

    with plan_csv.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=["path_rel", "recommended_keep", "archive_root", "sha256", "size_bytes", "eligible_reason"],
        )
        writer.writeheader()
        writer.writerows(plan_rows)

    applied_rows: list[dict[str, str]] = []
    removed_count = 0
    removed_bytes = 0
    missing_count = 0
    skipped_by_cap = 0
    removed_empty_dirs = 0

    if args.apply:
        cap = args.max_removals if args.max_removals > 0 else None
        stop_dir = (repo_root / RAW_PREFIX).resolve()
        for row in plan_rows:
            if cap is not None and removed_count >= cap:
                skipped_by_cap += 1
                continue
            target = (repo_root / row["path_rel"]).resolve()
            keep = (repo_root / row["recommended_keep"]).resolve()
            if not target.exists():
                missing_count += 1
                continue
            if not keep.exists() or not target.is_file():
                continue
            size = target.stat().st_size
            target.unlink()
            removed_count += 1
            removed_bytes += size
            removed_empty_dirs += prune_empty_dirs(target, stop_dir)
            applied_rows.append(
                {
                    "path_rel": row["path_rel"],
                    "recommended_keep": row["recommended_keep"],
                    "sha256": row["sha256"],
                    "size_bytes": str(size),
                    "applied_at": now_iso(),
                }
            )

    with applied_csv.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=["path_rel", "recommended_keep", "sha256", "size_bytes", "applied_at"],
        )
        writer.writeheader()
        writer.writerows(applied_rows)

    candidate_bytes = sum(int(row["size_bytes"] or 0) for row in plan_rows)
    summary = {
        "generated_at": now_iso(),
        "apply_mode": bool(args.apply),
        "plan_count": len(plan_rows),
        "candidate_bytes": candidate_bytes,
        "removed_count": removed_count,
        "removed_bytes": removed_bytes,
        "missing_count": missing_count,
        "skipped_by_cap": skipped_by_cap,
        "removed_empty_dirs": removed_empty_dirs,
        "skipped_reasons": skipped_reasons,
        "plan_csv": str(plan_csv),
        "applied_csv": str(applied_csv),
    }
    summary_json.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    print(f"PLAN_COUNT\t{len(plan_rows)}")
    print(f"CANDIDATE_BYTES\t{candidate_bytes}")
    print(f"APPLY_MODE\t{args.apply}")
    print(f"REMOVED_COUNT\t{removed_count}")
    print(f"REMOVED_BYTES\t{removed_bytes}")
    print(f"MISSING_COUNT\t{missing_count}")
    print(f"SKIPPED_BY_CAP\t{skipped_by_cap}")
    print(f"PLAN_CSV\t{plan_csv}")
    print(f"APPLIED_CSV\t{applied_csv}")
    print(f"SUMMARY_JSON\t{summary_json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
