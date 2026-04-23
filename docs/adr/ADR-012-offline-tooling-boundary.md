# ADR-012: Offline Tooling Boundary For ML And Research Integrations

## Status

Accepted - 2026-04-22

## Context

URPG is adding a bounded offline tooling lane for semantic retrieval, image segmentation, audio processing, and importer-side content preparation. Several candidate integrations come from research-oriented ecosystems and may depend on heavyweight Python/ML stacks.

URPG's shipped runtime and editor must remain deterministic, reviewable, and easy to validate. Pulling research dependencies directly into the player or native runtime would increase build complexity, blur ownership boundaries, and make completion claims harder to keep honest.

## Decision

URPG will keep ML and research integrations behind an explicit offline tooling boundary.

Rules:

1. Heavy ML/research dependencies live under `tools/` or in a separate helper environment.
2. The shipped runtime consumes exported artifacts only:
   - JSON manifests
   - index bundles
   - PNG/WebP cutouts
   - WAV/OGG outputs
   - metadata
3. No PyTorch-heavy or research-stack dependency enters the runtime/player build unless a later ADR proves it is required.
4. Offline jobs must be restartable, inspectable, and safe to rerun.
5. Outputs must be versionable and traceable to their source inputs and job metadata.

## Consequences

Positive:

- keeps the native runtime clean and reviewable
- allows experimentation in retrieval, segmentation, and audio tooling without destabilizing shipped code
- gives authors better asset/content pipelines while preserving runtime truthfulness

Costs:

- requires manifest schemas and job metadata discipline
- requires separate tooling documentation and helper environments
- some integrations will remain tooling-only even if they are useful

## Initial Approved Scope

The first approved tooling priorities are:

- FAISS-style retrieval tooling under `tools/retrieval`
- SAM / SAM2-style segmentation tooling under `tools/vision`
- Demucs / Encodec-style audio tooling under `tools/audio`

These are approved as offline pipeline scope only.
