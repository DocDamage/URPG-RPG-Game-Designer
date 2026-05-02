#!/usr/bin/env python3
"""Build a deterministic FAISS-compatible retrieval manifest.

This top-level entrypoint keeps retrieval indexing in offline tooling. It emits
JSON chunks and index metadata that runtime/editor code can inspect without
importing FAISS or embedding libraries.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


SUPPORTED_EXTENSIONS = {".md", ".txt", ".json", ".csv", ".yaml", ".yml"}


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="utf-8", errors="replace")


def iter_source_files(paths: list[Path]) -> list[Path]:
    files: list[Path] = []
    for path in paths:
        resolved = path.resolve()
        if resolved.is_dir():
            files.extend(
                candidate
                for candidate in sorted(resolved.rglob("*"))
                if candidate.is_file()
                and candidate.suffix.lower() in SUPPORTED_EXTENSIONS
            )
        elif resolved.is_file():
            files.append(resolved)
        else:
            raise FileNotFoundError(f"Source path does not exist: {path}")
    return sorted(dict.fromkeys(files))


def chunk_text(text: str, chunk_size: int, overlap: int) -> list[str]:
    normalized = " ".join(text.split())
    if not normalized:
        return []
    step = max(1, chunk_size - overlap)
    return [
        normalized[start : start + chunk_size]
        for start in range(0, len(normalized), step)
    ]


def stable_chunk_id(source_path: str, chunk_index: int, text: str) -> str:
    digest = hashlib.sha256(
        f"{source_path}:{chunk_index}:{text}".encode("utf-8")
    ).hexdigest()
    stem = Path(source_path).stem.lower().replace(" ", "-") or "chunk"
    return f"{stem}-{digest[:12]}"


def build_manifest(
    source_paths: list[Path], output_path: Path, chunk_size: int, overlap: int
) -> dict:
    files = iter_source_files(source_paths)
    root = Path.cwd().resolve()
    chunks = []
    for source_file in files:
        try:
            display_path = source_file.relative_to(root).as_posix()
        except ValueError:
            display_path = source_file.as_posix()
        for index, chunk in enumerate(
            chunk_text(read_text(source_file), chunk_size, overlap)
        ):
            chunks.append(
                {
                    "chunk_id": stable_chunk_id(display_path, index, chunk),
                    "source_path": display_path,
                    "chunk_index": index,
                    "excerpt": chunk,
                }
            )

    return {
        "schema": "content/schemas/retrieval_index_manifest.schema.json",
        "tool_id": "faiss_compatible_retrieval",
        "source_paths": [path.as_posix() for path in source_paths],
        "chunk_size": chunk_size,
        "overlap": overlap,
        "chunks": chunks,
        "index": {
            "engine": "faiss_compatible_json",
            "metadata_path": output_path.as_posix(),
            "dimension": 64,
            "entry_count": len(chunks),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build an offline retrieval index manifest."
    )
    parser.add_argument(
        "source_paths", nargs="+", help="Files or directories to chunk."
    )
    parser.add_argument("--output", required=True, help="Manifest path to write.")
    parser.add_argument("--chunk-size", type=int, default=400)
    parser.add_argument("--overlap", type=int, default=80)
    args = parser.parse_args()

    if args.chunk_size <= 0:
        raise ValueError("--chunk-size must be positive")
    if args.overlap < 0 or args.overlap >= args.chunk_size:
        raise ValueError("--overlap must be non-negative and smaller than --chunk-size")

    output_path = Path(args.output).resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    manifest = build_manifest(
        [Path(path) for path in args.source_paths],
        output_path,
        args.chunk_size,
        args.overlap,
    )
    output_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote retrieval index manifest: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
