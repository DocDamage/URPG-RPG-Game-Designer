#!/usr/bin/env python3
"""Query an offline retrieval manifest without runtime dependencies."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


TOKEN_RE = re.compile(r"[A-Za-z0-9_]+")


def tokens(text: str) -> set[str]:
    return {match.group(0).lower() for match in TOKEN_RE.finditer(text)}


def score(query: str, excerpt: str) -> float:
    query_tokens = tokens(query)
    excerpt_tokens = tokens(excerpt)
    if not query_tokens or not excerpt_tokens:
        return 0.0
    return len(query_tokens & excerpt_tokens) / float(len(query_tokens))


def query_manifest(manifest: dict, query: str, limit: int) -> list[dict]:
    rows = []
    for chunk in manifest.get("chunks", []):
        excerpt = str(chunk.get("excerpt", ""))
        rows.append(
            {
                "source_path": chunk.get("source_path", ""),
                "chunk_id": chunk.get("chunk_id", ""),
                "score": score(query, excerpt),
                "excerpt": excerpt,
            }
        )
    rows.sort(key=lambda row: (-row["score"], row["source_path"], row["chunk_id"]))
    return rows[: max(0, limit)]


def main() -> int:
    parser = argparse.ArgumentParser(description="Query an offline retrieval index manifest.")
    parser.add_argument("--manifest", required=True, help="Manifest produced by build_index.py.")
    parser.add_argument("--query", required=True)
    parser.add_argument("--limit", type=int, default=5)
    parser.add_argument("--json", action="store_true", help="Emit JSON rows instead of text.")
    args = parser.parse_args()

    manifest = json.loads(Path(args.manifest).read_text(encoding="utf-8"))
    rows = query_manifest(manifest, args.query, args.limit)
    if args.json:
        print(json.dumps({"query": args.query, "results": rows}, indent=2))
    else:
        for row in rows:
            print(f"{row['score']:.4f}\t{row['chunk_id']}\t{row['source_path']}\t{row['excerpt']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
