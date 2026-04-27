# Roadmap Update Baseline: 2026-04-15

Historical provenance: this document preserves the April 15, 2026 roadmap/update baseline for Wave 2 audio and state-sync planning. Later notes in this file are explicitly dated annotations added to keep the baseline aligned with canonical docs after subsequent closure work.

Status note: Wave 2 audio/state-sync baseline work landed on 2026-04-16, and the compat-audio Phase 2 truth/verification closure landed on 2026-04-19 in the canonical remediation/status docs. This document is roadmap/reference material, not the canonical latest-status authority.

## Wave 1: Native Subsystem Absorption (Historical roadmap snapshot)

Snapshot note: the matrix below is the April 15 planning/status baseline captured for roadmap purposes. It should not be read as the current program-status authority.

| Subsystem | Runtime | Model/Inspector | Schema/Import | Tests |
| :--- | :---: | :---: | :---: | :---: |
| **Message/Text** | [x] | [x] | [x] | [x] |
| **Battle** | [x] | [x] | [ ] | [x] |
| **UI/Menu** | [x] | [x] | [x] | [x] |
| **Save/Data** | [x] | [x] | [x] | [x] |
| **Audio** | [x] | [x] | [ ] | [x] |

## Wave 2: Audio & State Sync (Landed baseline with later-lane follow-up)

### Objective
Provide a unified mechanism for synchronizing global game state (Switches, Variables, Inventory) with audio and scene systems, with later-lane hardening reserved for roadmap/remediation work beyond the closed Phase 2 compat-audio pass.

### 1. Global State Hub (State Sync)
- [x] Implement `engine/core/global_state_hub.h` as the source of truth for dynamic runtime state.
- [x] Support "Diff-First" state updates to trigger audio/UI events only when values change via subscribers.
- [ ] Integrate transaction-based state locks for multi-threaded scene updates in a later hardening lane.

### 2. Audio Orchestration (Wave 2 Expansion)
- [x] Expand `AudioCore` to support BGM/BGS playlists and priority-based SE ducking.
- [x] Implement `StateDrivenAudioResolver` to trigger BGM transitions based on map/battle state tags.
- [x] Add editor `AudioInspectorPanel` to monitor active handles and category volumes.
  - (2026-04-17) `audio_inspector_model` now projects live `AudioCore` active-source rows (asset id, category, volume, pitch, channel state) instead of count-only placeholder state.
  - (2026-04-19) Canonical Phase 2 docs now record compat-audio closure as harness-backed `PARTIAL`: deterministic playback position, duck/unduck ramps, mix scaling, and compat bindings are covered, while live-backend parity remains out of scope for this document.

### 3. Serialization Persistence
- [x] Connect `SaveSerializationHub` to the `GlobalStateHub` for atomic snapshots.
- [x] Implement differential saving (only store state deltas from project base).
- [x] Add unit tests for global state serialization/deserialization.

## Authority and follow-up

- For canonical latest status, use `docs/PROGRAM_COMPLETION_STATUS.md`.
- For canonical closure boundaries and residual compat-audio limitations, use `docs/PROGRAM_COMPLETION_STATUS.md`.
- Any remaining audio backend, threading, or release-hardening work belongs to later roadmap/remediation lanes, not to a reopened Phase 2 closure claim in this document.

---
Baseline roadmap snapshot authored on April 15, 2026. Later annotation notes were added during documentation truth-reconciliation to preserve provenance while pointing current status readers back to the canonical docs.
