#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import shlex
import shutil
import struct
import subprocess
import wave
import zipfile
from pathlib import Path

IMAGE_EXTS = {"png", "jpg", "jpeg", "gif", "bmp", "webp"}
RUNTIME_AUDIO_EXTS = {"wav"}
CONVERSION_AUDIO_EXTS = {"ogg", "mp3", "flac", "m4a", "aac", "opus"}
SOURCE_EXTS = {"psd", "kra", "aseprite", "ase", "blend"}
TOOL_EXTS = {"exe", "msi", "bat", "cmd", "ps1", "sh", "py", "js", "ts"}
ARCHIVE_EXTS = {"zip", "rar", "7z"}
JUNK_NAMES = {".DS_Store", "Thumbs.db", "Desktop.ini"}
FRAME_SEQUENCE_RE = re.compile(r"^(?P<stem>.+?)(?:[_\-. ]?)(?P<index>\d{2,5})$")
EXTERNAL_EXTRACTOR_ENV = "URPG_ASSET_ARCHIVE_EXTRACTOR"
FATAL_IMPORT_DIAGNOSTICS = {
    "unsafe_archive_path",
    "archive_read_failed",
    "import_file_count_limit_exceeded",
    "import_byte_limit_exceeded",
    "external_extractor_missing",
    "external_extractor_timeout",
    "external_extractor_failed",
}


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")


def slugify(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-") or "asset"


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_image_size(path: Path) -> tuple[int, int] | None:
    ext = path.suffix.lower().lstrip(".")
    try:
        with path.open("rb") as handle:
            if ext == "png":
                data = handle.read(24)
                if len(data) >= 24 and data[:8] == b"\x89PNG\r\n\x1a\n" and data[12:16] == b"IHDR":
                    return struct.unpack(">II", data[16:24])
            if ext == "gif":
                data = handle.read(10)
                if len(data) >= 10 and (data.startswith(b"GIF87a") or data.startswith(b"GIF89a")):
                    return struct.unpack("<HH", data[6:10])
            if ext == "bmp":
                data = handle.read(26)
                if len(data) >= 26 and data[:2] == b"BM":
                    width, height = struct.unpack("<ii", data[18:26])
                    return abs(width), abs(height)
            if ext in {"jpg", "jpeg"}:
                if handle.read(2) != b"\xff\xd8":
                    return None
                while True:
                    marker_start = handle.read(1)
                    if not marker_start:
                        return None
                    if marker_start != b"\xff":
                        continue
                    marker = handle.read(1)
                    while marker == b"\xff":
                        marker = handle.read(1)
                    if not marker or marker in {b"\xd8", b"\xd9"}:
                        return None
                    length_bytes = handle.read(2)
                    if len(length_bytes) != 2:
                        return None
                    segment_length = struct.unpack(">H", length_bytes)[0]
                    if segment_length < 2:
                        return None
                    if marker in {b"\xc0", b"\xc1", b"\xc2", b"\xc3", b"\xc5", b"\xc6", b"\xc7", b"\xc9", b"\xca", b"\xcb", b"\xcd", b"\xce", b"\xcf"}:
                        data = handle.read(5)
                        if len(data) != 5:
                            return None
                        height, width = struct.unpack(">HH", data[1:5])
                        return width, height
                    handle.seek(segment_length - 2, os.SEEK_CUR)
    except OSError:
        return None
    return None


def read_wav_duration_ms(path: Path) -> int | None:
    try:
        with wave.open(str(path), "rb") as handle:
            rate = handle.getframerate()
            if rate <= 0:
                return None
            return int((handle.getnframes() / float(rate)) * 1000)
    except (OSError, EOFError, wave.Error):
        return None


def safe_join(root: Path, relative: str) -> Path:
    candidate = (root / relative).resolve()
    root_resolved = root.resolve()
    if candidate == root_resolved or root_resolved in candidate.parents:
        return candidate
    raise ValueError(f"unsafe_path:{relative}")


def zip_entry_safe(name: str) -> bool:
    normalized = name.replace("\\", "/")
    if not normalized or normalized.startswith("/") or re.match(r"^[A-Za-z]:", normalized):
        return False
    return ".." not in Path(normalized).parts


def copy_file(source: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)


def validate_tree_limits(root: Path, max_files: int, max_bytes: int) -> None:
    file_count = 0
    byte_count = 0
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.name in JUNK_NAMES:
            continue
        file_count += 1
        byte_count += path.stat().st_size
        if file_count > max_files:
            raise ValueError("import_file_count_limit_exceeded")
        if byte_count > max_bytes:
            raise ValueError("import_byte_limit_exceeded")


def run_external_archive_extractor(
    source: Path,
    session_root: Path,
    external_extractor_command: list[str],
    max_files: int,
    max_bytes: int,
) -> tuple[Path, list[dict]]:
    diagnostics: list[dict] = []
    target_root = session_root / "extracted"
    target_root.mkdir(parents=True, exist_ok=True)
    if not external_extractor_command:
        diagnostics.append({"code": "unsupported_extractor", "message": "RAR/7z import requires a configured extractor.", "path": str(source)})
        return target_root, diagnostics
    placeholders = {"{source}": str(source), "{destination}": str(target_root)}
    uses_template = any(placeholder in token for token in external_extractor_command for placeholder in placeholders)
    command = []
    for token in external_extractor_command:
        expanded = token
        for placeholder, value in placeholders.items():
            expanded = expanded.replace(placeholder, value)
        command.append(expanded)
    if not uses_template:
        command.extend([str(source), str(target_root)])
    try:
        result = subprocess.run(command, check=False, capture_output=True, text=True, timeout=120)
    except FileNotFoundError:
        diagnostics.append({"code": "external_extractor_missing", "message": "Configured archive extractor could not be started.", "path": str(source)})
        return target_root, diagnostics
    except subprocess.TimeoutExpired:
        diagnostics.append({"code": "external_extractor_timeout", "message": "Configured archive extractor exceeded the import timeout.", "path": str(source)})
        return target_root, diagnostics
    if result.returncode != 0:
        diagnostics.append({
            "code": "external_extractor_failed",
            "message": "Configured archive extractor failed.",
            "path": str(source),
            "stderr": result.stderr.strip()[:1000],
        })
        return target_root, diagnostics
    try:
        validate_tree_limits(target_root, max_files, max_bytes)
    except ValueError as exc:
        diagnostics.append({"code": str(exc), "message": "External archive extraction exceeded configured safety limits.", "path": str(source)})
        return target_root, diagnostics
    diagnostics.append({"code": "external_archive_extracted", "message": "Archive was extracted with the configured external extractor.", "path": str(source)})
    return target_root, diagnostics


def copy_source_to_quarantine(
    source: Path,
    session_root: Path,
    source_kind: str,
    max_files: int,
    max_bytes: int,
    external_extractor_command: list[str] | None = None,
) -> tuple[Path, list[dict]]:
    diagnostics: list[dict] = []
    if source_kind == "file":
        target_root = session_root / "original"
        if source.suffix.lower() in {".rar", ".7z"}:
            diagnostics.append({"code": "unsupported_extractor", "message": "RAR/7z import requires a configured extractor.", "path": str(source)})
            return target_root, diagnostics
        if max_files < 1:
            raise ValueError("import_file_count_limit_exceeded")
        if source.stat().st_size > max_bytes:
            raise ValueError("import_byte_limit_exceeded")
        copy_file(source, target_root / source.name)
        return target_root, diagnostics

    if source_kind == "folder":
        target_root = session_root / "original"
        copied_files = 0
        copied_bytes = 0
        for path in sorted(source.rglob("*")):
            if not path.is_file() or path.name in JUNK_NAMES:
                continue
            copied_files += 1
            copied_bytes += path.stat().st_size
            if copied_files > max_files:
                raise ValueError("import_file_count_limit_exceeded")
            if copied_bytes > max_bytes:
                raise ValueError("import_byte_limit_exceeded")
            copy_file(path, target_root / path.relative_to(source))
        return target_root, diagnostics

    target_root = session_root / "extracted"
    extracted_files = 0
    extracted_bytes = 0
    try:
        with zipfile.ZipFile(source) as archive:
            for info in archive.infolist():
                if info.is_dir():
                    continue
                if not zip_entry_safe(info.filename):
                    diagnostics.append({"code": "unsafe_archive_path", "message": "ZIP entry escaped the managed import root.", "path": info.filename})
                    continue
                extracted_files += 1
                extracted_bytes += int(info.file_size)
                if extracted_files > max_files:
                    raise ValueError("import_file_count_limit_exceeded")
                if extracted_bytes > max_bytes:
                    raise ValueError("import_byte_limit_exceeded")
                target = safe_join(target_root, info.filename)
                target.parent.mkdir(parents=True, exist_ok=True)
                with archive.open(info) as source_handle, target.open("wb") as target_handle:
                    shutil.copyfileobj(source_handle, target_handle)
    except zipfile.BadZipFile:
        diagnostics.append({"code": "archive_read_failed", "message": "ZIP archive could not be read.", "path": str(source)})
    return target_root, diagnostics


def infer_source_kind(source: Path) -> str:
    if source.is_dir():
        return "folder"
    ext = source.suffix.lower().lstrip(".")
    if ext == "zip":
        return "zip"
    if ext in {"rar", "7z"}:
        return "unsupported_archive"
    return "file"


def infer_media_kind(ext: str) -> str:
    if ext in IMAGE_EXTS:
        return "image"
    if ext in RUNTIME_AUDIO_EXTS or ext in CONVERSION_AUDIO_EXTS:
        return "audio"
    if ext in SOURCE_EXTS:
        return "source"
    if ext in TOOL_EXTS:
        return "tool"
    if ext in ARCHIVE_EXTS:
        return "archive"
    return "unsupported"


def infer_category(path: Path, media_kind: str) -> str:
    lower = path.as_posix().lower()
    parts = lower.split("/")
    if media_kind == "audio":
        if "bgm" in parts or "music" in lower or "theme" in lower:
            return "audio/bgm"
        if "bgs" in parts:
            return "audio/bgs"
        if "me" in parts:
            return "audio/me"
        if "ui" in parts or "button" in lower or "click" in lower:
            return "audio/ui"
        return "audio/se"
    if media_kind == "image":
        if len(parts) >= 2 and parts[0] == "img":
            if parts[1] in {"characters", "sv_actors", "sv_enemies", "enemies"}:
                return "sprite"
            if parts[1] == "tilesets":
                return "tileset"
            if parts[1] == "faces":
                return "portrait"
            if parts[1] in {"parallaxes", "pictures", "battlebacks1", "battlebacks2", "titles1", "titles2"}:
                return "background"
            if parts[1] == "animations":
                return "vfx"
            if parts[1] == "system":
                return "ui"
        if "tileset" in lower or "/tiles/" in lower or "tile" in lower:
            return "tileset"
        if "/ui/" in lower or "button" in lower or "icon" in lower or "window" in lower:
            return "ui"
        if "background" in lower or "parallax" in lower:
            return "background"
        if "portrait" in lower or "face" in lower:
            return "portrait"
        if "vfx" in lower or "effect" in lower:
            return "vfx"
        return "sprite"
    if media_kind == "source":
        return "source/art"
    if media_kind == "tool":
        return "tooling"
    if media_kind == "archive":
        return "archive"
    return "unsupported"


def conversion_target_for(relative: Path) -> str:
    return (Path("converted") / relative).with_suffix(".wav").as_posix()


def preview_metadata(kind: str, source_only: bool, tooling_only: bool) -> tuple[bool, str, str]:
    if kind in {"image", "audio"}:
        return True, kind, ""
    if source_only:
        return False, "none", "no_preview_source_only"
    if tooling_only:
        return False, "none", "no_preview_tooling_only"
    if kind == "archive":
        return False, "none", "no_preview_archive"
    return False, "none", "no_preview_unsupported_format"


def build_record(path: Path, scan_root: Path, session_id: str, source_name: str, license_note: str) -> dict:
    relative = path.relative_to(scan_root)
    ext = path.suffix.lower().lstrip(".")
    kind = infer_media_kind(ext)
    category = infer_category(relative, kind)
    digest = sha256_file(path)
    normalized_ext = f".{ext}" if ext else ""
    normalized_path = f"asset://{session_id}/{category}/{slugify(path.stem)}-{digest[:12]}{normalized_ext}"
    diagnostics: list[str] = []
    source_only = kind == "source"
    tooling_only = kind == "tool"
    runtime_ready = kind == "image" or ext in RUNTIME_AUDIO_EXTS
    if ext in CONVERSION_AUDIO_EXTS:
        diagnostics.append("conversion_required")
    if kind in {"unsupported", "archive"}:
        diagnostics.append("unsupported_format")
        runtime_ready = False
    if tooling_only:
        diagnostics.append("tooling_only")
        runtime_ready = False
    width = height = duration_ms = 0
    size = read_image_size(path) if kind == "image" else None
    if size:
        width, height = size
    if ext in RUNTIME_AUDIO_EXTS:
        duration_ms = read_wav_duration_ms(path) or 0
    conversion_required = ext in CONVERSION_AUDIO_EXTS
    conversion_target = conversion_target_for(relative) if conversion_required else ""
    conversion_command = [
        "ffmpeg",
        "-y",
        "-i",
        relative.as_posix(),
        conversion_target,
    ] if conversion_required else []
    preview_available, preview_kind, no_preview_diagnostic = preview_metadata(kind, source_only, tooling_only)
    return {
        "assetId": f"{session_id}:{digest[:16]}",
        "relativePath": relative.as_posix(),
        "normalizedPath": "" if tooling_only else normalized_path,
        "extension": f".{ext}" if ext else "",
        "mediaKind": kind,
        "category": category,
        "pack": source_name,
        "sha256": digest,
        "sizeBytes": path.stat().st_size,
        "width": width,
        "height": height,
        "durationMs": duration_ms,
        "duplicate": False,
        "duplicateOf": "",
        "sourceOnly": source_only,
        "toolingOnly": tooling_only,
        "runtimeReady": runtime_ready,
        "licenseRequired": not bool(license_note.strip()),
        "diagnostics": diagnostics,
        "conversionRequired": conversion_required,
        "conversionTargetPath": conversion_target,
        "conversionCommand": conversion_command,
        "sequenceId": "",
        "sequenceFrameIndex": -1,
        "sequenceFrameCount": 0,
        "previewAvailable": preview_available,
        "previewKind": preview_kind,
        "noPreviewDiagnostic": no_preview_diagnostic,
    }


def scan_records(scan_root: Path, session_id: str, source_name: str, license_note: str) -> list[dict]:
    records = [
        build_record(path, scan_root, session_id, source_name, license_note)
        for path in sorted(scan_root.rglob("*"))
        if path.is_file() and path.name not in JUNK_NAMES
    ]
    first_by_hash: dict[str, str] = {}
    for record in records:
        digest = record["sha256"]
        if digest in first_by_hash:
            record["duplicate"] = True
            record["duplicateOf"] = first_by_hash[digest]
        else:
            first_by_hash[digest] = record["assetId"]
    return records


def sequence_key_for_record(record: dict) -> tuple[str, str] | None:
    if record["mediaKind"] != "image":
        return None
    relative = Path(record["relativePath"])
    match = FRAME_SEQUENCE_RE.match(relative.stem)
    if not match:
        return None
    return relative.parent.as_posix(), match.group("stem").lower()


def assemble_sequence_groups(records: list[dict], session_id: str) -> list[dict]:
    grouped: dict[tuple[str, str], list[dict]] = {}
    for record in records:
        key = sequence_key_for_record(record)
        if key:
            grouped.setdefault(key, []).append(record)

    sequence_groups: list[dict] = []
    for (folder, stem), frames in sorted(grouped.items()):
        if len(frames) < 2:
            continue
        frames.sort(key=lambda item: item["relativePath"])
        sequence_id = f"{session_id}:{slugify('/'.join(part for part in [folder, stem] if part))}"
        for index, frame in enumerate(frames):
            frame["sequenceId"] = sequence_id
            frame["sequenceFrameIndex"] = index
            frame["sequenceFrameCount"] = len(frames)
        sequence_groups.append({
            "sequenceId": sequence_id,
            "category": frames[0]["category"],
            "mediaKind": "image_sequence",
            "frameCount": len(frames),
            "representativePath": frames[0]["relativePath"],
            "frames": [frame["relativePath"] for frame in frames],
        })
    return sequence_groups


def summarize(records: list[dict]) -> dict:
    summary = {
        "filesScanned": len(records),
        "readyCount": 0,
        "needsConversionCount": 0,
        "duplicateCount": 0,
        "missingLicenseCount": 0,
        "unsupportedCount": 0,
        "sourceOnlyCount": 0,
        "errorCount": 0,
    }
    for record in records:
        diagnostics = set(record["diagnostics"])
        if record["duplicate"]:
            summary["duplicateCount"] += 1
        elif record["sourceOnly"] or record["toolingOnly"]:
            summary["sourceOnlyCount"] += 1
        elif "unsupported_format" in diagnostics:
            summary["unsupportedCount"] += 1
        elif record["licenseRequired"]:
            summary["missingLicenseCount"] += 1
        elif not record["runtimeReady"]:
            summary["needsConversionCount"] += 1
        else:
            summary["readyCount"] += 1
    return summary


def has_fatal_diagnostic(diagnostics: list[dict]) -> bool:
    return any(diagnostic.get("code") in FATAL_IMPORT_DIAGNOSTICS for diagnostic in diagnostics)


def build_session(
    source: Path,
    library_root: Path,
    session_id: str,
    license_note: str,
    max_files: int,
    max_bytes: int,
    external_extractor_command: list[str] | None = None,
) -> dict:
    source = source.resolve()
    source_kind = infer_source_kind(source)
    manifest_source_kind = source_kind
    session_root = library_root / "sources" / session_id
    session_root.mkdir(parents=True, exist_ok=True)
    diagnostics: list[dict] = []
    if source_kind == "unsupported_archive":
        if external_extractor_command:
            manifest_source_kind = "external_archive"
            scan_root, diagnostics = run_external_archive_extractor(source, session_root, external_extractor_command, max_files, max_bytes)
        else:
            diagnostics.append({"code": "unsupported_extractor", "message": "RAR/7z import requires a configured extractor.", "path": str(source)})
            scan_root = session_root / "original"
            scan_root.mkdir(parents=True, exist_ok=True)
    else:
        try:
            scan_root, diagnostics = copy_source_to_quarantine(source, session_root, source_kind, max_files, max_bytes, external_extractor_command)
        except ValueError as exc:
            code = str(exc)
            diagnostics.append({"code": code, "message": "Import session exceeded configured safety limits.", "path": str(source)})
            scan_root = session_root / ("extracted" if source_kind == "zip" else "original")
            scan_root.mkdir(parents=True, exist_ok=True)
    records = [] if has_fatal_diagnostic(diagnostics) else scan_records(
        scan_root,
        session_id,
        source.stem if source.is_file() else source.name,
        license_note,
    )
    sequence_groups = assemble_sequence_groups(records, session_id)
    status = "review_ready" if not has_fatal_diagnostic(diagnostics) else "failed"
    session = {
        "schemaVersion": "1.0.0",
        "sessionId": session_id,
        "sourceKind": manifest_source_kind,
        "sourcePath": str(source),
        "managedSourceRoot": scan_root.as_posix(),
        "status": status,
        "createdAt": iso_now(),
        "summary": summarize(records),
        "records": records,
        "sequenceGroups": sequence_groups,
        "diagnostics": diagnostics,
        "licenseNote": license_note,
    }
    source_manifest = session_root / "source_manifest.json"
    source_manifest.write_text(json.dumps(session, indent=2), encoding="utf-8")
    catalog_manifest = library_root / "catalog" / "import_sessions" / f"{session_id}.json"
    catalog_manifest.parent.mkdir(parents=True, exist_ok=True)
    catalog_manifest.write_text(json.dumps(session, indent=2), encoding="utf-8")
    return session


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Import a file, folder, or ZIP into the URPG global asset library quarantine.")
    parser.add_argument("--source", required=True, help="Source file, folder, or ZIP archive.")
    parser.add_argument("--library-root", default=".urpg/asset-library", help="Managed global asset library root.")
    parser.add_argument("--session-id", help="Stable session id. Defaults to import_<UTC timestamp>.")
    parser.add_argument("--license-note", default="", help="User-provided license/attribution note for review.")
    parser.add_argument("--max-files", type=int, default=10000)
    parser.add_argument("--max-bytes", type=int, default=1024 * 1024 * 1024)
    parser.add_argument(
        "--external-extractor-command",
        help=(
            "Optional command for RAR/7z extraction. Source and destination are appended as final arguments "
            "unless {source}/{destination} placeholders are present. Defaults to URPG_ASSET_ARCHIVE_EXTRACTOR."
        ),
    )
    parser.add_argument("--output", help="Optional output manifest path.")
    return parser.parse_args(argv)


def configured_external_extractor_command(cli_value: str | None) -> list[str] | None:
    command = cli_value if cli_value is not None else os.environ.get(EXTERNAL_EXTRACTOR_ENV)
    if command is None or not command.strip():
        return None
    try:
        return shlex.split(command)
    except ValueError as exc:
        raise ValueError(f"{EXTERNAL_EXTRACTOR_ENV} could not be parsed: {exc}") from exc


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    source = Path(args.source)
    if not source.exists():
        raise SystemExit(f"source not found: {source}")
    session_id = args.session_id or f"import_{dt.datetime.now(dt.timezone.utc).strftime('%Y%m%d_%H%M%S')}"
    library_root = Path(args.library_root)
    try:
        external_extractor_command = configured_external_extractor_command(args.external_extractor_command)
    except ValueError as exc:
        raise SystemExit(str(exc))
    session = build_session(source, library_root, session_id, args.license_note, args.max_files, args.max_bytes, external_extractor_command)
    output = Path(args.output) if args.output else library_root / "catalog" / "import_sessions" / f"{session_id}.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(session, indent=2), encoding="utf-8")
    print(json.dumps({"sessionId": session_id, "status": session["status"], "output": output.as_posix(), "summary": session["summary"]}, indent=2))
    return 0 if session["status"] == "review_ready" else 2


if __name__ == "__main__":
    raise SystemExit(main())
