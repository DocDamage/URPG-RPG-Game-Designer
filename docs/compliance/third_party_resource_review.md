# Third-Party Resource Review Checklist

Status: Draft checklist
Scope: External assets, tools, snippets, icons, audio, models, fonts, shaders, tutorials, and engine references

## Rule

No external resource becomes usable URPG project content until review proves that it is legally safe, technically useful, and product-fit.

Reference links may be cataloged before review. Actual files may not be imported or promoted until review is complete.

## Required Review Fields

- Resource ID
- Resource name
- Source URL
- Source repo or origin
- Original author or organization
- Resource type
- License name
- License URL or license file path
- Commercial-use permission
- Modification permission
- Redistribution permission
- Attribution requirement
- Trademark/logo restrictions
- AI/generated-content restrictions, if any
- Runtime dependency impact
- Editor/tooling dependency impact
- Style/product fit
- Reviewer
- Review date
- Decision
- Notes

## Decision Values

| Decision | Meaning |
| --- | --- |
| `reference_only` | May be cataloged or studied, but files/code/assets must not be copied. |
| `asset_source_candidate` | May be retained as a potential source, but each downstream asset still needs review. |
| `usable_asset` | May be imported into URPG or a URPG project with recorded license obligations. |
| `usable_tool_reference` | May be recommended or integrated as an external workflow/tool reference. |
| `rejected` | Must not be used except as historical audit context. |

## Minimum Bar for Usable Assets

A resource may be marked `usable_asset` only when all are true:

- License is identified and recorded.
- Commercial use is allowed.
- Modification rights are allowed or not needed.
- Redistribution rights are allowed for the intended URPG use.
- Attribution obligations are recorded.
- Any required notices can be included in export/package outputs.
- Style/product fit is documented.
- The resource does not force URPG to vendor a full unrelated engine.

## Automatic Blockers

Reject or keep reference-only when:

- License is missing and the resource would be copied into the repo.
- Commercial-use rights are unclear.
- Redistribution rights are unclear.
- The resource is an editor icon/sample asset from a third-party engine with unclear attribution.
- The resource would introduce a full external engine dependency.
- The resource is only useful because it is copyrighted/trademarked content from another game.
- Attribution obligations cannot be met in exported builds.

## Review Template

```text
Resource ID:
Resource name:
Source URL:
Source repo/origin:
Original author/org:
Resource type:
License name:
License URL/file:
Commercial use: yes/no/unknown
Modification allowed: yes/no/unknown
Redistribution allowed: yes/no/unknown
Attribution required: yes/no/unknown
Trademark/logo restrictions:
AI/generated-content restrictions:
Runtime dependency impact:
Editor/tooling dependency impact:
Style/product fit:
Reviewer:
Review date:
Decision: reference_only / asset_source_candidate / usable_asset / usable_tool_reference / rejected
Notes:
```

## Current Default Decisions

- `DocDamage/GameDev-Resources`: asset-source candidate index only.
- `DocDamage/defold`: reference-only engine/editor/toolchain study.
- `DocDamage/panda3d`: reference-only tooling/scene-graph study.
- `DocDamage/stride`: reference-only editor/content-pipeline study.
