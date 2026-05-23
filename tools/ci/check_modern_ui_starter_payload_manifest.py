#!/usr/bin/env python3
# fmt: off
from __future__ import annotations

import json
import sys
from pathlib import Path

MANIFEST_PATH = Path(
    "imports/manifests/asset_bundles/BND-009-modern-ui-starter-payload.json"
)
INDEX_ROOT = Path("content/asset_indexes/game_maker")
EXPECTED_INDEXES = {
    "action_rpg_starter.json",
    "cozy_life_starter.json",
    "jrpg_starter.json",
    "monster_collector_starter.json",
    "platform_adventure_starter.json",
    "tactical_rpg_starter.json",
    "visual_novel_hybrid_starter.json",
}


def fail(message: str) -> None:
    print(f"modern UI starter payload manifest check failed: {message}", file=sys.stderr)
    raise SystemExit(1)


def load_json(path: Path) -> dict:
    try:
        with path.open("r", encoding="utf-8") as handle:
            value = json.load(handle)
    except FileNotFoundError:
        fail(f"missing required file: {path}")
    except json.JSONDecodeError as exc:
        fail(f"invalid JSON in {path}: {exc}")
    if not isinstance(value, dict):
        fail(f"{path} must contain a JSON object")
    return value


def collect_index_assets() -> dict[str, dict]:
    if not INDEX_ROOT.is_dir():
        fail(f"missing starter index root: {INDEX_ROOT}")

    discovered = {path.name for path in INDEX_ROOT.glob("*.json")}
    missing = sorted(EXPECTED_INDEXES - discovered)
    if missing:
        fail(f"missing starter indexes: {missing}")

    assets: dict[str, dict] = {}
    for index_name in sorted(EXPECTED_INDEXES):
        index_path = INDEX_ROOT / index_name
        payload = load_json(index_path)
        records = payload.get("records")
        if not isinstance(records, list):
            fail(f"{index_path} must contain a records list")
        declared_count = payload.get("recordCount")
        if declared_count != len(records):
            fail(
                f"{index_path} recordCount={declared_count} "
                f"does not match {len(records)} records"
            )
        for record in records:
            if not isinstance(record, dict):
                fail(f"{index_path} contains a non-object record")
            source_path = record.get("sourcePath")
            if not isinstance(source_path, str) or not source_path:
                fail(f"{index_path} has a record without sourcePath")
            payload_info = record.get("unloadablePayload")
            if not isinstance(payload_info, dict):
                fail(f"{index_path} record {source_path} is missing unloadablePayload")
            size_bytes = payload_info.get("sizeBytes")
            if not isinstance(size_bytes, int) or size_bytes <= 0:
                fail(
                    f"{index_path} record {source_path} "
                    "must declare positive sizeBytes"
                )
            if source_path in assets:
                fail(f"duplicate sourcePath across starter indexes: {source_path}")
            assets[source_path] = {
                "referencedBy": f"content/asset_indexes/game_maker/{index_name}",
                "expectedSizeBytes": size_bytes,
            }
    return assets


def main() -> int:
    manifest = load_json(MANIFEST_PATH)
    if manifest.get("bundleId") != "BND-009":
        fail("bundleId must be BND-009")
    if manifest.get("status") != "candidate_manifest_only":
        fail(
            "status must remain candidate_manifest_only until binary payloads are promoted"
        )
    if manifest.get("releaseRequired") is not False:
        fail("releaseRequired must be false for this manifest-only candidate")
    if manifest.get("releaseEligible") is not False:
        fail("releaseEligible must be false until source/license evidence is added")
    if manifest.get("referencePullRequest") != 22:
        fail("referencePullRequest must point at draft quarantine PR #22")

    candidate_assets = manifest.get("candidateAssets")
    if not isinstance(candidate_assets, list) or not candidate_assets:
        fail("candidateAssets must be a non-empty list")

    index_assets = collect_index_assets()
    manifest_assets: dict[str, dict] = {}
    for asset in candidate_assets:
        if not isinstance(asset, dict):
            fail("candidateAssets contains a non-object row")
        path = asset.get("path")
        if not isinstance(path, str) or not path.endswith(".png"):
            fail(f"candidate path must be a PNG path: {path}")
        if not path.startswith("content/assets/gameplay/"):
            fail(f"candidate path must stay under content/assets/gameplay/: {path}")
        if path in manifest_assets:
            fail(f"duplicate candidate path: {path}")
        expected_size = asset.get("expectedSizeBytes")
        if not isinstance(expected_size, int) or expected_size <= 0:
            fail(f"candidate path must declare positive expectedSizeBytes: {path}")
        referenced_by = asset.get("referencedBy")
        if not isinstance(referenced_by, list) or len(referenced_by) != 1:
            fail(
                f"candidate path must have exactly one starter index reference: {path}"
            )
        manifest_assets[path] = asset

    missing_from_manifest = sorted(set(index_assets) - set(manifest_assets))
    extra_in_manifest = sorted(set(manifest_assets) - set(index_assets))
    if missing_from_manifest:
        fail(f"starter index source paths missing from manifest: {missing_from_manifest}")
    if extra_in_manifest:
        fail(f"manifest paths not referenced by starter indexes: {extra_in_manifest}")

    for path, index_row in index_assets.items():
        manifest_row = manifest_assets[path]
        if manifest_row["expectedSizeBytes"] != index_row["expectedSizeBytes"]:
            fail(
                f"size mismatch for {path}: manifest={manifest_row['expectedSizeBytes']} "
                f"index={index_row['expectedSizeBytes']}"
            )
        if manifest_row["referencedBy"] != [index_row["referencedBy"]]:
            fail(
                f"reference mismatch for {path}: manifest={manifest_row['referencedBy']} "
                f"index={[index_row['referencedBy']]}"
            )

    print(f"modern UI starter payload manifest check passed: {len(manifest_assets)} candidates")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
# fmt: on
