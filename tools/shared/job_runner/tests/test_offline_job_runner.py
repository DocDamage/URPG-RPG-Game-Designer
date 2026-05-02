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


if __name__ == "__main__":
    unittest.main()
