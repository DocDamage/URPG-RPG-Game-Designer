#!/usr/bin/env python3
"""Tests for the game-development resource catalog validator."""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.knowledge.validate_resource_catalog import (  # noqa: E402
    main,
    validate_catalog,
)

SEED_CATALOG_PATH = (
    REPO_ROOT
    / "content"
    / "knowledge"
    / "game_dev_resources"
    / "resource_catalog.seed.json"
)


def _seed_catalog() -> dict:
    return json.loads(SEED_CATALOG_PATH.read_text(encoding="utf-8"))


class ResourceCatalogValidationTests(unittest.TestCase):
    def test_seed_catalog_is_valid(self) -> None:
        self.assertEqual(validate_catalog(_seed_catalog()), [])

    def test_duplicate_ids_are_rejected(self) -> None:
        catalog = _seed_catalog()
        catalog["entries"].append(copy.deepcopy(catalog["entries"][0]))

        errors = validate_catalog(catalog)

        self.assertTrue(any("duplicates" in error for error in errors), errors)

    def test_usable_assets_require_verified_license(self) -> None:
        catalog = _seed_catalog()
        catalog["entries"][0]["approved_use"] = "usable_asset"
        catalog["entries"][0]["license_status"] = "needs_review"

        errors = validate_catalog(catalog)

        self.assertTrue(
            any("usable_asset" in error and "verified" in error for error in errors),
            errors,
        )

    def test_rejected_license_requires_rejected_approval(self) -> None:
        catalog = _seed_catalog()
        catalog["entries"][0]["license_status"] = "rejected"
        catalog["entries"][0]["approved_use"] = "knowledge_base"

        errors = validate_catalog(catalog)

        self.assertTrue(any("rejected licenses" in error for error in errors), errors)

    def test_unknown_fields_are_rejected(self) -> None:
        catalog = _seed_catalog()
        catalog["entries"][0]["surprise"] = "not allowed"

        errors = validate_catalog(catalog)

        self.assertTrue(any("unknown fields" in error for error in errors), errors)

    def test_cli_validates_seed_catalog(self) -> None:
        self.assertEqual(main([str(SEED_CATALOG_PATH)]), 0)


if __name__ == "__main__":
    unittest.main()
