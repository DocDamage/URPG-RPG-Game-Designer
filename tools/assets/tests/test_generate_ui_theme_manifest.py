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

from tools.assets import generate_ui_theme_manifest  # noqa: E402


class GenerateUiThemeManifestTests(unittest.TestCase):
    def test_generates_manifest_and_completeness_report_from_ui_folder(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "source"
            sprites = source / "Sprites"
            sprites.mkdir(parents=True)
            for name in [
                "UI_Flat_Button01a_1.png",
                "UI_Flat_Button01a_2.png",
                "UI_Flat_Button01a_3.png",
                "UI_Flat_Button01a_4.png",
                "UI_Flat_InputField01a.png",
                "UI_Flat_Select01a_1.png",
                "UI_Flat_ToggleOn01a.png",
                "UI_Flat_ToggleOff01a.png",
                "UI_Flat_Frame01a.png",
                "UI_Flat_Bar01a.png",
                "UI_Flat_BarFill01a.png",
                "UI_Flat_IconCheck01a.png",
            ]:
                Image.new("RGBA", (32, 32), (80, 120, 160, 255)).save(sprites / name)
            (source / "license.txt").write_text("CC0 1.0", encoding="utf-8")

            result = generate_ui_theme_manifest.generate_theme(
                repo_root=root,
                source_root=source,
                theme_id="complete_ui_flat",
                display_name="Complete UI Flat",
                raw_output=root / "imports" / "raw" / "ui_themes" / "complete_ui_flat",
                asset_output=root / "content" / "assets" / "ui_themes" / "complete_ui_flat",
                manifest_output=root / "content" / "ui_themes" / "complete_ui_flat.json",
                report_output=root / "imports" / "reports" / "ui_theme_validation" / "complete_ui_flat.json",
                initial_surfaces=["game_ui_theme"],
            )

            self.assertEqual(result.asset_count, 12)
            manifest = json.loads((root / "content" / "ui_themes" / "complete_ui_flat.json").read_text())
            self.assertEqual(manifest["themeId"], "complete_ui_flat")
            self.assertIn("game_ui_theme", manifest["surfaces"])
            self.assertTrue(manifest["components"]["buttons"]["states"])
            self.assertEqual(manifest["themeCompleteness"]["gameUiReady"]["status"], "pass")
            self.assertEqual(manifest["themeCompleteness"]["editorThemeReady"]["status"], "fail")

            report = json.loads((root / "imports" / "reports" / "ui_theme_validation" / "complete_ui_flat.json").read_text())
            self.assertIn("tab_states", report["checks"]["editorThemeReady"]["missing"])


if __name__ == "__main__":
    unittest.main()
