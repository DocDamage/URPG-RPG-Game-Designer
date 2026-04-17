# Roadmap Update: 2026-04-15

## Wave 1: Native Subsystem Absorption (Status)

| Subsystem | Runtime | Model/Inspector | Schema/Import | Tests |
| :--- | :---: | :---: | :---: | :---: |
| **Message/Text** | [x] | [x] | [x] | [x] |
| **Battle** | [x] | [x] | [ ] | [x] |
| **UI/Menu** | [x] | [x] | [x] | [x] |
| **Save/Data** | [x] | [x] | [x] | [x] |
| **Audio** | [x] | [x] | [ ] | [x] |

## Wave 2: Audio & State Sync (Planned)

### Objective
Provide a unified, thread-safe mechanism for synchronizing global game state (Switches, Variables, Inventory) with the Audio Mixer and Scene Graph.

### 1. Global State Hub (State Sync)
- [x] Implement `engine/core/global_state_hub.h` as the source of truth for dynamic runtime state.
- [x] Support "Diff-First" state updates to trigger audio/UI events only when values change via subscribers.
- [ ] Integrate transaction-based state locks for multi-threaded scene updates.

### 2. Audio Orchestration (Wave 2 Expansion)
- [x] Expand `AudioCore` to support BGM/BGS playlists and priority-based SE ducking.
- [x] Implement `StateDrivenAudioResolver` to trigger BGM transitions based on map/battle state tags.
- [x] Add editor `AudioInspectorPanel` to monitor active handles and category volumes.
  - (2026-04-17) `audio_inspector_model` now projects live `AudioCore` active-source rows (asset id, category, volume, pitch, channel state) instead of count-only placeholder state.

### 3. Serialization Persistence
- [x] Connect `SaveSerializationHub` to the `GlobalStateHub` for atomic snapshots.
- [x] Implement differential saving (only store state deltas from project base).
- [x] Add unit tests for global state serialization/deserialization.

---
*Last update by GitHub Copilot on April 15, 2026.*
