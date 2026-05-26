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

## Task 3: Bounded Project JSON Control

**Files:**
- Modify: `tools/urpg_mcp/tests/test_server.py`
- Modify: `tools/urpg_mcp/server.py`
- Modify: `tools/urpg_mcp/README.md`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] **Step 1: Write failing project-control tests**

Add tests for `urpg.project_summary` and `urpg.project_patch`. Summary must read only JSON files inside the repository. Patch must preview `set_startup_map` without writing by default, apply only with `apply=true`, and reject unknown patch kinds.

- [x] **Step 2: Verify red**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected red: `urpg.project_summary` and `urpg.project_patch` are unknown tools.

- [x] **Step 3: Implement bounded project JSON tools**

Add bounded path resolution, project JSON loading, project summary output, allowlisted `set_startup_map` JSON Patch preview, and explicit apply behavior.

- [x] **Step 4: Verify green**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected green: all MCP tests pass.

## Task 4: Robust Project Validation And Mutations

**Files:**
- Modify: `tools/urpg_mcp/tests/test_server.py`
- Modify: `tools/urpg_mcp/server.py`
- Modify: `tools/urpg_mcp/README.md`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] **Step 1: Write failing robust project-operation tests**

Add tests for `urpg.project_validate`, backup creation on apply, and allowlisted patch kinds `set_map_asset`, `add_p2d_map`, and `add_p2d_event`.

- [x] **Step 2: Verify red**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected red: `urpg.project_validate`, backup creation, and the new patch kinds are not implemented.

- [x] **Step 3: Implement validation, backups, and additional patch kinds**

Add startup/P2D reference validation, `.urpg_mcp_backup` writes before explicit apply, and named patch operations for map assets, P2D maps, and P2D events.

- [x] **Step 4: Verify green**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected green: all MCP tests pass.

## Task 5: Content Database And Asset Catalog Controls

**Files:**
- Modify: `tools/urpg_mcp/tests/test_server.py`
- Modify: `tools/urpg_mcp/server.py`
- Modify: `tools/urpg_mcp/README.md`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] **Step 1: Write failing content-control tests**

Add tests for project database patch kinds `add_actor`, `add_item`, `add_switch`, `add_variable`, `add_common_event`, and `add_asset_reference`. Add tests for read-only `urpg.asset_catalog_summary` and for startup asset reference validation against project assets.

- [x] **Step 2: Verify red**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected red: `urpg.asset_catalog_summary`, startup asset validation, and the new database patch kinds are not implemented.

- [x] **Step 3: Implement content controls**

Add read-only bounded asset catalog summaries, database record patch helpers, idempotent asset reference insertion, and startup asset validation while preserving explicit-apply backups and no arbitrary file writes.

- [x] **Step 4: Verify green**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected green: all MCP tests pass.

## Task 6: Structured P2D Authoring Controls

**Files:**
- Modify: `tools/urpg_mcp/tests/test_server.py`
- Modify: `tools/urpg_mcp/server.py`
- Modify: `tools/urpg_mcp/README.md`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] **Step 1: Write failing structured-authoring tests**

Add tests for allowlisted P2D tileset creation, tile metadata updates, P2D event command insertion, missing-event rejection, and unsupported P2D event command validation.

- [x] **Step 2: Verify red**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected red: `add_p2d_tileset`, `set_p2d_tile_metadata`, `add_p2d_event_command`, and P2D command validation are not implemented.

- [x] **Step 3: Implement structured P2D authoring controls**

Add named patch operations for tilesets, tile metadata, and event commands. Keep event command insertion constrained to the first-class P2D runtime command set, reject missing events, and preserve explicit-apply backups.

- [x] **Step 4: Verify green**

Run:

```powershell
python tools/urpg_mcp/tests/test_server.py
```

Expected green: all MCP tests pass.

## Self-Review

- Spec coverage: The MCP slices expose safe IDE control over URPG status, bounded project JSON summaries, startup/P2D/asset validation, explicit-apply project patching with backups, structured P2D map/event/tileset/tile-command authoring, project database records, read-only asset catalog summaries, P2D capabilities, focused gates, and guardrails. It intentionally does not bypass git, asset licensing, release gates, or arbitrary shell execution.
- Placeholder scan: No TBD/TODO placeholders are used.
- Type consistency: Tool names and gate ids are shared by tests, server, README, and sample config.
