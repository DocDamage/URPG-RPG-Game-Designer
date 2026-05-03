#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[4]
RUNNER = REPO_ROOT / "tools" / "shared" / "job_runner" / "run_offline_job.py"


class OfflineJobRunnerTests(unittest.TestCase):
    def setUp(self) -> None:
        (REPO_ROOT / ".urpg").mkdir(exist_ok=True)

    def run_job(self, job: dict) -> Path:
        with tempfile.TemporaryDirectory() as tmp:
            job_path = Path(tmp) / "job.json"
            job_path.write_text(json.dumps(job), encoding="utf-8")
            completed = subprocess.run(
                [sys.executable, str(RUNNER), "--job-file", str(job_path)],
                cwd=REPO_ROOT,
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"stdout:\n{completed.stdout}\nstderr:\n{completed.stderr}",
            )
        return REPO_ROOT / job["output"]

    def test_vision_segmentation_job_writes_manifest(self) -> None:
        with tempfile.TemporaryDirectory(dir=REPO_ROOT / ".urpg") as tmp:
            tmp_rel = Path(tmp).relative_to(REPO_ROOT).as_posix()
            output = f"{tmp_rel}/segmentation_manifest.json"
            self.run_job(
                {
                    "job_type": "vision_segmentation",
                    "source_image": "imports/normalized/src012_cc0_tiles_vfx/tilesets/grunge_tileset.png",
                    "output": output,
                    "output_root": f"{tmp_rel}/segments",
                    "asset_ids": ["grunge_tileset"],
                }
            )
            manifest = json.loads((REPO_ROOT / output).read_text(encoding="utf-8"))
            self.assertEqual(
                manifest["schema"], "content/schemas/segmentation_manifest.schema.json"
            )
            self.assertEqual(manifest["outputs"][0]["asset_id"], "grunge_tileset")

    def test_audio_processing_job_writes_manifest(self) -> None:
        with tempfile.TemporaryDirectory(dir=REPO_ROOT / ".urpg") as tmp:
            tmp_rel = Path(tmp).relative_to(REPO_ROOT).as_posix()
            output = f"{tmp_rel}/audio_manifest.json"
            self.run_job(
                {
                    "job_type": "audio_processing",
                    "source_audio": "content/fixtures/audio/silence.ogg",
                    "output": output,
                    "output_root": f"{tmp_rel}/audio",
                    "mode": "compression",
                }
            )
            manifest = json.loads((REPO_ROOT / output).read_text(encoding="utf-8"))
            self.assertEqual(
                manifest["schema"], "content/schemas/audio_tool_manifest.schema.json"
            )
            self.assertEqual(
                manifest["outputs"][0]["kind"], "encodec_compression_experiment"
            )

    def test_retrieval_jobs_write_chunk_manifest_and_bundle(self) -> None:
        with tempfile.TemporaryDirectory(dir=REPO_ROOT / ".urpg") as tmp:
            tmp_path = Path(tmp)
            source_root = tmp_path / "docs"
            source_root.mkdir()
            (source_root / "lore.txt").write_text(
                "Ancient ruins define the retrieval smoke corpus.\n"
                "Offline indexing produces artifacts for authoring only.\n",
                encoding="utf-8",
            )
            tmp_rel = tmp_path.relative_to(REPO_ROOT).as_posix()
            source_rel = source_root.relative_to(REPO_ROOT).as_posix()
            manifest_output = f"{tmp_rel}/retrieval_chunks.json"
            bundle_output = f"{tmp_rel}/retrieval_bundle.json"

            self.run_job(
                {
                    "job_type": "retrieval_chunk_manifest",
                    "source_root": source_rel,
                    "output": manifest_output,
                    "chunk_size": 80,
                    "overlap": 10,
                }
            )
            manifest = json.loads(
                (REPO_ROOT / manifest_output).read_text(encoding="utf-8")
            )
            self.assertEqual(
                manifest["schema"], "content/schemas/retrieval_chunk_manifest.schema.json"
            )
            self.assertGreaterEqual(len(manifest["chunks"]), 1)

            self.run_job(
                {
                    "job_type": "retrieval_bundle",
                    "manifest": manifest_output,
                    "output": bundle_output,
                    "dimension": 16,
                    "adapter": "builtin_hashed",
                }
            )
            bundle = json.loads((REPO_ROOT / bundle_output).read_text(encoding="utf-8"))
            self.assertEqual(
                bundle["schema"], "content/schemas/retrieval_index_bundle.schema.json"
            )
            self.assertEqual(bundle["entry_count"], len(manifest["chunks"]))
            self.assertEqual(bundle["engine"], "builtin_hashed")
            self.assertEqual(
                bundle["embedding_adapter"]["adapter_id"], "builtin_hashed"
            )


if __name__ == "__main__":
    unittest.main()
