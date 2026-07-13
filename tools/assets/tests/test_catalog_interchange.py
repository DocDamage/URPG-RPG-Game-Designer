from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


ASSET_DB_PATH = Path(__file__).resolve().parents[1] / "asset_db.py"
ASSET_DB_SPEC = importlib.util.spec_from_file_location("urpg_asset_db", ASSET_DB_PATH)
assert ASSET_DB_SPEC is not None and ASSET_DB_SPEC.loader is not None
asset_db = importlib.util.module_from_spec(ASSET_DB_SPEC)
ASSET_DB_SPEC.loader.exec_module(asset_db)

MODULE_PATH = Path(__file__).resolve().parents[1] / "catalog_interchange.py"
SPEC = importlib.util.spec_from_file_location("urpg_catalog_interchange", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
catalog_interchange = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(catalog_interchange)


class CatalogInterchangeTests(unittest.TestCase):
    def test_export_is_metadata_only_sharded_and_replaces_manifest_last(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            source = root / "external-assets"
            source.mkdir()
            (source / "Hero Sprite.PNG").write_bytes(b"hero-payload-must-not-appear")
            (source / "characters.7z").write_bytes(b"archive-payload-must-not-appear")
            database = root / ".urpg" / "asset-index" / "asset_catalog.db"
            catalog = asset_db.Catalog(root, database)
            try:
                catalog.init_db()
                catalog.index([source], hash_files=False)
            finally:
                catalog.close()

            output = root / ".urpg" / "asset-index"
            manifest = catalog_interchange.export_catalog(database, output, shard_size=1)

            self.assertEqual(manifest["schema_version"], "urpg.asset_catalog.v1")
            self.assertTrue(manifest["scan_complete"])
            self.assertEqual(manifest["counts"]["asset_count"], 2)
            self.assertEqual(manifest["counts"]["hash_pending_count"], 2)
            self.assertEqual(len(manifest["shards"]), 2)
            self.assertTrue((output / "catalog_meta.json").is_file())
            self.assertGreaterEqual(len(manifest["roots"]), 1)

            serialized = "\n".join(
                (output / shard["path"]).read_text(encoding="utf-8") for shard in manifest["shards"]
            )
            self.assertNotIn("hero-payload-must-not-appear", serialized)
            records = [json.loads(line) for line in serialized.splitlines() if line]
            hero = next(record for record in records if record["filename"] == "Hero Sprite.PNG")
            archive = next(record for record in records if record["filename"] == "characters.7z")
            self.assertEqual(hero["normalized_filename"], "hero sprite.png")
            self.assertEqual(archive["archive_kind"], "7z")
            self.assertNotIn("path_abs", hero)

            previous_shards = {shard["path"] for shard in manifest["shards"]}
            second = catalog_interchange.export_catalog(database, output, shard_size=10)
            self.assertEqual(len(second["shards"]), 1)
            self.assertEqual(json.loads((output / "catalog_meta.json").read_text(encoding="utf-8")), second)
            self.assertFalse(any((output / old).exists() for old in previous_shards))

    def test_rejects_uninitialized_database_without_replacing_previous_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            database = root / "empty.db"
            database.touch()
            output = root / "catalog"
            output.mkdir()
            old_manifest = {"schema_version": "old", "keep": True}
            (output / "catalog_meta.json").write_text(json.dumps(old_manifest), encoding="utf-8")

            with self.assertRaises(RuntimeError):
                catalog_interchange.export_catalog(database, output)

            self.assertEqual(json.loads((output / "catalog_meta.json").read_text(encoding="utf-8")), old_manifest)


if __name__ == "__main__":
    unittest.main()
