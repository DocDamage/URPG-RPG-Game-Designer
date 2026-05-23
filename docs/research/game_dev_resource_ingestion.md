# Game Development Resource Ingestion Workflow

Status: Draft policy
Owner: Asset intake / knowledge tooling
Related roadmap: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
Related triage: `docs/research/engine_reference_triage_plan.md`

## Purpose

URPG can use external game-development resource indexes to discover asset sources, tools, tutorials, editor references, and engine architecture ideas without importing unreviewed assets or code into the project.

The initial controlled seed sources are:

- `DocDamage/GameDev-Resources`
- `DocDamage/defold`
- `DocDamage/panda3d`
- `DocDamage/stride`

## Boundary

Resource ingestion means creating reviewable catalog entries. It does not mean copying files into URPG.

Allowed at this stage:

- Link cataloging
- Source categorization
- License-status tracking
- URPG relevance notes
- Editor-reference notes
- Asset-source candidate tracking

Not allowed at this stage:

- Bulk asset import
- Vendoring external engines
- Copying editor icons, sample textures, models, sounds, fonts, shaders, or code
- Treating a link-list entry as approved content
- Marking any external asset as usable without verified license metadata

## Catalog Files

```text
content/knowledge/game_dev_resources/resource_catalog.schema.json
content/knowledge/game_dev_resources/resource_catalog.seed.json
tools/knowledge/validate_resource_catalog.py
tools/knowledge/tests/test_resource_catalog.py
```

## Required Review States

Every entry must carry explicit review metadata:

- `license_status`
- `commercial_use`
- `modification_allowed`
- `redistribution_allowed`
- `attribution_required`
- `urpg_relevance`
- `approved_use`

Unknown is allowed only while a resource remains a reference or candidate. Unknown is not allowed for a usable asset.

## Approved-Use Meanings

| Value | Meaning |
| --- | --- |
| `knowledge_base` | Safe to keep as a searchable external reference. |
| `editor_reference` | Safe to study for UX, pipeline, docs, or architecture ideas. |
| `asset_source_candidate` | Candidate source for future asset review; no files are approved yet. |
| `usable_asset` | Asset is cleared for project use after license/product/style review. |
| `rejected` | Do not use except as historical audit context. |

## Promotion Rules

An entry may move from `asset_source_candidate` to `usable_asset` only when all of the following are true:

- `license_status` is `verified`
- commercial use is explicitly allowed
- modification rights are known
- redistribution rights are known
- attribution requirements are recorded
- style/product fit is documented
- source URL and author/origin are preserved
- the resource has a tracked reviewer/date in the companion review record

## Initial Source Decisions

| Source | Initial approved use | Reason |
| --- | --- | --- |
| `DocDamage/GameDev-Resources` | `asset_source_candidate` | Useful index of possible assets/tools, but links require individual review. |
| `DocDamage/defold` | `editor_reference` | Useful for engine/editor/CLI separation and export/release docs. |
| `DocDamage/panda3d` | `editor_reference` | Useful for Python/C++ tooling and packaging concepts. |
| `DocDamage/stride` | `editor_reference` | Useful for editor UX, content pipeline, and project-browser patterns. |

## Validation

Run:

```bash
python tools/knowledge/validate_resource_catalog.py
python tools/knowledge/tests/test_resource_catalog.py
```

The validator fails when:

- required fields are missing
- unknown fields are present
- duplicate IDs exist
- enum values are invalid
- `usable_asset` is used without `license_status=verified`
- rejected license or relevance states are not paired with `approved_use=rejected`

## Next Step

After this skeleton lands, expand `resource_catalog.seed.json` only with reviewed source entries. Do not mass-import every `GameDev-Resources` link until the validator and review process are in place.
