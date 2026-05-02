# Job Runner

The job runner provides one repeatable entry point for offline tooling jobs.

Current job types:

- `retrieval_chunk_manifest`
- `retrieval_bundle`
- `vision_segmentation`
- `audio_processing`

Usage:

```powershell
python tools/shared/job_runner/run_offline_job.py --job-file path\to\job.json
```

Example job files:

- `tools/shared/job_runner/example_retrieval_job.json`
- `tools/shared/job_runner/example_retrieval_bundle_job.json`
- `tools/shared/job_runner/example_vision_segmentation_job.json`
- `tools/shared/job_runner/example_audio_processing_job.json`

All approved offline tooling lanes must be invokable through this runner before they are counted toward the 100-percent completion claim.
