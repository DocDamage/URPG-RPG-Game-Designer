# URPG MCP Completion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the local URPG MCP by adding narrow client-friendly tools, richer validation, safer backups, read-only inspection, gate readiness, stdio contract tests, and practical docs.

**Architecture:** Keep `tools/urpg_mcp/server.py` as the dependency-free stdio JSON-RPC server and retain `urpg.project_patch` for backwards compatibility. Add narrow tools that delegate to the same bounded project helpers, enrich patch previews with changed sections and summaries, and improve validation without introducing arbitrary shell or file-write surfaces.

**Tech Stack:** Python standard library, unittest, JSON-RPC over stdio, CTest Python tool registration.

---

## Task 1: Contract Tests

**Files:**
- Modify: `tools/urpg_mcp/tests/test_server.py`

- [x] Add failing tests for new tool names, manifest output, gate status, timestamped backups, restore, read-only list tools, richer validation diagnostics, patch summaries, and stdio subprocess JSON-RPC.
- [x] Run `python tools/urpg_mcp/tests/test_server.py` and confirm the new tests fail because tools/fields are missing.

## Task 2: Server Implementation

**Files:**
- Modify: `tools/urpg_mcp/server.py`

- [x] Add narrow tools while preserving `urpg.project_patch`: `urpg.mcp_manifest`, `urpg.gate_status`, `urpg.project_restore_backup`, `urpg.project_list_maps`, `urpg.project_list_events`, `urpg.project_list_assets`, `urpg.project_list_database`, `urpg.p2d_map_summary`, `urpg.project_set_startup`, `urpg.p2d_add_map`, `urpg.p2d_add_event`, `urpg.p2d_add_event_command`, `urpg.database_add_record`, `urpg.playable_add_transfer`, and `urpg.project_add_asset_reference`.
- [x] Add structural project diagnostics for duplicate ids, invalid project shapes, invalid P2D command fields, transfer coordinate types, tile page/passability/collision/priority bounds, and asset path existence when present.
- [x] Add patch summaries, changed sections, before/after changed subtrees, and timestamped backups with a backup manifest.
- [x] Add bounded backup restore and gate readiness checks.
- [x] Run `python tools/urpg_mcp/tests/test_server.py` and confirm green.

## Task 3: Documentation

**Files:**
- Modify: `tools/urpg_mcp/README.md`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] Document the narrow tools, richer validation, backup restore, gate status, and example JSON-RPC calls.
- [x] Run `git diff --check`.

## Task 4: Verification

**Files:**
- No additional files.

- [x] Run `python tools/urpg_mcp/tests/test_server.py`.
- [x] Run `ctest --test-dir build\dev-ninja-debug -R urpg_local_mcp_tool_test --output-on-failure`.
- [x] Run a live stdio `tools/list` smoke through `python tools/urpg_mcp/server.py`.
- [x] Run `git diff --check`.
