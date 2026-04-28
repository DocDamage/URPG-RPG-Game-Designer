# URPG AI Copilot Integration Guide

This document describes how to leverage the AI Copilot infrastructure within the URPG engine.

## Overview

The URPG AI system is built on "Knowledge Bridges"—specialized classes that serialize complex engine states (Map, Audio, Debug, etc.) into natural language contexts for Large Language Models (LLMs). This allows the AI to "see" and "hear" the game world.

## Architectural Components

### 1. ChatbotComponent
The primary hub for AI interaction. It handles the `IChatService` communication and maintains conversation history.
- **File**: `engine/core/message/chatbot_component.h`
- **Capabilities**:
  - **Streaming Support**: Processes partial AI responses in real-time for immediate UI feedback.
  - **Tool Calling**: Executes native C++ functions (e.g., `GIVE_ITEM`, `SET_SWITCH`) requested by the AI.
  - **History Lifecycle**: Managed via `AISyncCoordinator`; the in-tree path is backed by `LocalInMemoryCloudService` and only exercises process-local memory synchronization unless a real `ICloudService` backend is provided out of tree.

### 2. IChatService & Connectivity
The in-tree runtime is interface-first:
- **IChatService**: Provider abstraction consumed by `ChatbotComponent`.
- **MockChatService**: Deterministic in-tree provider for CI/CD pipelines and harness scenarios.
- **Out-of-tree providers**: `engine/core/ai/ai_connectivity.h` now documents the boundary where live transport or inference integrations belong; no OpenAI-, Anthropic-, or llama.cpp-backed provider is implemented in-tree.

### 2a. Creator Command Planner
`CreatorCommandPlanner` turns a selected tile plus a creator prompt into a reviewable WYSIWYG edit plan. The shipped deterministic intents are:

- `make_house`
- `make_shop`
- `make_inn`
- `make_dungeon_room`
- `make_npc`
- `make_quest_giver`
- `make_treasure_chest`
- `make_locked_door`
- `make_puzzle`
- `make_farm_plot`

Plans include terrain/decor/collision tile edits, prop edits, runtime logic edits, diagnostics, and an apply-ready JSON contract. Building plans include passable doors and transfer logic. Interaction plans include the runtime logic they need, such as shop-open, party recovery, quest-state, item grants, locked-door checks, switches/variables, and crop planting.

The planner exposes provider profiles for hosted and local providers:

- Hosted: `chatgpt`, `gemini`, `kimi`, `anthropic`, `mistral`, `cohere`, `groq`, `perplexity`, `xai`, `deepseek`, `together`, `openrouter`, `azure_openai`, and `aws_bedrock`.
- Local: `ollama`, `lmstudio`, `llamacpp`, `vllm`, `text_generation_webui`, and `localai`.

Expected API-key environment variables are recorded in the profiles, including `OPENAI_API_KEY`, `GEMINI_API_KEY`, `KIMI_API_KEY`, `ANTHROPIC_API_KEY`, `MISTRAL_API_KEY`, `COHERE_API_KEY`, `GROQ_API_KEY`, `PERPLEXITY_API_KEY`, `XAI_API_KEY`, `DEEPSEEK_API_KEY`, `TOGETHER_API_KEY`, `OPENROUTER_API_KEY`, `AZURE_OPENAI_API_KEY`, and `AWS_BEDROCK_BEARER_TOKEN`. Local providers use localhost OpenAI-compatible endpoints and do not require API keys by default.

The in-tree transport can build and execute a `curl` request from project configuration through `CreatorProviderTransportConfig`. It writes the provider-specific request payload, posts it to the configured endpoint, and stores the raw provider response path for review/import. Secrets are supplied at runtime by project configuration or environment resolution, not hardcoded into the engine. The deterministic local planner remains available as a fallback so the editor can preview and test the feature without a live provider.

Provider responses are imported through `extractCreatorPlanJsonFromProviderResponse` and `parseCreatorCommandPlan`. The importer supports direct URPG plan JSON, OpenAI Responses `output_text`/`output`, OpenAI-compatible chat `choices`, Gemini `candidates.content.parts`, Anthropic `content`, and Cohere-style message content. Malformed or unsupported responses become non-applyable diagnostic plans instead of throwing.

Before any plan is applied, `validateCreatorCommandPlan` checks schema, apply readiness, map bounds, nonnegative tile ids, complete prop/logic records, and intent-specific runtime requirements. `applyCreatorCommandPlan` writes validated tile, prop, and logic edits into project JSON under the selected map and appends creator-command history for review. `CreatorCommandPanel` exposes the selected tile, provider payload, dry-run transport command, live preview metrics, validation diagnostics, apply preview, and last-apply result for WYSIWYG editor surfaces.

### 2b. App Knowledge and Tool Registry
The chatbot now has an in-tree knowledge foundation in `engine/core/ai/ai_knowledge_base.*`. It is deterministic and safe to run without a live model provider.

- `AppCapabilityRegistry` catalogs what the app can do across map authoring, event graphs, dialogue, abilities, battle VFX, save labs, export preview, assets, templates, and creator commands.
- `ProjectKnowledgeIndex` summarizes supplied project JSON into searchable project entries for maps, events, dialogue, abilities, assets, templates, localization, and export settings.
- `DocumentationKnowledgeIndex` exposes canonical docs such as the agent index, architecture map, quality gates, AI Copilot guide, and release readiness matrix.
- `AiToolRegistry` defines safe callable tools across maps, regions, lighting/weather, events, dialogue, localization, quests, NPC schedules, abilities, battle VFX, save labs, assets, templates, export preview, validation, and creator commands.
- `AiTaskPlanner` turns user requests into reviewable `urpg.ai_task_plan.v1` tool plans.
- `applyApprovedPlan` refuses mutating steps until they are approved, then writes the approved map/event/dialogue/ability/export/creator-command changes into project JSON and records `ai_tool_applications`.
- `approvalManifest` lists pending approval steps and the project paths they can touch.

`AiAssistantPanel` includes a knowledge snapshot and current task plan in its deterministic render snapshot, giving the WYSIWYG editor a stable surface for showing what the chatbot knows, what tools it wants to call, and what will be changed before anything is applied.

The mutating tools that require approval are `create_map`, `place_tile`, `paint_region`, `configure_environment`, `add_event`, `edit_dialogue`, `add_localization_entry`, `add_quest`, `set_npc_schedule`, `add_ability`, `add_vfx_keyframe`, `configure_save_preview`, `import_asset_record`, `create_template_project`, and `plan_creator_command`. `run_validation` and `run_export_preview` are non-mutating queue/preview tools and do not require approval.

Editor approval is handled by `AiAssistantPanel::approveStep(stepId)` or `AiAssistantPanel::approveAllPendingSteps()`. After approval, `AiAssistantPanel::applyApprovedPlan()` applies the validated tool plan to project JSON and records the result in the panel snapshot under `last_apply`.

`ChatbotComponent` is wired to the same tool registry through explicit tool commands:

- `AI_TASK:<creator request>` builds a reviewable `urpg.ai_task_plan.v1`.
- `AI_APPROVE_STEP:<step id>` approves one pending step.
- `AI_REJECT_STEP:<step id>` rejects one pending step.
- `AI_APPROVE_ALL` approves all pending mutating steps.
- `AI_APPLY` applies the approved plan to project JSON.

The chatbot exposes the current `task_plan`, `approval` manifest, and `last_apply` result through `lastAiToolSnapshot()`.

### 3. Knowledge Bridges
- **WorldKnowledgeBridge**: Serializes NPC locations, item names, and plot flags into a "World Context" digest.
- **BattleKnowledgeBridge**: Provides tactical context (HP, Mana, Elemental weaknesses) for real-time combat advice.
- **AudioKnowledgeBridge**: Maps scene moods and current BGM to AI-friendly metadata for orchestration.
- **AnimationKnowledgeBridge**: Translates natural language descriptions (e.g., "Take a bow") into `[KEYFRAME]` sequences.
- **DebugKnowledgeBridge**: Dumps call-stacks, watch-tables, and memory snapshots for conversational debugging.

### 4. Personality Registry
Standardized NPC archetypes stored in `engine/core/ai/personality_registry.h`:
- **Elder**: Wise, cryptic, uses archaic language.
- **Warrior**: Blunt, tactical, impatient.
- **Rogue**: Witty, cynical, profit-focused.
- **Healer**: Compassionate, soft-spoken, peace-oriented.

---

## Audio Orchestration Example

The AI can dynamically change background music based on the narrative.

**Command Format:**
`[ACTION: CROSSFADE, ASSET: Boss_Dark, VOL: 0.8, FADE: 3.0]`

---

## Technical Features

### Multi-Line Text Wrapping
The `ChatWindow` component supports native word-wrapping via `urpg::ui::ChatWindow::draw`. This ensures that long AI responses are legible on all display resolutions.

### Pattern-Based Command Parsing
The engine uses high-performance regex validaton (tested in `tests/unit/test_ai_bridge_regex.cpp`) to extract multi-subsystem commands from a single AI message.
Example Response:
> "Be careful in the dungeon! [ACTION: CROSSFADE, ASSET: Dark_Ambient, VOL: 0.4, FADE: 2.0] I've spotted patterns on the wall. [KEYFRAME: 1.0, POS: 0:5:0]"

In this case, the engine simultaneously:
1. Displays the warning text to the player.
2. Crossfades the background music.
3. Triggers a physical movement on the player sprite.

### Incremental Save Optimization
AI histories are saved using `SaveSerializationHub::CompressionLevel::Optimal`, which trims repetitive system contexts to preserve disk space and future transport bandwidth.

---

## Best Practices

1. **Structured Prompts**: Always use the `generatePrompt` methods in the Bridges to ensure the LLM receives the data in the optimized format.
2. **Hidden Commands**: You can instruct the AI to include orchestration commands (like Audio or Animation) hidden in its response; the engine will parse them out before displaying the text to the player.
3. **Cloud Sync**: Treat `AISyncCoordinator` as cloud-sync plumbing only. In the current tree it routes through `LocalInMemoryCloudService`, which preserves data only in process-local memory for tests and harness scenarios and is not an operational cross-device sync path.

## Debugging

To debug AI context, use the `DebugKnowledgeBridge` to dump what the AI "sees" to the console or log file.
