#!/usr/bin/env python3
"""S33-T01: FAISS retrieval acceptance — deterministic indexing and ingestion.

Tests that the retrieval bundle pipeline produces stable, reproducible outputs
from the same input manifest across multiple invocations.  These are acceptance
tests: they use the builtin_hashed adapter (no external dependencies) and
verify the output contract, not model quality.

Exit criteria (S33-T01):
  - Chunk manifest → bundle is deterministic for the same input.
  - Bundle schema field is present and correct.
  - Entry count matches the input manifest.
  - Embeddings are non-zero vectors of the declared dimension.
  - Adapter metadata is present and identifies the backend.
  - Two independent bundles from the same manifest compare equal in structure
    and in every embedding value (bit-for-bit determinism).
"""

from __future__ import annotations

import copy
import hashlib
import json
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.faiss_index_builder.build_retrieval_bundle import build_bundle
from tools.retrieval.faiss_index_builder.build_chunk_manifest import build_manifest


# ---------------------------------------------------------------------------
# Shared fixtures
# ---------------------------------------------------------------------------

def _make_chunk_manifest(texts: list[str]) -> dict:
    """Minimal inline chunk manifest for determinism tests."""
    chunks = []
    for i, text in enumerate(texts):
        chunk_id = hashlib.sha1(f"test:{i}:{text}".encode()).hexdigest()[:16]
        chunks.append({
            "chunk_id": chunk_id,
            "source_path": f"test_source_{i}.txt",
            "chunk_index": 0,
            "text": text,
        })
    return {
        "schema": "content/schemas/retrieval_chunk_manifest.schema.json",
        "source_root": "test/",
        "chunk_size": 400,
        "overlap": 80,
        "chunks": chunks,
    }


_SAMPLE_TEXTS = [
    "The hero enters the dungeon cautiously.",
    "Battle system resolves damage using fixed-point arithmetic.",
    "Scene transitions are handled by the SceneManager via push and pop.",
    "Audio mix presets control BGM and SE ducking rules.",
    "The presentation core builds frame intents from authoring data.",
]


# ---------------------------------------------------------------------------
# Acceptance tests
# ---------------------------------------------------------------------------

class BundleSchemContract(unittest.TestCase):
    """S33-T01: Bundle output satisfies the schema contract."""

    def _build(self, texts: list[str]) -> dict:
        manifest = _make_chunk_manifest(texts)
        return build_bundle(
            chunk_manifest=manifest,
            adapter_id="builtin_hashed",
            dimension=128,
            command=None,
            adapter_batch_size=64,
        )

    def test_bundle_schema_field_is_correct(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        self.assertEqual(bundle["schema"], "content/schemas/retrieval_index_bundle.schema.json")

    def test_bundle_entry_count_matches_manifest(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        self.assertEqual(bundle["entry_count"], len(_SAMPLE_TEXTS))
        self.assertEqual(len(bundle["entries"]), len(_SAMPLE_TEXTS))

    def test_bundle_dimension_matches_requested(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        self.assertEqual(bundle["dimension"], 128)

    def test_bundle_engine_field_is_set(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        self.assertIn("engine", bundle)
        self.assertEqual(bundle["engine"], "builtin_hashed")

    def test_bundle_embedding_adapter_metadata_present(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        self.assertIn("embedding_adapter", bundle)
        adapter_meta = bundle["embedding_adapter"]
        self.assertIn("adapter_id", adapter_meta)
        self.assertIn("dimension", adapter_meta)
        self.assertEqual(adapter_meta["adapter_id"], "builtin_hashed")
        self.assertEqual(adapter_meta["dimension"], 128)

    def test_all_entries_have_required_fields(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        required_fields = {"chunk_id", "source_path", "chunk_index", "text", "embedding"}
        for entry in bundle["entries"]:
            for field in required_fields:
                self.assertIn(field, entry, msg=f"Entry missing field: {field}")

    def test_embeddings_are_non_zero_and_correct_dimension(self) -> None:
        bundle = self._build(_SAMPLE_TEXTS)
        for entry in bundle["entries"]:
            emb = entry["embedding"]
            self.assertEqual(len(emb), 128)
            self.assertTrue(any(v != 0.0 for v in emb), "Embedding must not be all-zero")

    def test_empty_manifest_produces_empty_bundle(self) -> None:
        bundle = self._build([])
        self.assertEqual(bundle["entry_count"], 0)
        self.assertEqual(bundle["entries"], [])

    def test_single_chunk_manifest(self) -> None:
        bundle = self._build(["single entry text"])
        self.assertEqual(bundle["entry_count"], 1)
        self.assertEqual(len(bundle["entries"]), 1)
        self.assertEqual(len(bundle["entries"][0]["embedding"]), 128)


class BundleDeterminismAcceptance(unittest.TestCase):
    """S33-T01: Same input produces identical outputs across independent calls."""

    def _build(self, manifest: dict) -> dict:
        return build_bundle(
            chunk_manifest=manifest,
            adapter_id="builtin_hashed",
            dimension=128,
            command=None,
            adapter_batch_size=64,
        )

    def test_same_manifest_produces_identical_bundles(self) -> None:
        manifest = _make_chunk_manifest(_SAMPLE_TEXTS)
        first = self._build(copy.deepcopy(manifest))
        second = self._build(copy.deepcopy(manifest))

        self.assertEqual(first["entry_count"], second["entry_count"])
        self.assertEqual(first["dimension"], second["dimension"])
        self.assertEqual(first["engine"], second["engine"])

        for i, (e1, e2) in enumerate(zip(first["entries"], second["entries"])):
            self.assertEqual(e1["chunk_id"], e2["chunk_id"],
                             msg=f"chunk_id mismatch at index {i}")
            self.assertEqual(e1["embedding"], e2["embedding"],
                             msg=f"embedding mismatch at index {i}")

    def test_different_text_order_produces_different_embeddings(self) -> None:
        forward_manifest = _make_chunk_manifest(["alpha bravo", "charlie delta"])
        reverse_manifest = _make_chunk_manifest(["charlie delta", "alpha bravo"])

        forward_bundle = self._build(forward_manifest)
        reverse_bundle = self._build(reverse_manifest)

        fwd_emb_0 = forward_bundle["entries"][0]["embedding"]
        rev_emb_0 = reverse_bundle["entries"][0]["embedding"]
        # The first entries have different texts → embeddings should differ
        self.assertNotEqual(fwd_emb_0, rev_emb_0)

    def test_serialized_bundle_round_trips_via_json(self) -> None:
        manifest = _make_chunk_manifest(_SAMPLE_TEXTS[:2])
        bundle = self._build(manifest)
        serialized = json.dumps(bundle)
        reloaded = json.loads(serialized)
        self.assertEqual(bundle["entry_count"], reloaded["entry_count"])
        for e_orig, e_rt in zip(bundle["entries"], reloaded["entries"]):
            self.assertEqual(e_orig["chunk_id"], e_rt["chunk_id"])
            self.assertAlmostEqual(
                sum(abs(a - b) for a, b in zip(e_orig["embedding"], e_rt["embedding"])),
                0.0,
                places=12,
                msg="Embedding values should survive JSON round-trip without drift",
            )


class ChunkManifestDeterminism(unittest.TestCase):
    """S33-T01: Chunk manifest building from a directory is stable across calls."""

    def test_manifest_chunk_ids_are_stable_across_calls(self, tmp_path: "Path | None" = None) -> None:
        import tempfile
        import os

        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            (root / "doc_a.md").write_text("# Title\nSome content about the engine.", encoding="utf-8")
            (root / "doc_b.txt").write_text("Another document with different content.", encoding="utf-8")

            first = build_manifest(root, chunk_size=200, overlap=40)
            second = build_manifest(root, chunk_size=200, overlap=40)

        self.assertEqual(len(first["chunks"]), len(second["chunks"]))
        for c1, c2 in zip(first["chunks"], second["chunks"]):
            self.assertEqual(c1["chunk_id"], c2["chunk_id"])
            self.assertEqual(c1["text"], c2["text"])

    def test_manifest_schema_field_is_correct(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            (root / "test.md").write_text("content", encoding="utf-8")
            manifest = build_manifest(root, chunk_size=100, overlap=20)

        self.assertEqual(manifest["schema"], "content/schemas/retrieval_chunk_manifest.schema.json")

    def test_manifest_includes_source_root(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            (root / "data.txt").write_text("test data", encoding="utf-8")
            manifest = build_manifest(root, chunk_size=100, overlap=20)

        self.assertIn("source_root", manifest)


if __name__ == "__main__":
    unittest.main()
