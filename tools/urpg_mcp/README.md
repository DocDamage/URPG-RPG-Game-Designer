# URPG Local MCP

`tools/urpg_mcp/server.py` is a local-only MCP-style JSON-RPC stdio server for IDE agents.

## Tools

- `urpg.mcp_manifest`: reports server/protocol/project-schema versions, tool names, tool-list checksum, repo root, and guardrails.
- `urpg.project_status`: reports local branch, head, dirty files, diagnostics, and guardrails.
- `urpg.project_summary`: reads a bounded project JSON file and reports startup map, map counts, P2D counts, and asset ids.
- `urpg.project_validate`: validates startup, startup asset, P2D references, duplicate ids, asset paths, tile metadata, P2D event command fields, starting party actors, transfers, and encounters inside bounded project JSON.
- `urpg.project_list_maps`: reads project map records without writing.
- `urpg.project_list_events`: reads P2D event records without writing.
- `urpg.project_list_assets`: reads project asset references without writing.
- `urpg.project_list_database`: reads supported database collections without writing.
- `urpg.p2d_map_summary`: summarizes one P2D map, its tileset id, and its events without writing.
- `urpg.project_patch`: previews or explicitly applies an allowlisted project JSON patch. Supported patch kinds are `set_startup_map`, `set_map_asset`, `add_p2d_map`, `add_p2d_event`, `add_p2d_tileset`, `set_p2d_tile_metadata`, `add_p2d_event_command`, `add_actor`, `add_item`, `add_switch`, `add_variable`, `add_common_event`, `add_asset_reference`, `add_starting_party_actor`, `add_transfer`, `add_encounter`, and `add_save_profile`. Patch responses include JSON Patch, a summary, changed top-level sections, changed subtrees, and a full preview.
- `urpg.project_set_startup`: narrow wrapper for startup map changes.
- `urpg.p2d_add_map`: narrow wrapper for P2D map insertion.
- `urpg.p2d_add_event`: narrow wrapper for P2D event insertion.
- `urpg.p2d_add_event_command`: narrow wrapper for typed P2D event command insertion.
- `urpg.database_add_record`: narrow wrapper for actor, item, switch, variable, and common-event record insertion.
- `urpg.project_add_asset_reference`: narrow wrapper for asset reference insertion.
- `urpg.playable_add_transfer`: narrow wrapper for transfer insertion.
- `urpg.project_restore_backup`: previews or explicitly restores a bounded project JSON backup.
- `urpg.asset_catalog_summary`: reads a bounded asset catalog JSON file and reports media-kind counts, license-status counts, release-ready count, and asset ids.
- `urpg.p2d_capabilities`: reports the current Perspective 2D tile, event runtime, and project/database capability surface.
- `urpg.focused_gate`: returns an allowlisted validation command and runs it only when `run=true`.
- `urpg.gate_status`: reports whether an allowlisted gate appears runnable without running it.
- `urpg.release_guardrails`: reports safety rules the server refuses to bypass.

## Run

```powershell
python tools/urpg_mcp/server.py
```

The server reads one JSON-RPC request per line from stdin and writes one response per line to stdout.

## Guardrails

- Local filesystem only.
- Allowlisted commands only.
- No arbitrary shell command tool.
- No arbitrary file write tool.
- Project writes require an allowlisted `patch_kind` and `apply=true`.
- Project writes create a timestamped `.urpg_mcp_backup.<timestamp>.json` copy and update a `.urpg_mcp_backups.json` manifest before replacing JSON.
- Backup restore is bounded to JSON files inside the repository and also requires `apply=true`.
- Asset catalog reads are summary-only and never alter license or release eligibility state.
- No destructive git operations.
- No release, LFS, or asset-license gate bypass.

## Examples

Inspect the server contract:

```json
{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"urpg.mcp_manifest","arguments":{}}}
```

Preview a startup map change:

```json
{"jsonrpc":"2.0","id":2,"method":"tools/call","params":{"name":"urpg.project_set_startup","arguments":{"project_path":"project.json","map_id":"Town"}}}
```

Apply a typed P2D event command:

```json
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"urpg.p2d_add_event_command","arguments":{"project_path":"project.json","event_id":"ev_intro","command_type":"show_text","text":"Welcome home.","apply":true}}}
```

Check a focused gate before running it:

```json
{"jsonrpc":"2.0","id":4,"method":"tools/call","params":{"name":"urpg.gate_status","arguments":{"gate_id":"p2d_depth"}}}
```

Restore a previous backup:

```json
{"jsonrpc":"2.0","id":5,"method":"tools/call","params":{"name":"urpg.project_restore_backup","arguments":{"project_path":"project.json","backup_path":"project.json.urpg_mcp_backup.20260526T120000000000Z.json","apply":true}}}
```

## Extend

Add a tool to `list_tools()`, implement it in `call_tool()`, and add coverage in
`tools/urpg_mcp/tests/test_server.py`. Prefer returning structured diagnostics and
commands first; run commands only behind explicit arguments and a fixed allowlist. New mutating workflows should prefer narrow tools that delegate to the same bounded patch helpers as `urpg.project_patch`.
