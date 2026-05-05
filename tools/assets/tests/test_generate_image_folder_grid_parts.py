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

from tools.assets import generate_image_folder_grid_parts  # noqa: E402


class GenerateImageFolderGridPartsTests(unittest.TestCase):
    def test_generates_catalog_from_individual_images(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "imports" / "raw" / "portraits"
            source.mkdir(parents=True)
            Image.new("RGBA", (128, 128), (80, 40, 120, 255)).save(source / "mage.png")

            result = generate_image_folder_grid_parts.generate_catalog(
                repo_root=root,
                source_root=source,
                output_root=root / "content" / "assets" / "gameplay" / "portraits",
                catalog_output=root / "content" / "part_catalogs" / "generated" / "portraits_parts.json",
                catalog_id="portraits",
                display_name="Portraits",
                part_prefix="portrait",
                category="Npc",
                source_bundle_id="human_portraits",
                max_thumbnail_size=64,
            )

            self.assertEqual(result.generated_parts, 1)
            catalog = json.loads((root / "content" / "part_catalogs" / "generated" / "portraits_parts.json").read_text())
            self.assertEqual(catalog["parts"][0]["partId"], "portrait.00000")
            self.assertEqual(catalog["parts"][0]["category"], "Npc")
            self.assertEqual(catalog["parts"][0]["defaultProperties"]["sourceBundleId"], "human_portraits")
            self.assertTrue((root / catalog["parts"][0]["previewPath"]).exists())


if __name__ == "__main__":
    unittest.main()
