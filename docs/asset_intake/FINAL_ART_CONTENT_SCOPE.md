# Final Art And Content Scope

> Product-scope resolution for final-quality art/content claims.

## Decision

The current bounded release now includes a curated final starter art slice:
`BND-005` contributes CC0/public-domain title, map, and battle assets from
`SRC-012`, and the broader promoted asset library is validated as governed
project-selectable content.

## Current Evidence

| Lane | Current State | Release Claim |
| --- | --- | --- |
| Starter actor/title/map/battle proof | `BND-001` promoted starter asset | Satisfies bounded starter visual coverage |
| Starter UI frame/chrome and VFX proof | `BND-003` promoted repo-generated assets | Satisfies bounded starter skin/VFX proof coverage |
| App icons | Repo resources verified by release asset gates | Satisfies app package icon coverage |
| UI/audio feedback | Explicit silent/muted fallback | Satisfies bounded release fallback policy |
| Curated environment/title/map art | `BND-005` promoted CC0/public-domain cavern background and tilesets | Satisfies curated final starter title/map coverage |
| Curated battle/map VFX | `BND-005` promoted CC0/public-domain hit effect sheet and torch frame | Satisfies curated final starter battle/map VFX coverage |
| Broad environment/prop/character/UI/VFX/audio library | `BND-006`, `BND-007`, `BND-008`, `BND-010`, and `BND-011` governed promoted-library inventory | Validated project-selectable content; exports still select exact assets |
| Cohesive final art direction | Bounded to release starter scope | Broader template-specific art direction requires project selection |

## Definition Of Done For Future Final-Art Claims

A template or product lane may claim final-quality art/content only when all are
true:

1. The exact selected assets are listed in a project or template manifest.
2. Every selected asset has source, bundle, attribution, checksum, license, and
   package-destination evidence.
3. `.\tools\ci\check_lfs_release_scope.ps1` classifies the payloads and reports
   no unknown LFS paths.
4. The target release commit has hydration evidence for every selected LFS
   payload.
5. The selected set has an art-direction note covering visual style, required
   surfaces, missing categories, and fallback behavior.
6. Runtime/editor/package tests cover the selected surfaces.

## Current Branch Claim

The current branch can say:

- release-required starter assets are governed and verified;
- `BND-005` provides curated final starter title/map/battle visual content;
- broad normalized LFS libraries are governed promoted-library inventory;
- broader template-specific final art direction requires exact project selection.

The current branch must not say:

- the repository is zero-LFS;
- every template has final-quality cohesive content;
- broad asset libraries are automatically shipped by default.

## Change Log

| Date | Change |
| --- | --- |
| 2026-05-27 | Added explicit final-art/content scope resolution tied to the LFS reconciliation gate. |
