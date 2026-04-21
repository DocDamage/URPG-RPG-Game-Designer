# 2.5D RPG Template Spec

Status Date: 2026-04-20  
Authority: canonical template spec for `2_5d_rpg`

## Purpose

The `2_5d_rpg` template covers games that use a raycast-based or faux-3D presentation mode layered over 2D RPG mechanics. It depends on the optional 2.5D presentation lane and is intended for projects that want depth-cue exploration without a full 3D engine.

## Required Subsystems

| Subsystem | Rationale |
| --- | --- |
| `presentation_runtime` | Raycast rendering, camera, and spatial updates for the 2.5D mode. |
| `save_data_core` | Persistent progression, settings, and session recovery. |
| `2_5d_mode` *(optional / future)* | Raycast art pipeline, depth-cue rendering, and mode-specific camera contracts. |

## Cross-Cutting Minimum Bars

| Bar | Status | Notes |
| --- | --- | --- |
| Accessibility | `PLANNED` | No template-grade accessibility governance yet. |
| Audio | `PLANNED` | 2.5D-specific spatial audio requirements are undefined. |
| Input | `PARTIAL` | Basic input exists; 2.5D-specific camera and movement contracts are not landed. |
| Localization | `PLANNED` | Planned once message localization patterns are finalized. |
| Performance | `PARTIAL` | General budgets exist; raycast rendering performance at production scale is untested. |

## Safe Scope Today

Raycast-mode exploration demos using the optional 2.5D lane within `presentation_runtime`. Bounded technical demonstrations that leverage landed presentation surfaces for faux-3D exploration are safe, provided they do not claim production-grade map authoring or export validation.

## Main Blockers

1. **Raycast art pipeline** — Canonical pipeline for authoring and importing raycast-compatible art assets is not established.
2. **Map authoring adapters at production grade** — Editor workflows for 2.5D map construction are not landed.
3. **Template-specific export validation** — Export-time checks for 2.5D mode correctness and performance are undefined.

## Promotion Path

This template may advance to `PARTIAL` once:

- `presentation_runtime` reaches `READY` or `PARTIAL` with 2.5D-relevant evidence.
- A 2.5D authoring or export lane reaches `PARTIAL` with canonical evidence.

It may advance to `READY` only when all required subsystems are `READY`, all cross-cutting bars are at least `PARTIAL`, and 2.5D-specific editor and export flows exist.
