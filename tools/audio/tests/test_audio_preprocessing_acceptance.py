#!/usr/bin/env python3
"""S33-T03: Demucs/Encodec audio preprocessing acceptance — contract, determinism, round-trip.

These acceptance tests validate the *manifest contract* that any Demucs or
Encodec audio preprocessing tool must produce before the runtime may consume
its output.  No live model is invoked; the tests work with deterministic
in-memory fixtures.

Exit criteria (S33-T03):
  - Audio stem manifest satisfies AUDIO_STEM_MANIFEST_CONTRACT field
    requirements.
  - A compliant manifest passes validate_artifact_contract without error.
  - Missing or incorrectly-prefixed schema fields raise ArtifactContractViolation.
  - Stem entries contain required per-stem fields (stem_id, label, file_path).
  - file_path must be a non-empty string (runtime locates the audio file by it).
  - A manifest serialised to JSON and reloaded produces an identical structure
    (round-trip fidelity).
  - Two independently constructed manifests from the same logical inputs compare
    equal in structure (determinism contract).
  - Additional provenance-aware field (sample_rate_hz) is preserved when present.
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

from tools.retrieval.shared.artifact_contracts import (  # noqa: E402
    AUDIO_STEM_MANIFEST_CONTRACT,
    ArtifactContractViolation,
    validate_artifact_contract,
)


# ---------------------------------------------------------------------------
# Shared fixtures
# ---------------------------------------------------------------------------


def _make_stem(
    stem_id: str,
    label: str,
    file_path: str,
    sample_rate_hz: int = 44100,
) -> dict:
    """Build a single stem entry dict."""
    return {
        "stem_id": stem_id,
        "label": label,
        "file_path": file_path,
        "sample_rate_hz": sample_rate_hz,
    }


def _make_audio_stem_manifest(
    source_asset: str,
    stems: list[dict],
    tool_id: str = "demucs_stem_separator",
    schema: str = "content/schemas/audio_stem_manifest.schema.json",
) -> dict:
    """Construct a minimal compliant audio stem manifest."""
    return {
        "schema": schema,
        "tool_id": tool_id,
        "source_asset": source_asset,
        "stems": stems,
    }


_SAMPLE_SOURCE_ASSET = "imports/audio/battle_theme_01.ogg"

_SAMPLE_STEMS = [
    _make_stem("stem_001", "drums", "output/stems/battle_theme_01_drums.wav"),
    _make_stem("stem_002", "bass", "output/stems/battle_theme_01_bass.wav"),
    _make_stem("stem_003", "other", "output/stems/battle_theme_01_other.wav"),
    _make_stem("stem_004", "vocals", "output/stems/battle_theme_01_vocals.wav"),
]


# ---------------------------------------------------------------------------
# S33-T03: Schema contract enforcement
# ---------------------------------------------------------------------------


class AudioStemManifestContractTests(unittest.TestCase):
    """Audio stem manifest satisfies AUDIO_STEM_MANIFEST_CONTRACT."""

    def _compliant_manifest(self) -> dict:
        return _make_audio_stem_manifest(_SAMPLE_SOURCE_ASSET, _SAMPLE_STEMS)

    def test_compliant_manifest_passes_contract(self) -> None:
        manifest = self._compliant_manifest()
        # Must not raise
        validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_missing_schema_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["schema"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_missing_tool_id_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["tool_id"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_missing_source_asset_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["source_asset"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_missing_stems_field_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        del manifest["stems"]
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_wrong_schema_prefix_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        manifest["schema"] = "wrong/prefix/audio_stem_manifest.schema.json"
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_empty_schema_string_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        manifest["schema"] = ""
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)

    def test_non_string_schema_raises_violation(self) -> None:
        manifest = self._compliant_manifest()
        manifest["schema"] = 99
        with self.assertRaises(ArtifactContractViolation):
            validate_artifact_contract(manifest, AUDIO_STEM_MANIFEST_CONTRACT)


# ---------------------------------------------------------------------------
# S33-T03: Per-stem field shape validation
# ---------------------------------------------------------------------------


class StemEntryShapeTests(unittest.TestCase):
    """Each stem entry must have required fields with valid values."""

    def _check_stem(self, stem: dict) -> list[str]:
        """Return a list of violation messages for *stem*."""
        violations = []
        for required_field in ("stem_id", "label", "file_path"):
            if required_field not in stem:
                violations.append(f"Stem missing required field: '{required_field}'")
        if "file_path" in stem and not isinstance(stem["file_path"], str):
            violations.append("Stem 'file_path' must be a string")
        if (
            "file_path" in stem
            and isinstance(stem["file_path"], str)
            and not stem["file_path"].strip()
        ):
            violations.append("Stem 'file_path' must not be empty or whitespace")
        if "sample_rate_hz" in stem:
            rate = stem["sample_rate_hz"]
            if not isinstance(rate, int) or rate <= 0:
                violations.append(
                    f"Stem 'sample_rate_hz' must be a positive integer, got {rate!r}"
                )
        return violations

    def test_sample_stems_are_all_valid(self) -> None:
        for stem in _SAMPLE_STEMS:
            violations = self._check_stem(stem)
            self.assertEqual(violations, [], msg=f"Unexpected violations: {violations}")

    def test_missing_stem_id_is_detected(self) -> None:
        bad = _make_stem("x", "drums", "some/file.wav")
        del bad["stem_id"]
        violations = self._check_stem(bad)
        self.assertIn("Stem missing required field: 'stem_id'", violations)

    def test_missing_label_is_detected(self) -> None:
        bad = _make_stem("s1", "drums", "some/file.wav")
        del bad["label"]
        violations = self._check_stem(bad)
        self.assertIn("Stem missing required field: 'label'", violations)

    def test_empty_file_path_is_detected(self) -> None:
        bad = _make_stem("s1", "drums", "   ")
        violations = self._check_stem(bad)
        self.assertTrue(any("empty" in v.lower() for v in violations))

    def test_zero_sample_rate_is_detected(self) -> None:
        bad = _make_stem("s1", "drums", "path.wav", sample_rate_hz=0)
        violations = self._check_stem(bad)
        self.assertTrue(any("sample_rate_hz" in v for v in violations))

    def test_negative_sample_rate_is_detected(self) -> None:
        bad = _make_stem("s1", "drums", "path.wav", sample_rate_hz=-44100)
        violations = self._check_stem(bad)
        self.assertTrue(any("sample_rate_hz" in v for v in violations))


# ---------------------------------------------------------------------------
# S33-T03: Round-trip fidelity
# ---------------------------------------------------------------------------


class AudioStemManifestRoundTripTests(unittest.TestCase):
    """JSON serialisation must not mutate any manifest field."""

    def test_manifest_round_trips_via_json(self) -> None:
        manifest = _make_audio_stem_manifest(_SAMPLE_SOURCE_ASSET, _SAMPLE_STEMS)
        serialised = json.dumps(manifest)
        reloaded = json.loads(serialised)

        self.assertEqual(manifest["schema"], reloaded["schema"])
        self.assertEqual(manifest["tool_id"], reloaded["tool_id"])
        self.assertEqual(manifest["source_asset"], reloaded["source_asset"])
        self.assertEqual(len(manifest["stems"]), len(reloaded["stems"]))

        for original, recovered in zip(manifest["stems"], reloaded["stems"]):
            self.assertEqual(original["stem_id"], recovered["stem_id"])
            self.assertEqual(original["label"], recovered["label"])
            self.assertEqual(original["file_path"], recovered["file_path"])
            if "sample_rate_hz" in original:
                self.assertEqual(
                    original["sample_rate_hz"], recovered["sample_rate_hz"]
                )

    def test_empty_stems_list_round_trips(self) -> None:
        manifest = _make_audio_stem_manifest(_SAMPLE_SOURCE_ASSET, [])
        reloaded = json.loads(json.dumps(manifest))
        self.assertEqual(reloaded["stems"], [])

    def test_extra_provenance_fields_survive_round_trip(self) -> None:
        """Additional metadata (e.g. a provenance block) must not be dropped."""
        manifest = _make_audio_stem_manifest(_SAMPLE_SOURCE_ASSET, _SAMPLE_STEMS)
        manifest["__provenance__"] = {
            "tool_id": "demucs_stem_separator",
            "tool_version": "0.1.0",
            "input_hash": "abc123",
            "output_hash": "def456",
        }
        reloaded = json.loads(json.dumps(manifest))
        self.assertIn("__provenance__", reloaded)
        self.assertEqual(
            manifest["__provenance__"]["tool_id"],
            reloaded["__provenance__"]["tool_id"],
        )


# ---------------------------------------------------------------------------
# S33-T03: Determinism contract
# ---------------------------------------------------------------------------


class AudioStemManifestDeterminismTests(unittest.TestCase):
    """Same logical inputs must produce identical manifest structure."""

    def test_two_manifests_from_same_input_are_identical(self) -> None:
        first = _make_audio_stem_manifest(
            _SAMPLE_SOURCE_ASSET, copy.deepcopy(_SAMPLE_STEMS)
        )
        second = _make_audio_stem_manifest(
            _SAMPLE_SOURCE_ASSET, copy.deepcopy(_SAMPLE_STEMS)
        )
        self.assertEqual(
            json.dumps(first, sort_keys=True),
            json.dumps(second, sort_keys=True),
        )

    def test_different_source_assets_produce_different_manifests(self) -> None:
        first = _make_audio_stem_manifest("audio/track_a.ogg", _SAMPLE_STEMS)
        second = _make_audio_stem_manifest("audio/track_b.ogg", _SAMPLE_STEMS)
        self.assertNotEqual(first["source_asset"], second["source_asset"])

    def test_stem_count_reflects_input(self) -> None:
        zero_stems = _make_audio_stem_manifest(_SAMPLE_SOURCE_ASSET, [])
        one_stem = _make_audio_stem_manifest(_SAMPLE_SOURCE_ASSET, [_SAMPLE_STEMS[0]])
        self.assertEqual(len(zero_stems["stems"]), 0)
        self.assertEqual(len(one_stem["stems"]), 1)

    def test_demucs_four_stem_labels_cover_standard_separation(self) -> None:
        """Verify standard 4-stem Demucs split (drums/bass/other/vocals) is represented."""
        labels = {s["label"] for s in _SAMPLE_STEMS}
        expected = {"drums", "bass", "other", "vocals"}
        self.assertEqual(labels, expected)


if __name__ == "__main__":
    unittest.main()
