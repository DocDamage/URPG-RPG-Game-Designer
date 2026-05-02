# Vision Tooling

Offline segmentation and asset-prep tools live here.

The Phase 8 entrypoint is `segment_assets.py`. It writes
`content/schemas/segmentation_manifest.schema.json` manifests and records
`skipped`, `reused`, or `generated` rows so reruns preserve manual overrides.
Dependencies in `requirements.txt` are optional offline tooling dependencies;
the shipped runtime consumes only masks, cutouts, and JSON manifests.

Planned scope:

- SAM / SAM2-compatible batch segmentation
- cutout and mask export
- per-asset manifest generation
- manual-override-aware reruns
