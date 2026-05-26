#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any


SERVER_NAME = "urpg-local-mcp"
SERVER_VERSION = "0.1.0"

FOCUSED_GATES: dict[str, list[str]] = {
    "build_spatial": ["cmake", "--build", "--preset", "dev-debug", "--target", "urpg_spatial_unit_tests"],
    "p2d_depth": [
        ".\\build\\dev-ninja-debug\\urpg_spatial_unit_tests.exe",
        "[editor][spatial][p2d_depth]",
        "--reporter",
        "compact",
    ],
    "spatial_editor": [
        ".\\build\\dev-ninja-debug\\urpg_spatial_unit_tests.exe",
        "[editor][spatial]",
        "--reporter",
        "compact",
    ],
    "diff_check": ["git", "diff", "--check"],
}

GUARDRAILS = [
    "local_only",
    "allowlisted_commands_only",
    "no_destructive_git",
    "no_release_gate_bypass",
    "no_asset_license_bypass",
]

PROJECT_PATCH_KINDS = [
    "set_startup_map",
    "set_map_asset",
    "add_p2d_map",
    "add_p2d_event",
    "add_actor",
    "add_item",
    "add_switch",
    "add_variable",
    "add_common_event",
    "add_asset_reference",
]

DATABASE_PATCH_TARGETS = {
    "add_actor": ("actors", "/database/actors"),
    "add_item": ("items", "/database/items"),
    "add_switch": ("switches", "/database/switches"),
    "add_variable": ("variables", "/database/variables"),
    "add_common_event": ("common_events", "/database/common_events"),
}


@dataclass(frozen=True)
class CompletedProcessLike:
    returncode: int
    stdout: str
    stderr: str


class ToolError(RuntimeError):
    def __init__(self, code: str, message: str) -> None:
        super().__init__(message)
        self.code = code
        self.message = message


def _object_schema(properties: dict[str, Any] | None = None, required: list[str] | None = None) -> dict[str, Any]:
    schema: dict[str, Any] = {
        "type": "object",
        "properties": properties or {},
        "additionalProperties": False,
    }
    if required:
        schema["required"] = required
    return schema


def list_tools() -> list[dict[str, Any]]:
    return [
        {
            "name": "urpg.project_status",
            "description": "Inspect the local URPG checkout branch, head, dirty state, and safety guardrails.",
            "inputSchema": _object_schema(),
        },
        {
            "name": "urpg.project_summary",
            "description": "Read a bounded URPG project JSON file and return maps, P2D counts, startup, and assets.",
            "inputSchema": _object_schema(
                {"project_path": {"type": "string", "default": "project.json"}},
            ),
        },
        {
            "name": "urpg.project_validate",
            "description": "Validate bounded URPG project JSON references for startup and P2D records.",
            "inputSchema": _object_schema(
                {"project_path": {"type": "string", "default": "project.json"}},
            ),
        },
        {
            "name": "urpg.project_patch",
            "description": "Preview or explicitly apply an allowlisted URPG project JSON patch.",
            "inputSchema": _object_schema(
                {
                    "project_path": {"type": "string", "default": "project.json"},
                    "patch_kind": {
                        "type": "string",
                        "enum": PROJECT_PATCH_KINDS,
                    },
                    "value": {"type": "string"},
                    "key": {"type": "string"},
                    "map_id": {"type": "string"},
                    "label": {"type": "string"},
                    "path": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["patch_kind", "value"],
            ),
        },
        {
            "name": "urpg.asset_catalog_summary",
            "description": "Read a bounded URPG asset catalog JSON file and summarize media, license, and release readiness.",
            "inputSchema": _object_schema(
                {"catalog_path": {"type": "string", "default": "asset_catalog.json"}},
            ),
        },
        {
            "name": "urpg.p2d_capabilities",
            "description": "List the current Perspective 2D authoring/runtime surfaces exposed to IDE agents.",
            "inputSchema": _object_schema(),
        },
        {
            "name": "urpg.focused_gate",
            "description": "Return or optionally run an allowlisted focused URPG validation command.",
            "inputSchema": _object_schema(
                {
                    "gate_id": {"type": "string", "enum": sorted(FOCUSED_GATES)},
                    "run": {"type": "boolean", "default": False},
                    "timeout_seconds": {"type": "integer", "minimum": 1, "maximum": 300, "default": 120},
                },
                ["gate_id"],
            ),
        },
        {
            "name": "urpg.release_guardrails",
            "description": "Return the safety rules this local MCP refuses to bypass.",
            "inputSchema": _object_schema(),
        },
    ]


def _run_git_status(repo_root: Path) -> CompletedProcessLike:
    completed = subprocess.run(
        ["git", "branch", "--show-current"],
        cwd=repo_root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    head = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"],
        cwd=repo_root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    status = subprocess.run(
        ["git", "status", "--short"],
        cwd=repo_root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return CompletedProcessLike(
        max(completed.returncode, head.returncode, status.returncode),
        completed.stdout + head.stdout + status.stdout,
        completed.stderr + head.stderr + status.stderr,
    )


def _project_status(repo_root: Path) -> dict[str, Any]:
    diagnostics: list[str] = []
    branch = "unknown"
    head = "unknown"
    dirty_files: list[str] = []
    completed = _run_git_status(repo_root)
    if completed.returncode == 0:
        lines = completed.stdout.splitlines()
        if len(lines) >= 1 and lines[0].strip():
            branch = lines[0].strip()
        if len(lines) >= 2 and lines[1].strip():
            head = lines[1].strip()
        dirty_files = [line for line in lines[2:] if line.strip()]
    else:
        diagnostics.append("repo_status_unavailable")

    return {
        "repo_root": str(repo_root),
        "branch": branch,
        "head": head,
        "dirty": bool(dirty_files),
        "dirty_files": dirty_files,
        "guardrails": GUARDRAILS,
        "diagnostics": diagnostics,
    }


def _resolve_repo_file(repo_root: Path, project_path: str) -> Path:
    root = repo_root.resolve()
    candidate = (root / project_path).resolve()
    try:
        candidate.relative_to(root)
    except ValueError as exc:
        raise ToolError("path_outside_repo", f"Path is outside the repository: {project_path}") from exc
    if candidate.suffix.lower() != ".json":
        raise ToolError("unsupported_project_file", "URPG MCP project tools only accept JSON files.")
    return candidate


def _read_project_json(repo_root: Path, project_path: str) -> tuple[Path, dict[str, Any]]:
    resolved = _resolve_repo_file(repo_root, project_path)
    if not resolved.is_file():
        raise ToolError("project_file_missing", f"Project file does not exist: {project_path}")
    try:
        loaded = json.loads(resolved.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ToolError("project_json_invalid", str(exc)) from exc
    if not isinstance(loaded, dict):
        raise ToolError("project_json_invalid", "URPG project JSON root must be an object.")
    return resolved, loaded


def _read_json_file(repo_root: Path, json_path: str) -> tuple[Path, dict[str, Any]]:
    resolved = _resolve_repo_file(repo_root, json_path)
    if not resolved.is_file():
        raise ToolError("json_file_missing", f"JSON file does not exist: {json_path}")
    try:
        loaded = json.loads(resolved.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ToolError("json_invalid", str(exc)) from exc
    if not isinstance(loaded, dict):
        raise ToolError("json_invalid", "URPG MCP JSON root must be an object.")
    return resolved, loaded


def _project_summary(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    resolved, project = _read_project_json(repo_root, project_path)
    startup = project.get("startup", {})
    if not isinstance(startup, dict):
        startup = {}
    map_assets = startup.get("map_assets", {})
    asset_ids: list[str] = []
    if isinstance(map_assets, dict):
        for value in map_assets.values():
            if isinstance(value, dict) and isinstance(value.get("id"), str):
                asset_ids.append(value["id"])

    maps = project.get("maps", [])
    p2d = project.get("p2d", {})
    if not isinstance(p2d, dict):
        p2d = {}
    return {
        "project_path": str(resolved),
        "name": project.get("name", ""),
        "startup_map": startup.get("map", ""),
        "map_count": len(maps) if isinstance(maps, list) else 0,
        "asset_ids": sorted(asset_ids),
        "p2d_map_count": len(p2d.get("maps", [])) if isinstance(p2d.get("maps", []), list) else 0,
        "p2d_event_count": len(p2d.get("events", [])) if isinstance(p2d.get("events", []), list) else 0,
        "p2d_tileset_count": len(p2d.get("tilesets", [])) if isinstance(p2d.get("tilesets", []), list) else 0,
        "guardrails": ["read_only_summary", "bounded_repo_path"],
    }


def _project_validate(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    resolved, project = _read_project_json(repo_root, project_path)
    diagnostics: list[str] = []
    maps = project.get("maps", [])
    map_ids = {
        row.get("id")
        for row in maps
        if isinstance(maps, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
    }
    startup = project.get("startup", {})
    startup_map = startup.get("map") if isinstance(startup, dict) else None
    if startup_map and map_ids and startup_map not in map_ids:
        diagnostics.append("startup_map_missing")

    assets = project.get("assets", [])
    asset_ids = {
        row.get("id")
        for row in assets
        if isinstance(assets, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
    }
    if asset_ids and isinstance(startup, dict):
        map_assets = startup.get("map_assets", {})
        if isinstance(map_assets, dict):
            for map_asset in map_assets.values():
                if not isinstance(map_asset, dict):
                    continue
                asset_id = map_asset.get("id", "")
                if isinstance(asset_id, str) and asset_id and asset_id not in asset_ids:
                    diagnostics.append(f"startup_asset_missing:{asset_id}")

    p2d = project.get("p2d", {})
    if isinstance(p2d, dict):
        p2d_maps = p2d.get("maps", [])
        p2d_map_ids = {
            row.get("id")
            for row in p2d_maps
            if isinstance(p2d_maps, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
        }
        for p2d_map_id in sorted(p2d_map_ids):
            if map_ids and p2d_map_id not in map_ids:
                diagnostics.append(f"p2d_map_missing_project_map:{p2d_map_id}")
        p2d_events = p2d.get("events", [])
        if isinstance(p2d_events, list):
            for event in p2d_events:
                if not isinstance(event, dict):
                    diagnostics.append("p2d_event_invalid")
                    continue
                event_id = event.get("id", "")
                map_id = event.get("map_id", "")
                if not event_id:
                    diagnostics.append("p2d_event_missing_id")
                if map_id and p2d_map_ids and map_id not in p2d_map_ids:
                    diagnostics.append(f"p2d_event_map_missing:{event_id}")
    return {
        "project_path": str(resolved),
        "valid": not diagnostics,
        "diagnostics": diagnostics,
        "guardrails": ["read_only_validation", "bounded_repo_path"],
    }


def _ensure_object(parent: dict[str, Any], key: str) -> dict[str, Any]:
    value = parent.setdefault(key, {})
    if not isinstance(value, dict):
        parent[key] = {}
    return parent[key]


def _ensure_array(parent: dict[str, Any], key: str) -> list[Any]:
    value = parent.setdefault(key, [])
    if not isinstance(value, list):
        parent[key] = []
    return parent[key]


def _write_backup(resolved: Path, project: dict[str, Any]) -> Path:
    backup_path = resolved.with_suffix(resolved.suffix + ".urpg_mcp_backup")
    backup_path.write_text(json.dumps(project, indent=2) + "\n", encoding="utf-8")
    return backup_path


def _add_unique_record(records: list[Any], row: dict[str, str], path: str) -> list[dict[str, Any]]:
    record_id = row["id"]
    if any(isinstance(record, dict) and record.get("id") == record_id for record in records):
        return [{"op": "test", "path": path, "value": record_id}]
    records.append(row)
    return [{"op": "add", "path": f"{path}/-", "value": row}]


def _project_patch(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    patch_kind = str(arguments.get("patch_kind", ""))
    value = str(arguments.get("value", ""))
    apply_patch = bool(arguments.get("apply", False))
    resolved, project = _read_project_json(repo_root, project_path)
    if not value:
        raise ToolError("invalid_patch_value", f"{patch_kind} requires a non-empty value.")

    preview = json.loads(json.dumps(project))
    patch: list[dict[str, Any]]
    if patch_kind == "set_startup_map":
        startup = _ensure_object(preview, "startup")
        op = "replace" if "map" in startup else "add"
        startup["map"] = value
        patch = [{"op": op, "path": "/startup/map", "value": value}]
    elif patch_kind == "set_map_asset":
        key = str(arguments.get("key", ""))
        if not key:
            raise ToolError("invalid_patch_value", "set_map_asset requires key.")
        startup = _ensure_object(preview, "startup")
        map_assets = _ensure_object(startup, "map_assets")
        op = "replace" if key in map_assets else "add"
        map_assets[key] = {"id": value}
        patch = [{"op": op, "path": f"/startup/map_assets/{key}", "value": {"id": value}}]
    elif patch_kind == "add_p2d_map":
        p2d = _ensure_object(preview, "p2d")
        maps = _ensure_array(p2d, "maps")
        if not any(isinstance(row, dict) and row.get("id") == value for row in maps):
            maps.append({"id": value})
        patch = [{"op": "add", "path": "/p2d/maps/-", "value": {"id": value}}]
    elif patch_kind == "add_p2d_event":
        map_id = str(arguments.get("map_id", ""))
        label = str(arguments.get("label", value))
        if not map_id:
            raise ToolError("invalid_patch_value", "add_p2d_event requires map_id.")
        p2d = _ensure_object(preview, "p2d")
        events = _ensure_array(p2d, "events")
        row = {"id": value, "map_id": map_id, "label": label}
        if not any(isinstance(event, dict) and event.get("id") == value for event in events):
            events.append(row)
        patch = [{"op": "add", "path": "/p2d/events/-", "value": row}]
    elif patch_kind in DATABASE_PATCH_TARGETS:
        collection, path = DATABASE_PATCH_TARGETS[patch_kind]
        label = str(arguments.get("label", value))
        database = _ensure_object(preview, "database")
        records = _ensure_array(database, collection)
        patch = _add_unique_record(records, {"id": value, "name": label}, path)
    elif patch_kind == "add_asset_reference":
        label = str(arguments.get("label", value))
        asset_path = str(arguments.get("path", ""))
        assets = _ensure_array(preview, "assets")
        row = {"id": value, "name": label}
        if asset_path:
            row["path"] = asset_path
        patch = _add_unique_record(assets, row, "/assets")
    else:
        raise ToolError("unknown_patch_kind", f"Unknown allowlisted patch kind: {patch_kind}")

    backup_path = ""
    if apply_patch:
        backup_path = str(_write_backup(resolved, project))
        resolved.write_text(json.dumps(preview, indent=2) + "\n", encoding="utf-8")
    result = {
        "project_path": str(resolved),
        "patch_kind": patch_kind,
        "patch": patch,
        "preview": preview,
        "applied": apply_patch,
        "guardrails": ["allowlisted_patch_kind", "explicit_apply_required", "bounded_repo_path"],
    }
    if backup_path:
        result["backup_path"] = backup_path
    return result


def _record_field(record: dict[str, Any], *names: str) -> str:
    for name in names:
        value = record.get(name)
        if isinstance(value, str) and value:
            return value
    return ""


def _asset_catalog_summary(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    catalog_path = str(arguments.get("catalog_path", "asset_catalog.json"))
    resolved, catalog = _read_json_file(repo_root, catalog_path)
    raw_records = catalog.get("records", catalog.get("assets", []))
    records = raw_records if isinstance(raw_records, list) else []
    media_kind_counts: dict[str, int] = {}
    license_status_counts: dict[str, int] = {}
    asset_ids: list[str] = []
    release_ready_count = 0
    release_ready_license_statuses = {"cc0", "cleared", "verified"}

    for raw_record in records:
        if not isinstance(raw_record, dict):
            continue
        asset_id = _record_field(raw_record, "id", "asset_id")
        if asset_id:
            asset_ids.append(asset_id)
        media_kind = _record_field(raw_record, "mediaKind", "media_kind") or "unknown"
        license_status = _record_field(raw_record, "licenseStatus", "license_status") or "unknown"
        media_kind_counts[media_kind] = media_kind_counts.get(media_kind, 0) + 1
        license_status_counts[license_status] = license_status_counts.get(license_status, 0) + 1
        release_ready = raw_record.get("releaseReady", raw_record.get("release_ready", False))
        if release_ready is True or license_status.lower() in release_ready_license_statuses:
            release_ready_count += 1

    return {
        "catalog_path": str(resolved),
        "asset_count": len([record for record in records if isinstance(record, dict)]),
        "media_kind_counts": dict(sorted(media_kind_counts.items())),
        "license_status_counts": dict(sorted(license_status_counts.items())),
        "release_ready_count": release_ready_count,
        "asset_ids": sorted(asset_ids),
        "guardrails": ["read_only_summary", "bounded_repo_path", "no_license_bypass"],
    }


def _p2d_capabilities() -> dict[str, Any]:
    return {
        "surface": "SpatialAuthoringWorkspace",
        "tile_system": [
            "tileset_pages_A_Z",
            "autotiles",
            "animated_tiles",
            "passage_flags",
            "collision_flags",
            "terrain_tags",
            "region_ids",
            "priority_star_passability",
            "tile_previews",
        ],
        "event_runtime": [
            "show_text",
            "transfer_player",
            "change_switch",
            "change_variable",
            "change_self_switch",
            "change_gold",
            "change_item",
            "move_route",
            "call_common_event",
            "conditional_branch",
        ],
        "project_database": [
            "actors",
            "items",
            "switches",
            "variables",
            "common_events",
            "maps",
            "starting_party",
            "transfers",
            "encounters",
            "assets",
            "save_load",
        ],
        "focused_gates": sorted(FOCUSED_GATES),
    }


def _focused_gate(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    gate_id = str(arguments.get("gate_id", ""))
    if gate_id not in FOCUSED_GATES:
        raise ToolError("unknown_gate_id", f"Unknown focused gate id: {gate_id}")
    command = FOCUSED_GATES[gate_id]
    run = bool(arguments.get("run", False))
    timeout_seconds = int(arguments.get("timeout_seconds", 120))
    result: dict[str, Any] = {
        "gate_id": gate_id,
        "command": command,
        "ran": False,
        "guardrail": "Command is allowlisted; execution requires run=true.",
    }
    if not run:
        return result

    completed = subprocess.run(
        command,
        cwd=repo_root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout_seconds,
    )
    result.update(
        {
            "ran": True,
            "returncode": completed.returncode,
            "stdout": completed.stdout,
            "stderr": completed.stderr,
        }
    )
    return result


def call_tool(name: str, arguments: dict[str, Any] | None = None, repo_root: Path | None = None) -> dict[str, Any]:
    args = arguments or {}
    root = repo_root or Path.cwd()
    if name == "urpg.project_status":
        return _project_status(root)
    if name == "urpg.project_summary":
        return _project_summary(args, root)
    if name == "urpg.project_validate":
        return _project_validate(args, root)
    if name == "urpg.project_patch":
        return _project_patch(args, root)
    if name == "urpg.asset_catalog_summary":
        return _asset_catalog_summary(args, root)
    if name == "urpg.p2d_capabilities":
        return _p2d_capabilities()
    if name == "urpg.focused_gate":
        return _focused_gate(args, root)
    if name == "urpg.release_guardrails":
        return {"guardrails": GUARDRAILS}
    raise ToolError("unknown_tool", f"Unknown URPG MCP tool: {name}")


def _tool_content(payload: dict[str, Any]) -> dict[str, Any]:
    return {"content": [{"type": "text", "text": json.dumps(payload, indent=2)}]}


def handle_json_rpc(request: dict[str, Any], repo_root: Path | None = None) -> dict[str, Any]:
    request_id = request.get("id")
    method = request.get("method")
    try:
        if method == "initialize":
            result = {
                "protocolVersion": "2024-11-05",
                "serverInfo": {"name": SERVER_NAME, "version": SERVER_VERSION},
                "capabilities": {"tools": {}},
            }
        elif method == "tools/list":
            result = {"tools": list_tools()}
        elif method == "tools/call":
            params = request.get("params", {})
            name = params.get("name", "")
            arguments = params.get("arguments", {})
            result = _tool_content(call_tool(name, arguments, repo_root))
        else:
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "error": {"code": -32601, "message": f"Unsupported method: {method}"},
            }
        return {"jsonrpc": "2.0", "id": request_id, "result": result}
    except ToolError as exc:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "error": {"code": -32000, "message": exc.message, "data": {"code": exc.code}},
        }


def serve_stdio(repo_root: Path | None = None) -> int:
    root = repo_root or Path.cwd()
    for line in sys.stdin:
        if not line.strip():
            continue
        try:
            request = json.loads(line)
            response = handle_json_rpc(request, root)
        except json.JSONDecodeError as exc:
            response = {
                "jsonrpc": "2.0",
                "id": None,
                "error": {"code": -32700, "message": str(exc)},
            }
        sys.stdout.write(json.dumps(response) + "\n")
        sys.stdout.flush()
    return 0


def main() -> int:
    return serve_stdio(Path.cwd())


if __name__ == "__main__":
    raise SystemExit(main())
