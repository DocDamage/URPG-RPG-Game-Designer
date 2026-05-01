#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import tempfile
import unittest
import wave
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))

from tools.assets import global_asset_import


PNG_1X1 = (
    b"\x89PNG\r\n\x1a\n"
    b"\x00\x00\x00\rIHDR"
    b"\x00\x00\x00\x01\x00\x00\x00\x01"
    b"\x08\x06\x00\x00\x00"
    b"\x1f\x15\xc4\x89"
)

JPEG_2X3 = (
    b"\xff\xd8"
    b"\xff\xe0\x00\x10JFIF\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00"
    b"\xff\xc0\x00\x11\x08\x00\x03\x00\x02\x03\x01\x11\x00\x02\x11\x00\x03\x11\x00"
    b"\xff\xd9"
)


def write_wav(path: Path) -> None:
    with wave.open(str(path), "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(8000)
        handle.writeframes(b"\x00\x00" * 800)


class GlobalAssetImportTests(unittest.TestCase):
    def test_folder_import_classifies_records_and_duplicates(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "fantasy_pack"
            (source / "characters").mkdir(parents=True)
            (source / "audio" / "bgm").mkdir(parents=True)
            (source / "source").mkdir(parents=True)
            (source / "characters" / "hero.png").write_bytes(PNG_1X1)
            (source / "characters" / "hero-copy.png").write_bytes(PNG_1X1)
            write_wav(source / "audio" / "bgm" / "theme.wav")
            (source / "source" / "hero.psd").write_bytes(b"psd")

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_test_001",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            self.assertEqual(session["status"], "review_ready")
            self.assertEqual(session["sourceKind"], "folder")
            self.assertEqual(session["summary"]["filesScanned"], 4)
            self.assertEqual(session["summary"]["readyCount"], 2)
            self.assertEqual(session["summary"]["duplicateCount"], 1)
            self.assertEqual(session["summary"]["sourceOnlyCount"], 1)

            by_path = {record["relativePath"]: record for record in session["records"]}
            self.assertEqual(by_path["characters/hero.png"]["mediaKind"], "image")
            self.assertEqual(by_path["characters/hero.png"]["width"], 1)
            self.assertTrue(by_path["characters/hero.png"]["previewAvailable"])
            self.assertEqual(by_path["characters/hero.png"]["previewKind"], "image")
            self.assertEqual(by_path["audio/bgm/theme.wav"]["category"], "audio/bgm")
            self.assertTrue(by_path["audio/bgm/theme.wav"]["previewAvailable"])
            self.assertEqual(by_path["audio/bgm/theme.wav"]["previewKind"], "audio")
            self.assertGreater(by_path["audio/bgm/theme.wav"]["durationMs"], 0)
            duplicate_flags = [
                by_path["characters/hero.png"]["duplicate"],
                by_path["characters/hero-copy.png"]["duplicate"],
            ]
            self.assertEqual(duplicate_flags.count(True), 1)
            self.assertTrue(by_path["source/hero.psd"]["sourceOnly"])
            self.assertFalse(by_path["source/hero.psd"]["previewAvailable"])
            self.assertEqual(by_path["source/hero.psd"]["noPreviewDiagnostic"], "no_preview_source_only")

            manifest_path = root / ".urpg" / "asset-library" / "sources" / "import_test_001" / "source_manifest.json"
            self.assertTrue(manifest_path.is_file())
            loaded = json.loads(manifest_path.read_text(encoding="utf-8"))
            self.assertEqual(loaded["sessionId"], "import_test_001")

    def test_zip_import_blocks_path_traversal(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "unsafe_pack.zip"
            with zipfile.ZipFile(archive, "w") as zf:
                zf.writestr("characters/hero.png", PNG_1X1)
                zf.writestr("../escape.txt", "bad")

            session = global_asset_import.build_session(
                source=archive,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_zip_unsafe",
                license_note="",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            self.assertEqual(session["sourceKind"], "zip")
            self.assertEqual(session["status"], "failed")
            self.assertEqual(session["diagnostics"][0]["code"], "unsafe_archive_path")
            escaped = root / ".urpg" / "asset-library" / "sources" / "escape.txt"
            self.assertFalse(escaped.exists())
            self.assertEqual(session["summary"]["missingLicenseCount"], 1)

    def test_unsupported_archive_reports_extractor_diagnostic(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "pack.rar"
            archive.write_bytes(b"not really rar")

            session = global_asset_import.build_session(
                source=archive,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_rar",
                license_note="",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            self.assertEqual(session["sourceKind"], "unsupported_archive")
            self.assertEqual(session["diagnostics"][0]["code"], "unsupported_extractor")
            self.assertEqual(session["summary"]["filesScanned"], 0)

    def test_loose_file_import_enforces_byte_limit(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "hero.png"
            source.write_bytes(PNG_1X1)

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_file_limit",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1,
            )

            self.assertEqual(session["status"], "failed")
            self.assertEqual(session["diagnostics"][0]["code"], "import_byte_limit_exceeded")
            self.assertEqual(session["summary"]["filesScanned"], 0)
            copied = root / ".urpg" / "asset-library" / "sources" / "import_file_limit" / "original" / "hero.png"
            self.assertFalse(copied.exists())

    def test_malformed_zip_reports_stable_diagnostic(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "broken.zip"
            archive.write_bytes(b"not a zip file")

            session = global_asset_import.build_session(
                source=archive,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_broken_zip",
                license_note="",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            self.assertEqual(session["status"], "failed")
            self.assertEqual(session["sourceKind"], "zip")
            self.assertEqual(session["diagnostics"][0]["code"], "archive_read_failed")
            self.assertEqual(session["summary"]["filesScanned"], 0)

    def test_build_session_writes_catalog_import_session_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "hero.png"
            source.write_bytes(PNG_1X1)

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_catalog_mirror",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            catalog_manifest = (
                root
                / ".urpg"
                / "asset-library"
                / "catalog"
                / "import_sessions"
                / "import_catalog_mirror.json"
            )
            self.assertTrue(catalog_manifest.is_file())
            loaded = json.loads(catalog_manifest.read_text(encoding="utf-8"))
            self.assertEqual(loaded["sessionId"], session["sessionId"])

    def test_jpeg_import_records_dimensions(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "portraits"
            source.mkdir()
            (source / "hero_face.jpg").write_bytes(JPEG_2X3)

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_jpeg_dimensions",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            record = session["records"][0]
            self.assertEqual(record["mediaKind"], "image")
            self.assertEqual(record["category"], "portrait")
            self.assertEqual(record["width"], 2)
            self.assertEqual(record["height"], 3)

    def test_rpg_maker_folder_conventions_map_to_urpg_categories(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "mz_pack"
            for folder in [
                "img/characters",
                "img/tilesets",
                "img/faces",
                "img/parallaxes",
                "img/pictures",
                "img/animations",
                "img/system",
                "audio/se",
                "audio/me",
            ]:
                (source / folder).mkdir(parents=True, exist_ok=True)
            (source / "img" / "characters" / "Actor1.png").write_bytes(PNG_1X1)
            (source / "img" / "tilesets" / "Outside_A1.png").write_bytes(PNG_1X1)
            (source / "img" / "faces" / "Actor1.png").write_bytes(PNG_1X1 + b"face")
            (source / "img" / "parallaxes" / "Forest.png").write_bytes(PNG_1X1 + b"parallax")
            (source / "img" / "pictures" / "Title.png").write_bytes(PNG_1X1 + b"picture")
            (source / "img" / "animations" / "Slash.png").write_bytes(PNG_1X1 + b"vfx")
            (source / "img" / "system" / "Window.png").write_bytes(PNG_1X1 + b"system")
            write_wav(source / "audio" / "se" / "Attack1.wav")
            write_wav(source / "audio" / "me" / "Victory.wav")

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_rpg_maker_pack",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            by_path = {record["relativePath"]: record for record in session["records"]}
            self.assertEqual(by_path["img/characters/Actor1.png"]["category"], "sprite")
            self.assertEqual(by_path["img/tilesets/Outside_A1.png"]["category"], "tileset")
            self.assertEqual(by_path["img/faces/Actor1.png"]["category"], "portrait")
            self.assertEqual(by_path["img/parallaxes/Forest.png"]["category"], "background")
            self.assertEqual(by_path["img/pictures/Title.png"]["category"], "background")
            self.assertEqual(by_path["img/animations/Slash.png"]["category"], "vfx")
            self.assertEqual(by_path["img/system/Window.png"]["category"], "ui")
            self.assertEqual(by_path["audio/se/Attack1.wav"]["category"], "audio/se")
            self.assertEqual(by_path["audio/me/Victory.wav"]["category"], "audio/me")

    def test_conversion_audio_records_conversion_plan(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "mz_audio"
            (source / "audio" / "bgm").mkdir(parents=True)
            (source / "audio" / "bgm" / "theme.ogg").write_bytes(b"OggS\x00test")

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_audio_conversion",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            record = session["records"][0]
            self.assertFalse(record["runtimeReady"])
            self.assertTrue(record["conversionRequired"])
            self.assertIn("conversion_required", record["diagnostics"])
            self.assertEqual(record["conversionTargetPath"], "converted/audio/bgm/theme.wav")
            self.assertEqual(record["conversionCommand"][-2:], ["audio/bgm/theme.ogg", "converted/audio/bgm/theme.wav"])
            self.assertEqual(session["summary"]["needsConversionCount"], 1)

    def test_optional_external_archive_extractor_invocation(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "pack.7z"
            archive.write_bytes(b"not really 7z")
            extractor = root / "fake_extractor.py"
            extractor.write_text(
                "import pathlib, sys\n"
                "out = pathlib.Path(sys.argv[2]) / 'img' / 'characters'\n"
                "out.mkdir(parents=True, exist_ok=True)\n"
                "(out / 'Hero.png').write_bytes(b'\\x89PNG\\r\\n\\x1a\\n' + b'0' * 32)\n",
                encoding="utf-8",
            )

            session = global_asset_import.build_session(
                source=archive,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_7z_external",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
                external_extractor_command=[sys.executable, str(extractor)],
            )

            self.assertEqual(session["sourceKind"], "external_archive")
            self.assertEqual(session["status"], "review_ready")
            self.assertEqual(session["diagnostics"][0]["code"], "external_archive_extracted")
            self.assertEqual(session["records"][0]["relativePath"], "img/characters/Hero.png")
            self.assertEqual(session["records"][0]["category"], "sprite")

    def test_animation_sequence_assembly_groups_numbered_frames(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "animations"
            (source / "img" / "animations").mkdir(parents=True)
            for index in range(3):
                (source / "img" / "animations" / f"Slash_{index:03}.png").write_bytes(PNG_1X1 + bytes([index]))

            session = global_asset_import.build_session(
                source=source,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_animation_sequence",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
            )

            self.assertEqual(len(session["sequenceGroups"]), 1)
            group = session["sequenceGroups"][0]
            self.assertEqual(group["sequenceId"], "import_animation_sequence:img-animations-slash")
            self.assertEqual(group["category"], "vfx")
            self.assertEqual(group["frameCount"], 3)
            self.assertEqual(group["frames"], [
                "img/animations/Slash_000.png",
                "img/animations/Slash_001.png",
                "img/animations/Slash_002.png",
            ])
            for index, record in enumerate(sorted(session["records"], key=lambda item: item["relativePath"])):
                self.assertEqual(record["sequenceId"], group["sequenceId"])
                self.assertEqual(record["sequenceFrameIndex"], index)
                self.assertEqual(record["sequenceFrameCount"], 3)


if __name__ == "__main__":
    unittest.main()
