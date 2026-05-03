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


SRC010_SOURCE_ID = "SRC-010"
ITCH_SOURCE_ID = "SRC-013"
SRC010_CATALOG_ROOT = Path("imports/reports/asset_intake/more_assets_to_ingest_promotion_catalog")
SRC010_ARCHIVE_LICENSE_SCAN = Path("imports/reports/asset_intake/src010_archive_license_scan_with_folder_licenses.json")
SRC010_SOURCE_LICENSE_SCAN = Path("imports/reports/asset_intake/src010_source_folder_license_scan.json")
ITCH_LICENSE_SCAN = Path("imports/reports/asset_intake/itch_loose_license_scan.json")
ITCH_ROOT = Path("itch/loose")

SRC010_BUNDLE = Path("imports/manifests/asset_bundles/BND-007.json")
ITCH_BUNDLE = Path("imports/manifests/asset_bundles/BND-008.json")
SRC010_ATTRIBUTION = Path("imports/reports/asset_intake/attribution/BND-007_src010_newly_licensed_bulk.json")
ITCH_ATTRIBUTION = Path("imports/reports/asset_intake/attribution/BND-008_itch_loose_cc0.json")
SCAN_REPORT = Path("imports/reports/asset_intake/current_ingestable_promotion_report.json")
ITCH_SOURCE_MANIFEST = Path("imports/manifests/asset_sources/SRC-013.json")

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
            if code in {0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF}:
                payload = stream.read(size - 2)
                if len(payload) >= 5:
                    return struct.unpack(">H", payload[3:5])[0], struct.unpack(">H", payload[1:3])[0]
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


def technical_ok(path: Path, ext: str) -> tuple[bool, str]:
    if not path.is_file():
        return False, "missing_file"
    if path.stat().st_size <= 0:
        return False, "empty_file"
    if ext in {"png", "gif", "jpg", "jpeg"}:
        return (True, "ok") if image_size(path, ext) else (False, "invalid_image_header")
    if ext == "ogg":
        return (True, "ok") if valid_ogg(path) else (False, "invalid_ogg_header")
    if ext in {"ttf", "otf", "woff", "woff2"}:
        return (True, "ok") if valid_font(path, ext) else (False, "invalid_font_header")
    return False, "unsupported_extension"


def category_to_bundle_category(category: str, ext: str) -> str:
    if category in {"characters", "characters/portraits"}:
        return "prototype_sprite"
    if category == "tilesets":
        return "tileset"
    if category in {"backgrounds", "props"}:
        return "background"
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


def infer_itch_category(path: Path) -> str:
    stem = path.stem.lower()
    if "tile" in stem or "wall" in stem or "floor" in stem or "terrain" in stem:
        return "tilesets"
    if "anim" in stem or "effect" in stem or "fx" in stem:
        return "vfx"
    if "ui" in stem or "icon" in stem or "button" in stem:
        return "ui"
    if any(token in stem for token in ["char", "walk", "base", "bandit", "alien", "astronaut", "slime"]):
        return "characters"
    return "props"


def promoted_source_paths() -> set[str]:
    paths = set()
    for bundle_path in Path("imports/manifests/asset_bundles").glob("BND-*.json"):
        try:
            bundle = read_json(bundle_path)
        except Exception:
            continue
        for asset in bundle.get("assets", []):
            original = asset.get("original_relative_path")
            if original:
                paths.add(original.replace("\\", "/"))
    return paths


def iter_src010_assets() -> list[dict]:
    assets = []
    for shard_path in sorted(SRC010_CATALOG_ROOT.glob("*.json")):
        shard = read_json(shard_path)
        assets.extend(shard.get("assets", []))
    return assets


def src010_cc0_pack_suffixes() -> set[str]:
    suffixes: set[str] = set()
    archive_scan = read_json(SRC010_ARCHIVE_LICENSE_SCAN)
    for archive in archive_scan.get("archives", []):
        if archive.get("classification") == "permissive_cc0_or_public_domain":
            digest = archive.get("archive_sha256", "")
            if len(digest) >= 12:
                suffixes.add(digest[:12].lower())
    source_scan = read_json(SRC010_SOURCE_LICENSE_SCAN)
    for item in source_scan.get("licenses", []):
        if item.get("classification") == "permissive_cc0_or_public_domain":
            parts = Path(item.get("relative_path", "")).parts
            if len(parts) >= 4 and parts[0] == "imports" and parts[2] == "more_assets_to_ingest":
                # raw folder license lives under imports/raw/more_assets_to_ingest/<pack>/...
                suffixes.add(parts[3].split("-")[-1].lower())
    return suffixes


def make_bundle_asset(
    source_path: str,
    promoted_rel: str,
    category: str,
    ext: str,
    digest: str,
    attribution: Path,
    note_prefix: str,
) -> dict:
    return {
        "original_relative_path": source_path,
        "promoted_relative_path": promoted_rel,
        "category": category_to_bundle_category(category, ext),
        "status": "promoted",
        "release_required": False,
        "release_surfaces": category_surfaces(category),
        "license_cleared": True,
        "release_eligible": True,
        "distribution": "deferred",
        "checksum_sha256": digest,
        "attribution_record": attribution.as_posix(),
        "package_destination": f"share/urpg/imports/normalized/{promoted_rel}",
        "notes": f"{note_prefix}; default export bundling is deferred until a project explicitly selects the asset.",
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Promote currently ingestable local assets.")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    already_promoted = promoted_source_paths()
    src010_allowed_suffixes = src010_cc0_pack_suffixes()
    seen_hashes = set()

    src010_selected = []
    src010_rejected = []
    for asset in iter_src010_assets():
        source_path = asset.get("source_path", "").replace("\\", "/")
        pack = asset.get("pack", "")
        ext = asset.get("ext", "").lower()
        category = asset.get("category", "")
        source_file = Path(source_path)
        reason = ""
        if source_path in already_promoted:
            reason = "already_promoted"
        elif not any(pack.lower().endswith(suffix) for suffix in src010_allowed_suffixes):
            reason = "no_cc0_folder_or_archive_license"
        elif asset.get("status") == "duplicate" or asset.get("sha256") in seen_hashes:
            reason = "duplicate_payload"
        elif ext not in APP_USABLE_EXTS:
            reason = "extension_not_app_usable"
        elif category in EXCLUDED_CATEGORIES:
            reason = "category_not_runtime_payload"
        else:
            ok, reason = technical_ok(source_file, ext)
            if ok:
                digest = sha256_file(source_file)
                if digest != asset.get("sha256"):
                    reason = "catalog_checksum_mismatch"
                else:
                    src010_selected.append((asset, source_file, digest))
                    seen_hashes.add(digest)
                    continue
        src010_rejected.append({"source_path": source_path, "pack": pack, "category": category, "ext": ext, "reason": reason})

    src010_bundle_assets = []
    for asset, source_file, digest in sorted(src010_selected, key=lambda item: item[0]["source_path"]):
        category = asset["category"]
        ext = asset["ext"].lower()
        pack_slug = slug(asset["pack"])
        stem_slug = slug(Path(asset["filename"]).stem)
        promoted_rel = f"src010_newly_licensed_bulk/{category.replace('/', '_')}/{pack_slug}/{stem_slug}-{digest[:12]}.{ext}"
        target = Path("imports/normalized") / promoted_rel
        if not args.dry_run:
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
        src010_bundle_assets.append(
            make_bundle_asset(
                asset["source_path"],
                promoted_rel,
                category,
                ext,
                digest,
                SRC010_ATTRIBUTION,
                "SRC-010 newly license-covered payload promoted for release-eligible app/library use",
            )
        )

    itch_scan = read_json(ITCH_LICENSE_SCAN)
    itch_selected = []
    itch_rejected = []
    if itch_scan.get("license_classification") == "permissive_cc0_or_public_domain":
        for source_file in sorted(ITCH_ROOT.glob("*")):
            if not source_file.is_file():
                continue
            source_path = source_file.as_posix()
            ext = source_file.suffix.lower().lstrip(".")
            reason = ""
            if source_path in already_promoted:
                reason = "already_promoted"
            elif ext != "png":
                reason = "not_runtime_png_payload"
            else:
                ok, reason = technical_ok(source_file, ext)
                if ok:
                    digest = sha256_file(source_file)
                    itch_selected.append((source_file, digest, infer_itch_category(source_file)))
                    continue
            itch_rejected.append({"source_path": source_path, "ext": ext, "reason": reason})
    else:
        itch_rejected.append({"source_path": ITCH_ROOT.as_posix(), "ext": "", "reason": "license_not_cc0_public_domain"})

    itch_bundle_assets = []
    for source_file, digest, category in itch_selected:
        ext = source_file.suffix.lower().lstrip(".")
        promoted_rel = f"itch_loose_cc0/{category}/{slug(source_file.stem)}-{digest[:12]}.{ext}"
        target = Path("imports/normalized") / promoted_rel
        if not args.dry_run:
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
        itch_bundle_assets.append(
            make_bundle_asset(
                source_file.as_posix(),
                promoted_rel,
                category,
                ext,
                digest,
                ITCH_ATTRIBUTION,
                "itch/loose CC0 payload promoted for release-eligible app/library use",
            )
        )

    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()
    src010_bundle = {
        "bundle_id": "BND-007",
        "bundle_name": "src010_newly_licensed_bulk",
        "source_id": SRC010_SOURCE_ID,
        "bundle_state": "promoted",
        "release_required": False,
        "release_surfaces": ["title", "map", "battle", "ui", "audio", "fonts"],
        "assets": src010_bundle_assets,
    }
    itch_bundle = {
        "bundle_id": "BND-008",
        "bundle_name": "itch_loose_cc0",
        "source_id": ITCH_SOURCE_ID,
        "bundle_state": "promoted",
        "release_required": False,
        "release_surfaces": ["title", "map", "battle", "ui"],
        "assets": itch_bundle_assets,
    }

    src010_attribution = {
        "asset_id": "imports/normalized/src010_newly_licensed_bulk",
        "source_id": SRC010_SOURCE_ID,
        "source_repo": "Local more assets to ingest drop",
        "source_url": "local://more assets to ingest and imports/raw/more_assets_to_ingest",
        "original_relative_path": "imports/raw/more_assets_to_ingest/<newly CC0-covered pack roots>",
        "author": "SRC-010 local pack authors; see source folder and archive license scan evidence",
        "license": "CC0-1.0 / Public Domain evidence reviewed from source folder and archive license files",
        "commercial_use_allowed": True,
        "redistribution_allowed": True,
        "promotion_status": "promoted",
        "bundle_id": "BND-007",
        "promoted_asset_count": len(src010_bundle_assets),
        "license_evidence": [
            SRC010_ARCHIVE_LICENSE_SCAN.as_posix(),
            SRC010_SOURCE_LICENSE_SCAN.as_posix(),
        ],
        "notes": [
            "Promotion includes only unique app-usable payload files not already promoted in BND-004 or BND-006.",
            "Assets are release eligible but not default-bundled; project selection is required for packaging.",
        ],
    }
    itch_attribution = {
        "asset_id": "imports/normalized/itch_loose_cc0",
        "source_id": ITCH_SOURCE_ID,
        "source_repo": "Local itch loose asset drop",
        "source_url": "local://itch/loose",
        "original_relative_path": "itch/loose",
        "author": "Local itch loose asset authors; see license.txt",
        "license": "CC0-1.0 / Public Domain evidence reviewed from itch/loose/license.txt",
        "commercial_use_allowed": True,
        "redistribution_allowed": True,
        "promotion_status": "promoted",
        "bundle_id": "BND-008",
        "promoted_asset_count": len(itch_bundle_assets),
        "license_evidence": ["itch/loose/license.txt", ITCH_LICENSE_SCAN.as_posix()],
        "notes": [
            "Promotion includes valid PNG payloads from itch/loose.",
            "Assets are release eligible but not default-bundled; project selection is required for packaging.",
        ],
    }
    itch_source_manifest = {
        "source_id": ITCH_SOURCE_ID,
        "repo_name": "Local itch loose asset drop",
        "source_url": "local://itch/loose",
        "capture_state": "mirrored",
        "snapshot_commit": None,
        "snapshot_date": "2026-05-03",
        "source_type": "direct_asset_pack",
        "category_tags": ["sprites", "tilesets", "ui", "vfx", "props"],
        "intended_use": [
            "Editor asset browser discovery from local itch loose payloads",
            "Template polish candidates after project selection",
            "Release-eligible app/library selection with deferred default export bundling",
        ],
        "handling_path": "direct_ingest_when_captured",
        "legal_disposition": "cc0_public_domain_promoted",
        "promotion_status": "promoted",
        "notes": [
            "Captured from C:\\dev\\URPG Maker\\itch\\loose.",
            "BND-008 promotes valid PNG payloads with checksums and attribution evidence.",
            "Default export bundling is deferred until a project explicitly selects assets.",
        ],
    }
    report = {
        "schema": "urpg.current_ingestable_promotion.v1",
        "generated_at": generated_at,
        "src010": {
            "selected_asset_count": len(src010_bundle_assets),
            "rejected_asset_count": len(src010_rejected),
            "rejection_summary": {},
            "bundle": SRC010_BUNDLE.as_posix(),
        },
        "itch_loose": {
            "selected_asset_count": len(itch_bundle_assets),
            "rejected_asset_count": len(itch_rejected),
            "rejection_summary": {},
            "bundle": ITCH_BUNDLE.as_posix(),
        },
    }
    for item in src010_rejected:
        report["src010"]["rejection_summary"][item["reason"]] = report["src010"]["rejection_summary"].get(item["reason"], 0) + 1
    for item in itch_rejected:
        report["itch_loose"]["rejection_summary"][item["reason"]] = report["itch_loose"]["rejection_summary"].get(item["reason"], 0) + 1

    if not args.dry_run:
        write_json(SRC010_BUNDLE, src010_bundle)
        write_json(ITCH_BUNDLE, itch_bundle)
        write_json(SRC010_ATTRIBUTION, src010_attribution)
        write_json(ITCH_ATTRIBUTION, itch_attribution)
        write_json(ITCH_SOURCE_MANIFEST, itch_source_manifest)
        write_json(SCAN_REPORT, report)
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
