from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "asset_db.py"
SPEC = importlib.util.spec_from_file_location("urpg_asset_db", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
asset_db = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(asset_db)


class AssetDbExternalRootTests(unittest.TestCase):
    def test_external_root_is_indexed_without_hiding_other_roots(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            workspace = Path(temp_dir)
            repo_root = workspace / "repo"
            internal_root = repo_root / "imports" / "raw" / "internal"
            external_root = workspace / "All 2D Assets Stay Here"
            internal_root.mkdir(parents=True)
            (external_root / "characters").mkdir(parents=True)

            (internal_root / "ui.svg").write_text("<svg/>", encoding="utf-8")
            (external_root / "characters" / "hero.png").write_bytes(b"not-a-real-png")
            (external_root / "character-pack.7z").write_bytes(b"archive")
            (external_root / "license.txt").write_text("local review required", encoding="utf-8")

            catalog = asset_db.Catalog(repo_root, repo_root / ".urpg" / "asset-index" / "asset_catalog.db")
            try:
                catalog.init_db()
                catalog.index([internal_root])
                result = catalog.index([external_root])

                self.assertEqual(result["files_seen"], 3)
                rows = catalog.conn.execute(
                    "SELECT path_rel,source_root,media_kind,pack,missing FROM assets ORDER BY path_rel"
                ).fetchall()
                records = {row["path_rel"]: row for row in rows}

                internal = records["imports/raw/internal/ui.svg"]
                self.assertEqual(internal["media_kind"], "image")
                self.assertEqual(internal["missing"], 0)

                external_paths = [path for path in records if path.startswith("external/")]
                self.assertEqual(len(external_paths), 3)
                hero = next(records[path] for path in external_paths if path.endswith("characters/hero.png"))
                archive = next(records[path] for path in external_paths if path.endswith("character-pack.7z"))
                self.assertEqual(hero["source_root"], archive["source_root"])
                self.assertEqual(hero["media_kind"], "image")
                self.assertEqual(hero["pack"], "characters")
                self.assertEqual(archive["media_kind"], "archive")

                (external_root / "characters" / "hero.png").unlink()
                (external_root / "characters" / "villain.png").write_bytes(b"another-not-real-png")
                catalog.index(
                    [external_root],
                    include_kinds={"image", "archive"},
                    hash_files=False,
                )
                missing = catalog.conn.execute(
                    "SELECT missing FROM assets WHERE path_abs=?",
                    (str((external_root / "characters" / "hero.png").resolve()),),
                ).fetchone()
                self.assertIsNotNone(missing)
                self.assertEqual(missing["missing"], 1)

                internal_missing = catalog.conn.execute(
                    "SELECT missing FROM assets WHERE path_abs=?",
                    (str((internal_root / "ui.svg").resolve()),),
                ).fetchone()
                self.assertIsNotNone(internal_missing)
                self.assertEqual(internal_missing["missing"], 0)

                external_text = catalog.conn.execute(
                    "SELECT missing FROM assets WHERE path_abs=?",
                    (str((external_root / "license.txt").resolve()),),
                ).fetchone()
                self.assertIsNotNone(external_text)
                self.assertEqual(external_text["missing"], 0)

                archive_hash = catalog.conn.execute(
                    "SELECT sha256 FROM assets WHERE path_abs=?",
                    (str((external_root / "character-pack.7z").resolve()),),
                ).fetchone()
                self.assertIsNotNone(archive_hash)
                self.assertIsNotNone(archive_hash["sha256"])

                unhashed = catalog.conn.execute(
                    "SELECT sha256 FROM assets WHERE path_abs=?",
                    (str((external_root / "characters" / "villain.png").resolve()),),
                ).fetchone()
                self.assertIsNotNone(unhashed)
                self.assertIsNone(unhashed["sha256"])
            finally:
                catalog.close()


if __name__ == "__main__":
    unittest.main()
