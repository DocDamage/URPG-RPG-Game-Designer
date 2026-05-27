# Final Art And Content Scope

> Product-scope resolution for final-quality art/content claims.

## Decision

The current bounded release claim does **not** require final cohesive art direction.
It requires governed starter visuals, explicit fallbacks, release-required asset
hydration, attribution, and package gates.

Final-quality art/content is now tracked as a project-selection and art-direction
lane, not as unfinished engine code.

## Current Evidence

| Lane | Current State | Release Claim |
| --- | --- | --- |
| Starter actor/title/map/battle proof | `BND-001` promoted starter asset | Satisfies bounded starter visual coverage |
| Starter UI frame/chrome and VFX proof | `BND-003` promoted repo-generated assets | Satisfies bounded starter skin/VFX proof coverage |
| App icons | Repo resources verified by release asset gates | Satisfies app package icon coverage |
| UI/audio feedback | Explicit silent/muted fallback | Satisfies bounded release fallback policy |
| Broad environment/prop/character/UI/VFX/audio library | `BND-006`, `BND-007`, `BND-008`, `BND-010`, and `BND-011` deferred curated-library inventory | Not release-required; eligible only when a project selects and hydrates exact assets |
| Cohesive final art direction | Not selected for the bounded release | Must not be claimed as complete without a project art-direction pass |

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
- broad normalized LFS libraries are reconciled as deferred curated inventory;
- final cohesive art/content is outside the bounded app release claim unless a
  future project selection promotes exact assets.

The current branch must not say:

- the repository is zero-LFS;
- deferred library inventory is final art;
- every template has final-quality cohesive content;
- broad asset libraries are automatically shipped by default.

## Change Log

| Date | Change |
| --- | --- |
| 2026-05-27 | Added explicit final-art/content scope resolution tied to the LFS reconciliation gate. |
