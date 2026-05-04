# URPG End User License Agreement

Status Date: 2026-05-04

This document records the current URPG binary-use terms accepted by release-owner waiver in
`docs/release/LEGAL_REVIEW_SIGNOFF.md`. It is not qualified legal counsel approval. Public binary prerelease or release
distribution remains a release-owner decision and responsibility under that waiver.

## Current Distribution Status

URPG source-code use remains governed by the project `LICENSE`. Native app packages produced from this repository may be
used for internal testing, private release-candidate validation, public binary prerelease, or public binary release only
within the distribution scope recorded in `docs/release/LEGAL_REVIEW_SIGNOFF.md`.

## License Grant

Within the recorded release-owner-waived distribution scope, you may:

- install and run URPG binaries for development, testing, evaluation, and RPG project authoring;
- use the editor and runtime to create, validate, and package projects that rely on URPG-owned schemas, templates,
  manifests, and sample project data;
- distribute your own project content separately, subject to the licenses and permissions for assets, plugins, code, and
  data that you include.

## Restrictions

You may not:

- redistribute private builds outside the distribution scope recorded by the release owner;
- redistribute third-party asset packs, RPG Maker DLC content, imported raw assets, source/vendor intake material, or
  toolchain components except where their own licenses and the release package notices allow it;
- remove third-party notices, license files, provenance manifests, privacy policy text, credits, or release-readiness
  warnings from a package;
- treat deferred local proof assets, including ignored WAV files under `imports/normalized/ui_sfx/`, as public release
  content unless a later release review explicitly promotes them into the release-required asset set;
- use URPG in a way that violates applicable third-party licenses or platform distribution rules.

## User-Created Projects

You retain ownership of project content you author, subject to any third-party assets, plugins, code, fonts, audio, or
other material you choose to include. URPG does not grant rights to content that URPG does not own or have permission to
redistribute.

## Third-Party Content

Third-party code, assets, plugins, and toolchain runtimes remain subject to their own licenses. Repository intake and
reference material is not approved for redistribution merely because it exists in the working tree. Release package
contents and notices are tracked in `THIRD_PARTY_NOTICES.md`, `CREDITS.md`, and the governed asset manifests.

## Privacy

Privacy terms are recorded in `PRIVACY_POLICY.md`. The current editor analytics path is opt-in and local-export based
unless a reviewed endpoint profile is configured.

## Warranty And Liability

URPG is provided as-is, without warranties of merchantability, fitness for a particular purpose, non-infringement, or
uninterrupted operation. To the maximum extent allowed by applicable law, URPG contributors and release owners are not
liable for indirect, incidental, special, consequential, or punitive damages arising from use or distribution of URPG.

## Termination

Rights granted under this document end if you violate these terms or applicable third-party license obligations. On
termination, you must stop using and redistributing affected URPG binaries and package contents.

## Review Status

Qualified legal/privacy/distribution review has not been performed. The current public-distribution path is explicitly
release-owner-waived, not counsel-approved. Any future package that adds release-required third-party assets, remote
analytics defaults, store integrations, signing/notarization requirements, or materially different redistribution terms
must update the legal signoff before release.
