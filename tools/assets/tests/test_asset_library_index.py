#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))

from tools.assets import asset_db  # noqa: E402


PNG_1X1 = (
    b"\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01"
    b"\x00\x00\x00\x01\x08\x06\x00\x00\x00\x1f\x15\xc4\x89"
    b"\x00\x00\x00\nIDATx\x9cc\xf8\x0f\x00\x01\x01\x01\x00"
    b"\x18\xdd\x8d\xb0\x00\x00\x00\x00IEND\xaeB`\x82"
)


class AssetLibraryIndexTests(unittest.TestCase):
    def test_exports_preview_records_for_mixed_asset_folder(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            pack_root = root / "imports" / "raw" / "more_assets" / "FixturePack"
            pack_root.mkdir(parents=True)
            (pack_root / "grass_tile.png").write_bytes(PNG_1X1)
            (pack_root / "hero_walk.aseprite").write_bytes(b"ASEPRITE")
            (pack_root / "README.xyz").write_bytes(b"unknown payload")
            theme = pack_root / "flat_theme.json"
            theme.write_text(
                json.dumps(
                    {
                        "schemaVersion": 2,
                        "themeId": "flat_theme",
                        "displayName": "Flat Theme",
                        "surfaces": ["game_ui_theme"],
                    }
                ),
                encoding="utf-8",
            )

            catalog = asset_db.Catalog(root, root / ".urpg" / "asset-index" / "asset_catalog.db")
            try:
                catalog.init_db()
                catalog.index([pack_root])
                index = catalog.export_library_index(limit=20)
            finally:
                catalog.close()

            self.assertEqual(index["schemaVersion"], 1)
            records = {record["sourcePath"]: record for record in index["records"]}

            png = records["imports/raw/more_assets/FixturePack/grass_tile.png"]
            self.assertTrue(png["stableId"].startswith("asset_"))
            self.assertEqual(png["displayName"], "Grass Tile")
            self.assertEqual(png["previewKind"], "image")
            self.assertEqual(png["mediaKind"], "image")
            self.assertEqual(png["category"], "more-assets-raw")
            self.assertEqual(png["pack"], "FixturePack")
            self.assertEqual(png["dimensions"], {"width": 1, "height": 1})
            self.assertEqual(png["unloadablePayload"]["policy"], "lazy")

            aseprite = records["imports/raw/more_assets/FixturePack/hero_walk.aseprite"]
            self.assertEqual(aseprite["previewKind"], "aseprite_metadata")
            self.assertEqual(aseprite["mediaKind"], "image")

            ui_theme = records["imports/raw/more_assets/FixturePack/flat_theme.json"]
            self.assertEqual(ui_theme["previewKind"], "ui_theme_manifest")
            self.assertEqual(ui_theme["mediaKind"], "ui_theme")
            self.assertEqual(ui_theme["category"], "game-ui-theme")

            unknown = records["imports/raw/more_assets/FixturePack/README.xyz"]
            self.assertEqual(unknown["previewKind"], "file_metadata")
            self.assertEqual(unknown["mediaKind"], "binary")


if __name__ == "__main__":
    unittest.main()
