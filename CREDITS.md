# Credits

Status Date: 2026-04-27

URPG is developed as a native C++ RPG engine/editor/runtime. This file records release-facing credits and attribution status. It is not a substitute for legal review.

## Project

- URPG engine, editor, runtime, tooling, schemas, docs, and release data: URPG project contributors.
- Project license: MIT, see `LICENSE`.

## Third-Party Code And Libraries

- SDL2: windowing, input, platform integration.
- Dear ImGui: editor UI foundation.
- nlohmann/json: JSON parsing and serialization.
- stb: image loading support.
- QuickJS-NG: JavaScript compatibility runtime.
- Catch2: test framework for non-runtime validation builds.

See `THIRD_PARTY_NOTICES.md` for source paths, license status, install status, and review requirements.

## Asset Intake And Promoted Proof Lanes

- `SRC-002` GDQuest/game-sprites: recorded release-required promoted visual proof lane for `gdquest_blue_actor.svg`, bundled through `BND-001` for title, map, and battle placeholder surfaces.
- `SRC-003` Calinou/kenney-interface-sounds: recorded deferred UI SFX proof lane; the former WAV payload was removed from GitHub and future promoted audio payloads must be OGG.

Both promoted lanes are currently recorded as CC0 candidates for private-use intake. External redistribution remains unverified until a qualified legal reviewer confirms the upstream license evidence and attribution requirements.

## Repository-Only Reference Material

The repository contains source/intake/reference material under `third_party/` and `imports/`. These folders are not automatically release credits. They require source-specific review and promotion before any asset or plugin can be treated as shipped product content.

## Required Before Public Release

- Confirm final third-party dependency license texts.
- Confirm final promoted asset attribution requirements, including `BND-001` if it remains bundled for public distribution.
- Record an explicit public-release waiver if qualified legal review is not obtained.
- Confirm whether RPG Maker DLC/plugin samples or itch asset packs are excluded from all public packages, or add source-specific credits for any approved shipped subset.
- Record legal reviewer signoff in the release signoff workflow.
