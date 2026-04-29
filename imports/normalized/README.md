# normalized

Governance directory for URPG asset and external repository intake.
See docs/PROGRAM_COMPLETION_STATUS.md (P3-02, P3-03) for program context.

TD Sprint 04 activated the first bounded promoted outputs:

- `prototype_sprites/gdquest_blue_actor.svg`
- `ui_sfx/` remains reserved for future promoted audio; WAV/MP3 payloads are not tracked in GitHub and future promoted audio must use OGG.

Release attribution records for these promoted outputs live under
`imports/reports/asset_intake/attribution/`.

Large local raw drops may be catalog-normalized before binary promotion. `SRC-007` is exposed through
`imports/reports/asset_intake/urpg_stuff_promotion_catalog.json` and category shards under
`imports/reports/asset_intake/urpg_stuff_promotion_catalog/`, which give local raw assets stable virtual
`asset://` normalized paths, tags, preview paths, and duplicate markers for editor/library browsing.
Those catalog records are not release/export eligible until a curated subset is promoted into this
directory or another approved hydrated bundle surface.

Future additions should continue to arrive through source manifests, bundle manifests, and asset-intake reports rather than broad content dumps.
