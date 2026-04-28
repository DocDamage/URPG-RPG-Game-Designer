# Template Readiness Matrix

Status Date: 2026-04-28
Authority: canonical template readiness reference for product-facing template labels

This matrix answers a narrower question than the roadmap: which game templates are currently safe to describe as `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, or `PLANNED`.

Template support is not implied solely by subsystem composition. A template can only advance when both required subsystems and required cross-cutting bars are satisfied for that template.

## Status Values

| Status | Meaning |
| --- | --- |
| `READY` | Safe for release-facing template claims within the documented scope. |
| `PARTIAL` | Real support exists, but important required bars are still missing. |
| `EXPERIMENTAL` | A documented first-class template lane has early implementation or validation anchors, but it is not yet safe for release-facing product claims. |
| `BLOCKED` | Core prerequisites are missing or contradicted by current evidence. |
| `PLANNED` | Explicit roadmap intent exists, but no readiness-grade claim should be made yet. |

## Cross-Cutting Minimum Bars

These bars apply to template promotion and are tracked conservatively:

| Bar | Meaning |
| --- | --- |
| Accessibility | Exported UI/readability expectations are governed for the template. |
| Audio | Audio requirements and ownership are clear enough for the template's shipped scope. |
| Input | Required action/controller behavior is governed for the template. |
| Localization | Required localization completeness checks exist for the template scope. |
| Performance | Relevant runtime/presentation budgets are defined for the template scope. |

## Current Matrix

| Template | Status | Required Subsystems | Accessibility | Audio | Input | Localization | Performance | Safe Scope Today | Main Blockers |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `jrpg` | `PARTIAL` | `ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core` | Partial | Partial | Partial | Partial | Partial | Wave 1 native-first baseline with truthful residual gaps. | Cross-cutting bars are not complete. |
| `turn_based_rpg` | `PARTIAL` | `message_text_core`, `battle_core`, `save_data_core` | Partial | Partial | Partial | Partial | Partial | A conservative turn-based template slice similar to `jrpg` without implying broader productization. | Cross-cutting readiness bars and dedicated template governance remain incomplete. |
| `visual_novel` | `PARTIAL` | `message_text_core`, `save_data_core` | Partial | Partial | Partial | Partial | Partial | Dialogue-heavy projects within current native message and save scope. | Template-grade governance is not complete. |
| `tactics_rpg` | `READY` | `battle_core`, `presentation_runtime`, `save_data_core`, `accessibility_auditor`, `audio_mix_presets` | Ready | Ready | Ready | Ready | Ready | Grid tactics starter projects with scenario authoring, deployment zones, deterministic turn-order, cross-cutting bars, and save loop. | None for claimed template scope. |
| `arpg` | `READY` | `presentation_runtime`, `save_data_core`, `gameplay_ability_framework`, `input_runtime` | Ready | Ready | Ready | Ready | Ready | Action RPG starter projects with real-time combat states, stamina/dodge/attack actions, closure visibility cues, growth loop, and save loop. | None for claimed template scope. |
| `monster_collector_rpg` | `READY` | `ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core`, `gameplay_ability_framework` | Ready | Ready | Ready | Ready | Ready | Monster collector starter projects with species definitions, capture formula, party assembly, battle loop, and save loop. | None for claimed template scope. |
| `cozy_life_rpg` | `READY` | `ui_menu_core`, `message_text_core`, `save_data_core`, `crafting`, `economy`, `shop` | Ready | Ready | Ready | Ready | Ready | Cozy life starter projects with day phases, NPC routines, relationships, crafting, economy, vendors, and save loop. | None for claimed template scope. |
| `metroidvania_lite` | `READY` | `presentation_runtime`, `save_data_core`, `gameplay_ability_framework`, `map_runtime` | Ready | Ready | Ready | Ready | Ready | Metroidvania-lite starter projects with dash/wall-jump/double-jump traversal gates, region unlock data, map loop, and save loop. | None for claimed template scope. |
| `2_5d_rpg` | `READY` | `presentation_runtime`, `save_data_core`, `raycast_renderer`, `spatial_projection` | Ready | Ready | Ready | Ready | Ready | 2.5D RPG starter projects with raycast authoring adapter, blocking-cell validation, spatial navigation, export validation contract, and save loop. | None for claimed template scope. |

## Promotion Rules

A template may be promoted to `READY` only when:

1. All required subsystems are themselves `READY`.
2. Required editor or setup flows exist for the template scope.
3. The project audit can detect missing prerequisites and blockers for that template.
4. Canonical docs and template-facing docs agree on the template status.
5. The template satisfies its required cross-cutting minimum bars.

## Notes

- `PARTIAL` does not mean unsafe. It means the template is not yet eligible for full product-facing readiness language.
- `EXPERIMENTAL` should be used when a first-class template lane has early implementation anchors but not enough evidence for release-facing productization.
- `PLANNED` should be preferred over inflated implied readiness.
