#!/usr/bin/env python3
"""Mirror a UI asset folder and generate theme metadata plus completeness reports."""

from __future__ import annotations

import argparse
import json
import re
import shutil
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


ASSET_EXTENSIONS = {".png", ".aseprite"}

GAME_UI_REQUIRED = {
    "button_states",
    "panel_frame",
    "icons",
    "bars",
}

EDITOR_THEME_REQUIRED = {
    "button_states",
    "input_field_states",
    "select_dropdown_states",
    "toggle_checkbox_radio",
    "panel_frame",
    "modal_window_frame",
    "tab_states",
    "menu_item_states",
    "list_tree_row_states",
    "scrollbar_track_thumb",
    "splitter_resize_handle",
    "toolbar_button_states",
    "icon_semantic_aliases",
    "font_pairing",
    "nine_slice_metadata",
    "scale_policy",
    "color_tokens",
}


@dataclass(frozen=True)
class GenerationResult:
    asset_count: int
    manifest_path: Path
    report_path: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--theme-id", required=True)
    parser.add_argument("--display-name", required=True)
    parser.add_argument("--raw-output", type=Path, required=True)
    parser.add_argument("--asset-output", type=Path, required=True)
    parser.add_argument("--manifest-output", type=Path, required=True)
    parser.add_argument("--report-output", type=Path, required=True)
    parser.add_argument("--surface", action="append", default=["game_ui_theme"])
    return parser.parse_args()


def slug(value: str) -> str:
    value = value.strip().lower()
    value = re.sub(r"[^a-z0-9]+", "_", value)
    value = re.sub(r"_+", "_", value)
    return value.strip("_") or "asset"


def relative_posix(repo_root: Path, path: Path) -> str:
    return path.resolve().relative_to(repo_root.resolve()).as_posix()


def mirror_tree(source_root: Path, destination: Path) -> None:
    if destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True, exist_ok=True)
    for path in source_root.rglob("*"):
        if not path.is_file():
            continue
        target = destination / path.relative_to(source_root)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)


def copy_runtime_assets(source_root: Path, destination: Path) -> list[Path]:
    if destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True, exist_ok=True)
    copied: list[Path] = []
    for path in sorted(source_root.rglob("*"), key=lambda item: item.as_posix().lower()):
        if not path.is_file() or path.suffix.lower() not in ASSET_EXTENSIONS:
            continue
        target = destination / path.relative_to(source_root)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)
        copied.append(target)
    return copied


def image_info(repo_root: Path, path: Path) -> dict:
    rel_path = relative_posix(repo_root, path)
    if path.suffix.lower() != ".png":
        return {"path": rel_path, "kind": "source"}
    with Image.open(path) as image:
        return {
            "path": rel_path,
            "kind": "image",
            "width": image.width,
            "height": image.height,
            "mode": image.mode,
            "transparent": image.mode == "RGBA",
        }


def classify_assets(repo_root: Path, assets: list[Path]) -> dict:
    components = {
        "buttons": {"states": []},
        "inputs": [],
        "selects": [],
        "toggles": [],
        "frames": [],
        "bars": [],
        "icons": [],
        "handles": [],
        "slots": [],
        "spritesheets": [],
        "sourceFiles": [],
    }
    for asset in assets:
        lower = asset.name.lower()
        rel = relative_posix(repo_root, asset)
        if asset.suffix.lower() == ".aseprite":
            components["sourceFiles"].append(rel)
        elif "button" in lower:
            components["buttons"]["states"].append(rel)
        elif "input" in lower:
            components["inputs"].append(rel)
        elif "select" in lower or "dropdown" in lower:
            components["selects"].append(rel)
        elif "icon" in lower:
            components["icons"].append(rel)
        elif "toggle" in lower or "check" in lower:
            components["toggles"].append(rel)
        elif "frame" in lower or "panel" in lower or "banner" in lower or "card" in lower:
            components["frames"].append(rel)
        elif "bar" in lower or "fill" in lower:
            components["bars"].append(rel)
        elif "handle" in lower:
            components["handles"].append(rel)
        elif "slot" in lower:
            components["slots"].append(rel)
        elif "spritesheet" in lower:
            components["spritesheets"].append(rel)
        else:
            components["icons"].append(rel)
    return components


def present_checks(components: dict, manifest: dict) -> set[str]:
    present = set()
    if len(components["buttons"]["states"]) >= 4:
        present.add("button_states")
        present.add("toolbar_button_states")
    if components["inputs"]:
        present.add("input_field_states")
    if components["selects"]:
        present.add("select_dropdown_states")
    if components["toggles"]:
        present.add("toggle_checkbox_radio")
    if components["frames"]:
        present.add("panel_frame")
        present.add("modal_window_frame")
    if components["bars"]:
        present.add("bars")
    if components["icons"]:
        present.add("icons")
    if components["handles"]:
        present.add("splitter_resize_handle")
    if manifest.get("semanticIconAliases"):
        present.add("icon_semantic_aliases")
    if manifest.get("fontPairing"):
        present.add("font_pairing")
    if manifest.get("nineSlice"):
        present.add("nine_slice_metadata")
    if manifest.get("scalePolicy"):
        present.add("scale_policy")
    if manifest.get("colorTokens"):
        present.add("color_tokens")
    return present


def check_status(required: set[str], present: set[str]) -> dict:
    missing = sorted(required - present)
    return {"status": "pass" if not missing else "fail", "missing": missing}


def build_manifest(
    *,
    repo_root: Path,
    theme_id: str,
    display_name: str,
    raw_output: Path,
    asset_output: Path,
    assets: list[Path],
    initial_surfaces: list[str],
) -> dict:
    components = classify_assets(repo_root, assets)
    manifest = {
        "schemaVersion": 2,
        "themeId": theme_id,
        "displayName": display_name,
        "level": "candidate",
        "surfaces": sorted(set(initial_surfaces)),
        "source": {
            "rawPath": relative_posix(repo_root, raw_output),
            "assetPath": relative_posix(repo_root, asset_output),
            "license": "CC0-1.0",
        },
        "allowIncompleteRuntimeSelection": False,
        "bypassPolicy": {
            "developerSetting": True,
            "settingsCheckbox": True,
            "perThemeUseAnywayWarning": True,
        },
        "components": components,
        "semanticIconAliases": {},
        "fontPairing": {},
        "nineSlice": {},
        "scalePolicy": {},
        "colorTokens": {},
    }
    present = present_checks(components, manifest)
    game = check_status(GAME_UI_REQUIRED, present)
    editor = check_status(EDITOR_THEME_REQUIRED, present)
    if game["status"] == "pass":
        manifest["level"] = "game_ui_ready"
    manifest["themeCompleteness"] = {
        "gameUiReady": game,
        "editorThemeReady": editor,
    }
    return manifest


def write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def generate_theme(
    *,
    repo_root: Path,
    source_root: Path,
    theme_id: str,
    display_name: str,
    raw_output: Path,
    asset_output: Path,
    manifest_output: Path,
    report_output: Path,
    initial_surfaces: list[str],
) -> GenerationResult:
    repo_root = repo_root.resolve()
    source_root = (repo_root / source_root).resolve() if not source_root.is_absolute() else source_root.resolve()
    raw_output = (repo_root / raw_output).resolve() if not raw_output.is_absolute() else raw_output.resolve()
    asset_output = (repo_root / asset_output).resolve() if not asset_output.is_absolute() else asset_output.resolve()
    manifest_output = (
        (repo_root / manifest_output).resolve() if not manifest_output.is_absolute() else manifest_output.resolve()
    )
    report_output = (repo_root / report_output).resolve() if not report_output.is_absolute() else report_output.resolve()

    mirror_tree(source_root, raw_output)
    assets = copy_runtime_assets(source_root, asset_output)
    manifest = build_manifest(
        repo_root=repo_root,
        theme_id=theme_id,
        display_name=display_name,
        raw_output=raw_output,
        asset_output=asset_output,
        assets=assets,
        initial_surfaces=initial_surfaces,
    )
    write_json(manifest_output, manifest)
    report = {
        "schema": "urpg/ui_theme_validation_report/v1",
        "themeId": theme_id,
        "assetCount": len(assets),
        "checks": {
            "gameUiReady": manifest["themeCompleteness"]["gameUiReady"],
            "editorThemeReady": manifest["themeCompleteness"]["editorThemeReady"],
        },
        "assets": [image_info(repo_root, path) for path in assets],
    }
    write_json(report_output, report)
    return GenerationResult(len(assets), manifest_output.relative_to(repo_root), report_output.relative_to(repo_root))


def main() -> int:
    args = parse_args()
    result = generate_theme(
        repo_root=args.repo_root,
        source_root=args.source_root,
        theme_id=args.theme_id,
        display_name=args.display_name,
        raw_output=args.raw_output,
        asset_output=args.asset_output,
        manifest_output=args.manifest_output,
        report_output=args.report_output,
        initial_surfaces=args.surface,
    )
    print(f"Generated UI theme manifest for {args.theme_id}: {result.asset_count} assets.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
