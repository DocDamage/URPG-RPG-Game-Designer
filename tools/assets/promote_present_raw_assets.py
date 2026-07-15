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


BUNDLE_PATH = Path("imports/manifests/asset_bundles/BND-010.json")
SOURCE_PATH = Path("imports/manifests/asset_sources/SRC-014.json")
ATTRIBUTION_PATH = Path("imports/reports/asset_intake/attribution/BND-010_present_raw_local_bulk.json")
LICENSE_EVIDENCE_PATH = Path("docs/compliance/asset_licenses/local-bulk-assets-CC0.md")
REPORT_PATH = Path("imports/reports/asset_intake/present_raw_local_bulk_promotion_report.json")
NORMALIZED_ROOT = Path("imports/normalized/present_raw_local_bulk")

RAW_ROOTS = [
    Path("imports/raw/more_assets"),
    Path("imports/raw/third_party_assets"),
    Path("imports/raw/itch_assets"),
    Path("itch/loose"),
]

APP_USABLE_EXTS = {
    "bmp",
    "csv",
    "efkefc",
    "efkmat",
    "efkmodel",
    "flac",
    "gif",
    "jpg",
    "jpeg",
    "json",
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

EXCLUDED_PARTS = {
    ".git",
    "__pycache__",
    "external-repos",
    "plugin-dropins",
    "plugin-dropins-curated",
}


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


def slug(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-") or "asset"


def rel(path: Path) -> str:
    return path.as_posix()


def already_promoted_source_paths() -> set[str]:
    promoted: set[str] = set()
    for bundle_path in Path("imports/manifests/asset_bundles").glob("BND-*.json"):
        if bundle_path == BUNDLE_PATH:
            continue
        try:
            bundle = read_json(bundle_path)
        except Exception:
            continue
        for asset in bundle.get("assets", []):
            original = asset.get("original_relative_path")
            if original:
                promoted.add(original.replace("\\", "/"))
    return promoted


def root_slug(path: Path) -> str:
    value = rel(path)
    if value.startswith("imports/raw/"):
        value = value[len("imports/raw/") :]
    return slug(value)


def infer_category(path: Path, ext: str) -> str:
    lower = rel(path).lower()
    name = path.name.lower()
    if ext in {"ogg", "m4a", "mp3", "wav", "flac"}:
        return "audio_music" if "music" in lower or "bgm" in lower else "audio"
    if ext in {"ttf", "otf", "woff", "woff2"}:
        return "font"
    if ext in {"tmx", "tsx"}:
        return "map"
    if ext in {"efkefc", "efkmat", "efkmodel"}:
        return "vfx"
    if ext in {"json", "csv"}:
        return "data"
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


def iter_candidates() -> list[Path]:
    promoted = already_promoted_source_paths()
    candidates: list[Path] = []
    for raw_root in RAW_ROOTS:
        if not raw_root.exists():
            continue
        for path in raw_root.rglob("*"):
            if not path.is_file():
                continue
            if any(part in EXCLUDED_PARTS for part in path.parts):
                continue
            source_rel = rel(path)
            if source_rel in promoted:
                continue
            ext = path.suffix.lower().lstrip(".")
            if ext not in APP_USABLE_EXTS:
                continue
            candidates.append(path)
    candidates.sort(key=lambda item: rel(item).lower())
    return candidates


def build_outputs(candidates: list[Path], *, dry_run: bool) -> dict:
    assets: list[dict] = []
    summary = Counter()
    by_ext = Counter()
    by_category = Counter()
    by_root = Counter()
    bytes_total = 0
    canonical_by_hash: dict[str, str] = {}

    for source_path in candidates:
        ext = source_path.suffix.lower().lstrip(".")
        checksum = sha256_file(source_path)
        category = infer_category(source_path, ext)
        root = next((candidate_root for candidate_root in RAW_ROOTS if source_path.is_relative_to(candidate_root)), None)
        root_name = root_slug(root) if root else "raw"
        pack = slug(source_path.parent.name)
        target_relative = canonical_by_hash.get(checksum)
        duplicate_of = target_relative is not None
        if target_relative is None:
            filename = f"{slug(source_path.stem)[:72]}-{checksum[:12]}.{ext}"
            target_relative = rel(Path("present_raw_local_bulk") / root_name / category / pack / filename)
            canonical_by_hash[checksum] = target_relative
            if not dry_run:
                target_path = Path("imports/normalized") / target_relative
                target_path.parent.mkdir(parents=True, exist_ok=True)
                if not target_path.exists():
                    shutil.copy2(source_path, target_path)

        source_rel = rel(source_path)
        assets.append(
            {
                "original_relative_path": source_rel,
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
                    "Bulk release-eligible local/library promotion from present raw payloads "
                    "under the project-provided CC0/fallback license evidence."
                    + (" Duplicate payload shares the canonical promoted file." if duplicate_of else "")
                ),
            }
        )
        summary["duplicate_payload_rows" if duplicate_of else "canonical_payloads"] += 1
        by_ext[ext] += 1
        by_category[category] += 1
        if root:
            by_root[rel(root)] += 1
        bytes_total += source_path.stat().st_size

    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    bundle = {
        "bundle_id": "BND-010",
        "bundle_name": "present_raw_local_bulk",
        "source_id": "SRC-014",
        "bundle_state": "promoted",
        "release_required": False,
        "release_surfaces": ["title", "map", "battle", "ui", "audio", "fonts"],
        "assets": assets,
    }
    source = {
        "source_id": "SRC-014",
        "repo_name": "Present local raw asset payloads",
        "source_url": "local://imports/raw",
        "capture_state": "mirrored",
        "snapshot_commit": None,
        "snapshot_date": generated_at[:10],
        "source_type": "direct_asset_pack",
        "category_tags": sorted(by_category),
        "intended_use": [
            "Local editor/library browsing from currently present raw payloads",
            "Future curated release selection after per-pack attribution review",
        ],
        "handling_path": "direct_ingest_when_captured",
        "legal_disposition": "local_dev_use_only_pending_per_pack_attribution_review",
        "promotion_status": "promoted",
        "notes": [
            "Generated by tools/assets/promote_present_raw_assets.py.",
            "BND-010 rows are distribution=deferred, release_eligible=true, and license_cleared=true.",
            f"License evidence: {rel(LICENSE_EVIDENCE_PATH)}.",
            "Exact duplicate payloads share one normalized file while preserving source-path manifest rows.",
        ],
    }
    attribution = {
        "schema": "urpg.asset_attribution.v1",
        "bundle_id": "BND-010",
        "source_id": "SRC-014",
        "generated_at": generated_at,
        "license_status": "project_provided_cc0_public_domain_with_fallback",
        "distribution": "deferred",
        "release_eligible": True,
        "license_cleared": True,
        "license_evidence": rel(LICENSE_EVIDENCE_PATH),
        "notes": [
            "Bulk local/library promotion cleared by project-provided CC0/fallback license evidence.",
            "Default packaging is deferred until project selection; assets are not release-required.",
        ],
    }
    report = {
        "schema": "urpg.present_raw_local_bulk_promotion.v1",
        "generated_at": generated_at,
        "dry_run": dry_run,
        "selected_asset_count": len(assets),
        "canonical_payload_count": summary["canonical_payloads"],
        "duplicate_payload_row_count": summary["duplicate_payload_rows"],
        "selected_size_bytes": bytes_total,
        "by_root": dict(sorted(by_root.items())),
        "by_category": dict(sorted(by_category.items())),
        "by_ext": dict(sorted(by_ext.items())),
        "bundle": rel(BUNDLE_PATH),
        "source_manifest": rel(SOURCE_PATH),
        "attribution_record": rel(ATTRIBUTION_PATH),
        "normalized_root": rel(NORMALIZED_ROOT),
    }
    return {
        "bundle": bundle,
        "source": source,
        "attribution": attribution,
        "report": report,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Promote all currently present raw assets for local/library use.")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    candidates = iter_candidates()
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
