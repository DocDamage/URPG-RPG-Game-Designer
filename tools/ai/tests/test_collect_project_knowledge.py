#!/usr/bin/env python3
from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.ai import collect_project_knowledge


class ProjectKnowledgeCollectorTests(unittest.TestCase):
    def test_collects_text_documents_and_reports_skips(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "docs").mkdir()
            (root / "engine").mkdir()
            (root / "build").mkdir()
            (root / "docs" / "guide.md").write_text("URPG guide content", encoding="utf-8")
            (root / "engine" / "system.cpp").write_text("void sample() {}", encoding="utf-8")
            (root / "build" / "generated.cpp").write_text("ignored", encoding="utf-8")
            (root / "image.png").write_bytes(b"not-text-image")

            result = collect_project_knowledge.collect_documents(
                root=root,
                include=["docs/**", "engine/**", "image.png", "build/**"],
                exclude=[],
                max_files=20,
                max_bytes=1024 * 1024,
                max_file_bytes=1024,
                max_age_days=30,
            )

            paths = {record["path"] for record in result["filesystem_documents"]}
            self.assertEqual(paths, {"docs/guide.md", "engine/system.cpp"})
            self.assertEqual(result["summary"]["records"], 2)
            self.assertGreaterEqual(result["summary"]["skipped"].get("excluded", 0), 1)
            self.assertGreaterEqual(result["summary"]["skipped"].get("unsupported_extension", 0), 1)
            by_path = {record["path"]: record for record in result["filesystem_documents"]}
            self.assertEqual(by_path["docs/guide.md"]["kind"], "doc")
            self.assertEqual(by_path["engine/system.cpp"]["kind"], "source")
            self.assertIn("indexed_at_epoch", by_path["docs/guide.md"])
            self.assertIn("modified_at_epoch", by_path["docs/guide.md"])

    def test_enforces_max_file_limit(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "a.md").write_text("a", encoding="utf-8")
            (root / "b.md").write_text("b", encoding="utf-8")

            result = collect_project_knowledge.collect_documents(
                root=root,
                include=["**/*"],
                exclude=[],
                max_files=1,
                max_bytes=20,
                max_file_bytes=100,
                max_age_days=30,
            )

            self.assertEqual(result["summary"]["records"], 1)
            self.assertGreaterEqual(result["summary"]["skipped"].get("max_files_exceeded", 0), 1)

    def test_project_data_contract_shape(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / "project"
            root.mkdir()
            (root / "README.md").write_text("Project README", encoding="utf-8")

            result = collect_project_knowledge.collect_documents(
                root=root,
                include=["README.md"],
                exclude=[],
                max_files=5,
                max_bytes=1024,
                max_file_bytes=1024,
                max_age_days=30,
            )

            loaded = json.loads(json.dumps(result))
            self.assertEqual(loaded["schemaVersion"], "1.0.0")
            self.assertEqual(len(loaded["filesystem_documents"]), 1)
            self.assertEqual(loaded["filesystem_documents"][0]["path"], "README.md")
            self.assertEqual(loaded["filesystem_documents"][0]["content"], "Project README")


if __name__ == "__main__":
    unittest.main()
