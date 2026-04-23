#!/usr/bin/env python3
"""Focused tests for retrieval bundle adapter resolution defaults."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.faiss_index_builder.build_retrieval_bundle import resolve_adapter


def _expected_default_adapter_command(backend_id: str) -> list[str]:
    return [
        sys.executable,
        str(
            REPO_ROOT
            / "tools"
            / "retrieval"
            / "embedding_jobs"
            / "external_embedding_adapter.py"
        ),
        "--backend",
        backend_id,
        "--serve",
    ]


class ResolveAdapterTests(unittest.TestCase):
    def test_local_ngram_projection_defaults_to_command_adapter(self) -> None:
        adapter_id, command = resolve_adapter("local_ngram_projection", command=None)
        self.assertEqual(adapter_id, "command_adapter")
        self.assertEqual(command, _expected_default_adapter_command("local_ngram_projection"))

    def test_optional_sentence_transformer_defaults_to_command_adapter(self) -> None:
        adapter_id, command = resolve_adapter("optional_sentence_transformer", command=None)
        self.assertEqual(adapter_id, "command_adapter")
        self.assertEqual(command, _expected_default_adapter_command("optional_sentence_transformer"))

    def test_builtin_hashed_uses_builtin_adapter_and_no_command(self) -> None:
        adapter_id, command = resolve_adapter("builtin_hashed", command=None)
        self.assertEqual(adapter_id, "builtin_hashed")
        self.assertIsNone(command)

    def test_command_adapter_requires_explicit_command(self) -> None:
        with self.assertRaises(ValueError) as exc:
            resolve_adapter("command_adapter", command=None)
        self.assertIn(
            "--adapter-command is required when adapter is command_adapter.",
            str(exc.exception),
        )

    def test_command_adapter_preserves_explicit_command(self) -> None:
        command = ["python", "my_adapter.py", "--serve"]
        adapter_id, normalized_command = resolve_adapter("command_adapter", command=command)
        self.assertEqual(adapter_id, "command_adapter")
        self.assertEqual(normalized_command, command)

    def test_unsupported_adapter_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, r"Unsupported adapter_id"):
            resolve_adapter("definitely_invalid_backend", command=None)


if __name__ == "__main__":
    unittest.main()
