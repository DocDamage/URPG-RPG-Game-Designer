#!/usr/bin/env python3
"""Query a retrieval bundle for local authoring/debug use."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.shared.retrieval_index import (  # noqa: E402
    create_adapter_from_bundle,
    dot_product,
    load_json,
    tokenize,
)


def safe_print(text: str) -> None:
    encoding = sys.stdout.encoding or "utf-8"
    normalized = text.encode(encoding, errors="replace").decode(
        encoding, errors="replace"
    )
    print(normalized)


def lexical_overlap_score(query: str, text: str) -> float:
    query_tokens = set(tokenize(query))
    text_tokens = set(tokenize(text))
    if not query_tokens or not text_tokens:
        return 0.0

    overlap = len(query_tokens & text_tokens)
    return overlap / float(len(query_tokens))


def main() -> int:
    parser = argparse.ArgumentParser(description="Query a retrieval bundle.")
    parser.add_argument(
        "--bundle", required=True, help="Path to the retrieval bundle JSON file."
    )
    parser.add_argument("--query", required=True, help="Query text.")
    parser.add_argument(
        "--limit", type=int, default=5, help="Maximum number of matches to return."
    )
    args = parser.parse_args()

    bundle_path = Path(args.bundle).resolve()
    bundle = load_json(bundle_path)
    adapter = create_adapter_from_bundle(bundle)
    query_embedding = adapter.embed_text(args.query)

    scored_entries = []
    for entry in bundle.get("entries", []):
        vector_score = dot_product(query_embedding, entry.get("embedding", []))
        keyword_score = lexical_overlap_score(args.query, entry.get("text", ""))
        score = (0.65 * keyword_score) + (0.35 * vector_score)
        scored_entries.append((score, entry))

    scored_entries.sort(key=lambda item: item[0], reverse=True)

    safe_print(f"Query: {args.query}")
    safe_print(f"Bundle: {bundle_path}")
    safe_print(
        f"Adapter: {bundle.get('embedding_adapter', {}).get('adapter_id', bundle.get('engine', 'builtin_hashed'))}"
    )
    safe_print("")
    for rank, (score, entry) in enumerate(scored_entries[: args.limit], start=1):
        safe_print(
            f"[{rank}] score={score:.4f} chunk_id={entry['chunk_id']} source={entry['source_path']}"
        )
        safe_print(f"    {entry['text']}")
        safe_print("")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
