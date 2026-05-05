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

from tools.assets import generate_cutesckr_grid_parts  # noqa: E402


class GenerateCuteSckrGridPartsTests(unittest.TestCase):
    def test_generates_grouped_catalogs_from_extracted_archives(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            extracted = root / "imports" / "raw" / "cutesckr_all" / "__archive_extracted"
            pack = extracted / "forest-pack-abc123"
            pack.mkdir(parents=True)
            Image.new("RGBA", (96, 48), (30, 160, 80, 255)).save(pack / "1.png")
            Image.new("RGBA", (48, 48), (120, 90, 40, 255)).save(pack / "props.png")

            result = generate_cutesckr_grid_parts.generate_catalogs(
                repo_root=root,
                source_root=extracted,
                output_root=root / "content" / "assets" / "gameplay" / "cutesckr_all",
                catalog_root=root / "content" / "part_catalogs" / "generated" / "cutesckr_all",
                aggregate_catalog=root / "content" / "part_catalogs" / "generated" / "cutesckr_all_parts.json",
                tile_size=48,
                max_tiles_per_sheet=2,
            )

            self.assertEqual(result.generated_sheets, 2)
            self.assertEqual(result.generated_parts, 3)
            aggregate = json.loads((root / "content" / "part_catalogs" / "generated" / "cutesckr_all_parts.json").read_text())
            self.assertEqual(aggregate["includes"], ["cutesckr_all/forest_pack_abc123_1_parts.json", "cutesckr_all/forest_pack_abc123_props_parts.json"])

            first_catalog = json.loads((root / "content" / "part_catalogs" / "generated" / aggregate["includes"][0]).read_text())
            self.assertEqual(first_catalog["parts"][0]["category"], "Tile")
            self.assertEqual(first_catalog["parts"][0]["defaultProperties"]["sourceBundleId"], "forest-pack-abc123")
            self.assertEqual(first_catalog["parts"][0]["previewPath"], "content/assets/gameplay/cutesckr_all/forest_pack_abc123/1/tile_000.png")
            self.assertTrue((root / first_catalog["parts"][0]["previewPath"]).exists())

            second_catalog = json.loads((root / "content" / "part_catalogs" / "generated" / aggregate["includes"][1]).read_text())
            self.assertEqual(second_catalog["parts"][0]["category"], "Prop")


if __name__ == "__main__":
    unittest.main()
