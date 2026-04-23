#!/usr/bin/env python3
"""Build a retrieval bundle from a chunk manifest.

This currently produces a deterministic JSON bundle with lightweight hashed
embeddings, while keeping the format ready for a later FAISS-backed build step.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.shared.retrieval_index import (
    DEFAULT_DIMENSION,
    build_command_adapter_args,
    create_embedding_adapter,
    load_json,
    normalize_command_adapter_args,
    write_json,
)


def batched_texts(texts: list[str], batch_size: int) -> list[list[str]]:
    return [texts[index : index + batch_size] for index in range(0, len(texts), batch_size)]


def build_bundle(
    chunk_manifest: dict,
    adapter_id: str,
    dimension: int,
    command: list[str] | None,
    adapter_batch_size: int,
) -> dict:
    adapter = create_embedding_adapter(adapter_id=adapter_id, dimension=dimension, command=command)
    try:
        chunks = chunk_manifest.get("chunks", [])
        all_texts = [chunk["text"] for chunk in chunks]
        embeddings: list[list[float]] = []
        if all_texts:
            for text_batch in batched_texts(all_texts, max(1, adapter_batch_size)):
                embeddings.extend(adapter.embed_texts(text_batch))
        spec = adapter.spec()
        runtime_metadata = adapter.runtime_metadata(include_cache_stats=True)
        if runtime_metadata:
            spec.metadata = dict(spec.metadata or {})
            spec.metadata.update(runtime_metadata)

        entries = []
        for chunk, embedding in zip(chunks, embeddings):
            entries.append(
                {
                    "chunk_id": chunk["chunk_id"],
                    "source_path": chunk["source_path"],
                    "chunk_index": chunk["chunk_index"],
                    "text": chunk["text"],
                    "embedding": embedding,
                }
            )

        return {
            "schema": "content/schemas/retrieval_index_bundle.schema.json",
            "dimension": spec.dimension,
            "engine": spec.adapter_id,
            "embedding_adapter": spec.to_metadata(),
            "source_manifest": chunk_manifest.get("schema", ""),
            "source_root": chunk_manifest.get("source_root", ""),
            "entry_count": len(entries),
            "entries": entries,
        }
    finally:
        adapter.close()


def main() -> int:
    parser = argparse.ArgumentParser(description="Build a retrieval bundle from a chunk manifest.")
    parser.add_argument("--manifest", required=True, help="Path to the retrieval chunk manifest.")
    parser.add_argument("--output", required=True, help="Path to the output retrieval bundle.")
    parser.add_argument("--dimension", type=int, default=DEFAULT_DIMENSION)
    parser.add_argument("--adapter", default="builtin_hashed", help="Embedding adapter id.")
    parser.add_argument(
        "--adapter-batch-size",
        type=int,
        default=256,
        help="Maximum number of texts to send in one adapter batch request.",
    )
    parser.add_argument(
        "--adapter-command",
        default="",
        help="Optional external command for command_adapter. Receives JSON on stdin and returns JSON on stdout.",
    )
    parser.add_argument(
        "--adapter-command-arg",
        action="append",
        default=[],
        help="Optional repeated external command argument. Prefer this for paths containing spaces.",
    )
    args = parser.parse_args()

    manifest_path = Path(args.manifest).resolve()
    output_path = Path(args.output).resolve()
    manifest = load_json(manifest_path)
    bundle = build_bundle(
        manifest,
        adapter_id=args.adapter,
        dimension=args.dimension,
        command=normalize_command_adapter_args(
            build_command_adapter_args(args.adapter_command_arg or args.adapter_command),
            REPO_ROOT,
        ),
        adapter_batch_size=args.adapter_batch_size,
    )
    write_json(output_path, bundle)
    print(f"Wrote retrieval bundle: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
