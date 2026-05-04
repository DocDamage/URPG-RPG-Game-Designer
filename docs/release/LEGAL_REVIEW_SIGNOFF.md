# Legal Review Signoff

Status Date: 2026-05-04

This file records the legal/privacy/distribution review decision for the current URPG release candidate.

## Review Scope

| Field | Value |
| --- | --- |
| Repository | `C:\dev\URPG Maker` |
| Branch | `development` |
| Review walkthrough | `docs/release/LEGAL_HUMAN_REVIEW_WALKTHROUGH.md` |
| Readiness report | `docs/release/AAA_RELEASE_READINESS_REPORT.md` |
| App readiness matrix | `docs/APP_RELEASE_READINESS_MATRIX.md` |
| Package command | `.\tools\ci\run_release_candidate_gate.ps1` |

## Current Decision

Decision: `WAIVED_BY_RELEASE_OWNER`

Reviewer: `DocDamage`

Review date: `2026-04-30`

Reviewed commit: `7261576a0`

Distribution scope: `Public source, public binary prerelease, and public binary release at release-owner discretion`

Public release waiver: `RECORDED_BY_RELEASE_OWNER`

P5-02 evidence status: `OWNER_WAIVED_FOR_PUBLIC_RELEASE`

Decision notes:

- The release owner stated they read the legal/release files and accept responsibility for public distribution without qualified legal counsel approval.
- This is an explicit release-owner waiver, not qualified legal counsel approval for public binary distribution.
- The release owner stated that their shipped assets are paid/licensed for use in games and are usable in distributed games.
- The current release-required asset manifest bundles `BND-001` only for title, map, and battle placeholder coverage; `BND-002` UI SFX WAV payloads are deferred/local-only and ignored from GitHub packaging.
- The 2026-05-04 release asset gate records current visuals as bounded starter/proof assets, not final AAA art direction, through `releaseAssets.visualClaimScope`.
- The current release-required audio surface is intentionally silent/muted unless a future non-LFS bundled release-required audio asset is promoted.
- Raw/vendor/source asset packs remain excluded from release packages unless source-specific license and attribution evidence is reviewed before inclusion.
- Privacy policy text was checked against the editor analytics implementation: analytics starts disabled, requires explicit opt-in, and writes only to local JSONL export in the shipped editor entry point.
- Phase 7 analytics readiness covers engine-owned endpoint profile validation and editor diagnostics only. It is not qualified privacy/legal counsel approval, and public analytics policy/legal approval remains under this release-owner waiver.

## Required Review Checklist

- [x] Distribution type is recorded: internal-only, private beta, public source, public binary prerelease, or public binary release.
- [x] `LICENSE` is accepted for the intended private/internal RC distribution.
- [x] `EULA.md` is accepted by release-owner waiver for public distribution.
- [x] `PRIVACY_POLICY.md` is accepted by release-owner waiver for public distribution.
- [x] `THIRD_PARTY_NOTICES.md` covers shipped dependencies and runtime DLLs for engineering review scope.
- [x] Required third-party license texts or notice obligations are accepted by release-owner waiver for public distribution.
- [x] `CREDITS.md` is accepted by release-owner waiver for public distribution.
- [x] `CHANGELOG.md` wording is accepted by release-owner waiver for public distribution.
- [x] Package contents were reviewed against `docs/release/RELEASE_PACKAGING.md`.
- [x] Raw/vendor/source asset packs are confirmed excluded from the current package scope.
- [x] Release owner certifies paid/licensed shipped assets are usable in distributed games.
- [x] Promoted proof-lane asset scope is accepted for private/internal RC scope: `BND-001` visual proof is bundled; `BND-002` audio proof is deferred/local-only.
- [x] Bounded starter/proof visual scope is accepted for the current release-candidate package and is not represented as final AAA art direction.
- [x] Release-required UI/audio surfaces are covered by explicit silent/muted fallback policy entries instead of a bundled WAV dependency.
- [x] Repository-wide source/vendor LFS budget/access constraint is accepted as non-release-package scope for private/internal RC use.
- [x] Any required changes are listed below.

## Reviewer Decision

Use exactly one decision value:

- `APPROVED_FOR_PUBLIC_RELEASE`
- `APPROVED_FOR_PRIVATE_RC_ONLY`
- `REJECTED_CHANGES_REQUIRED`
- `WAIVED_BY_RELEASE_OWNER`

Recorded decision: `WAIVED_BY_RELEASE_OWNER`

Reviewer name: DocDamage

Reviewer role: Release owner

Reviewer contact: URPG Project support via GitHub Issues: https://github.com/DocDamage/URPG-RPG-Game-Designer/issues

Review date: 2026-04-30

Reviewed commit: `7261576a0`

Distribution scope: Public source, public binary prerelease, and public binary release at release-owner discretion

Required changes:

- Qualified legal/privacy/distribution approval has not been performed; public distribution proceeds only under this release-owner waiver.
- Public release contact/support routing is recorded as URPG Project GitHub Issues and remains release-owner
  responsibility to monitor.
- Public release should not ship raw/vendor/source asset packs unless source-specific license and attribution review approves them.
- If any audio asset becomes release-required, it must be hosted as a hydrated non-LFS release artifact or covered by a reviewed external artifact process before public distribution.

Approval or rejection notes:

- The release owner accepts the current legal/release files for public distribution and records an explicit waiver of qualified legal review.
- This decision does not replace qualified legal advice; it records owner acceptance of the risk and distribution responsibility.
- `EULA.md` has been reconciled with this waiver decision and no longer describes the current waiver-approved
  distribution scope as forbidden under obsolete internal-only text.

## Release Owner Follow-Up

After this signoff is completed:

- Update `docs/APP_RELEASE_READINESS_MATRIX.md`.
- Update `docs/release/AAA_RELEASE_READINESS_REPORT.md`.
- Run `.\tools\ci\run_release_candidate_gate.ps1`.
- Remote manual GitHub Actions release-candidate workflow recorded: run `25025111713` passed on `2026-04-27` at commit `7439132f4fa2638730498781f617d78af7b16514`: `https://github.com/DocDamage/URPG-RPG-Game-Designer/actions/runs/25025111713`.
- Create an annotated prerelease or release tag only after all release exits are closed or formally waived.
