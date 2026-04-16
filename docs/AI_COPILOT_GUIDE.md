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
  - **History Lifecycle**: Managed via `AISyncCoordinator` for multi-device persistence.

### 2. IChatService & Connectivity
Engineers can plug in different AI providers using the specialized service classes in `engine/core/ai/ai_connectivity.h`:
- **OpenAIChatService**: Integration for GPT-4 series models via HTTP/SSE.
- **LlamaLocalService**: Offline inference support using the `llama.cpp` runtime.
- **MockChatService**: Deterministic testing provider for CI/CD pipelines.

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
AI histories are saved using `SaveSerializationHub::CompressionLevel::Optimal`, which trims repetitive system contexts to preserve disk space and cloud bandwidth.

---

## Best Practices

1. **Structured Prompts**: Always use the `generatePrompt` methods in the Bridges to ensure the LLM receives the data in the optimized format.
2. **Hidden Commands**: You can instruct the AI to include orchestration commands (like Audio or Animation) hidden in its response; the engine will parse them out before displaying the text to the player.
3. **Cloud Sync**: Use `AISyncCoordinator` to ensure that conversation history is preserved across devices via the `ICloudService` interface.

## Debugging

To debug AI context, use the `DebugKnowledgeBridge` to dump what the AI "sees" to the console or log file.
