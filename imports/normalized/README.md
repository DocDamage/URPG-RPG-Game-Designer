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
`asset://` normalized paths, tags, preview paths, and duplicate status for editor/library browsing.
Those catalog records are not release/export eligible until a curated subset is promoted into this
directory or another approved hydrated bundle surface.

The latest `SRC-007` refresh also catalogs non-audio maps, models, archives, fonts, and tooling metadata.
Generator/tool candidates are tracked separately in
`imports/reports/asset_intake/urpg_stuff_generator_candidates.json`; they are not promoted runtime
assets and require engineering review before any URPG Maker editor or generated-game integration.
Exact raw duplicates were pruned after preserving canonical copies, so the refreshed promotion catalog
currently reports zero duplicate groups.

Future additions should continue to arrive through source manifests, bundle manifests, and asset-intake reports rather than broad content dumps.

`SRC-010` now has three governed normalized promotions:

- `src010_cc0_starter_pack/` is the original small starter subset tracked by `BND-004`.
- `src010_cc0_release_bulk/` is the bulk app-usable CC0/public-domain subset tracked by `BND-006`.
- `src010_newly_licensed_bulk/` is the follow-up app-usable subset unlocked by folder/archive license evidence and tracked by `BND-007`.

The `BND-006` and `BND-007` promotions intentionally exclude duplicate payloads, source/tool/archive files, invalid payloads,
non-OGG audio originals, and packs without CC0/public-domain release evidence. It is release-eligible for
app/library selection, but default export bundling is deferred so export smoke lanes do not package the entire
bulk catalog unless selected by a project. Its scan evidence is recorded in
`imports/reports/asset_intake/src010_release_bulk_scan_report.json`.

`SRC-013` promotes `itch_loose_cc0/` through `BND-008` from the local `itch/loose` CC0 PNG drop. Its evidence is
recorded in `imports/reports/asset_intake/itch_loose_license_scan.json`.
