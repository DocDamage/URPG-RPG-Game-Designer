# Legal And Human Release Review Walkthrough

Status Date: 2026-04-27

This walkthrough is an engineering-prepared review aid. It is not legal advice and does not approve release. A qualified reviewer or release owner must record the final decision in `docs/release/LEGAL_REVIEW_SIGNOFF.md`.

## Review Goal

Decide whether the current URPG release candidate can be distributed publicly under the current legal, privacy, license, credit, and packaging documents.

The current engineering position is conservative:

- Local release-candidate validation passes.
- Release-required assets are verified from a fresh GitHub clone.
- Raw source/vendor asset packs are not part of the current install/package path.
- Legal sufficiency is still unverified until a qualified reviewer approves or rejects it.

## Files To Review

| File | Review Purpose | Required Decision |
| --- | --- | --- |
| `LICENSE` | Project source/license grant. | Confirm the project license is intended for public source and/or package distribution. |
| `EULA.md` | App binary end-user terms. | Approve as production text, replace with production EULA, or keep internal-only. |
| `PRIVACY_POLICY.md` | Analytics, consent, retention, erasure, and contact wording. | Confirm it matches intended public distribution and jurisdiction/contact requirements. |
| `THIRD_PARTY_NOTICES.md` | Dependency, runtime, asset, and manifest notices. | Confirm third-party license coverage and required bundled notices. |
| `CREDITS.md` | Release-facing attribution. | Confirm all shipped and promoted materials are credited correctly. |
| `CHANGELOG.md` | Release history and release notes source. | Confirm public wording is acceptable. |
| `docs/APP_RELEASE_READINESS_MATRIX.md` | App-level release-gate status. | Confirm legal rows remain conservative until review is actually complete. |
| `docs/release/AAA_RELEASE_READINESS_REPORT.md` | Authoritative release-readiness verdict. | Confirm the release decision and remaining blockers are truthful. |
| `docs/release/RELEASE_PACKAGING.md` | What the package installs/ships. | Confirm legal review is scoped to the actual package contents. |

## Package Scope To Confirm

Confirm the reviewed package is the CPack/install output produced by:

```powershell
.\tools\ci\run_release_candidate_gate.ps1
```

The package scope currently includes:

- URPG runtime/editor binaries.
- Runtime data under `share/urpg/content/`.
- Asset/source manifests under `share/urpg/imports/manifests/`.
- Release docs under `share/doc/urpg/`.
- Runtime/editor icon PNGs under package metadata paths.

The package scope currently excludes raw source/reference packs under:

- `third_party/itch-assets/`
- `third_party/rpgmaker-mz/`
- `third_party/aseprite/`
- `third_party/huggingface/`
- `third_party/github_assets/`
- `imports/raw/`
- `imports/staging/`
- `imports/normalized/`

If the release owner wants to ship any excluded asset/plugin/source material, stop this walkthrough and perform source-specific license, attribution, and packaging review first.

## Review Steps

### 1. Confirm Distribution Type

Record whether this is:

- internal-only test build,
- private beta,
- public source release,
- public binary prerelease,
- public binary release.

The current `EULA.md` is explicitly internal-only placeholder text. A public binary release requires either legal approval of that text or replacement with a production EULA.

### 2. Review Project License

Check `LICENSE` and confirm:

- copyright owner is correct,
- intended license is correct,
- source distribution obligations are acceptable,
- binary/package distribution can reference this license cleanly.

### 3. Review EULA

Check `EULA.md` and decide whether it is:

- approved for public distribution,
- approved only for internal/private release candidates,
- rejected and must be replaced.

Minimum public-release topics to verify:

- license grant and restrictions,
- ownership of user-created projects,
- third-party component boundaries,
- asset/plugin redistribution limits,
- warranty disclaimer,
- limitation of liability,
- termination,
- governing law and venue,
- privacy-policy linkage,
- support/contact details.

### 4. Review Privacy Policy

Check `PRIVACY_POLICY.md` against the implemented analytics behavior:

- analytics default is `unknown` and uploads are disabled,
- uploads require consent `granted`,
- no upload handler is configured by default in shipped editor/runtime entry points,
- local export/erasure behavior is described honestly,
- public contact/support routing is acceptable.

If a public support or privacy contact does not exist yet, either add one or keep the release blocked.

### 5. Review Third-Party Notices

Check `THIRD_PARTY_NOTICES.md` for every shipped dependency and runtime:

- SDL2,
- MinGW runtime DLLs,
- nlohmann/json,
- Dear ImGui,
- stb,
- QuickJS-NG,
- Catch2 as test-only,
- URPG-owned schemas/templates/manifests,
- promoted proof-lane asset manifests.

For each shipped dependency, confirm:

- upstream license is identified correctly,
- license text or notice requirement is satisfied,
- binary redistribution is allowed,
- required attribution is present,
- dependency is actually shipped or test-only as claimed.

### 6. Review Asset And Plugin Boundaries

Confirm that source/vendor asset packs in `third_party/` and raw/staging imports are not treated as redistributable product content.

Review the promoted proof lanes:

- `SRC-002` / `BND-001`: GDQuest/game-sprites visual proof lane.
- `SRC-003` / `BND-002`: Calinou/kenney-interface-sounds UI SFX proof lane.

Required decision:

- approve current manifest-only release scope,
- approve specific promoted assets for public package distribution,
- or keep asset redistribution blocked.

### 7. Review Credits

Check `CREDITS.md` and confirm:

- project credits are acceptable,
- third-party library credits are complete enough,
- promoted proof-lane attribution is acceptable,
- excluded reference material is not credited as shipped content.

### 8. Review Release Notes

Check `CHANGELOG.md` and confirm:

- public wording is acceptable,
- known limitations are not misleading,
- release-readiness status does not overclaim.

### 9. Record Decision

Fill in `docs/release/LEGAL_REVIEW_SIGNOFF.md` with one of these decisions:

- `APPROVED_FOR_PUBLIC_RELEASE`
- `APPROVED_FOR_PRIVATE_RC_ONLY`
- `REJECTED_CHANGES_REQUIRED`
- `WAIVED_BY_RELEASE_OWNER`

Do not mark `docs/release/AAA_RELEASE_READINESS_REPORT.md` release-ready unless legal review, remote release-candidate workflow verification, and release tagging are all completed or formally waived.

## Recommended Reviewer Questions

- Are we releasing source, binaries, or both?
- Is the current MIT project license the intended source license?
- Is the current EULA production-ready or internal-only?
- Does the privacy policy need a public contact address before release?
- Are all shipped binary dependencies covered by notices and allowed for redistribution?
- Are MinGW runtime DLL redistribution terms acceptable for the final toolchain?
- Are raw RPG Maker DLC, itch asset packs, and other vendor/source packs excluded from public packages?
- Are promoted proof-lane assets approved for the current release scope?
- Are any unreviewed third-party assets accidentally copied into package output?
- Is the release owner willing to ship with repository-wide source/vendor LFS hydration still blocked outside the package path?

## Evidence Commands

Use these commands to regenerate the engineering evidence before signoff:

```powershell
pre-commit run --all-files
.\tools\ci\run_release_candidate_gate.ps1
git status --short --branch
```

Optional package inspection:

```powershell
Get-ChildItem -Recurse .\build\release-candidate-package
Get-ChildItem -Recurse .\build\release-candidate-install
```
