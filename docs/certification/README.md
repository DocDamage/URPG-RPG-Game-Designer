# Certification And Governance

Status Date: 2026-04-25

FFS-17 adds conservative certification checks for future-feature work. The checks are advisory
unless a separate release gate explicitly promotes them. The current project completeness score is
not a release gate and must not be treated as proof that a template is shippable.

Supported scope:

- Template certification fixtures for JRPG, visual novel, turn-based RPG, tactics RPG, ARPG,
  monster collector, cozy/life RPG, metroidvania-lite, and 2.5D RPG certification suites.
- Feature governance manifests that point at stable docs, schema/data, tests, and owners.
- Project audit completeness output as a non-authoritative advisory signal.

Unsupported scope:

- License clearance certification for assets.
- Automatic READY promotion for templates or subsystems.
- Live-service or vendor-specific certification backends.

Residual gaps remain for full product-depth validation, broad asset entitlement checks, and
human review of release-signoff decisions. Disabled optional features should be excluded from
advisory completeness scoring so intentionally small projects are not penalized.
