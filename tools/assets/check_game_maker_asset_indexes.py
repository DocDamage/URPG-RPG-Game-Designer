#!/usr/bin/env python3
"""Validate game-maker starter asset indexes and their referenced payloads."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

LFS_POINTER_PREFIX = b"version https://git-lfs.github.com/spec/v1\n"
STABLE_ID_RE = re.compile(r"^asset_[a-f0-9]{16}$")


@dataclass
class Issue:
    path: str
    code: str
    message: str
    record: str = ""

    def to_json(self) -> dict[str, str]:
        payload = {"path": self.path, "code": self.code, "message": self.message}
        if self.record:
            payload["record"] = self.record
        return payload


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd(), help="Repository root to validate.")
    parser.add_argument(
        "--indexes-root",
        type=Path,
        default=Path("content/asset_indexes/game_maker"),
        help="Index directory, relative to --repo-root unless absolute.",
    )
    parser.add_argument(
        "--allow-lfs-pointers",
        action="store_true",
        help="Allow unresolved Git LFS pointer files. This should stay off for release-packaged starter assets.",
    )
    parser.add_argument(
        "--skip-git-index-check",
        action="store_true",
        help="Skip checking whether the Git index still stores a referenced starter file as an LFS pointer.",
    )
    parser.add_argument("--json-output", type=Path, help="Optional JSON report output path.")
    return parser.parse_args()


def repo_relative(repo_root: Path, path: Path) -> str:
    try:
        return path.resolve().relative_to(repo_root.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def is_safe_repo_relative_path(value: str) -> bool:
    path = Path(value)
    return bool(value) and not path.is_absolute() and ".." not in path.parts


def read_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def lfs_pointer_size(data: bytes) -> int | None:
    if not data.startswith(LFS_POINTER_PREFIX):
        return None
    try:
        for line in data.decode("utf-8").splitlines():
            if line.startswith("size "):
                return int(line.split(" ", 1)[1])
    except (UnicodeDecodeError, ValueError):
        return None
    return 0


def file_pointer_size(path: Path) -> int | None:
    try:
        with path.open("rb") as stream:
            return lfs_pointer_size(stream.read(256))
    except OSError:
        return None


def git_is_work_tree(repo_root: Path) -> bool:
    try:
        completed = subprocess.run(
            ["git", "rev-parse", "--is-inside-work-tree"],
            cwd=repo_root,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.SubprocessError):
        return False
    return completed.returncode == 0 and completed.stdout.strip() == "true"


def git_path_tracked(repo_root: Path, rel_path: str) -> bool | None:
    if not git_is_work_tree(repo_root):
        return None
    try:
        completed = subprocess.run(
            ["git", "ls-files", "--error-unmatch", "--", rel_path],
            cwd=repo_root,
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    return completed.returncode == 0


def git_blob(repo_root: Path, spec: str) -> bytes | None:
    try:
        completed = subprocess.run(
            ["git", "cat-file", "-p", spec],
            cwd=repo_root,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if completed.returncode != 0:
        return None
    return completed.stdout


def git_attribute(repo_root: Path, attr: str, rel_path: str) -> str | None:
    try:
        completed = subprocess.run(
            ["git", "check-attr", attr, "--", rel_path],
            cwd=repo_root,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if completed.returncode != 0:
        return None
    # Format: path: attr: value
    parts = completed.stdout.strip().rsplit(": ", 1)
    if len(parts) != 2:
        return None
    return parts[1]


def iter_index_files(indexes_root: Path) -> Iterable[Path]:
    return sorted(path for path in indexes_root.glob("*.json") if path.is_file())


def record_id(index_path: Path, index: int, record: Any) -> str:
    if isinstance(record, dict):
        stable_id = record.get("stableId")
        if isinstance(stable_id, str) and stable_id:
            return stable_id
    return f"{index_path.name}:records[{index}]"


def validate_record(
    *,
    repo_root: Path,
    index_path: Path,
    record_index: int,
    record: Any,
    seen_ids: dict[str, str],
    allow_lfs_pointers: bool,
    skip_git_index_check: bool,
) -> list[Issue]:
    issues: list[Issue] = []
    rec_id = record_id(index_path, record_index, record)
    index_rel = repo_relative(repo_root, index_path)

    if not isinstance(record, dict):
        return [Issue(index_rel, "record_not_object", "Asset index record must be an object.", rec_id)]

    stable_id = record.get("stableId", "")
    if not isinstance(stable_id, str) or not STABLE_ID_RE.match(stable_id):
        issues.append(Issue(index_rel, "stable_id_invalid", "stableId must match ^asset_[a-f0-9]{16}$.", rec_id))
    elif stable_id in seen_ids:
        issues.append(
            Issue(
                index_rel,
                "stable_id_duplicate",
                f"stableId duplicates an earlier record in {seen_ids[stable_id]}.",
                rec_id,
            )
        )
    else:
        seen_ids[stable_id] = index_rel

    source_path_value = record.get("sourcePath", "")
    if not isinstance(source_path_value, str) or not is_safe_repo_relative_path(source_path_value):
        issues.append(Issue(index_rel, "source_path_invalid", "sourcePath must be a safe repo-relative path.", rec_id))
        return issues

    source_path = repo_root / source_path_value
    source_rel = Path(source_path_value).as_posix()
    if not source_path.is_file():
        issues.append(Issue(index_rel, "source_path_missing", "sourcePath does not exist on disk.", rec_id))
        return issues

    worktree_pointer_size = file_pointer_size(source_path)
    if worktree_pointer_size is not None and not allow_lfs_pointers:
        issues.append(
            Issue(index_rel, "source_path_unresolved_lfs_pointer", "sourcePath is an unresolved Git LFS pointer.", rec_id)
        )

    if not allow_lfs_pointers:
        filter_attr = git_attribute(repo_root, "filter", source_rel)
        if filter_attr == "lfs":
            issues.append(
                Issue(index_rel, "source_path_lfs_attribute", "sourcePath still resolves to the Git LFS filter.", rec_id)
            )

    if not skip_git_index_check:
        tracked = git_path_tracked(repo_root, source_rel)
        if tracked is False:
            issues.append(
                Issue(index_rel, "source_path_not_tracked", "sourcePath is not tracked by Git.", rec_id)
            )

    if not skip_git_index_check and not allow_lfs_pointers:
        index_blob = git_blob(repo_root, f":{source_rel}")
        if index_blob is not None and lfs_pointer_size(index_blob) is not None:
            issues.append(
                Issue(
                    index_rel,
                    "source_path_lfs_pointer_in_git_index",
                    "sourcePath is still stored as a Git LFS pointer in the Git index; re-add it after the LFS exemption.",
                    rec_id,
                )
            )

    unloadable = record.get("unloadablePayload")
    if not isinstance(unloadable, dict):
        issues.append(Issue(index_rel, "unloadable_payload_missing", "unloadablePayload is required.", rec_id))
        return issues

    if unloadable.get("policy") != "lazy":
        issues.append(Issue(index_rel, "unloadable_payload_policy_invalid", "unloadablePayload.policy must be lazy.", rec_id))
    if unloadable.get("sourcePath") != source_path_value:
        issues.append(
            Issue(index_rel, "unloadable_payload_source_mismatch", "unloadablePayload.sourcePath must match sourcePath.", rec_id)
        )

    declared_size = unloadable.get("sizeBytes")
    if not isinstance(declared_size, int) or declared_size <= 0:
        issues.append(
            Issue(index_rel, "unloadable_payload_size_invalid", "unloadablePayload.sizeBytes must be a positive integer.", rec_id)
        )
    else:
        actual_size = worktree_pointer_size if worktree_pointer_size is not None and allow_lfs_pointers else source_path.stat().st_size
        if declared_size != actual_size:
            issues.append(
                Issue(
                    index_rel,
                    "unloadable_payload_size_mismatch",
                    f"unloadablePayload.sizeBytes is {declared_size}, expected {actual_size}.",
                    rec_id,
                )
            )

    return issues


def validate_index_file(
    *,
    repo_root: Path,
    index_path: Path,
    seen_ids: dict[str, str],
    allow_lfs_pointers: bool,
    skip_git_index_check: bool,
) -> list[Issue]:
    issues: list[Issue] = []
    index_rel = repo_relative(repo_root, index_path)
    try:
        payload = read_json(index_path)
    except (OSError, json.JSONDecodeError) as exc:
        return [Issue(index_rel, "index_parse_failed", f"Could not parse index JSON: {exc}")]

    if not isinstance(payload, dict):
        return [Issue(index_rel, "index_not_object", "Asset index root must be an object.")]

    if payload.get("schemaVersion") != 1:
        issues.append(Issue(index_rel, "schema_version_invalid", "schemaVersion must be 1."))

    records = payload.get("records")
    if not isinstance(records, list):
        issues.append(Issue(index_rel, "records_missing", "records must be an array."))
        return issues

    record_count = payload.get("recordCount")
    if record_count != len(records):
        issues.append(Issue(index_rel, "record_count_mismatch", f"recordCount is {record_count}, expected {len(records)}."))

    for record_index, record in enumerate(records):
        issues.extend(
            validate_record(
                repo_root=repo_root,
                index_path=index_path,
                record_index=record_index,
                record=record,
                seen_ids=seen_ids,
                allow_lfs_pointers=allow_lfs_pointers,
                skip_git_index_check=skip_git_index_check,
            )
        )

    return issues


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    indexes_root = args.indexes_root if args.indexes_root.is_absolute() else repo_root / args.indexes_root

    issues: list[Issue] = []
    if not indexes_root.is_dir():
        issues.append(
            Issue(repo_relative(repo_root, indexes_root), "indexes_root_missing", "Game-maker asset index directory is missing.")
        )
    else:
        seen_ids: dict[str, str] = {}
        index_files = list(iter_index_files(indexes_root))
        if not index_files:
            issues.append(Issue(repo_relative(repo_root, indexes_root), "index_files_missing", "No asset index JSON files found."))
        for index_path in index_files:
            issues.extend(
                validate_index_file(
                    repo_root=repo_root,
                    index_path=index_path,
                    seen_ids=seen_ids,
                    allow_lfs_pointers=args.allow_lfs_pointers,
                    skip_git_index_check=args.skip_git_index_check,
                )
            )

    report = {
        "schema": "urpg.game_maker_asset_index_check.v1",
        "repo_root": repo_root.as_posix(),
        "indexes_root": indexes_root.as_posix(),
        "allow_lfs_pointers": args.allow_lfs_pointers,
        "issue_count": len(issues),
        "issues": [issue.to_json() for issue in issues],
    }

    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    if issues:
        print(f"Game-maker asset index validation failed with {len(issues)} issue(s).", file=sys.stderr)
        for issue in issues:
            suffix = f" [{issue.record}]" if issue.record else ""
            print(f"- {issue.path}{suffix}: {issue.code}: {issue.message}", file=sys.stderr)
        return 1

    print("Game-maker asset index validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
