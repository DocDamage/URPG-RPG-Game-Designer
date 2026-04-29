# Hugging Face Curated Imports

This repo carries a small curated seed set from a few Hugging Face datasets that are useful for importer work, spatial-map experiments, dialogue tooling, and cross-engine reference material.

## Policy

- Vendor direct only when the dataset license is straightforward for in-repo storage.
- Keep restrictive corpora as manifest-only sources that can be fetched on demand.
- Preserve original source-relative paths under `imports/raw/third_party_assets/huggingface/` for traceability.

## Current Sources

- `NintenHero/RPG-Maker-MV-Data`
  - License: `cc-by-nc-sa-4.0`
  - Mode: manifest-only
  - Use: MV project-data references for future import/migration tooling.
- `NintenHero/RPG-Maker-XP-Data`
  - License: `cc-by-nc-sa-4.0`
  - Mode: manifest-only
  - Use: XP raw-data references for fixture planning and importer design.
- `NintenHero/Tiled-Map-Editor-TMX`
  - License: `bsd-3-clause`
  - Mode: vendored curated samples
  - Use: TMX importer fixtures and spatial presentation experiments.
- `NintenHero/Visual-Novel-Maker-Data`
  - License: `apache-2.0`
  - Mode: vendored curated samples
  - Use: dialogue/event data references.
- `wallstoneai/godot-gdscript-dataset`
  - License: `apache-2.0`
  - Mode: vendored curated samples
  - Use: cross-engine scripting references for tooling and migration research.

## Refresh

```powershell
.\tools\assets\ingest_huggingface_curated.ps1
.\tools\assets\ingest_huggingface_curated.ps1 -Refresh
```

Manifest:
- `tools/assets/huggingface_curated_manifest.json`

Generated report:
- `imports/reports/huggingface_curated_inventory.json`
