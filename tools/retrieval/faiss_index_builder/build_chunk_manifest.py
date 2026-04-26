#!/usr/bin/env python3
"""Build a stable retrieval chunk manifest from project text files.

This intentionally stops at chunk manifest generation. Real embeddings and FAISS
index generation can be added later without changing the runtime boundary.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Iterable


SUPPORTED_EXTENSIONS = {".md", ".txt", ".json"}


def iter_source_files(root: Path) -> Iterable[Path]:
    for path in sorted(root.rglob("*")):
        if path.is_file() and path.suffix.lower() in SUPPORTED_EXTENSIONS:
            yield path


def chunk_text(text: str, chunk_size: int, overlap: int) -> list[str]:
    normalized = " ".join(text.split())
    if not normalized:
        return []

    chunks: list[str] = []
    start = 0
    step = max(1, chunk_size - overlap)
    while start < len(normalized):
        chunks.append(normalized[start : start + chunk_size])
        start += step
    return chunks


def build_manifest(source_root: Path, chunk_size: int, overlap: int) -> dict:
    chunks = []
    for source_path in iter_source_files(source_root):
        try:
            text = source_path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            text = source_path.read_text(encoding="utf-8", errors="replace")
        relative_path = source_path.relative_to(source_root).as_posix()
        for index, chunk in enumerate(chunk_text(text, chunk_size, overlap)):
            chunk_id = hashlib.sha1(
                f"{relative_path}:{index}:{chunk}".encode("utf-8")
            ).hexdigest()[:16]
            chunks.append(
                {
                    "chunk_id": chunk_id,
                    "source_path": relative_path,
                    "chunk_index": index,
                    "text": chunk,
                }
            )

    return {
        "schema": "content/schemas/retrieval_chunk_manifest.schema.json",
        "source_root": source_root.as_posix(),
        "chunk_size": chunk_size,
        "overlap": overlap,
        "chunks": chunks,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Build a retrieval chunk manifest.")
    parser.add_argument(
        "--source-root",
        required=True,
        help="Root directory containing project text to index.",
    )
    parser.add_argument("--output", required=True, help="Output manifest path.")
    parser.add_argument("--chunk-size", type=int, default=400)
    parser.add_argument("--overlap", type=int, default=80)
    args = parser.parse_args()

    source_root = Path(args.source_root).resolve()
    output_path = Path(args.output).resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)

    manifest = build_manifest(source_root, args.chunk_size, args.overlap)
    output_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote retrieval chunk manifest: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
