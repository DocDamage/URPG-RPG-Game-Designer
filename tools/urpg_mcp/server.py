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
