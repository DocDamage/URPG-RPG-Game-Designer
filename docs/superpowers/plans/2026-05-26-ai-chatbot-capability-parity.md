# AI Chatbot Capability Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make release editor workflows actionable or explicitly readonly through the chatbot while making all non-release panels discoverable without enabling unsafe mutation.

**Architecture:** Extend the existing AI knowledge registry with access metadata and panel ownership rather than creating a second capability catalog. Use the editor panel registry as the source for shipped versus non-release exposure, add readonly panel tools, and harden the WYSIWYG chatbot coverage report.

**Tech Stack:** C++20, nlohmann/json, existing editor panel registry, Catch2 tests, CMake/CTest.

---

## File Structure

- Modify `engine/core/ai/ai_knowledge_base.h`: add metadata fields to `AppCapability` and `AiToolDefinition`.
- Modify `engine/core/ai/ai_knowledge_base.cpp`: serialize metadata, index all panels, auto-register panel capabilities, add readonly tools, planner routing, and readonly apply previews.
- Modify `engine/core/ai/wysiwyg_chatbot_coverage.h`: add coverage counters for panel discoverability, release coverage, non-release coverage, and tool approval safety.
- Modify `engine/core/ai/wysiwyg_chatbot_coverage.cpp`: compute the expanded coverage report from the editor panel registry, capability registry, docs index, and tool registry.
- Modify `tests/unit/test_ai_knowledge_base.cpp`: add focused assertions for parity metadata, panel discoverability, readonly panel tools, and approval safety.
- Modify `docs/integrations/AI_COPILOT_GUIDE.md`: document release actionable versus dev discoverable chatbot behavior.

## Task 1: Metadata Contract

**Files:**
- Modify: `engine/core/ai/ai_knowledge_base.h`
- Modify: `engine/core/ai/ai_knowledge_base.cpp`
- Test: `tests/unit/test_ai_knowledge_base.cpp`

- [x] Add failing tests that inspect default capabilities/tools and require `access_level`, `editor_exposure`, `panel_id`, and `promotion_gate` metadata in JSON.
- [x] Add the metadata fields to `AppCapability` and `AiToolDefinition`.
- [x] Update `toJson()` for both structs.
- [x] Run `ctest --preset dev-all -R "AI (knowledge|task|tool|assistant)|Chatbot component" --output-on-failure`.

## Task 2: All-Panel Knowledge Indexing

**Files:**
- Modify: `engine/core/ai/ai_knowledge_base.cpp`
- Test: `tests/unit/test_ai_knowledge_base.cpp`

- [x] Add failing tests proving every `editorPanelRegistry()` entry is searchable in `DocumentationKnowledgeIndex`.
- [x] Change `DocumentationKnowledgeIndex::buildDefault()` to index release, nested, dev-only, and deferred panels.
- [x] Include metadata for `panel_id`, `category`, `owner`, `exposure`, `access_level`, and `promotion_gate`.
- [x] Run the focused AI/chatbot CTest lane.

## Task 3: Panel Capability Coverage

**Files:**
- Modify: `engine/core/ai/ai_knowledge_base.cpp`
- Test: `tests/unit/test_ai_knowledge_base.cpp`

- [x] Add failing tests proving every release top-level panel has either actionable or readonly capability coverage, and every non-release panel has discoverable coverage.
- [x] Register explicit release capabilities with panel ids and access levels.
- [x] Auto-register readonly/discoverable panel capabilities for uncovered editor panel entries.
- [x] Run the focused AI/chatbot CTest lane.

## Task 4: Readonly Panel Tools

**Files:**
- Modify: `engine/core/ai/ai_knowledge_base.cpp`
- Test: `tests/unit/test_ai_knowledge_base.cpp`

- [x] Add failing tests for `describe_panel`, `list_panel_actions`, and `route_to_panel` tools.
- [x] Register the three tools as non-mutating and not approval-required.
- [x] Teach `AiTaskPlanner` to plan describe/list/route requests.
- [x] Teach `AiToolRegistry::applyApprovedPlan()` to emit `ai_tool_previews` records for those tools.
- [x] Run the focused AI/chatbot CTest lane.

## Task 5: Coverage Report Hardening

**Files:**
- Modify: `engine/core/ai/wysiwyg_chatbot_coverage.h`
- Modify: `engine/core/ai/wysiwyg_chatbot_coverage.cpp`
- Test: `tests/unit/test_ai_knowledge_base.cpp`

- [x] Add failing tests for expanded coverage counters and mutating-tool approval safety.
- [x] Report all-panel counts, release coverage counts, non-release discoverability counts, and unsafe mutating tool count.
- [x] Keep existing report fields compatible.
- [x] Run the focused AI/chatbot CTest lane.

## Task 6: Chatbot Snapshot and Docs

**Files:**
- Modify: `tests/unit/test_ai_knowledge_base.cpp`
- Modify: `docs/integrations/AI_COPILOT_GUIDE.md`

- [x] Add a chatbot command-flow test for a panel-routing request.
- [x] Document release actionable, release readonly, and dev discoverable behavior.
- [x] Run `git diff --check`.
- [x] Run `ctest --preset dev-all -R "AI (knowledge|task|tool|assistant)|Chatbot component" --output-on-failure`.
