#!/usr/bin/env python3
"""Validate URPG game-development resource catalog files.

This validator intentionally uses only the Python standard library so the
catalog gate can run anywhere CI already runs Python.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any


REQUIRED_ENTRY_FIELDS = {
    "id",
    "name",
    "source_url",
    "source_repo",
    "category",
    "cost",
    "license_status",
    "commercial_use",
    "modification_allowed",
    "redistribution_allowed",
    "attribution_required",
    "urpg_relevance",
    "approved_use",
    "notes",
}

ALLOWED_TOP_LEVEL_FIELDS = {"version", "entries"}
ALLOWED_ENTRY_FIELDS = REQUIRED_ENTRY_FIELDS
ENTRY_ID_PATTERN = re.compile(r"^[a-z0-9][a-z0-9_\-]*$")

ENUMS = {
    "category": {
        "asset",
        "tool",
        "tutorial",
        "engine",
        "audio",
        "spritesheet",
        "tile_editor",
        "resource_index",
        "reference",
    },
    "cost": {"free", "paid", "limited_free", "mixed", "unknown"},
    "license_status": {"verified", "needs_review", "rejected", "unknown"},
    "commercial_use": {"yes", "no", "unknown"},
    "modification_allowed": {"yes", "no", "unknown"},
    "redistribution_allowed": {"yes", "no", "unknown"},
    "attribution_required": {"yes", "no", "unknown"},
    "urpg_relevance": {"high", "medium", "low", "rejected"},
    "approved_use": {
        "knowledge_base",
        "editor_reference",
        "asset_source_candidate",
        "usable_asset",
        "rejected",
    },
}


def _load_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ValueError(
            f"{path}: invalid JSON at line {exc.lineno}: {exc.msg}"
        ) from exc


def validate_catalog(catalog: Any) -> list[str]:
    errors: list[str] = []

    if not isinstance(catalog, dict):
        return ["catalog must be a JSON object"]

    unknown_top_level = sorted(set(catalog) - ALLOWED_TOP_LEVEL_FIELDS)
    if unknown_top_level:
        errors.append(f"unknown top-level fields: {', '.join(unknown_top_level)}")

    version = catalog.get("version")
    if not isinstance(version, int) or version < 1:
        errors.append("version must be an integer >= 1")

    entries = catalog.get("entries")
    if not isinstance(entries, list):
        errors.append("entries must be a list")
        return errors

    seen_ids: set[str] = set()
    for index, entry in enumerate(entries):
        prefix = f"entries[{index}]"
        if not isinstance(entry, dict):
            errors.append(f"{prefix} must be an object")
            continue

        missing = sorted(REQUIRED_ENTRY_FIELDS - set(entry))
        if missing:
            errors.append(f"{prefix} missing required fields: {', '.join(missing)}")

        unknown = sorted(set(entry) - ALLOWED_ENTRY_FIELDS)
        if unknown:
            errors.append(f"{prefix} has unknown fields: {', '.join(unknown)}")

        entry_id = entry.get("id")
        if not isinstance(entry_id, str) or not entry_id:
            errors.append(f"{prefix}.id must be a non-empty string")
        elif not ENTRY_ID_PATTERN.match(entry_id):
            errors.append(f"{prefix}.id must be lowercase snake/kebab slug text")
        elif entry_id in seen_ids:
            errors.append(f"{prefix}.id duplicates '{entry_id}'")
        else:
            seen_ids.add(entry_id)

        for field_name in ("name", "source_url", "source_repo", "notes"):
            value = entry.get(field_name)
            if not isinstance(value, str) or not value.strip():
                errors.append(f"{prefix}.{field_name} must be a non-empty string")

        for field_name, allowed_values in ENUMS.items():
            value = entry.get(field_name)
            if value not in allowed_values:
                allowed = ", ".join(sorted(allowed_values))
                errors.append(f"{prefix}.{field_name} must be one of: {allowed}")

        approved_use = entry.get("approved_use")
        license_status = entry.get("license_status")
        if approved_use == "usable_asset" and license_status != "verified":
            errors.append(
                f"{prefix} cannot be approved as usable_asset without "
                "license_status=verified"
            )

        if license_status == "rejected" and approved_use != "rejected":
            errors.append(
                f"{prefix} rejected licenses must also use approved_use=rejected"
            )

        if entry.get("urpg_relevance") == "rejected" and approved_use != "rejected":
            errors.append(
                f"{prefix} rejected relevance must also use approved_use=rejected"
            )

    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "catalog",
        nargs="?",
        default="content/knowledge/game_dev_resources/resource_catalog.seed.json",
        help="Path to a resource catalog JSON file.",
    )
    args = parser.parse_args(argv)

    catalog_path = Path(args.catalog)
    try:
        catalog = _load_json(catalog_path)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    errors = validate_catalog(catalog)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(f"Validated {catalog_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
