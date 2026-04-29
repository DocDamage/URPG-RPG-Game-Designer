#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import json
import sqlite3
from pathlib import Path


DEFAULT_DB = "third_party/asset-index/asset_catalog.db"
DEFAULT_REPORT = "imports/reports/asset_intake/third_party_itch_ingest_summary.json"


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rows(conn: sqlite3.Connection, sql: str, params: tuple = ()) -> list[dict]:
    return [dict(row) for row in conn.execute(sql, params)]


def scalar(conn: sqlite3.Connection, sql: str, params: tuple = ()) -> int:
    value = conn.execute(sql, params).fetchone()[0]
    return int(value or 0)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Summarize the local third_party + itch asset DB ingest.")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--db", default=DEFAULT_DB)
    parser.add_argument("--report", default=DEFAULT_REPORT)
    parser.add_argument("--duplicate-limit", type=int, default=25)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    db_path = (repo_root / args.db).resolve()
    report_path = (repo_root / args.report).resolve()
    if not db_path.is_file():
        raise SystemExit(f"asset DB not found: {db_path}")

    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row
    latest_scan = conn.execute("SELECT * FROM scan_runs ORDER BY id DESC LIMIT 1").fetchone()
    if latest_scan is None:
        raise SystemExit("asset DB has no scan runs")

    duplicate_groups = rows(
        conn,
        """
        SELECT sha256, COUNT(*) AS copies, SUM(size_bytes) AS total_size_bytes
        FROM assets
        WHERE missing = 0 AND sha256 IS NOT NULL
        GROUP BY sha256
        HAVING COUNT(*) > 1
        ORDER BY copies DESC, total_size_bytes DESC
        LIMIT ?
        """,
        (args.duplicate_limit,),
    )
    for group in duplicate_groups:
        group["sample_paths"] = [
            row["path_rel"]
            for row in conn.execute(
                """
                SELECT path_rel
                FROM assets
                WHERE missing = 0 AND sha256 = ?
                ORDER BY path_rel
                LIMIT 12
                """,
                (group["sha256"],),
            )
        ]

    report = {
        "schema": "urpg/third_party_itch_ingest_summary/v1",
        "generated_at": iso_now(),
        "asset_db": str(db_path.relative_to(repo_root)).replace("\\", "/"),
        "latest_scan": {
            "id": latest_scan["id"],
            "started_at": latest_scan["started_at"],
            "finished_at": latest_scan["finished_at"],
            "roots": json.loads(latest_scan["roots_json"]),
            "files_seen": latest_scan["files_seen"],
            "files_indexed": latest_scan["files_indexed"],
            "files_skipped": latest_scan["files_skipped"],
            "files_removed_from_db": latest_scan["files_removed"],
        },
        "summary": {
            "asset_count": scalar(conn, "SELECT COUNT(*) FROM assets WHERE missing = 0"),
            "total_size_bytes": scalar(conn, "SELECT SUM(size_bytes) FROM assets WHERE missing = 0"),
            "duplicate_group_count": scalar(
                conn,
                """
                SELECT COUNT(*) FROM (
                  SELECT sha256 FROM assets
                  WHERE missing = 0 AND sha256 IS NOT NULL
                  GROUP BY sha256
                  HAVING COUNT(*) > 1
                )
                """,
            ),
            "duplicate_asset_count": scalar(
                conn,
                """
                SELECT SUM(copies - 1) FROM (
                  SELECT COUNT(*) AS copies FROM assets
                  WHERE missing = 0 AND sha256 IS NOT NULL
                  GROUP BY sha256
                  HAVING COUNT(*) > 1
                )
                """,
            ),
        },
        "by_root": rows(
            conn,
            """
            SELECT source_root, COUNT(*) AS count, SUM(size_bytes) AS size_bytes
            FROM assets
            WHERE missing = 0
            GROUP BY source_root
            ORDER BY count DESC
            """,
        ),
        "by_kind": rows(
            conn,
            """
            SELECT media_kind, COUNT(*) AS count, SUM(size_bytes) AS size_bytes
            FROM assets
            WHERE missing = 0
            GROUP BY media_kind
            ORDER BY count DESC
            """,
        ),
        "by_category": rows(
            conn,
            """
            SELECT COALESCE(category, '(none)') AS category, COUNT(*) AS count, SUM(size_bytes) AS size_bytes
            FROM assets
            WHERE missing = 0
            GROUP BY COALESCE(category, '(none)')
            ORDER BY count DESC
            """,
        ),
        "top_extensions": rows(
            conn,
            """
            SELECT CASE WHEN ext = '' THEN '(none)' ELSE ext END AS ext, COUNT(*) AS count, SUM(size_bytes) AS size_bytes
            FROM assets
            WHERE missing = 0
            GROUP BY ext
            ORDER BY count DESC
            LIMIT 40
            """,
        ),
        "top_packs": rows(
            conn,
            """
            SELECT COALESCE(pack, '(none)') AS pack, COALESCE(category, '(none)') AS category,
                   COUNT(*) AS count, SUM(size_bytes) AS size_bytes
            FROM assets
            WHERE missing = 0
            GROUP BY COALESCE(pack, '(none)'), COALESCE(category, '(none)')
            ORDER BY count DESC
            LIMIT 40
            """,
        ),
        "top_duplicate_groups": duplicate_groups,
        "notes": [
            "This is a local catalog/index ingest, not release promotion.",
            "The SQLite database is ignored by Git; this JSON is the tracked audit summary.",
            "third_party/itch-assets/packs-by-category junctions are intentionally excluded from asset_db indexing to avoid double counting.",
            "files_removed_from_db means stale asset DB rows were pruned; it does not mean files were deleted from disk.",
        ],
    }

    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"report": str(report_path.relative_to(repo_root)).replace("\\", "/"), **report["summary"]}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
