# Audio Pipeline

The offline audio pipeline prepares source audio into stable processed outputs for runtime consumption.

Current boundary rules:

- stem separation and compression experiments stay under `tools/audio`
- generated prototype audio must be clearly labeled as temporary
- runtime consumes exported audio files and manifests only

Initial target outputs:

- separated stems
- processed runtime-ready files
- experiment reports
- per-output manifests
