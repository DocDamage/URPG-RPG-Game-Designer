#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.urpg_mcp import server  # noqa: E402


class UrpgMcpServerTests(unittest.TestCase):
    def test_tool_list_exposes_local_urpg_controls(self) -> None:
        tools = server.list_tools()

        names = {tool["name"] for tool in tools}
        self.assertEqual(
            names,
            {
                "urpg.project_status",
                "urpg.p2d_capabilities",
                "urpg.focused_gate",
                "urpg.release_guardrails",
            },
        )
        for tool in tools:
            self.assertIn("inputSchema", tool)
            self.assertEqual(tool["inputSchema"]["type"], "object")

    def test_focused_gate_returns_command_without_running_by_default(self) -> None:
        result = server.call_tool(
            "urpg.focused_gate",
            {"gate_id": "p2d_depth"},
            repo_root=REPO_ROOT,
        )

        self.assertFalse(result["ran"])
        self.assertEqual(result["gate_id"], "p2d_depth")
        self.assertEqual(
            result["command"],
            [
                ".\\build\\dev-ninja-debug\\urpg_spatial_unit_tests.exe",
                "[editor][spatial][p2d_depth]",
                "--reporter",
                "compact",
            ],
        )
        self.assertIn("allowlisted", result["guardrail"])

    def test_focused_gate_rejects_unknown_gate_id(self) -> None:
        with self.assertRaises(server.ToolError) as context:
            server.call_tool("urpg.focused_gate", {"gate_id": "rm -rf"}, repo_root=REPO_ROOT)

        self.assertEqual(context.exception.code, "unknown_gate_id")

    def test_unknown_tool_is_rejected(self) -> None:
        with self.assertRaises(server.ToolError) as context:
            server.call_tool("urpg.shell", {"command": "anything"}, repo_root=REPO_ROOT)

        self.assertEqual(context.exception.code, "unknown_tool")

    def test_project_status_reports_git_and_guardrails(self) -> None:
        completed = server.CompletedProcessLike(
            returncode=0,
            stdout="development\n52a7d56419\n M docs/example.md\n",
            stderr="",
        )
        with mock.patch.object(server, "_run_git_status", return_value=completed):
            result = server.call_tool("urpg.project_status", {}, repo_root=REPO_ROOT)

        self.assertEqual(result["branch"], "development")
        self.assertEqual(result["head"], "52a7d56419")
        self.assertTrue(result["dirty"])
        self.assertIn("no_destructive_git", result["guardrails"])

    def test_project_status_handles_non_git_folder(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            result = server.call_tool("urpg.project_status", {}, repo_root=Path(tmp))

        self.assertEqual(result["branch"], "unknown")
        self.assertEqual(result["head"], "unknown")
        self.assertIn("repo_status_unavailable", result["diagnostics"])

    def test_json_rpc_initialize_tools_and_call(self) -> None:
        initialize = server.handle_json_rpc(
            {"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}},
            repo_root=REPO_ROOT,
        )
        self.assertEqual(initialize["id"], 1)
        self.assertEqual(initialize["result"]["serverInfo"]["name"], "urpg-local-mcp")

        listed = server.handle_json_rpc(
            {"jsonrpc": "2.0", "id": 2, "method": "tools/list", "params": {}},
            repo_root=REPO_ROOT,
        )
        self.assertEqual(len(listed["result"]["tools"]), 4)

        called = server.handle_json_rpc(
            {
                "jsonrpc": "2.0",
                "id": 3,
                "method": "tools/call",
                "params": {"name": "urpg.release_guardrails", "arguments": {}},
            },
            repo_root=REPO_ROOT,
        )
        payload = json.loads(called["result"]["content"][0]["text"])
        self.assertIn("local_only", payload["guardrails"])

    def test_json_rpc_unknown_tool_returns_error(self) -> None:
        response = server.handle_json_rpc(
            {
                "jsonrpc": "2.0",
                "id": 4,
                "method": "tools/call",
                "params": {"name": "urpg.shell", "arguments": {}},
            },
            repo_root=REPO_ROOT,
        )

        self.assertEqual(response["error"]["code"], -32000)
        self.assertEqual(response["error"]["data"]["code"], "unknown_tool")


if __name__ == "__main__":
    unittest.main()
