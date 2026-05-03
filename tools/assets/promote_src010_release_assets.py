#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import struct
from datetime import datetime, timezone
from pathlib import Path


SOURCE_ID = "SRC-010"
RAW_ROOT = Path("imports/raw/more_assets_to_ingest")
CATALOG_SHARD_ROOT = Path(
    "imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog"
)
CANDIDATE_REPORT = Path(
    "imports/reports/more_assets_to_ingest/promotion_candidate_report.json"
)
BUNDLE_PATH = Path("imports/manifests/asset_bundles/BND-006.json")
ATTRIBUTION_PATH = Path(
    "imports/reports/asset_intake/attribution/BND-006_src010_cc0_release_bulk.json"
)
SCAN_REPORT_PATH = Path(
    "imports/reports/asset_intake/src010_release_bulk_scan_report.json"
)
NORMALIZED_ROOT = Path("imports/normalized/src010_cc0_release_bulk")

APP_USABLE_EXTS = {"png", "gif", "jpg", "jpeg", "ogg", "ttf", "otf", "woff", "woff2"}
EXCLUDED_CATEGORIES = {"archives", "documentation", "tooling/source"}


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


def png_size(path: Path) -> tuple[int, int] | None:
    with path.open("rb") as stream:
        data = stream.read(24)
    if len(data) >= 24 and data[:8] == b"\x89PNG\r\n\x1a\n" and data[12:16] == b"IHDR":
        return struct.unpack(">II", data[16:24])
    return None


def gif_size(path: Path) -> tuple[int, int] | None:
    with path.open("rb") as stream:
        data = stream.read(10)
    if len(data) >= 10 and data[:6] in (b"GIF87a", b"GIF89a"):
        return struct.unpack("<HH", data[6:10])
    return None


def jpeg_size(path: Path) -> tuple[int, int] | None:
    with path.open("rb") as stream:
        if stream.read(2) != b"\xff\xd8":
            return None
        while True:
            prefix = stream.read(1)
            if not prefix:
                return None
            if prefix != b"\xff":
                continue
            marker = stream.read(1)
            while marker == b"\xff":
                marker = stream.read(1)
            if not marker:
                return None
            code = marker[0]
            if code in {0xD8, 0xD9}:
                continue
            size_data = stream.read(2)
            if len(size_data) != 2:
                return None
            size = struct.unpack(">H", size_data)[0]
            if code in {
                0xC0,
                0xC1,
                0xC2,
                0xC3,
                0xC5,
                0xC6,
                0xC7,
                0xC9,
                0xCA,
                0xCB,
                0xCD,
                0xCE,
                0xCF,
            }:
                payload = stream.read(size - 2)
                if len(payload) >= 5:
                    return struct.unpack(">H", payload[3:5])[0], struct.unpack(
                        ">H", payload[1:3]
                    )[0]
                return None
            stream.seek(size - 2, 1)


def image_size(path: Path, ext: str) -> tuple[int, int] | None:
    try:
        if ext == "png":
            return png_size(path)
        if ext == "gif":
            return gif_size(path)
        if ext in {"jpg", "jpeg"}:
            return jpeg_size(path)
    except OSError:
        return None
    return None


def valid_font(path: Path, ext: str) -> bool:
    try:
        with path.open("rb") as stream:
            magic = stream.read(4)
    except OSError:
        return False
    if ext == "woff":
        return magic == b"wOFF"
    if ext == "woff2":
        return magic == b"wOF2"
    if ext == "otf":
        return magic == b"OTTO"
    if ext == "ttf":
        return magic in {b"\x00\x01\x00\x00", b"true", b"ttcf"}
    return False


def valid_ogg(path: Path) -> bool:
    try:
        with path.open("rb") as stream:
            return stream.read(4) == b"OggS"
    except OSError:
        return False


def technical_ok(path: Path, ext: str) -> tuple[bool, str, tuple[int, int] | None]:
    if not path.is_file():
        return False, "missing_file", None
    if path.stat().st_size <= 0:
        return False, "empty_file", None
    if ext in {"png", "gif", "jpg", "jpeg"}:
        size = image_size(path, ext)
        if size is None or size[0] <= 0 or size[1] <= 0:
            return False, "invalid_image_header", None
        return True, "ok", size
    if ext == "ogg":
        return (
            (True, "ok", None)
            if valid_ogg(path)
            else (False, "invalid_ogg_header", None)
        )
    if ext in {"ttf", "otf", "woff", "woff2"}:
        return (
            (True, "ok", None)
            if valid_font(path, ext)
            else (False, "invalid_font_header", None)
        )
    return False, "unsupported_extension", None


def category_to_bundle_category(category: str, ext: str) -> str:
    if category in {"characters", "characters/portraits"}:
        return "prototype_sprite"
    if category in {"backgrounds", "props", "tilesets"}:
        return "tileset" if category == "tilesets" else "background"
    if category == "ui":
        return "ui_frames_chrome"
    if category == "vfx":
        return "vfx_sheet"
    if category in {"audio", "audio/music"}:
        return "audio"
    if category == "fonts" or ext in {"ttf", "otf", "woff", "woff2"}:
        return "font"
    return category.replace("/", "_")


def category_surfaces(category: str) -> list[str]:
    if category == "ui":
        return ["ui"]
    if category == "fonts":
        return ["fonts", "ui"]
    if category in {"audio", "audio/music"}:
        return ["audio"]
    if category == "vfx":
        return ["battle", "map"]
    if category == "backgrounds":
        return ["title", "battle", "map"]
    if category in {"characters", "characters/portraits"}:
        return ["battle", "map"]
    if category in {"props", "tilesets"}:
        return ["map"]
    return ["ui"]


def cc0_packs(candidate_report: dict) -> set[str]:
    packs = set()
    for pack in candidate_report.get("all_packs", []):
        if (
            pack.get("promotion_recommendation") == "candidate_review"
            and pack.get("license_confidence") == "reviewable_permissive_evidence"
            and pack.get("license_statuses") == ["permissive_cc0_or_public_domain"]
        ):
            packs.add(pack["pack"])
    return packs


def license_evidence_by_pack(candidate_report: dict) -> dict[str, list[str]]:
    evidence: dict[str, list[str]] = {}
    for record in candidate_report.get("license_evidence", []):
        pack = record.get("pack")
        path = record.get("source_path")
        if pack and path:
            evidence.setdefault(pack, []).append(path)
    return evidence


def iter_catalog_assets() -> list[dict]:
    assets: list[dict] = []
    for shard_path in sorted(CATALOG_SHARD_ROOT.glob("*.json")):
        shard = read_json(shard_path)
        assets.extend(shard.get("assets", []))
    return assets


def promoted_source_paths() -> set[str]:
    paths = set()
    bundle_root = Path("imports/manifests/asset_bundles")
    for bundle_path in bundle_root.glob("BND-*.json"):
        if bundle_path == BUNDLE_PATH:
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


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Promote all release-safe SRC-010 app-usable assets."
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    candidate_report = read_json(CANDIDATE_REPORT)
    allowed_packs = cc0_packs(candidate_report)
    evidence_by_pack = license_evidence_by_pack(candidate_report)
    already_promoted = promoted_source_paths()
    selected: list[tuple[dict, Path, str, tuple[int, int] | None]] = []
    rejected: list[dict] = []
    seen_hashes: set[str] = set()

    for asset in iter_catalog_assets():
        source_path = asset.get("source_path", "").replace("\\", "/")
        ext = asset.get("ext", "").lower()
        category = asset.get("category", "")
        pack = asset.get("pack", "")
        reason = ""
        if source_path in already_promoted:
            reason = "already_promoted_elsewhere"
        elif pack not in allowed_packs:
            reason = "pack_not_cc0_public_domain_reviewed"
        elif asset.get("status") == "duplicate" or asset.get("sha256") in seen_hashes:
            reason = "duplicate_payload"
        elif ext not in APP_USABLE_EXTS:
            reason = "extension_not_app_usable"
        elif category in EXCLUDED_CATEGORIES:
            reason = "category_not_runtime_payload"
        elif not evidence_by_pack.get(pack):
            reason = "missing_pack_license_evidence"

        source_file = Path(source_path)
        size = None
        if not reason:
            ok, reason, size = technical_ok(source_file, ext)
            if ok:
                digest = sha256_file(source_file)
                if digest != asset.get("sha256"):
                    reason = "catalog_checksum_mismatch"
                else:
                    selected.append((asset, source_file, digest, size))
                    seen_hashes.add(digest)
                    continue

        rejected.append(
            {
                "source_path": source_path,
                "pack": pack,
                "category": category,
                "ext": ext,
                "reason": reason,
            }
        )

    selected.sort(key=lambda item: item[0]["source_path"])
    bundle_assets = []
    promoted_assets = []
    copied_bytes = 0
    for asset, source_file, digest, _size in selected:
        category = asset["category"]
        ext = asset["ext"].lower()
        pack_slug = slug(asset["pack"])
        stem_slug = slug(Path(asset["filename"]).stem)
        promoted_rel = f"src010_cc0_release_bulk/{category.replace('/', '_')}/{pack_slug}/{stem_slug}-{digest[:12]}.{ext}"
        target = Path("imports/normalized") / promoted_rel
        if not args.dry_run:
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
        copied_bytes += source_file.stat().st_size
        promoted_assets.append(promoted_rel)
        bundle_assets.append(
            {
                "original_relative_path": asset["source_path"],
                "promoted_relative_path": promoted_rel,
                "category": category_to_bundle_category(category, ext),
                "status": "promoted",
                "release_required": False,
                "release_surfaces": category_surfaces(category),
                "license_cleared": True,
                "release_eligible": True,
                "distribution": "deferred",
                "checksum_sha256": digest,
                "attribution_record": str(ATTRIBUTION_PATH).replace("\\", "/"),
                "package_destination": f"share/urpg/imports/normalized/{promoted_rel}",
                "notes": "SRC-010 CC0/public-domain reviewed payload promoted for release-eligible app/library use; default export bundling is deferred until a project explicitly selects the asset.",
            }
        )

    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    bundle = {
        "bundle_id": "BND-006",
        "bundle_name": "src010_cc0_release_bulk",
        "source_id": SOURCE_ID,
        "bundle_state": "promoted",
        "release_required": False,
        "release_surfaces": ["title", "map", "battle", "ui", "audio", "fonts"],
        "assets": bundle_assets,
    }
    attribution = {
        "asset_id": "imports/normalized/src010_cc0_release_bulk",
        "source_id": SOURCE_ID,
        "source_repo": "Local more assets to ingest drop",
        "source_url": "local://imports/raw/more_assets_to_ingest",
        "original_relative_path": "imports/raw/more_assets_to_ingest/<127 CC0-public-domain pack roots>",
        "author": "SRC-010 CC0/public-domain pack authors; see license_evidence for per-pack records",
        "license": "CC0-1.0 / Public Domain evidence reviewed from source readme/license files",
        "commercial_use_allowed": True,
        "redistribution_allowed": True,
        "promotion_status": "promoted",
        "bundle_id": "BND-006",
        "promoted_asset_count": len(promoted_assets),
        "promoted_assets": promoted_assets,
        "license_evidence": sorted(
            {path for paths in evidence_by_pack.values() for path in paths}
        ),
        "notes": [
            "Bulk SRC-010 promotion includes only app-usable unique payload files from packs with permissive CC0/public-domain evidence.",
            "Duplicate payloads, archives, executable/tools, source-art files, source map files, and non-OGG audio originals remain raw quarantine/local catalog only.",
            "Bundle assets are release eligible but not default-bundled; a future project or release asset fixture must explicitly connect selected assets to package them.",
        ],
    }
    scan = {
        "schema": "urpg.src010_release_bulk_scan.v1",
        "generated_at": generated_at,
        "source_id": SOURCE_ID,
        "policy": "Promote every unique app-usable SRC-010 payload from CC0/public-domain reviewed packs; hold all other raw assets in quarantine.",
        "cc0_public_domain_pack_count": len(allowed_packs),
        "selected_asset_count": len(selected),
        "selected_total_bytes": copied_bytes,
        "rejected_asset_count": len(rejected),
        "rejection_summary": {},
        "selected_by_category": {},
        "rejected_samples": rejected[:200],
    }
    for asset, _source_file, _digest, _size in selected:
        scan["selected_by_category"][asset["category"]] = (
            scan["selected_by_category"].get(asset["category"], 0) + 1
        )
    for item in rejected:
        scan["rejection_summary"][item["reason"]] = (
            scan["rejection_summary"].get(item["reason"], 0) + 1
        )

    if not args.dry_run:
        write_json(BUNDLE_PATH, bundle)
        write_json(ATTRIBUTION_PATH, attribution)
        write_json(SCAN_REPORT_PATH, scan)
    print(json.dumps(scan, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
