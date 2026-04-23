#!/usr/bin/env python3
"""S33-T04/T05: Runtime artifact contract enforcement and provenance checks.

S33-T04 — Runtime contracts: the runtime must consume only exported artifacts,
  never tooling source code.  This module provides:
    - ArtifactContract: describes the expected shape of a tool-exported artifact.
    - validate_artifact_contract: raises on violations.

S33-T05 — Provenance / reproducibility: every exported artifact must carry
  lineage metadata (tool_id, tool_version, input_hash, output_hash).
    - ProvenanceRecord: captures that metadata.
    - attach_provenance: adds it to a bundle dict.
    - verify_provenance: checks it is intact and the declared output_hash matches.

These helpers are consumed by:
  - tools/retrieval: attach provenance when writing bundles.
  - tools/ci/check_tooling_boundary.ps1: call verify_provenance on staged artifacts.
  - Runtime code (engine/): must NEVER import this file.  It is a tooling-only module.
"""

from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


# ---------------------------------------------------------------------------
# S33-T04: Artifact contract
# ---------------------------------------------------------------------------

@dataclass
class ArtifactContract:
    """Declares the expected top-level fields that a runtime-consumable artifact must contain.

    The runtime accepts ONLY artifacts that pass this contract.  Tooling code
    (FAISS builders, SAM segmenters, audio preprocessors) must export artifacts
    that satisfy their declared contract before the runtime may reference them.
    """
    required_fields: list[str] = field(default_factory=list)
    schema_field: str = "schema"
    expected_schema_prefix: str = "content/schemas/"


# Well-known contracts for each approved tooling output type.
RETRIEVAL_BUNDLE_CONTRACT = ArtifactContract(
    required_fields=["schema", "dimension", "engine", "embedding_adapter", "entry_count", "entries"],
    expected_schema_prefix="content/schemas/",
)

RETRIEVAL_MANIFEST_CONTRACT = ArtifactContract(
    required_fields=["schema", "source_root", "chunk_size", "overlap", "chunks"],
    expected_schema_prefix="content/schemas/",
)

SEGMENTATION_MANIFEST_CONTRACT = ArtifactContract(
    required_fields=["schema", "tool_id", "source_asset", "masks"],
    expected_schema_prefix="content/schemas/",
)

AUDIO_STEM_MANIFEST_CONTRACT = ArtifactContract(
    required_fields=["schema", "tool_id", "source_asset", "stems"],
    expected_schema_prefix="content/schemas/",
)


class ArtifactContractViolation(ValueError):
    """Raised when an artifact does not satisfy its declared contract."""


def validate_artifact_contract(artifact: dict[str, Any], contract: ArtifactContract) -> None:
    """Verify that *artifact* satisfies *contract*.

    Raises:
        ArtifactContractViolation: on the first violation found.
    """
    for field_name in contract.required_fields:
        if field_name not in artifact:
            raise ArtifactContractViolation(
                f"Artifact is missing required field: '{field_name}'.  "
                f"Runtime cannot consume artifacts without this field."
            )

    schema_value = artifact.get(contract.schema_field, "")
    if not isinstance(schema_value, str) or not schema_value.startswith(contract.expected_schema_prefix):
        raise ArtifactContractViolation(
            f"Artifact '{contract.schema_field}' field must be a string starting with "
            f"'{contract.expected_schema_prefix}', got: {schema_value!r}"
        )


# ---------------------------------------------------------------------------
# S33-T05: Provenance record
# ---------------------------------------------------------------------------

_PROVENANCE_FIELD = "__provenance__"


@dataclass
class ProvenanceRecord:
    """Lineage metadata attached to every tool-exported artifact.

    Fields
    ------
    tool_id:        Stable identifier for the tool that produced the artifact
                    (e.g. "faiss_index_builder", "sam2_segmenter").
    tool_version:   Semantic version string of the tool at output time.
    input_hash:     SHA-256 hex digest of the canonical input (e.g. manifest file
                    contents).  Empty string if input was not file-backed.
    output_hash:    SHA-256 hex digest of the artifact payload (excluding this
                    provenance record itself) at write time.
    """
    tool_id: str
    tool_version: str
    input_hash: str
    output_hash: str

    def to_dict(self) -> dict[str, str]:
        return {
            "tool_id": self.tool_id,
            "tool_version": self.tool_version,
            "input_hash": self.input_hash,
            "output_hash": self.output_hash,
        }

    @classmethod
    def from_dict(cls, data: dict[str, str]) -> "ProvenanceRecord":
        return cls(
            tool_id=data["tool_id"],
            tool_version=data["tool_version"],
            input_hash=data["input_hash"],
            output_hash=data["output_hash"],
        )


def _stable_payload_hash(artifact: dict[str, Any]) -> str:
    """SHA-256 of the artifact dict excluding the provenance field, in sorted key order."""
    payload = {k: v for k, v in artifact.items() if k != _PROVENANCE_FIELD}
    canonical = json.dumps(payload, sort_keys=True, ensure_ascii=True)
    return hashlib.sha256(canonical.encode("utf-8")).hexdigest()


def attach_provenance(
    artifact: dict[str, Any],
    *,
    tool_id: str,
    tool_version: str,
    input_hash: str = "",
) -> dict[str, Any]:
    """Attach a ProvenanceRecord to *artifact* and return the updated dict.

    The ``output_hash`` is computed from the artifact payload before attachment,
    so it reflects the content the runtime will actually receive.

    Note: modifies *artifact* in-place AND returns it for convenience.
    """
    output_hash = _stable_payload_hash(artifact)
    record = ProvenanceRecord(
        tool_id=tool_id,
        tool_version=tool_version,
        input_hash=input_hash,
        output_hash=output_hash,
    )
    artifact[_PROVENANCE_FIELD] = record.to_dict()
    return artifact


def verify_provenance(artifact: dict[str, Any]) -> ProvenanceRecord:
    """Verify provenance metadata is present and the output_hash is intact.

    Returns the parsed ProvenanceRecord on success.

    Raises:
        ArtifactContractViolation: if provenance is missing or the hash does not match.
    """
    if _PROVENANCE_FIELD not in artifact:
        raise ArtifactContractViolation(
            f"Artifact is missing provenance record (field: '{_PROVENANCE_FIELD}').  "
            "All runtime-consumable artifacts must carry provenance."
        )

    raw = artifact[_PROVENANCE_FIELD]
    required_prov_fields = {"tool_id", "tool_version", "input_hash", "output_hash"}
    missing = required_prov_fields - set(raw.keys())
    if missing:
        raise ArtifactContractViolation(
            f"Provenance record is incomplete; missing fields: {sorted(missing)}"
        )

    record = ProvenanceRecord.from_dict(raw)

    # Recompute the payload hash (excluding the provenance field) and compare
    recomputed = _stable_payload_hash(artifact)
    if recomputed != record.output_hash:
        raise ArtifactContractViolation(
            f"Artifact provenance output_hash mismatch.  "
            f"Expected: {record.output_hash!r}, recomputed: {recomputed!r}.  "
            "Artifact may have been tampered with or mutated after provenance attachment."
        )

    return record


def verify_artifact_file(path: Path, contract: ArtifactContract) -> ProvenanceRecord:
    """Load a JSON artifact from *path*, validate its contract, and verify provenance.

    Convenience helper for CI gate scripts.

    Raises:
        FileNotFoundError: if the path does not exist.
        ArtifactContractViolation: on contract or provenance failure.
    """
    raw_text = path.read_text(encoding="utf-8")
    artifact = json.loads(raw_text)
    validate_artifact_contract(artifact, contract)
    return verify_provenance(artifact)
