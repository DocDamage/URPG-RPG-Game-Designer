#!/usr/bin/env python3
"""External command embedding adapter.

This script defines the stable stdin/stdout JSON contract used by the
`command_adapter` retrieval path. It currently uses the same deterministic
hashed embedding strategy as the in-process fallback, but runs in a separate
process so local model runners can replace it later without changing bundle or
query contracts.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
from dataclasses import dataclass, field
import hashlib
import math
import random

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.shared.retrieval_index import BuiltinHashedAdapter, DEFAULT_DIMENSION


class EmbeddingBackend:
    def embed_texts(self, texts: list[str], dimension: int) -> list[list[float]]:
        raise NotImplementedError

    def backend_info(self) -> dict:
        raise NotImplementedError

    def health_report(self) -> dict:
        raise NotImplementedError

    def cache_stats(self) -> dict:
        raise NotImplementedError


@dataclass
class BuiltinHashedBackend(EmbeddingBackend):
    _cache: dict[tuple[int, str], list[float]] = field(default_factory=dict)
    _hits: int = 0
    _misses: int = 0

    def embed_texts(self, texts: list[str], dimension: int) -> list[list[float]]:
        adapter = BuiltinHashedAdapter(dimension=dimension)
        vectors: list[list[float]] = []
        for text in texts:
            key = (dimension, text)
            if key in self._cache:
                self._hits += 1
                vectors.append(self._cache[key])
                continue
            self._misses += 1
            vector = adapter.embed_text(text)
            self._cache[key] = vector
            vectors.append(vector)
        return vectors

    def backend_info(self) -> dict:
        return {
            "backend_id": "builtin_hashed",
            "backend_version": "1.0",
        }

    def health_report(self) -> dict:
        return {
            "ok": True,
            "status": "ready",
        }

    def cache_stats(self) -> dict:
        return {
            "enabled": True,
            "entries": len(self._cache),
            "hits": self._hits,
            "misses": self._misses,
        }


@dataclass
class LocalNgramProjectionBackend(EmbeddingBackend):
    _cache: dict[tuple[int, str], list[float]] = field(default_factory=dict)
    _hits: int = 0
    _misses: int = 0

    @staticmethod
    def _seed_for_feature(prefix: str, feature: str) -> int:
        digest = hashlib.sha256(f"{prefix}:{feature}".encode("utf-8")).digest()
        return int.from_bytes(digest[:8], byteorder="big", signed=False)

    @staticmethod
    def _tokenize(text: str) -> list[str]:
        return [token.lower() for token in str(text).split() if token.strip()]

    @staticmethod
    def _char_ngrams(text: str) -> list[str]:
        normalized = f" {str(text).lower()} "
        ngrams: list[str] = []
        for n in (3, 4, 5):
            if len(normalized) < n:
                continue
            for index in range(len(normalized) - n + 1):
                ngrams.append(normalized[index : index + n])
        return ngrams

    def _project_feature(self, prefix: str, feature: str, dimension: int) -> list[float]:
        rng = random.Random(self._seed_for_feature(prefix, feature))
        return [rng.gauss(0.0, 1.0) for _ in range(dimension)]

    def _embed_one(self, text: str, dimension: int) -> list[float]:
        tokens = self._tokenize(text)
        ngrams = self._char_ngrams(text)
        if not tokens and not ngrams:
            return [0.0] * dimension

        vector = [0.0] * dimension
        for token in tokens:
            projection = self._project_feature("tok", token, dimension)
            for index, value in enumerate(projection):
                vector[index] += value
        for ngram in ngrams:
            projection = self._project_feature("chr", ngram, dimension)
            for index, value in enumerate(projection):
                vector[index] += 0.35 * value

        norm = math.sqrt(sum(value * value for value in vector))
        if norm > 0.0:
            vector = [value / norm for value in vector]
        return [float(value) for value in vector]

    def embed_texts(self, texts: list[str], dimension: int) -> list[list[float]]:
        vectors: list[list[float]] = []
        for text in texts:
            key = (dimension, text)
            if key in self._cache:
                self._hits += 1
                vectors.append(self._cache[key])
                continue
            self._misses += 1
            vector = self._embed_one(text, dimension)
            self._cache[key] = vector
            vectors.append(vector)
        return vectors

    def backend_info(self) -> dict:
        return {
            "backend_id": "local_ngram_projection",
            "backend_version": "1.0",
        }

    def health_report(self) -> dict:
        return {
            "ok": True,
            "status": "ready",
        }

    def cache_stats(self) -> dict:
        return {
            "enabled": True,
            "entries": len(self._cache),
            "hits": self._hits,
            "misses": self._misses,
        }


def create_backend(backend_id: str) -> EmbeddingBackend:
    if backend_id == "builtin_hashed":
        return BuiltinHashedBackend()
    if backend_id == "local_ngram_projection":
        return LocalNgramProjectionBackend()
    raise ValueError(f"Unsupported backend_id: {backend_id}")


def build_status(backend: EmbeddingBackend, include_cache_stats: bool) -> dict:
    status = {
        "backend": backend.backend_info(),
        "health": backend.health_report(),
    }
    if include_cache_stats:
        status["cache_stats"] = backend.cache_stats()
    return status


def build_response(payload: dict, backend: EmbeddingBackend) -> dict:
    if payload.get("control") == "status":
        return {
            "status": build_status(backend, include_cache_stats=bool(payload.get("include_cache_stats", False))),
            "adapter_id": "command_adapter",
        }

    dimension = int(payload.get("dimension", DEFAULT_DIMENSION))
    if "texts" in payload:
        texts = [str(text) for text in payload.get("texts", [])]
        return {
            "embeddings": backend.embed_texts(texts, dimension),
            "adapter_id": "command_adapter",
            "backend": backend.backend_info()["backend_id"],
            "status": build_status(backend, include_cache_stats=False),
        }
    text = str(payload.get("text", ""))
    vector = backend.embed_texts([text], dimension)[0]
    return {
        "embedding": vector,
        "adapter_id": "command_adapter",
        "backend": backend.backend_info()["backend_id"],
        "status": build_status(backend, include_cache_stats=False),
    }


def run_one_shot(backend: EmbeddingBackend) -> int:
    payload = json.load(sys.stdin)
    response = build_response(payload, backend)
    json.dump(response, sys.stdout)
    sys.stdout.write("\n")
    return 0


def run_server(backend: EmbeddingBackend) -> int:
    for line in sys.stdin:
        stripped = line.strip()
        if not stripped:
            continue
        payload = json.loads(stripped)
        response = build_response(payload, backend)
        json.dump(response, sys.stdout)
        sys.stdout.write("\n")
        sys.stdout.flush()
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="URPG external command embedding adapter.")
    parser.add_argument(
        "--serve",
        action="store_true",
        help="Run as a persistent JSONL worker. Reads one JSON request per line and writes one JSON response per line.",
    )
    parser.add_argument(
        "--backend",
        default="local_ngram_projection",
        help="Backend id to load inside the external adapter worker.",
    )
    args = parser.parse_args()
    backend = create_backend(str(args.backend))
    if args.serve:
        return run_server(backend)
    return run_one_shot(backend)


if __name__ == "__main__":
    raise SystemExit(main())
