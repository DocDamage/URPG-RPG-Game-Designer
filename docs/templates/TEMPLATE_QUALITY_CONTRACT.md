# Template Quality Contract

Status Date: 2026-05-01
Authority: canonical template quality-tier contract

This document defines template quality metadata. Quality tiers are separate from release readiness: a template can remain `READY` for its documented starter scope while still having future upgrade targets.

## Tiers

| Tier | Meaning |
| --- | --- |
| `starter_ready` | Bounded starter content is coherent, validated, and export-ready. |
| `showcase_ready` | Starter includes richer demo flow and family-specific WYSIWYG surfaces. |
| `production_seed` | Starter is suitable as a production seed with deeper balance, content, and diagnostics. |

## Required Manifest Fields

Every `content/templates/*_starter.json` manifest must declare `quality_contract` with:

| Field | Meaning |
| --- | --- |
| `tier` | Current quality tier. All current templates use `starter_ready`. |
| `quality_score` | Non-authoritative quality score used for upgrade tracking. Current floor is `70`. |
| `genre_family` | Shared family used for reusable defaults and future batch upgrades. |
| `target_camera` | Expected starter camera/navigation style. |
| `core_loops` | Starter loops proven by manifest/profile/certification evidence. |
| `starter_content` | Minimum starter content counts and WYSIWYG surface hints. |
| `required_inputs` | Common input actions expected in starter UX. |
| `default_accessibility` | Baseline accessibility expectations. |
| `audio_profile` | Starter audio mix profile. |
| `localization_namespaces` | Template-owned localization namespaces. |
| `performance_budget` | Starter frame, memory arena, and entity-count budget envelope. |
| `export_profile` | Export policy used by the starter template. |
| `known_limits` | Explicit non-claims that keep READY language honest. |

## Current Family Set

| Family | Used For |
| --- | --- |
| `party_rpg` | Party, quest, and general RPG starters. |
| `action_arcade` | Platforming, action, racing, rhythm, shooter, and arcade starters. |
| `narrative_adventure` | VN, adventure, mystery, dating, hidden-object, and choice-led starters. |
| `builder_management` | Builder, management, life-sim, idle, school, and economy-led starters. |
| `strategy_tactics` | Tactics, cards, board-game, tower-defense, and strategy starters. |
| `procedural_survival` | Roguelite, dungeon, survival, open-world, and generated-world starters. |
| `collection_progression` | Monster, gacha, and collection-driven starters. |

## Known Limits

All current `starter_ready` templates explicitly do not claim:

- Unbounded commercial content completeness.
- Platform-holder certification.
- Online services, multiplayer infrastructure, or live economy support.
- Final art/audio direction beyond governed starter placeholders.
- Full genre balance validation beyond the bounded starter loops.

## Upgrade Path

A template can move from `starter_ready` to `showcase_ready` when its starter manifest gains richer family-specific WYSIWYG surfaces, stronger sample content, and higher quality score evidence. A template can move to `production_seed` only after deeper balance/content diagnostics and more complete starter project data are governed by tests and docs.
