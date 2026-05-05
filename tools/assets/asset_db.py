#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import shutil
import sqlite3
import struct
import subprocess
import sys
from pathlib import Path

SCHEMA_VERSION = 1
DEFAULT_DB = Path(".urpg/asset-index/asset_catalog.db")
DEFAULT_ROOTS = [
    "imports/raw/third_party_assets/itch-assets/packs",
    "imports/raw/third_party_assets/itch-assets/loose-files",
    "imports/raw/third_party_assets/rpgmaker-mz",
    "imports/raw/third_party_assets/huggingface",
    "imports/raw/third_party_assets/aseprite",
    "imports/raw/third_party_assets/external-repos",
    "imports/raw/third_party_assets/github_assets",
    "imports/raw/itch_assets/loose",
    "imports/root-drop/archives",
    "imports/raw/more_assets",
    "imports/raw/more_assets_to_ingest",
    "imports/raw/src014_20260504_extracted",
    "imports/raw/assets_for_my_game",
    "imports/raw/godogenui_assets",
    "imports/raw/urpg_stuff",
]
EXCLUDED_DIRS = {
    ".git",
    ".venv",
    "__pycache__",
    "node_modules",
    "packs-by-category",
    "unzipped",
}

IMAGE_EXTS = {
    "png",
    "jpg",
    "jpeg",
    "gif",
    "bmp",
    "webp",
    "ase",
    "aseprite",
    "tif",
    "tiff",
}
AUDIO_EXTS = {"ogg", "wav", "mp3", "flac", "m4a", "aac", "opus"}
VIDEO_EXTS = {"mp4", "webm", "mov", "mkv", "avi"}
ARCHIVE_EXTS = {"zip", "7z", "rar", "tar", "gz", "bz2", "xz"}
TEXT_EXTS = {
    "txt",
    "md",
    "csv",
    "json",
    "js",
    "ts",
    "yaml",
    "yml",
    "xml",
    "ini",
    "cfg",
    "log",
    "pdf",
}


def now_utc() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def slugify(s: str) -> str:
    s = re.sub(r"[^a-z0-9]+", "-", s.lower()).strip("-")
    return s or "unknown"


def infer_kind(ext: str) -> str:
    if ext in IMAGE_EXTS:
        return "image"
    if ext in AUDIO_EXTS:
        return "audio"
    if ext in VIDEO_EXTS:
        return "video"
    if ext in ARCHIVE_EXTS:
        return "archive"
    if ext in TEXT_EXTS:
        return "text-data"
    return "binary"


def infer_pack_category(path_rel: str) -> tuple[str | None, str | None]:
    parts = path_rel.replace("\\", "/").split("/")
    pack = None
    category = None
    if "itch-assets" in parts and "packs" in parts:
        i = parts.index("packs")
        if i + 1 < len(parts):
            pack = parts[i + 1]
        category = "itch-pack"
    if "steam-dlc" in parts:
        category = "rpgmaker-mz-steam-dlc"
        if "packs" in parts:
            i = parts.index("packs")
            if i + 1 < len(parts):
                pack = parts[i + 1]
    if "cgmz" in parts:
        category = "rpgmaker-mz-cgmz"
        if "packages" in parts:
            i = parts.index("packages")
            if i + 1 < len(parts):
                pack = parts[i + 1]
    if "visumz-sample-project" in parts and not category:
        category = "rpgmaker-mz-visustella-sample"
        pack = pack or "VisuMZ_Sample_Game_Project"
    if "huggingface" in parts:
        i = parts.index("huggingface")
        if i + 1 < len(parts) and parts[i + 1] != "README.md":
            category = f"huggingface-{parts[i + 1]}"
            pack = pack or parts[i + 1]
    if "itch_assets" in parts and "loose" in parts:
        category = "itch-loose"
        pack = pack or "loose"
    if "imports" in parts and "raw" in parts and "more_assets" in parts:
        i = parts.index("more_assets")
        if i + 1 < len(parts):
            pack = parts[i + 1]
        category = "more-assets-raw"
    if "imports" in parts and "raw" in parts and "more_assets_to_ingest" in parts:
        i = parts.index("more_assets_to_ingest")
        if i + 1 < len(parts):
            pack = parts[i + 1]
        category = "more-assets-to-ingest-raw"
    if "imports" in parts and "raw" in parts and "src014_20260504_extracted" in parts:
        i = parts.index("src014_20260504_extracted")
        if i + 2 < len(parts) and parts[i + 1] == "__archive_extracted":
            pack = parts[i + 2]
        elif i + 1 < len(parts):
            pack = parts[i + 1]
        category = "src014-20260504-extracted"
    if "imports" in parts and "raw" in parts and "assets_for_my_game" in parts:
        i = parts.index("assets_for_my_game")
        if i + 1 < len(parts):
            pack = parts[i + 1]
        category = "assets-for-my-game-raw"
    if "imports" in parts and "raw" in parts and "godogenui_assets" in parts:
        i = parts.index("godogenui_assets")
        if i + 1 < len(parts):
            pack = parts[i + 1]
        category = "godogenui-assets-raw"
    if "imports" in parts and "raw" in parts and "urpg_stuff" in parts:
        i = parts.index("urpg_stuff")
        if i + 1 < len(parts):
            pack = parts[i + 1]
        category = "urpg-stuff-local"
    return pack, category


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_image_size(path: Path, ext: str) -> tuple[int, int] | None:
    ext = ext.lower()
    try:
        with path.open("rb") as f:
            if ext == "png":
                d = f.read(24)
                if (
                    len(d) >= 24
                    and d[:8] == b"\x89PNG\r\n\x1a\n"
                    and d[12:16] == b"IHDR"
                ):
                    return struct.unpack(">II", d[16:24])
            if ext in {"jpg", "jpeg"}:
                if f.read(2) != b"\xff\xd8":
                    return None
                while True:
                    b = f.read(1)
                    if not b:
                        return None
                    if b != b"\xff":
                        continue
                    while b == b"\xff":
                        b = f.read(1)
                    if not b:
                        return None
                    marker = b[0]
                    if marker in {0xD8, 0xD9}:
                        continue
                    seg = f.read(2)
                    if len(seg) != 2:
                        return None
                    seg_len = struct.unpack(">H", seg)[0]
                    if marker in {
                        0xC0,
                        0xC1,
                        0xC2,
                        0xC3,
                        0xC5,
                        0xC6,
                        0xC7,
                        0xC9,
                        0xCA,
                        0xCB,
                        0xCD,
                        0xCE,
                        0xCF,
                    }:
                        p = f.read(seg_len - 2)
                        if len(p) >= 5:
                            return struct.unpack(">H", p[3:5])[0], struct.unpack(
                                ">H", p[1:3]
                            )[0]
                        return None
                    f.seek(seg_len - 2, os.SEEK_CUR)
            if ext == "gif":
                d = f.read(10)
                if len(d) >= 10 and (
                    d.startswith(b"GIF87a") or d.startswith(b"GIF89a")
                ):
                    return struct.unpack("<HH", d[6:10])
            if ext == "bmp":
                d = f.read(26)
                if len(d) >= 26 and d[:2] == b"BM":
                    w, h = struct.unpack("<ii", d[18:26])
                    return abs(w), abs(h)
            if ext == "webp":
                d = f.read(64)
                if (
                    len(d) >= 30
                    and d[:4] == b"RIFF"
                    and d[8:12] == b"WEBP"
                    and d[12:16] == b"VP8X"
                ):
                    w = 1 + int.from_bytes(d[24:27], "little")
                    h = 1 + int.from_bytes(d[27:30], "little")
                    return w, h
    except Exception:
        return None
    return None


def ffprobe_duration_ms(path: Path, ffprobe_bin: str | None) -> int | None:
    if not ffprobe_bin:
        return None
    cmd = [
        ffprobe_bin,
        "-v",
        "error",
        "-show_entries",
        "format=duration",
        "-of",
        "default=noprint_wrappers=1:nokey=1",
        str(path),
    ]
    try:
        out = subprocess.check_output(cmd, stderr=subprocess.DEVNULL, text=True).strip()
        if not out:
            return None
        return int(float(out) * 1000.0)
    except Exception:
        return None


class Catalog:
    def __init__(self, repo_root: Path, db_path: Path) -> None:
        self.repo_root = repo_root.resolve()
        self.db_path = db_path.resolve()
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        self.conn = sqlite3.connect(str(self.db_path))
        self.conn.row_factory = sqlite3.Row
        self.conn.execute("PRAGMA journal_mode=WAL")
        self.conn.execute("PRAGMA foreign_keys=ON")
        self.ffprobe_bin = shutil.which("ffprobe") or shutil.which("ffprobe.exe")

    def close(self) -> None:
        self.conn.close()

    def init_db(self) -> None:
        self.conn.executescript(
            """
            CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT NOT NULL);
            CREATE TABLE IF NOT EXISTS scan_runs (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              started_at TEXT NOT NULL, finished_at TEXT, roots_json TEXT NOT NULL,
              files_seen INTEGER NOT NULL DEFAULT 0, files_indexed INTEGER NOT NULL DEFAULT 0,
              files_skipped INTEGER NOT NULL DEFAULT 0, files_removed INTEGER NOT NULL DEFAULT 0
            );
            CREATE TABLE IF NOT EXISTS assets (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              path_abs TEXT NOT NULL UNIQUE, path_rel TEXT NOT NULL, source_root TEXT NOT NULL,
              filename TEXT NOT NULL, stem TEXT NOT NULL, ext TEXT NOT NULL,
              size_bytes INTEGER NOT NULL, mtime_ns INTEGER NOT NULL, sha256 TEXT,
              media_kind TEXT NOT NULL, pack TEXT, category TEXT,
              width INTEGER, height INTEGER, duration_ms INTEGER, extra_json TEXT,
              first_seen_at TEXT NOT NULL, last_seen_at TEXT NOT NULL, last_scan_id INTEGER,
              missing INTEGER NOT NULL DEFAULT 0
            );
            CREATE INDEX IF NOT EXISTS idx_assets_kind ON assets(media_kind);
            CREATE INDEX IF NOT EXISTS idx_assets_ext ON assets(ext);
            CREATE INDEX IF NOT EXISTS idx_assets_sha ON assets(sha256);
            CREATE INDEX IF NOT EXISTS idx_assets_pack ON assets(pack);
            CREATE INDEX IF NOT EXISTS idx_assets_cat ON assets(category);
            CREATE INDEX IF NOT EXISTS idx_assets_missing ON assets(missing);
            CREATE TABLE IF NOT EXISTS tags (
              id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE
            );
            CREATE TABLE IF NOT EXISTS asset_tags (
              asset_id INTEGER NOT NULL, tag_id INTEGER NOT NULL, source TEXT NOT NULL DEFAULT 'manual',
              PRIMARY KEY(asset_id, tag_id)
            );
            CREATE VIRTUAL TABLE IF NOT EXISTS asset_fts USING fts5(path_rel, filename, pack, category, tags);
            CREATE VIEW IF NOT EXISTS duplicate_groups AS
              SELECT sha256, COUNT(*) AS copies, SUM(size_bytes) AS total_size_bytes
              FROM assets WHERE missing=0 AND sha256 IS NOT NULL GROUP BY sha256 HAVING COUNT(*) > 1;
            """
        )
        self.conn.execute(
            "INSERT OR REPLACE INTO settings(key, value) VALUES(?, ?)",
            ("schema_version", str(SCHEMA_VERSION)),
        )
        self.conn.commit()

    def _iter_files(self, root: Path):
        if root.is_file():
            yield root
            return
        for current, dirs, files in os.walk(root, topdown=True, followlinks=False):
            dirs[:] = [d for d in dirs if d not in EXCLUDED_DIRS]
            base = Path(current)
            for name in files:
                yield base / name

    def _rel(self, p: Path) -> str:
        return str(p.resolve().relative_to(self.repo_root)).replace("\\", "/")

    def _source_root_rel(self, p: Path, roots: list[Path]) -> str:
        ap = p.resolve()
        for r in roots:
            rr = r.resolve()
            if r.is_dir() and str(ap).startswith(str(rr)):
                return str(rr.relative_to(self.repo_root)).replace("\\", "/")
            if r.is_file() and ap == rr:
                return str(rr.relative_to(self.repo_root)).replace("\\", "/")
        return "."

    def _existing_by_abs(self) -> dict[str, sqlite3.Row]:
        rows = self.conn.execute(
            "SELECT id,path_abs,path_rel,size_bytes,mtime_ns,sha256,missing FROM assets"
        ).fetchall()
        return {r["path_abs"]: r for r in rows}

    def _rebuild_auto_tags(self) -> None:
        cur = self.conn.cursor()
        cur.execute("DELETE FROM asset_tags WHERE source='auto'")
        tag_cache = {
            r["name"]: int(r["id"]) for r in cur.execute("SELECT id,name FROM tags")
        }

        def ensure_tag(name: str) -> int:
            if name in tag_cache:
                return tag_cache[name]
            cur.execute("INSERT OR IGNORE INTO tags(name) VALUES(?)", (name,))
            row = cur.execute("SELECT id FROM tags WHERE name=?", (name,)).fetchone()
            tid = int(row["id"])
            tag_cache[name] = tid
            return tid

        rows = cur.execute(
            "SELECT id,filename,media_kind,ext,pack,category FROM assets WHERE missing=0"
        ).fetchall()
        inserts: list[tuple[int, int, str]] = []
        for r in rows:
            tags = {
                f"kind:{r['media_kind']}",
                f"ext:{r['ext']}",
            }
            if r["category"]:
                tags.add(f"category:{slugify(r['category'])}")
            if r["pack"]:
                tags.add(f"pack:{slugify(r['pack'])}")
            for token in re.findall(r"[a-z0-9]{3,}", Path(r["filename"]).stem.lower())[
                :8
            ]:
                tags.add(f"name:{token}")
            for t in tags:
                inserts.append((int(r["id"]), ensure_tag(t), "auto"))
        if inserts:
            cur.executemany(
                "INSERT OR IGNORE INTO asset_tags(asset_id,tag_id,source) VALUES(?,?,?)",
                inserts,
            )

    def _rebuild_fts(self) -> None:
        cur = self.conn.cursor()
        cur.execute("DELETE FROM asset_fts")
        rows = cur.execute(
            """
            SELECT a.id,a.path_rel,a.filename,COALESCE(a.pack,'') pack,COALESCE(a.category,'') category,
                   COALESCE(GROUP_CONCAT(t.name,' '),'') tags
            FROM assets a
            LEFT JOIN asset_tags at ON at.asset_id=a.id
            LEFT JOIN tags t ON t.id=at.tag_id
            WHERE a.missing=0
            GROUP BY a.id
            """
        ).fetchall()
        cur.executemany(
            "INSERT INTO asset_fts(rowid,path_rel,filename,pack,category,tags) VALUES(?,?,?,?,?,?)",
            [
                (
                    int(r["id"]),
                    r["path_rel"],
                    r["filename"],
                    r["pack"],
                    r["category"],
                    r["tags"],
                )
                for r in rows
            ],
        )

    def index(self, roots: list[Path], force: bool = False) -> dict[str, int]:
        now = now_utc()
        roots_json = json.dumps(
            [
                str(r.resolve().relative_to(self.repo_root)).replace("\\", "/")
                for r in roots
            ]
        )
        cur = self.conn.cursor()
        cur.execute(
            "INSERT INTO scan_runs(started_at,roots_json) VALUES(?,?)",
            (now, roots_json),
        )
        scan_id = int(cur.lastrowid)

        existing = self._existing_by_abs()
        seen: set[str] = set()
        files_seen = files_indexed = files_skipped = 0

        for root in roots:
            for p in self._iter_files(root):
                files_seen += 1
                try:
                    st = p.stat()
                except OSError:
                    continue
                p_abs = str(p.resolve())
                p_rel = self._rel(p)
                src_root = self._source_root_rel(p, roots)
                seen.add(p_abs)
                ext = p.suffix.lower().lstrip(".")
                kind = infer_kind(ext)
                old = existing.get(p_abs)
                unchanged = (
                    old is not None
                    and int(old["size_bytes"]) == int(st.st_size)
                    and int(old["mtime_ns"]) == int(st.st_mtime_ns)
                    and not force
                )
                if unchanged:
                    pack, category = infer_pack_category(p_rel)
                    cur.execute(
                        """
                        UPDATE assets
                        SET path_rel=?,source_root=?,media_kind=?,pack=?,category=?,
                            missing=0,last_seen_at=?,last_scan_id=?
                        WHERE path_abs=?
                        """,
                        (p_rel, src_root, kind, pack, category, now, scan_id, p_abs),
                    )
                    files_skipped += 1
                    continue

                sha = sha256_file(p)
                width = height = duration_ms = None
                if kind == "image":
                    size = read_image_size(p, ext)
                    if size:
                        width, height = int(size[0]), int(size[1])
                elif kind in {"audio", "video"}:
                    duration_ms = ffprobe_duration_ms(p, self.ffprobe_bin)
                pack, category = infer_pack_category(p_rel)
                extra = {}
                if width and height:
                    extra["pixels"] = width * height

                cur.execute(
                    """
                    INSERT INTO assets(
                      path_abs,path_rel,source_root,filename,stem,ext,size_bytes,mtime_ns,sha256,
                      media_kind,pack,category,width,height,duration_ms,extra_json,
                      first_seen_at,last_seen_at,last_scan_id,missing
                    ) VALUES(
                      :path_abs,:path_rel,:source_root,:filename,:stem,:ext,:size_bytes,:mtime_ns,:sha256,
                      :media_kind,:pack,:category,:width,:height,:duration_ms,:extra_json,
                      :first_seen_at,:last_seen_at,:last_scan_id,0
                    )
                    ON CONFLICT(path_abs) DO UPDATE SET
                      path_rel=excluded.path_rel,
                      source_root=excluded.source_root,
                      filename=excluded.filename,
                      stem=excluded.stem,
                      ext=excluded.ext,
                      size_bytes=excluded.size_bytes,
                      mtime_ns=excluded.mtime_ns,
                      sha256=excluded.sha256,
                      media_kind=excluded.media_kind,
                      pack=excluded.pack,
                      category=excluded.category,
                      width=excluded.width,
                      height=excluded.height,
                      duration_ms=excluded.duration_ms,
                      extra_json=excluded.extra_json,
                      last_seen_at=excluded.last_seen_at,
                      last_scan_id=excluded.last_scan_id,
                      missing=0
                    """,
                    {
                        "path_abs": p_abs,
                        "path_rel": p_rel,
                        "source_root": src_root,
                        "filename": p.name,
                        "stem": p.stem,
                        "ext": ext,
                        "size_bytes": int(st.st_size),
                        "mtime_ns": int(st.st_mtime_ns),
                        "sha256": sha,
                        "media_kind": kind,
                        "pack": pack,
                        "category": category,
                        "width": width,
                        "height": height,
                        "duration_ms": duration_ms,
                        "extra_json": json.dumps(extra) if extra else None,
                        "first_seen_at": now,
                        "last_seen_at": now,
                        "last_scan_id": scan_id,
                    },
                )
                files_indexed += 1

        removed = [
            (now, scan_id, p_abs) for p_abs in existing.keys() if p_abs not in seen
        ]
        if removed:
            cur.executemany(
                "UPDATE assets SET missing=1,last_seen_at=?,last_scan_id=? WHERE path_abs=?",
                removed,
            )
        files_removed = len(removed)

        self._rebuild_auto_tags()
        self._rebuild_fts()

        cur.execute(
            "UPDATE scan_runs SET finished_at=?,files_seen=?,files_indexed=?,files_skipped=?,files_removed=? WHERE id=?",
            (
                now_utc(),
                files_seen,
                files_indexed,
                files_skipped,
                files_removed,
                scan_id,
            ),
        )
        self.conn.commit()
        return {
            "scan_id": scan_id,
            "files_seen": files_seen,
            "files_indexed": files_indexed,
            "files_skipped": files_skipped,
            "files_removed": files_removed,
        }

    def find(
        self,
        query: str | None,
        limit: int,
        ext: str | None,
        kind: str | None,
        pack: str | None,
        category: str | None,
    ):
        where = ["a.missing=0"]
        params: list[object] = []
        join = ""
        order = "a.path_rel ASC"
        if query:
            join = "JOIN asset_fts f ON f.rowid=a.id"
            where.append("f.asset_fts MATCH ?")
            params.append(query)
            order = "bm25(asset_fts), a.size_bytes DESC"
        if ext:
            where.append("a.ext=?")
            params.append(ext.lower().lstrip("."))
        if kind:
            where.append("a.media_kind=?")
            params.append(kind)
        if pack:
            where.append("a.pack LIKE ?")
            params.append(f"%{pack}%")
        if category:
            where.append("a.category LIKE ?")
            params.append(f"%{category}%")
        sql = f"""
          SELECT a.id,a.path_rel,a.media_kind,a.ext,a.size_bytes,a.pack,a.category,a.width,a.height,a.duration_ms
          FROM assets a {join}
          WHERE {" AND ".join(where)}
          ORDER BY {order}
          LIMIT ?
        """
        params.append(limit)
        return self.conn.execute(sql, params).fetchall()

    def dupes(self, limit: int):
        return self.conn.execute(
            "SELECT sha256,copies,total_size_bytes FROM duplicate_groups ORDER BY copies DESC,total_size_bytes DESC LIMIT ?",
            (limit,),
        ).fetchall()

    def dupes_for_hash(self, h: str):
        return self.conn.execute(
            "SELECT path_rel,size_bytes,media_kind,ext FROM assets WHERE missing=0 AND sha256=? ORDER BY path_rel",
            (h,),
        ).fetchall()

    def stats(self):
        return {
            "kind": self.conn.execute(
                "SELECT media_kind,COUNT(*) count,SUM(size_bytes) size FROM assets WHERE missing=0 GROUP BY media_kind ORDER BY count DESC"
            ).fetchall(),
            "ext": self.conn.execute(
                "SELECT ext,COUNT(*) count FROM assets WHERE missing=0 GROUP BY ext ORDER BY count DESC LIMIT 30"
            ).fetchall(),
            "category": self.conn.execute(
                "SELECT COALESCE(category,'(none)') category,COUNT(*) count FROM assets WHERE missing=0 GROUP BY COALESCE(category,'(none)') ORDER BY count DESC"
            ).fetchall(),
        }


def resolve_roots(repo_root: Path, roots: list[str]) -> list[Path]:
    out: list[Path] = []
    for root in roots:
        p = (repo_root / root).resolve()
        if p.exists():
            out.append(p)
    return out


def cmd_init(cat: Catalog, _args: argparse.Namespace) -> int:
    cat.init_db()
    print(f"Initialized: {cat.db_path}")
    return 0


def cmd_index(cat: Catalog, args: argparse.Namespace) -> int:
    cat.init_db()
    roots = resolve_roots(cat.repo_root, args.roots or DEFAULT_ROOTS)
    if not roots:
        print("No valid index roots.", file=sys.stderr)
        return 2
    print(json.dumps(cat.index(roots=roots, force=args.force), indent=2))
    return 0


def cmd_find(cat: Catalog, args: argparse.Namespace) -> int:
    rows = cat.find(
        args.query, args.limit, args.ext, args.kind, args.pack, args.category
    )
    for r in rows:
        print(dict(r))
    return 0


def cmd_dupes(cat: Catalog, args: argparse.Namespace) -> int:
    for r in cat.dupes(args.limit):
        print(
            f"sha256={r['sha256']} copies={r['copies']} total_size_bytes={r['total_size_bytes']}"
        )
        if args.show_paths:
            for f in cat.dupes_for_hash(r["sha256"]):
                print(
                    f"  - {f['path_rel']} ({f['size_bytes']} bytes, {f['media_kind']}/{f['ext']})"
                )
    return 0


def cmd_stats(cat: Catalog, _args: argparse.Namespace) -> int:
    s = cat.stats()
    print("== by_kind ==")
    for r in s["kind"]:
        print(f"{r['media_kind']}: count={r['count']} size_bytes={r['size'] or 0}")
    print("\n== by_ext ==")
    for r in s["ext"]:
        print(f"{r['ext']}: {r['count']}")
    print("\n== by_category ==")
    for r in s["category"]:
        print(f"{r['category']}: {r['count']}")
    return 0


def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Robust asset catalog (SQLite + FTS + duplicate tracking)"
    )
    p.add_argument("--repo-root", default=".", help="Repo root (default: .)")
    p.add_argument(
        "--db", default=str(DEFAULT_DB), help=f"DB path (default: {DEFAULT_DB})"
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("init", help="Initialize schema.")

    idx = sub.add_parser("index", help="Incremental index scan.")
    idx.add_argument("--roots", nargs="*", help="Relative roots to index.")
    idx.add_argument("--force", action="store_true", help="Re-index unchanged files.")

    f = sub.add_parser("find", help="Search catalog.")
    f.add_argument("--query", help="FTS query (optional).")
    f.add_argument("--ext", help="Extension filter.")
    f.add_argument("--kind", help="Kind filter.")
    f.add_argument("--pack", help="Pack filter substring.")
    f.add_argument("--category", help="Category filter substring.")
    f.add_argument("--limit", type=int, default=50)

    d = sub.add_parser("dupes", help="Duplicate binary groups.")
    d.add_argument("--limit", type=int, default=50)
    d.add_argument("--show-paths", action="store_true")

    sub.add_parser("stats", help="Catalog stats.")
    return p


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    repo_root = Path(args.repo_root).resolve()
    db = (repo_root / args.db).resolve()
    cat = Catalog(repo_root, db)
    try:
        if args.cmd == "init":
            return cmd_init(cat, args)
        if args.cmd == "index":
            return cmd_index(cat, args)
        if args.cmd == "find":
            return cmd_find(cat, args)
        if args.cmd == "dupes":
            return cmd_dupes(cat, args)
        if args.cmd == "stats":
            return cmd_stats(cat, args)
        return 2
    finally:
        cat.close()


if __name__ == "__main__":
    raise SystemExit(main())
