# Audio Tooling

Offline audio tooling lives here.

The Phase 8 entrypoint is `process_audio_assets.py`. It writes
`content/schemas/audio_tool_manifest.schema.json` manifests for Demucs-style
stems and Encodec-style compression experiments. Generated prototype outputs
are non-release by default until reviewed and promoted.

Planned scope:

- stem separation
- compression experiments
- temporary prototype-audio generation
- processed-audio manifest generation

The shipped runtime must consume output files and manifests only.
