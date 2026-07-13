#!/usr/bin/env python3
"""Export the local SQLite asset index as a bounded, editor-readable catalog.

The SQLite database remains the authoring/indexing store.  This module publishes
only metadata through an atomically replaced manifest and JSONL shards so the
native editor never needs SQLite or access to the external asset payloads.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import sqlite3
import sys
import uuid
from collections import Counter
from pathlib import Path
from typing import Any, Iterable


INTERCHANGE_SCHEMA = "urpg.asset_catalog.v1"
DEFAULT_SHARD_SIZE = 1_000
MAX_SHARD_SIZE = 10_000


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def _normalized(value: str | None) -> str:
    return (value or "").replace("\\", "/").casefold()


def _atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    temporary = path.with_name(f".{path.name}.{uuid.uuid4().hex}.tmp")
    try:
        temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _asset_rows(connection: sqlite3.Connection) -> Iterable[sqlite3.Row]:
    return connection.execute(
        """
        SELECT a.id, a.path_rel, a.source_root, a.filename, a.ext, a.size_bytes,
               a.mtime_ns, a.sha256, a.media_kind, a.pack, a.category, a.width,
               a.height, a.duration_ms,
               COALESCE(GROUP_CONCAT(t.name, '\u001f'), '') AS tags
        FROM assets AS a
        LEFT JOIN asset_tags AS at ON at.asset_id = a.id
        LEFT JOIN tags AS t ON t.id = at.tag_id
        WHERE a.missing = 0
        GROUP BY a.id
        ORDER BY a.path_rel COLLATE NOCASE, a.id
        """
    )


def _record(row: sqlite3.Row) -> dict[str, Any]:
    tags = sorted(tag for tag in row["tags"].split("\x1f") if tag)
    extension = row["ext"].lower()
    return {
        "asset_id": f"local:{row['id']}",
        "virtual_path": row["path_rel"],
        "source_root": row["source_root"],
        "filename": row["filename"],
        "extension": extension,
        "media_kind": row["media_kind"],
        "archive_kind": extension if row["media_kind"] == "archive" else "",
        "size_bytes": row["size_bytes"],
        "mtime_ns": row["mtime_ns"],
        "sha256": row["sha256"],
        "pack": row["pack"] or "",
        "category": row["category"] or "",
        "width": row["width"],
        "height": row["height"],
        "duration_ms": row["duration_ms"],
        "tags": tags,
        "normalized_filename": _normalized(row["filename"]),
        "normalized_virtual_path": _normalized(row["path_rel"]),
        "normalized_extension": _normalized(extension),
        "normalized_pack": _normalized(row["pack"]),
        "normalized_category": _normalized(row["category"]),
        "normalized_tags": [_normalized(tag) for tag in tags],
    }


def _source_roots(connection: sqlite3.Connection, scan_complete: bool) -> list[dict[str, Any]]:
    rows = connection.execute(
        """
        SELECT source_root, COUNT(*) AS asset_count,
               SUM(CASE WHEN sha256 IS NULL THEN 1 ELSE 0 END) AS hash_pending_count
        FROM assets
        WHERE missing = 0
        GROUP BY source_root
        ORDER BY source_root COLLATE NOCASE
        """
    ).fetchall()
    state = "complete" if scan_complete else "incomplete"
    return [
        {
            "id": row["source_root"],
            "state": state,
            "asset_count": row["asset_count"],
            "hash_pending_count": row["hash_pending_count"],
        }
        for row in rows
    ]


def export_catalog(db_path: Path, output_dir: Path, shard_size: int = DEFAULT_SHARD_SIZE) -> dict[str, Any]:
    """Export a metadata catalog and return its manifest.

    Existing manifests are not replaced until every new shard has been written.
    This makes the manifest an atomic generation pointer, including when a
    process is interrupted midway through a large export.
    """
    if shard_size < 1 or shard_size > MAX_SHARD_SIZE:
        raise ValueError(f"shard_size must be between 1 and {MAX_SHARD_SIZE}")
    if not db_path.is_file():
        raise FileNotFoundError(f"asset catalog database not found: {db_path}")

    output_dir.mkdir(parents=True, exist_ok=True)
    created_paths: list[Path] = []
    connection = sqlite3.connect(f"file:{db_path.resolve().as_posix()}?mode=ro", uri=True)
    connection.row_factory = sqlite3.Row
    try:
        tables = {
            row["name"]
            for row in connection.execute("SELECT name FROM sqlite_master WHERE type='table'").fetchall()
        }
        required_tables = {"assets", "asset_tags", "tags", "scan_runs", "settings"}
        if not required_tables.issubset(tables):
            raise RuntimeError("asset catalog database is not initialized; run asset_db.py init and index first")

        incomplete_runs = connection.execute(
            "SELECT COUNT(*) AS count FROM scan_runs WHERE finished_at IS NULL"
        ).fetchone()["count"]
        scan_complete = incomplete_runs == 0
        generation = uuid.uuid4().hex
        counts: Counter[str] = Counter()
        shards: list[dict[str, Any]] = []
        shard_file = None
        shard_path = None
        shard_final_path = None
        shard_count = 0

        def close_shard() -> None:
            nonlocal shard_file, shard_path, shard_final_path, shard_count
            if shard_file is None or shard_path is None or shard_final_path is None:
                return
            shard_file.close()
            os.replace(shard_path, shard_final_path)
            created_paths.append(shard_final_path)
            shards.append({"path": shard_final_path.name, "record_count": shard_count})
            shard_file = None
            shard_path = None
            shard_final_path = None
            shard_count = 0

        for row in _asset_rows(connection):
            if shard_file is None:
                name = f"catalog-{generation}-{len(shards) + 1:05d}.jsonl"
                shard_final_path = output_dir / name
                shard_path = output_dir / f".{name}.tmp"
                shard_file = shard_path.open("x", encoding="utf-8", newline="\n")
            record = _record(row)
            shard_file.write(json.dumps(record, sort_keys=True, separators=(",", ":")) + "\n")
            shard_count += 1
            counts["asset_count"] += 1
            counts["hash_pending_count"] += int(record["sha256"] is None)
            counts["archive_count"] += int(record["media_kind"] == "archive")
            counts[f"media_kind:{record['media_kind']}"] += 1
            if shard_count >= shard_size:
                close_shard()
        close_shard()

        db_schema_row = connection.execute(
            "SELECT value FROM settings WHERE key = 'schema_version'"
        ).fetchone()
        manifest: dict[str, Any] = {
            "schema_version": INTERCHANGE_SCHEMA,
            "generated_at": now_utc(),
            "scan_complete": scan_complete,
            "source_database_schema_version": db_schema_row["value"] if db_schema_row else "unknown",
            "counts": {
                "asset_count": counts["asset_count"],
                "hash_pending_count": counts["hash_pending_count"],
                "archive_count": counts["archive_count"],
                "media_kind": {
                    key.removeprefix("media_kind:"): value
                    for key, value in sorted(counts.items())
                    if key.startswith("media_kind:")
                },
            },
            "roots": _source_roots(connection, scan_complete),
            "shards": shards,
            "regenerate_command": "python tools/assets/catalog_interchange.py --db .urpg/asset-index/asset_catalog.db",
        }
        _atomic_write_json(output_dir / "catalog_meta.json", manifest)
        referenced = {entry["path"] for entry in shards}
        for stale in output_dir.glob("catalog-*.jsonl"):
            if stale.name not in referenced:
                stale.unlink(missing_ok=True)
        return manifest
    except Exception:
        for path in created_paths:
            path.unlink(missing_ok=True)
        raise
    finally:
        connection.close()


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description="Export the SQLite asset index as an editor catalog interchange.")
    result.add_argument("--db", type=Path, default=Path(".urpg/asset-index/asset_catalog.db"))
    result.add_argument("--output", type=Path, default=Path(".urpg/asset-index"))
    result.add_argument("--shard-size", type=int, default=DEFAULT_SHARD_SIZE)
    return result


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        manifest = export_catalog(args.db, args.output, args.shard_size)
    except (OSError, RuntimeError, ValueError, sqlite3.Error) as error:
        print(f"catalog export failed: {error}", file=sys.stderr)
        return 2
    print(json.dumps({"asset_count": manifest["counts"]["asset_count"], "shards": len(manifest["shards"])}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
