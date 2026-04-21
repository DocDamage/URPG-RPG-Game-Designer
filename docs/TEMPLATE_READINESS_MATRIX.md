# Template Readiness Matrix

Status Date: 2026-04-20  
Authority: canonical template readiness reference for product-facing template labels

This matrix answers a narrower question than the roadmap: which game templates are currently safe to describe as `READY`, `PARTIAL`, `EXPERIMENTAL`, `BLOCKED`, or `PLANNED`.

Template support is not implied solely by subsystem composition. A template can only advance when both required subsystems and required cross-cutting bars are satisfied for that template.

## Status Values

| Status | Meaning |
| --- | --- |
| `READY` | Safe for release-facing template claims within the documented scope. |
| `PARTIAL` | Real support exists, but important required bars are still missing. |
| `EXPERIMENTAL` | Useful for bounded exploration or internal demos only. |
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
| `jrpg` | `PARTIAL` | `ui_menu_core`, `message_text_core`, `battle_core`, `save_data_core` | Planned | Partial | Partial | Planned | Partial | Native-first Wave 1 gameplay and authoring slices with truthful residual gaps. | Battle and save closure plus cross-cutting bars are not complete. |
| `turn_based_rpg` | `PARTIAL` | `message_text_core`, `battle_core`, `save_data_core` | Planned | Partial | Partial | Planned | Partial | Similar to `jrpg`, but without implying full template productization. | Same as JRPG plus no dedicated template governance row was previously enforced. |
| `visual_novel` | `PARTIAL` | `message_text_core`, `save_data_core` | Planned | Partial | Partial | Planned | Partial | Dialogue-heavy projects within today's native message and save scope. | Template-grade readiness rules and cross-cutting bars are still missing. |
| `tactics_rpg` | `EXPERIMENTAL` | `battle_core`, `presentation_runtime`, `save_data_core` | Planned | Planned | Partial | Planned | Partial | Exploration and roadmap-aligned experimentation. | Template-specific scenario authoring and release bars are missing. |
| `arpg` | `EXPERIMENTAL` | `presentation_runtime`, `save_data_core`, future action/combat lanes | Planned | Planned | Partial | Planned | Partial | Conceptual roadmap support only. | Runtime/editor closure visibility and cross-cutting bars are still missing. |
| `monster_collector_rpg` | `PLANNED` | future collection/party/template lanes | Planned | Planned | Planned | Planned | Planned | Roadmap intent only. | Template spec and required subsystems are still planned. |
| `cozy_life_rpg` | `PLANNED` | future schedule/sim/social/template lanes | Planned | Planned | Planned | Planned | Planned | Roadmap intent only. | Template spec and required subsystems are still planned. |
| `metroidvania_lite` | `PLANNED` | future traversal/progression/template lanes | Planned | Planned | Planned | Planned | Planned | Roadmap intent only. | Template spec and required subsystems are still planned. |
| `2_5d_rpg` | `PLANNED` | `presentation_runtime`, future template closure lanes | Planned | Planned | Planned | Planned | Planned | Optional 2.5D presentation seed exists, but no template-grade claim should be made yet. | Template spec and release bars are still planned. |

## Promotion Rules

A template may be promoted to `READY` only when:

1. All required subsystems are themselves `READY`.
2. Required editor or setup flows exist for the template scope.
3. The project audit can detect missing prerequisites and blockers for that template.
4. Canonical docs and template-facing docs agree on the template status.
5. The template satisfies its required cross-cutting minimum bars.

## Notes

- `PARTIAL` does not mean unsafe. It means the template is not yet eligible for full product-facing readiness language.
- `EXPERIMENTAL` should be used when the engine demonstrates a direction, not when it demonstrates full productization.
- `PLANNED` should be preferred over inflated implied readiness.
