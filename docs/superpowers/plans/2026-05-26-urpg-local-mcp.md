# URPG Local MCP Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a local-only URPG MCP-style server so IDE agents can inspect and operate bounded URPG workflows through safe, deterministic tool calls.

**Architecture:** Implement a small Python stdio JSON-RPC server under `tools/urpg_mcp/` without external runtime dependencies. The first tool surface exposes repo status, P2D authoring/runtime capabilities, release guardrails, and allowlisted focused gate commands with optional execution.

**Tech Stack:** Python standard library, unittest, JSON-RPC over stdio, CTest Python tool registration.

---

## Task 1: Local MCP Tool Contract

**Files:**
- Create: `tools/urpg_mcp/tests/test_server.py`
- Create: `tools/urpg_mcp/__init__.py`
- Create: `tools/urpg_mcp/server.py`
- Modify: `CMakeLists.txt`

- [x] **Step 1: Write failing tests**

Test that the server lists only local URPG tools, rejects unknown tools and unapproved gate ids, returns non-running focused-gate commands by default, and supports MCP initialize/tools/list/tools/call JSON-RPC requests.

- [x] **Step 2: Verify red**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected red: import failure because `tools.urpg_mcp.server` does not exist yet.

- [x] **Step 3: Implement minimal server**

Implement the tool registry, allowlisted focused gates, JSON-RPC dispatch, and stdio loop.

- [x] **Step 4: Verify green**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected green: all MCP tests pass.

## Task 2: IDE Setup And Docs

**Files:**
- Create: `tools/urpg_mcp/README.md`
- Create: `.urpg-mcp/mcp.server.sample.json`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] **Step 1: Add local IDE configuration sample**

Add a stdio MCP client snippet pointing at `python tools/urpg_mcp/server.py`.

- [x] **Step 2: Document guardrails**

Document local-only scope, allowlisted commands, no release-gate bypass, and how to extend the server.

- [x] **Step 3: Verify docs and test registration**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
git diff --check
```

Expected green: tests pass and no whitespace errors.

## Self-Review

- Spec coverage: The first MCP slice exposes safe IDE control over URPG status, P2D capabilities, focused gates, and guardrails. It intentionally does not bypass git, asset licensing, release gates, or arbitrary shell execution.
- Placeholder scan: No TBD/TODO placeholders are used.
- Type consistency: Tool names and gate ids are shared by tests, server, README, and sample config.
