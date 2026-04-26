#!/usr/bin/env python3
"""C++ formatting and static-analysis gate for pre-commit and CI."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path


CPP_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
TIDY_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx"}
FORMAT_BATCH_SIZE = 100
DEFAULT_TIDY_MAX_FILES = 24
EXCLUDED_PARTS = {
    ".cache",
    ".git",
    "imports",
    "third_party",
}


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def is_candidate(path: Path, tidy_only: bool, check_exists: bool = True) -> bool:
    if any(
        part in EXCLUDED_PARTS or part.startswith("build") or part == "_deps"
        for part in path.parts
    ):
        return False
    if path.suffix.lower() not in (TIDY_EXTENSIONS if tidy_only else CPP_EXTENSIONS):
        return False
    return not check_exists or path.exists()


def normalize_paths(paths: list[str], tidy_only: bool) -> list[Path]:
    root = repo_root()
    normalized: list[Path] = []
    for raw_path in paths:
        path = Path(raw_path)
        if not path.is_absolute():
            path = root / path
        try:
            relative = path.resolve().relative_to(root)
        except ValueError:
            continue
        if is_candidate(relative, tidy_only):
            normalized.append(root / relative)
    return sorted(set(normalized))


def discover_paths(tidy_only: bool) -> list[Path]:
    root = repo_root()
    completed = subprocess.run(
        ["git", "ls-files"],
        cwd=root,
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        print(completed.stderr, file=sys.stderr)
        return []

    paths: list[Path] = []
    for raw_path in completed.stdout.splitlines():
        relative = Path(raw_path)
        if is_candidate(relative, tidy_only=tidy_only, check_exists=False):
            paths.append(root / relative)
    return sorted(set(paths))


def changed_paths(tidy_only: bool) -> list[Path]:
    root = repo_root()
    base_ref = os.environ.get("URPG_CPP_GATE_BASE_REF", "HEAD")
    commands = [
        ["git", "diff", "--name-only", "--diff-filter=ACMRT", base_ref, "--"],
        ["git", "ls-files", "--others", "--exclude-standard"],
    ]
    raw_paths: list[str] = []
    for command in commands:
        completed = subprocess.run(
            command,
            cwd=root,
            check=False,
            capture_output=True,
            text=True,
        )
        if completed.returncode != 0:
            print(completed.stderr, file=sys.stderr)
            return []
        raw_paths.extend(completed.stdout.splitlines())

    return normalize_paths(raw_paths, tidy_only=tidy_only)


def run_command(command: list[str], cwd: Path) -> int:
    print(" ".join(command), flush=True)
    completed = subprocess.run(command, cwd=cwd, check=False)
    return completed.returncode


def find_tool(name: str) -> str:
    tool = shutil.which(name)
    if tool is None:
        print(f"ERROR: required tool '{name}' was not found on PATH.", file=sys.stderr)
        return ""
    return tool


def compile_database(args: argparse.Namespace) -> Path | None:
    root = repo_root()
    candidates: list[Path] = []
    if args.build_dir:
        candidates.append(Path(args.build_dir))
    if os.environ.get("URPG_CLANG_TIDY_BUILD_DIR"):
        candidates.append(Path(os.environ["URPG_CLANG_TIDY_BUILD_DIR"]))
    candidates.extend([root / "build" / "ci", root / "build" / "dev-ninja-debug"])

    for candidate in candidates:
        build_dir = candidate if candidate.is_absolute() else root / candidate
        database = build_dir / "compile_commands.json"
        if database.exists():
            return build_dir
    return None


def requires_compile_database(args: argparse.Namespace) -> bool:
    return (
        args.require_compile_database
        or os.environ.get("URPG_REQUIRE_CLANG_TIDY") == "1"
    )


def uses_mingw_compiler(build_dir: Path) -> bool:
    database = build_dir / "compile_commands.json"
    try:
        entries = json.loads(database.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return False

    for entry in entries:
        command = str(entry.get("command", "")).lower()
        if "mingw" in command or "x86_64-w64-mingw32" in command:
            return True
    return False


def check_format(files: list[Path]) -> int:
    if not files:
        print("clang-format: no C/C++ files to check.")
        return 0

    tool = find_tool("clang-format")
    if not tool:
        return 1

    exit_code = 0
    for index in range(0, len(files), FORMAT_BATCH_SIZE):
        batch = files[index : index + FORMAT_BATCH_SIZE]
        command = [tool, "--dry-run", "--Werror", *[str(path) for path in batch]]
        exit_code = max(exit_code, run_command(command, repo_root()))
    return exit_code


def check_tidy(files: list[Path], args: argparse.Namespace) -> int:
    if not files:
        print("clang-tidy: no C/C++ translation units to check.")
        return 0

    tool = find_tool("clang-tidy")
    if not tool:
        return 1

    build_dir = compile_database(args)
    if build_dir is None:
        message = (
            "clang-tidy: compile_commands.json was not found. "
            "Run `cmake --preset dev-ninja-debug` first, or set URPG_CLANG_TIDY_BUILD_DIR."
        )
        if requires_compile_database(args):
            print(f"ERROR: {message}", file=sys.stderr)
            return 1
        print(f"SKIP: {message}")
        return 0

    if (
        os.name == "nt"
        and uses_mingw_compiler(build_dir)
        and not requires_compile_database(args)
    ):
        print(
            "SKIP: clang-tidy analysis is enforced in Linux CI. "
            "Local Windows MinGW compile databases mix poorly with Windows LLVM header discovery; "
            "set URPG_REQUIRE_CLANG_TIDY=1 to force this check locally.",
            flush=True,
        )
        return 0

    exit_code = 0
    root = repo_root()
    max_files = int(os.environ.get("URPG_CLANG_TIDY_MAX_FILES", DEFAULT_TIDY_MAX_FILES))
    selected_files = files if max_files <= 0 else files[:max_files]
    if len(selected_files) < len(files):
        print(
            "clang-tidy: checking "
            f"{len(selected_files)} of {len(files)} changed translation units. "
            "Set URPG_CLANG_TIDY_MAX_FILES=0 for full local coverage.",
            flush=True,
        )

    for path in selected_files:
        command = [tool, "-p", str(build_dir), "--quiet", str(path)]
        exit_code = max(exit_code, run_command(command, root))
    return exit_code


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("format", "tidy"), required=True)
    parser.add_argument(
        "--all",
        action="store_true",
        help="Discover and check all repository C/C++ files",
    )
    parser.add_argument(
        "--changed",
        action="store_true",
        help="Check changed and untracked C/C++ files instead of the incoming file list",
    )
    parser.add_argument(
        "--build-dir", help="CMake build directory containing compile_commands.json"
    )
    parser.add_argument("--require-compile-database", action="store_true")
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    tidy_only = args.mode == "tidy"
    if args.changed:
        files = changed_paths(tidy_only=tidy_only)
    elif args.all:
        files = discover_paths(tidy_only=tidy_only)
    else:
        files = normalize_paths(args.files, tidy_only=tidy_only)
    if args.mode == "format":
        return check_format(files)
    return check_tidy(files, args)


if __name__ == "__main__":
    raise SystemExit(main())
