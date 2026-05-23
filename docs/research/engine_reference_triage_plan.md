# URPG Engine Reference and Resource Triage Plan

Status: Draft planning annex
Scope: Research, knowledge ingestion, asset-lead triage, editor UX reference, and anti-vendoring guardrails
Primary roadmap: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`

## Executive Decision

Use the referenced repositories as **controlled research inputs**, not as direct dependencies.

URPG may study the repos for architecture, editor UX, asset workflow, tooling, build structure, documentation, and knowledge-base seeds. URPG must not vendor or migrate to Defold, Panda3D, Stride, or any other full engine without a separate approved ADR.

## Asset Answer

Some of these sources can help the project find useful assets, but almost none of them should be treated as direct asset drops.

| Source | Asset Value | Use Decision |
| --- | --- | --- |
| `DocDamage/GameDev-Resources` | High as an asset-link index, not as an asset payload. It lists 2D assets, 3D assets, audio assets, material libraries, pixel tools, spritesheet tools, and tile/level editors. | Ingest as a license-gated resource catalog. Do not auto-import assets. |
| `DocDamage/defold` | Low for game assets. It is an engine/editor/toolchain repo and may contain editor/sample/platform assets. | Reference only. Do not use Defold sample/editor assets in URPG. |
| `DocDamage/panda3d` | Low/selective. It may contain sample models/textures for engine demos, but these are not aligned with URPG's JRPG/editor product direction. | Reference only unless a specific sample asset passes license and style review. |
| `DocDamage/stride` | Low for game assets; medium for editor-icon/UI workflow study. It contains third-party icon/license complexity. | Reference editor/content pipeline ideas. Do not copy icons/assets without explicit license review. |

## Hard Rules

1. No bulk import of external engine assets.
2. No vendor drop of Defold, Panda3D, or Stride.
3. No third-party art/audio/model/resource enters URPG until license, attribution, modification, redistribution, commercial-use, and style-fit are reviewed.
4. External resources can be indexed as links before they are approved as usable content.
5. `GameDev-Resources` should seed a catalog of possible sources, not populate the project with unreviewed files.
6. Engine/editor sample assets are not production game assets by default.
7. Any direct copy of code, icons, art, sounds, models, fonts, shaders, or sample content requires a tracked third-party review entry.

## Repository Triage Matrix

| Source | Useful Ideas | Rejected Use |
| --- | --- | --- |
| `GameDev-Resources` | Resource discovery, asset-source catalog, tool catalog, spritesheet tools, audio tools, pixel editors, tile/level editors, material/modeling references. | Treating linked assets as automatically usable. |
| `defold` | Clean engine/editor/CLI separation, build docs, release docs, platform export discipline, compact runtime/toolchain boundaries. | Porting URPG to Defold, depending on Defold, or copying Defold editor/runtime code. |
| `panda3d` | Python/C++ tooling ideas, scene graph concepts, import/export experiments, packaging/build reference. | Using Panda3D as URPG runtime or editor base. |
| `stride` | Visual editor UX, content pipeline concepts, asset browser patterns, project organization, modular rendering/editor design, release documentation. | Migrating URPG to Stride or copying Stride's editor/assets wholesale. |

## Main Roadmap Integration

This plan should be absorbed into the main roadmap as a controlled section:

```text
Research / External Reference / Knowledge-Base Ingestion
```

It should not become:

```text
Engine migration
Third-party engine dependency
Bulk asset import
Feature expansion mandate
```

## Main Plan Areas to Update

| Main Plan Area | Addition |
| --- | --- |
| Knowledge Database | Add external resource catalog seeded from `GameDev-Resources`. |
| Asset Pipeline | Add license-gated asset-lead ingestion and status tracking. |
| Editor UX | Use Stride/Defold as reference for file explorer, asset browser, preview panes, diagnostics, and export UI. |
| Offline Tooling | Use Panda3D only as light reference for Python-side experiments and conversion tools. |
| Compliance | Add third-party resource review before assets/code/tools are promoted to usable. |
| Architecture Governance | Add anti-vendoring ADR for full engine references. |

## Proposed URPG Files

```text
content/knowledge/game_dev_resources/resource_catalog.schema.json
content/knowledge/game_dev_resources/resource_catalog.seed.json
docs/research/game_dev_resource_ingestion.md
docs/research/engine_reference_triage_plan.md
docs/compliance/third_party_resource_review.md
docs/adr/ADR-000X-engine-reference-without-vendoring.md
tools/knowledge/validate_resource_catalog.py
tests/tools/test_validate_resource_catalog.py
```

## Resource Catalog Schema Direction

Every external entry should eventually normalize into a shape like this:

```json
{
  "id": "resource_slug",
  "name": "Resource Name",
  "source_url": "https://example.com",
  "source_repo": "DocDamage/GameDev-Resources",
  "category": "asset | tool | tutorial | engine | audio | spritesheet | tile_editor | reference",
  "cost": "free | paid | limited_free | unknown",
  "license_status": "verified | needs_review | rejected | unknown",
  "commercial_use": "yes | no | unknown",
  "modification_allowed": "yes | no | unknown",
  "redistribution_allowed": "yes | no | unknown",
  "attribution_required": "yes | no | unknown",
  "urpg_relevance": "high | medium | low | rejected",
  "approved_use": "knowledge_base | editor_reference | asset_source_candidate | usable_asset | rejected",
  "notes": "Short review note."
}
```

## Useful Asset Leads From GameDev-Resources

These should become catalog entries for review, not imported files:

- 2D asset libraries and RPG icon packs.
- Audio asset libraries and sound-effect sources.
- PBR/material libraries.
- Pixel-art editors and conversion tools.
- Spritesheet packers/slicers.
- Tile and level editors.
- Character generator references.
- Terrain/modeling/texturing tools.

## Defold Reference Extraction

Use Defold for:

- editor/runtime/CLI separation
- platform export documentation patterns
- build and release documentation patterns
- strict content pipeline thinking
- diagnostics around bundled dependencies

Do not use Defold for:

- URPG runtime base
- URPG editor base
- copied sample assets
- copied editor assets
- project-format support unless separately justified

## Stride Reference Extraction

Use Stride for:

- asset browser and content workflow ideas
- project browser/editor UX study
- content pipeline status reporting
- material/shader editor inspiration
- release/build documentation structure
- modular editor architecture ideas

Do not use Stride for:

- direct engine migration
- copied editor UI assets/icons
- copied runtime/editor systems
- wholesale C# architecture import

## Panda3D Reference Extraction

Use Panda3D for:

- Python-side tooling patterns
- asset conversion experiments
- scene graph reference ideas
- packaging/build script inspiration

Do not use Panda3D for:

- runtime migration
- editor base
- bulk samples/models/textures

## Implementation Phases

### Phase 1: Catalog Skeleton

Create schema and seed catalog entries for the four reviewed sources only.

Acceptance:

- Catalog validates.
- Every entry has a license status.
- No linked resource is marked usable without review.

### Phase 2: License Gate

Create a third-party review checklist for all external assets, code snippets, icons, audio, models, shaders, and tools.

Acceptance:

- `usable_asset` cannot be set without verified license metadata.
- Commercial-use ambiguity blocks inclusion.
- Reference-only links remain allowed.

### Phase 3: Editor UX Reference Notes

Extract non-code UX ideas from Stride and Defold.

Acceptance:

- Ideas map to URPG editor features.
- No copied code or assets.
- Accepted ideas list target URPG systems.

### Phase 4: File Explorer and Asset Preview Plan

Fold the best ideas into URPG's file explorer / asset preview work.

Feature targets:

- IDE-style file tree
- asset filters
- audio preview
- sprite atlas preview
- tilemap preview
- metadata inspector
- import status badges
- broken dependency display
- context actions for reimport/validate/reveal/copy ID

### Phase 5: ADR

Create an ADR that locks the anti-vendoring policy.

Decision:

- External engines may be studied.
- External resources may be cataloged.
- Similar workflows may be implemented in original URPG code.
- Full engines may not be vendored or depended on without separate approval.

## Final Recommendation

Use these repos to make URPG smarter, not bigger.

The asset value is mostly in `GameDev-Resources` as a curated lead list. Defold, Panda3D, and Stride are architecture/editor/tooling references. Their assets are not worth importing into the project unless a specific item later passes license, style, and product-fit review.
