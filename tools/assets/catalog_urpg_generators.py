#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import json
import os
from pathlib import Path

GENERATOR_HINTS = {
    "SpriteGenerator": {
        "kind": "godot_sprite_generator",
        "integration": "candidate_for_character_sprite_generator_panel",
        "target_surfaces": ["editor", "generated_game_runtime"],
        "runtime_use_cases": [
            "character_creator",
            "npc_party_generator",
            "procedural_sprite_variant_generation",
        ],
        "notes": "Godot project with sprite, color scheme, map, and name generator scripts.",
    },
    "pixel planet maker": {
        "kind": "source_archives",
        "integration": "candidate_for_planet_and_space_background_generation_tools",
        "target_surfaces": ["editor", "generated_game_runtime"],
        "runtime_use_cases": [
            "space_world_preview",
            "planet_thumbnail_generation",
            "in_game_procedural_space_scenes",
        ],
        "notes": "Source archives for planet, sprite, and space background generators.",
    },
    "BackgroundGenerator": {
        "kind": "godot_space_background_generator",
        "integration": "candidate_for_background_generator_panel",
        "target_surfaces": ["editor", "generated_game_runtime"],
        "runtime_use_cases": [
            "background_generator_panel",
            "battleback_generation",
            "in_game_procedural_space_backgrounds",
        ],
        "notes": "Godot scene/scripts/shaders for procedural space backgrounds.",
    },
    "tiled-map editor": {
        "kind": "map_editor_source",
        "integration": "candidate_for_tiled_map_importer_and_editor_interop",
        "target_surfaces": ["editor", "generated_game_runtime"],
        "runtime_use_cases": [
            "tmx_import_validation",
            "tileset_collision_preview",
            "runtime_tilemap_loading_reference",
        ],
        "notes": "Tiled editor source tree and examples; treat as a tool integration reference, not bundled game art.",
    },
}

SOURCE_EXTS = {
    ".bat",
    ".c",
    ".cfg",
    ".cmake",
    ".cmd",
    ".conf",
    ".cpp",
    ".css",
    ".gd",
    ".gdshader",
    ".h",
    ".hpp",
    ".java",
    ".js",
    ".json",
    ".md",
    ".py",
    ".qbs",
    ".qml",
    ".qrc",
    ".rst",
    ".sh",
    ".ts",
    ".tscn",
    ".tres",
    ".ui",
    ".xml",
    ".yaml",
    ".yml",
}


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def rel(path: Path, repo_root: Path) -> str:
    return str(path.resolve().relative_to(repo_root)).replace("\\", "/")


def find_generator_roots(source_root: Path) -> list[Path]:
    roots: list[Path] = []
    for current, dirs, _files in os.walk(source_root, topdown=True):
        dirs[:] = [
            d for d in dirs if d not in {".git", ".godot", ".import", "__MACOSX"}
        ]
        base = Path(current)
        for name in list(dirs):
            if name in GENERATOR_HINTS:
                roots.append(base / name)
    return sorted(set(roots), key=lambda p: str(p).lower())


def summarize(root: Path, repo_root: Path) -> dict:
    file_count = 0
    source_file_count = 0
    total_bytes = 0
    extension_counts: dict[str, int] = {}
    sample_files: list[str] = []

    for current, dirs, files in os.walk(root, topdown=True):
        dirs[:] = [
            d for d in dirs if d not in {".git", ".godot", ".import", "__MACOSX"}
        ]
        base = Path(current)
        for name in files:
            path = base / name
            try:
                stat = path.stat()
            except OSError:
                continue
            file_count += 1
            total_bytes += stat.st_size
            ext = path.suffix.lower() or "(none)"
            extension_counts[ext] = extension_counts.get(ext, 0) + 1
            if ext in SOURCE_EXTS:
                source_file_count += 1
            if len(sample_files) < 12 and (
                ext in SOURCE_EXTS
                or name.lower() in {"license", "copying", "readme.md"}
            ):
                sample_files.append(rel(path, repo_root))

    root_name = root.name
    hint = GENERATOR_HINTS.get(root_name, GENERATOR_HINTS.get(root.parent.name, {}))
    return {
        "name": root_name,
        "root": rel(root, repo_root),
        "kind": hint.get("kind", "generator_or_tool_source"),
        "integration": hint.get("integration", "candidate_for_tool_review"),
        "target_surfaces": hint.get("target_surfaces", ["editor"]),
        "runtime_use_cases": hint.get("runtime_use_cases", []),
        "integration_priority": "high",
        "status": "cataloged_local_review_required",
        "file_count": file_count,
        "source_file_count": source_file_count,
        "total_bytes": total_bytes,
        "extension_counts": dict(sorted(extension_counts.items())),
        "sample_files": sample_files,
        "notes": hint.get(
            "notes",
            "Review licensing, runtime dependencies, and code quality before integration.",
        ),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Catalog tool/generator candidates from the local URPG asset drop."
    )
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--source-root", default="urpg stuff")
    parser.add_argument(
        "--report",
        default="imports/reports/asset_intake/urpg_stuff_generator_candidates.json",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    source_root = (repo_root / args.source_root).resolve()
    if not source_root.is_dir():
        raise SystemExit(f"source root not found: {source_root}")

    roots = find_generator_roots(source_root)
    report = {
        "schema": "urpg/generator_candidate_catalog/v1",
        "generated_at": iso_now(),
        "source_root": rel(source_root, repo_root),
        "candidate_count": len(roots),
        "candidates": [summarize(root, repo_root) for root in roots],
        "notes": [
            "Generator/tool candidates are cataloged for engineering review only.",
            "Do not execute or bundle tool code until licensing, dependencies, and sandboxing are reviewed.",
            "Game asset cataloging excludes audio for this refresh per user instruction.",
        ],
    }
    report_path = (repo_root / args.report).resolve()
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(
        json.dumps(
            {"report": rel(report_path, repo_root), "candidate_count": len(roots)},
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
