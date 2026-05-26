#!/usr/bin/env python3
from __future__ import annotations

import json
import hashlib
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


SERVER_NAME = "urpg-local-mcp"
SERVER_VERSION = "0.2.0"
PROTOCOL_VERSION = "2024-11-05"
PROJECT_SCHEMA_VERSION = "urpg.project.v1"

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
    "add_p2d_tileset",
    "set_p2d_tile_metadata",
    "add_p2d_event_command",
    "add_actor",
    "add_item",
    "add_switch",
    "add_variable",
    "add_common_event",
    "add_asset_reference",
    "add_starting_party_actor",
    "add_transfer",
    "add_encounter",
    "add_save_profile",
]

DATABASE_PATCH_TARGETS = {
    "add_actor": ("actors", "/database/actors"),
    "add_item": ("items", "/database/items"),
    "add_switch": ("switches", "/database/switches"),
    "add_variable": ("variables", "/database/variables"),
    "add_common_event": ("common_events", "/database/common_events"),
}

P2D_EVENT_COMMAND_TYPES = {
    "call_common_event",
    "change_gold",
    "change_item",
    "change_self_switch",
    "change_switch",
    "change_variable",
    "conditional_branch",
    "move_route",
    "show_text",
    "transfer_player",
}

P2D_COMMAND_REQUIRED_FIELDS: dict[str, list[str]] = {
    "call_common_event": ["common_event_id"],
    "change_gold": ["operation", "amount"],
    "change_item": ["item_id", "operation", "amount"],
    "change_self_switch": ["self_switch", "state"],
    "change_switch": ["switch_id", "state"],
    "change_variable": ["variable_id", "operation", "amount"],
    "conditional_branch": ["condition"],
    "move_route": ["route"],
    "show_text": [],
    "transfer_player": ["map_id", "x", "y"],
}

VALID_P2D_PAGES = {chr(ord("A") + index) for index in range(26)}
VALID_PASSABILITY = {"passable", "blocked", "star"}
VALID_COLLISION = {"none", "blocked", "platform", "water", "hazard"}


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


def _project_path_schema() -> dict[str, Any]:
    return {"project_path": {"type": "string", "default": "project.json"}}


def list_tools() -> list[dict[str, Any]]:
    return [
        {
            "name": "urpg.mcp_manifest",
            "description": "Return server, protocol, project schema, tool checksum, and guardrails.",
            "inputSchema": _object_schema(),
        },
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
            "name": "urpg.project_list_maps",
            "description": "Read bounded project JSON and list project maps without writing.",
            "inputSchema": _object_schema(_project_path_schema()),
        },
        {
            "name": "urpg.project_list_events",
            "description": "Read bounded project JSON and list P2D events without writing.",
            "inputSchema": _object_schema(_project_path_schema()),
        },
        {
            "name": "urpg.project_list_assets",
            "description": "Read bounded project JSON and list project asset references without writing.",
            "inputSchema": _object_schema(_project_path_schema()),
        },
        {
            "name": "urpg.project_list_database",
            "description": "Read bounded project JSON and list supported database collections without writing.",
            "inputSchema": _object_schema(_project_path_schema()),
        },
        {
            "name": "urpg.p2d_map_summary",
            "description": "Read bounded project JSON and summarize one P2D map without writing.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "map_id": {"type": "string"},
                },
                ["map_id"],
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
                    "event_id": {"type": "string"},
                    "tileset_id": {"type": "string"},
                    "asset_id": {"type": "string"},
                    "from_map": {"type": "string"},
                    "to_map": {"type": "string"},
                    "enemy_id": {"type": "string"},
                    "switch_id": {"type": "string"},
                    "variable_id": {"type": "string"},
                    "self_switch": {"type": "string"},
                    "item_id": {"type": "string"},
                    "common_event_id": {"type": "string"},
                    "page": {"type": "string"},
                    "passability": {"type": "string"},
                    "collision": {"type": "string"},
                    "terrain_tag": {"type": "string"},
                    "region_id": {"type": "string"},
                    "priority": {"type": "string"},
                    "x": {"type": "string"},
                    "y": {"type": "string"},
                    "amount": {"type": "string"},
                    "weight": {"type": "string"},
                    "slot": {"type": "string"},
                    "operation": {"type": "string"},
                    "label": {"type": "string"},
                    "path": {"type": "string"},
                    "text": {"type": "string"},
                    "route": {"type": "string"},
                    "condition": {"type": "string"},
                    "state": {"type": "boolean"},
                    "animated": {"type": "boolean"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["patch_kind", "value"],
            ),
        },
        {
            "name": "urpg.project_set_startup",
            "description": "Preview or apply a startup map change through the allowlisted project patcher.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "map_id": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["map_id"],
            ),
        },
        {
            "name": "urpg.p2d_add_map",
            "description": "Preview or apply a P2D map insertion through the allowlisted project patcher.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "map_id": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["map_id"],
            ),
        },
        {
            "name": "urpg.p2d_add_event",
            "description": "Preview or apply a P2D event insertion through the allowlisted project patcher.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "event_id": {"type": "string"},
                    "map_id": {"type": "string"},
                    "label": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["event_id", "map_id"],
            ),
        },
        {
            "name": "urpg.p2d_add_event_command",
            "description": "Preview or apply a typed P2D event command through the allowlisted project patcher.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "event_id": {"type": "string"},
                    "command_type": {"type": "string", "enum": sorted(P2D_EVENT_COMMAND_TYPES)},
                    "map_id": {"type": "string"},
                    "switch_id": {"type": "string"},
                    "variable_id": {"type": "string"},
                    "self_switch": {"type": "string"},
                    "item_id": {"type": "string"},
                    "common_event_id": {"type": "string"},
                    "x": {"type": "string"},
                    "y": {"type": "string"},
                    "amount": {"type": "string"},
                    "operation": {"type": "string"},
                    "text": {"type": "string"},
                    "route": {"type": "string"},
                    "condition": {"type": "string"},
                    "state": {"type": "boolean"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["event_id", "command_type"],
            ),
        },
        {
            "name": "urpg.database_add_record",
            "description": "Preview or apply an actor/item/switch/variable/common-event database insertion.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "collection": {
                        "type": "string",
                        "enum": ["actors", "items", "switches", "variables", "common_events"],
                    },
                    "record_id": {"type": "string"},
                    "label": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["collection", "record_id"],
            ),
        },
        {
            "name": "urpg.project_add_asset_reference",
            "description": "Preview or apply an asset reference insertion through the allowlisted project patcher.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "asset_id": {"type": "string"},
                    "label": {"type": "string"},
                    "path": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["asset_id"],
            ),
        },
        {
            "name": "urpg.playable_add_transfer",
            "description": "Preview or apply a map transfer insertion through the allowlisted project patcher.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "transfer_id": {"type": "string"},
                    "from_map": {"type": "string"},
                    "to_map": {"type": "string"},
                    "x": {"type": "string"},
                    "y": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["transfer_id", "from_map", "to_map"],
            ),
        },
        {
            "name": "urpg.project_restore_backup",
            "description": "Preview or explicitly restore a bounded project JSON backup.",
            "inputSchema": _object_schema(
                {
                    **_project_path_schema(),
                    "backup_path": {"type": "string"},
                    "apply": {"type": "boolean", "default": False},
                },
                ["backup_path"],
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
            "name": "urpg.gate_status",
            "description": "Report whether an allowlisted focused gate appears runnable without running it.",
            "inputSchema": _object_schema(
                {"gate_id": {"type": "string", "enum": sorted(FOCUSED_GATES)}},
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


def _tool_list_checksum(tools: list[dict[str, Any]]) -> str:
    names = sorted(tool["name"] for tool in tools)
    return hashlib.sha256("\n".join(names).encode("utf-8")).hexdigest()[:16]


def _mcp_manifest(repo_root: Path) -> dict[str, Any]:
    tools = list_tools()
    return {
        "server_name": SERVER_NAME,
        "server_version": SERVER_VERSION,
        "protocol_version": PROTOCOL_VERSION,
        "project_schema_version": PROJECT_SCHEMA_VERSION,
        "repo_root": str(repo_root),
        "tools": sorted(tool["name"] for tool in tools),
        "tool_list_checksum": _tool_list_checksum(tools),
        "guardrails": GUARDRAILS,
    }


def _is_int_like(value: Any) -> bool:
    if isinstance(value, bool):
        return False
    if isinstance(value, int):
        return True
    if isinstance(value, str):
        try:
            int(value)
            return True
        except ValueError:
            return False
    return False


def _duplicate_id_diagnostics(records: Any, prefix: str) -> list[str]:
    if not isinstance(records, list):
        return [f"{prefix}_collection_invalid"]
    seen: set[str] = set()
    diagnostics: list[str] = []
    for record in records:
        if not isinstance(record, dict):
            diagnostics.append(f"{prefix}_record_invalid")
            continue
        record_id = record.get("id")
        if not isinstance(record_id, str) or not record_id:
            diagnostics.append(f"{prefix}_id_missing")
            continue
        if record_id in seen:
            diagnostics.append(f"duplicate_{prefix}_id:{record_id}")
        seen.add(record_id)
    return diagnostics


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
    diagnostics.extend(_duplicate_id_diagnostics(maps, "map"))
    map_ids = {
        row.get("id")
        for row in maps
        if isinstance(maps, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
    }
    database = project.get("database", {})
    if not isinstance(database, dict):
        database = {}
    actors = database.get("actors", [])
    actor_ids = {
        row.get("id")
        for row in actors
        if isinstance(actors, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
    }
    items = database.get("items", [])
    item_ids = {
        row.get("id")
        for row in items
        if isinstance(items, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
    }
    startup = project.get("startup", {})
    startup_map = startup.get("map") if isinstance(startup, dict) else None
    if startup_map and map_ids and startup_map not in map_ids:
        diagnostics.append("startup_map_missing")
    if isinstance(startup, dict) and actor_ids:
        starting_party = startup.get("starting_party", [])
        if isinstance(starting_party, list):
            for actor in starting_party:
                if not isinstance(actor, dict):
                    diagnostics.append("starting_party_actor_invalid")
                    continue
                actor_id = actor.get("id", "")
                if isinstance(actor_id, str) and actor_id and actor_id not in actor_ids:
                    diagnostics.append(f"starting_party_actor_missing:{actor_id}")

    assets = project.get("assets", [])
    diagnostics.extend(_duplicate_id_diagnostics(assets, "asset") if isinstance(assets, list) else ["asset_collection_invalid"])
    asset_ids = {
        row.get("id")
        for row in assets
        if isinstance(assets, list) and isinstance(row, dict) and isinstance(row.get("id"), str)
    }
    if isinstance(assets, list):
        for asset in assets:
            if not isinstance(asset, dict):
                continue
            asset_id = asset.get("id", "")
            asset_path = asset.get("path", "")
            if isinstance(asset_id, str) and isinstance(asset_path, str) and asset_path:
                if not (repo_root / asset_path).is_file():
                    diagnostics.append(f"asset_path_missing:{asset_id}:{asset_path}")
    if asset_ids and isinstance(startup, dict):
        map_assets = startup.get("map_assets", {})
        if isinstance(map_assets, dict):
            for map_asset in map_assets.values():
                if not isinstance(map_asset, dict):
                    continue
                asset_id = map_asset.get("id", "")
                if isinstance(asset_id, str) and asset_id and asset_id not in asset_ids:
                    diagnostics.append(f"startup_asset_missing:{asset_id}")

    transfers = project.get("transfers", [])
    if isinstance(transfers, list):
        for transfer in transfers:
            if not isinstance(transfer, dict):
                diagnostics.append("transfer_invalid")
                continue
            transfer_id = transfer.get("id", "")
            from_map = transfer.get("from_map", "")
            to_map = transfer.get("to_map", "")
            if isinstance(from_map, str) and from_map and map_ids and from_map not in map_ids:
                diagnostics.append(f"transfer_from_map_missing:{transfer_id}:{from_map}")
            if isinstance(to_map, str) and to_map and map_ids and to_map not in map_ids:
                diagnostics.append(f"transfer_to_map_missing:{transfer_id}:{to_map}")
            for coordinate in ("x", "y"):
                if coordinate in transfer and not _is_int_like(transfer.get(coordinate)):
                    diagnostics.append(f"transfer_{coordinate}_invalid:{transfer_id}")

    encounters = project.get("encounters", [])
    if isinstance(encounters, list):
        for encounter in encounters:
            if not isinstance(encounter, dict):
                diagnostics.append("encounter_invalid")
                continue
            encounter_id = encounter.get("id", "")
            map_id = encounter.get("map_id", "")
            enemy_id = encounter.get("enemy_id", "")
            if isinstance(map_id, str) and map_id and map_ids and map_id not in map_ids:
                diagnostics.append(f"encounter_map_missing:{encounter_id}:{map_id}")
            if isinstance(enemy_id, str) and enemy_id and item_ids and enemy_id not in item_ids:
                diagnostics.append(f"encounter_enemy_missing:{encounter_id}:{enemy_id}")

    p2d = project.get("p2d", {})
    if isinstance(p2d, dict):
        p2d_maps = p2d.get("maps", [])
        diagnostics.extend(_duplicate_id_diagnostics(p2d_maps, "p2d_map"))
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
                commands = event.get("commands", [])
                if isinstance(commands, list):
                    for command in commands:
                        if not isinstance(command, dict):
                            diagnostics.append(f"p2d_event_command_invalid:{event_id}")
                            continue
                        command_type = command.get("type", "")
                        if not isinstance(command_type, str) or command_type not in P2D_EVENT_COMMAND_TYPES:
                            diagnostics.append(f"p2d_event_command_unsupported:{event_id}:{command_type}")
                            continue
                        for field in P2D_COMMAND_REQUIRED_FIELDS[command_type]:
                            if field not in command or command.get(field) in ("", None):
                                diagnostics.append(
                                    f"p2d_event_command_missing_field:{event_id}:{command_type}:{field}"
                                )
                            elif field in {"amount", "x", "y"} and not _is_int_like(command.get(field)):
                                diagnostics.append(
                                    f"p2d_event_command_field_invalid:{event_id}:{command_type}:{field}"
                                )
        p2d_tilesets = p2d.get("tilesets", [])
        diagnostics.extend(_duplicate_id_diagnostics(p2d_tilesets, "p2d_tileset"))
        if isinstance(p2d_tilesets, list):
            for tileset in p2d_tilesets:
                if not isinstance(tileset, dict):
                    continue
                tileset_id = str(tileset.get("id", ""))
                pages = tileset.get("pages", [])
                if isinstance(pages, list):
                    for page in pages:
                        if not isinstance(page, str) or page not in VALID_P2D_PAGES:
                            diagnostics.append(f"p2d_tileset_page_invalid:{tileset_id}:{page}")
                tiles = tileset.get("tiles", [])
                diagnostics.extend(_duplicate_id_diagnostics(tiles, "p2d_tile") if isinstance(tiles, list) else [])
                if isinstance(tiles, list):
                    for tile in tiles:
                        if not isinstance(tile, dict):
                            continue
                        tile_id = str(tile.get("id", ""))
                        page = tile.get("page")
                        if isinstance(page, str) and page not in VALID_P2D_PAGES:
                            diagnostics.append(f"p2d_tile_page_invalid:{tileset_id}:{tile_id}:{page}")
                        passability = tile.get("passability")
                        if isinstance(passability, str) and passability not in VALID_PASSABILITY:
                            diagnostics.append(
                                f"p2d_tile_passability_invalid:{tileset_id}:{tile_id}:{passability}"
                            )
                        collision = tile.get("collision")
                        if isinstance(collision, str) and collision not in VALID_COLLISION:
                            diagnostics.append(f"p2d_tile_collision_invalid:{tileset_id}:{tile_id}:{collision}")
                        priority = tile.get("priority")
                        if priority is not None and (not isinstance(priority, int) or priority < 0 or priority > 5):
                            diagnostics.append(f"p2d_tile_priority_invalid:{tileset_id}:{tile_id}:{priority}")
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


def _backup_manifest_path(resolved: Path) -> Path:
    return resolved.with_suffix(resolved.suffix + ".urpg_mcp_backups.json")


def _write_backup(resolved: Path, project: dict[str, Any], patch_kind: str = "unknown") -> Path:
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    backup_path = resolved.with_name(f"{resolved.name}.urpg_mcp_backup.{timestamp}.json")
    backup_path.write_text(json.dumps(project, indent=2) + "\n", encoding="utf-8")
    manifest_path = _backup_manifest_path(resolved)
    manifest: dict[str, Any] = {"backups": []}
    if manifest_path.is_file():
        try:
            loaded = json.loads(manifest_path.read_text(encoding="utf-8"))
            if isinstance(loaded, dict) and isinstance(loaded.get("backups"), list):
                manifest = loaded
        except json.JSONDecodeError:
            manifest = {"backups": []}
    manifest["backups"].append(
        {
            "backup_path": str(backup_path),
            "project_path": str(resolved),
            "patch_kind": patch_kind,
            "created_utc": timestamp,
        }
    )
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return backup_path


def _add_unique_record(records: list[Any], row: dict[str, str], path: str) -> list[dict[str, Any]]:
    record_id = row["id"]
    if any(isinstance(record, dict) and record.get("id") == record_id for record in records):
        return [{"op": "test", "path": path, "value": record_id}]
    records.append(row)
    return [{"op": "add", "path": f"{path}/-", "value": row}]


def _changed_sections(before: dict[str, Any], after: dict[str, Any]) -> list[str]:
    keys = sorted(set(before) | set(after))
    return [key for key in keys if before.get(key) != after.get(key)]


def _changed_subtrees(before: dict[str, Any], after: dict[str, Any], sections: list[str]) -> dict[str, Any]:
    return {section: {"before": before.get(section), "after": after.get(section)} for section in sections}


def _patch_summary(patch_kind: str, patch: list[dict[str, Any]]) -> str:
    if not patch:
        return f"{patch_kind}: no changes"
    first = patch[0]
    return f"{patch_kind}: {first.get('path', '')} -> {first.get('value')}"


def _to_optional_int(arguments: dict[str, Any], key: str) -> int | None:
    value = arguments.get(key)
    if value is None or value == "":
        return None
    try:
        return int(value)
    except (TypeError, ValueError) as exc:
        raise ToolError("invalid_patch_value", f"{key} must be an integer.") from exc


def _require_int(arguments: dict[str, Any], key: str) -> int:
    value = _to_optional_int(arguments, key)
    if value is None:
        raise ToolError("invalid_patch_value", f"{key} is required.")
    return value


def _require_string(arguments: dict[str, Any], key: str) -> str:
    value = str(arguments.get(key, ""))
    if not value:
        raise ToolError("invalid_patch_value", f"{key} is required.")
    return value


def _require_bool(arguments: dict[str, Any], key: str) -> bool:
    if key not in arguments:
        raise ToolError("invalid_patch_value", f"{key} is required.")
    return bool(arguments.get(key, False))


def _find_record(records: list[Any], record_id: str) -> dict[str, Any] | None:
    for record in records:
        if isinstance(record, dict) and record.get("id") == record_id:
            return record
    return None


def _build_p2d_event_command(command_type: str, arguments: dict[str, Any]) -> dict[str, Any]:
    command: dict[str, Any] = {"type": command_type}
    if command_type == "show_text":
        text = str(arguments.get("text", ""))
        if text:
            command["text"] = text
    elif command_type == "transfer_player":
        command["map_id"] = _require_string(arguments, "map_id")
        command["x"] = _require_int(arguments, "x")
        command["y"] = _require_int(arguments, "y")
    elif command_type == "change_switch":
        command["switch_id"] = _require_string(arguments, "switch_id")
        command["state"] = _require_bool(arguments, "state")
    elif command_type == "change_variable":
        command["variable_id"] = _require_string(arguments, "variable_id")
        command["operation"] = _require_string(arguments, "operation")
        command["amount"] = _require_int(arguments, "amount")
    elif command_type == "change_self_switch":
        command["self_switch"] = _require_string(arguments, "self_switch")
        command["state"] = _require_bool(arguments, "state")
    elif command_type == "change_gold":
        command["operation"] = _require_string(arguments, "operation")
        command["amount"] = _require_int(arguments, "amount")
    elif command_type == "change_item":
        command["item_id"] = _require_string(arguments, "item_id")
        command["operation"] = _require_string(arguments, "operation")
        command["amount"] = _require_int(arguments, "amount")
    elif command_type == "call_common_event":
        command["common_event_id"] = _require_string(arguments, "common_event_id")
    elif command_type == "move_route":
        command["route"] = _require_string(arguments, "route")
    elif command_type == "conditional_branch":
        command["condition"] = _require_string(arguments, "condition")
    else:
        raise ToolError("unsupported_p2d_event_command", f"Unsupported P2D event command: {command_type}")
    return command


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
    elif patch_kind == "add_p2d_tileset":
        p2d = _ensure_object(preview, "p2d")
        tilesets = _ensure_array(p2d, "tilesets")
        label = str(arguments.get("label", value))
        asset_id = str(arguments.get("asset_id", ""))
        page = str(arguments.get("page", "A"))
        row = {"id": value, "name": label}
        if asset_id:
            row["asset_id"] = asset_id
        row["pages"] = [page]
        row["tiles"] = []
        patch = _add_unique_record(tilesets, row, "/p2d/tilesets")
    elif patch_kind == "set_p2d_tile_metadata":
        tileset_id = str(arguments.get("tileset_id", ""))
        if not tileset_id:
            raise ToolError("invalid_patch_value", "set_p2d_tile_metadata requires tileset_id.")
        p2d = _ensure_object(preview, "p2d")
        tilesets = _ensure_array(p2d, "tilesets")
        tileset = _find_record(tilesets, tileset_id)
        if tileset is None:
            raise ToolError("p2d_tileset_missing", f"P2D tileset does not exist: {tileset_id}")
        page = str(arguments.get("page", "A"))
        pages = _ensure_array(tileset, "pages")
        if page not in pages:
            pages.append(page)
        tiles = _ensure_array(tileset, "tiles")
        tile = _find_record(tiles, value)
        op = "replace" if tile is not None else "add"
        if tile is None:
            tile = {"id": value}
            tiles.append(tile)
        tile["page"] = page
        for key in ("passability", "collision", "terrain_tag"):
            field_value = str(arguments.get(key, ""))
            if field_value:
                tile[key] = field_value
        for key in ("region_id", "priority"):
            field_value = _to_optional_int(arguments, key)
            if field_value is not None:
                tile[key] = field_value
        if "animated" in arguments:
            tile["animated"] = bool(arguments.get("animated", False))
        patch = [{"op": op, "path": f"/p2d/tilesets/{tileset_id}/tiles/{value}", "value": tile}]
    elif patch_kind == "add_p2d_event_command":
        event_id = str(arguments.get("event_id", ""))
        if not event_id:
            raise ToolError("invalid_patch_value", "add_p2d_event_command requires event_id.")
        if value not in P2D_EVENT_COMMAND_TYPES:
            raise ToolError("unsupported_p2d_event_command", f"Unsupported P2D event command: {value}")
        p2d = _ensure_object(preview, "p2d")
        events = _ensure_array(p2d, "events")
        event = _find_record(events, event_id)
        if event is None:
            raise ToolError("p2d_event_missing", f"P2D event does not exist: {event_id}")
        commands = _ensure_array(event, "commands")
        command = _build_p2d_event_command(value, arguments)
        commands.append(command)
        patch = [{"op": "add", "path": f"/p2d/events/{event_id}/commands/-", "value": command}]
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
    elif patch_kind == "add_starting_party_actor":
        startup = _ensure_object(preview, "startup")
        starting_party = _ensure_array(startup, "starting_party")
        patch = _add_unique_record(starting_party, {"id": value}, "/startup/starting_party")
    elif patch_kind == "add_transfer":
        from_map = str(arguments.get("from_map", ""))
        to_map = str(arguments.get("to_map", ""))
        if not from_map or not to_map:
            raise ToolError("invalid_patch_value", "add_transfer requires from_map and to_map.")
        x = _to_optional_int(arguments, "x")
        y = _to_optional_int(arguments, "y")
        transfers = _ensure_array(preview, "transfers")
        row = {"id": value, "from_map": from_map, "to_map": to_map}
        if x is not None:
            row["x"] = x
        if y is not None:
            row["y"] = y
        patch = _add_unique_record(transfers, row, "/transfers")
    elif patch_kind == "add_encounter":
        map_id = str(arguments.get("map_id", ""))
        enemy_id = str(arguments.get("enemy_id", ""))
        if not map_id or not enemy_id:
            raise ToolError("invalid_patch_value", "add_encounter requires map_id and enemy_id.")
        weight = _to_optional_int(arguments, "weight")
        encounters = _ensure_array(preview, "encounters")
        row = {"id": value, "map_id": map_id, "enemy_id": enemy_id}
        if weight is not None:
            row["weight"] = weight
        patch = _add_unique_record(encounters, row, "/encounters")
    elif patch_kind == "add_save_profile":
        label = str(arguments.get("label", value))
        slot = _to_optional_int(arguments, "slot")
        save_profiles = _ensure_array(preview, "save_profiles")
        row = {"id": value, "name": label}
        if slot is not None:
            row["slot"] = slot
        patch = _add_unique_record(save_profiles, row, "/save_profiles")
    else:
        raise ToolError("unknown_patch_kind", f"Unknown allowlisted patch kind: {patch_kind}")

    sections = _changed_sections(project, preview)
    backup_path = ""
    if apply_patch:
        backup_path = str(_write_backup(resolved, project, patch_kind))
        resolved.write_text(json.dumps(preview, indent=2) + "\n", encoding="utf-8")
    result = {
        "project_path": str(resolved),
        "patch_kind": patch_kind,
        "patch": patch,
        "preview": preview,
        "summary": _patch_summary(patch_kind, patch),
        "changed_sections": sections,
        "changed_subtrees": _changed_subtrees(project, preview, sections),
        "applied": apply_patch,
        "guardrails": ["allowlisted_patch_kind", "explicit_apply_required", "bounded_repo_path"],
    }
    if backup_path:
        result["backup_path"] = backup_path
    return result


def _project_restore_backup(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    backup_path = _require_string(arguments, "backup_path")
    apply_restore = bool(arguments.get("apply", False))
    resolved, project = _read_project_json(repo_root, project_path)
    backup_resolved, backup = _read_json_file(repo_root, backup_path)
    sections = _changed_sections(project, backup)
    restore_patch = [
        {"op": "replace", "path": f"/{section}", "value": backup.get(section)}
        for section in sections
        if section in backup
    ]
    restore_patch.extend(
        {"op": "remove", "path": f"/{section}"}
        for section in sections
        if section not in backup
    )
    result: dict[str, Any] = {
        "project_path": str(resolved),
        "backup_path": str(backup_resolved),
        "patch_kind": "restore_backup",
        "patch": restore_patch,
        "preview": backup,
        "summary": f"restore_backup: {project_path}",
        "changed_sections": sections,
        "changed_subtrees": _changed_subtrees(project, backup, sections),
        "applied": apply_restore,
        "guardrails": ["bounded_repo_path", "explicit_apply_required", "backup_restore_only"],
    }
    if apply_restore:
        result["pre_restore_backup_path"] = str(_write_backup(resolved, project, "restore_backup"))
        resolved.write_text(json.dumps(backup, indent=2) + "\n", encoding="utf-8")
    return result


def _project_list_maps(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    resolved, project = _read_project_json(repo_root, project_path)
    maps = project.get("maps", [])
    return {
        "project_path": str(resolved),
        "maps": [row for row in maps if isinstance(row, dict)] if isinstance(maps, list) else [],
        "guardrails": ["read_only_summary", "bounded_repo_path"],
    }


def _project_list_events(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    resolved, project = _read_project_json(repo_root, project_path)
    p2d = project.get("p2d", {})
    events = p2d.get("events", []) if isinstance(p2d, dict) else []
    return {
        "project_path": str(resolved),
        "events": [row for row in events if isinstance(row, dict)] if isinstance(events, list) else [],
        "guardrails": ["read_only_summary", "bounded_repo_path"],
    }


def _project_list_assets(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    resolved, project = _read_project_json(repo_root, project_path)
    assets = project.get("assets", [])
    return {
        "project_path": str(resolved),
        "assets": [row for row in assets if isinstance(row, dict)] if isinstance(assets, list) else [],
        "guardrails": ["read_only_summary", "bounded_repo_path"],
    }


def _project_list_database(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    resolved, project = _read_project_json(repo_root, project_path)
    database = project.get("database", {})
    collections: dict[str, list[Any]] = {}
    if isinstance(database, dict):
        for collection in ("actors", "items", "switches", "variables", "common_events"):
            rows = database.get(collection, [])
            collections[collection] = [row for row in rows if isinstance(row, dict)] if isinstance(rows, list) else []
    return {
        "project_path": str(resolved),
        "collections": collections,
        "guardrails": ["read_only_summary", "bounded_repo_path"],
    }


def _p2d_map_summary(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    project_path = str(arguments.get("project_path", "project.json"))
    map_id = _require_string(arguments, "map_id")
    resolved, project = _read_project_json(repo_root, project_path)
    p2d = project.get("p2d", {})
    maps = p2d.get("maps", []) if isinstance(p2d, dict) else []
    events = p2d.get("events", []) if isinstance(p2d, dict) else []
    p2d_map = _find_record(maps, map_id) if isinstance(maps, list) else None
    if p2d_map is None:
        raise ToolError("p2d_map_missing", f"P2D map does not exist: {map_id}")
    map_events = [
        event
        for event in events
        if isinstance(events, list) and isinstance(event, dict) and event.get("map_id") == map_id
    ]
    return {
        "project_path": str(resolved),
        "map": p2d_map,
        "map_id": map_id,
        "tileset_id": p2d_map.get("tileset_id", ""),
        "event_count": len(map_events),
        "events": map_events,
        "guardrails": ["read_only_summary", "bounded_repo_path"],
    }


def _narrow_patch(arguments: dict[str, Any], repo_root: Path, patch_kind: str, value_key: str) -> dict[str, Any]:
    value = _require_string(arguments, value_key)
    return _project_patch({**arguments, "patch_kind": patch_kind, "value": value}, repo_root)


def _database_add_record(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    collection = _require_string(arguments, "collection")
    record_id = _require_string(arguments, "record_id")
    patch_kinds = {
        "actors": "add_actor",
        "items": "add_item",
        "switches": "add_switch",
        "variables": "add_variable",
        "common_events": "add_common_event",
    }
    if collection not in patch_kinds:
        raise ToolError("unknown_database_collection", f"Unknown database collection: {collection}")
    return _project_patch({**arguments, "patch_kind": patch_kinds[collection], "value": record_id}, repo_root)


def _p2d_add_event_command(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    command_type = _require_string(arguments, "command_type")
    return _project_patch({**arguments, "patch_kind": "add_p2d_event_command", "value": command_type}, repo_root)


def _gate_status(arguments: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    gate_id = _require_string(arguments, "gate_id")
    if gate_id not in FOCUSED_GATES:
        raise ToolError("unknown_gate_id", f"Unknown focused gate id: {gate_id}")
    command = FOCUSED_GATES[gate_id]
    diagnostics: list[str] = []
    executable = command[0]
    if executable == "cmake":
        if shutil.which("cmake") is None or not (repo_root / "CMakeLists.txt").is_file():
            diagnostics.append("cmake_unavailable")
    elif executable == "git":
        if shutil.which("git") is None or not (repo_root / ".git").exists():
            diagnostics.append("git_unavailable")
    else:
        candidate = (repo_root / executable.replace("\\", "/").lstrip("./")).resolve()
        try:
            candidate.relative_to(repo_root.resolve())
        except ValueError:
            diagnostics.append("gate_executable_outside_repo")
        if not candidate.is_file():
            diagnostics.append(f"gate_executable_missing:{executable}")
    return {
        "gate_id": gate_id,
        "command": command,
        "ready": not diagnostics,
        "ran": False,
        "diagnostics": diagnostics,
        "guardrails": ["read_only_status", "allowlisted_commands_only"],
    }


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
    if name == "urpg.mcp_manifest":
        return _mcp_manifest(root)
    if name == "urpg.project_status":
        return _project_status(root)
    if name == "urpg.project_summary":
        return _project_summary(args, root)
    if name == "urpg.project_validate":
        return _project_validate(args, root)
    if name == "urpg.project_list_maps":
        return _project_list_maps(args, root)
    if name == "urpg.project_list_events":
        return _project_list_events(args, root)
    if name == "urpg.project_list_assets":
        return _project_list_assets(args, root)
    if name == "urpg.project_list_database":
        return _project_list_database(args, root)
    if name == "urpg.p2d_map_summary":
        return _p2d_map_summary(args, root)
    if name == "urpg.project_patch":
        return _project_patch(args, root)
    if name == "urpg.project_set_startup":
        return _narrow_patch(args, root, "set_startup_map", "map_id")
    if name == "urpg.p2d_add_map":
        return _narrow_patch(args, root, "add_p2d_map", "map_id")
    if name == "urpg.p2d_add_event":
        return _narrow_patch(args, root, "add_p2d_event", "event_id")
    if name == "urpg.p2d_add_event_command":
        return _p2d_add_event_command(args, root)
    if name == "urpg.database_add_record":
        return _database_add_record(args, root)
    if name == "urpg.project_add_asset_reference":
        return _narrow_patch(args, root, "add_asset_reference", "asset_id")
    if name == "urpg.playable_add_transfer":
        return _narrow_patch(args, root, "add_transfer", "transfer_id")
    if name == "urpg.project_restore_backup":
        return _project_restore_backup(args, root)
    if name == "urpg.asset_catalog_summary":
        return _asset_catalog_summary(args, root)
    if name == "urpg.p2d_capabilities":
        return _p2d_capabilities()
    if name == "urpg.focused_gate":
        return _focused_gate(args, root)
    if name == "urpg.gate_status":
        return _gate_status(args, root)
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
