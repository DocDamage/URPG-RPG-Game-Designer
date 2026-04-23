#!/usr/bin/env python3
"""S33-T04/T05: Tests for artifact contract enforcement and provenance verification."""

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.shared.artifact_contracts import (
    RETRIEVAL_BUNDLE_CONTRACT,
    RETRIEVAL_MANIFEST_CONTRACT,
    ArtifactContractViolation,
    ArtifactContract,
    ProvenanceRecord,
    attach_provenance,
    validate_artifact_contract,
    verify_provenance,
)


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

def _minimal_bundle() -> dict:
    return {
        "schema": "content/schemas/retrieval_index_bundle.schema.json",
        "dimension": 128,
        "engine": "builtin_hashed",
        "embedding_adapter": {"adapter_id": "builtin_hashed", "dimension": 128},
        "entry_count": 1,
        "entries": [
            {
                "chunk_id": "abc123",
                "source_path": "doc.md",
                "chunk_index": 0,
                "text": "sample",
                "embedding": [0.1] * 128,
            }
        ],
    }


def _minimal_manifest() -> dict:
    return {
        "schema": "content/schemas/retrieval_chunk_manifest.schema.json",
        "source_root": "docs/",
        "chunk_size": 400,
        "overlap": 80,
        "chunks": [],
    }


# ---------------------------------------------------------------------------
# S33-T04: Contract validation
# ---------------------------------------------------------------------------

class ArtifactContractValidation(unittest.TestCase):

    def test_valid_bundle_passes_contract(self) -> None:
        validate_artifact_contract(_minimal_bundle(), RETRIEVAL_BUNDLE_CONTRACT)

    def test_valid_manifest_passes_contract(self) -> None:
        validate_artifact_contract(_minimal_manifest(), RETRIEVAL_MANIFEST_CONTRACT)

    def test_missing_required_field_raises(self) -> None:
        bundle = _minimal_bundle()
        del bundle["entry_count"]
        with self.assertRaises(ArtifactContractViolation) as ctx:
            validate_artifact_contract(bundle, RETRIEVAL_BUNDLE_CONTRACT)
        self.assertIn("entry_count", str(ctx.exception))

    def test_missing_schema_field_raises(self) -> None:
        bundle = _minimal_bundle()
        del bundle["schema"]
        with self.assertRaises(ArtifactContractViolation) as ctx:
            validate_artifact_contract(bundle, RETRIEVAL_BUNDLE_CONTRACT)
        self.assertIn("schema", str(ctx.exception))

    def test_schema_with_wrong_prefix_raises(self) -> None:
        bundle = _minimal_bundle()
        bundle["schema"] = "http://external.example.com/schema.json"
        with self.assertRaises(ArtifactContractViolation) as ctx:
            validate_artifact_contract(bundle, RETRIEVAL_BUNDLE_CONTRACT)
        self.assertIn("content/schemas/", str(ctx.exception))

    def test_all_bundle_required_fields_individually_raise_if_missing(self) -> None:
        required = ["schema", "dimension", "engine", "embedding_adapter", "entry_count", "entries"]
        for field_name in required:
            bundle = _minimal_bundle()
            del bundle[field_name]
            with self.assertRaises(ArtifactContractViolation, msg=f"Should fail for missing: {field_name}"):
                validate_artifact_contract(bundle, RETRIEVAL_BUNDLE_CONTRACT)

    def test_custom_contract_validates_correctly(self) -> None:
        contract = ArtifactContract(
            required_fields=["schema", "tool_id", "masks"],
            expected_schema_prefix="content/schemas/",
        )
        artifact = {
            "schema": "content/schemas/segmentation_manifest.schema.json",
            "tool_id": "sam2",
            "masks": [],
        }
        validate_artifact_contract(artifact, contract)

    def test_empty_artifact_raises_for_all_required_fields(self) -> None:
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract({}, RETRIEVAL_BUNDLE_CONTRACT)


# ---------------------------------------------------------------------------
# S33-T05: Provenance attachment and verification
# ---------------------------------------------------------------------------

class ProvenanceAttachment(unittest.TestCase):

    def test_attach_adds_provenance_field(self) -> None:
        bundle = _minimal_bundle()
        attach_provenance(bundle, tool_id="faiss_index_builder", tool_version="1.0.0")
        self.assertIn("__provenance__", bundle)

    def test_provenance_contains_required_fields(self) -> None:
        bundle = _minimal_bundle()
        attach_provenance(bundle, tool_id="faiss_index_builder", tool_version="1.0.0", input_hash="abcd1234")
        prov = bundle["__provenance__"]
        self.assertEqual(prov["tool_id"], "faiss_index_builder")
        self.assertEqual(prov["tool_version"], "1.0.0")
        self.assertEqual(prov["input_hash"], "abcd1234")
        self.assertIn("output_hash", prov)
        self.assertIsInstance(prov["output_hash"], str)
        self.assertTrue(len(prov["output_hash"]) == 64)  # SHA-256 hex

    def test_attach_returns_same_object(self) -> None:
        bundle = _minimal_bundle()
        result = attach_provenance(bundle, tool_id="tool", tool_version="0.1")
        self.assertIs(result, bundle)

    def test_output_hash_is_deterministic_for_same_payload(self) -> None:
        bundle_a = _minimal_bundle()
        bundle_b = copy.deepcopy(bundle_a)
        attach_provenance(bundle_a, tool_id="tool", tool_version="1.0")
        attach_provenance(bundle_b, tool_id="tool", tool_version="1.0")
        self.assertEqual(bundle_a["__provenance__"]["output_hash"],
                         bundle_b["__provenance__"]["output_hash"])

    def test_output_hash_changes_when_payload_changes(self) -> None:
        bundle_a = _minimal_bundle()
        bundle_b = copy.deepcopy(bundle_a)
        bundle_b["engine"] = "different_engine"

        attach_provenance(bundle_a, tool_id="tool", tool_version="1.0")
        attach_provenance(bundle_b, tool_id="tool", tool_version="1.0")
        self.assertNotEqual(bundle_a["__provenance__"]["output_hash"],
                            bundle_b["__provenance__"]["output_hash"])


class ProvenanceVerification(unittest.TestCase):

    def _build_with_provenance(self, **kwargs) -> dict:
        bundle = _minimal_bundle()
        attach_provenance(bundle, tool_id="faiss_index_builder", tool_version="1.2.0", **kwargs)
        return bundle

    def test_verify_passes_for_clean_artifact(self) -> None:
        bundle = self._build_with_provenance()
        record = verify_provenance(bundle)
        self.assertIsInstance(record, ProvenanceRecord)
        self.assertEqual(record.tool_id, "faiss_index_builder")

    def test_verify_raises_if_provenance_missing(self) -> None:
        bundle = _minimal_bundle()
        with self.assertRaises(ArtifactContractViolation) as ctx:
            verify_provenance(bundle)
        self.assertIn("missing provenance", str(ctx.exception))

    def test_verify_raises_if_payload_mutated_after_attach(self) -> None:
        bundle = self._build_with_provenance()
        # Mutate a field after provenance was attached → hash mismatch
        bundle["engine"] = "tampered_engine"
        with self.assertRaises(ArtifactContractViolation) as ctx:
            verify_provenance(bundle)
        self.assertIn("mismatch", str(ctx.exception))

    def test_verify_raises_if_output_hash_manually_corrupted(self) -> None:
        bundle = self._build_with_provenance()
        bundle["__provenance__"]["output_hash"] = "0" * 64
        with self.assertRaises(ArtifactContractViolation):
            verify_provenance(bundle)

    def test_verify_raises_if_provenance_fields_incomplete(self) -> None:
        bundle = _minimal_bundle()
        bundle["__provenance__"] = {"tool_id": "faiss_index_builder"}  # missing fields
        with self.assertRaises(ArtifactContractViolation) as ctx:
            verify_provenance(bundle)
        self.assertIn("incomplete", str(ctx.exception))

    def test_verify_returns_provenance_record_with_correct_values(self) -> None:
        bundle = self._build_with_provenance(input_hash="feed0123")
        record = verify_provenance(bundle)
        self.assertEqual(record.tool_id, "faiss_index_builder")
        self.assertEqual(record.tool_version, "1.2.0")
        self.assertEqual(record.input_hash, "feed0123")

    def test_provenance_survives_json_round_trip(self) -> None:
        import json
        bundle = self._build_with_provenance()
        serialized = json.dumps(bundle)
        reloaded = json.loads(serialized)
        record = verify_provenance(reloaded)
        self.assertEqual(record.tool_id, "faiss_index_builder")


if __name__ == "__main__":
    unittest.main()
