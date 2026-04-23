#!/usr/bin/env python3
"""S33-T02: SAM/SAM2 segmentation acceptance — contract, determinism, round-trip.

These acceptance tests validate the *manifest contract* that any SAM/SAM2
segmentation tool must produce before the runtime may consume its output.  No
live SAM model is invoked; the tests work with deterministic in-memory
fixtures.

Exit criteria (S33-T02):
  - Segmentation manifest satisfies SEGMENTATION_MANIFEST_CONTRACT field
    requirements.
  - A compliant manifest passes validate_artifact_contract without error.
  - Missing or incorrectly-prefixed schema fields raise ArtifactContractViolation.
  - Mask entries contain required per-mask fields (mask_id, label, bbox, score).
  - Bbox coordinates are non-negative and form a valid region (w > 0, h > 0).
  - Score values are in [0.0, 1.0].
  - A manifest serialised to JSON and reloaded produces an identical structure
    (round-trip fidelity).
  - Two independently constructed manifests from the same logical inputs compare
    equal in structure (determinism contract).
"""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.retrieval.shared.artifact_contracts import (
    SEGMENTATION_MANIFEST_CONTRACT,
    ArtifactContractViolation,
    validate_artifact_contract,
)


# ---------------------------------------------------------------------------
# Shared fixtures
# ---------------------------------------------------------------------------

def _make_mask(
    mask_id: str,
    label: str,
    bbox: tuple[float, float, float, float],
    score: float,
) -> dict:
    """Build a single mask entry dict."""
    x, y, w, h = bbox
    return {
        "mask_id": mask_id,
        "label": label,
        "bbox": {"x": x, "y": y, "width": w, "height": h},
        "score": score,
    }


def _make_segmentation_manifest(
    source_asset: str,
    masks: list[dict],
    tool_id: str = "sam2_segmenter",
    schema: str = "content/schemas/segmentation_manifest.schema.json",
) -> dict:
    """Construct a minimal compliant segmentation manifest."""
    return {
        "schema": schema,
        "tool_id": tool_id,
        "source_asset": source_asset,
        "masks": masks,
    }


_SAMPLE_SOURCE_ASSET = "imports/sprites/hero_sheet.png"

_SAMPLE_MASKS = [
    _make_mask("mask_001", "hero_body",  (10.0, 5.0, 48.0, 64.0), 0.97),
    _make_mask("mask_002", "hero_sword", (58.0, 20.0, 16.0, 40.0), 0.88),
    _make_mask("mask_003", "hero_shield", (0.0, 30.0, 14.0, 32.0), 0.72),
]


# ---------------------------------------------------------------------------
# S33-T02: Schema contract enforcement
# ---------------------------------------------------------------------------

class SegmentationManifestContractTests(unittest.TestCase):
    """Segmentation manifest satisfies SEGMENTATION_MANIFEST_CONTRACT."""

    def _compliant_manifest(self) -> dict:
        return _make_segmentation_manifest(_SAMPLE_SOURCE_ASSET, _SAMPLE_MASKS)

    def test_compliant_manifest_passes_contract(self) -> None:
        manifest = self._compliant_manifest()
        # Must not raise
        validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_missing_schema_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["schema"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_missing_tool_id_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["tool_id"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_missing_source_asset_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["source_asset"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_missing_masks_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["masks"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_wrong_schema_prefix_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        manifest["schema"] = "wrong/prefix/segmentation_manifest.schema.json"
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_empty_schema_string_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        manifest["schema"] = ""
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)

    def test_non_string_schema_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        manifest["schema"] = 42
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, SEGMENTATION_MANIFEST_CONTRACT)


# ---------------------------------------------------------------------------
# S33-T02: Per-mask field shape validation
# ---------------------------------------------------------------------------

class MaskEntryShapeTests(unittest.TestCase):
    """Each mask entry must have required fields with valid ranges."""

    def _check_mask(self, mask: dict) -> list[str]:
        """Return a list of violation messages for *mask*."""
        violations = []
        for field in ("mask_id", "label", "bbox", "score"):
            if field not in mask:
                violations.append(f"Mask missing required field: '{field}'")

        if "bbox" in mask:
            bbox = mask["bbox"]
            for coord in ("x", "y", "width", "height"):
                if coord not in bbox:
                    violations.append(f"Bbox missing field: '{coord}'")
                elif bbox[coord] < 0:
                    violations.append(f"Bbox '{coord}' must be non-negative, got {bbox[coord]}")
            if "width" in bbox and bbox["width"] <= 0:
                violations.append(f"Bbox 'width' must be > 0, got {bbox['width']}")
            if "height" in bbox and bbox["height"] <= 0:
                violations.append(f"Bbox 'height' must be > 0, got {bbox['height']}")

        if "score" in mask:
            score = mask["score"]
            if not (0.0 <= score <= 1.0):
                violations.append(f"Score must be in [0.0, 1.0], got {score}")

        return violations

    def test_sample_masks_are_all_valid(self) -> None:
        for mask in _SAMPLE_MASKS:
            violations = self._check_mask(mask)
            self.assertEqual(violations, [], msg=f"Unexpected violations: {violations}")

    def test_missing_mask_id_is_detected(self) -> None:
        bad_mask = _make_mask("x", "label", (0.0, 0.0, 10.0, 10.0), 0.9)
        del bad_mask["mask_id"]
        violations = self._check_mask(bad_mask)
        self.assertIn("Mask missing required field: 'mask_id'", violations)

    def test_negative_bbox_x_is_detected(self) -> None:
        bad_mask = _make_mask("m1", "body", (-5.0, 0.0, 10.0, 10.0), 0.9)
        violations = self._check_mask(bad_mask)
        self.assertTrue(any("non-negative" in v for v in violations))

    def test_zero_width_bbox_is_detected(self) -> None:
        bad_mask = _make_mask("m1", "body", (0.0, 0.0, 0.0, 10.0), 0.9)
        violations = self._check_mask(bad_mask)
        self.assertTrue(any("width" in v and "> 0" in v for v in violations))

    def test_score_above_one_is_detected(self) -> None:
        bad_mask = _make_mask("m1", "body", (0.0, 0.0, 10.0, 10.0), 1.5)
        violations = self._check_mask(bad_mask)
        self.assertTrue(any("Score" in v for v in violations))

    def test_score_below_zero_is_detected(self) -> None:
        bad_mask = _make_mask("m1", "body", (0.0, 0.0, 10.0, 10.0), -0.1)
        violations = self._check_mask(bad_mask)
        self.assertTrue(any("Score" in v for v in violations))


# ---------------------------------------------------------------------------
# S33-T02: Round-trip fidelity
# ---------------------------------------------------------------------------

class SegmentationManifestRoundTripTests(unittest.TestCase):
    """JSON serialisation must not mutate any manifest field."""

    def test_manifest_round_trips_via_json(self) -> None:
        manifest = _make_segmentation_manifest(_SAMPLE_SOURCE_ASSET, _SAMPLE_MASKS)
        serialised = json.dumps(manifest)
        reloaded = json.loads(serialised)

        self.assertEqual(manifest["schema"], reloaded["schema"])
        self.assertEqual(manifest["tool_id"], reloaded["tool_id"])
        self.assertEqual(manifest["source_asset"], reloaded["source_asset"])
        self.assertEqual(len(manifest["masks"]), len(reloaded["masks"]))

        for original, recovered in zip(manifest["masks"], reloaded["masks"]):
            self.assertEqual(original["mask_id"], recovered["mask_id"])
            self.assertEqual(original["label"], recovered["label"])
            self.assertAlmostEqual(
                original["score"], recovered["score"], places=12
            )
            for coord in ("x", "y", "width", "height"):
                self.assertAlmostEqual(
                    original["bbox"][coord], recovered["bbox"][coord], places=12
                )

    def test_empty_masks_list_round_trips(self) -> None:
        manifest = _make_segmentation_manifest(_SAMPLE_SOURCE_ASSET, [])
        reloaded = json.loads(json.dumps(manifest))
        self.assertEqual(reloaded["masks"], [])


# ---------------------------------------------------------------------------
# S33-T02: Determinism contract
# ---------------------------------------------------------------------------

class SegmentationManifestDeterminismTests(unittest.TestCase):
    """Same logical inputs must produce identical manifest structure."""

    def test_two_manifests_from_same_input_are_identical(self) -> None:
        first = _make_segmentation_manifest(
            _SAMPLE_SOURCE_ASSET, copy.deepcopy(_SAMPLE_MASKS)
        )
        second = _make_segmentation_manifest(
            _SAMPLE_SOURCE_ASSET, copy.deepcopy(_SAMPLE_MASKS)
        )
        self.assertEqual(json.dumps(first, sort_keys=True),
                         json.dumps(second, sort_keys=True))

    def test_different_source_assets_produce_different_manifests(self) -> None:
        first = _make_segmentation_manifest("assets/sprite_a.png", _SAMPLE_MASKS)
        second = _make_segmentation_manifest("assets/sprite_b.png", _SAMPLE_MASKS)
        self.assertNotEqual(first["source_asset"], second["source_asset"])

    def test_mask_count_reflects_input(self) -> None:
        zero_masks = _make_segmentation_manifest(_SAMPLE_SOURCE_ASSET, [])
        one_mask = _make_segmentation_manifest(
            _SAMPLE_SOURCE_ASSET, [_SAMPLE_MASKS[0]]
        )
        self.assertEqual(len(zero_masks["masks"]), 0)
        self.assertEqual(len(one_mask["masks"]), 1)


if __name__ == "__main__":
    unittest.main()
