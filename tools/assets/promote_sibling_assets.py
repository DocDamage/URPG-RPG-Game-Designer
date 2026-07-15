#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path


BUNDLE_PATH = Path("imports/manifests/asset_bundles/BND-011.json")
SOURCE_PATH = Path("imports/manifests/asset_sources/SRC-015.json")
ATTRIBUTION_PATH = Path("imports/reports/asset_intake/attribution/BND-011_sibling_bulk_assets.json")
REPORT_PATH = Path("imports/reports/asset_intake/sibling_bulk_asset_promotion_report.json")
LICENSE_EVIDENCE_PATH = Path("docs/compliance/asset_licenses/local-bulk-assets-CC0.md")

NORMALIZED_ROOT = Path("imports/normalized/sibling_bulk_assets")

APP_USABLE_EXTS = {
    "bmp",
    "efkefc",
    "efkmat",
    "efkmodel",
    "flac",
    "gif",
    "jpg",
    "jpeg",
    "m4a",
    "mp3",
    "ogg",
    "otf",
    "png",
    "tmx",
    "tsx",
    "ttf",
    "wav",
    "webp",
    "woff",
    "woff2",
}

EXCLUDED_DIR_NAMES = {
    ".git",
    ".gradle",
    ".idea",
    ".next",
    ".urpg",
    ".venv",
    ".vs",
    "__pycache__",
    "bin",
    "build",
    "build-debug",
    "build-local",
    "build-release",
    "cmakefiles",
    "dist",
    "library",
    "node_modules",
    "obj",
    "out",
    "target",
    "temp",
    "venv",
}


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def rel(path: Path) -> str:
    return path.as_posix()


def slug(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-") or "asset"


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def existing_payload_hashes() -> set[str]:
    hashes: set[str] = set()
    for bundle_path in Path("imports/manifests/asset_bundles").glob("BND-*.json"):
        try:
            bundle = read_json(bundle_path)
        except Exception:
            continue
        for asset in bundle.get("assets", []):
            checksum = asset.get("checksum_sha256")
            if checksum:
                hashes.add(checksum.lower())
    return hashes


def sibling_roots() -> list[Path]:
    repo_root = Path.cwd().resolve()
    roots = []
    for child in sorted(repo_root.parent.iterdir(), key=lambda item: item.name.lower()):
        if child.is_dir() and child.resolve() != repo_root:
            roots.append(child)
    return roots


def should_skip_dir(path: Path) -> bool:
    return path.name.lower() in EXCLUDED_DIR_NAMES


def iter_candidate_files() -> list[Path]:
    candidates: list[Path] = []
    for root in sibling_roots():
        stack = [root]
        while stack:
            directory = stack.pop()
            try:
                entries = list(directory.iterdir())
            except OSError:
                continue
            for entry in entries:
                if entry.is_dir():
                    if not should_skip_dir(entry):
                        stack.append(entry)
                    continue
                if not entry.is_file():
                    continue
                ext = entry.suffix.lower().lstrip(".")
                if ext in APP_USABLE_EXTS:
                    candidates.append(entry)
    candidates.sort(key=lambda item: str(item).lower())
    return candidates


def infer_category(path: Path, ext: str) -> str:
    lower = str(path).replace("\\", "/").lower()
    name = path.name.lower()
    if ext in {"ogg", "m4a", "mp3", "wav", "flac"}:
        return "audio_music" if "music" in lower or "bgm" in lower else "audio"
    if ext in {"ttf", "otf", "woff", "woff2"}:
        return "font"
    if ext in {"tmx", "tsx"}:
        return "map"
    if ext in {"efkefc", "efkmat", "efkmodel"}:
        return "vfx"
    if any(token in lower for token in ["ui", "icon", "button", "menu", "window", "hud"]):
        return "ui"
    if any(token in lower for token in ["character", "characters", "sprite", "actor", "npc", "enemy", "monster"]):
        return "character"
    if any(token in lower for token in ["tile", "tileset", "terrain", "wall", "floor"]):
        return "tileset"
    if any(token in lower for token in ["effect", "vfx", "particle", "fx", "animation"]):
        return "vfx"
    if any(token in name for token in ["bg", "background", "backdrop"]):
        return "background"
    return "prop"


def release_surfaces(category: str) -> list[str]:
    if category == "ui":
        return ["ui"]
    if category == "font":
        return ["fonts", "ui"]
    if category.startswith("audio"):
        return ["audio"]
    if category == "vfx":
        return ["battle", "map"]
    if category == "background":
        return ["title", "battle", "map"]
    if category == "character":
        return ["battle", "map"]
    if category in {"prop", "tileset", "map"}:
        return ["map"]
    return ["ui"]


def root_for(path: Path) -> Path:
    for root in sibling_roots():
        try:
            path.relative_to(root)
            return root
        except ValueError:
            continue
    return path.anchor and Path(path.anchor) or Path("external")


def source_id_for(path: Path) -> str:
    try:
        return rel(path.relative_to(Path.cwd().parent))
    except ValueError:
        return str(path)


def build_outputs(candidates: list[Path], *, dry_run: bool) -> dict:
    known_hashes = existing_payload_hashes()
    canonical_by_hash: dict[str, str] = {}
    assets: list[dict] = []
    skipped_existing_hash = 0
    bytes_total = 0
    by_root = Counter()
    by_category = Counter()
    by_ext = Counter()

    for source_path in candidates:
        ext = source_path.suffix.lower().lstrip(".")
        checksum = sha256_file(source_path)
        if checksum.lower() in known_hashes:
            skipped_existing_hash += 1
            continue

        category = infer_category(source_path, ext)
        root = root_for(source_path)
        root_name = slug(root.name)
        pack = slug(source_path.parent.name)
        target_relative = canonical_by_hash.get(checksum)
        duplicate_of = target_relative is not None
        if target_relative is None:
            filename = f"{slug(source_path.stem)[:72]}-{checksum[:12]}.{ext}"
            target_relative = rel(Path("sibling_bulk_assets") / root_name / category / pack / filename)
            canonical_by_hash[checksum] = target_relative
            if not dry_run:
                target_path = Path("imports/normalized") / target_relative
                target_path.parent.mkdir(parents=True, exist_ok=True)
                if not target_path.exists():
                    shutil.copy2(source_path, target_path)

        assets.append(
            {
                "original_relative_path": source_id_for(source_path),
                "promoted_relative_path": target_relative,
                "category": category,
                "status": "promoted",
                "release_required": False,
                "release_surfaces": release_surfaces(category),
                "license_cleared": True,
                "release_eligible": True,
                "distribution": "deferred",
                "checksum_sha256": checksum,
                "attribution_record": rel(ATTRIBUTION_PATH),
                "package_destination": "",
                "notes": (
                    "Sibling-folder bulk asset promotion under project-provided CC0/fallback license evidence."
                    + (" Duplicate payload shares the canonical promoted file." if duplicate_of else "")
                ),
            }
        )
        by_root[root.name] += 1
        by_category[category] += 1
        by_ext[ext] += 1
        bytes_total += source_path.stat().st_size

    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    source = {
        "source_id": "SRC-015",
        "repo_name": "Sibling folder bulk assets",
        "source_url": "local://C:/Users/dferr/dev",
        "capture_state": "mirrored",
        "snapshot_commit": None,
        "snapshot_date": generated_at[:10],
        "source_type": "direct_asset_pack",
        "category_tags": sorted(by_category),
        "intended_use": [
            "Release-eligible asset library payloads discovered one directory below the URPG workspace parent",
            "Project-selected export payloads after template or project selection",
        ],
        "handling_path": "direct_ingest_when_captured",
        "legal_disposition": "project_provided_cc0_public_domain_with_fallback",
        "promotion_status": "promoted",
        "notes": [
            "Generated by tools/assets/promote_sibling_assets.py.",
            f"License evidence: {rel(LICENSE_EVIDENCE_PATH)}.",
            "Default packaging is deferred until project selection; assets are not release-required.",
        ],
    }
    bundle = {
        "bundle_id": "BND-011",
        "bundle_name": "sibling_bulk_assets",
        "source_id": "SRC-015",
        "bundle_state": "promoted",
        "release_required": False,
        "release_surfaces": ["title", "map", "battle", "ui", "audio", "fonts"],
        "assets": assets,
    }
    attribution = {
        "schema": "urpg.asset_attribution.v1",
        "bundle_id": "BND-011",
        "source_id": "SRC-015",
        "generated_at": generated_at,
        "license_status": "project_provided_cc0_public_domain_with_fallback",
        "license_evidence": rel(LICENSE_EVIDENCE_PATH),
        "distribution": "deferred",
        "release_eligible": True,
        "license_cleared": True,
        "notes": [
            "Sibling-folder bulk promotion cleared by project-provided CC0/fallback license evidence.",
            "Default packaging is deferred until project selection; assets are not release-required.",
        ],
    }
    report = {
        "schema": "urpg.sibling_bulk_asset_promotion.v1",
        "generated_at": generated_at,
        "dry_run": dry_run,
        "candidate_file_count": len(candidates),
        "selected_asset_count": len(assets),
        "canonical_payload_count": len(canonical_by_hash),
        "duplicate_payload_row_count": len(assets) - len(canonical_by_hash),
        "skipped_existing_hash_count": skipped_existing_hash,
        "selected_size_bytes": bytes_total,
        "by_root": dict(sorted(by_root.items())),
        "by_category": dict(sorted(by_category.items())),
        "by_ext": dict(sorted(by_ext.items())),
        "bundle": rel(BUNDLE_PATH),
        "source_manifest": rel(SOURCE_PATH),
        "attribution_record": rel(ATTRIBUTION_PATH),
        "normalized_root": rel(NORMALIZED_ROOT),
    }
    return {"bundle": bundle, "source": source, "attribution": attribution, "report": report}


def main() -> int:
    parser = argparse.ArgumentParser(description="Promote sibling-folder assets into a release-eligible deferred bundle.")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    candidates = iter_candidate_files()
    outputs = build_outputs(candidates, dry_run=args.dry_run)
    if not args.dry_run:
        write_json(BUNDLE_PATH, outputs["bundle"])
        write_json(SOURCE_PATH, outputs["source"])
        write_json(ATTRIBUTION_PATH, outputs["attribution"])
        write_json(REPORT_PATH, outputs["report"])

    print(json.dumps(outputs["report"], indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
