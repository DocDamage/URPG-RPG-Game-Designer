# Vision Tooling

Offline segmentation and asset-prep tools live here.

The Phase 11 entrypoint is `segment_assets.py`. It writes
`content/schemas/segmentation_manifest.schema.json` manifests and records
`skipped`, `reused`, or `generated` rows so reruns preserve manual overrides.
Dependencies in `requirements.txt` are isolated helper dependencies for offline jobs;
the shipped runtime consumes only masks, cutouts, and JSON manifests.

Mandatory Phase 11 scope:

- SAM / SAM2-compatible batch segmentation
- cutout and mask export
- per-asset manifest generation
- manual-override-aware reruns
