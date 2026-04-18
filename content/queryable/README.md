# Queryable External Feature Intel

This folder stores ingestion-ready research data for URPG planning.

## Current dataset

- `github_codeberg_feature_intel_2026-04-15.jsonl`
- `github_codeberg_feature_intel_2026-04-15_batch2.jsonl`
- `feature_upgrade_tickets_2026-04-15.jsonl`

Each line is a JSON object with:

- `id`
- `scan_date`
- `source_platform`
- `source_repo`
- `source_url`
- `source_updated_at`
- `signal_type`
- `feature_signal`
- `evidence`
- `urpg_target_lane`
- `confidence`
- `actionability`
- `tags`

## Suggested indexing

```powershell
.\tools\codemunch\index-project.ps1 -ProjectRoot . -Embed -OutFile ".codemunch\last-index.json"
```

If you also mirror into ContextLattice, include this folder in your normal project ingestion pass.
