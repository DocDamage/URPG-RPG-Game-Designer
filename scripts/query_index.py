#!/usr/bin/env python3
"""Convenience wrapper for querying the generated docs retrieval bundle."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Query the default docs retrieval bundle.")
    parser.add_argument("--query", required=True)
    parser.add_argument("--limit", type=int, default=5)
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    command = [
        sys.executable,
        str(repo_root / "tools" / "retrieval" / "query_debugger" / "query_retrieval_bundle.py"),
        "--bundle",
        str(repo_root / "data" / "indexes" / "docs_bundle.json"),
        "--query",
        args.query,
        "--limit",
        str(args.limit),
    ]
    return subprocess.run(command, check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
