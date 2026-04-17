# Wave 2 AI Subsystem Roadmap & Closure Checklist

This document tracks the detailed implementation status of the AI Copilot infrastructure (Wave 2 Advanced Capability).

## Subsystem Capabilities

### 1. Unified Knowledge Hub
- [x] **State Serialization**: [WorldKnowledgeBridge](engine/core/message/world_knowledge_bridge.h) correctly snapshots Switches, Variables, and Map metadata.
- [x] **Context Freshness**: `ChatbotComponent` forced to refresh world context on every GET request.
- [x] **Cross-Domain Presence**: AI aware of Battle (Tactics), Audio (Mood), and Animation (Physicality).

### 2. Connectivity & Infrastructure
- [x] **IChatService Interface**: Abstracted base for all AI providers.
- [x] **Streaming Architecture**: End-to-end support for token-by-token processing.
- [x] **Function/Tool Calling**: Logic for AI-triggered engine commands (`GIVE_ITEM`, `SET_SWITCH`).
- [x] **Offline Support**: Skeletal Llama.cpp provider for edge inference.

### 3. Native UI (ChatWindow)
- [x] **Dynamic Layout**: Word-wrapping for varying window widths.
- [x] **Visual Feedback**: Real-time text append for streaming chunks.
- [x] **Command Stripping**: Logic to parse but hide `[CMD]` tags from player-visible text.

### 4. Persistence & Security
- [x] **Differential Snapshots**: History serialization avoids redundant system prompt blobs.
- [x] **Encrypted Sync Plumbing**: `AISyncCoordinator` is wired for `ICloudService` integration, but the in-tree path still depends on an in-memory `CloudServiceStub` rather than a live cloud backend.

## Closure Checklist (Production Readiness)

| Requirement | Evidence | Status |
| :--- | :--- | :---: |
| **Command Robustness** | `tests/unit/test_ai_bridge_regex.cpp` covers whitespace/commentary variants. | [x] |
| **BGM Performance** | AI-driven crossfades verified in `MapScene` update loop. | [x] |
| **Memory Management** | Chat history limits and trimming implemented in `ChatbotComponent`. | [x] |
| **API Abstraction** | Model-agnostic request format supports GPT-4o, Claude 3.5, and Local models. | [x] |

## Future Expansion
- [ ] **Image Generation Bridge**: Natural language to sprite/icon generation hooks.
- [ ] **Level Design Copilot**: In-editor AI for generating procedural dungeon layouts.
- [ ] **Automated QC**: AI-driven "smoke testing" that plays the game to find collision bugs.
