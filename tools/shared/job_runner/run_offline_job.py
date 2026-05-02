#!/usr/bin/env python3
"""Run an offline tooling job from a JSON job description."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Run a URPG offline tooling job.")
    parser.add_argument("--job-file", required=True, help="Path to a JSON job file.")
    args = parser.parse_args()

    job_file = Path(args.job_file).resolve()
    job = json.loads(job_file.read_text(encoding="utf-8"))
    job_type = job.get("job_type", "")

    repo_root = Path(__file__).resolve().parents[3]

    if job_type == "retrieval_chunk_manifest":
        command = [
            sys.executable,
            str(
                repo_root
                / "tools"
                / "retrieval"
                / "faiss_index_builder"
                / "build_chunk_manifest.py"
            ),
            "--source-root",
            str(repo_root / job["source_root"]),
            "--output",
            str(repo_root / job["output"]),
            "--chunk-size",
            str(job.get("chunk_size", 400)),
            "--overlap",
            str(job.get("overlap", 80)),
        ]
    elif job_type == "retrieval_bundle":
        command = [
            sys.executable,
            str(
                repo_root
                / "tools"
                / "retrieval"
                / "faiss_index_builder"
                / "build_retrieval_bundle.py"
            ),
            "--manifest",
            str(repo_root / job["manifest"]),
            "--output",
            str(repo_root / job["output"]),
            "--dimension",
            str(job.get("dimension", 128)),
            "--adapter",
            str(job.get("adapter", "builtin_hashed")),
            "--adapter-batch-size",
            str(job.get("adapter_batch_size", 256)),
        ]
        adapter_command_args = job.get("adapter_command_args", [])
        if isinstance(adapter_command_args, list) and adapter_command_args:
            for adapter_arg in adapter_command_args:
                command.append(f"--adapter-command-arg={adapter_arg}")
        else:
            adapter_command = str(job.get("adapter_command", ""))
            if adapter_command:
                command.extend(["--adapter-command", adapter_command])
    elif job_type == "vision_segmentation":
        command = [
            sys.executable,
            str(repo_root / "tools" / "vision" / "segment_assets.py"),
            "--source-image",
            str(repo_root / job["source_image"]),
            "--output",
            str(repo_root / job["output"]),
            "--output-root",
            str(repo_root / job["output_root"]),
        ]
        for asset_id in job.get("asset_ids", []):
            command.extend(["--asset-id", str(asset_id)])
        if job.get("manual_override", False):
            command.append("--manual-override")
    elif job_type == "audio_processing":
        command = [
            sys.executable,
            str(repo_root / "tools" / "audio" / "process_audio_assets.py"),
            "--source-audio",
            str(repo_root / job["source_audio"]),
            "--output",
            str(repo_root / job["output"]),
            "--output-root",
            str(repo_root / job["output_root"]),
            "--mode",
            str(job.get("mode", "stems")),
        ]
        if job.get("reviewed", False):
            command.append("--reviewed")
    else:
        raise SystemExit(f"Unsupported job_type: {job_type}")

    print("Running offline job:", " ".join(command))
    completed = subprocess.run(command, check=False)
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
