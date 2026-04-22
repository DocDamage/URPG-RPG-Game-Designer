#!/usr/bin/env python3
"""Rewrite space-containing Windows paths before invoking windres."""

from __future__ import annotations

import ctypes
import os
import subprocess
import sys
from pathlib import Path


def get_short_path(path: str) -> str:
    if os.name != "nt" or " " not in path:
        return path

    buffer_size = ctypes.windll.kernel32.GetShortPathNameW(path, None, 0)
    if buffer_size == 0:
        return path

    buffer = ctypes.create_unicode_buffer(buffer_size)
    result = ctypes.windll.kernel32.GetShortPathNameW(path, buffer, buffer_size)
    if result == 0:
        return path
    return buffer.value


def looks_like_path(argument: str) -> bool:
    if " " not in argument:
        return False

    candidate = Path(argument)
    if candidate.exists():
        return True

    parent = candidate.parent
    if parent and str(parent) not in ("", ".") and parent.exists():
        return True

    return any(sep in argument for sep in ("\\", "/"))


def normalize_argument(argument: str) -> str:
    return get_short_path(argument) if looks_like_path(argument) else argument


def build_command(argv: list[str]) -> list[str]:
    compiler = argv[1]
    raw_args = argv[2:]
    command = [compiler]
    pending_include = False

    for argument in raw_args:
        if pending_include:
            command.append(normalize_argument(argument))
            pending_include = False
            continue

        if argument in ("-I", "--include-dir"):
            command.append(argument)
            pending_include = True
            continue

        if argument.startswith("-I") and len(argument) > 2:
            command.append("-I" + normalize_argument(argument[2:]))
            continue

        if argument.startswith("--include-dir="):
            prefix, value = argument.split("=", 1)
            command.append(prefix + "=" + normalize_argument(value))
            continue

        command.append(normalize_argument(argument))

    return command


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: windres_wrapper.py <windres> [args...]", file=sys.stderr)
        return 2

    command = build_command(sys.argv)
    completed = subprocess.run(command, check=False)
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
