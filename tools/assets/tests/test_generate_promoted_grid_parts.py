#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))

from tools.assets import generate_promoted_grid_parts  # noqa: E402


class GeneratePromotedGridPartsTests(unittest.TestCase):
    def test_promoted_image_assets_become_grouped_grid_part_catalog(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            normalized = root / "imports" / "normalized" / "legacy"
            normalized.mkdir(parents=True)
            Image.new("RGBA", (32, 16), (100, 80, 60, 255)).save(normalized / "stone.png")
            Image.new("RGBA", (24, 48), (20, 180, 90, 255)).save(normalized / "hero.png")

            bundle = root / "imports" / "manifests" / "asset_bundles" / "BND-900.json"
            bundle.parent.mkdir(parents=True)
            bundle.write_text(
                json.dumps(
                    {
                        "bundle_id": "BND-900",
                        "bundle_name": "legacy_pack",
                        "source_id": "SRC-900",
                        "bundle_state": "promoted",
                        "assets": [
                            {
                                "promoted_relative_path": "legacy/stone.png",
                                "category": "tileset",
                                "license_cleared": True,
                                "release_eligible": True,
                                "checksum_sha256": "a" * 64,
                            },
                            {
                                "promoted_relative_path": "legacy/hero.png",
                                "category": "prototype_sprite",
                                "license_cleared": True,
                                "release_eligible": True,
                                "checksum_sha256": "b" * 64,
                            },
                            {
                                "promoted_relative_path": "legacy/theme.ogg",
                                "category": "audio",
                                "license_cleared": True,
                                "release_eligible": True,
                            },
                        ],
                    }
                ),
                encoding="utf-8",
            )

            result = generate_promoted_grid_parts.generate_catalogs(
                repo_root=root,
                bundle_paths=[bundle],
                output_root=root / "content" / "assets" / "gameplay" / "legacy",
                catalog_root=root / "content" / "part_catalogs" / "generated" / "legacy",
                max_thumbnail_size=16,
            )

            self.assertEqual(result.generated_parts, 2)
            self.assertEqual(result.generated_catalogs, 1)

            catalog = json.loads((root / result.catalog_paths[0]).read_text(encoding="utf-8"))
            self.assertEqual(catalog["catalogId"], "legacy.bnd_900")
            self.assertEqual(len(catalog["parts"]), 2)
            self.assertEqual(catalog["parts"][0]["category"], "Tile")
            self.assertEqual(catalog["parts"][0]["defaultLayer"], "Terrain")
            self.assertEqual(catalog["parts"][0]["sourceImagePath"], "imports/normalized/legacy/stone.png")
            self.assertEqual(catalog["parts"][0]["previewPath"], "content/assets/gameplay/legacy/bnd_900/tile/00000.png")
            self.assertEqual(catalog["parts"][0]["atlasRect"], {"x": 0, "y": 0, "width": 32, "height": 16})
            self.assertIn("bnd-900", catalog["parts"][0]["tags"])
            self.assertEqual(catalog["parts"][1]["category"], "Npc")
            self.assertTrue((root / catalog["parts"][0]["previewPath"]).exists())
            with Image.open(root / catalog["parts"][0]["previewPath"]) as thumbnail:
                self.assertLessEqual(max(thumbnail.size), 16)


if __name__ == "__main__":
    unittest.main()
