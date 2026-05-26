#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
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
                "urpg.database_add_record",
                "urpg.gate_status",
                "urpg.mcp_manifest",
                "urpg.p2d_add_event",
                "urpg.p2d_add_event_command",
                "urpg.p2d_add_map",
                "urpg.p2d_map_summary",
                "urpg.playable_add_transfer",
                "urpg.project_add_asset_reference",
                "urpg.project_list_assets",
                "urpg.project_list_database",
                "urpg.project_list_events",
                "urpg.project_list_maps",
                "urpg.project_status",
                "urpg.project_summary",
                "urpg.project_validate",
                "urpg.project_patch",
                "urpg.project_restore_backup",
                "urpg.project_set_startup",
                "urpg.asset_catalog_summary",
                "urpg.p2d_capabilities",
                "urpg.focused_gate",
                "urpg.release_guardrails",
            },
        )
        for tool in tools:
            self.assertIn("inputSchema", tool)
            self.assertEqual(tool["inputSchema"]["type"], "object")

    def test_manifest_reports_versions_tools_and_guardrails(self) -> None:
        result = server.call_tool("urpg.mcp_manifest", {}, repo_root=REPO_ROOT)

        self.assertEqual(result["server_name"], "urpg-local-mcp")
        self.assertEqual(result["protocol_version"], "2024-11-05")
        self.assertEqual(result["project_schema_version"], "urpg.project.v1")
        self.assertIn("urpg.project_patch", result["tools"])
        self.assertIn("tool_list_checksum", result)
        self.assertIn("local_only", result["guardrails"])

    def test_project_patch_schema_includes_parameterized_command_fields(self) -> None:
        tools = {tool["name"]: tool for tool in server.list_tools()}
        schema = tools["urpg.project_patch"]["inputSchema"]

        properties = schema["properties"]
        for field in (
            "amount",
            "common_event_id",
            "condition",
            "item_id",
            "map_id",
            "operation",
            "route",
            "self_switch",
            "state",
            "switch_id",
            "text",
            "variable_id",
            "x",
            "y",
        ):
            self.assertIn(field, properties)

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
        self.assertEqual(len(listed["result"]["tools"]), len(server.list_tools()))

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

    def test_stdio_json_rpc_subprocess_contract(self) -> None:
        request = {
            "jsonrpc": "2.0",
            "id": 77,
            "method": "tools/call",
            "params": {"name": "urpg.mcp_manifest", "arguments": {}},
        }
        completed = subprocess.run(
            [sys.executable, "tools/urpg_mcp/server.py"],
            cwd=REPO_ROOT,
            input=json.dumps(request) + "\n",
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
            timeout=10,
        )

        self.assertEqual(completed.returncode, 0)
        response = json.loads(completed.stdout)
        payload = json.loads(response["result"]["content"][0]["text"])
        self.assertEqual(response["id"], 77)
        self.assertEqual(payload["server_name"], "urpg-local-mcp")

    def test_project_summary_reads_bounded_project_json(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps(
                    {
                        "name": "MCP Project",
                        "startup": {
                            "map": "Town",
                            "map_assets": {
                                "player_sprite": {"id": "hero"},
                                "tileset": {"id": "overworld"},
                            },
                        },
                        "maps": [{"id": "Town"}, {"id": "Castle"}],
                        "p2d": {
                            "maps": [{"id": "Town"}],
                            "events": [{"id": "ev_001"}],
                            "tilesets": [{"id": "overworld"}],
                        },
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool(
                "urpg.project_summary",
                {"project_path": "project.json"},
                repo_root=root,
            )

        self.assertEqual(result["name"], "MCP Project")
        self.assertEqual(result["startup_map"], "Town")
        self.assertEqual(result["map_count"], 2)
        self.assertEqual(result["p2d_map_count"], 1)
        self.assertEqual(result["p2d_event_count"], 1)
        self.assertEqual(result["asset_ids"], ["hero", "overworld"])

    def test_project_summary_rejects_path_escape(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaises(server.ToolError) as context:
                server.call_tool(
                    "urpg.project_summary",
                    {"project_path": "../outside.json"},
                    repo_root=Path(tmp),
                )

        self.assertEqual(context.exception.code, "path_outside_repo")

    def test_project_patch_previews_startup_map_without_writing(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps({"name": "Patch Project", "startup": {"map": "Town"}}),
                encoding="utf-8",
            )

            result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "set_startup_map",
                    "value": "Castle",
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertFalse(result["applied"])
        self.assertEqual(result["patch"], [{"op": "replace", "path": "/startup/map", "value": "Castle"}])
        self.assertEqual(result["preview"]["startup"]["map"], "Castle")
        self.assertEqual(result["summary"], "set_startup_map: /startup/map -> Castle")
        self.assertEqual(result["changed_sections"], ["startup"])
        self.assertEqual(result["changed_subtrees"]["startup"]["before"], {"map": "Town"})
        self.assertEqual(result["changed_subtrees"]["startup"]["after"], {"map": "Castle"})
        self.assertEqual(loaded["startup"]["map"], "Town")

    def test_project_patch_applies_when_explicit(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps({"name": "Patch Project", "startup": {"map": "Town"}}),
                encoding="utf-8",
            )

            result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "set_startup_map",
                    "value": "Castle",
                    "apply": True,
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))
            backup_path = Path(result["backup_path"])
            self.assertTrue(backup_path.is_file())
            backup = json.loads(backup_path.read_text(encoding="utf-8"))
            manifest_path = project_path.with_suffix(project_path.suffix + ".urpg_mcp_backups.json")
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))

        self.assertTrue(result["applied"])
        self.assertEqual(loaded["startup"]["map"], "Castle")
        self.assertEqual(backup["startup"]["map"], "Town")
        self.assertIn(".urpg_mcp_backup.", backup_path.name)
        self.assertEqual(manifest["backups"][-1]["patch_kind"], "set_startup_map")
        self.assertEqual(manifest["backups"][-1]["backup_path"], str(backup_path))

    def test_project_restore_backup_restores_bounded_backup(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps({"name": "Restore Project", "startup": {"map": "Town"}}),
                encoding="utf-8",
            )
            patch_result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "set_startup_map",
                    "value": "Castle",
                    "apply": True,
                },
                repo_root=root,
            )

            restore_result = server.call_tool(
                "urpg.project_restore_backup",
                {"project_path": "project.json", "backup_path": patch_result["backup_path"], "apply": True},
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertTrue(restore_result["applied"])
        self.assertEqual(restore_result["summary"], "restore_backup: project.json")
        self.assertEqual(loaded["startup"]["map"], "Town")

    def test_project_patch_rejects_unknown_patch_kind(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text("{}", encoding="utf-8")
            with self.assertRaises(server.ToolError) as context:
                server.call_tool(
                    "urpg.project_patch",
                    {"project_path": "project.json", "patch_kind": "arbitrary_write", "value": "x"},
                    repo_root=root,
                )

        self.assertEqual(context.exception.code, "unknown_patch_kind")

    def test_narrow_tools_delegate_to_allowlisted_project_patches(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps(
                    {
                        "name": "Narrow Tool Project",
                        "p2d": {"events": [{"id": "ev_intro", "map_id": "Town"}]},
                    }
                ),
                encoding="utf-8",
            )

            calls = [
                ("urpg.project_set_startup", {"map_id": "Town"}),
                ("urpg.p2d_add_map", {"map_id": "Town"}),
                ("urpg.p2d_add_event", {"event_id": "ev_shop", "map_id": "Town", "label": "Shop"}),
                (
                    "urpg.p2d_add_event_command",
                    {"event_id": "ev_intro", "command_type": "show_text", "text": "Welcome"},
                ),
                (
                    "urpg.database_add_record",
                    {"collection": "actors", "record_id": "actor.hero", "label": "Hero"},
                ),
                (
                    "urpg.playable_add_transfer",
                    {"transfer_id": "transfer.town.castle", "from_map": "Town", "to_map": "Castle"},
                ),
                (
                    "urpg.project_add_asset_reference",
                    {"asset_id": "asset.tiles.town", "label": "Town Tiles", "path": "content/tiles/town.png"},
                ),
            ]
            results = [
                server.call_tool(name, {"project_path": "project.json", **arguments}, repo_root=root)
                for name, arguments in calls
            ]

        self.assertEqual(
            [result["patch_kind"] for result in results],
            [
                "set_startup_map",
                "add_p2d_map",
                "add_p2d_event",
                "add_p2d_event_command",
                "add_actor",
                "add_transfer",
                "add_asset_reference",
            ],
        )
        self.assertTrue(all(result["summary"] for result in results))

    def test_project_validate_reports_missing_startup_map_reference(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps({"name": "Invalid Project", "startup": {"map": "Missing"}, "maps": [{"id": "Town"}]}),
                encoding="utf-8",
            )

            result = server.call_tool("urpg.project_validate", {"project_path": "project.json"}, repo_root=root)

        self.assertFalse(result["valid"])
        self.assertIn("startup_map_missing", result["diagnostics"])

    def test_project_validate_reports_structural_and_field_diagnostics(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "content").mkdir()
            (root / "project.json").write_text(
                json.dumps(
                    {
                        "name": "Broken Structure Project",
                        "maps": [{"id": "Town"}, {"id": "Town"}],
                        "assets": [{"id": "asset.missing", "path": "content/missing.png"}],
                        "transfers": [{"id": "transfer.bad", "from_map": "Town", "to_map": "Town", "x": "bad"}],
                        "p2d": {
                            "tilesets": [
                                {
                                    "id": "tileset.bad",
                                    "pages": ["AA"],
                                    "tiles": [
                                        {
                                            "id": "tile.bad",
                                            "page": "AA",
                                            "passability": "maybe",
                                            "collision": "opaque",
                                            "priority": 99,
                                        }
                                    ],
                                }
                            ],
                            "events": [{"id": "ev_bad", "commands": [{"type": "change_gold"}]}],
                        },
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool("urpg.project_validate", {"project_path": "project.json"}, repo_root=root)

        self.assertFalse(result["valid"])
        self.assertIn("duplicate_map_id:Town", result["diagnostics"])
        self.assertIn("asset_path_missing:asset.missing:content/missing.png", result["diagnostics"])
        self.assertIn("transfer_x_invalid:transfer.bad", result["diagnostics"])
        self.assertIn("p2d_tileset_page_invalid:tileset.bad:AA", result["diagnostics"])
        self.assertIn("p2d_tile_passability_invalid:tileset.bad:tile.bad:maybe", result["diagnostics"])
        self.assertIn("p2d_tile_collision_invalid:tileset.bad:tile.bad:opaque", result["diagnostics"])
        self.assertIn("p2d_tile_priority_invalid:tileset.bad:tile.bad:99", result["diagnostics"])
        self.assertIn("p2d_event_command_missing_field:ev_bad:change_gold:operation", result["diagnostics"])

    def test_project_validate_accepts_p2d_references(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps(
                    {
                        "name": "Valid Project",
                        "startup": {"map": "Town"},
                        "maps": [{"id": "Town"}],
                        "p2d": {
                            "maps": [{"id": "Town"}],
                            "events": [{"id": "ev_001", "map_id": "Town"}],
                            "tilesets": [{"id": "overworld"}],
                        },
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool("urpg.project_validate", {"project_path": "project.json"}, repo_root=root)

        self.assertTrue(result["valid"])
        self.assertEqual(result["diagnostics"], [])

    def test_project_inspection_tools_return_read_only_summaries(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps(
                    {
                        "name": "Inspect Project",
                        "maps": [{"id": "Town", "name": "Town"}, {"id": "Castle"}],
                        "assets": [{"id": "asset.tiles.town", "media_kind": "tileset"}],
                        "database": {"actors": [{"id": "actor.hero"}], "items": [{"id": "item.potion"}]},
                        "p2d": {
                            "maps": [{"id": "Town", "tileset_id": "tileset.town"}],
                            "events": [{"id": "ev_intro", "map_id": "Town"}],
                            "tilesets": [{"id": "tileset.town"}],
                        },
                    }
                ),
                encoding="utf-8",
            )

            maps = server.call_tool("urpg.project_list_maps", {"project_path": "project.json"}, repo_root=root)
            events = server.call_tool("urpg.project_list_events", {"project_path": "project.json"}, repo_root=root)
            assets = server.call_tool("urpg.project_list_assets", {"project_path": "project.json"}, repo_root=root)
            database = server.call_tool(
                "urpg.project_list_database",
                {"project_path": "project.json"},
                repo_root=root,
            )
            p2d_map = server.call_tool(
                "urpg.p2d_map_summary",
                {"project_path": "project.json", "map_id": "Town"},
                repo_root=root,
            )

        self.assertEqual(maps["maps"][0]["id"], "Town")
        self.assertEqual(events["events"][0]["id"], "ev_intro")
        self.assertEqual(assets["assets"][0]["id"], "asset.tiles.town")
        self.assertEqual(database["collections"]["actors"][0]["id"], "actor.hero")
        self.assertEqual(p2d_map["event_count"], 1)
        self.assertEqual(p2d_map["tileset_id"], "tileset.town")

    def test_project_validate_reports_missing_startup_asset_reference(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps(
                    {
                        "name": "Invalid Asset Project",
                        "startup": {
                            "map": "Town",
                            "map_assets": {
                                "tileset": {"id": "asset.tiles.missing"},
                            },
                        },
                        "maps": [{"id": "Town"}],
                        "assets": [{"id": "asset.tiles.grass"}],
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool("urpg.project_validate", {"project_path": "project.json"}, repo_root=root)

        self.assertFalse(result["valid"])
        self.assertIn("startup_asset_missing:asset.tiles.missing", result["diagnostics"])

    def test_project_patch_sets_map_asset(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps({"name": "Asset Project", "startup": {"map_assets": {}}}),
                encoding="utf-8",
            )

            result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "set_map_asset",
                    "key": "tileset",
                    "value": "overworld",
                },
                repo_root=root,
            )

        self.assertFalse(result["applied"])
        self.assertEqual(result["preview"]["startup"]["map_assets"]["tileset"]["id"], "overworld")
        self.assertEqual(
            result["patch"],
            [{"op": "add", "path": "/startup/map_assets/tileset", "value": {"id": "overworld"}}],
        )

    def test_project_patch_adds_p2d_map_and_event(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(json.dumps({"name": "P2D Project"}), encoding="utf-8")

            map_result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "add_p2d_map",
                    "value": "Town",
                    "apply": True,
                },
                repo_root=root,
            )
            event_result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "add_p2d_event",
                    "value": "ev_intro",
                    "map_id": "Town",
                    "label": "Intro",
                    "apply": True,
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertTrue(map_result["applied"])
        self.assertTrue(event_result["applied"])
        self.assertEqual(loaded["p2d"]["maps"][0]["id"], "Town")
        self.assertEqual(loaded["p2d"]["events"][0]["id"], "ev_intro")
        self.assertEqual(loaded["p2d"]["events"][0]["map_id"], "Town")

    def test_project_patch_adds_p2d_tilesets_and_tile_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(json.dumps({"name": "Tileset Project"}), encoding="utf-8")

            tileset_result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "add_p2d_tileset",
                    "value": "tileset.overworld",
                    "label": "Overworld",
                    "asset_id": "asset.tiles.overworld",
                    "page": "A",
                    "apply": True,
                },
                repo_root=root,
            )
            tile_result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "set_p2d_tile_metadata",
                    "value": "tile.grass",
                    "tileset_id": "tileset.overworld",
                    "page": "A",
                    "passability": "star",
                    "collision": "blocked",
                    "terrain_tag": "field",
                    "region_id": "7",
                    "priority": "3",
                    "animated": True,
                    "apply": True,
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertTrue(tileset_result["applied"])
        self.assertTrue(tile_result["applied"])
        self.assertEqual(
            loaded["p2d"]["tilesets"],
            [
                {
                    "id": "tileset.overworld",
                    "name": "Overworld",
                    "asset_id": "asset.tiles.overworld",
                    "pages": ["A"],
                    "tiles": [
                        {
                            "id": "tile.grass",
                            "page": "A",
                            "passability": "star",
                            "collision": "blocked",
                            "terrain_tag": "field",
                            "region_id": 7,
                            "priority": 3,
                            "animated": True,
                        }
                    ],
                }
            ],
        )

    def test_project_patch_adds_p2d_event_commands(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps(
                    {
                        "name": "Event Command Project",
                        "p2d": {
                            "events": [
                                {"id": "ev_intro", "map_id": "Town", "label": "Intro"},
                            ]
                        },
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "add_p2d_event_command",
                    "value": "show_text",
                    "event_id": "ev_intro",
                    "text": "Welcome home.",
                    "apply": True,
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertTrue(result["applied"])
        self.assertEqual(loaded["p2d"]["events"][0]["commands"], [{"type": "show_text", "text": "Welcome home."}])

    def test_project_patch_adds_parameterized_p2d_event_commands(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps(
                    {
                        "name": "Parameterized Event Command Project",
                        "p2d": {"events": [{"id": "ev_intro", "map_id": "Town"}]},
                    }
                ),
                encoding="utf-8",
            )
            commands = [
                {"value": "transfer_player", "map_id": "Castle", "x": "8", "y": "3"},
                {"value": "change_switch", "switch_id": "switch.door_open", "state": True},
                {"value": "change_variable", "variable_id": "variable.rank", "operation": "set", "amount": "5"},
                {"value": "change_self_switch", "self_switch": "A", "state": False},
                {"value": "change_gold", "operation": "add", "amount": "25"},
                {"value": "change_item", "item_id": "item.potion", "operation": "add", "amount": "2"},
                {"value": "call_common_event", "common_event_id": "common.unlock"},
                {"value": "move_route", "route": "left,up,wait"},
                {"value": "conditional_branch", "condition": "switch.door_open == true"},
            ]

            for command in commands:
                result = server.call_tool(
                    "urpg.project_patch",
                    {
                        "project_path": "project.json",
                        "patch_kind": "add_p2d_event_command",
                        "event_id": "ev_intro",
                        "apply": True,
                        **command,
                    },
                    repo_root=root,
                )
                self.assertTrue(result["applied"])
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertEqual(
            loaded["p2d"]["events"][0]["commands"],
            [
                {"type": "transfer_player", "map_id": "Castle", "x": 8, "y": 3},
                {"type": "change_switch", "switch_id": "switch.door_open", "state": True},
                {"type": "change_variable", "variable_id": "variable.rank", "operation": "set", "amount": 5},
                {"type": "change_self_switch", "self_switch": "A", "state": False},
                {"type": "change_gold", "operation": "add", "amount": 25},
                {"type": "change_item", "item_id": "item.potion", "operation": "add", "amount": 2},
                {"type": "call_common_event", "common_event_id": "common.unlock"},
                {"type": "move_route", "route": "left,up,wait"},
                {"type": "conditional_branch", "condition": "switch.door_open == true"},
            ],
        )

    def test_project_patch_rejects_parameterized_event_command_missing_required_field(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps({"name": "Invalid Command Project", "p2d": {"events": [{"id": "ev_intro"}]}}),
                encoding="utf-8",
            )

            with self.assertRaises(server.ToolError) as context:
                server.call_tool(
                    "urpg.project_patch",
                    {
                        "project_path": "project.json",
                        "patch_kind": "add_p2d_event_command",
                        "event_id": "ev_intro",
                        "value": "transfer_player",
                        "x": "8",
                        "y": "3",
                        "apply": True,
                    },
                    repo_root=root,
                )

        self.assertEqual(context.exception.code, "invalid_patch_value")

    def test_project_patch_rejects_event_command_for_missing_p2d_event(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(json.dumps({"name": "Missing Event Project"}), encoding="utf-8")

            with self.assertRaises(server.ToolError) as context:
                server.call_tool(
                    "urpg.project_patch",
                    {
                        "project_path": "project.json",
                        "patch_kind": "add_p2d_event_command",
                        "value": "show_text",
                        "event_id": "ev_missing",
                        "text": "Nope.",
                        "apply": True,
                    },
                    repo_root=root,
                )

        self.assertEqual(context.exception.code, "p2d_event_missing")

    def test_project_validate_reports_unsupported_p2d_event_command(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps(
                    {
                        "name": "Invalid Command Project",
                        "p2d": {
                            "events": [
                                {
                                    "id": "ev_intro",
                                    "commands": [{"type": "launch_rocket"}],
                                }
                            ]
                        },
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool("urpg.project_validate", {"project_path": "project.json"}, repo_root=root)

        self.assertFalse(result["valid"])
        self.assertIn("p2d_event_command_unsupported:ev_intro:launch_rocket", result["diagnostics"])

    def test_project_patch_adds_starting_party_transfer_encounter_and_save_profile(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(
                json.dumps(
                    {
                        "name": "Playable Project",
                        "maps": [{"id": "Town"}, {"id": "Castle"}],
                        "database": {
                            "actors": [{"id": "actor.hero", "name": "Hero"}],
                            "items": [{"id": "enemy.slime", "name": "Slime"}],
                        },
                    }
                ),
                encoding="utf-8",
            )

            patches = [
                {"patch_kind": "add_starting_party_actor", "value": "actor.hero"},
                {
                    "patch_kind": "add_transfer",
                    "value": "transfer.town.castle",
                    "from_map": "Town",
                    "to_map": "Castle",
                    "x": "12",
                    "y": "4",
                },
                {
                    "patch_kind": "add_encounter",
                    "value": "encounter.town.slime",
                    "map_id": "Town",
                    "enemy_id": "enemy.slime",
                    "weight": "8",
                },
                {
                    "patch_kind": "add_save_profile",
                    "value": "save.quick",
                    "label": "Quick Save",
                    "slot": "1",
                },
            ]
            for patch in patches:
                result = server.call_tool(
                    "urpg.project_patch",
                    {"project_path": "project.json", "apply": True, **patch},
                    repo_root=root,
                )
                self.assertTrue(result["applied"])

            duplicate = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "add_starting_party_actor",
                    "value": "actor.hero",
                    "apply": True,
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertEqual(duplicate["patch"][0]["op"], "test")
        self.assertEqual(loaded["startup"]["starting_party"], [{"id": "actor.hero"}])
        self.assertEqual(
            loaded["transfers"],
            [{"id": "transfer.town.castle", "from_map": "Town", "to_map": "Castle", "x": 12, "y": 4}],
        )
        self.assertEqual(
            loaded["encounters"],
            [{"id": "encounter.town.slime", "map_id": "Town", "enemy_id": "enemy.slime", "weight": 8}],
        )
        self.assertEqual(loaded["save_profiles"], [{"id": "save.quick", "name": "Quick Save", "slot": 1}])

    def test_project_validate_reports_playable_project_reference_errors(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "project.json").write_text(
                json.dumps(
                    {
                        "name": "Broken Playable Project",
                        "maps": [{"id": "Town"}],
                        "startup": {"starting_party": [{"id": "actor.missing"}]},
                        "database": {"actors": [{"id": "actor.hero"}], "items": [{"id": "enemy.slime"}]},
                        "transfers": [
                            {"id": "transfer.bad", "from_map": "Town", "to_map": "Castle"},
                        ],
                        "encounters": [
                            {"id": "encounter.bad", "map_id": "Castle", "enemy_id": "enemy.missing"},
                        ],
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool("urpg.project_validate", {"project_path": "project.json"}, repo_root=root)

        self.assertFalse(result["valid"])
        self.assertIn("starting_party_actor_missing:actor.missing", result["diagnostics"])
        self.assertIn("transfer_to_map_missing:transfer.bad:Castle", result["diagnostics"])
        self.assertIn("encounter_map_missing:encounter.bad:Castle", result["diagnostics"])
        self.assertIn("encounter_enemy_missing:encounter.bad:enemy.missing", result["diagnostics"])

    def test_project_patch_adds_database_records_and_asset_references(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            project_path = root / "project.json"
            project_path.write_text(json.dumps({"name": "Database Project"}), encoding="utf-8")

            patches = [
                {"patch_kind": "add_actor", "value": "actor.hero", "label": "Hero"},
                {"patch_kind": "add_item", "value": "item.potion", "label": "Potion"},
                {"patch_kind": "add_switch", "value": "switch.door_open", "label": "Door Open"},
                {"patch_kind": "add_variable", "value": "variable.rank", "label": "Rank"},
                {"patch_kind": "add_common_event", "value": "common.unlock", "label": "Unlock Door"},
                {
                    "patch_kind": "add_asset_reference",
                    "value": "asset.overworld.tiles",
                    "label": "Overworld Tiles",
                    "path": "content/tiles/overworld.png",
                },
            ]
            for patch in patches:
                result = server.call_tool(
                    "urpg.project_patch",
                    {"project_path": "project.json", "apply": True, **patch},
                    repo_root=root,
                )
                self.assertTrue(result["applied"])

            duplicate = server.call_tool(
                "urpg.project_patch",
                {
                    "project_path": "project.json",
                    "patch_kind": "add_actor",
                    "value": "actor.hero",
                    "label": "Hero",
                    "apply": True,
                },
                repo_root=root,
            )
            loaded = json.loads(project_path.read_text(encoding="utf-8"))

        self.assertEqual(duplicate["patch"][0]["op"], "test")
        self.assertEqual(loaded["database"]["actors"], [{"id": "actor.hero", "name": "Hero"}])
        self.assertEqual(loaded["database"]["items"], [{"id": "item.potion", "name": "Potion"}])
        self.assertEqual(loaded["database"]["switches"], [{"id": "switch.door_open", "name": "Door Open"}])
        self.assertEqual(loaded["database"]["variables"], [{"id": "variable.rank", "name": "Rank"}])
        self.assertEqual(loaded["database"]["common_events"], [{"id": "common.unlock", "name": "Unlock Door"}])
        self.assertEqual(
            loaded["assets"],
            [{"id": "asset.overworld.tiles", "name": "Overworld Tiles", "path": "content/tiles/overworld.png"}],
        )

    def test_asset_catalog_summary_counts_media_license_and_release_status(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "catalog.json").write_text(
                json.dumps(
                    {
                        "records": [
                            {
                                "id": "asset.tiles.grass",
                                "mediaKind": "tileset",
                                "licenseStatus": "cleared",
                                "releaseReady": True,
                            },
                            {
                                "asset_id": "asset.music.theme",
                                "media_kind": "audio",
                                "license_status": "verified",
                            },
                            {
                                "id": "asset.sketch.local",
                                "mediaKind": "concept",
                                "licenseStatus": "dev_only",
                            },
                        ]
                    }
                ),
                encoding="utf-8",
            )

            result = server.call_tool(
                "urpg.asset_catalog_summary",
                {"catalog_path": "catalog.json"},
                repo_root=root,
            )

        self.assertEqual(result["asset_count"], 3)
        self.assertEqual(result["media_kind_counts"], {"audio": 1, "concept": 1, "tileset": 1})
        self.assertEqual(result["license_status_counts"], {"cleared": 1, "dev_only": 1, "verified": 1})
        self.assertEqual(result["release_ready_count"], 2)
        self.assertEqual(
            result["asset_ids"],
            ["asset.music.theme", "asset.sketch.local", "asset.tiles.grass"],
        )

    def test_asset_catalog_summary_rejects_path_escape(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaises(server.ToolError) as context:
                server.call_tool(
                    "urpg.asset_catalog_summary",
                    {"catalog_path": "../catalog.json"},
                    repo_root=Path(tmp),
                )

        self.assertEqual(context.exception.code, "path_outside_repo")

    def test_gate_status_reports_readiness_without_running_commands(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "build" / "dev-ninja-debug").mkdir(parents=True)
            (root / "build" / "dev-ninja-debug" / "urpg_spatial_unit_tests.exe").write_text("", encoding="utf-8")

            ready = server.call_tool("urpg.gate_status", {"gate_id": "p2d_depth"}, repo_root=root)
            missing = server.call_tool("urpg.gate_status", {"gate_id": "build_spatial"}, repo_root=root)

        self.assertEqual(ready["gate_id"], "p2d_depth")
        self.assertTrue(ready["ready"])
        self.assertFalse(ready["ran"])
        self.assertFalse(missing["ready"])
        self.assertIn("cmake_unavailable", missing["diagnostics"])


if __name__ == "__main__":
    unittest.main()
