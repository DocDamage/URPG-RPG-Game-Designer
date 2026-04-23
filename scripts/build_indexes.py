#!/usr/bin/env python3
"""Convenience wrapper for rebuilding retrieval-side index manifests."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    runner = repo_root / "tools" / "shared" / "job_runner" / "run_offline_job.py"
    job_files = [
        repo_root / "tools" / "shared" / "job_runner" / "example_retrieval_job.json",
        repo_root / "tools" / "shared" / "job_runner" / "example_retrieval_bundle_job.json",
    ]
    for job_file in job_files:
        command = [
            sys.executable,
            str(runner),
            "--job-file",
            str(job_file),
        ]
        completed = subprocess.run(command, check=False)
        if completed.returncode != 0:
            return completed.returncode
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
