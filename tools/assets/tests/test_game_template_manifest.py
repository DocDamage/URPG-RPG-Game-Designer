#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))


EXPECTED_MANIFESTS = {
    "jrpg_starter.json": "jrpg",
    "action_rpg_starter.json": "arpg",
    "tactical_rpg_starter.json": "tactics_rpg",
    "visual_novel_hybrid_starter.json": "visual_novel",
    "cozy_life_starter.json": "cozy_life_rpg",
    "monster_collector_starter.json": "monster_collector_rpg",
    "platform_adventure_starter.json": "platformer_rpg",
}


class GameTemplateManifestTests(unittest.TestCase):
    def test_schema_and_starter_manifests_exist(self) -> None:
        self.assertTrue((REPO_ROOT / "content" / "schemas" / "game_template_manifest.schema.json").is_file())
        manifest_root = REPO_ROOT / "content" / "templates" / "game_maker"
        for filename in EXPECTED_MANIFESTS:
            self.assertTrue((manifest_root / filename).is_file(), filename)

    def test_starter_manifests_are_bounded_and_onboarding_ready(self) -> None:
        manifest_root = REPO_ROOT / "content" / "templates" / "game_maker"
        for filename, template_id in EXPECTED_MANIFESTS.items():
            with self.subTest(filename=filename):
                manifest = json.loads((manifest_root / filename).read_text(encoding="utf-8"))
                self.assertEqual(manifest["schemaVersion"], 1)
                self.assertEqual(manifest["templateId"], template_id)
                self.assertTrue(manifest["displayName"])
                self.assertTrue(manifest["gameType"])
                self.assertTrue(manifest["questionProfile"])
                self.assertEqual(manifest["defaultWorldSize"]["preset"], "small")
                self.assertGreaterEqual(len(manifest["recommendedMechanics"]), 2)
                self.assertGreaterEqual(len(manifest["defaultCatalogs"]), 1)
                self.assertNotIn("content/part_catalogs/game_maker_all_parts.json", manifest["defaultCatalogs"])
                self.assertIn("content/part_catalogs/game_maker_all_parts.json", manifest["optionalCatalogs"])
                self.assertEqual(manifest["fullLibraryPolicy"], "opt_in_lazy_load")
                self.assertEqual(manifest["browserLayout"], "left_collapsible_folder_tree")
                self.assertEqual(manifest["indexBackend"], "sqlite")
                self.assertTrue(manifest["assetIndexPath"])
                self.assertTrue((REPO_ROOT / manifest["assetIndexPath"]).is_file(), manifest["assetIndexPath"])
                self.assertEqual(manifest["uiThemes"]["defaultGameUiTheme"], "complete_ui_essential_flat")
                self.assertIn("complete_ui_essential_flat", manifest["uiThemes"]["availableGameUiThemes"])
                self.assertTrue(manifest["futureCommunityTemplateSlot"])
                for catalog in manifest["defaultCatalogs"]:
                    self.assertTrue((REPO_ROOT / catalog).is_file(), catalog)
                for catalog in manifest["optionalCatalogs"]:
                    self.assertTrue((REPO_ROOT / catalog).is_file(), catalog)


if __name__ == "__main__":
    unittest.main()
