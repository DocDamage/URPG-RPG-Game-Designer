# URPG Local MCP

`tools/urpg_mcp/server.py` is a local-only MCP-style JSON-RPC stdio server for IDE agents.

## Tools

- `urpg.project_status`: reports local branch, head, dirty files, diagnostics, and guardrails.
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
- No destructive git operations.
- No release, LFS, or asset-license gate bypass.

## Extend

Add a tool to `list_tools()`, implement it in `call_tool()`, and add coverage in
`tools/urpg_mcp/tests/test_server.py`. Prefer returning structured diagnostics and
commands first; run commands only behind explicit arguments and a fixed allowlist.
