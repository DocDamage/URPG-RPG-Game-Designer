#!/usr/bin/env python3
"""Shared retrieval-side helpers for bundle building and local querying."""

from __future__ import annotations

import json
import math
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
from dataclasses import dataclass


TOKEN_PATTERN = re.compile(r"[A-Za-z0-9_]+")
DEFAULT_DIMENSION = 128


def tokenize(text: str) -> list[str]:
    return [token.lower() for token in TOKEN_PATTERN.findall(text)]


def dot_product(lhs: list[float], rhs: list[float]) -> float:
    return sum(left * right for left, right in zip(lhs, rhs))


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _normalize_vector(vector: list[float]) -> list[float]:
    norm = math.sqrt(sum(value * value for value in vector))
    if norm <= 0.0:
        return vector
    return [value / norm for value in vector]


@dataclass
class EmbeddingAdapterSpec:
    adapter_id: str
    dimension: int
    command: list[str] | None = None
    metadata: dict | None = None

    def to_metadata(self) -> dict:
        payload = {
            "adapter_id": self.adapter_id,
            "dimension": self.dimension,
        }
        if self.command:
            payload["command"] = self.command
        if self.metadata:
            payload["metadata"] = self.metadata
        return payload


class EmbeddingAdapter:
    def spec(self) -> EmbeddingAdapterSpec:
        raise NotImplementedError

    def embed_text(self, text: str) -> list[float]:
        raise NotImplementedError

    def embed_texts(self, texts: list[str]) -> list[list[float]]:
        return [self.embed_text(text) for text in texts]

    def close(self) -> None:
        return None

    def runtime_metadata(self, include_cache_stats: bool = False) -> dict | None:
        return None


class BuiltinHashedAdapter(EmbeddingAdapter):
    def __init__(self, dimension: int = DEFAULT_DIMENSION) -> None:
        self._dimension = dimension

    def spec(self) -> EmbeddingAdapterSpec:
        return EmbeddingAdapterSpec(
            adapter_id="builtin_hashed",
            dimension=self._dimension,
            metadata={
                "kind": "local_builtin",
                "backend": {
                    "backend_id": "builtin_hashed",
                    "backend_version": "1.0",
                },
                "health": {
                    "ok": True,
                    "status": "ready",
                },
            },
        )

    def embed_text(self, text: str) -> list[float]:
        vector = [0.0] * self._dimension
        tokens = tokenize(text)
        if not tokens:
            return vector

        for token in tokens:
            slot = hash(token) % self._dimension
            vector[slot] += 1.0

        return _normalize_vector(vector)

    def embed_texts(self, texts: list[str]) -> list[list[float]]:
        return [self.embed_text(text) for text in texts]

    def runtime_metadata(self, include_cache_stats: bool = False) -> dict | None:
        metadata = self.spec().metadata or {}
        if include_cache_stats:
            metadata = dict(metadata)
            metadata["cache_stats"] = {
                "enabled": False,
                "entries": 0,
                "hits": 0,
                "misses": 0,
            }
        return metadata


class CommandEmbeddingAdapter(EmbeddingAdapter):
    def __init__(self, command: list[str], dimension: int, adapter_id: str = "command_adapter") -> None:
        self._command = command
        self._dimension = dimension
        self._adapter_id = adapter_id
        self._worker: subprocess.Popen[str] | None = None
        self._runtime_metadata: dict | None = None

    def spec(self) -> EmbeddingAdapterSpec:
        metadata = {
            "kind": "external_command",
            "transport": "persistent_jsonl" if self._supports_persistent_worker() else "one_shot_stdio",
        }
        runtime_metadata = self.runtime_metadata(include_cache_stats=False)
        if runtime_metadata:
            metadata.update(runtime_metadata)
        return EmbeddingAdapterSpec(
            adapter_id=self._adapter_id,
            dimension=self._dimension,
            command=self._command,
            metadata=metadata,
        )

    def _supports_persistent_worker(self) -> bool:
        return "--serve" in self._command

    def _run_one_shot(self, payload: dict) -> dict:
        request = json.dumps(payload)
        completed = subprocess.run(
            self._command,
            input=request,
            text=True,
            capture_output=True,
            check=False,
        )
        if completed.returncode != 0:
            raise RuntimeError(
                f"Embedding adapter command failed ({completed.returncode}): {completed.stderr.strip()}"
            )
        return json.loads(completed.stdout)

    def _ensure_worker(self) -> subprocess.Popen[str]:
        if self._worker is not None and self._worker.poll() is None:
            return self._worker

        self._worker = subprocess.Popen(
            self._command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
        return self._worker

    def _run_persistent(self, payload: dict) -> dict:
        worker = self._ensure_worker()
        if worker.stdin is None or worker.stdout is None:
            raise RuntimeError("Embedding adapter worker did not expose stdin/stdout pipes.")

        worker.stdin.write(json.dumps(payload) + "\n")
        worker.stdin.flush()
        response_line = worker.stdout.readline()
        if not response_line:
            stderr_text = ""
            if worker.stderr is not None:
                stderr_text = worker.stderr.read().strip()
            self.close()
            raise RuntimeError(f"Embedding adapter worker closed unexpectedly. {stderr_text}".strip())
        return json.loads(response_line)

    def _request(self, payload: dict) -> dict:
        if self._supports_persistent_worker():
            return self._run_persistent(payload)
        return self._run_one_shot(payload)

    def _update_runtime_metadata_from_payload(self, payload: dict) -> None:
        status = payload.get("status")
        if isinstance(status, dict):
            self._runtime_metadata = status

    def embed_text(self, text: str) -> list[float]:
        payload = self._request({"text": text, "dimension": self._dimension})
        self._update_runtime_metadata_from_payload(payload)
        vector = payload.get("embedding", [])
        if len(vector) != self._dimension:
            raise RuntimeError(
                f"Embedding adapter returned dimension {len(vector)} but expected {self._dimension}"
            )
        return _normalize_vector([float(value) for value in vector])

    def embed_texts(self, texts: list[str]) -> list[list[float]]:
        payload = self._request({"texts": texts, "dimension": self._dimension})
        self._update_runtime_metadata_from_payload(payload)
        embeddings = payload.get("embeddings", [])
        if len(embeddings) != len(texts):
            raise RuntimeError(
                f"Embedding adapter returned {len(embeddings)} embeddings but expected {len(texts)}"
            )

        normalized_vectors: list[list[float]] = []
        for vector in embeddings:
            if len(vector) != self._dimension:
                raise RuntimeError(
                    f"Embedding adapter returned dimension {len(vector)} but expected {self._dimension}"
                )
            normalized_vectors.append(_normalize_vector([float(value) for value in vector]))
        return normalized_vectors

    def runtime_metadata(self, include_cache_stats: bool = False) -> dict | None:
        if self._runtime_metadata is not None:
            if include_cache_stats:
                payload = self._request({"control": "status", "include_cache_stats": True})
                self._update_runtime_metadata_from_payload(payload)
            return self._runtime_metadata

        payload = self._request({"control": "status", "include_cache_stats": include_cache_stats})
        self._update_runtime_metadata_from_payload(payload)
        return self._runtime_metadata

    def close(self) -> None:
        worker = self._worker
        self._worker = None
        if worker is None:
            return
        if worker.stdin is not None:
            worker.stdin.close()
        if worker.stdout is not None:
            worker.stdout.close()
        if worker.stderr is not None:
            worker.stderr.close()
        if worker.poll() is None:
            worker.terminate()
            try:
                worker.wait(timeout=2)
            except subprocess.TimeoutExpired:
                worker.kill()
                worker.wait(timeout=2)

    def __del__(self) -> None:
        try:
            self.close()
        except Exception:
            pass


def create_embedding_adapter(
    adapter_id: str = "builtin_hashed",
    dimension: int = DEFAULT_DIMENSION,
    command: list[str] | None = None,
) -> EmbeddingAdapter:
    if adapter_id == "builtin_hashed":
        return BuiltinHashedAdapter(dimension)
    if adapter_id == "command_adapter":
        if not command:
            raise ValueError("command_adapter requires a command.")
        return CommandEmbeddingAdapter(command=command, dimension=dimension, adapter_id=adapter_id)
    raise ValueError(f"Unsupported adapter_id: {adapter_id}")


def create_adapter_from_bundle(bundle: dict) -> EmbeddingAdapter:
    metadata = bundle.get("embedding_adapter", {})
    adapter_id = metadata.get("adapter_id", bundle.get("engine", "builtin_hashed"))
    dimension = int(metadata.get("dimension", bundle.get("dimension", DEFAULT_DIMENSION)))
    command = metadata.get("command")
    if isinstance(command, list):
        return create_embedding_adapter(adapter_id=adapter_id, dimension=dimension, command=command)
    return create_embedding_adapter(adapter_id=adapter_id, dimension=dimension)


def build_command_adapter_args(command: str | list[str] | None) -> list[str] | None:
    if not command:
        return None
    if isinstance(command, list):
        return [str(segment) for segment in command if str(segment)]
    stripped = command.strip()
    if not stripped:
        return None
    if stripped.startswith("["):
        payload = json.loads(stripped)
        if not isinstance(payload, list):
            raise ValueError("adapter command JSON must decode to a list of strings.")
        return [str(segment) for segment in payload if str(segment)]
    return [segment for segment in shlex.split(stripped, posix=False) if segment]


def normalize_command_adapter_args(command: list[str] | None, repo_root: Path) -> list[str] | None:
    if not command:
        return None

    normalized = [str(segment) for segment in command if str(segment)]
    if not normalized:
        return None

    if normalized[0].lower() in {"python", "python3"}:
        normalized[0] = sys.executable

    for index in range(1, len(normalized)):
        candidate = Path(normalized[index])
        if not candidate.is_absolute():
            repo_candidate = repo_root / candidate
            if repo_candidate.exists():
                normalized[index] = os.fspath(repo_candidate.resolve())

    return normalized


def default_command_adapter_example() -> dict:
    return {
        "stdin_json": {"texts": ["query text", "chunk text"], "dimension": DEFAULT_DIMENSION},
        "stdout_json": {"embeddings": [[0.0, 0.1, 0.0], [0.2, 0.0, 0.0]]},
        "control_json": {"control": "status", "include_cache_stats": True},
        "notes": "External command adapters should support batched texts, while single-text requests remain valid. Persistent workers use one JSON object per line and expose status/control reporting.",
    }
