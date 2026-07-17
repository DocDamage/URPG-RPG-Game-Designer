from __future__ import annotations

import hashlib
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "generate_product_benchmark_projects.py"
SPEC = importlib.util.spec_from_file_location("urpg_product_benchmark_generator", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
generator = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(generator)


def plan_fixture() -> dict:
    fixtures = [
        {"id": "tiny_test", "scale": "tiny", "maps": 2, "events": 5, "assets": 3,
         "database_records": 4, "content_bytes": 4096},
        {"id": "medium_test", "scale": "medium", "maps": 3, "events": 7, "assets": 5,
         "database_records": 6, "content_bytes": 8192},
        {"id": "large_test", "scale": "large", "maps": 4, "events": 9, "assets": 7,
         "database_records": 8, "content_bytes": 16384},
    ]
    hardware = [
        {"id": "minimum", "class": "minimum_desktop", "logical_cores": 4,
         "memory_bytes": 8 * 1024**3, "graphics_class": "integrated"},
        {"id": "recommended", "class": "recommended_desktop", "logical_cores": 8,
         "memory_bytes": 16 * 1024**3, "graphics_class": "entry_discrete"},
        {"id": "high_end", "class": "high_end_desktop", "logical_cores": 16,
         "memory_bytes": 32 * 1024**3, "graphics_class": "modern_discrete"},
    ]
    thresholds = {
        row["id"]: {
            metric: {scale: 1000 for scale in generator.FIXTURE_SCALES}
            for metric in generator.METRICS
        }
        for row in hardware
    }
    return {"schema": generator.PLAN_SCHEMA, "baseline_version": "pcq700.v1",
            "fixtures": fixtures, "hardware": hardware, "thresholds": thresholds}


def tree_digest(root: Path) -> str:
    digest = hashlib.sha256()
    for path in sorted(item for item in root.rglob("*") if item.is_file()):
        digest.update(path.relative_to(root).as_posix().encode("utf-8"))
        digest.update(path.read_bytes())
    return digest.hexdigest()


class ProductBenchmarkFixtureGeneratorTests(unittest.TestCase):
    def test_materializes_exact_deterministic_project_scales(self) -> None:
        with tempfile.TemporaryDirectory() as temp_directory:
            root = Path(temp_directory)
            plan_path = root / "plan.json"
            plan = plan_fixture()
            plan_path.write_text(json.dumps(plan), encoding="utf-8")
            output = root / "generated"
            generated = generator.generate(plan_path, output)
            self.assertEqual([path.name for path in generated], ["tiny_test", "medium_test", "large_test"])

            for fixture in plan["fixtures"]:
                project = output / fixture["id"]
                manifest = json.loads((project / "project.json").read_text(encoding="utf-8"))
                inventory = json.loads((project / "benchmark_inventory.json").read_text(encoding="utf-8"))
                maps = [json.loads(path.read_text(encoding="utf-8"))
                        for path in sorted((project / "content" / "maps").glob("*.json"))]
                assets = json.loads((project / "content" / "assets" / "benchmark_catalog.json").read_text(encoding="utf-8"))
                database = json.loads((project / "content" / "database.json").read_text(encoding="utf-8"))
                self.assertEqual(manifest["schema_version"], "urpg.project.v1")
                self.assertEqual(len(maps), fixture["maps"])
                self.assertEqual(sum(len(row["events"]) for row in maps), fixture["events"])
                self.assertEqual(len(assets["assets"]), fixture["assets"])
                self.assertEqual(len(database["items"]), fixture["database_records"])
                self.assertEqual((project / "content" / "benchmark_payload.bin").stat().st_size,
                                 fixture["content_bytes"])
                self.assertEqual(inventory["events"], fixture["events"])

            first_digest = tree_digest(output)
            generator.generate(plan_path, output, force=True)
            self.assertEqual(tree_digest(output), first_digest)

    def test_rejects_unsafe_ids_and_incomplete_thresholds(self) -> None:
        with tempfile.TemporaryDirectory() as temp_directory:
            root = Path(temp_directory)
            plan = plan_fixture()
            plan["fixtures"][0]["id"] = "../escape"
            plan_path = root / "unsafe.json"
            plan_path.write_text(json.dumps(plan), encoding="utf-8")
            with self.assertRaisesRegex(generator.PlanError, "fixture id"):
                generator.load_plan(plan_path)

            plan = plan_fixture()
            del plan["thresholds"]["minimum"]["save"]["large"]
            plan_path.write_text(json.dumps(plan), encoding="utf-8")
            with self.assertRaisesRegex(generator.PlanError, "all fixture scales"):
                generator.load_plan(plan_path)


if __name__ == "__main__":
    unittest.main()
