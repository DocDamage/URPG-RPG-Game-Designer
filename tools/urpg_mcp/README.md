# URPG Local MCP

`tools/urpg_mcp/server.py` is a local-only MCP-style JSON-RPC stdio server for IDE agents.

## Tools

- `urpg.project_status`: reports local branch, head, dirty files, diagnostics, and guardrails.
- `urpg.project_summary`: reads a bounded project JSON file and reports startup map, map counts, P2D counts, and asset ids.
- `urpg.project_validate`: validates startup and P2D references inside bounded project JSON.
- `urpg.project_patch`: previews or explicitly applies an allowlisted project JSON patch. Supported patch kinds are `set_startup_map`, `set_map_asset`, `add_p2d_map`, and `add_p2d_event`.
- `urpg.p2d_capabilities`: reports the current Perspective 2D tile, event runtime, and project/database capability surface.
- `urpg.focused_gate`: returns an allowlisted validation command and runs it only when `run=true`.
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
- Project writes create a `.urpg_mcp_backup` copy before replacing JSON.
- No destructive git operations.
- No release, LFS, or asset-license gate bypass.

## Extend

Add a tool to `list_tools()`, implement it in `call_tool()`, and add coverage in
`tools/urpg_mcp/tests/test_server.py`. Prefer returning structured diagnostics and
commands first; run commands only behind explicit arguments and a fixed allowlist.
