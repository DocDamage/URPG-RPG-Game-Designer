#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import re
import shutil
from pathlib import Path


SOURCE_ID = "SRC-014"
BUNDLE_ID = "BND-010"
BUNDLE_NAME = "src014_modernui_portrait_generator"
CATALOG_ROOT = Path("imports/reports/asset_intake/ingest_20260504_promotion_catalog")
NORMALIZED_ROOT = Path("imports/normalized/src014_modernui_portrait_generator")
BUNDLE_PATH = Path("imports/manifests/asset_bundles/BND-010.json")
ATTRIBUTION_PATH = Path(
    "imports/reports/asset_intake/attribution/BND-010_src014_modernui_portrait_generator.json"
)
REPORT_PATH = Path(
    "imports/reports/asset_intake/src014_modernui_portrait_generator_promotion_report.json"
)


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()


def slug(value: str, fallback: str = "asset") -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-") or fallback


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def iter_catalog_assets(catalog_root: Path) -> list[dict]:
    assets: list[dict] = []
    for shard_path in sorted(catalog_root.glob("*.json")):
        shard = read_json(shard_path)
        assets.extend(shard.get("assets", []))
    return assets


def promoted_source_paths(bundle_root: Path, current_bundle: Path) -> set[str]:
    paths: set[str] = set()
    for bundle_path in bundle_root.glob("BND-*.json"):
        if bundle_path.resolve() == current_bundle.resolve():
            continue
        try:
            bundle = read_json(bundle_path)
        except Exception:
            continue
        for asset in bundle.get("assets", []):
            original = asset.get("original_relative_path")
            if original:
                paths.add(original.replace("\\", "/"))
    return paths


def target_relative_path(source_path: str, digest: str) -> Path:
    marker = "ingest stuff 5-4-26/ModernUI/"
    if not source_path.startswith(marker):
        raise ValueError(f"unexpected source path: {source_path}")
    local = Path(source_path[len(marker) :])
    parts = [slug(part, "part") for part in local.parts[:-1]]
    filename = f"{slug(local.stem)}-{digest[:12]}{local.suffix.lower()}"
    return Path(BUNDLE_NAME).joinpath(*parts, filename)


def category_for(asset: dict) -> str:
    ext = asset.get("ext", "").lower()
    if ext == "aseprite":
        return "portrait_generator_source_art"
    return "portrait_generator_part"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Promote SRC-014 ModernUI portrait generator loose assets into a deferred governed bundle."
    )
    parser.add_argument("--repo-root", default=".", help="Repository root.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    catalog_root = repo_root / CATALOG_ROOT
    normalized_root = repo_root / "imports" / "normalized"
    target_root = repo_root / NORMALIZED_ROOT
    bundle_path = repo_root / BUNDLE_PATH
    attribution_path = repo_root / ATTRIBUTION_PATH
    report_path = repo_root / REPORT_PATH

    if not catalog_root.is_dir():
        raise SystemExit(f"catalog root not found: {catalog_root}")

    already_promoted = promoted_source_paths(repo_root / "imports/manifests/asset_bundles", bundle_path)
    selected = [
        asset
        for asset in iter_catalog_assets(catalog_root)
        if asset.get("source_id") == SOURCE_ID
        and asset.get("pack") == "ModernUI"
        and asset.get("category") == "characters/portraits"
        and asset.get("media_kind") == "image"
        and asset.get("source_path") not in already_promoted
    ]

    assets: list[dict] = []
    copied = 0
    skipped = []
    for asset in sorted(selected, key=lambda a: a["source_path"].lower()):
        source_rel = asset["source_path"].replace("\\", "/")
        source_path = repo_root / source_rel
        if not source_path.is_file():
            skipped.append({"source_path": source_rel, "reason": "missing_source"})
            continue
        digest = sha256_file(source_path)
        if digest != asset.get("sha256"):
            skipped.append({"source_path": source_rel, "reason": "checksum_mismatch"})
            continue

        promoted_rel = target_relative_path(source_rel, digest)
        target = normalized_root / promoted_rel
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists() and sha256_file(target) != digest:
            skipped.append({"source_path": source_rel, "reason": "target_checksum_conflict"})
            continue
        if not target.exists():
            shutil.copy2(source_path, target)
            copied += 1

        surfaces = ["ui"] if asset.get("ext") == "aseprite" else ["ui", "battle", "map"]
        assets.append(
            {
                "original_relative_path": source_rel,
                "promoted_relative_path": promoted_rel.as_posix(),
                "category": category_for(asset),
                "status": "promoted",
                "release_required": False,
                "release_surfaces": surfaces,
                "license_cleared": True,
                "release_eligible": True,
                "distribution": "deferred",
                "checksum_sha256": digest,
                "attribution_record": ATTRIBUTION_PATH.as_posix(),
                "package_destination": f"share/urpg/imports/normalized/{promoted_rel.as_posix()}",
                "notes": (
                    "ModernUI portrait generator asset promoted for release-eligible "
                    "app/library use; default export bundling is deferred until a project "
                    "explicitly selects the asset."
                ),
            }
        )

    bundle = {
        "bundle_id": BUNDLE_ID,
        "bundle_name": BUNDLE_NAME,
        "source_id": SOURCE_ID,
        "bundle_state": "promoted",
        "release_required": False,
        "release_surfaces": ["ui", "battle", "map"],
        "assets": assets,
    }
    attribution = {
        "asset_id": NORMALIZED_ROOT.as_posix(),
        "source_id": SOURCE_ID,
        "source_repo": "Local 2026-05-04 asset drop with ModernUI",
        "source_url": "local://asset-drop/2026-05-04",
        "original_relative_path": "ingest stuff 5-4-26/ModernUI",
        "author": "ModernUI local asset author; no attribution required under CC0",
        "license": "CC0-1.0 / Public Domain evidence reviewed from ModernUI/license.txt",
        "commercial_use_allowed": True,
        "redistribution_allowed": True,
        "promotion_status": "promoted",
        "bundle_id": BUNDLE_ID,
        "promoted_asset_count": len(assets),
        "license_evidence": [
            "ingest stuff 5-4-26/ModernUI/license.txt",
            "imports/manifests/asset_sources/SRC-014.json",
        ],
        "notes": [
            "Promotion includes ModernUI portrait-generator PNG parts plus Aseprite source art.",
            "Assets are release-eligible but distribution is deferred so default exports remain bounded.",
        ],
    }
    report = {
        "schema": "urpg/src014_modernui_portrait_generator_promotion_report/v1",
        "generated_at": now_utc(),
        "source_id": SOURCE_ID,
        "bundle_id": BUNDLE_ID,
        "selected_asset_count": len(selected),
        "promoted_asset_count": len(assets),
        "copied_file_count": copied,
        "skipped_count": len(skipped),
        "skipped": skipped,
        "normalized_root": NORMALIZED_ROOT.as_posix(),
        "bundle_manifest": BUNDLE_PATH.as_posix(),
        "attribution_record": ATTRIBUTION_PATH.as_posix(),
    }

    write_json(bundle_path, bundle)
    write_json(attribution_path, attribution)
    write_json(report_path, report)

    print(f"Promoted ModernUI portrait-generator assets: {len(assets)}")
    print(f"Copied files: {copied}, skipped: {len(skipped)}")
    print(f"Bundle manifest written: {BUNDLE_PATH.as_posix()}")
    print(f"Promotion report written: {REPORT_PATH.as_posix()}")
    return 0 if not skipped else 1


if __name__ == "__main__":
    raise SystemExit(main())
